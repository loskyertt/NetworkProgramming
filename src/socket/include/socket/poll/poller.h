/**
 * @File    :   src/socket/include/socket/poll/poller.h
 * @Time    :   2026/04/18 10:22:10
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   poll 的封装，只检查读事件
 */

#pragma once

#include <map>
#include <vector>
#include <poll.h>

namespace sky {
namespace socket {

class Poller {

 private:
  std::vector<struct pollfd> m_fds;  // fds 数组
  size_t m_max_conns;                // 最大连接数
  std::map<int, size_t> m_fd_map;    // fd 到索引的映射 -> {fd, idx}

 public:
  Poller();
  ~Poller();

  /**
   * @brief 初始化 fds 数组
   *
   * @param size 数组大小
   */
  void create(size_t size);

  /**
   * @brief 检查 fds 中的文件描述符是否就绪
   *
   * @param fd 文件描述符
   * @return true 就绪；false 不就绪
   */
  bool isSetByFd(int fd);

  /**
   * @brief 检查 fds 中的文件描述符是否就绪
   *
   * @param index 索引
   * @return true 就绪；false 不就绪
   */
  bool isSetByIndex(size_t index);

  /**
   * @brief 向 fds 中添加文件描述符
   *
   * @param fd listen_fd 或者 conn_fd
   */
  void setFd(int fd);

  /**
   * @brief 从 fds 中删除文件描述符
   *
   * @param fd listen_fd 或者 conn_fd
   */
  void deleteFd(int fd);

  /**
   * @brief poll 操作
   *
   * @param milliseconds 超时时间：
   * - -1 表示永久阻塞，直到有事件发生；
   * - 0 表示非阻塞，立即返回（轮询模式）；
   * - 正数表示等待指定的毫秒数
   *
   * @return
   * - -1：出错，errno 会被设置（如 EINTR 被信号中断）。
   * - 0：超时时间到，没有任何 fd 就绪。
   * - 正整数：数组中 revents 不为 0 的 pollfd 结构体的数量（即就绪的 fd 数量）。
   */
  int poll(int milliseconds = -1);

  /**
   * @brief 获取 fds 中的文件描述符
   *
   * @param index 索引
   * @return 文件描述符
   */
  int getFd(size_t index) const;

  // getters and setters
 public:
  /**
   * @brief 获取最大连接数
   *
   * @return 最大连接数
   */
  size_t getMaxConns() const { return m_max_conns; }
};

}  // namespace socket
}  // namespace sky
