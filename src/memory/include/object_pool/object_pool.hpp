//
// Created by sky on 2026/7/2.
//

#ifndef NETWORK_PROGRAMMING_OBJECT_POOL_H
#define NETWORK_PROGRAMMING_OBJECT_POOL_H
#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>

template <typename T>
class ObjectPool {
 private:
  // 槽位，每个槽位可容纳一个 T（对象）的存储单元
  union Slot {
    Slot *next;
    T object;

    Slot() {}

    ~Slot() {}
  };

  // 存放预申请的内存块的首地址
  struct Block {
    Block *next;
  };

  Block *m_block_head = nullptr;
  Slot *m_free_head = nullptr;
  std::size_t m_block_capacity = 0;
  std::size_t m_live_count = 0;  // 已有的对象的数量

  /**
   * @details 把地址按 alignment 对齐
   *
   * @param p 原始地址
   * @param alignment 对齐字节大小
   * @return 返回按 alignment 对齐后的基地址
   */
  static char *align_forward(char *p, std::size_t alignment) {
    auto addr = reinterpret_cast<uintptr_t>(p);
    auto aligned = (addr + alignment - 1) & ~(alignment - 1);
    return reinterpret_cast<char *>(aligned);
  }

  /**
   * @details 申请空闲内存块，然后把内存块切成 Slot
   *
   * @param count Slot 的数量
   */
  void expand(std::size_t count) {
    std::size_t raw_size = sizeof(Block) + alignof(Slot) - 1 + sizeof(Slot) * count;
    Block *block = static_cast<Block *>(::operator new(raw_size));
    block->next = m_block_head;
    m_block_head = block;

    char *raw = reinterpret_cast<char *>(block) + sizeof(Block);
    char *base = align_forward(raw, alignof(Slot));
    for (int i = 0; i < m_block_capacity; ++i) {
      Slot *slot = reinterpret_cast<Slot *>(base + i * sizeof(Slot));
      slot->next = m_free_head;
      m_free_head = slot;
    }
  }

  ObjectPool(const ObjectPool &) = delete;
  ObjectPool &operator=(const ObjectPool &) = delete;

 public:
  /**
   *
   * @param initial_capacity 预申请的存放对象槽位的数量（容量）
   */
  ObjectPool(std::size_t initial_capacity = 64) : m_block_capacity(initial_capacity) { expand(m_block_capacity); }

  ~ObjectPool() {
    // 用户需要在池析构前归还所有对象，这里指负责释放申请到的内存块
    Block *cur = m_block_head;
    while (cur) {
      Block *next = cur->next;
      ::operator delete(cur);
      cur = next;
    }
  }

  template <typename... Args>
  T *create(Args &&...args) {
    if (!m_free_head) {
      expand(m_block_capacity);
    }

    Slot *slot = m_free_head;
    m_free_head = m_free_head->next;

    T *obj = new (&slot->object) T(std::forward<Args>(args)...);
    ++m_live_count;
    return obj;
  }

  void destroy(T *obj) {
    if (!obj)
      return;

    obj->~T();  // obj 先调用自身的析构函数释放持有的资源

    // 将 Slot 归还给 FreeList
    Slot *slot = reinterpret_cast<Slot *>(obj);
    slot->next = m_free_head;
    m_free_head = slot;
    --m_live_count;
  }

  // getters
  std::size_t get_live_count() const { return m_live_count; }
};

#endif  //NETWORK_PROGRAMMING_OBJECT_POOL_H
