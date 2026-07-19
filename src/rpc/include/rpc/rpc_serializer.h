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

/**
 * RPC 二进制序列化器。
 *
 * 写入接口会将数据追加到内部缓冲区；读取接口会从当前读位置取出数据，
 * 并推进读位置。Stub 和 handler 必须严格保持相同的写入/读取顺序，否则
 * 会造成字段错位或读取越界。
 *
 * 多字节数值当前直接按主机小端序写入，没有做网络字节序转换，因此该
 * 序列化器只适合同字节序机器之间互通。
 */
class RpcSerializer {
 private:
  std::vector<char> m_buffer;
  size_t m_read_pos = 0;

  void ensure_readable(size_t n) const;

 public:
  RpcSerializer() = default;
  ~RpcSerializer() = default;

  // ============ 序列化（写入） ============

  /**
   * 写入一个 8 位有符号整数。
   *
   * @param v 待写入的整数值。
   */
  void write_int8(int8_t v);

  /**
   * 写入一个 32 位有符号整数。
   *
   * @param v 待写入的整数值，编码后占 4 字节。
   */
  void write_int32(int32_t v);

  /**
   * 写入一个 32 位无符号整数。
   *
   * @param v 待写入的整数值，编码后占 4 字节。
   */
  void write_uint32(uint32_t v);

  /**
   * 写入一个 64 位有符号整数。
   *
   * @param v 待写入的整数值，编码后占 8 字节。
   */
  void write_int64(int64_t v);

  /**
   * 写入一个双精度浮点数。
   *
   * @param v 待写入的浮点值，编码后占 8 字节。
   */
  void write_double(double v);

  /**
   * 写入一个布尔值。
   *
   * @param v 待写入的布尔值；true 编码为 1，false 编码为 0。
   */
  void write_bool(bool v);

  /**
   * 写入一个字符串。
   *
   * 由于字符串是边长的，因此编码格式为 uint32_t(字符串长度) + 原始字符字节，字符串内容不追加结尾 \0。
   *
   * @param v 待写入的字符串。
   * @throws std::length_error 当字符串长度超过 uint32_t 可表示范围时抛出。
   */
  void write_string(const std::string &v);

  /**
   * 写入一段原始字节。
   *
   * @param data 指向待写入字节的缓冲区；当 len > 0 时不能为 nullptr。
   * @param len 待写入的字节数；为 0 时该函数不写入任何内容。
   * @throws std::invalid_argument 当 len > 0 且 data == nullptr 时抛出。
   */
  void write_raw(const char *data, size_t len);

  /**
   * 获取当前序列化缓冲区。
   *
   * @return 内部字节缓冲区的只读引用。引用在下一次修改缓冲区后可能失效。
   */
  const std::vector<char> &data() const { return m_buffer; }

  /**
   * 清空内部缓冲区并重置读位置。
   */
  void clear();

  // ============ 反序列化（读取） ============

  /**
   * 绑定一段待反序列化的原始字节。
   *
   * 调用后内部缓冲区会复制 [data, data + len)，读位置重置为起点。
   *
   * @param data 指向待读取字节的缓冲区。
   * @param len 待读取字节数。
   */
  void reset(const char *data, size_t len);

  /**
   * 绑定一段待反序列化的字节数组。
   *
   * 调用后内部缓冲区会复制 data，读位置重置为起点。
   *
   * @param data 待读取的字节数组。
   */
  void reset(const std::vector<char> &data);

  /**
   * 读取一个 8 位有符号整数。
   *
   * @return 读取到的整数值。
   * @throws std::out_of_range 当剩余字节不足 1 字节时抛出。
   */
  int8_t read_int8();

  /**
   * 读取一个 32 位有符号整数。
   *
   * @return 读取到的整数值。
   * @throws std::out_of_range 当剩余字节不足 4 字节时抛出。
   */
  int32_t read_int32();

  /**
   * 读取一个 32 位无符号整数。
   *
   * @return 读取到的整数值。
   * @throws std::out_of_range 当剩余字节不足 4 字节时抛出。
   */
  uint32_t read_uint32();

  /**
   * 读取一个 64 位有符号整数。
   *
   * @return 读取到的整数值。
   * @throws std::out_of_range 当剩余字节不足 8 字节时抛出。
   */
  int64_t read_int64();

  /**
   * 读取一个双精度浮点数。
   *
   * @return 读取到的浮点值。
   * @throws std::out_of_range 当剩余字节不足 8 字节时抛出。
   */
  double read_double();

  /**
   * 读取一个布尔值。
   *
   * @return 当前字节非 0 时返回 true，否则返回 false。
   * @throws std::out_of_range 当剩余字节不足 1 字节时抛出。
   */
  bool read_bool();

  /**
   * 读取一个字符串。
   *
   * 读取格式必须与 write_string 对应：先读取 4 字节长度，再读取指定
   * 长度的字符串内容。
   *
   * @return 读取到的字符串。
   * @throws std::out_of_range 当剩余字节不足长度字段或字符串内容时抛出。
   */
  std::string read_string();

  /**
   * 获取当前缓冲区中还未读取的字节数。
   *
   * @return m_buffer.size() - m_read_pos。
   */
  size_t remaining() const { return m_buffer.size() - m_read_pos; }
};

}  // namespace rpc
}  // namespace sky
