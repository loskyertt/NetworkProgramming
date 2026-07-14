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

/**
 * @brief RPC 请求消息。
 *
 * @details 该结构体是协议头之外的逻辑请求表示。发送时会被编码为：RpcHeader + service_name + method_name + payload。
 */
struct RpcRequest {
  uint32_t call_id = 0;       // 调用 ID。由客户端通道生成，服务端响应时原样回填
  std::string service_name;   // 服务名，例如 Calculator。不能为空
  std::string method_name;    // 方法名，例如 add。不能为空
  std::vector<char> payload;  // 已序列化的调用参数。字段内容由具体 Stub 和 handler 约定
};

/**
 * @brief RPC 响应消息。
 *
 * @details 该结构体是协议头之外的逻辑响应表示。发送时会被编码为：
 * RpcHeader + payload。响应中不携带服务名和方法名。
 */
struct RpcResponse {
  uint32_t call_id = 0;       // 调用 ID。必须与对应请求的 call_id 一致
  uint8_t status = 0;         // 响应状态码，取值见 RpcStatus。
  std::vector<char> payload;  // 已序列化的返回值或错误信息。错误信息当前约定为序列化字符串
};

/**
 * @brief 向文件描述符发送一个完整 RPC 请求。
 *
 * @details 函数会依次写入协议头、服务名、方法名和 payload，并循环调用 send 直到所有字节写完或发生错误。
 *
 * @param fd 已连接的 socket 文件描述符。
 * @param req 待发送的 RPC 请求；service_name 和 method_name 必须非空，body 总大小不能超过 RPC_MAX_BODY_SIZE。
 * @return 全部字节发送成功返回 true；连接断开、写入失败或长度非法返回 false。
 */
bool sendRequest(int fd, const RpcRequest &req);

/**
 * @brief 向文件描述符发送一个完整 RPC 响应。
 *
 * @details 函数会写入协议头和 payload，并循环调用 send 直到所有字节写完或发生错误。响应中的服务名和方法名长度字段会被编码为 0。
 *
 * @param fd 已连接的 socket 文件描述符。
 * @param resp 待发送的 RPC 响应；payload 大小不能超过 RPC_MAX_BODY_SIZE。
 * @return 全部字节发送成功返回 true；连接断开、写入失败或长度非法返回 false。
 */
bool sendResponse(int fd, const RpcResponse &resp);

/**
 * @brief 从文件描述符读取一个完整 RPC 请求。
 *
 * @details 函数先读满固定长度 RpcHeader，校验魔数、版本、消息类型和 body 大小，再按照 svc_len、meth_len、payload_len 读满剩余 body。
 * 该函数用于处理 TCP 半包场景，不会在消息未完整时提前返回成功。
 *
 * @param fd 已连接的 socket 文件描述符。
 * @param req 输出参数；读取成功后填入调用 ID、服务名、方法名和 payload。
 * @return 成功读取并校验完整请求返回 true；连接关闭、读取失败或 header 校验失败返回 false。
 */
bool recvRequest(int fd, RpcRequest &req);

/**
 * @brief 从文件描述符读取一个完整 RPC 响应。
 *
 * @details 函数先读满固定长度 RpcHeader，校验魔数、版本、消息类型和 body 大小，再按照 payload_len 读满响应 payload。
 * 响应 header 中 svc_len 与 meth_len 必须为 0。
 *
 * @param fd 已连接的 socket 文件描述符。
 * @param resp 输出参数；读取成功后填入调用 ID、状态码和 payload。
 * @return 成功读取并校验完整响应返回 true；连接关闭、读取失败或 header 校验失败返回 false。
 */
bool recvResponse(int fd, RpcResponse &resp);

}  // namespace rpc
}  // namespace sky
