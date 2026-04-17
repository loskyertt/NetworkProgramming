/**
 * @File    :   src/socket/examples/select01/s_server01.cpp
 * @Time    :   2026/04/16 22:08:16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   多线程 slelect 并发
 */

#include "logger/logger.h"
#include "socket/server_socket.h"
#include "socket/socket.h"

#include <bits/types/struct_timeval.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <mutex>
#include <print>
#include <string>
#include <thread>
#include <unordered_set>

using namespace sky::socket;
using namespace sky::utility;

// 全局变量
int listen_fd = 0;
int max_fd = 0;
std::mutex conn_mutex;
std::unordered_set<int> active_connections;

void acceptConnection(ServerSocket &server) {
  std::println("thread id of acceptConnection: {}", std::this_thread::get_id());

  int conn_fd = server.accept();
  if (conn_fd < 0) {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(conn_mutex);
    active_connections.insert(conn_fd);
    max_fd = std::max(max_fd, conn_fd);
  }

  Log_info("New client connected: conn_fd=%d, max_fd=%d", conn_fd, max_fd);
}

void communication(int fd) {
  std::println("thread id of communication: {}", std::this_thread::get_id());

  Socket client_conn(fd);
  client_conn.setRelease();

  char buf[1024] = {0};
  ssize_t bytes_read = client_conn.recv(buf, sizeof(buf));

  if (bytes_read == 0) {
    Log_info("Client disconnected: fd=%d", fd);

    std::lock_guard<std::mutex> lock(conn_mutex);
    active_connections.erase(fd);

    // 更新 max_fd
    if (fd == max_fd) {
      max_fd = listen_fd;
      for (int active_fd : active_connections) {
        max_fd = std::max(max_fd, active_fd);
      }
    }

    client_conn.close();
  } else if (bytes_read > 0) {
    std::println("Received {} bytes from fd={}, data={}", bytes_read, fd, std::string(buf));

    std::string new_data = "world";
    client_conn.send(new_data.c_str(), new_data.size());
  } else {
    Log_error("recv error: errno=%d errmsg=%s", errno, strerror(errno));

    std::lock_guard<std::mutex> lock(conn_mutex);
    active_connections.erase(fd);

    if (fd == max_fd) {
      max_fd = listen_fd;  // ✅ 可以访问
      for (int active_fd : active_connections) {
        max_fd = std::max(max_fd, active_fd);
      }
    }

    client_conn.close();
  }
}

int main() {
  Singleton<Logger>::getInstance().open("log/server.log");

  ServerSocket server("127.0.0.1", 8080);
  listen_fd = server.getSocketFd();  // 设置全局变量

  fd_set fds;
  fd_set read_fds;
  FD_ZERO(&fds);
  FD_SET(listen_fd, &fds);

  max_fd = listen_fd;
  active_connections.clear();

  while (true) {
    FD_ZERO(&read_fds);
    FD_SET(listen_fd, &read_fds);

    {
      std::lock_guard<std::mutex> lock(conn_mutex);
      for (int fd : active_connections) {
        FD_SET(fd, &read_fds);
      }
    }

    timeval tv{5, 0};
    int ready_count = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);

    if (ready_count < 0) {
      Log_error("select error: errno=%d errmsg=%s", errno, strerror(errno));
      break;
    } else if (ready_count == 0) {
      Log_debug("select timeout");
      continue;
    }
    Log_debug("select ok: ready_count=%d", ready_count);

    if (FD_ISSET(listen_fd, &read_fds)) {
      Log_debug("New connection request on listen_fd=%d", listen_fd);
      std::thread t1(acceptConnection, std::ref(server));
      t1.detach();
    }

    {
      std::unique_lock<std::mutex> lock(conn_mutex);
      auto connections_copy = active_connections;
      lock.unlock();  // 手动解锁

      for (int fd : connections_copy) {
        if (FD_ISSET(fd, &read_fds)) {
          Log_debug("Client data available on fd=%d", fd);

          std::thread t2(communication, fd);
          t2.detach();
        }
      }
      // lock 在作用域结束时自动析构（如果还持有锁的话）
    }
  }

  return 0;
}
