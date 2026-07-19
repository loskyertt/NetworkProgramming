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
#include <memory>
#include <mutex>
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

/**
 * @brief RPC 业务处理函数类型。
 *
 * @details Handler 接收请求 payload 中的序列化参数，返回响应 payload 中的序列化结果。
 * 具体参数和返回值的编码顺序由对应 Stub 与 handler 共同约定。
 *
 * @param params 请求中的序列化参数字节。
 * @return 序列化后的业务返回值字节。
 */
using RpcHandler = std::function<std::vector<char>(const std::vector<char> &)>;

/**
 * @brief RPC 服务端。
 *
 * @details 服务端负责监听 TCP 连接、读取完整 RPC 请求、根据 service.method 查找
 * 已注册 handler，并在线程池中执行业务处理。当前实现是一连接一请求模型：
 * 服务端读取一个请求、发送一个响应后关闭该连接。
 */
class RpcServer {
 public:
  /**
   * @brief 构造 RPC 服务端。
   *
   * @details 构造过程会创建监听 socket、epoll 封装和线程池对象。
   *
   * @param ip 监听 IP 地址，例如 "127.0.0.1" 或 "0.0.0.0"。
   * @param port 监听端口。
   */
  RpcServer(const std::string &ip, uint16_t port);

  /**
   * @brief 析构 RPC 服务端。
   *
   * @details 析构时会调用 stop()，使事件循环在下一次超时或事件返回后退出。
   */
  ~RpcServer();

  /**
   * @brief 注册一个服务方法处理器。
   *
   * @details 服务端内部使用 service + "." + method 作为 handler 表的 key。若重复注册同一个服务方法，
   * 新的 handler 会覆盖旧的 handler。注册过程受互斥锁保护，可与请求处理并发执行。
   *
   * @param service 服务名，例如 "Calculator"。
   * @param method 方法名，例如 "add"。
   * @param handler 业务处理函数；输入为请求 payload，输出为响应 payload。
   */
  void register_handler(const std::string &service, const std::string &method, RpcHandler handler);

  /**
   * @brief 启动 RPC 服务端事件循环。
   *
   * @details 该函数会阻塞当前线程，直到 stop() 被调用或 epoll 出现不可恢复错误。
   * 新连接由 epoll 监听，完整请求读取成功后会提交到线程池执行 handler。
   */
  void start();

  /**
   * @brief 请求停止 RPC 服务端。
   *
   * @details 该函数将运行标志置为 false。由于事件循环使用带超时的 epoll 等待，start() 通常会在下一次等待返回后退出。
   */
  void stop();

 private:
  /**
   * @brief 处理一个已完整读取的 RPC 请求。
   *
   * @param conn_fd 客户端连接文件描述符。
   * @param req 已解析完成的 RPC 请求。
   */
  void handle_request(int conn_fd, const RpcRequest &req);

  /**
   * @brief 向客户端连接写入 RPC 响应。
   *
   * @param conn_fd 客户端连接文件描述符。
   * @param resp 待发送的 RPC 响应。
   */
  void send_response_to_conn(int conn_fd, const RpcResponse &resp);

  std::unique_ptr<socket::ServerSocket> m_server;
  std::unique_ptr<socket::EPoller> m_epoller;
  std::unique_ptr<thread::ThreadPool> m_pool;

  std::unordered_map<std::string, RpcHandler> m_handlers;
  std::mutex m_handlers_mutex;
  std::atomic<bool> m_running{false};
};

}  // namespace rpc
}  // namespace sky
