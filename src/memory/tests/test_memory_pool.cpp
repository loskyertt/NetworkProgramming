//
// Created by sky on 2026/6/24.
//

#include "memory_pool/fixed_memory_pool.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <new>
#include <vector>

// ─── 防止编译器优化掉基准循环 ──────────────────────────────────
// 将指针逃逸出编译器可见范围，使分配/释放不能被优化掉
static void escape(void* p) {
  // 使编译器认为 p 被外部读取，无法消除对 p 涉及的任何操作
  asm volatile("" : : "r"(p) : "memory");
}

// ─── 被池管理的对象 ────────────────────────────────────────────
struct Connection {
  int fd;
  char ip[16];

  Connection(int f, const char *s) : fd(f) { std::strcpy(ip, s); }

  ~Connection() { /* 关闭 fd 等 */ }
};

// ─── 辅助：计时 ────────────────────────────────────────────────
using Clock = std::chrono::high_resolution_clock;
using ns = std::chrono::nanoseconds;

static double elapsed_ms(Clock::time_point t0, Clock::time_point t1) {
  return std::chrono::duration_cast<ns>(t1 - t0).count() / 1e6;
}

// ─── 1. 基本功能测试 ────────────────────────────────────────────
static void test_basic() {
  std::printf("\n========== 1. 基本功能测试 ==========\n");

  FixedMemoryPool pool(sizeof(Connection), 4);

  // 分配 4 个对象（刚好耗尽初始池）
  Connection* conns[4];
  for (int i = 0; i < 4; ++i) {
    void* mem = pool.allocate();
    conns[i] = new (mem) Connection(i + 1, "127.0.0.1");
  }

  // 验证对象构造正确
  bool ok = true;
  for (int i = 0; i < 4; ++i) {
    if (conns[i]->fd != i + 1 || std::strcmp(conns[i]->ip, "127.0.0.1") != 0) {
      ok = false;
      std::printf("  [FAIL] conns[%d]: fd=%d, ip=%s\n", i, conns[i]->fd, conns[i]->ip);
    }
  }
  if (ok) std::printf("  [PASS] 4 个 Connection 构造正确\n");

  // 手动析构 + 释放
  for (int i = 0; i < 4; ++i) {
    conns[i]->~Connection();
    pool.deallocate(conns[i]);
  }
  std::printf("  [PASS] 4 个 Connection 析构 + 回收成功\n");
}

// ─── 2. 自动扩容测试 ────────────────────────────────────────────
static void test_expand() {
  std::printf("\n========== 2. 自动扩容测试 ==========\n");

  // 初始只有 2 个 chunk
  FixedMemoryPool pool(sizeof(Connection), 2);

  Connection* conns[10];
  for (int i = 0; i < 10; ++i) {
    void* mem = pool.allocate();
    conns[i] = new (mem) Connection(i, "10.0.0.1");
  }
  std::printf("  [PASS] 从初始 2 chunk 扩容到容纳 10 个对象\n");

  // 验证数据正确
  bool ok = true;
  for (int i = 0; i < 10; ++i) {
    if (conns[i]->fd != i || std::strcmp(conns[i]->ip, "10.0.0.1") != 0) {
      ok = false;
    }
  }
  if (ok) std::printf("  [PASS] 扩容后所有对象数据正确\n");

  for (int i = 0; i < 10; ++i) {
    conns[i]->~Connection();
    pool.deallocate(conns[i]);
  }
  std::printf("  [PASS] 扩容后的对象全部回收成功\n");
}

// ─── 3. 重复分配/释放测试 ──────────────────────────────────────
static void test_reuse() {
  std::printf("\n========== 3. 重复分配/释放测试 ==========\n");

  FixedMemoryPool pool(sizeof(Connection), 4);

  // 反复 alloc → dealloc，验证池能正确回收和复用 chunk
  for (int round = 0; round < 100; ++round) {
    void* mem = pool.allocate();
    auto* conn = new (mem) Connection(round, "192.168.1.1");
    conn->~Connection();
    pool.deallocate(conn);
  }
  std::printf("  [PASS] 100 轮 alloc/dealloc 循环无异常\n");
}

// ─── 4. deallocate(nullptr) 安全性测试 ──────────────────────────
static void test_null_dealloc() {
  std::printf("\n========== 4. deallocate(nullptr) 安全性 ==========\n");

  FixedMemoryPool pool(sizeof(int), 4);
  pool.deallocate(nullptr);  // 应该安全跳过
  std::printf("  [PASS] deallocate(nullptr) 不崩溃\n");
}

// ─── 5. 性能基准：逐次分配+释放 ────────────────────────────────
static void bench_allocate_deallocate() {
  std::printf("\n========== 5. 性能基准：逐次分配+释放 ==========\n");

  constexpr std::size_t OBJ_SIZE = sizeof(Connection);
  constexpr int N = 10'000'000;

  // --- FixedMemoryPool ---
  {
    FixedMemoryPool pool(OBJ_SIZE, 1024);

    auto t0 = Clock::now();
    for (int i = 0; i < N; ++i) {
      void* mem = pool.allocate();
      new (mem) Connection(i, "127.0.0.1");
      static_cast<Connection*>(mem)->~Connection();
      pool.deallocate(mem);
      escape(mem);
    }
    auto t1 = Clock::now();

    std::printf("  FixedMemoryPool : %8.2f ms  (%d 次 alloc+dealloc)\n",
                elapsed_ms(t0, t1), N);
  }

  // --- malloc / free ---
  {
    auto t0 = Clock::now();
    for (int i = 0; i < N; ++i) {
      void* mem = std::malloc(OBJ_SIZE);
      new (mem) Connection(i, "127.0.0.1");
      static_cast<Connection*>(mem)->~Connection();
      std::free(mem);
      escape(mem);
    }
    auto t1 = Clock::now();

    std::printf("  malloc/free     : %8.2f ms  (%d 次 alloc+dealloc)\n",
                elapsed_ms(t0, t1), N);
  }

  // --- new / delete ---
  {
    auto t0 = Clock::now();
    for (int i = 0; i < N; ++i) {
      auto* p = new Connection(i, "127.0.0.1");
      delete p;
      escape(p);
    }
    auto t1 = Clock::now();

    std::printf("  new/delete      : %8.2f ms  (%d 次 alloc+dealloc)\n",
                elapsed_ms(t0, t1), N);
  }
}

// ─── 6. 性能基准：批量分配再批量释放 ────────────────────────────
static void bench_batch() {
  std::printf("\n========== 6. 性能基准：批量分配再批量释放 ==========\n");

  constexpr std::size_t OBJ_SIZE = sizeof(Connection);
  constexpr int N = 1'000'000;

  // --- FixedMemoryPool ---
  {
    FixedMemoryPool pool(OBJ_SIZE, 1024);
    std::vector<void*> ptrs;
    ptrs.reserve(N);

    auto t0 = Clock::now();

    for (int i = 0; i < N; ++i) {
      void* mem = pool.allocate();
      new (mem) Connection(i, "127.0.0.1");
      ptrs.push_back(mem);
    }
    for (auto* p : ptrs) {
      static_cast<Connection*>(p)->~Connection();
      pool.deallocate(p);
    }

    auto t1 = Clock::now();
    std::printf("  FixedMemoryPool : %8.2f ms  (批量 %d 次)\n",
                elapsed_ms(t0, t1), N);
  }

  // --- malloc / free ---
  {
    std::vector<void*> ptrs;
    ptrs.reserve(N);

    auto t0 = Clock::now();

    for (int i = 0; i < N; ++i) {
      void* mem = std::malloc(OBJ_SIZE);
      new (mem) Connection(i, "127.0.0.1");
      ptrs.push_back(mem);
    }
    for (auto* p : ptrs) {
      static_cast<Connection*>(p)->~Connection();
      std::free(p);
    }

    auto t1 = Clock::now();
    std::printf("  malloc/free     : %8.2f ms  (批量 %d 次)\n",
                elapsed_ms(t0, t1), N);
  }

  // --- new / delete ---
  {
    std::vector<void*> ptrs;
    ptrs.reserve(N);

    auto t0 = Clock::now();

    for (int i = 0; i < N; ++i) {
      auto* p = new Connection(i, "127.0.0.1");
      ptrs.push_back(p);
    }
    for (auto* p : ptrs) {
      delete static_cast<Connection*>(p);
    }

    auto t1 = Clock::now();
    std::printf("  new/delete      : %8.2f ms  (批量 %d 次)\n",
                elapsed_ms(t0, t1), N);
  }
}

// ─── 7. 性能基准：多轮扩容压力测试 ──────────────────────────────
static void bench_expand_stress() {
  std::printf("\n========== 7. 扩容压力测试 ==========\n");

  // 初始只有 1 个 chunk，强制反复扩容
  constexpr int N = 100'000;
  constexpr std::size_t OBJ_SIZE = sizeof(Connection);

  FixedMemoryPool pool(OBJ_SIZE, 1);
  std::vector<void*> ptrs;
  ptrs.reserve(N);

  auto t0 = Clock::now();
  for (int i = 0; i < N; ++i) {
    void* mem = pool.allocate();
    new (mem) Connection(i, "0.0.0.0");
    ptrs.push_back(mem);
  }
  for (auto* p : ptrs) {
    static_cast<Connection*>(p)->~Connection();
    pool.deallocate(p);
  }
  auto t1 = Clock::now();

  std::printf("  从 1 chunk 扩容到 %d 个: %8.2f ms\n", N, elapsed_ms(t0, t1));
}

// ─── main ──────────────────────────────────────────────────────
int main() {
  std::printf("╔══════════════════════════════════════════════╗\n");
  std::printf("║       FixedMemoryPool 测试 & 性能基准        ║\n");
  std::printf("╚══════════════════════════════════════════════╝\n");

  test_basic();
  test_expand();
  test_reuse();
  test_null_dealloc();

  bench_allocate_deallocate();
  bench_batch();
  bench_expand_stress();

  std::printf("\n========== 全部测试完成 ==========\n");
  return 0;
}
