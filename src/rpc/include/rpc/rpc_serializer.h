/**
 * @File    :   src/rpc/include/rpc/rpc_serializer.h
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   简易二进制序列化器
 *
 * 支持基本类型（int32_t, uint32_t, int64_t, double, bool, std::string）的
 * 序列化和反序列化。所有多字节数据采用小端序。
 */

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace sky {
namespace rpc {

class RpcSerializer {
 public:
  RpcSerializer() = default;
  ~RpcSerializer() = default;

  // ============ 序列化（写入） ============

  void writeInt8(int8_t v);
  void writeInt32(int32_t v);
  void writeUint32(uint32_t v);
  void writeInt64(int64_t v);
  void writeDouble(double v);
  void writeBool(bool v);
  void writeString(const std::string &v);

  // 写入原始字节
  void writeRaw(const char *data, size_t len);

  /** 获取序列化后的数据 */
  const std::vector<char> &data() const { return m_buffer; }

  /** 清空缓冲区 */
  void clear();

  // ============ 反序列化（读取） ============

  /** 绑定待反序列化的数据 */
  void reset(const char *data, size_t len);
  void reset(const std::vector<char> &data);

  int8_t   readInt8();
  int32_t  readInt32();
  uint32_t readUint32();
  int64_t  readInt64();
  double   readDouble();
  bool     readBool();
  std::string readString();

  /** 获取剩余可读字节数 */
  size_t remaining() const { return m_buffer.size() - m_read_pos; }

 private:
  std::vector<char> m_buffer;
  size_t m_read_pos = 0;

  void ensureReadable(size_t n) const;
};

}  // namespace rpc
}  // namespace sky
