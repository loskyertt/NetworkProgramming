/**
 * @File    :   src/socket/impl/scoket_handler.cpp
 * @Time    :   2026/04/17 13:43:10
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "logger/logger.h"
#include "socket/scoket_handler.h"
#include "socket/server_socket.h"
#include "socket/socket.h"

#include <cstring>
#include <print>

using namespace sky::socket;
using namespace sky::utility;

SocketHandler::SocketHandler() = default;

SocketHandler::~SocketHandler() {
  if (m_server) {
    delete m_server;
    m_server = nullptr;
  }

  // 清理所有连接
  for (auto &pair : m_connections) {
    delete pair.second;
  }
  m_connections.clear();

  Log_info("SocketHandler destructor is completed");
}

void SocketHandler::listen(const std::string &ip, uint16_t port) {
  if (m_server) {
    Log_warn("SocketHandler already listening");
    delete m_server;
    m_server = nullptr;
  }

  m_server = new ServerSocket(ip, port);

  // 将服务器 socket 加入监控
  m_selector.setFd(m_server->getSocketFd());
}

void SocketHandler::attach(Socket *socket) {
  if (!socket) {
    Log_error("Socket is null!");
    return;
  }

  int fd = socket->getSocketFd();
  m_connections[fd] = socket;  // 建立 conn_fd 和 连接套接字对象的映射关系
  m_selector.setFd(fd);

  Log_info("Socket attached: fd=%d", fd);
}

void SocketHandler::detach(Socket *socket) {
  if (!socket) {
    Log_error("Socket is null!");
    return;
  }

  int fd = socket->getSocketFd();
  m_selector.deleteFd(fd);

  Log_info("Socket detached: fd=%d", fd);
}

void SocketHandler::remove(Socket *socket) {
  if (!socket) {
    Log_error("Socket is null!");
    return;
  }

  int fd = socket->getSocketFd();

  // 从监控集合中移除
  m_selector.deleteFd(fd);

  // 从连接映射中删除
  auto it = m_connections.find(fd);
  if (it != m_connections.end()) {
    m_connections.erase(it);
  }

  // 关闭并删除 socket
  delete socket;
}

void SocketHandler::handle(int wait_time) {
  if (!m_server) {
    Log_error("Server not initialized!");
    return;
  }

  while (true) {
    // ===== 等待 I/O 事件 =====
    int ready_count = m_selector.select(wait_time);

    // ===== 处理 select 结果 =====
    if (ready_count < 0) {
      Log_error("select error: errno=%d errmsg=%s", errno, strerror(errno));
      return;
    } else if (ready_count == 0) {
      Log_debug("select timeout");
      continue;
    }
    Log_debug("select ok: ready_count=%d", ready_count);

    // 检查服务器 socket 是否有新连接
    if (m_selector.isSet(m_server->getSocketFd())) {
      handleNewConnection();
    }

    // 检查客户端连接是否有数据
    handleClientConnections();
  }
}

void SocketHandler::handleNewConnection() {
  int conn_fd = m_server->accept();
  if (conn_fd < 0) {
    Log_error("accept error: errno=%d errmsg=%s", errno, strerror(errno));
    return;
  }

  // 创建客户端连接 socket 对象
  Socket *conn_socket = new Socket(conn_fd);
  attach(conn_socket);
}

void SocketHandler::handleClientConnections() {
  // 遍历所有连接，检查是否有数据可读
  for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
    int fd = it->first;
    Socket *conn_socket = it->second;

    if (m_selector.isSet(fd)) {
      // 有数据可读，处理
      char buffer[1024] = {0};
      ssize_t bytes_read = conn_socket->recv(buffer, sizeof(buffer));

      if (bytes_read == 0) {
        // 客户端关闭连接
        Log_info("Client disconnected: fd=%d", fd);
        m_selector.deleteFd(fd);
        delete conn_socket;
        it = m_connections.erase(it);
        continue;  // 跳过 ++it
      } else if (bytes_read > 0) {
        std::println("Received {} bytes from conn_fd={}, data={}", bytes_read, fd, std::string(buffer));

        // 向客户端发送数据
        std::string new_data = "Echo from server: " + std::string(buffer);
        conn_socket->send(new_data.c_str(), new_data.size());
      } else {
        Log_error("recv error: errno=%d errmsg=%s", errno, strerror(errno));
        m_selector.deleteFd(fd);
        delete conn_socket;
        it = m_connections.erase(it);
        continue;  // 跳过 ++it
      }
    }
  }
}
