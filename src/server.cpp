/**
 * @File    :   src/server.cpp
 * @Time    :   2026/04/16 22:08:16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   poll 示例
 */

#include "logger/logger.h"
#include "socket/server_socket.h"
#include "socket/socket.h"

#include <bits/types/struct_timeval.h>
#include <poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
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
  int listen_fd = server.getSocketFd();

  // === 等待连接 -> 循环 ===
  // 检测 -> 读缓冲区, 委托内核去处理
  // 数据初始化, 创建自定义的文件描述符集
  struct pollfd fds[1024];
  for (int i = 0; i < 1024; ++i) {
    fds[i].fd = -1;
    fds[i].events = POLLIN;
  }

  fds[0].fd = listen_fd;
  int max_fd = 0;  // 在结构体数组中的目前最大下标是 0

  // 主事件循环
  while (true) {
    // 调用 poll 等待事件
    int ret = poll(fds, static_cast<nfds_t>(max_fd) + 1, -1);  // -1 表示永久阻塞，直到有事件发生
    if (ret < 0) {
      Log_error("poll error: errno=%d errmsg=%s", errno, strerror(errno));
      break;
    }

    // 检查监听套接字是否有事件
    if (fds[0].revents & POLLIN) {
      // 有新连接
      int conn_fd = server.accept();

      // 找到空闲位置
      for (int i = 0; i <= 1024; ++i) {
        if (fds[i].fd == -1) {
          fds[i].fd = conn_fd;
          break;
        }
      }
      max_fd = std::max(max_fd, conn_fd);
    }

    // 通信, 有客户端发送数据过来
    for (int i = 1; i <= max_fd; ++i) {
      if (fds[i].fd != -1 && (fds[i].revents & POLLIN)) {
        Socket client_conn(fds[i].fd);
        client_conn.setRelease();

        // 有数据可读
        char buffer[1024];
        ssize_t bytes_read = client_conn.recv(buffer, sizeof(buffer));
        if (bytes_read == 0) {
          // 客户端关闭连接
          Log_info("Client disconnected: fd=%d", fds[i].fd);
          // 将检测的文件描述符从读集合中删除
          client_conn.close();  // 手动关闭
          fds[i].fd = -1;
        } else if (bytes_read > 0) {
          std::println("Received {} bytes from fd={}, data={}", bytes_read, fds[i].fd, std::string(buffer));

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
