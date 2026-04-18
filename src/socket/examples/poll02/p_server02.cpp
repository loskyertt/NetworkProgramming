/**
 * @File    :   src/server.cpp
 * @Time    :   2026/04/16 22:08:16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   poll 示例
 */

#include "logger/logger.h"
#include "socket/poll/poller.h"
#include "socket/server_socket.h"
#include "socket/socket.h"

#include <bits/types/struct_timeval.h>
#include <poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <print>
#include <string>

using namespace sky::socket;
using namespace sky::utility;

int main() {
  // 初始化日志
  Singleton<Logger>::getInstance().open("log/server.log");

  // 创建服务器监听套接字
  ServerSocket server("127.0.0.1", 8080);
  int listen_fd = server.getSockFd();

  // === 等待连接 -> 循环 ===
  // 检测 -> 读缓冲区, 委托内核去处理
  // 数据初始化, 创建自定义的文件描述符集
  Poller poller;
  poller.create(1024);
  poller.setFd(listen_fd);

  // 主事件循环
  while (true) {
    // 调用 poll 等待事件
    int ret = poller.poll();
    if (ret < 0) {
      Log_error("poll error: errno=%d errmsg=%s", errno, strerror(errno));
      break;
    }

    // 检查监听套接字是否有事件
    if (poller.isSetByFd(listen_fd)) {
      // 有新连接
      int conn_fd = server.accept();
      if (conn_fd < 0) {
        continue;
      }

      poller.setFd(conn_fd);
    }

    // 通信, 有客户端发送数据过来
    for (size_t i = 1; i < poller.getMaxConns(); ++i) {
      if (poller.isSetByIndex(i)) {
        Socket client_conn(poller.getFd(i));
        client_conn.setNonBlocking();
        client_conn.setRelease();

        // 有数据可读
        char buffer[1024];
        ssize_t bytes_read = client_conn.recv(buffer, sizeof(buffer));
        if (bytes_read == 0) {
          // 客户端关闭连接
          Log_info("Client disconnected: fd=%d", poller.getFd(i));
          // 将检测的文件描述符从读集合中删除
          client_conn.close();  // 手动关闭
          poller.deleteFd(poller.getFd(i));
        } else if (bytes_read > 0) {
          std::println("Received {} bytes from fd={}, data={}", bytes_read, poller.getFd(i), std::string(buffer));

          // 向客户端发送数据
          std::string new_data = "Echo: " + std::string(buffer);
          client_conn.send(new_data.c_str(), new_data.size());
        } else {
          Log_error("recv error: errno=%d errmsg=%s", errno, strerror(errno));
        }
      }
    }
  }

  return 0;
}
