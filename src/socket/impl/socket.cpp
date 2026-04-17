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
  /**
   * - __domain: 协议族，如 AF_INET（IPv4）、AF_INET6（IPv6）
   * - __type: 套接字类型，如 SOCK_STREAM（流式，TCP）、SOCK_DGRAM（数据报，UDP）
   * - __protocol: 具体协议，通常为 0，表示由系统自动选择
   * - 作用: 在内核中分配资源并创建一个可用于通信的套接字文件描述符
   */
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
  /**
   * @brief 字段说明
   * - sin_family：地址族，必须设为 AF_INET
   * - sin_port：网络字节序的 16 位端口号（需用 htons() 转换）
   * - sin_addr：32 位 IPv4 地址，类型为 struct in_addr
   * - sin_zero：填充字节，确保与通用结构体 struct sockaddr 大小一致（通常为 8 字节）
   * - ⚠️ 注意：虽然 sin_zero 存在，但在现代编程中通常通过 memset() 清零整个结构体来初始化，而不是手动填充。
   */
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (ip.empty()) {
    addr.sin_addr.s_addr = INADDR_ANY;  // 相当于绑定到 0.0.0.0
  } else {
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
  }

  /**
   * @brief
   * - __fd: 监听套接字的文件描述符，一般是 listen_fd（由 socket() 创建）
   * - __addr: 指向 struct sockaddr 的指针，包含要绑定的地址信息（IP 和端口）
   * - __len: 地址结构体的字节长度
   * - 作用: 修改套接字的状态，将其绑定到指定的本地地址
   * - 线程安全: 是（但套接字本身不是可重入资源）
   */
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
  /**
   * @brief
   * - __fd: 已创建并绑定的套接字文件描述符，即 listen_fd（通常由 socket() 创建，bind() 绑定）
   * - __n: 最大待处理连接数，即连接请求队列（全连接队列）的长度
   * - 作用: 将套接字从“主动”状态转为“被动”状态，使其可用于接受连接
   * - 注意: 该函数仅设置监听状态，不会阻塞或处理任何实际连接
   */
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

  /**
   * @brief
   * - __fd: 客户端已通过 socket() 创建的套接字文件描述符，即 client_fd。
   * - __addr: 指向 struct sockaddr 类型的指针，包含目标服务器的协议族、IP 地址和端口号。
   * - __len: 地址结构体的实际长度（字节）。
   * - 作用: 改变套接字状态为“已连接”，对面向连接的协议（如 TCP）会触发三次握手。
   * - 注意: 该函数是取消点（cancellation point），因此未标记 __THROW，在多线程环境下需注意信号中断问题。
   */
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
  /**
   * - __fd: 监听套接字的文件描述符，一般是 listen_fd（由 socket() 创建，并通过 listen() 开始监听）。
   * - __addr: 指向 struct sockaddr 的指针，用于接收客户端的地址信息；可为 nullptr。
   * - __addr_len: 指向 socklen_t 的指针，表示地址缓冲区的大小；若 __addr 为 nullptr，则此值也应为 nullptr。
   * - 作用: 阻塞当前线程，直到有一个连接到达（除非套接字设置为非阻塞模式）。
   */
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
