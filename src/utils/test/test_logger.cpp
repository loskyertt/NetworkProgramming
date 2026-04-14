/**
 * @File    :   src/utils/test/test_logger.cpp
 * @Time    :   2026/04/14 20:28:45
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   Logger 测试
 */

#include "logger/logger.h"

int main() {
  utility::Logger &logger = utility::Logger::getInstance();

  logger.open("log/test.log");
  // logger.setLevel(utility::Logger::Level::ERROR);
  logger.setMax(1000);

  // logger.log(utility::Logger::Level::DEBUG, __FILE__, __LINE__, "name is %s, age is %d", "sky", 22);
  debug("name is %s, age is %d", "sky", 22);
  info("name is %s, age is %d", "sky", 22);
  warn("name is %s, age is %d", "sky", 22);
  error("name is %s, age is %d", "sky", 22);
  fatal("name is %s, age is %d", "sky", 22);
  return 0;
}
