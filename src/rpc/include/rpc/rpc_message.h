/**
 * @File    :   src/rpc/include/rpc/rpc_message.h
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   RPC 消息封装：RpcRequest / RpcResponse 结构体及 TCP 收发函数
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sky {
namespace rpc {

struct RpcRequest {
    uint32_t call_id = 0;
    std::string service_name;
    std::string method_name;
    std::vector<char> payload;  // 序列化后的参数
};

struct RpcResponse {
    uint32_t call_id = 0;
    uint8_t status = 0;         // 0=OK, 1=ERROR, 2=NOT_FOUND
    std::vector<char> payload;  // 序列化后的返回值
};

/**
 * @brief 向 fd 发送一个 RPC 请求
 * @return true 发送成功；false 发送失败
 */
bool sendRequest(int fd, const RpcRequest &req);

/**
 * @brief 向 fd 发送一个 RPC 响应
 * @return true 发送成功；false 发送失败
 */
bool sendResponse(int fd, const RpcResponse &resp);

/**
 * @brief 从 fd 读取一个完整的 RPC 请求
 * @details 会循环读直到消息完整（TCP 流可能分片）
 * @return true 读取成功；false 读取失败或连接断开
 */
bool recvRequest(int fd, RpcRequest &req);

/**
 * @brief 从 fd 读取一个完整的 RPC 响应
 * @return true 读取成功；false 读取失败或连接断开
 */
bool recvResponse(int fd, RpcResponse &resp);

}  // namespace rpc
}  // namespace sky
