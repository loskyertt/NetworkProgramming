/**
 * @File    :   socket/socket.cpp
 * @Time    :   2026/04/14 15:45:19
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "socket/socket.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <print>
#include <string>

using namespace yazi::socket;

Socket::Socket() : m_ip("127.0.0.1"), m_port(0), m_sockfd(-1) {
  // 1. 创建 socket（:: 表示调用全局的 socket 函数）
  m_sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (m_sockfd < 0) {
    std::println("create socket error: errno = {}, errmsg = {}", errno, strerror(errno));
    return;
  } else {
    std::println("socket create success!");
  }
}
