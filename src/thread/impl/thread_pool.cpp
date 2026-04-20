/**
 * @File    :   src/thread/impl/thread_pool.cpp
 * @Time    :   2026/04/19 22:40:20
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "thread/thread_pool.h"

#include <chrono>
#include <functional>
#include <mutex>
#include <print>
#include <thread>
#include <utility>

using namespace sky::thread;

ThreadPool::ThreadPool(int min_thread, int max_thread)
    : m_min_thread(min_thread),
      m_max_thread(max_thread),
      m_idle_thread(min_thread),
      m_current_thread(min_thread),
      m_is_running(true) {
  // 初始化比在大括号 {} 中进行赋值的效率高很多

  // 创建管理者线程
  m_manager = new std::thread(&ThreadPool::doManage, this);

  // 创建工作线程
  for (int i = 0; i < min_thread; i++) {
    std::thread t(&ThreadPool::doWork, this);
    m_workers.insert(std::make_pair(t.get_id(), std::move(t)));
  }
}

ThreadPool::~ThreadPool() {
  m_is_running.store(false);
  m_task_condition.notify_all();

  for (auto &pair : m_workers) {
    std::thread &t = pair.second;
    if (t.joinable()) {
      std::println("**** 工作线程 {} 将要被销毁 ....", t.get_id());
      t.join();
    }
  }

  if (m_manager->joinable()) {
    std::println("**** 管理者线程 {} 将要被销毁 ....", m_manager->get_id());
    m_manager->join();
  }
  if (m_manager) {
    delete m_manager;
    m_manager = nullptr;
  }
}

void ThreadPool::addTask(std::function<void(void)> task) {
  // TODO: 实现添加任务的逻辑
  {
    std::lock_guard<std::mutex> lock(m_task_mutex);
    m_task_queue.emplace(task);
  }
  m_task_condition.notify_one();  // 任务队列不为空，就唤醒
}

void ThreadPool::doManage(void) {
  while (m_is_running.load()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));  // 阻塞三秒：运行态 -> 阻塞态

    int idle_thread = m_idle_thread.load();
    int current_thread = m_current_thread.load();
    // 条件满足，说明空闲的线程过多，需要移除一些空闲线程
    // 和 m_min_thread 比较的作用是：保证线程池即使在完全空闲时，也保留足够的线程随时响应任务，不会缩容到失去服务能力
    if (current_thread > m_min_thread.load() && idle_thread > current_thread / 2) {
      // 每次销毁两个线程
      m_exit_thread.store(2);
      m_task_condition.notify_all();  // 会通知所有在等待的线程，让它们退出
      /* lock ids 开始 */
      {
        std::lock_guard<std::mutex> lock_ids(m_ids_mutex);
        for (auto id : m_ids) {
          auto it = m_workers.find(id);
          if (it != m_workers.end()) {
            std::println("==== 工作线程 {} 将要被销毁（doManage） ...", it->first);
            it->second.join();  // 阻塞管理者线程
            m_workers.erase(it);
          }
        }
        m_ids.clear();
      }
    }
    // 没有空闲线程，且当前线程数小于最大线程数，就创建新线程
    // 和 m_max_thread 比较的作用是：防止在持续高负载下无限创建线程，耗尽系统的内存和文件描述符等资源。
    else if (current_thread < m_max_thread.load() && idle_thread == 0) {
      std::thread t(&ThreadPool::doWork, this);
      m_workers.insert(std::make_pair(t.get_id(), std::move(t)));
      ++m_current_thread;
      ++m_idle_thread;
    }
  }
}

void ThreadPool::doWork(void) {
  while (m_is_running.load()) {
    std::function<void(void)> task = nullptr;
    /* lock 开始 */
    {
      std::unique_lock<std::mutex> lock(m_task_mutex);
      // 只要任务队列为空，且线程池正在运行，就把线程挂起：线程从运行态 -> 阻塞态
      while (m_task_queue.empty() && m_is_running.load()) {
        m_task_condition.wait(lock);  // 让工作线程进入阻塞态
        if (m_exit_thread.load() > 0) {
          --m_idle_thread;
          --m_current_thread;
          --m_exit_thread;
          /* lock ids 开始 */
          {
            std::lock_guard<std::mutex> lock_ids(m_ids_mutex);
            std::println("---- 工作线程 {} 将要退出（doWork） ...", std::this_thread::get_id());
            m_ids.emplace_back(std::this_thread::get_id());
          }
          return;  // 让线程退出任务函数，即工作线程少了一个
        }
      }
      if (!m_task_queue.empty()) {
        std::println("---- 线程 {} 取出一个任务...", std::this_thread::get_id());
        task = std::move(m_task_queue.front());
        m_task_queue.pop();
      }
    }
    if (task) {
      --m_idle_thread;
      task();  // 执行任务
      ++m_idle_thread;
    }
  }
}
