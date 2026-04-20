/**
 * @File    :   examples/example01.cpp
 * @Time    :   2026/04/12 20:46:40
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   RAII 封装文件描述符
 */

#include <unistd.h>
#include <stdexcept>
#include <utility>  // std::exchange, std::swap

class FdGuard {
 private:
  int m_fd;

 public:
  // 1. 构造函数：获取资源
  explicit FdGuard(int fd) : m_fd(fd) {
    if (m_fd < 0)
      throw std::runtime_error("Invalid fd");
  }

  // 2. 析构函数：释放资源
  // 必须标记为 noexcept，防止析构时抛出异常导致程序终止
  ~FdGuard() noexcept {
    if (m_fd >= 0) {
      close(m_fd);
    }
  }

  // === 禁止拷贝 ===
  // 拷贝会导致两个对象指向同一个 fd，析构时 double-close
  FdGuard(const FdGuard &) = delete;
  FdGuard &operator=(const FdGuard &) = delete;

  // === 支持移动 ===
  // 移动 = 转移所有权，原对象失去 fd（置为 -1）
  FdGuard(FdGuard &&other) noexcept : m_fd(std::exchange(other.m_fd, -1)) {}  // 拿走控制权，并将原对象置空

  FdGuard &operator=(FdGuard &&other) noexcept {
    if (this != &other) {
      reset();  // 先释放自己持有的 fd
      m_fd = std::exchange(other.m_fd, -1);
    }
    return *this;
  }

  // === 访问器 ===
  int get() const noexcept { return m_fd; }

  bool valid() const noexcept { return m_fd >= 0; }

  explicit operator bool() const noexcept { return valid(); }

  // 释放所有权，返回裸 fd（由调用者负责 close）
  int release() noexcept { return std::exchange(m_fd, -1); }

  // 主动提前关闭
  void reset(int new_fd = -1) noexcept {
    if (m_fd >= 0)
      ::close(m_fd);
    m_fd = new_fd;
  }
};

int main() {}
