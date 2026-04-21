/**
 * @File    :   src/thread/include/thread_pool.h
 * @Time    :   2026/04/19 22:40:17
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   异步线程池
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sky {
namespace thread {

class ThreadPool {
 private:
  std::thread *m_manager;                                      // 管理者线程
  std::vector<std::thread::id> m_ids;                          // 存储已经退出任务函数的线程 ID
  std::unordered_map<std::thread::id, std::thread> m_workers;  // 工作线程容器

  // 这些变量不能保证在多线程下不会进行访问，故设为原子变量
  std::atomic<int> m_min_thread;      // 最小线程数
  std::atomic<int> m_max_thread;      // 最大线程数
  std::atomic<int> m_idle_thread;     // 空闲线程数，一般 <= 当前线程数
  std::atomic<int> m_current_thread;  // 当前线程数
  std::atomic<int> m_exit_thread;     // 退出的线程数
  std::atomic<bool> m_is_running;     // 线程池是否运行

  std::queue<std::function<void(void)>> m_tasks;  // 任务队列
  std::mutex m_task_mutex;                        // 任务队列互斥锁
  std::mutex m_ids_mutex;                         // 访问 m_ids 容器的互斥锁
  std::condition_variable m_queue_condition;      // 任务队列条件变量

 public:
  ThreadPool(int min_thread = 4, int max_thread = static_cast<int>(std::thread::hardware_concurrency()));
  ~ThreadPool();

  /* 添加任务 -> 任务队列 */
  void addTask(std::function<void(void)> task);

  template <typename F, typename... Args>
  auto addTask(F &&f, Args &&...args) {
    // 定义返回类型
    using returnType = std::invoke_result_t<F, Args...>;
    // 1. 创建 package_task 实例
    auto my_task = std::make_shared<std::packaged_task<returnType()>>(
        [_f = std::forward<F>(f), _args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
          return std::apply(_f, _args);
        });

    // 2. 获取 future
    std::future<returnType> res = my_task->get_future();
    // 3. 将任务添加到任务队列
    {
      std::lock_guard<std::mutex> lock(m_task_mutex);
      m_tasks.emplace([my_task]() { (*my_task)(); });
    }
    m_queue_condition.notify_one();
    return res;
  }

 private:
  /* 管理者函数 */
  void doManage(void);

  /* 工作者函数 */
  void doWork(void);
};

}  // namespace thread
}  // namespace sky
