/**
 * @File    :   src/test_thread_pool.cpp
 * @Time    :   2026/04/19 22:40:23
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   线程池测试
 */

#include "thread/thread_pool.h"

#include <chrono>
#include <cstdio>
#include <print>
#include <thread>

using namespace sky::thread;

void calc(int a, int b) {
  std::println("calc: {} + {} = {}", a, b, a + b);
  std::this_thread::sleep_for(std::chrono::seconds(2));
}

int main() {
  ThreadPool pool;
  for (int i = 0; i < 10; ++i) {
    pool.addTask(std::bind(calc, i, i * 2));  // 方式一
    // pool.addTask([=]() { calc(i, i * 2); });  // 方式二（一定要用值传递）
  }

  getchar();
}
