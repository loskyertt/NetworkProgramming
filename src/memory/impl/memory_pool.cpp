//
// Created by sky on 2026/6/25.
//

#include "memory_pool/memory_pool.h"

#include <new>        // for ::operator new, ::operator delete
#include <stdexcept>  // for std::bad_alloc

FixedSizePool::FixedSizePool(std::size_t block_size, std::size_t blocks_per_page, std::size_t max_pages)
    : m_block_size(adjust_block_size(block_size)),
      m_blocks_per_page(blocks_per_page),
      m_max_pages(max_pages) {}

FixedSizePool::~FixedSizePool() {
  for (void *p : m_pages) {
    ::operator delete(p);
  }
}

void *FixedSizePool::allocate() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_free_list) {
    if (m_max_pages > 0 && m_pages.size() >= m_max_pages) {
      return nullptr;  // 达到上限，分配失败
    }
    expand_blocks_locked();
  }

  Node *head = m_free_list;
  m_free_list = head->next;

  return head;
}

void FixedSizePool::deallocate(void *ptr) {
  if (!ptr)
    return;

  std::lock_guard<std::mutex> lock(m_mutex);

  Node *node = static_cast<Node *>(ptr);
  node->next = m_free_list;
  m_free_list = node;
}

void FixedSizePool::reserve(std::size_t num_pages) {
  std::lock_guard<std::mutex> lock(m_mutex);
  for (std::size_t i = 0; i < num_pages; ++i) {
    if (m_max_pages > 0 && m_pages.size() >= m_max_pages)
      break;
    expand_blocks_locked();
  }
}

void FixedSizePool::expand_blocks_locked() {
  // 一整页内存的字节数
  std::size_t page_bytes = m_block_size * m_blocks_per_page;
  char *page = static_cast<char *>(::operator new(page_bytes));
  m_pages.push_back(page);

  // 把一整页的内存分成 m_blocks_per_page 个节点，串成 m_free_list
  for (std::size_t i = 0; i < m_blocks_per_page; ++i) {
    char *addr = page + i * m_block_size;
    Node *node = reinterpret_cast<Node *>(addr);
    node->next = m_free_list;
    m_free_list = node;
  }
}
