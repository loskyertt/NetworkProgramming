/**
 * @File    :   src/socket/examples/simple/server01.cpp
 * @Time    :   2026/04/14 21:15:33
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   使用封装好的 socket 来构造一个服务端案例
 */

#include "logger/logger.h"
#include "socket/server_socket.h"

#include <sys/types.h>
#include <print>
#include <string>

using namespace sky::socket;

int main() {
  sky::utility::Logger::getInstance().open("log/server.log");

  ServerSocket server("127.0.0.1", 8080);

  while (true) {
    // 4. 接受连接
    int connfd = server.accept();
    if (connfd < 0) {
      return 1;
    }

    // 连接 Socket
    Socket connection(connfd);

    // 5. 接收客户端的数据
    char buf[1024] = {0};
    ssize_t len = connection.recv(buf, sizeof(buf));
    std::println("recv: connfd={} data={}", connfd, std::string(buf));

    // 6. 向客户端发送数据
    connection.send(buf, static_cast<size_t>(len));
  }  // 每次循环超出 while 作用域范围，会触发 connection 的析构，关闭 connfd

  return 0;
}
