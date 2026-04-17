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
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <string>

using namespace sky::socket;
using namespace sky::utility;

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
  if (!m_is_release) {
    close();
  }
  // 如果 m_is_release 为 true，则不关闭 sockfd，由调用者负责释放
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
  Log_debug("Server started listening: %s:%d", m_ip.c_str(), m_port);
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

  Log_debug("accept new connection: fd = %d", connfd);
  return connfd;
}

ssize_t Socket::send(const void *buf, size_t len) {
  /**
  * - __fd: conn_fd（来自 accept()）
  * - __buf: 指向待发送数据缓冲区的指针
  * - __n: 要发送的数据字节数
  * - __flags: 控制发送行为的标志位（如 MSG_NOSIGNAL 避免 SIGPIPE 信号）
  */
  ssize_t bytes_sent = ::send(m_sockfd, buf, len, 0);
  return bytes_sent;
}

ssize_t Socket::recv(void *buf, size_t len) {
  /**
  * - __fd: conn_fd（来自 accept()）
  * - __buf: 指向接收缓冲区的指针，用于存放接收到的数据
  * - __n: 缓冲区最大容量，即最多接收的字节数
  * - __flags: 控制接收行为的标志位（如 MSG_PEEK、MSG_WAITALL 等）
  */
  ssize_t bytes_received = ::recv(m_sockfd, buf, len, 0);
  return bytes_received;
}

void Socket::close() {
  if (m_sockfd > 0) {
    ::close(m_sockfd);
    m_sockfd = -1;
  }
}

bool Socket::setNonBlocking() {
  // 1. 首先使用 F_GETFL 获取（get）当前文件描述符 m_sockfd 的状态标志
  int flags = fcntl(m_sockfd, F_GETFL, 0);
  if (flags < 0) {
    Log_error("fcntl get socket flags error: errno = %d, errmsg = %s", errno, strerror(errno));
    return false;
  }
  // 2. 然后使用 F_SETFL（set）将 O_NONBLOCK 标志加入原有标志中，使该 socket 变为非阻塞模式
  flags |= O_NONBLOCK;
  if (fcntl(m_sockfd, F_SETFL, flags) < 0) {
    Log_error("fcntl set socket flags error: errno = %d, errmsg = %s", errno, strerror(errno));
    return false;
  }
  return true;
}

bool Socket::setSendBufferSize(size_t size) {
  uint32_t buffer_size = static_cast<uint32_t>(size);
  if (setsockopt(m_sockfd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size)) < 0) {
    Log_error("set socket send buffer size error: errno = %d, errmsg = %s", errno, strerror(errno));
    return false;
  }
  return true;
}

bool Socket::setReceiveBufferSize(size_t size) {
  uint32_t buffer_size = static_cast<uint32_t>(size);
  if (setsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size)) < 0) {
    Log_error("set socket receive buffer size error: errno = %d, errmsg = %s", errno, strerror(errno));
    return false;
  }
  return true;
}

bool Socket::setLinger(bool active, int seconds) {
  struct linger ling;
  memset(&ling, 0, sizeof(ling));
  ling.l_onoff = active ? 1 : 0;
  ling.l_linger = seconds;
  if (setsockopt(m_sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) < 0) {
    Log_error("set socket linger error: errno = %d, errmsg = %s", errno, strerror(errno));
    return false;
  }
  return true;
}

bool Socket::setKeepAlive() {
  int flag = 1;
  if (setsockopt(m_sockfd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) < 0) {
    Log_error("set socket keepalive error: errno = %d, errmsg = %s", errno, strerror(errno));
    return false;
  }
  return true;
}

bool Socket::setReuseAddress() {
  int flag = 1;
  if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
    Log_error("set socket reuse address error: errno = %d, errmsg = %s", errno, strerror(errno));
    return false;
  }
  return true;
}
