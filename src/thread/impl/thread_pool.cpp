/**
 * @File    :   src/thread/impl/thread_pool.cpp
 * @Time    :   2026/04/19 22:40:20
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   .....
 */

#include "thread_pool/thread_pool.h"

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
      m_exit_thread(0),
      m_active_task(0),
      m_is_running(true) {
  // 初始化比在大括号 {} 中进行赋值的效率高很多

  // 创建管理者线程
  m_manager = new std::thread(&ThreadPool::doManage, this);
  // &ThreadPool::doManage：获取成员函数的指针
  // this：作为隐藏参数传递给成员函数，相当于 this->doManage()

  // 创建工作线程
  for (int i = 0; i < min_thread; i++) {
    std::thread t(&ThreadPool::doWork, this);
    m_workers.insert(std::make_pair(t.get_id(), std::move(t)));
  }
}

ThreadPool::~ThreadPool() {
  m_is_running.store(false);
  m_queue_condition.notify_all();

  // 这里要避免「调用析构函数的主线程」和「调用 doManage 的管理者线程」之间存在的竞争窗口
  // 所以要把清理工作放在停止「管理者线程」之后，清理工作也会访问 m_workers

  // 第一步：先停止「管理者线程」，确保「管理者线程」不再修改 m_workers
  if (m_manager->joinable()) {
    std::println("**** 管理者线程 {} 将要被销毁 ....", m_manager->get_id());
    m_manager->join();
  }
  delete m_manager;
  m_manager = nullptr;

  // 第二步：再清理工作线程
  for (auto &pair : m_workers) {
    std::thread &t = pair.second;
    if (t.joinable()) {
      std::println("**** 工作线程 {} 将要被销毁 ....", t.get_id());
      t.join();
    }
  }
}

void ThreadPool::addTask(std::function<void(void)> task) {
  // TODO: 实现添加任务的逻辑
  {
    std::lock_guard<std::mutex> lock(m_task_mutex);
    m_tasks.push(std::move(task));  // 移动而非拷贝
  }
  m_queue_condition.notify_one();  // 任务队列不为空，就唤醒
}

void ThreadPool::waitForDone() {
  std::unique_lock<std::mutex> lock(m_task_mutex);
  m_done_condition.wait(lock, [this] {
    // 条件为 false，就阻塞；一旦条件为 true，就立即返回
    return m_tasks.empty() && m_active_task.load() == 0;
    //     ↑ 队列无等待任务    ↑ 且无正在执行的任务
  });
}

void ThreadPool::doManage(void) {
  while (m_is_running.load()) {

    // 第一步：清理上一轮已退出的线程（此时它们已有足够时间写入 m_ids）
    {
      // 先在锁内取出 ids，然后释放锁（减少锁持有时间）
      std::vector<std::thread::id> ids_to_join;
      /* lock_ids 作用域（和「工作线程」竞争）：只用于读取 m_ids */
      {
        std::lock_guard<std::mutex> lock_ids(m_ids_mutex);
        ids_to_join.swap(m_ids);  // swap 比 copy + clear 更高效
      }

      // 锁外执行耗时的 join 和 erase
      for (auto id : ids_to_join) {
        auto it = m_workers.find(id);
        if (it != m_workers.end()) {
          it->second.join();
          m_workers.erase(it);
        }
      }
    }

    // 第二步：休眠，期间工作线程有充足时间完成退出并写入 m_ids
    std::this_thread::sleep_for(std::chrono::seconds(1));  // 阻塞 1 秒：运行态 -> 阻塞态

    // 第三步：检查是否需要扩容或缩容
    int idle_thread = m_idle_thread.load();
    int current_thread = m_current_thread.load();
    // 条件满足，说明空闲的线程过多，需要移除一些空闲线程
    // 和 m_min_thread 比较的作用是：保证线程池即使在完全空闲时，也保留足够的线程随时响应任务，不会缩容到失去服务能力
    if (current_thread > m_min_thread.load() && idle_thread > current_thread / 2) {
      // 每次销毁两个线程
      m_exit_thread.store(2);

      // 唤醒两个线程
      m_queue_condition.notify_one();
      m_queue_condition.notify_one();
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
    /* lock 作用域（工作线程之间的竞争）：只用于操作队列和检查退出信号 */
    {
      std::unique_lock<std::mutex> lock(m_task_mutex);
      // 任务队列为空、线程池正在运行、没有退出信号这三者同时满足，就把线程阻塞：线程从运行态 -> 阻塞态
      while (m_tasks.empty() && m_is_running.load() && m_exit_thread.load() == 0) {
        m_queue_condition.wait(lock);  // 让工作线程进入阻塞态，唤醒后会重写竞争互斥锁
      }
      // 任务优先
      if (!m_tasks.empty()) {
        std::println("---- 线程 {} 取出一个任务...", std::this_thread::get_id());
        task = std::move(m_tasks.front());
        m_tasks.pop();
      }
      // 队列为空时才响应退出信号
      else if (m_exit_thread.load() > 0) {
        --m_idle_thread;
        --m_current_thread;
        --m_exit_thread;
        /* lock_ids 作用域（和管理者线程竞争）：只用于记录退出的线程ID */
        {
          std::lock_guard<std::mutex> lock_ids(m_ids_mutex);
          std::println("---- 工作线程 {} 将要退出（doWork） ...", std::this_thread::get_id());
          m_ids.emplace_back(std::this_thread::get_id());
        }
        return;  // 让线程退出任务函数，即工作线程少了一个
      }
    }
    // 锁已释放，其他线程可以并发取任务
    if (task) {
      --m_idle_thread;
      ++m_active_task;  // 开始执行，计数加一

      task();  // 执行任务

      --m_active_task;  // 执行完毕，计数减一
      ++m_idle_thread;

      // 执行完后检查：队列空且无活跃任务，才真正完成
      if (m_tasks.empty() && m_active_task.load() == 0) {
        m_done_condition.notify_all();
      }
    }
  }
}
