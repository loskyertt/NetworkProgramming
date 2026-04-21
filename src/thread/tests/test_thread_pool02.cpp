/**
 * @File    :   src/thread/tests/test_async.cpp
 * @Time    :   2026/04/21 14:16:12
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   异步测试
 */

#include "thread/thread_pool.h"

#include <chrono>
#include <future>
#include <print>
#include <thread>
#include <vector>

using namespace sky::thread;

int calc(int a, int b) {
  int res = a + b;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  return res;
}

int main() {
  std::vector<std::future<int>> res;

  ThreadPool pool;
  for (int i = 0; i < 10; ++i) {
    res.emplace_back(pool.addTask(calc, i, i * 2));
  }

  for (auto &item : res) {
    std::println("获得的结果: {}", item.get());
  }
}
