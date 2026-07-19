/**
 * @File    :   src/rpc/include/rpc/rpc_protocol.h
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   RPC 协议定义：Header 结构体、常量、枚举类型
 *
 * 协议格式：定长 Header + 变长 Body
 *
 * Request:
 *   Header + ServiceName (svc_len) + MethodName (meth_len) + Payload (payload_len)
 *
 * Response:
 *   Header + Payload (payload_len)
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace sky {
namespace rpc {

#pragma pack(push, 1)  // 禁止字节对齐

/**
 * RPC 二进制协议头。
 *
 * 该结构体会被直接按字节写入 socket，因此必须保持固定布局。
 * #pragma pack(push, 1) 用于禁止编译器在字段之间插入 padding，使 sizeof(RpcHeader) 固定为 23 字节。
 *
 * 请求包 body 由 ServiceName + MethodName + Payload 组成；
 * 响应包 body 只包含 Payload，因此响应中的 svc_len 和 meth_len 必须为 0。
 */
struct RpcHeader {
  uint32_t magic;        // 协议魔数，固定为 k_rpc_magic，用于识别 RPC 数据包
  uint8_t version;       // 协议版本号，当前固定为 k_rpc_version
  uint8_t msg_type;      // 消息类型，取值见 RpcMsgType
  uint8_t status;        // 响应状态码，取值见 RpcStatus；请求中固定为 0
  uint32_t call_id;      // 调用 ID，用于将客户端请求与服务端响应配对
  uint32_t svc_len;      // 服务名长度，单位为字节；仅请求使用，响应中必须为 0
  uint32_t meth_len;     // 方法名长度，单位为字节；仅请求使用，响应中必须为 0
  uint32_t payload_len;  // 消息的业务载荷长度，单位为字节
};

#pragma pack(pop)

/**
 * @brief RPC 协议头的固定字节数。
 *
 * @return sizeof(RpcHeader)，当前为 23 字节。
 */
constexpr size_t k_rpc_header_size = sizeof(RpcHeader);

/**
 * @brief 单个请求或响应 body 允许的最大字节数。
 *
 * @details 请求 body 大小按 svc_len + meth_len + payload_len 计算，响应 body 大小按 payload_len 计算。
 * 该限制用于拒绝异常长度字段，避免接收方因恶意或损坏的 header 一次性分配过大内存。
 */
constexpr uint32_t k_rpc_max_body_size = 16 * 1024 * 1024;

/**
 * @brief 魔数，"RPC" 的 ASCII 值
 *
 * @details 四个数字相“或” -> 0x52504330
 * - 'R' = 82 = 0x52
 * - 'P' = 80 = 0x50
 * - 'C' = 67 = 0x43
 * - '0' = 48 = 0x30
 */
constexpr uint32_t k_rpc_magic =
    ('R' << 24) |  // 将 0x52 左移 24 位，移位后，二进制表示为：0101 0010 0000 0000 0000 0000 0000 0000
    ('P' << 16) |  // 将 0x50 左移 16 位
    ('C' << 8) |   // 同上
    ('0');

/**
 * @brief 当前 RPC 协议版本号。
 *
 * @details 协议字段含义或编码方式发生不兼容变化时，应提升版本号并在收包端做兼容性判断。
 */
constexpr uint8_t k_rpc_version = 1;

/**
 * @brief RPC 消息类型。
 */
enum class RpcMsgType : uint8_t {
  REQUEST  = 0,  // 客户端发往服务端的调用请求
  RESPONSE = 1,  // 服务端发回客户端的调用响应
};

/**
 * @brief RPC 响应状态码。
 */
enum class RpcStatus : uint8_t {
  OK          = 0,  // 调用成功，payload 为业务返回值
  ERROR       = 1,  // 网络失败、handler 异常或其他通用错误
  NOT_FOUND   = 2,  // 服务名或方法名没有注册对应 handler
  BAD_REQUEST = 3,  // 请求格式错误，当前作为预留状态
};

/**
 * @brief 将响应状态码转换为可读字符串。
 *
 * @param status 原始状态码字节，通常来自 RpcResponse::status。
 * @return 状态码对应的字符串；未知状态码返回 "UNKNOWN"。
 */
inline std::string_view status_to_string(uint8_t status) {
  switch (static_cast<RpcStatus>(status)) {
    case RpcStatus::OK:
      return "OK";
    case RpcStatus::ERROR:
      return "ERROR";
    case RpcStatus::NOT_FOUND:
      return "NOT_FOUND";
    case RpcStatus::BAD_REQUEST:
      return "BAD_REQUEST";
    default:
      return "UNKNOWN";
  }
}

}  // namespace rpc
}  // namespace sky
