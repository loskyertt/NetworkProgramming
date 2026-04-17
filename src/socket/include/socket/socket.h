/**
 * @File    :   socket/socket.h
 * @Time    :   2026/04/14 15:42:35
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   对 socket 的封装
 */

#pragma once

#include <sys/types.h>
#include <cstddef>
#include <cstdint>
#include <string>

namespace sky {
namespace socket {
class Socket {
 protected:
  std::string m_ip;  // 监听/连接的 IP 地址
  uint16_t m_port;   // 监听/连接的端口号
  int m_sockfd;      // 套接字文件描述符

  bool m_is_release = false;  // 是否释放 m_sockfd 的所有权

 public:
  /* 创建 socket_fd */
  Socket();
  /* 使用已有的 socket_fd */
  Socket(int sockfd);
  virtual ~Socket();

  /* 绑定地址 */
  bool bind(const std::string &ip, uint16_t port);

  /* 监听连接 */
  bool listen(int backlog = 128);

  /* 连接服务器 */
  bool connect(const std::string &ip, uint16_t port);

  /* 接受客户端连接 */
  int accept();

  /**
   * @brief 向客户端/服务端发送数据
   *
   * - @param buf 指向待发送数据缓冲区的指针
   * - @param len 要发送的数据字节数
   * - @return ssize_t 成功时返回实际发送的字节数（可能小于请求发送的字节数），失败时返回 -1 并设置 errno
   */
  ssize_t send(const void *buf, size_t len);

  /**
   * @brief 从客户端/服务端接收数据
   *
   * - @param buf 指向接收缓冲区的指针，用于存放接收到的数据
   * - @param len 缓冲区最大容量，即最多接收的字节数
   * - @return ssize_t 成功时返回接收到的字节数（可能小于请求的 len），连接关闭时返回 0，出错时返回 -1
   */
  ssize_t recv(void *buf, size_t len);

  /* 关闭 socket */
  void close();

  /* 设置为非阻塞 I/O */
  bool setNonBlocking();

  /* 设置发送缓冲区大小 */
  bool setSendBufferSize(size_t size);

  /* 设置接收缓冲区大小 */
  bool setReceiveBufferSize(size_t size);

  /**
   * @brief 设置 linger
   *
   * - @param active 是否启用 linger
   * - @param seconds linger 时间（秒），当设置为 0 时，会关闭 TIME_WAIT 状态
   * - @return true 设置成功
   * - @return false 设置失败
   */
  bool setLinger(bool active, int seconds);

  /* 设置 keepalive */
  bool setKeepAlive();

  /* 设置 reuse address */
  bool setReuseAddress();

  // getters and setters
 public:
  int getSocketFd() const { return m_sockfd; }

  /* 设置释放 m_sockfd 的所有权 */
  void setRelease() { m_is_release = true; }
};
}  // namespace socket
}  // namespace sky
