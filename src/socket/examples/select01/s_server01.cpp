/**
 * @File    :   src/socket/examples/select01/s_server01.cpp
 * @Time    :   2026/04/16 22:08:16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   select 示例
 */

#include "logger/logger.h"
#include "socket/server_socket.h"
#include "socket/socket.h"

#include <bits/types/struct_timeval.h>
#include <sys/select.h>
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

  // 初始化文件描述符集合
  fd_set fds;               // 主文件描述符集合
  FD_ZERO(&fds);            // 清空集合
  FD_SET(listen_fd, &fds);  // 添加监听套接字
  fd_set read_fds;          // 临时文件描述符集合，用于 select

  int max_fd = listen_fd;  // 所有需要监听的文件描述符中的最大

  // 主事件循环
  while (true) {
    // ===== 准备 select 参数 =====
    read_fds = fds;    // 复制主集合到临时集合，因为 select 会修改这个集合
    timeval tv{5, 0};  // 5 秒超时

    // ===== 等待 I/O 事件 =====
    int ready_count = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);
    // select 调用前：read_fds 包含所有监听的 fd
    // select 调用后：read_fds 只包含可读的 fd
    // 所以必须从主集合 fds 重新复制

    // ===== 处理 select 结果 =====
    if (ready_count < 0) {
      Log_error("select error: errno=%d errmsg=%s", errno, strerror(errno));
      break;
    } else if (ready_count == 0) {
      Log_debug("select timeout");
      continue;
    }
    Log_debug("select ok: ready_count=%d", ready_count);

    // 接受连接请求, 这个调用不阻塞
    // 如果有新连接，listen_fd 在 read_fs 中会被位置 1
    if (FD_ISSET(listen_fd, &read_fds)) {
      // 处理新连接
      Log_debug("New connection request on listen_fd=%d", listen_fd);

      int conn_fd = server.accept();
      if (conn_fd < 0) {
        continue;
      }
      // 得到了有效的文件描述符
      // 通信的文件描述符添加到主集合
      // 在下一轮 select 检测的时候, 就能得到缓冲区的状态
      FD_SET(conn_fd, &fds);
      // 重置最大的文件描述符
      max_fd = std::max(max_fd, conn_fd);

      Log_info("New client connected: conn_fd=%d, max_fd=%d", conn_fd, max_fd);
    }

    // 处理已连接的客户端
    for (int fd = 0; fd <= max_fd; fd++) {
      // 判断从监听的文件描述符之后到 max_fd 这个范围内的文件描述符是否读缓冲区有数据
      if (fd != listen_fd && FD_ISSET(fd, &read_fds)) {
        // 处理客户端数据
        Log_debug("Client data available on fd=%d", fd);

        Socket client_conn(fd);
        client_conn.setRelease();  // 释放所有权，析构函数不再 close(fd)
        char buf[1024] = {0};
        // 一次只能接收 1024 个字节, 如果客户端一次发送 2000 个字节，
        // 一次是接收不完的, 因此文件描述符对应的读缓冲区中还有数据，
        // 下一轮 select 检测的时候, 内核还会标记这个文件描述符缓冲区有数据 -> 再读一次
        // 循环会一直持续, 直到缓冲区数据被读完位置

        ssize_t bytes_read = client_conn.recv(buf, sizeof(buf));

        if (bytes_read == 0) {
          // 客户端关闭连接
          Log_info("Client disconnected: fd=%d", fd);
          // 将检测的文件描述符从读集合中删除
          FD_CLR(fd, &fds);
          client_conn.close();  // 手动关闭
        } else if (bytes_read > 0) {
          std::println("Received {} bytes from fd={}, data={}", bytes_read, fd, std::string(buf));

          // 向客户端发送数据
          std::string new_data = "Echo: " + std::string(buf);
          client_conn.send(new_data.c_str(), new_data.size());
        } else {
          Log_error("recv error: errno=%d errmsg=%s", errno, strerror(errno));
        }
      }
      // 虽然超出这个 if 的作用域后，client_conn 这个对象会被销毁，但这个对象只是内核连接的包装器
      // 同时由于我们调用了 setRelease()，所以析构函数不会关闭文件描述符，文件描述符在内核中仍然有效
      // 真正的连接是由内核管理，通过文件描述符访问
    }
  }

  return 0;
}
