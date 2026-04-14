/**
 * @File    :   examples/example05.cpp
 * @Time    :   2026/04/12 23:44:44
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   转换函数：hton* / ntoh*
 */

#include <arpa/inet.h>  // htonl, htons, ntohl, ntohs, inet_pton, inet_ntop
#include <cstdint>
#include <cstdio>
#include <print>

void demo_byte_order() {
  uint16_t port_host = 8080;             // 十六进制为 0x1F90
  uint16_t port_net = htons(port_host);  // host → network (16位)

  std::println("- port 8080 in host order: 0x{:04X}", port_host);    // 在大端机器上输出: 0x1F90
  std::println("- port 8080 in network order: 0x{:04X}", port_net);  // 在小端机器上输出: 0x901F

  uint32_t addr_host = 0xC0A80101;       // 192.168.1.1
  uint32_t addr_net = htonl(addr_host);  // host → network (32位)

  // 从网络读回来后，用 ntoh* 转换回主机序
  uint16_t port_back = ntohs(port_net);
  printf("converted back: %u\n", port_back);  // 8080

  // 检测当前机器字节序（面试常考）
  union {
    uint32_t i;
    uint8_t c[4];
  } u = {0x01020304};

  if (u.c[0] == 0x04) {
    std::println("- This machine is Little-Endian (x86/ARM)");
  } else {
    std::println("- This machine is Big-Endian");
  }
}

int main() {
  demo_byte_order();
}
