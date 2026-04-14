/**
 * @File    :   examples/example04.cpp
 * @Time    :   2026/04/12 22:40:11
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   errno 与错误处理规范
 */

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>  // strerror
#include <stdexcept>
#include <string>

// 封装一个错误检查工具：返回值 < 0 就抛异常，附带错误原因
int check(int ret, const char *call) {
  if (ret < 0) {
    throw std::runtime_error(std::string(call) + " failed: " + strerror(errno));
  }
  return ret;
}

// 使用示例：
void demo() {
  int fd = check(open("/nonexistent", O_RDONLY), "open");
  // 如果 open 失败，抛出：open failed: No such file or directory
}

// 重要：errno 必须在系统调用返回后立即读取
// 不能在调用之间做任何其他系统调用（会覆盖 errno）
void wrong_way(int fd) {
  read(fd, nullptr, 0);
  printf("error\n");      // printf 可能修改 errno！
  printf("%d\n", errno);  // 此时 errno 已不可靠
}

void right_way(int fd) {
  ssize_t n = read(fd, nullptr, 0);
  if (n < 0) {
    int err = errno;  // 立即保存
    // 之后再使用 err
    fprintf(stderr, "read failed: %s\n", strerror(err));
  }
}

int main() {
  demo();
  return 0;
}
