/**
 * @File    :   examples/example09.cpp
 * @Time    :   2026/04/20 18:40:16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   条件变量：生产者-消费者模型
 */

#include <print>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

std::queue<int> q;           // 缓冲区：生产者写入，消费者读取
std::mutex mtx;              // 互斥锁：保护 q 的并发访问
std::condition_variable cv;  // 条件变量：协调两个线程的执行时序

void producer(const std::string &name) {
  for (int i = 1; i <= 5; ++i) {
    {
      std::lock_guard<std::mutex> lock(mtx);  // ① 加锁
      q.push(i);                              // ② 写入数据
      std::println("Producer {} produced: {}", name, i);
    }  // ③ lock_guard 析构，自动解锁

    cv.notify_one();  // ④ 解锁之后再通知消费者

    std::this_thread::sleep_for(  // ⑤ 模拟生产耗时
        std::chrono::milliseconds(500));
  }
}

void consumer(const std::string &name) {
  while (true) {
    std::unique_lock<std::mutex> lock(mtx);  // ① 加锁

    cv.wait(lock, [] { return !q.empty(); });  // ② 核心：等待条件

    int val = q.front();  // ③ 取出数据
    q.pop();

    std::println("Consumer {} consumed: {}", name, val);

    if (val == 5)
      break;  // ④ 取到 5 就退出
  }
  // ⑤ unique_lock 析构，自动解锁
}

int main() {
  std::thread producer_thread1(producer, "Producer 1");  // 生产者 1
  std::thread consumer_thread1(consumer, "Consumer 1");  // 消费者 1

  producer_thread1.join();
  consumer_thread1.join();
}
