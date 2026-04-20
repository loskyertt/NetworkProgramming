/**
 * @File    :   src/server.cpp
 * @Time    :   2026/04/16 22:08:16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   poll 示例
 */

#include "logger/logger.h"
#include "socket/poll/poll_handler.h"

using namespace sky::socket;
using namespace sky::utility;

int main() {
  // 初始化日志
  Singleton<Logger>::getInstance().open("log/server.log");

  auto &handler = Singleton<PollHandler>::getInstance();

  // 监听指定端口
  handler.listen("127.0.0.1", 8080);

  // 开始处理事件
  handler.handle(1024);

  return 0;
}
