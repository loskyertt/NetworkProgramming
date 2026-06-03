/**
 * @File    :   src/socket/include/socket/client_socket.h
 * @Time    :   2026/04/15 21:56:37
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#pragma once

#include "socket/socket.h"

namespace sky {
namespace socket {
/* 创建客户端套接字：创建 client_fd -> 连接 */
class ClientSocket : public Socket {
 public:
  ClientSocket() = delete;
  ClientSocket(const std::string &ip, uint16_t port);
  ~ClientSocket() override = default;
};
}  // namespace socket
}  // namespace sky
