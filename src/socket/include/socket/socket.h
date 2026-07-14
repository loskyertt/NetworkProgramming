/**
 * @File    :   socket/socket.h
 * @Time    :   2026/04/14 15:42:35
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   对 socket 的封装
 */

#pragma once

#include <sys/types.h>
#include <cstddef>
#include <cstdint>
#include <string>

namespace sky {
namespace socket {
class Socket {
 protected:
  std::string m_ip;  // 监听/连接的 IP 地址
  uint16_t m_port;   // 监听/连接的端口号
  int m_sockfd;      // 套接字文件描述符

  bool m_is_release = false;  // 是否释放 m_sockfd 的所有权

 public:
  /* 创建 socket_fd */
  Socket();
  /* 使用已有的 socket_fd */
  Socket(int sockfd);
  virtual ~Socket();

  /**
   * @brief 绑定地址
   *
   * - @param ip 绑定的 IP 地址
   * - @param port 绑定的端口号
   * - @return true 绑定成功；false 绑定失败
   */
  bool bind(const std::string &ip, uint16_t port);

  /**
   * @brief 监听连接
   *
   * - @param backlog 最大待处理连接数，即连接请求队列（全连接队列）的长度
   * - @return true 监听成功；false 监听失败
   */
  bool listen(int backlog = 128);

  /**
   * @brief 客户端使用，连接服务器
   *
   * - @param ip 服务器 IP 地址
   * - @param port 服务器端口号
   * - @return true 连接成功；false 连接失败
   */
  bool connect(const std::string &ip, uint16_t port);

  /**
   * @brief 接受客户端连接
   *
   * @return int 成功时返回一个新的文件描述符 connfd（代表与客户端的连接），失败时返回 -1
   */
  int accept();

  /**
   * @brief 向客户端/服务端发送数据
   *
   * - @param buf 指向待发送数据缓冲区的指针
   * - @param len 要发送的数据字节数
   *
   * @return ssize_t
   * - 成功时返回实际发送的字节数（可能小于请求发送的字节数）；
   * - 失败时返回 -1 并设置 errno
   */
  ssize_t send(const void *buf, size_t len);

  /**
   * @brief 从客户端/服务端接收数据
   *
   * - @param buf 指向接收缓冲区的指针，用于存放接收到的数据
   * - @param len 缓冲区最大容量，即最多接收的字节数
   *
   * @return ssize_t
   * - 成功时返回接收到的字节数（可能小于请求的 len）；
   * - 连接关闭时返回 0；
   * - 出错时返回 -1
   */
  ssize_t recv(void *buf, size_t len);

  /**
   * @brief 关闭 socket
   */
  void close();

  /**
   * @brief 设置为非阻塞 I/O
   * @details 作用对象：listen_fd 和 conn_fd，在 I/O 多路复用中，两者都要设置
   * @return true 设置成功；false 设置失败
   */
  bool setNonBlocking();

  /**
   * @brief 设置发送缓冲区大小
   * @details 作用对象：listen_fd 和 conn_fd。由于可继承特性，如果你希望所有新连接都拥有特定的缓冲区大小，必须对 listen_fd 进行设置，且必须在 listen(...) 调用之前完成。
   *
   * @param size 缓冲区大小
   * @return true 设置成功；false 设置失败
   */
  bool setSendBufferSize(size_t size);

  /**
   * @brief 设置接收缓冲区大小
   * @details 作用对象：listen_fd 和 conn_fd。由于可继承特性，如果你希望所有新连接都拥有特定的缓冲区大小，必须对 listen_fd 进行设置，且必须在 listen() 调用之前完成。
   *
   * @param size 缓冲区大小
   * @return true 设置成功；false 设置失败
   */
  bool setReceiveBufferSize(size_t size);

  /**
   * @brief 设置 linger
   * @details 作用对象：conn_fd
   *
   * - @param active 是否启用 linger
   * - @param seconds linger 时间（秒），当设置为 0 时，会关闭 TIME_WAIT 状态
   * - @return true 设置成功；false 设置失败
   */
  bool setLinger(bool active, int seconds);

  /**
   * @brief 设置 keepalive
   * @details 作用对象：conn_fd
   * @return true 设置成功；false 设置失败
   */
  bool setKeepAlive();

  /**
   * @brief 设置 reuse address
   * @details 作用对象：listen_fd，必须在 bind(...) 之前设置
   * @return true 设置成功；false 设置失败
   */
  bool setReuseAddress();

  // getters and setters
 public:
  int getSockFd() const { return m_sockfd; }

  /**
   * @brief 让出 m_sockfd 的所有权，即 m_sockfd 不再由析构函数释放
   */
  void setRelease() { m_is_release = true; }
};
}  // namespace socket
}  // namespace sky
