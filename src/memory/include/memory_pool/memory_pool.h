//
// Created by sky on 2026/6/24.
//

#ifndef GEOMETRY_APP_MEMORY_POOL_H
#define GEOMETRY_APP_MEMORY_POOL_H

#include <cstddef>
#include <vector>
#include <mutex>

// 页 [大块内存] -> 切分（allocate） -> [小块1] [小块2] [小块3] [小块4] [小块5] ... [小块n]
//                                  deallocate（归还内存）
// 我们这里获取每一小块的内存都是通过指针去拿的，因此要找到指针的地址，为了寻址方便，要保证地址块是以指针大小（即地址位数）进行对齐的

/**
 * 将字节大小向上对齐到指定对齐数的整数倍。
 *
 * 例如：align_upward(13, 8) == 16，align_upward(8, 8) == 8
 *
 * @param size      需要对齐的原始字节大小
 * @param alignment 对齐基数，必须是 2 的幂次方（如 1、2、4、8、16...）
 * @return          大于等于 size、且是 alignment 整数倍的最小值
 */
static inline std::size_t align_upward(std::size_t size, std::size_t alignment) {
  return (size + alignment - 1) & ~(alignment - 1);  // 等价于 ⌊size + alignment - 1⌋ / alignment
}

// 固定大小的内存池（线程安全版）
class FixedSizePool {
 private:
  std::size_t m_block_size;       // 每一小块的内存大小（已对齐调整）
  std::size_t m_blocks_per_page;  // 每一页的小块内存的数量
  std::size_t m_max_pages;        // 最大允许的页数，0 表示无限制
  std::vector<void *> m_pages;    // 所有已经分配的页，析构的时候统一释放

  // 存放每一小块空闲块内存
  struct Node {
    // 指向下一个空闲块的指针，因此设置 m_block_size 大小时，一定是要大于一个指针的大小（一般是 4B 或者 8B），否则一个空闲内存块连指向下一个空闲内存的地址都存放不了
    Node *next;
  };

  Node *m_free_list = nullptr;  // 空闲小块的单向链表头（采用头插法）
  mutable std::mutex m_mutex;   // 保护所有成员操作的互斥锁

  /**
   * 调整 block_size 以确保：
   *   1) 至少能存下一个指针（侵入式 free list 需要）
   *   2) 按指针大小对齐
   */
  static std::size_t adjust_block_size(std::size_t block_size) {
    constexpr std::size_t min_block_size = sizeof(void *);
    return align_upward(block_size < min_block_size ? min_block_size : block_size, alignof(void *));
  }

 public:
  /**
   * @param block_size       每小块期望的内存大小（会被上调对齐）
   * @param blocks_per_page  每页切分多少个小块
   * @param max_pages        最大页数限制（0 = 无限制，默认 1024）
   */
  explicit FixedSizePool(std::size_t block_size,
                         std::size_t blocks_per_page = 1024,
                         std::size_t max_pages = 0);
  ~FixedSizePool();

  FixedSizePool(const FixedSizePool &) = delete;
  FixedSizePool &operator=(const FixedSizePool &) = delete;

  /**
   * 分配一小块内存
   */
  void *allocate();

  /**
   * 将小块内存归还
   *
   * @param ptr
   */
  void deallocate(void* ptr);

  /**
   * 预分配若干页，减少首次分配时的延迟。
   * @param num_pages  预分配的页数
   */
  void reserve(std::size_t num_pages);

 public:
  std::size_t get_block_size() const { return m_block_size; }
  std::size_t get_blocks_per_page() const { return m_blocks_per_page; }

 private:
  /**
   * 每次向系统申请一页的内存，并把这一页的内存切为很多小块，并把这些小块挂到空闲链表中
   * 调用前必须持有 m_mutex
   */
  void expand_blocks_locked();
};

#endif  //GEOMETRY_APP_MEMORY_POOL_H
