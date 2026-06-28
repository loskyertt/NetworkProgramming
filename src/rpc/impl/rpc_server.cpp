/**
 * @File    :   src/rpc/impl/rpc_server.cpp
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   RPC 服务端实现
 *
 * 事件循环流程：
 *   1. epoll_wait 等待事件
 *   2. listen_fd 就绪 → accept() 新连接 → 将 conn_fd 加入 epoll（LT，阻塞模式）
 *   3. conn_fd 可读 → 读取完整请求 → 从 epoll 移除 → 提交到线程池处理
 *   4. 线程池中：查 Handler → 执行 → 序列化结果 → send 响应 → 关闭连接
 *
 * 为什么 conn_fd 使用阻塞模式 + LT：
 *   我们每次读完整消息（header + body）后才处理，不存在 ET 模式下的数据残留问题。
 *   阻塞模式简化了 recvFull 的循环逻辑。
 */

#include "rpc/rpc_server.h"
#include "rpc/rpc_protocol.h"
#include "rpc/rpc_message.h"
#include "rpc/rpc_serializer.h"
#include "socket/server_socket.h"
#include "socket/epoll/epoller.h"
#include "thread_pool/thread_pool.h"
#include "logger/logger.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <mutex>

namespace sky {
namespace rpc {

// ============ 构造 / 析构 ============

RpcServer::RpcServer(const std::string &ip, uint16_t port)
    : m_server(new socket::ServerSocket(ip, port))
    , m_epoller(new socket::EPoller())
    , m_pool(new thread::ThreadPool()) {
}

RpcServer::~RpcServer() {
    stop();
    delete m_pool;
    delete m_epoller;
    delete m_server;
}

// ============ Handler 注册 ============

void RpcServer::registerHandler(const std::string &service,
                                const std::string &method,
                                RpcHandler handler) {
    std::string key = service + "." + method;
    std::lock_guard<std::mutex> lock(m_handlers_mutex);
    m_handlers[key] = std::move(handler);
    Log_info("RPC handler registered: %s", key.c_str());
}

// ============ 请求处理 ============

void RpcServer::handleRequest(int conn_fd, const RpcRequest &req) {
    std::string key = req.service_name + "." + req.method_name;
    Log_info("RPC request: %s (call_id=%u)", key.c_str(), req.call_id);

    RpcResponse resp;
    resp.call_id = req.call_id;

    RpcHandler handler;
    {
        std::lock_guard<std::mutex> lock(m_handlers_mutex);
        auto it = m_handlers.find(key);
        if (it != m_handlers.end()) {
            handler = it->second;
        }
    }

    if (!handler) {
        Log_warn("Handler not found: %s", key.c_str());
        resp.status = static_cast<uint8_t>(RpcStatus::NOT_FOUND);
        RpcSerializer writer;
        writer.writeString("handler not found: " + key);
        resp.payload = writer.data();
        sendResponseToConn(conn_fd, resp);
        return;
    }

    // 调用 Handler
    try {
        resp.payload = handler(req.payload);
        resp.status = static_cast<uint8_t>(RpcStatus::OK);
    } catch (const std::exception &e) {
        Log_error("Handler exception: %s : %s", key.c_str(), e.what());
        resp.status = static_cast<uint8_t>(RpcStatus::ERROR);
        RpcSerializer writer;
        writer.writeString(e.what());
        resp.payload = writer.data();
    } catch (...) {
        Log_error("Handler unknown exception: %s", key.c_str());
        resp.status = static_cast<uint8_t>(RpcStatus::ERROR);
        RpcSerializer writer;
        writer.writeString("unknown handler exception");
        resp.payload = writer.data();
    }

    sendResponseToConn(conn_fd, resp);
}

void RpcServer::sendResponseToConn(int conn_fd, const RpcResponse &resp) {
    bool ok = sendResponse(conn_fd, resp);
    if (!ok) {
        Log_error("Failed to send response to fd=%d", conn_fd);
    }
}

// ============ 启动 / 停止 ============

void RpcServer::start() {
    // 创建 epoll
    if (!m_epoller->create(1024)) {
        Log_error("Failed to create epoll");
        return;
    }

    int listen_fd = m_server->getSockFd();

    // 将 listen_fd 加入 epoll（LT 模式）
    if (!m_epoller->setFd(listen_fd, EPOLLIN)) {
        Log_error("Failed to add listen_fd to epoll");
        return;
    }

    m_running = true;
    Log_info("RpcServer started on fd=%d", listen_fd);

    // 主事件循环
    while (m_running) {
        int ready_count = m_epoller->epoll(100);  // 100ms 超时，便于响应 stop
        if (ready_count < 0) {
            if (errno == EINTR) continue;
            Log_error("epoll_wait error: errno=%d errmsg=%s", errno, strerror(errno));
            break;
        }

        for (int i = 0; i < ready_count; ++i) {
            int current_fd = m_epoller->getFd(i);

            // 新连接
            if (current_fd == listen_fd) {
                int conn_fd = m_server->accept();
                if (conn_fd < 0) {
                    Log_error("accept error: errno=%d errmsg=%s", errno, strerror(errno));
                    continue;
                }
                Log_info("New connection: fd=%d", conn_fd);

                // 新连接加入 epoll（LT 模式，阻塞读取）
                socket::Socket conn_sock(conn_fd);
                conn_sock.setRelease();  // 避免析构 close(fd)
                if (!m_epoller->setFd(conn_fd, EPOLLIN)) {
                    Log_error("Failed to add conn_fd=%d to epoll", conn_fd);
                    ::close(conn_fd);
                    continue;
                }
            } else {
                // 读取请求，读取成功后立即从 epoll 移除，避免重复触发
                socket::Socket conn_sock(current_fd);
                conn_sock.setRelease();

                RpcRequest req;
                bool ok = recvRequest(current_fd, req);

                if (!ok) {
                    // 连接断开或出错
                    Log_info("Client disconnected: fd=%d", current_fd);
                    m_epoller->deleteFd(current_fd);
                    ::close(current_fd);
                    continue;
                }

                // 从 epoll 移除（请求已读取，不再监听此连接的事件）
                m_epoller->deleteFd(current_fd);

                // 提交到线程池处理（处理完成后关闭连接）
                m_pool->addTask([this, current_fd, req = std::move(req)]() {
                    this->handleRequest(current_fd, req);
                    ::close(current_fd);
                });
            }
        }
    }

    Log_info("RpcServer stopped");
}

void RpcServer::stop() {
    m_running = false;
}

}  // namespace rpc
}  // namespace sky
