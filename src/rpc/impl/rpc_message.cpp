/**
 * @File    :   src/rpc/impl/rpc_message.cpp
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   RPC 消息收发实现
 *
 * 核心逻辑：TCP 是流协议，recv 可能分片到达，必须循环读
 *   先读满 RPC_HEADER_SIZE 字节 → 解析 Header → 再读满 Body
 *   Header.payload_len 提供了消息定界
 */

#include "rpc/rpc_protocol.h"
#include "rpc/rpc_message.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstddef>
#include <cstring>
#include <limits>
#include <new>

namespace sky {
namespace rpc {

/**
 * @brief 从 fd 循环读取 n 字节（保证读满或出错）
 * @return true 读取成功；false 读取失败或连接关闭
 */
static bool recvFull(int fd, char *buf, size_t n) {
    size_t total = 0;
    while (total < n) {
        ssize_t nread = ::recv(fd, buf + total, n - total, 0);
        if (nread <= 0) {
            return false;  // 连接关闭或出错
        }
        total += static_cast<size_t>(nread);
    }
    return true;
}

static bool hasValidBodySize(const RpcHeader &header) {
    uint64_t body_size = static_cast<uint64_t>(header.svc_len) +
                         static_cast<uint64_t>(header.meth_len) +
                         static_cast<uint64_t>(header.payload_len);
    return body_size <= RPC_MAX_BODY_SIZE;
}

static bool hasValidHeader(const RpcHeader &header, RpcMsgType expected_type) {
    return header.magic == RPC_MAGIC &&
           header.version == RPC_VERSION &&
           header.msg_type == static_cast<uint8_t>(expected_type) &&
           hasValidBodySize(header);
}

/**
 * @brief 向 fd 发送全部 n 字节（循环写保证全量发送）
 * @return true 发送成功；false 发送失败
 */
static bool sendFull(int fd, const char *buf, size_t n) {
    size_t total = 0;
    while (total < n) {
        ssize_t nwritten = ::send(fd, buf + total, n - total, 0);
        if (nwritten <= 0) {
            return false;
        }
        total += static_cast<size_t>(nwritten);
    }
    return true;
}

// ---- Request 收发 ----

bool sendRequest(int fd, const RpcRequest &req) {
    if (req.service_name.size() > std::numeric_limits<uint32_t>::max() ||
        req.method_name.size() > std::numeric_limits<uint32_t>::max() ||
        req.payload.size() > std::numeric_limits<uint32_t>::max()) {
        return false;
    }

    RpcHeader header;
    std::memset(&header, 0, sizeof(header));
    header.magic       = RPC_MAGIC;
    header.version     = RPC_VERSION;
    header.msg_type    = static_cast<uint8_t>(RpcMsgType::REQUEST);
    header.call_id     = req.call_id;
    header.svc_len     = static_cast<uint32_t>(req.service_name.size());
    header.meth_len    = static_cast<uint32_t>(req.method_name.size());
    header.payload_len = static_cast<uint32_t>(req.payload.size());

    if (!hasValidBodySize(header)) {
        return false;
    }

    // 发送 Header
    if (!sendFull(fd, reinterpret_cast<const char *>(&header), sizeof(header))) {
        return false;
    }
    // 发送 ServiceName
    if (header.svc_len > 0 && !sendFull(fd, req.service_name.data(), header.svc_len)) {
        return false;
    }
    // 发送 MethodName
    if (header.meth_len > 0 && !sendFull(fd, req.method_name.data(), header.meth_len)) {
        return false;
    }
    // 发送 Payload
    if (header.payload_len > 0 && !sendFull(fd, req.payload.data(), header.payload_len)) {
        return false;
    }
    return true;
}

bool recvRequest(int fd, RpcRequest &req) {
    RpcHeader header;
    if (!recvFull(fd, reinterpret_cast<char *>(&header), sizeof(header))) {
        return false;
    }

    // 校验 Header，避免把非 RPC 数据或异常长度当作正常请求处理。
    if (!hasValidHeader(header, RpcMsgType::REQUEST)) {
        return false;
    }

    req.call_id       = header.call_id;
    req.service_name  = std::string(header.svc_len, '\0');
    req.method_name   = std::string(header.meth_len, '\0');
    req.payload.resize(header.payload_len);

    // 读取 ServiceName
    if (header.svc_len > 0 && !recvFull(fd, req.service_name.data(), header.svc_len)) {
        return false;
    }
    // 读取 MethodName
    if (header.meth_len > 0 && !recvFull(fd, req.method_name.data(), header.meth_len)) {
        return false;
    }
    // 读取 Payload
    if (header.payload_len > 0 && !recvFull(fd, req.payload.data(), header.payload_len)) {
        return false;
    }
    return true;
}

// ---- Response 收发 ----

bool sendResponse(int fd, const RpcResponse &resp) {
    if (resp.payload.size() > std::numeric_limits<uint32_t>::max()) {
        return false;
    }

    RpcHeader header;
    std::memset(&header, 0, sizeof(header));
    header.magic       = RPC_MAGIC;
    header.version     = RPC_VERSION;
    header.msg_type    = static_cast<uint8_t>(RpcMsgType::RESPONSE);
    header.status      = resp.status;
    header.call_id     = resp.call_id;
    header.payload_len = static_cast<uint32_t>(resp.payload.size());

    if (!hasValidBodySize(header)) {
        return false;
    }

    // 发送 Header
    if (!sendFull(fd, reinterpret_cast<const char *>(&header), sizeof(header))) {
        return false;
    }
    // 发送 Payload
    if (header.payload_len > 0 && !sendFull(fd, resp.payload.data(), header.payload_len)) {
        return false;
    }
    return true;
}

bool recvResponse(int fd, RpcResponse &resp) {
    RpcHeader header;
    if (!recvFull(fd, reinterpret_cast<char *>(&header), sizeof(header))) {
        return false;
    }

    // 校验 Header，确保客户端只接收 RPC 响应包。
    if (!hasValidHeader(header, RpcMsgType::RESPONSE)) {
        return false;
    }

    resp.call_id = header.call_id;
    resp.status  = header.status;
    resp.payload.resize(header.payload_len);

    // 读取 Payload
    if (header.payload_len > 0 && !recvFull(fd, resp.payload.data(), header.payload_len)) {
        return false;
    }
    return true;
}

}  // namespace rpc
}  // namespace sky
