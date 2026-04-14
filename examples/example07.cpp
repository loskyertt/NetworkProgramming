/**
 * @File    :   examples/example07.cpp
 * @Time    :   2026/04/13 13:33:41
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   inet_pton 与 inet_ntop——地址与字符串互转
 */

#include <arpa/inet.h>
#include <print>

void addr_conversion_demo() {
  // ---- 字符串 → 二进制（inet_pton）----

  // IPv4
  struct in_addr ipv4_bin;
  const char *ipv4_str = "192.168.1.100";
  if (inet_pton(AF_INET, ipv4_str, &ipv4_bin) == 1) {
    // ipv4_bin.s_addr 现在是网络字节序的 uint32_t
    std::println("IPv4 binary (hex): 0x{:08X}", ntohl(ipv4_bin.s_addr));
    // 输出: 0xC0A80164 = 192.168.1.100
  }

  // IPv6
  struct in6_addr ipv6_bin;
  const char *ipv6_str = "2001:db8::1";
  inet_pton(AF_INET6, ipv6_str, &ipv6_bin);
  // ipv6_bin.s6_addr 是 16 字节数组，已是网络字节序
  // 输出: 0x2001db80000000000000000000000001 = 2001:db8::1

  // ---- 二进制 → 字符串（inet_ntop）----

  char ipv4_buf[INET_ADDRSTRLEN];   // INET_ADDRSTRLEN = 16，足够放 "255.255.255.255"
  char ipv6_buf[INET6_ADDRSTRLEN];  // INET6_ADDRSTRLEN = 46

  inet_ntop(AF_INET, &ipv4_bin, ipv4_buf, sizeof(ipv4_buf));
  inet_ntop(AF_INET6, &ipv6_bin, ipv6_buf, sizeof(ipv6_buf));

  std::println("IPv4: {}", ipv4_buf);  // 192.168.1.100
  std::println("IPv6: {}", ipv6_buf);  // 2001:db8::1

  // 从 accept() 拿到对端地址后打印（服务器日志常见写法）
  sockaddr_in peer_addr;                   // 用来存放对方（客户端）的地址信息
  socklen_t peer_len = sizeof(peer_addr);  // 告诉操作系统容器的大小
  // int conn_fd = accept(listen_fd, (sockaddr*)&peer_addr, &peer_len);

  char peer_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &peer_addr.sin_addr, peer_ip, sizeof(peer_ip));
  uint16_t peer_port = ntohs(peer_addr.sin_port);  // 网络序 → 主机序
  std::println("client: {}:{}", peer_ip, peer_port);
}

// 常见错误示范（绝对不要这样写）：
void wrong_examples() {
  sockaddr_in addr;
  // 错误1：端口忘记 htons
  // 8080 -> 0x1F90 -> 0x901F (小端序) -> 36895（❌ 实际绑定到端口）
  addr.sin_port = 8080;

  // 错误2：使用已废弃的 inet_addr（不能表示 255.255.255.255，出错返回 -1 但类型是 uint32_t）
  addr.sin_addr.s_addr = inet_addr("192.168.1.1");  // ❌ 不检查错误

  // 错误3：inet_ntoa 返回静态缓冲区，多线程不安全，已废弃
  // char* ip = inet_ntoa(addr.sin_addr);  // ❌
}

int main() {
  addr_conversion_demo();
}
