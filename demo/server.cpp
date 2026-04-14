/**
 * @File    :   demo01/server.cpp
 * @Time    :   2026/04/14 13:31:16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   一个简单的服务端
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <print>
#include <string>

int main() {
  // 1. 创建 socket（:: 表示调用全局的 socket 函数）
  int server_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_fd < 0) {
    std::println("create socket error: errno = {}, errmsg = {}", errno, strerror(errno));
    return 1;
  } else {
    std::println("socket create success!");
  }

  // 2. 绑定 socket
  std::string ip = "127.0.0.1";
  uint16_t port = 8080;  // 在 TCP/IP 协议栈中，端口号占用 16 位（2 字节）

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;                     // 设置为 IPV4 的地址簇
  server_addr.sin_addr.s_addr = inet_addr(ip.c_str());  // 地址转换：点分十进制 -> 32位整数
  server_addr.sin_port = htons(port);                   // 端口转换：主机字节序 -> 网络字节序

  if (::bind(server_fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
    std::println("socket bind error: errno = {}, errmsg = {}", errno, strerror(errno));
    return 1;
  } else {
    std::println("socket bind success!");
  }

  // 3. 监听
  if (::listen(server_fd, 1024) < 0) {
    std::println("socket listen error: errno = {}, errmsg = {}", errno, strerror(errno));
    return 1;
  } else {
    std::println("socket listening ...");
  }

  while (true) {
    // 4. 接受客户端连接
    int connfd = ::accept(server_fd, nullptr, nullptr);
    if (connfd < 0) {
      std::println("accept error: errno = {}, errmsg = {}", errno, strerror(errno));
      return 1;
    }
    std::println("accept success!");

    char buf[1024] = {0};

    // 5. 接收客户端的数据
    // 假设一次 recv 能读完所有数据
    ssize_t len = ::recv(connfd, buf, sizeof(buf), 0);
    if (len < 0) {
      std::println("recv error: errno = {}, errmsg = {}", errno, strerror(errno));
      return 1;
    }
    std::println("- recv: connfd = {}, msg = {}, len = {}", connfd, std::string(buf), len);

    // 6. 向客户端发送数据
    ::send(connfd, buf, static_cast<size_t>(len), 0);
  }

  // 7. 关闭 socket
  ::close(server_fd);
  return 0;
}

