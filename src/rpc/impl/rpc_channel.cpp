/**
 * @File    :   src/rpc/impl/rpc_channel.cpp
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   客户端 RPC 通道实现
 *
 * 每次 call 创建新的 ClientSocket 连接（服务端处理完请求后会关闭连接），
 * 避免因连接复用导致 SIGPIPE。
 */

#include "rpc/rpc_channel.h"
#include "rpc/rpc_message.h"
#include "rpc/rpc_protocol.h"
#include "socket/client_socket.h"

#include <atomic>
#include <utility>

namespace sky {
namespace rpc {

RpcChannel::RpcChannel(const std::string &ip, uint16_t port) : m_ip(ip), m_port(port) {}

RpcResponse RpcChannel::call(const RpcRequest &req) {
  // 每次调用创建新连接（服务端处理完请求后会关闭连接）
  socket::ClientSocket client(m_ip, m_port);
  int fd = client.getSockFd();

  // 构造请求（使用自增 call_id）
  RpcRequest send_req = req;
  send_req.call_id = m_next_call_id.fetch_add(1, std::memory_order_relaxed);

  // 发送请求
  if (!sendRequest(fd, send_req)) {
    RpcResponse resp;
    resp.call_id = send_req.call_id;
    resp.status = static_cast<uint8_t>(RpcStatus::ERROR);
    return resp;
  }

  // 接收响应
  RpcResponse resp;
  if (!recvResponse(fd, resp)) {
    resp.call_id = send_req.call_id;
    resp.status = static_cast<uint8_t>(RpcStatus::ERROR);
    return resp;
  }

  if (resp.call_id != send_req.call_id) {
    RpcResponse mismatch_resp;
    mismatch_resp.call_id = send_req.call_id;
    mismatch_resp.status = static_cast<uint8_t>(RpcStatus::ERROR);
    return mismatch_resp;
  }

  return resp;
}

RpcResponse RpcChannel::call(const std::string &service, const std::string &method, RpcSerializer &params) {
  RpcRequest req;
  req.service_name = service;
  req.method_name = method;
  req.payload = params.data();

  return call(req);
}

}  // namespace rpc
}  // namespace sky
