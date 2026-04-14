/**
 * @File    :   examples/example03.cpp
 * @Time    :   2026/04/12 21:16:09
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   waitpid() 与僵尸进程
 */

#include <signal.h>
#include <sys/wait.h>
#include <cerrno>

// SIGCHLD 处理函数：循环回收所有已退出的子进程
void sigchld_handler(int signo) {
  (void)signo;              // 用于消除未使用变量的编译警告
  int saved_errno = errno;  // signal handler 会破坏 errno，需要保存

  int status;
  pid_t pid;
  // WNOHANG：非阻塞，没有子进程退出则立即返回
  // 循环直到回收完所有已退出子进程
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    // 可以在这里记录日志：子进程 pid 退出，退出码 WEXITSTATUS(status)
  }

  errno = saved_errno;  // 还原 errno
}

// 注册方式（在 main 或服务器初始化时）：
void setup_sigchld() {
  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;  // 让被信号中断的系统调用自动重启
  sigaction(SIGCHLD, &sa, nullptr);
}

int main() {
  setup_sigchld();
  // 主循环...
  return 0;
}
