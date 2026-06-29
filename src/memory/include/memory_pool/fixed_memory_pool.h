//
// Created by sky on 2026/6/24.
//

#ifndef GEOMETRY_APP_MEMORY_POOL_H
#define GEOMETRY_APP_MEMORY_POOL_H

#include <cstddef>
#include <cstdlib>

class FixedMemoryPool {
 private:
  // 块：池中最小分配单元，通常定长
  struct Chunk {
    Chunk *next;
  };

  // 大块/页：向 OS 申请的连续大内存，内部切分为多个 Chunk
  struct Block {
    Block *next;
  };

  std::size_t m_chunk_size;       // 每个 Chunk 的大小（已对齐）
  std::size_t m_chunk_count;      // Chunk 的数量
  Block *m_block_head = nullptr;  // 所有 Block 的链表头
  Chunk *m_free_head = nullptr;   // 直接指向 FreeList 里第一个真实的空闲 Chunk

 private:
  /**
   * 将字节大小（size）向上对齐到指定对齐数（alignment）的整数倍
   *
   * @param size 需要对齐的原始字节大小
   * @param alignment 对齐基数，必须是 2 的幂次方（如 1、2、4、8、16...）
   * @return 大于等于 size、且是 alignment 整数倍的最小值
   */
  static constexpr std::size_t align_upward(std::size_t size, std::size_t alignment) noexcept;

  /**
   * 申请大内存块，从大内存块中切分出 Chunk 并挂入 FreeList
   *
   * @param count Chunk 的数量
   */
  void expand(std::size_t count);

 public:
  FixedMemoryPool(std::size_t chunk_size, std::size_t chunk_count);
  ~FixedMemoryPool();  // 释放大内存块

  /**
   * 用户调用：获得空闲的 Chunk（从 FreeList 中摘下一块）
   *
   * @return 分配给用户的 Chunk 的首地址
   */
  void *allocate();

  /**
   * 用户调用：释放指定的 Chunk（将其挂回到 FreeList 中）
   *
   * @param ptr_chunk 用户要释放的 Chunk 的地址
   */
  void deallocate(void *ptr_chunk);
};

#endif  //GEOMETRY_APP_MEMORY_POOL_H
