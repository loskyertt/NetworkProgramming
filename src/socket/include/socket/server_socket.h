/**
 * @File    :   src/socket/include/socket/server_socket.h
 * @Time    :   2026/04/15 21:56:59
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   服务器端 socket 封装
 */

#pragma once

#include "socket/socket.h"

namespace sky {
namespace socket {
class ServerSocket : public Socket {
 public:
  ServerSocket() = delete;
  /* 启动服务端的监听套接字 */
  ServerSocket(const std::string &ip, uint16_t port);
  ~ServerSocket() override = default;
};
}  // namespace socket
}  // namespace sky
