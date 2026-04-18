/**
 * @File    :   src/client.cpp
 * @Time    :   2026/04/14 20:29:25
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include <unistd.h>
#include "logger/logger.h"
#include "socket/client_socket.h"

#include <iostream>
#include <string>
#include <print>

using namespace sky::socket;
using namespace sky::utility;

int main() {
  Singleton<Logger>::getInstance().open("log/client.log");

  ClientSocket client("127.0.0.1", 8080);

  // 通信
  while (true) {
    std::string data;
    std::getline(std::cin, data);

    // 发送数据
    client.send(data.c_str(), data.size());

    // 接收数据
    char buf[1024] = {0};
    client.recv(buf, sizeof(buf));
    std::println("recv: data={}", std::string(buf));
  }
}
