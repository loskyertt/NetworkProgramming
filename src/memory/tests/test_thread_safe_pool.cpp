//
// Created on 2026/6/30.
//
// ThreadSafePool 多线程正确性与性能测试

#include "memory_pool/thread_safe_pool.h"
#include "memory_pool/fixed_memory_pool.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <new>
#include <thread>
#include <vector>

// ─── 防止编译器优化掉基准循环 ──────────────────────────────────
static void escape(void* p) {
  asm volatile("" : : "r"(p) : "memory");
}

// ─── 被池管理的对象 ────────────────────────────────────────────
struct Connection {
  int fd;
  char ip[16];

  Connection(int f, const char* s) : fd(f) { std::strcpy(ip, s); }
  ~Connection() { /* 关闭 fd 等 */ }
};

// ─── 辅助：计时 ────────────────────────────────────────────────
using Clock = std::chrono::high_resolution_clock;
using ns    = std::chrono::nanoseconds;

static double elapsed_ms(Clock::time_point t0, Clock::time_point t1) {
  return std::chrono::duration_cast<ns>(t1 - t0).count() / 1e6;
}

// ─── 1. 基本多线程正确性测试 ────────────────────────────────────
//   N 个线程各分配 M 个对象，验证数据正确，然后统一析构+释放
static void test_basic_concurrent() {
  std::printf("\n========== 1. 基本多线程正确性测试 ==========\n");

  constexpr int k_num_threads = 8;
  constexpr int k_per_thread  = 100;
  constexpr std::size_t k_obj_size = sizeof(Connection);

  ThreadSafePool pool(k_obj_size, k_per_thread * k_num_threads);

  std::vector<std::thread> threads;
  // 每个线程保存自己分配的指针
  std::vector<std::vector<void*>> all_ptrs(k_num_threads);

  std::atomic<bool> start{false};

  for (int t = 0; t < k_num_threads; ++t) {
    threads.emplace_back([&, t]() {
      // 等待所有线程就绪后同时开始，增加并发冲突概率
      while (!start.load()) {
        // spin wait
      }
      auto& ptrs = all_ptrs[t];
      ptrs.reserve(k_per_thread);
      for (int i = 0; i < k_per_thread; ++i) {
        void* mem = pool.allocate();
        new (mem) Connection(t * k_per_thread + i, "127.0.0.1");
        ptrs.push_back(mem);
      }
    });
  }

  start.store(true);  // 释放所有线程

  for (auto& th : threads) th.join();

  // 验证数据正确性
  bool ok = true;
  int total = 0;
  for (int t = 0; t < k_num_threads; ++t) {
    for (int i = 0; i < k_per_thread; ++i) {
      auto* conn = static_cast<Connection*>(all_ptrs[t][i]);
      int expected_fd = t * k_per_thread + i;
      if (conn->fd != expected_fd || std::strcmp(conn->ip, "127.0.0.1") != 0) {
        ok = false;
        std::printf("  [FAIL] thread=%d, idx=%d: fd=%d, ip=%s\n",
                     t, i, conn->fd, conn->ip);
      }
      ++total;
    }
  }
  if (ok) {
    std::printf("  [PASS] %d 个 Connection 全部构造正确（%d 线程并发）\n",
                total, k_num_threads);
  }

  // 统一析构 + 释放
  for (int t = 0; t < k_num_threads; ++t) {
    for (auto* p : all_ptrs[t]) {
      static_cast<Connection*>(p)->~Connection();
      pool.deallocate(p);
    }
  }
  std::printf("  [PASS] %d 个 Connection 全部析构 + 回收成功\n", total);
}

// ─── 2. 并发 alloc/dealloc 循环测试 ────────────────────────────
//   N 个线程各执行 M 次 allocate→construct→destruct→deallocate 循环
static void test_concurrent_alloc_dealloc() {
  std::printf("\n========== 2. 并发 alloc/dealloc 循环测试 ==========\n");

  constexpr int k_num_threads = 8;
  constexpr int k_per_thread  = 10'000;
  constexpr std::size_t k_obj_size = sizeof(Connection);

  ThreadSafePool pool(k_obj_size, 64);

  std::atomic<bool> start{false};
  std::atomic<int> error_count{0};

  std::vector<std::thread> threads;

  for (int t = 0; t < k_num_threads; ++t) {
    threads.emplace_back([&, t]() {
      while (!start.load()) {}

      for (int i = 0; i < k_per_thread; ++i) {
        void* mem = pool.allocate();
        new (mem) Connection(t * k_per_thread + i, "192.168.1.1");
        auto* conn = static_cast<Connection*>(mem);

        // 立即验证刚构造的对象
        if (conn->fd != t * k_per_thread + i || std::strcmp(conn->ip, "192.168.1.1") != 0) {
          error_count.fetch_add(1);
        }

        conn->~Connection();
        pool.deallocate(mem);
        escape(mem);
      }
    });
  }

  start.store(true);

  for (auto& th : threads) th.join();

  if (error_count.load() == 0) {
    std::printf("  [PASS] %d 线程 × %d 次 alloc/dealloc 循环无数据损坏\n",
                k_num_threads, k_per_thread);
  } else {
    std::printf("  [FAIL] 数据损坏次数: %d\n", error_count.load());
  }
}

// ─── 3. 多线程性能基准 ──────────────────────────────────────────
//   对比 ThreadSafePool / malloc/free / new/delete 在多线程下的吞吐量
static void bench_concurrent_throughput() {
  std::printf("\n========== 3. 多线程性能基准 ==========\n");

  constexpr int k_num_threads = 4;
  constexpr int k_per_thread  = 1'000'000;
  constexpr std::size_t k_obj_size = sizeof(Connection);

  std::printf("  线程数: %d, 每线程操作数: %d\n", k_num_threads, k_per_thread);

  std::atomic<bool> start{false};

  // --- ThreadSafePool ---
  {
    ThreadSafePool pool(k_obj_size, 1024);
    std::vector<std::thread> threads;

    auto t0 = Clock::now();
    start.store(false);

    for (int t = 0; t < k_num_threads; ++t) {
      threads.emplace_back([&, t]() {
        while (!start.load()) {}
        for (int i = 0; i < k_per_thread; ++i) {
          void* mem = pool.allocate();
          new (mem) Connection(i, "127.0.0.1");
          static_cast<Connection*>(mem)->~Connection();
          pool.deallocate(mem);
          escape(mem);
        }
      });
    }

    start.store(true);
    for (auto& th : threads) th.join();
    auto t1 = Clock::now();

    std::printf("  ThreadSafePool  : %8.2f ms  (%d 线程 × %d alloc+dealloc)\n",
                elapsed_ms(t0, t1), k_num_threads, k_per_thread);
  }

  // --- malloc / free ---
  {
    std::vector<std::thread> threads;

    auto t0 = Clock::now();
    start.store(false);

    for (int t = 0; t < k_num_threads; ++t) {
      threads.emplace_back([&, t]() {
        while (!start.load()) {}
        for (int i = 0; i < k_per_thread; ++i) {
          void* mem = std::malloc(k_obj_size);
          new (mem) Connection(i, "127.0.0.1");
          static_cast<Connection*>(mem)->~Connection();
          std::free(mem);
          escape(mem);
        }
      });
    }

    start.store(true);
    for (auto& th : threads) th.join();
    auto t1 = Clock::now();

    std::printf("  malloc/free     : %8.2f ms  (%d 线程 × %d alloc+dealloc)\n",
                elapsed_ms(t0, t1), k_num_threads, k_per_thread);
  }

  // --- new / delete ---
  {
    std::vector<std::thread> threads;

    auto t0 = Clock::now();
    start.store(false);

    for (int t = 0; t < k_num_threads; ++t) {
      threads.emplace_back([&, t]() {
        while (!start.load()) {}
        for (int i = 0; i < k_per_thread; ++i) {
          auto* p = new Connection(i, "127.0.0.1");
          delete p;
          escape(p);
        }
      });
    }

    start.store(true);
    for (auto& th : threads) th.join();
    auto t1 = Clock::now();

    std::printf("  new/delete      : %8.2f ms  (%d 线程 × %d alloc+dealloc)\n",
                elapsed_ms(t0, t1), k_num_threads, k_per_thread);
  }
}

// ─── 4. 多线程 vs 单线程对比 ────────────────────────────────────
//   单线程 FixedMemoryPool vs 多线程 ThreadSafePool 的吞吐量对比
static void bench_single_vs_multi() {
  std::printf("\n========== 4. 单线程 vs 多线程对比 ==========\n");

  constexpr int k_total_ops       = 4'000'000;
  constexpr int k_num_threads     = 4;
  constexpr int k_per_thread      = k_total_ops / k_num_threads;
  constexpr std::size_t k_obj_size = sizeof(Connection);

  std::printf("  总操作数: %d\n", k_total_ops);

  // --- 单线程 FixedMemoryPool ---
  {
    FixedMemoryPool pool(k_obj_size, 1024);

    auto t0 = Clock::now();
    for (int i = 0; i < k_total_ops; ++i) {
      void* mem = pool.allocate();
      new (mem) Connection(i, "127.0.0.1");
      static_cast<Connection*>(mem)->~Connection();
      pool.deallocate(mem);
      escape(mem);
    }
    auto t1 = Clock::now();

    std::printf("  FixedMemoryPool (单线程) : %8.2f ms  (%d alloc+dealloc)\n",
                elapsed_ms(t0, t1), k_total_ops);
  }

  // --- 4 线程 ThreadSafePool ---
  {
    ThreadSafePool pool(k_obj_size, 1024);
    std::vector<std::thread> threads;
    std::atomic<bool> start{false};

    auto t0 = Clock::now();
    start.store(false);

    for (int t = 0; t < k_num_threads; ++t) {
      threads.emplace_back([&, t]() {
        while (!start.load()) {}
        for (int i = 0; i < k_per_thread; ++i) {
          void* mem = pool.allocate();
          new (mem) Connection(t * k_per_thread + i, "127.0.0.1");
          static_cast<Connection*>(mem)->~Connection();
          pool.deallocate(mem);
          escape(mem);
        }
      });
    }

    start.store(true);
    for (auto& th : threads) th.join();
    auto t1 = Clock::now();

    std::printf("  ThreadSafePool  (4线程)  : %8.2f ms  (%d alloc+dealloc)\n",
                elapsed_ms(t0, t1), k_total_ops);
  }
}

// ─── 5. 多线程扩容压力测试 ──────────────────────────────────────
//   初始只有极少量 chunk，多线程并发分配触发反复扩容
static void bench_expand_stress_multi() {
  std::printf("\n========== 5. 多线程扩容压力测试 ==========\n");

  constexpr int k_num_threads = 8;
  constexpr int k_per_thread  = 10'000;
  constexpr std::size_t k_obj_size = sizeof(Connection);

  // 初始只有 4 个 chunk，8 个线程各分配 10k 个 → 大量扩容
  ThreadSafePool pool(k_obj_size, 4);

  std::vector<std::thread> threads;
  std::vector<std::vector<void*>> all_ptrs(k_num_threads);
  std::atomic<bool> start{false};

  auto t0 = Clock::now();

  for (int t = 0; t < k_num_threads; ++t) {
    threads.emplace_back([&, t]() {
      while (!start.load()) {}
      auto& ptrs = all_ptrs[t];
      ptrs.reserve(k_per_thread);
      for (int i = 0; i < k_per_thread; ++i) {
        void* mem = pool.allocate();
        new (mem) Connection(t * k_per_thread + i, "0.0.0.0");
        ptrs.push_back(mem);
      }
    });
  }

  start.store(true);
  for (auto& th : threads) th.join();

  // 释放所有对象
  for (int t = 0; t < k_num_threads; ++t) {
    for (auto* p : all_ptrs[t]) {
      static_cast<Connection*>(p)->~Connection();
      pool.deallocate(p);
    }
  }

  auto t1 = Clock::now();

  std::printf("  %d 线程从 4 chunk 扩容到 %d 个对象: %8.2f ms\n",
              k_num_threads, k_num_threads * k_per_thread, elapsed_ms(t0, t1));
  std::printf("  [PASS] 扩容压力测试完成，无崩溃\n");
}

// ─── main ──────────────────────────────────────────────────────
int main() {
  std::printf("╔══════════════════════════════════════════════╗\n");
  std::printf("║     ThreadSafePool 多线程测试 & 性能基准     ║\n");
  std::printf("╚══════════════════════════════════════════════╝\n");

  test_basic_concurrent();
  test_concurrent_alloc_dealloc();

  bench_concurrent_throughput();
  bench_single_vs_multi();
  bench_expand_stress_multi();

  std::printf("\n========== 全部测试完成 ==========\n");
  return 0;
}
