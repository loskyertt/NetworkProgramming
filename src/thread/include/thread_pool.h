/**
 * @File    :   src/thread/include/thread_pool.h
 * @Time    :   2026/04/19 22:40:17
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace sky {
namespace thread {

class ThreadPool {
 private:
  std::thread *m_manager;
  std::vector<std::thread> m_workers;

  // 这些变量不能保证在多线程下不会进行访问，故设为原子变量
  std::atomic<int> m_min_thread;      // 最小线程数
  std::atomic<int> m_max_thread;      // 最大线程数
  std::atomic<int> m_idle_thread;     // 空闲线程数
  std::atomic<int> m_current_thread;  // 当前线程数
  std::atomic<bool> m_is_running;     // 线程池是否运行

  std::queue<std::function<void(void)>> m_task_queue;  // 任务队列
  std::mutex m_task_mutex;                             // 任务队列锁
  std::condition_variable m_task_condition;            // 任务队列条件变量
};

}  // namespace thread
}  // namespace sky
