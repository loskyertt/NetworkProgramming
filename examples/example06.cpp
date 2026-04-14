/**
 * @File    :   examples/example06.cpp
 * @Time    :   2026/04/13 10:59:23
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   字符串与二进制地址互转
 */

#include <netinet/in.h>  // sockaddr_in, sockaddr_in6, IPPROTO_TCP
#include <arpa/inet.h>   // htons, htonl, inet_pton, inet_ntop
#include <cstring>       // memset
#include <stdexcept>
#include <print>

// === IPv4 地址结构填写 ===
sockaddr_in make_ipv4_addr(const char *ip_str, uint16_t port) {
  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));  // 1. 清零，包括 sin_zero 填充字节

  addr.sin_family = AF_INET;    // 2. 指定地址族：AF_INET = 2，是用于 IPv4 协议的标准地址族
  addr.sin_port = htons(port);  // 3. 端口转换：主机序 -> 网络序

  // 4. IP 转换：点分十进制 -> 网络序二进制
  // 注意：inet_pton 的 target 是 &addr.sin_addr，不是 &addr
  if (inet_pton(AF_INET, ip_str, &addr.sin_addr) <= 0) {
    // inet_pton 返回 1 成功，0 格式错误，-1 系统错误
    throw std::runtime_error("invalid IPv4 address");
  }

  return addr;
}

// 监听所有接口（服务器常用）
sockaddr_in make_any_addr(uint16_t port) {
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 0.0.0.0
  // 或者 addr.sin_addr.s_addr = INADDR_ANY;  // INADDR_ANY 本身就是 0，大小端无关
  return addr;
}

// 回环地址（本机测试常用）
sockaddr_in make_loopback_addr(uint16_t port) {
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // 127.0.0.1
  return addr;
}

// === IPv6 地址结构填写 ===
sockaddr_in6 make_ipv6_addr(const char *ip6, uint16_t port) {
  sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port);
  if (inet_pton(AF_INET6, ip6, &addr.sin6_addr) <= 0) {
    throw std::runtime_error("invalid IPv6 address");
  }
  return addr;
}

// ==========================================
// 测试代码
// ==========================================

// === 辅助函数：比较 IPv4 地址是否一致 ===
bool check_ipv4_content(const sockaddr_in &addr, const char *expected_ip, uint16_t expected_port) {
  if (addr.sin_family != AF_INET)  // 检查地址族是否为 IPv4
    return false;
  if (addr.sin_port != htons(expected_port))  // 检查端口是否一致
    return false;

  in_addr expected_addr;
  if (inet_pton(AF_INET, expected_ip, &expected_addr) != 1)  // 检查 IP 地址格式
    return false;

  return addr.sin_addr.s_addr == expected_addr.s_addr;
}

// === 辅助函数：比较 IPv6 地址是否一致 ===
bool check_ipv6_content(const sockaddr_in6 &addr, const char *expected_ip, uint16_t expected_port) {
  if (addr.sin6_family != AF_INET6)  // 检查地址族是否为 IPv6
    return false;
  if (addr.sin6_port != htons(expected_port))  // 检查端口是否一致
    return false;

  in6_addr expected_addr;
  if (inet_pton(AF_INET6, expected_ip, &expected_addr) != 1)  // 检查 IP 地址格式
    return false;

  return memcmp(&addr.sin6_addr, &expected_addr, sizeof(in6_addr)) == 0;
}

int main() {
  int passed = 0;
  int total = 0;

  std::println("开始测试...");

  // --- 测试 1: 普通的 IPv4 地址 ---
  {
    total++;
    sockaddr_in addr = make_ipv4_addr("192.168.1.1", 8080);
    if (check_ipv4_content(addr, "192.168.1.1", 8080)) {
      std::println("[PASS] Test 1: make_ipv4_addr (192.168.1.1:8080)");
      passed++;
    } else {
      std::println("[FAIL] Test 1: Content mismatch for IPv4");
    }
  }

  // --- 测试 2: 无效的 IPv4 地址 (应抛出异常) ---
  {
    total++;
    bool caught_exception = false;
    try {
      make_ipv4_addr("300.400.500.600", 80);
    } catch (const std::runtime_error &e) {
      caught_exception = true;
    }
    if (caught_exception) {
      std::println("[PASS] Test 2: make_ipv4_addr throws on invalid IP");
      passed++;
    } else {
      std::println("[FAIL] Test 2: Did not throw exception for invalid IP");
    }
  }

  // --- 测试 3: INADDR_ANY (0.0.0.0) ---
  {
    total++;
    uint16_t port = 9090;
    sockaddr_in addr = make_any_addr(port);
    // INADDR_ANY 定义为 0，htonl(0) 仍然是 0
    bool check = (addr.sin_family == AF_INET) && (addr.sin_port == htons(port)) && (addr.sin_addr.s_addr == 0);

    if (check) {
      std::println("[PASS] Test 3: make_any_addr (0.0.0.0:9090)");
      passed++;
    } else {
      std::println("[FAIL] Test 3: INADDR_ANY mismatch");
    }
  }

  // --- 测试 4: INADDR_LOOPBACK (127.0.0.1) ---
  {
    total++;
    uint16_t port = 12345;
    sockaddr_in addr = make_loopback_addr(port);

    if (check_ipv4_content(addr, "127.0.0.1", port)) {
      std::println("[PASS] Test 4: make_loopback_addr (127.0.0.1:12345)");
      passed++;
    } else {
      std::println("[FAIL] Test 4: Loopback address mismatch");
    }
  }

  // --- 测试 5: 普通的 IPv6 地址 ---
  {
    total++;
    sockaddr_in6 addr = make_ipv6_addr("::1", 80);
    if (check_ipv6_content(addr, "::1", 80)) {
      std::println("[PASS] Test 5: make_ipv6_addr (::1:80)");
      passed++;
    } else {
      std::println("[FAIL] Test 5: IPv6 content mismatch");
    }
  }

  // --- 测试 6: 复杂的 IPv6 地址 ---
  {
    total++;
    sockaddr_in6 addr = make_ipv6_addr("2001:db8::1", 443);
    if (check_ipv6_content(addr, "2001:db8::1", 443)) {
      std::println("[PASS] Test 6: make_ipv6_addr (2001:db8::1:443)");
      passed++;
    } else {
      std::println("[FAIL] Test 6: IPv6 content mismatch");
    }
  }

  // --- 测试 7: 无效的 IPv6 地址 (应抛出异常) ---
  {
    total++;
    bool caught_exception = false;
    try {
      make_ipv6_addr("hello::world", 80);
    } catch (const std::runtime_error &e) {
      caught_exception = true;
    }
    if (caught_exception) {
      std::println("[PASS] Test 7: make_ipv6_addr throws on invalid IP");
      passed++;
    } else {
      std::println("[FAIL] Test 7: Did not throw exception for invalid IPv6");
    }
  }

  // --- 测试 8: 字节序验证 (测试端口转换) ---
  {
    total++;
    // 假设机器是小端序，8080 的网络序应该是 0x1F90 (即 8080 的大端序)
    // 在小端机器上，htons(8080) 结果在内存中看起来是 0x90 0x1F
    sockaddr_in addr = make_ipv4_addr("1.2.3.4", 8080);
    if (addr.sin_port == htons(8080)) {
      std::println("[PASS] Test 8: Port byte order conversion");
      passed++;
    } else {
      std::println("[FAIL] Test 8: Port byte order incorrect");
    }
  }

  std::println("---------------------------------");
  std::println("测试结果: {}/{}, {}", passed, total, " 通过");

  return (passed == total) ? 0 : 1;
}
