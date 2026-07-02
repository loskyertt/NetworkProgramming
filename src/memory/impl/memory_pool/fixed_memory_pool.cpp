//
// Created by sky on 2026/6/25.
//

#include "memory_pool/fixed_memory_pool.h"

#include <algorithm>

constexpr std::size_t FixedMemoryPool::align_upward(std::size_t size, std::size_t alignment) noexcept {
  return (size + alignment - 1) & ~(alignment - 1);
}

void FixedMemoryPool::expand(std::size_t count) {
  // 大内存块的大小 = Block 头的大小 + 所有 Chunk 的大小
  std::size_t block_size = sizeof(Block) + m_chunk_size * count;

  // 向 OS 申请原始内存（不构造对象）
  Block *block = static_cast<Block *>(::operator new(block_size));

  // 头插法插入 Block 链表
  block->next = m_block_head;
  m_block_head = block;

  // 定位 Chunk 区域起始地址
  char *base = reinterpret_cast<char *>(block) + sizeof(Block);

  // 切分出 Chunk 并用头插法挂入 FreeList
  for (std::size_t i = 0; i < count; ++i) {
    Chunk *c = reinterpret_cast<Chunk *>(base + i * m_chunk_size);
    c->next = m_free_head;
    m_free_head = c;
  }
}

FixedMemoryPool::FixedMemoryPool(std::size_t chunk_size, std::size_t chunk_count)
    : m_chunk_size(align_upward(std::max(chunk_size, sizeof(Chunk)), alignof(std::max_align_t))),
      m_chunk_count(chunk_count) {
  expand(m_chunk_count);
}

FixedMemoryPool::~FixedMemoryPool() {
  Block *cur = m_block_head;
  while (cur) {
    Block *next = cur->next;
    ::operator delete(cur);
    cur = next;
  }
}

void *FixedMemoryPool::allocate() {
  if (!m_free_head) {
    expand(m_chunk_count);  // 按用户初始设定的 chunk_count 的数量进行扩容
  }

  Chunk *c = m_free_head;
  m_free_head = m_free_head->next;

  return c;
}

void FixedMemoryPool::deallocate(void *ptr_chunk) {
  if (!ptr_chunk)
    return;

  Chunk *c = static_cast<Chunk *>(ptr_chunk);
  c->next = m_free_head;
  m_free_head = c;
}