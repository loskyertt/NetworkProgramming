/**
 * @File    :   src/socket/include/socket/selector.h
 * @Time    :   2026/04/17 12:27:55
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#pragma once

#include <bits/types/struct_timeval.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>

namespace sky {
namespace socket {

class Selector {
 private:
  fd_set m_fds;       // 主文件描述符集合
  fd_set m_read_fds;  // 临时文件描述符集合，用于 select
  int m_max_fd;       // 所有需要监听的文件描述符中的最大

 public:
  Selector();
  ~Selector();

  /* 将 fd 加入监控集合 */
  void setFd(int fd);

  /* 从监控集合中删除 fd */
  void deleteFd(int fd);

  /**
   * @brief select 操作：监控 fd，返回就绪的 fd 数量（目前仅实现了监控 read_fds）
   *
   * - @param milliseconds 超时时间（毫秒），-1 表示阻塞等待
   * - @return 0: 超时, >0: 就绪的 fd 数量, -1: 错误
   */
  int select(int milliseconds = -1);

  /* 检查 fd 是否就绪 */
  bool isSet(int fd) { return FD_ISSET(fd, &m_read_fds); }

  // getters and setters
 public:
  int getMaxFd() const { return m_max_fd; }
};

}  // namespace socket
}  // namespace sky
