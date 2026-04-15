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
class ClientSocket : public Socket {
 public:
  ClientSocket() = delete;
  /* 连接服务器 */
  ClientSocket(const std::string &ip, uint16_t port);
  ~ClientSocket() override = default;
};
}  // namespace socket
}  // namespace sky
