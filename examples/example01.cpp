/**
 * @File    :   examples/example01.cpp
 * @Time    :   2026/04/12 20:46:40
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   RAII 封装文件描述符
 */

#include <unistd.h>
#include <stdexcept>
#include <utility>     // std::exchange
#include <fcntl.h>     // 定义了 open(), O_RDONLY 等宏
#include <sys/stat.h>  // 在某些系统中 open 需要这个头文件配合定义权限

class FileDescriptor {
 private:
  int fd_;

 public:
  explicit FileDescriptor(int fd) : fd_(fd) {
    if (fd_ < 0)
      throw std::runtime_error("invalid fd");
  }

  ~FileDescriptor() {
    if (fd_ >= 0)
      ::close(fd_);  // 析构时自动关闭
  }

  // 禁止拷贝（fd 不能共享所有权）
  FileDescriptor(const FileDescriptor &) = delete;
  FileDescriptor &operator=(const FileDescriptor &) = delete;

  // 允许移动（转移所有权）
  FileDescriptor(FileDescriptor &&other) noexcept : fd_(std::exchange(other.fd_, -1)) {}

  FileDescriptor &operator=(FileDescriptor &&other) noexcept {
    if (this != &other) {
      if (fd_ >= 0)
        ::close(fd_);
      fd_ = std::exchange(other.fd_, -1);
    }
    return *this;
  }

  int get() const { return fd_; }
};

// 使用示例：函数返回时自动 close，即使抛出异常也安全
void example() {
  FileDescriptor f(::open("/tmp/test.txt", O_RDONLY));
  // 使用 f.get() 进行读写...
  // 不需要手动 close
}

int main() {
  example();
}
