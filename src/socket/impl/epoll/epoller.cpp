/**
 * @File    :   src/socket/impl/epoll/epoller.cpp
 * @Time    :   2026/04/19 10:08:54
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "socket/epoll/epoller.h"
#include "logger/logger.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <chrono>
#include <cstring>

using namespace sky::socket;
using namespace sky::utility;

EPoller::EPoller() : m_epoll_fd(-1) {}

EPoller::~EPoller() {
  if (m_epoll_fd >= 0) {
    ::close(m_epoll_fd);
    m_epoll_fd = -1;
  }
}

bool EPoller::create(size_t max_events) {
  m_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  if (m_epoll_fd < 0) {
    Log_error("epoll_create1 error: errno=%d errmsg=%s", errno, strerror(errno));
    return false;
  }
  m_events.resize(max_events);
  return true;
}

bool EPoller::setFd(int fd, uint32_t event) {
  struct epoll_event ev;  // 栈上局部变量，每次调用独立（可以避免多线程下的竞态条件）
  ev.data.fd = fd;
  ev.events = event;
  if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
    // 对于这种简单项目，可以就直接调用 epoll_ctl，让内核处理 EEXIST（进行重复添加检测）
    // 对于稍复杂项目，可以用哈希表来做状态管理，好处是可以避免触发内核错误路径
    if (errno == EEXIST) {
      Log_warn("fd %d already added to epoll, then modify it...", fd);
      return modFd(fd, event);
    }
    Log_error("epoll_ctl error: errno=%d errmsg=%s", errno, strerror(errno));
    return false;
  }
  return true;
}

bool EPoller::deleteFd(int fd) {
  if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
    Log_error("epoll_ctl error: errno=%d errmsg=%s", errno, strerror(errno));
    return false;
  }
  return true;
}

int EPoller::epoll(int milliseconds) {
  int max_events = static_cast<int>(m_events.size());

  // 永久阻塞，需要处理 EINTR
  if (milliseconds < 0) {
    while (true) {
      int n = ::epoll_wait(m_epoll_fd, m_events.data(), max_events, -1);
      if (n < 0 && errno == EINTR)
        continue;
      return n;
    }
  }

  // 有超时，需要修正剩余时间
  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
  int timeout = milliseconds;

  while (true) {
    int n = ::epoll_wait(m_epoll_fd, m_events.data(), max_events, timeout);
    if (n < 0 && errno == EINTR) {
      auto remaining =
          std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());
      if (remaining <= std::chrono::milliseconds(0))
        return 0;
      timeout = static_cast<int>(remaining.count());
      continue;
    }
    return n;
  }
}

int EPoller::getFd(int idx) {
  if (idx < 0 || idx >= static_cast<int>(m_events.size())) {
    throw std::out_of_range("EPoller::getFd: index out of range");
  }
  return m_events[static_cast<size_t>(idx)].data.fd;
}

uint32_t EPoller::getEvents(int idx) {
  if (idx < 0 || idx >= static_cast<int>(m_events.size())) {
    throw std::out_of_range("EPoller::getEvents: index out of range");
  }
  return m_events[static_cast<size_t>(idx)].events;
}

bool EPoller::modFd(int fd, uint32_t event) {
  struct epoll_event ev;
  ev.data.fd = fd;
  ev.events = event;
  if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0) {
    Log_error("epoll_ctl error: errno=%d errmsg=%s", errno, strerror(errno));
    return false;
  }
  return true;
}
