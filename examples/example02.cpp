/**
 * @File    :   examples/example02.cpp
 * @Time    :   2026/04/12 20:46:55
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   fork() 与进程创建
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <cstring>

int main() {
  int pipefd[2];
  pipe(pipefd);  // 创建管道：pipefd[0]=读端（In/Read）, pipefd[1]=写端（Out/Write）

  pid_t pid = fork();  // fork 复制的就是当前正在执行 fork() 这行代码的那个进程

  // 当 fork() 返回值小于 0 时，表示创建子进程失败
  if (pid < 0) {
    perror("fork");  // perror 会打印 strerror(errno)
    return 1;
  }

  // 当返回值 == 0 时，表示当前代码正在子进程中执行
  if (pid == 0) {
    // ===== 子进程 =====
    close(pipefd[1]);  // 子进程只读，关闭写端（引用计数-1）

    char buf[64];
    ssize_t n = read(pipefd[0], buf, sizeof(buf));
    buf[n] = '\0';
    printf("子进程收到: %s\n", buf);

    close(pipefd[0]);
    _exit(0);  // 子进程用 _exit，不触发父进程注册的 atexit
  }

  // ===== 父进程 =====
  close(pipefd[0]);  // 父进程只写，关闭读端

  const char *msg = "hello from parent";
  write(pipefd[1], msg, strlen(msg));
  close(pipefd[1]);  // 关闭写端 → 子进程 read 返回 0（EOF）

  // 等待子进程，避免僵尸进程
  int status;
  waitpid(pid, &status, 0);
  printf("子进程退出码: %d\n", WEXITSTATUS(status));
  return 0;
}
