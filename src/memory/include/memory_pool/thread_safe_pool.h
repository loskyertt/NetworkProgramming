//
// Created by luozetian on 2026/6/30.
//

#ifndef NETWORK_PROGRAMMING_THREAD_SAFE_POOL_H
#define NETWORK_PROGRAMMING_THREAD_SAFE_POOL_H

#include "fixed_memory_pool.h"

#include <mutex>

class ThreadSafePool {
 private:
  FixedMemoryPool m_pool;
  std::mutex m_mtx;  // 这里使用全局锁的方式

 public:
  ThreadSafePool(std::size_t chunk_size, std::size_t chunk_count);

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

#endif  //NETWORK_PROGRAMMING_THREAD_SAFE_POOL_H
