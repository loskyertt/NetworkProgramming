/**
 * @File    :   src/rpc/examples/rpc_calculator_client.cpp
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   RPC Calculator 客户端示例
 *
 * 通过 CalculatorStub 远程调用服务器上的计算函数。
 *
 * 构建 & 运行：
 *   cd build && make rpc_calculator_client
 *   ./src/rpc/examples/rpc_calculator_client
 */

#include "rpc_calculator_stub.h"
#include "logger/logger.h"

#include <print>
#include <stdexcept>

using namespace sky::rpc;
using namespace sky::utility;

int main() {
  // 初始化日志
  Singleton<Logger>::getInstance().open("log/rpc_client.log");

  try {
    // 创建存根（连接到 127.0.0.1:8080）
    CalculatorStub calc("127.0.0.1", 8080);

    // 远程调用：看起来和本地调用一模一样
    int sum = calc.add(3, 5);
    std::println("Calculator::add(3, 5) = {}", sum);

    int diff = calc.subtract(10, 4);
    std::println("Calculator::subtract(10, 4) = {}", diff);

    int product = calc.multiply(6, 7);
    std::println("Calculator::multiply(6, 7) = {}", product);

    int quotient = calc.divide(42, 6);
    std::println("Calculator::divide(42, 6) = {}", quotient);

    // 测试错误处理：除零异常
    try {
      calc.divide(1, 0);
    } catch (const std::exception &e) {
      std::println("Calculator::divide(1, 0) = ERROR: {}", e.what());
    }

  } catch (const std::exception &e) {
    std::println("Error: {}", e.what());
    return 1;
  }

  return 0;
}
