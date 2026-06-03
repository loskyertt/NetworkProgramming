/**
 * @File    :   src/socket/impl/poll/poller.cpp
 * @Time    :   2026/04/18 10:22:24
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "socket/poll/poller.h"

#include "logger/logger.h"

#include <cstddef>

using namespace sky::socket;
using namespace sky::utility;

Poller::Poller() {}

Poller::~Poller() {}

void Poller::create(size_t size) {
  m_fds.resize(size);
  // 显示初始化为无状态
  for (auto &elem : m_fds) {
    elem.fd = -1;
    elem.events = 0;
    elem.revents = 0;
  }

  m_max_conns = 0;  // 此时还没有添加 listen_fd
}

bool Poller::isSetByFd(int fd) {
  auto it = m_fd_map.find(fd);
  if (it == m_fd_map.end()) {
    return false;
  }

  return m_fds[it->second].revents & POLLIN;
}

bool Poller::isSetByIndex(size_t index) {
  if (m_fds[index].fd != -1 && m_fds[index].revents & POLLIN) {
    return true;
  }
  return false;
}

void Poller::setFd(int fd) {
  // 如果添加的是 listen_fd
  if (m_max_conns == 0) {
    m_fds[0].fd = fd;
    m_fds[0].events = POLLIN;
    m_fd_map[fd] = 0;
    m_max_conns = 1;
    Log_debug("set listen_fd: %d", fd);
  }
  // 如果添加的是 conn_fd
  else {
    for (size_t i = 0; i < m_fds.size(); ++i) {
      if (m_fds[i].fd == -1) {
        m_fds[i].fd = fd;
        m_fds[i].events = POLLIN;
        m_fd_map[fd] = i;
        m_max_conns = std::max(m_max_conns, i + 1);
        Log_debug("set conn_fd: %d", fd);
        break;
      }
    }
  }
}

void Poller::deleteFd(int fd) {
  auto it = m_fd_map.find(fd);
  if (it != m_fd_map.end()) {
    // 从 m_fds 中移除
    m_fds[it->second].fd = -1;
    m_fds[it->second].events = 0;
    m_fds[it->second].revents = 0;

    // 从映射表中删除
    m_fd_map.erase(it);
  }
}

int Poller::poll(int milliseconds) {
  return ::poll(m_fds.data(), m_fds.size(), milliseconds);
}

int Poller::getFd(size_t index) const {
  return m_fds[index].fd;
}
