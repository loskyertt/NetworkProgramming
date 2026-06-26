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

struct RpcHeader {
  uint32_t magic;        // 魔数，标识 RPC 协议包：0x435052 ('RPC')
  uint8_t version;       // 协议版本号：1
  uint8_t msg_type;      // 消息类型：0 = 请求，1 = 响应
  uint8_t status;        // 响应状态：0 = OK，1 = 错误，2 = 方法未找到
  uint32_t call_id;      // 调用 ID，请求和响应配对使用
  uint32_t svc_len;      // 服务名长度（字节），仅请求使用
  uint32_t meth_len;     // 方法名长度（字节），仅请求使用
  uint32_t payload_len;  // 载荷长度（字节）
};

#pragma pack(pop)

// Header 定长
constexpr size_t RPC_HEADER_SIZE = sizeof(RpcHeader);

// 单个请求/响应 body 的最大字节数。该限制避免异常 header 导致一次性分配过大内存。
constexpr uint32_t RPC_MAX_BODY_SIZE = 16 * 1024 * 1024;

// 魔数，"RPC" 的 ASCII 值
/*
'R' = 82 = 0x52
'P' = 80 = 0x50
'C' = 67 = 0x43
'0' = 48 = 0x30
*/
constexpr uint32_t RPC_MAGIC =
    ('R' << 24) |  // 将 0x52 左移 24 位，移位后，二进制表示为：01010010 00000000 00000000 00000000
    ('P' << 16) |  // 将 0x50 左移 16 位
    ('C' << 8 ) |  // 同理
    ('0');

// 协议版本
constexpr uint8_t RPC_VERSION = 1;

// 消息类型
enum class RpcMsgType : uint8_t {
  REQUEST = 0,
  RESPONSE = 1,
};

// 响应状态码
enum class RpcStatus : uint8_t {
  OK = 0,
  ERROR = 1,
  NOT_FOUND = 2,
  BAD_REQUEST = 3,
};

inline std::string_view statusToString(uint8_t status) {
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
