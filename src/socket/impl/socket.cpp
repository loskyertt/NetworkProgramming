/**
 * @File    :   socket/socket.cpp
 * @Time    :   2026/04/14 15:45:19
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "logger/logger.h"
#include "socket/socket.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <string>

using namespace yazi::socket;

Socket::Socket() : m_ip("127.0.0.1"), m_port(0), m_sockfd(-1) {
  m_sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (m_sockfd < 0) {
    Log_error("create socket error: errno = {}, errmsg = {}", errno, strerror(errno));
    return;
  } else {
    Log_debug("create socket success!");
  }
}

Socket::~Socket() {
  close();
}

bool Socket::bind(const std::string &ip, uint16_t port) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (ip.empty()) {
    addr.sin_addr.s_addr = INADDR_ANY;  // 相当于绑定到 0.0.0.0
  } else {
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
  }

  if (::bind(m_sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    Log_error("bind socket error: errno = {}, errmsg = {}", errno, strerror(errno));
    return false;
  }
  m_ip = ip;
  m_port = port;

  Log_debug("bind socket success! ip = {}, port = {}", ip.c_str(), port);
  return true;
}

bool Socket::listen(int backlog) {
  // 监听
  if (::listen(m_sockfd, backlog) < 0) {
    Log_error("listen socket error: errno = {}, errmsg = {}", errno, strerror(errno));
    return false;
  }
  Log_debug("listen socket success!");
  return true;
}

bool Socket::connect(const std::string &ip, uint16_t port) {
  // 连接服务端
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

  if (::connect(m_sockfd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
    Log_error("connect socket error: errno = {}, errmsg = {}", errno, strerror(errno));
    return false;
  }

  m_ip = ip;
  m_port = port;

  Log_debug("connect socket success! ip = {}, port = {}", ip.c_str(), port);
  return true;
}

int Socket::accept() {
  int connfd = ::accept(m_sockfd, nullptr, nullptr);
  if (connfd < 0) {
    Log_error("accept socket error: errno = {}, errmsg = {}", errno, strerror(errno));
    return -1;
  }

  Log_debug("accept socket success!");
  return connfd;
}

int Socket::send(const void *buf, int len) {
  return ::send(m_sockfd, buf, static_cast<size_t>(len), 0);
}

int Socket::recv(void *buf, int len) {
  return ::recv(m_sockfd, buf, static_cast<size_t>(len), 0);
}

void Socket::close() {
  if (m_sockfd > 0) {
    ::close(m_sockfd);
    m_sockfd = -1;
  }
}
