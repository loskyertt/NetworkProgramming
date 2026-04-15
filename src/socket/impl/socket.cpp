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
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <string>

using namespace sky::socket;

Socket::Socket() : m_ip("127.0.0.1"), m_port(0), m_sockfd(-1) {
  // 实例化 Socket 对象时，就创建 sockfd
  m_sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (m_sockfd < 0) {
    Log_error("create socket error: errno = %d, errmsg = %s", errno, strerror(errno));
    return;
  } else {
    Log_debug("create socket success!");
  }
}

Socket::Socket(int sockfd) : m_sockfd(sockfd) {}

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
    Log_error("bind socket error: errno = %d, errmsg = %s", errno, strerror(errno));
    return false;
  }
  m_ip = ip;
  m_port = port;

  Log_debug("bind socket success! ip = %s, port = %d", ip.c_str(), port);
  return true;
}

bool Socket::listen(int backlog) {
  // 监听
  if (::listen(m_sockfd, backlog) < 0) {
    Log_error("listen socket error: errno = %d, errmsg = %s", errno, strerror(errno));
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
    Log_error("connect socket error: errno = %d, errmsg = %s", errno, strerror(errno));
    return false;
  }

  m_ip = ip;
  m_port = port;

  Log_debug("connect socket success! ip = %s, port = %d", ip.c_str(), port);
  return true;
}

int Socket::accept() {
  int connfd = ::accept(m_sockfd, nullptr, nullptr);
  if (connfd < 0) {
    Log_error("accept socket error: errno = %d, errmsg = %s", errno, strerror(errno));
    return -1;
  }

  Log_debug("accept socket success!");
  return connfd;
}

ssize_t Socket::send(const void *buf, size_t len) {
  return ::send(m_sockfd, buf, len, 0);
}

ssize_t Socket::recv(void *buf, size_t len) {
  return ::recv(m_sockfd, buf, len, 0);
}

void Socket::close() {
  if (m_sockfd > 0) {
    ::close(m_sockfd);
    m_sockfd = -1;
  }
}
