/**
 * @File    :   src/utility/include/logger/custom_file_sink.h
 * @Time    :   2026/04/24 13:51:55
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   重新封装 g3log
 */

#pragma once

#include <g3log/logmessage.hpp>

#include <fstream>

namespace sky::utility {

class CustomFileSink {
private:
  std::ofstream m_out;

public:
  explicit CustomFileSink(const std::string &log_file_path);

  // Sink 接收函数：接收 LogMessage 对象进行自定义格式化
  void receive_log_message(g3::LogMessageMover message);
};

}  // namespace sky::utility
