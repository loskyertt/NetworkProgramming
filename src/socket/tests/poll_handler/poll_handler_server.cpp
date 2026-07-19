/**
 * @File    :   src/socket/tests/poll_handler/poll_handler_server.cpp
 * @Time    :   2026/04/16 22:08:16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   PollHandler 封装示例：使用封装好的 PollHandler 处理并发连接
 */

#include "logger/logger.h"
#include "socket/poll/poll_handler.h"

using namespace sky::socket;
using namespace sky::utility;

int main() {
  // 初始化日志
  Singleton<Logger>::instance().open("log/server.log");

  auto &handler = Singleton<PollHandler>::instance();

  // 监听指定端口
  handler.listen("127.0.0.1", 8080);

  // 开始处理事件
  handler.handle(1024);

  return 0;
}
