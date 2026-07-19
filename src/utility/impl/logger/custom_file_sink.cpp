/**
 * @File    :   src/utility/impl/logger/custom_file_sink.cpp
 * @Time    :   2026/04/24 13:52:15
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "logger/custom_file_sink.h"

using namespace sky::utility;

CustomFileSink::CustomFileSink(const std::string &log_file_path) : m_out(log_file_path, std::ios::app) {
  if (!m_out.is_open()) {
    throw std::runtime_error("Failed to open log file: " + log_file_path);
  }
}

// Sink 接收函数：接收 LogMessage 对象进行自定义格式化
void CustomFileSink::receive_log_message(g3::LogMessageMover message) {
  const auto &msg = message.get();

  // 1. 格式化时间：[2026-04-24 13:28:35]
  auto time_str = msg.timestamp("%Y-%m-%d %H:%M:%S");

  // 2. 获取日志级别（转换为大写短格式）
  std::string level = msg.level();
  if (level == "DEBUG")
    level = "DEBUG";
  else if (level == "INFO")
    level = "INFO";
  else if (level == "WARNING")
    level = "WARN";
  else if (level == "FATAL")
    level = "FATAL";
  else if (level == "ERROR")
    level = "ERROR";  // 如有自定义 ERROR 级别

  // 3. 获取完整文件路径 + 行号
  std::string file_path = msg.file_path();  // 关键：使用 file_path() 而非 file()
  std::string line = msg.line();

  // 4. 获取日志消息内容
  std::string log_msg = msg.message();

  // 5. 组装输出格式
  std::ostringstream oss;
  oss << "[" << time_str << "] " << level << " " << file_path << ":" << line << " " << log_msg << "\n";

  // 6. 写入文件（注意：g3log 保证此函数在后台线程串行执行，无需额外加锁）
  m_out << oss.str() << std::flush;
}
