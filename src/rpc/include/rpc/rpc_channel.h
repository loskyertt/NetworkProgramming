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

#include <string>

namespace sky {
namespace rpc {

class RpcChannel {
 public:
  /**
   * @brief 设置 RPC 服务器地址
   * @param ip   服务器 IP
   * @param port 服务器端口
   */
  RpcChannel(const std::string &ip, uint16_t port);
  ~RpcChannel() = default;

  /**
   * @brief 发起远程调用（每次调用独立连接）
   * @param req 请求
   * @return 响应
   */
  RpcResponse call(const RpcRequest &req);

  /**
   * @brief 便捷调用：从序列化器构造请求
   * @param service  服务名
   * @param method   方法名
   * @param params   已写入参数的序列化器
   * @return 响应
   */
  RpcResponse call(const std::string &service,
                   const std::string &method,
                   RpcSerializer &params);

 private:
  std::string m_ip;
  uint16_t m_port;
  uint32_t m_next_call_id = 1;
};

}  // namespace rpc
}  // namespace sky
