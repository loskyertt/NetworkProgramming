/**
 * @File    :   src/rpc/include/rpc/rpc_server.h
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   RPC 服务端
 *
 * 基于 EPoller（LT 模式）+ ThreadPool 的事件驱动 RPC 服务端：
 *   1. 监听新连接（listen_fd）
 *   2. 读取完整请求（conn_fd）
 *   3. 提交到线程池处理
 *   4. 将响应写回客户端
 *
 * Handler 签名：std::vector<char>(const std::vector<char>&)
 *   输入：序列化后的参数
 *   输出：序列化后的返回值
 */

#pragma once

#include "rpc_message.h"

#include <atomic>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace sky {
namespace socket {
class ServerSocket;
class EPoller;
}  // namespace socket

namespace thread {
class ThreadPool;
}  // namespace thread

namespace rpc {

/** Handler 签名：接收序列化参数，返回序列化结果 */
using RpcHandler = std::function<std::vector<char>(const std::vector<char> &)>;

class RpcServer {
 public:
  /**
   * @param ip   监听 IP
   * @param port 监听端口
   */
  RpcServer(const std::string &ip, uint16_t port);
  ~RpcServer();

  /**
   * @brief 注册服务方法处理器
   * @param service  服务名
   * @param method   方法名
   * @param handler  处理函数
   */
  void registerHandler(const std::string &service,
                       const std::string &method,
                       RpcHandler handler);

  /**
   * @brief 启动服务端（阻塞，直到 stop 被调用）
   */
  void start();

  /**
   * @brief 停止服务端
   */
  void stop();

 private:
  /** 处理单个连接上的一个完整请求 */
  void handleRequest(int conn_fd, const RpcRequest &req);

  /** 向连接写入响应 */
  void sendResponseToConn(int conn_fd, const RpcResponse &resp);

  socket::ServerSocket *m_server;
  socket::EPoller      *m_epoller;
  thread::ThreadPool   *m_pool;

  std::unordered_map<std::string, RpcHandler> m_handlers;  // key = "service.method"
  std::atomic<bool> m_running{false};
};

}  // namespace rpc
}  // namespace sky
