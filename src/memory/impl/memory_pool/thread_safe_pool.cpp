//
// Created by luozetian on 2026/6/30.
//

#include "memory_pool/thread_safe_pool.h"

ThreadSafePool::ThreadSafePool(std::size_t chunk_size, std::size_t chunk_count) : m_pool(chunk_size, chunk_count) {}

void *ThreadSafePool::allocate() {
  std::lock_guard<std::mutex> lock(m_mtx);
  return m_pool.allocate();
}

void ThreadSafePool::deallocate(void *ptr_chunk) {
  std::lock_guard<std::mutex> lock(m_mtx);
  m_pool.deallocate(ptr_chunk);
}
