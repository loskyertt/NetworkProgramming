/**
 * @File    :   src/socket/impl/client_socket.cpp
 * @Time    :   2026/04/16 00:01:19
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "socket/client_socket.h"

using namespace sky::socket;

ClientSocket::ClientSocket(const std::string &ip, uint16_t port) : Socket() {
  // 1. 客户端连接 Socket（实例化父类对象后，就生成了一个 sockfd）
  connect(ip, port);
}
