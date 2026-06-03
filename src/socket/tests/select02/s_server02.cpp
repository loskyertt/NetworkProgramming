/**
 * @File    :   src/server.cpp
 * @Time    :   2026/04/16 22:08:16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "logger/logger.h"
#include "socket/select/select_handler.h"

using namespace sky::socket;
using namespace sky::utility;

int main() {
  // 初始化日志
  Singleton<Logger>::getInstance().open("log/server.log");

  auto &handler = Singleton<SelectHandler>::getInstance();

  // 监听指定端口
  handler.listen("127.0.0.1", 8080);

  // 开始处理事件（5000ms 超时）
  handler.handle(5000);

  return 0;
}
