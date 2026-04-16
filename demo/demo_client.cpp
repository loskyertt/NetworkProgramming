/**
 * @File    :   demo/client.cpp
 * @Time    :   2026/04/14 13:31:02
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   一个简单的客户端
 */

#include <print>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>

int main() {
  // 1. 创建 socket
  int client_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_fd < 0) {
    std::println("create socket error: errno = {}, errmsg = {}", errno, strerror(errno));
    return 1;
  } else {
    std::println("socket create success!");
  }

  // 2. 连接服务端（服务端也是这个 IP 和端口）
  std::string ip = "127.0.0.1";
  uint16_t port = 8080;

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
  server_addr.sin_port = htons(port);

  if (::connect(client_fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
    std::println("connect error: errno = {}, errmsg = {}", errno, strerror(errno));
    return 1;
  } else {
    std::println("connect success!");
  }

  // 3. 向服务端发送数据
  std::string message = "Hello, server!";
  if (::send(client_fd, message.c_str(), message.size(), 0) < 0) {
    std::println("send error: errno = {}, errmsg = {}", errno, strerror(errno));
    return 1;
  } else {
    std::println("send success!");
  }

  // 4. 客户端接受服务端的数据
  char buf[1024] = {0};
  ssize_t len = ::recv(client_fd, buf, sizeof(buf), 0);
  if (len < 0) {
    std::println("recv error: errno = {}, errmsg = {}", errno, strerror(errno));
    return 1;
  }
  std::println("- recv: msg = {}, len = {}", std::string(buf), len);

  // 5. 关闭 socket
  ::close(client_fd);
  return 0;
}
