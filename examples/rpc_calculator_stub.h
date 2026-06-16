/**
 * @File    :   examples/rpc_calculator_stub.h
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   RPC Calculator 客户端存根
 *
 * 演示 Stub 模式：将远程调用封装成本地方法调用
 *   CalculatorStub stub(channel);
 *   int result = stub.add(3, 5);  // 看起来是本地调用，实际通过 RPC 执行
 */

#pragma once

#include "rpc/rpc_channel.h"
#include "rpc/rpc_serializer.h"

#include <print>
#include <stdexcept>
#include <string>

namespace sky {
namespace rpc {

class CalculatorStub {
 public:
  explicit CalculatorStub(const std::string &ip, uint16_t port)
      : m_channel(ip, port) {
  }

  /** 加法 */
  int add(int a, int b) {
    RpcSerializer params;
    params.writeInt32(a);
    params.writeInt32(b);

    RpcResponse resp = m_channel.call("Calculator", "add", params);

    if (resp.status != 0) {
      throw std::runtime_error("RPC call Calculator.add failed");
    }
    RpcSerializer result;
    result.reset(resp.payload);
    return result.readInt32();
  }

  /** 减法 */
  int subtract(int a, int b) {
    RpcSerializer params;
    params.writeInt32(a);
    params.writeInt32(b);

    RpcResponse resp = m_channel.call("Calculator", "subtract", params);

    if (resp.status != 0) {
      throw std::runtime_error("RPC call Calculator.subtract failed");
    }
    RpcSerializer result;
    result.reset(resp.payload);
    return result.readInt32();
  }

  /** 乘法 */
  int multiply(int a, int b) {
    RpcSerializer params;
    params.writeInt32(a);
    params.writeInt32(b);

    RpcResponse resp = m_channel.call("Calculator", "multiply", params);

    if (resp.status != 0) {
      throw std::runtime_error("RPC call Calculator.multiply failed");
    }
    RpcSerializer result;
    result.reset(resp.payload);
    return result.readInt32();
  }

  /** 除法 */
  int divide(int a, int b) {
    RpcSerializer params;
    params.writeInt32(a);
    params.writeInt32(b);

    RpcResponse resp = m_channel.call("Calculator", "divide", params);

    if (resp.status != 0) {
      throw std::runtime_error("RPC call Calculator.divide failed: division by zero");
    }
    RpcSerializer result;
    result.reset(resp.payload);
    return result.readInt32();
  }

 private:
  RpcChannel m_channel;
};

}  // namespace rpc
}  // namespace sky
