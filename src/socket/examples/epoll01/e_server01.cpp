/**
 * @File    :   src/server.cpp
 * @Time    :   2026/04/16 22:08:16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   epoll 示例（LT 模式）
 */

#include "logger/logger.h"
#include "socket/server_socket.h"

#include <sys/epoll.h>
#include <array>
#include <cstring>
#include <print>

using namespace sky::utility;
using namespace sky::socket;

int main() {
  // 初始化日志
  Singleton<Logger>::getInstance().open("log/server.log");

  // 创建服务器监听套接字
  ServerSocket server("127.0.0.1", 8080);
  int listen_fd = server.getSockFd();

  // 现在只有监听的文件描述符
  // 所有的文件描述符对应读写缓冲区状态都是委托内核进行检测的 epoll
  // 创建一个 epoll 模型
  int epoll_fd = epoll_create1(0);
  if (epoll_fd < 0) {
    Log_error("epoll_create1 error: errno=%d errmsg=%s", errno, strerror(errno));
    return 1;
  }

  // 往 epoll 实例中添加需要检测的节点，现在只有 listen_fd
  struct epoll_event event;
  event.events = EPOLLIN;  // 只检测可读事件
  event.data.fd = listen_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) < 0) {
    Log_error("epoll_ctl error: errno=%d errmsg=%s", errno, strerror(errno));
    return 1;
  }

  std::array<epoll_event, 1024> events;

  // 主事件循环
  while (true) {
    int ready_counts = epoll_wait(epoll_fd, events.data(), events.size(), -1);
    // 内核会向 events[0] 到 events[ready_counts-1] 写入有效数据
    // events[ready_counts] 到 events[1023] 保持不变（不会被访问）
    if (ready_counts < 0) {
      Log_error("epoll_wait error: errno=%d errmsg=%s", errno, strerror(errno));
      return 1;
    } else if (ready_counts == 0) {
      Log_debug("epoll_wait timeout...");
      continue;
    }
    Log_debug("epoll_wait ok: ready_count=%d", ready_counts);

    // 处理就绪事件
    for (int i = 0; i < ready_counts; ++i) {
      // events[i].data.fd 就是就绪的文件描述符
      // events[i].events 就是就绪的事件类型
      int current_fd = events[i].data.fd;
      // 检查否有新连接
      if (current_fd == listen_fd) {
        // 建立新的连接
        int conn_fd = server.accept();
        if (conn_fd < 0) {
          continue;
        }
        Log_debug("new connection: fd=%d", conn_fd);

        // 新得到的件描述符添加到 epol1 模型中，下轮循环的时候就可以被检测了
        // Q：这里需要创建一个新的结构体（new_event） 来传递信息吗？还是说可以复用之前创建的 event？
        event.events = EPOLLIN;
        event.data.fd = conn_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event) < 0) {
          Log_error("epoll_ctl error: errno=%d errmsg=%s", errno, strerror(errno));
          continue;
        }
      } else {
        Socket client_conn(current_fd);
        client_conn.setNonBlocking();  // 设置为非阻塞
        client_conn.setRelease();      // 释放所有权，析构函数不再 close(fd)
        char buf[1024] = {0};

        ssize_t bytes_read = client_conn.recv(buf, sizeof(buf));

        if (bytes_read == 0) {
          // 客户端关闭连接
          Log_info("Client disconnected: fd=%d", current_fd);
          epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, nullptr);
          client_conn.close();  // 手动关闭
        } else if (bytes_read > 0) {
          std::println("Received {} bytes from fd={}, data={}", bytes_read, current_fd, std::string(buf, bytes_read));

          // 向客户端发送数据
          std::string new_data = "Echo " + std::string(buf, bytes_read);
          client_conn.send(new_data.c_str(), new_data.size());
        } else {
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;  // 非阻塞模式正常情况
          }
          Log_error("recv error: errno=%d errmsg=%s", errno, strerror(errno));
          epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, nullptr);
          client_conn.close();  // 手动关闭
        }
      }
    }
  }

  return 0;
}
