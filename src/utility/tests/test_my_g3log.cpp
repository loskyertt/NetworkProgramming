/**
 * @File    :   examples\g3log\example_g3log02.cpp
 * @Time    :   2026/04/24 14:26:19
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   测试封装好的 g3log
 */

#include "logger/custom_file_sink.h"

#include <g3log/loglevels.hpp>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

#include <iostream>
#include <memory>

int main() {
  // 1. 创建 LogWorker
  auto worker = g3::LogWorker::createLogWorker();

  // 2. 添加自定义 Sink（关键：传递接收函数指针 &CustomFileSink::receiveLogMessage）
  auto sink_handle = worker->addSink(std::make_unique<CustomFileSink>("./logs/test02.log"),
      &CustomFileSink::receiveLogMessage  // [[2]] 指定接收函数
  );

  // 3. 初始化全局日志系统
  g3::initializeLogging(worker.get());

  // 4. 使用日志（与默认用法完全一致）
  LOG(INFO) << "name is sky, age is " << 22;
  LOG(DBUG) << "name is sky, age is " << 23;  // 如启用了 CHANGE_G3LOG_DEBUG_TO_DBUG
  LOG(WARNING) << "name is sky, age is " << 24;
  // LOG(FATAL) << "name is sky, age is " << 25;

  // 5. 程序退出时自动调用 shutDownLogging()（RAII 机制）
  return 0;
}
