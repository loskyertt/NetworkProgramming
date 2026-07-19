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

  // 配置项
  set_non_blocking(); // 设置为非阻塞 I/O 模式
  set_receive_buffer_size(10 * 1024);
  set_send_buffer_size(10 * 1024);
  set_reuse_address();
  // set_linger(true, 0);
  // set_keep_alive();

  // 2. 绑定地址
  bind(ip, port);

  // 3. 监听连接
  if (listen(1024)) {
    std::println("Server is listening ...");
  }
}
