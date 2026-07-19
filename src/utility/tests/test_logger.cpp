/**
 * @File    :   src/utility/test/test_logger.cpp
 * @Time    :   2026/04/14 20:28:45
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   Logger 测试
 */

#include "logger/logger.h"

using namespace sky::utility;

int main() {
  auto &logger = Singleton<Logger>::instance();

  logger.open("log/test_logger.log");
  // logger.set_level( Singleton<Logger>::Level::ERROR);
  logger.set_max(1000);

  LOG_DEBUG("name is %s, age is %d", "sky", 22);
  LOG_INFO("name is %s, age is %d", "sky", 22);
  LOG_WARN("name is %s, age is %d", "sky", 22);
  LOG_ERROR("name is %s, age is %d", "sky", 22);
  LOG_FATAL("name is %s, age is %d", "sky", 22);
  return 0;
}
