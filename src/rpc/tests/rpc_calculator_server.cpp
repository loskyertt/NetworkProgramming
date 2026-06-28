/**
 * @File    :   examples/rpc_calculator_server.cpp
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   RPC Calculator 服务端示例
 *
 * 注册 Calculator 服务的四个方法（add, subtract, multiply, divide），
 * 启动 RPC 服务器等待客户端调用。
 *
 * 构建 & 运行：
 *   mkdir -p build && cd build
 *   cmake .. && make rpc_calculator_server
 *   ./examples/rpc_calculator_server
 */

#include "rpc/rpc_server.h"
#include "rpc/rpc_serializer.h"
#include "logger/logger.h"

#include <print>

using namespace sky::utility;
using namespace sky::rpc;

int main() {
  // 初始化日志
  Singleton<Logger>::getInstance().open("log/rpc_server.log");

  // 创建 RPC 服务器
  RpcServer server("127.0.0.1", 8080);

  // ===== 注册 Calculator 服务 =====

  // add(a, b) → a + b
  server.registerHandler("Calculator", "add", [](const std::vector<char> &params) {
    RpcSerializer reader;
    reader.reset(params);
    int a = reader.readInt32();
    int b = reader.readInt32();
    int result = a + b;
    Log_info("Calculator::add(%d, %d) = %d", a, b, result);

    RpcSerializer writer;
    writer.writeInt32(result);
    return writer.data();
  });

  // subtract(a, b) → a - b
  server.registerHandler("Calculator", "subtract", [](const std::vector<char> &params) {
    RpcSerializer reader;
    reader.reset(params);
    int a = reader.readInt32();
    int b = reader.readInt32();
    int result = a - b;
    Log_info("Calculator::subtract(%d, %d) = %d", a, b, result);

    RpcSerializer writer;
    writer.writeInt32(result);
    return writer.data();
  });

  // multiply(a, b) → a * b
  server.registerHandler("Calculator", "multiply", [](const std::vector<char> &params) {
    RpcSerializer reader;
    reader.reset(params);
    int a = reader.readInt32();
    int b = reader.readInt32();
    int result = a * b;
    Log_info("Calculator::multiply(%d, %d) = %d", a, b, result);

    RpcSerializer writer;
    writer.writeInt32(result);
    return writer.data();
  });

  // divide(a, b) → a / b，除数不能为 0
  server.registerHandler("Calculator", "divide", [](const std::vector<char> &params) {
    RpcSerializer reader;
    reader.reset(params);
    int a = reader.readInt32();
    int b = reader.readInt32();

    if (b == 0) {
      Log_warn("Calculator::divide(%d, %d) division by zero!", a, b);
      throw std::runtime_error("division by zero");
    }

    int result = a / b;
    Log_info("Calculator::divide(%d, %d) = %d", a, b, result);

    RpcSerializer writer;
    writer.writeInt32(result);
    return writer.data();
  });

  // 启动服务（阻塞）
  std::println("Calculator RPC server starting on 127.0.0.1:8080 ...");
  server.start();

  return 0;
}
