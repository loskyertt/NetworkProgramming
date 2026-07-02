//
// Created by sky on 2026/7/2.
//
#include "object_pool/object_pool.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <set>

// ---------- 用于追踪构造/析构次数和参数转发是否正确的类型 ----------
struct Tracked {
  static int ctor_count;
  static int dtor_count;
  static int move_ctor_count;
  static int copy_ctor_count;

  int value;
  std::string name;

  Tracked(int v, std::string n) : value(v), name(std::move(n)) { ++ctor_count; }

  Tracked(const Tracked &other) : value(other.value), name(other.name) {
    ++ctor_count;
    ++copy_ctor_count;
  }

  Tracked(Tracked &&other) noexcept : value(other.value), name(std::move(other.name)) {
    ++ctor_count;
    ++move_ctor_count;
  }

  ~Tracked() { ++dtor_count; }
};

int Tracked::ctor_count = 0;
int Tracked::dtor_count = 0;
int Tracked::move_ctor_count = 0;
int Tracked::copy_ctor_count = 0;

#define CHECK(cond)                                                        \
  do {                                                                     \
    if (!(cond)) {                                                         \
      std::cerr << "FAILED: " << #cond << " at line " << __LINE__ << "\n"; \
      std::exit(1);                                                        \
    }                                                                      \
  } while (0)

// 1. 基本创建/销毁 + live_count 正确性
void test_basic_create_destroy() {
  ObjectPool<Tracked> pool(4);
  CHECK(pool.get_live_count() == 0);

  Tracked *a = pool.create(1, "a");
  Tracked *b = pool.create(2, "b");
  CHECK(pool.get_live_count() == 2);
  CHECK(a->value == 1 && a->name == "a");
  CHECK(b->value == 2 && b->name == "b");

  pool.destroy(a);
  CHECK(pool.get_live_count() == 1);
  pool.destroy(b);
  CHECK(pool.get_live_count() == 0);

  std::cout << "[PASS] test_basic_create_destroy\n";
}

// 2. 验证空闲链表按 LIFO 复用：destroy 后立即 create 应拿到同一块内存
void test_slot_reuse_lifo() {
  ObjectPool<Tracked> pool(4);
  Tracked *a = pool.create(1, "a");
  void *addr_a = static_cast<void *>(a);
  pool.destroy(a);

  Tracked *b = pool.create(2, "b");
  void *addr_b = static_cast<void *>(b);

  CHECK(addr_a == addr_b);  // 同一个 slot 被复用
  pool.destroy(b);

  std::cout << "[PASS] test_slot_reuse_lifo\n";
}

// 3. 超过 initial_capacity 时应自动扩容（触发多个 Block），且地址不重复
void test_expand_multiple_blocks() {
  const std::size_t cap = 4;
  ObjectPool<Tracked> pool(cap);

  std::vector<Tracked *> ptrs;
  std::set<void *> addrs;
  const int total = static_cast<int>(cap) * 3 + 1;  // 强制跨越多个 block

  for (int i = 0; i < total; ++i) {
    Tracked *p = pool.create(i, "n" + std::to_string(i));
    CHECK(addrs.find(static_cast<void *>(p)) == addrs.end());  // 地址不重复
    addrs.insert(static_cast<void *>(p));
    ptrs.push_back(p);
  }
  CHECK(pool.get_live_count() == static_cast<std::size_t>(total));

  // 校验每个对象的数据没有被踩内存
  for (int i = 0; i < total; ++i) {
    CHECK(ptrs[i]->value == i);
    CHECK(ptrs[i]->name == "n" + std::to_string(i));
  }

  for (auto *p : ptrs)
    pool.destroy(p);
  CHECK(pool.get_live_count() == 0);

  std::cout << "[PASS] test_expand_multiple_blocks\n";
}

// 4. 构造/析构次数应严格匹配，不多不少（检查有没有内存/对象泄漏）
void test_ctor_dtor_balance() {
  Tracked::ctor_count = 0;
  Tracked::dtor_count = 0;
  {
    ObjectPool<Tracked> pool(3);
    std::vector<Tracked *> ptrs;
    for (int i = 0; i < 10; ++i)
      ptrs.push_back(pool.create(i, "x"));
    CHECK(Tracked::ctor_count == 10);
    CHECK(Tracked::dtor_count == 0);

    for (auto *p : ptrs)
      pool.destroy(p);
    CHECK(Tracked::dtor_count == 10);
  }  // pool 析构（此时所有对象已被 destroy，符合该实现的使用约定）

  std::cout << "[PASS] test_ctor_dtor_balance\n";
}

// 5. 完美转发：左值应触发拷贝构造，右值(std::move)应触发移动构造
void test_perfect_forwarding() {
  Tracked::copy_ctor_count = 0;
  Tracked::move_ctor_count = 0;

  ObjectPool<Tracked> pool(4);

  Tracked lval(1, "lvalue");
  Tracked *p1 = pool.create(lval);  // 拷贝
  CHECK(Tracked::copy_ctor_count == 1);
  CHECK(Tracked::move_ctor_count == 0);

  Tracked *p2 = pool.create(std::move(lval));  // 移动
  CHECK(Tracked::move_ctor_count == 1);

  pool.destroy(p1);
  pool.destroy(p2);

  std::cout << "[PASS] test_perfect_forwarding\n";
}

// 6. 对齐正确性：确保返回的对象地址满足 T 的对齐要求
struct alignas(32) Aligned32 {
  int x;

  Aligned32(int v) : x(v) {}
};

void test_alignment() {
  ObjectPool<Aligned32> pool(8);
  for (int i = 0; i < 20; ++i) {
    Aligned32 *p = pool.create(i);
    CHECK(reinterpret_cast<uintptr_t>(p) % alignof(Aligned32) == 0);
  }
  std::cout << "[PASS] test_alignment\n";
}

int main() {
  test_basic_create_destroy();
  test_slot_reuse_lifo();
  test_expand_multiple_blocks();
  test_ctor_dtor_balance();
  test_perfect_forwarding();
  test_alignment();
  std::cout << "ALL TESTS PASSED\n";
  return 0;
}