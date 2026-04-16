/**
 * @File    :   src/socket/examples/simple/client01.cpp
 * @Time    :   2026/04/14 20:29:25
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "logger/logger.h"
#include "socket/client_socket.h"

#include <string>
#include <print>

using namespace sky::socket;

int main() {
  sky::utility::Logger::getInstance().open("log/client.log");

  ClientSocket client("127.0.0.1", 8080);

  // 3. 发送数据
  std::string data = "Hello, server!";
  client.send(data.c_str(), data.size());

  // 4. 接收数据
  char buf[1024] = {0};
  client.recv(buf, sizeof(buf));
  std::println("recv: data={}", std::string(buf));
}
