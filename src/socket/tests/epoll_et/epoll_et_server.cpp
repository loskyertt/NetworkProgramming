/**
 * @File    :   src/socket/tests/epoll_et/epoll_et_server.cpp
 * @Time    :   2026/04/16 22:08:16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   epoll ET（边沿触发）模式示例：使用封装好的 EPoller
 */

#include "logger/logger.h"
#include "socket/epoll/epoller.h"
#include "socket/server_socket.h"

#include <sys/epoll.h>
#include <cstring>
#include <print>

using namespace sky::utility;
using namespace sky::socket;

int main() {
  // 初始化日志
  Singleton<Logger>::instance().open("log/server.log");

  // 创建服务器监听套接字
  ServerSocket server("127.0.0.1", 8080);
  server.set_non_blocking();
  int listen_fd = server.get_sock_fd();

  EPoller epoller;
  epoller.create(1024);
  if (!epoller.set_fd(listen_fd, EPOLLIN)) {
    return 1;
  }

  // 主事件循环
  while (true) {
    int ready_counts = epoller.wait();
    if (ready_counts < 0) {
      LOG_ERROR("epoll_wait error: errno=%d errmsg=%s", errno, strerror(errno));
      return 1;
    } else if (ready_counts == 0) {
      LOG_DEBUG("epoll_wait timeout...");
      continue;
    }
    LOG_DEBUG("epoll_wait ok: ready_count=%d", ready_counts);

    // 处理就绪事件
    for (int i = 0; i < ready_counts; ++i) {
      // events[i].data.fd 就是就绪的文件描述符；events[i].events 就是就绪的事件类型
      int current_fd = epoller.get_fd(i);
      // 检查否有新连接
      if (current_fd == listen_fd) {
        // 建立新的连接
        int conn_fd = server.accept();
        if (conn_fd < 0) {
          continue;
        }
        LOG_DEBUG("new connection: fd=%d", conn_fd);

        // 新得到的件描述符添加到 epoll 模型中，下轮循环的时候就可以被检测了
        if (!epoller.set_fd(conn_fd, EPOLLIN | EPOLLET)) {
          continue;
        }
      } else {
        Socket client_conn(current_fd);
        client_conn.set_non_blocking();  // 设置为非阻塞
        client_conn.set_release();      // 释放所有权，析构函数不再 close(fd)

        // 处理读事件
        if (epoller.get_events(i) & EPOLLIN) {
          char buf[5] = {0};  // 设置一个小缓冲区，后面循环读数据
          std::string all_data;
          while (true) {
            ssize_t bytes_read = client_conn.recv(buf, sizeof(buf));

            if (bytes_read == 0) {
              // 客户端关闭连接
              LOG_INFO("Client disconnected: fd=%d", current_fd);
              epoller.delete_fd(current_fd);
              client_conn.close();  // 手动关闭
              break;
            } else if (bytes_read > 0) {
              std::println("Received {} bytes from fd={}, data={}",
                  bytes_read,
                  current_fd,
                  std::string(buf, static_cast<size_t>(bytes_read)));
              all_data += std::string(buf, static_cast<size_t>(bytes_read));
            } else {
              // 数据读完了
              if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 向客户端发送数据
                std::string new_data = "Echo " + all_data;
                client_conn.send(new_data.c_str(), new_data.size());
                break;  // 非阻塞模式正常情况
              }
              LOG_ERROR("recv error: errno=%d errmsg=%s", errno, strerror(errno));
              epoller.delete_fd(current_fd);
              client_conn.close();  // 手动关闭
              break;
            }
          }
        }
      }
    }
  }

  return 0;
}
