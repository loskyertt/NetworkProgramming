/**
 * @File    :   socket/socket.h
 * @Time    :   2026/04/14 15:42:35
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#pragma once

#include <cstdint>
#include <string>

namespace yazi {
namespace socket {
class Socket {
 protected:
  std::string m_ip;
  uint16_t m_port;
  int m_sockfd;

 public:
  Socket();
  ~Socket();
};
}  // namespace socket
}  // namespace yazi
