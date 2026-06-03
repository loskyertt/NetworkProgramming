/**
 * @File    :   src/socket/impl/selector.cpp
 * @Time    :   2026/04/17 12:27:52
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "socket/select/selector.h"

#include <algorithm>

using namespace sky::socket;

Selector::Selector() : m_max_fd(-1) {
  FD_ZERO(&m_fds);
}

Selector::~Selector() {
  FD_ZERO(&m_fds);
}

void Selector::setFd(int fd) {
  FD_SET(fd, &m_fds);
  m_max_fd = std::max(m_max_fd, fd);  // 更新 max
}

void Selector::deleteFd(int fd) {
  FD_CLR(fd, &m_fds);

  // 只有删除的是当前最大 fd 时才需要重新计算
  if (fd == m_max_fd) {
    m_max_fd = -1;
    for (int i = fd - 1; i >= 0; --i) {
      if (FD_ISSET(i, &m_fds)) {
        m_max_fd = i;
        break;
      }
    }
  }
}

int Selector::select(int milliseconds) {
  m_read_fds = m_fds;
  if (milliseconds >= 0) {
    struct timeval timeout;
    timeout.tv_sec = milliseconds / 1000;
    timeout.tv_usec = (milliseconds % 1000) * 1000;
    return ::select(m_max_fd + 1, &m_read_fds, nullptr, nullptr, &timeout);
  }
  return ::select(m_max_fd + 1, &m_read_fds, nullptr, nullptr, nullptr);
}
