/**
 * @File    :   socket/socket.h
 * @Time    :   2026/04/14 15:42:35
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#pragma once

#include <sys/types.h>
#include <cstddef>
#include <cstdint>
#include <string>

namespace yazi {
namespace socket {
class Socket {
 protected:
  std::string m_ip;  // 监听/连接的 IP 地址
  uint16_t m_port;   // 监听/连接的端口号
  int m_sockfd;      // 套接字文件描述符

 public:
  /* 创建 socket_fd */
  Socket();
  /* 使用已有的 socket_fd */
  Socket(int sockfd);
  ~Socket();

  /* 绑定地址 */
  bool bind(const std::string &ip, uint16_t port);

  /* 监听连接 */
  bool listen(int backlog = 128);

  /* 连接服务器 */
  bool connect(const std::string &ip, uint16_t port);

  /* 接受客户端连接 */
  int accept();

  /* 向客户端/服务端发送数据 */
  ssize_t send(const void *buf, size_t len);

  /* 从客户端/服务端接收数据 */
  ssize_t recv(void *buf, size_t len);

  /* 关闭 socket */
  void close();
};
}  // namespace socket
}  // namespace yazi
