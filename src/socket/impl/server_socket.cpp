/**
 * @File    :   src/socket/impl/server_socket.cpp
 * @Time    :   2026/04/15 22:03:31
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "socket/server_socket.h"
#include "socket/socket.h"

#include <print>

using namespace sky::socket;

ServerSocket::ServerSocket(const std::string &ip, uint16_t port) : Socket() {
  // 1. 服务端监听 Socket（实例化父类对象后，就生成了一个 sockfd）
  std::println("ServerSocket constructor: listen_fd={}", m_sockfd);

  // setNonBlocking(); // 设置为非阻塞 I/O 模式
  setReceiveBufferSize(10 * 1024);
  setSendBufferSize(10 * 1024);
  setLinger(true, 0);
  setKeepAlive();
  setReuseAddress();

  // 2. 绑定地址
  bind(ip, port);

  // 3. 监听连接
  if (listen(1024)) {
    std::println("Server is listening ...");
  }
}
