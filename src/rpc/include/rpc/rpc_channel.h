/**
 * @File    :   src/rpc/include/rpc/rpc_channel.h
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   客户端 RPC 通道
 *
 * 每次 call 创建一个新连接（服务端处理完一个请求后会关闭连接）。
 * 封装 ClientSocket，提供 call 接口：
 *   连接 → 序列化参数 → 发送请求 → 等待响应 → 反序列化返回值
 */

#pragma once

#include "rpc_message.h"
#include "rpc_serializer.h"

#include <atomic>
#include <string>

namespace sky {
namespace rpc {

/**
 * @brief RPC 客户端通道。
 *
 * @details RpcChannel 负责连接服务端、分配 call_id、发送请求、接收响应，
 * 并校验响应中的 call_id 是否与本次请求匹配。
 * 业务层通常不直接操作 socket，而是通过 Stub 调用该类完成一次 RPC。
 *
 * 当前实现采用短连接模型：每次 call 都创建一个新的 TCP 连接，收到响应后由底层 socket 生命周期关闭连接。
 */
class RpcChannel {
 private:
  std::string m_ip;
  uint16_t m_port;
  std::atomic<uint32_t> m_next_call_id{1};

 public:
  /**
   * @brief 构造 RPC 客户端通道。
   *
   * @details 构造函数只保存服务端地址，不立即建立连接；真正的连接发生在每次 call 调用期间。
   *
   * @param ip RPC 服务端 IP 地址，例如 "127.0.0.1"。
   * @param port RPC 服务端监听端口。
   */
  RpcChannel(const std::string &ip, uint16_t port);
  ~RpcChannel() = default;

  /**
   * @brief 发起一次远程调用。
   *
   * @details 函数会复制传入请求并写入新的自增 call_id，因此调用者设置在 req 中的
   * call_id 会被覆盖。若发送失败、接收失败或响应 call_id 不匹配，
   * 函数返回一个 status = RpcStatus::ERROR 的响应。
   *
   * @param req RPC 请求。service_name、method_name 和 payload 应由 Stub
   * 或调用方提前填好。
   * @return RPC 响应。调用方需要检查 status 后再反序列化 payload。
   */
  RpcResponse call(const RpcRequest &req);

  /**
   * @brief 使用服务名、方法名和序列化器发起一次远程调用。
   *
   * @details 该重载会从 params.data() 复制请求 payload，并构造 RpcRequest 后委托给
   * call(const RpcRequest&)。
   *
   * @param service 服务名，例如 "Calculator"。
   * @param method 方法名，例如 "add"。
   * @param params 已写入调用参数的序列化器。
   * @return RPC 响应。调用方需要检查 status 后再反序列化 payload。
   */
  RpcResponse call(const std::string &service, const std::string &method, RpcSerializer &params);
};

}  // namespace rpc
}  // namespace sky
