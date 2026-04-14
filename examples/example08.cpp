/**
 * @File    :   examples/example08.cpp
 * @Time    :   2026/04/13 15:16:24
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   getaddrinfo——统一地址解析（现代写法）
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>  // getaddrinfo, freeaddrinfo, gai_strerror
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <print>

/**
 * @brief 创建一个已连接的客户端 socket（IPv4/IPv6 均支持）
 *
 * - @param host 主机名或 IP 地址（如 "example.com" 或 "192.168.1.1"）
 * - @param service 服务名或端口号（如 "http" 或 "80"）
 * - @return int 已连接的 socket 描述符，失败返回 -1
 */
int connect_to(const char *host, const char *service) {
  addrinfo hints;  // 告诉 getaddrinfo 我们想要什么类型的地址
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;      // 同时支持 IPv4 和 IPv6
  hints.ai_socktype = SOCK_STREAM;  // TCP 流式套接字
  // hints.ai_flags = AI_PASSIVE;      // ❌ 客户端不用加（服务器 bind 时才用）

  addrinfo *result = nullptr;
  int err = getaddrinfo(host, service, &hints, &result);
  if (err != 0) {
    // getaddrinfo 的错误不用 strerror，用 gai_strerror
    std::println(stderr, "getaddrinfo: {}", gai_strerror(err));
    return -1;
  }

  int sockfd = -1;
  // getaddrinfo 返回链表（可能有多个地址），逐个尝试连接
  for (addrinfo *p = result; p != nullptr; p = p->ai_next) {
    // socket(...)：创建 socket，成功时返回一个非负整数（文件描述符），失败时返回 -1
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd < 0)
      continue;

    // 客户端：host 可能有多个 IP，需要逐个尝试
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0) {
      // 打印实际连接到的地址（IPv4 或 IPv6）
      char ip_str[INET6_ADDRSTRLEN];
      uint16_t port;
      if (p->ai_family == AF_INET) {
        // IPv4 地址结构
        auto *addr4 = (sockaddr_in *)p->ai_addr;
        inet_ntop(AF_INET, &addr4->sin_addr, ip_str, sizeof(ip_str));
        port = ntohs(addr4->sin_port);
      } else {
        // IPv6 地址结构
        auto *addr6 = (sockaddr_in6 *)p->ai_addr;
        inet_ntop(AF_INET6, &addr6->sin6_addr, ip_str, sizeof(ip_str));
        port = ntohs(addr6->sin6_port);
      }
      std::println("connected to [{}]:{}", ip_str, port);
      break;
    }

    // 连接失败，关闭当前 fd，继续循环
    close(sockfd);
    sockfd = -1;
  }

  freeaddrinfo(result);  // 必须释放，否则内存泄漏
  return sockfd;
}

/**
 * @brief 创建一个监听 socket（服务器端）
 *
 * - @param port 监听的端口号（如 "8080"）
 * - @param backlog 最大等待连接队列长度
 * - @return int 监听 socket 描述符，失败返回 -1
 */
int create_listener(const char *port, int backlog = 128) {
  addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;  // 用 IPv6 socket（可同时接受 IPv4 连接）
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;  // 监听模式：地址填 0.0.0.0 或 ::

  addrinfo *result = nullptr;
  if (getaddrinfo(nullptr, port, &hints, &result) != 0)
    return -1;

  // 服务器：绑定 0.0.0.0:8080 或 [::]:8080，一个 socket 就够了
  int fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (fd < 0) {
    freeaddrinfo(result);
    return -1;
  }

  // SO_REUSEADDR：允许服务器重启后立即绑定同一端口（第10课详解）
  int opt = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  bind(fd, result->ai_addr, result->ai_addrlen);
  listen(fd, backlog);
  freeaddrinfo(result);
  return fd;
}

int main() {
  // 用域名也可以，getaddrinfo 会做 DNS 解析
  int fd = connect_to("example.com", "80");
  if (fd >= 0) {
    std::println("Connected successfully");
    close(fd);
  }

  // 服务器：监听 8080
  int lfd = create_listener("8080");
  if (lfd >= 0) {
    std::println("Listening on port 8080");
    close(lfd);
  }
  return 0;
}
