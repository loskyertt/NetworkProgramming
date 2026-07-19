/**
 * @File    :   src/socket/impl/scoket_handler.cpp
 * @Time    :   2026/04/17 13:43:10
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "logger/logger.h"
#include "socket/select/select_handler.h"
#include "socket/server_socket.h"
#include "socket/socket.h"

#include <cstring>
#include <print>

using namespace sky::socket;
using namespace sky::utility;

SelectHandler::SelectHandler() = default;

SelectHandler::~SelectHandler() {
  if (m_server) {
    delete m_server;
    m_server = nullptr;
  }

  // 清理所有连接
  for (auto &pair : m_connections) {
    delete pair.second;
  }
  m_connections.clear();

  LOG_INFO("SelectHandler destructor is completed");
}

void SelectHandler::listen(const std::string &ip, uint16_t port) {
  if (m_server) {
    LOG_WARN("SelectHandler already listening");
    delete m_server;
    m_server = nullptr;
  }

  m_server = new ServerSocket(ip, port);

  // 将服务器 socket 加入监控
  m_selector.set_fd(m_server->get_sock_fd());
}

void SelectHandler::attach(Socket *socket) {
  if (!socket) {
    LOG_ERROR("Socket is null!");
    return;
  }

  int fd = socket->get_sock_fd();
  m_connections[fd] = socket;  // 建立 conn_fd 和 连接套接字对象的映射关系
  m_selector.set_fd(fd);

  LOG_INFO("Socket attached: fd=%d", fd);
}

void SelectHandler::detach(Socket *socket) {
  if (!socket) {
    LOG_ERROR("Socket is null!");
    return;
  }

  int fd = socket->get_sock_fd();
  m_selector.delete_fd(fd);

  LOG_INFO("Socket detached: fd=%d", fd);
}

void SelectHandler::remove(Socket *socket) {
  if (!socket) {
    LOG_ERROR("Socket is null!");
    return;
  }

  int fd = socket->get_sock_fd();

  // 从监控集合中移除
  m_selector.delete_fd(fd);

  // 从连接映射中删除
  auto it = m_connections.find(fd);
  if (it != m_connections.end()) {
    m_connections.erase(it);
  }

  // 关闭并删除 socket
  delete socket;
}

void SelectHandler::handle(int wait_time) {
  if (!m_server) {
    LOG_ERROR("Server not initialized!");
    return;
  }

  while (true) {
    // ===== 等待 I/O 事件 =====
    int ready_count = m_selector.select(wait_time);

    // ===== 处理 select 结果 =====
    if (ready_count < 0) {
      LOG_ERROR("select error: errno=%d errmsg=%s", errno, strerror(errno));
      return;
    } else if (ready_count == 0) {
      LOG_DEBUG("select timeout");
      continue;
    }
    LOG_DEBUG("select ok: ready_count=%d", ready_count);

    // 检查服务器 socket 是否有新连接
    if (m_selector.is_set(m_server->get_sock_fd())) {
      handleNewConnection();
    }

    // 检查客户端连接是否有数据
    handleClientConnections();
  }
}

void SelectHandler::handleNewConnection() {
  int conn_fd = m_server->accept();
  if (conn_fd < 0) {
    LOG_ERROR("accept error: errno=%d errmsg=%s", errno, strerror(errno));
    return;
  }

  // 创建客户端连接 socket 对象
  Socket *conn_socket = new Socket(conn_fd);
  attach(conn_socket);
}

void SelectHandler::handleClientConnections() {
  // 遍历所有连接，检查是否有数据可读
  for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
    int fd = it->first;
    Socket *conn_socket = it->second;

    if (m_selector.is_set(fd)) {
      // 有数据可读，处理
      char buffer[1024] = {0};
      ssize_t bytes_read = conn_socket->recv(buffer, sizeof(buffer));

      if (bytes_read == 0) {
        // 客户端关闭连接
        LOG_INFO("Client disconnected: fd=%d", fd);
        m_selector.delete_fd(fd);
        delete conn_socket;
        it = m_connections.erase(it);
        continue;  // 跳过 ++it
      } else if (bytes_read > 0) {
        std::println("Received {} bytes from conn_fd={}, data={}", bytes_read, fd, std::string(buffer));

        // 向客户端发送数据
        std::string new_data = "Echo " + std::string(buffer);
        conn_socket->send(new_data.c_str(), new_data.size());
      } else {
        LOG_ERROR("recv error: errno=%d errmsg=%s", errno, strerror(errno));
        m_selector.delete_fd(fd);
        delete conn_socket;
        it = m_connections.erase(it);
        continue;  // 跳过 ++it
      }
    }
  }
}
