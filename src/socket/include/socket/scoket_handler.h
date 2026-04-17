/**
 * @File    :   src/socket/include/socket/scoket_handler.h
 * @Time    :   2026/04/17 13:34:47
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#pragma once

#include "socket/selector.h"
#include "singleton.h"

#include <cstdint>
#include <map>
#include <string>

namespace sky {
namespace socket {

class Socket;

class SocketHandler {
  friend class sky::utility::Singleton<SocketHandler>;  // 允许 Singleton 访问私有构造函数

 private:
  Socket *m_server = nullptr;             // 监听（对象）套接字
  Selector m_selector;                    // 事件监控器（组合关系）
  std::map<int, Socket *> m_connections;  // 连接（对象）套接字：fd -> Socket*
                                          // 作用：统一管理所有客户端连接的生命周期

 public:
  /**
  * @brief 包含操作：创建 listen_fd -> bind -> listen -> 把 listen_fd 加入监控集合
  *
  * - @param ip 监听的 IP 地址
  * - @param port 监听的端口号
  */
  void listen(const std::string &ip, uint16_t port);

  /**
   * @brief 添加套接字的 conn_fd 到监控集合，并建立 conn_fd 和 连接套接字对象的映射关系（map）
   *
   * - @param socket 要添加的套接字
   */
  void attach(Socket *socket);

  /**
   * @brief 仅从 I/O 监控中移除，不再监听这个 conn_fd 的事件，连接对象仍然保留
   *
   * - @param socket 要删除的套接字
   */
  void detach(Socket *socket);

  /**
   * @brief 完全移除连接，从监控集合和连接映射中都删除
   *
   * - @param socket 要移除的套接字
   */
  void remove(Socket *socket);

  /**
   * @brief 处理事件
   *
   * - @param wait_time 等待时间（毫秒）
   */
  void handle(int wait_time);

 private:
  /* 处理新连接 */
  void handleNewConnection();

  /* 处理客户端连接 */
  void handleClientConnections();

 private:
  SocketHandler();
  ~SocketHandler();
  SocketHandler(const SocketHandler &) = delete;
  SocketHandler &operator=(const SocketHandler &) = delete;
};

}  // namespace socket
}  // namespace sky
