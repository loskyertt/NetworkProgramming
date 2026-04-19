/**
 * @File    :   src/socket/include/socket/epoll/epoller.h
 * @Time    :   2026/04/19 10:09:06
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   epoll 的封装
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <sys/epoll.h>

namespace sky {
namespace socket {

class EPoller {
 private:
  int m_epoll_fd;                            // epoll 专用文件描述符
  std::vector<struct epoll_event> m_events;  // 就绪事件数组

 public:
  EPoller();
  ~EPoller();

  /**
   * @brief 创建 epoll 实例
   * @param max_events 就绪事件数组的最大长度
   * @return true: 创建成功, false: 创建失败
   */
  bool create(size_t max_events);

  /**
   * @brief 将 fd 加入监控集合。如果 fd 已经在监控集合中，则修改其事件
   * @param fd 要监控的文件描述符
   * @param event 事件类型
   * @return true: 添加成功, false: 添加失败
   */
  bool setFd(int fd, uint32_t event);

  /**
   * @brief 从监控集合中删除 fd
   *
   * @param fd 要删除的文件描述符
   * @return true: 删除成功, false: 删除失败
   */
  bool deleteFd(int fd);

  /**
   * @brief epoll_wait 操作
   *
   * - @param milliseconds 超时时间（毫秒），默认为 -1，表示阻塞等待
   * - @return 0: 超时, >0: 就绪的 fd 数量, -1: 错误
   */
  int epoll(int milliseconds = -1);

  /**
   * @brief 获取 fd
   *
   * @param index 事件数组索引
   * @return int 已就绪的 fd
   */
  int getFd(int idx);

  /**
   * @brief 获取事件
   *
   * @param idx 事件数组索引
   * @return uint32_t 已就绪 fd 对应的事件
   */
  uint32_t getEvents(int idx);

 private:
  /**
   * @brief 修改 fd 的事件
   *
   * @param fd 要修改的文件描述符
   * @param event 事件类型
   * @return true: 修改成功, false: 修改失败
   */
  bool modFd(int fd, uint32_t event);
};
}  // namespace socket
}  // namespace sky
