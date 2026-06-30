/**
 * @File    :   src/rpc/impl/rpc_serializer.cpp
 * @Time    :   2026/06/16
 * @Author  :   loskyertt
 * @Github  :   https://github.com/loskyertt
 * @Desc    :   简易二进制序列化器实现
 *
 * 编码规则（小端序）：
 *   int8_t   → 1 字节
 *   int32_t  → 4 字节
 *   uint32_t → 4 字节
 *   int64_t  → 8 字节
 *   double   → 8 字节
 *   bool     → 1 字节 (0/1)
 *   string   → uint32_t(length) + content
 */

#include "rpc/rpc_serializer.h"

#include <cstring>
#include <limits>
#include <stdexcept>

namespace sky {
namespace rpc {

// ============ 序列化（写入） ============

void RpcSerializer::writeInt8(int8_t v) {
    m_buffer.push_back(static_cast<char>(v));
}

void RpcSerializer::writeInt32(int32_t v) {
    m_buffer.resize(m_buffer.size() + sizeof(v));
    std::memcpy(m_buffer.data() + m_buffer.size() - sizeof(v), &v, sizeof(v));
}

void RpcSerializer::writeUint32(uint32_t v) {
    m_buffer.resize(m_buffer.size() + sizeof(v));
    std::memcpy(m_buffer.data() + m_buffer.size() - sizeof(v), &v, sizeof(v));
}

void RpcSerializer::writeInt64(int64_t v) {
    m_buffer.resize(m_buffer.size() + sizeof(v));
    std::memcpy(m_buffer.data() + m_buffer.size() - sizeof(v), &v, sizeof(v));
}

void RpcSerializer::writeDouble(double v) {
    m_buffer.resize(m_buffer.size() + sizeof(v));
    std::memcpy(m_buffer.data() + m_buffer.size() - sizeof(v), &v, sizeof(v));
}

void RpcSerializer::writeBool(bool v) {
    m_buffer.push_back(v ? 1 : 0);
}

void RpcSerializer::writeString(const std::string &v) {
    if (v.size() > std::numeric_limits<uint32_t>::max()) {
        throw std::length_error("RpcSerializer: string too large");
    }
    uint32_t len = static_cast<uint32_t>(v.size());
    writeUint32(len);
    if (len > 0) {
        writeRaw(v.data(), len);
    }
}

void RpcSerializer::writeRaw(const char *data, size_t len) {
    if (len == 0) return;
    if (data == nullptr) {
        throw std::invalid_argument("RpcSerializer: null raw data");
    }
    m_buffer.resize(m_buffer.size() + len);
    std::memcpy(m_buffer.data() + m_buffer.size() - len, data, len);
}

void RpcSerializer::clear() {
    m_buffer.clear();
    m_read_pos = 0;
}

// ============ 反序列化（读取） ============

void RpcSerializer::reset(const char *data, size_t len) {
    m_buffer.assign(data, data + len);
    m_read_pos = 0;
}

void RpcSerializer::reset(const std::vector<char> &data) {
    m_buffer = data;
    m_read_pos = 0;
}

void RpcSerializer::ensureReadable(size_t n) const {
    if (m_read_pos + n > m_buffer.size()) {
        throw std::out_of_range("RpcSerializer: not enough data to read");
    }
}

int8_t RpcSerializer::readInt8() {
    ensureReadable(1);
    int8_t v = m_buffer[m_read_pos];
    m_read_pos += 1;
    return v;
}

int32_t RpcSerializer::readInt32() {
    ensureReadable(sizeof(int32_t));
    int32_t v;
    std::memcpy(&v, m_buffer.data() + m_read_pos, sizeof(v));
    m_read_pos += sizeof(v);
    return v;
}

uint32_t RpcSerializer::readUint32() {
    ensureReadable(sizeof(uint32_t));
    uint32_t v;
    std::memcpy(&v, m_buffer.data() + m_read_pos, sizeof(v));
    m_read_pos += sizeof(v);
    return v;
}

int64_t RpcSerializer::readInt64() {
    ensureReadable(sizeof(int64_t));
    int64_t v;
    std::memcpy(&v, m_buffer.data() + m_read_pos, sizeof(v));
    m_read_pos += sizeof(v);
    return v;
}

double RpcSerializer::readDouble() {
    ensureReadable(sizeof(double));
    double v;
    std::memcpy(&v, m_buffer.data() + m_read_pos, sizeof(v));
    m_read_pos += sizeof(v);
    return v;
}

bool RpcSerializer::readBool() {
    ensureReadable(1);
    bool v = (m_buffer[m_read_pos] != 0);
    m_read_pos += 1;
    return v;
}

std::string RpcSerializer::readString() {
    uint32_t len = readUint32();
    ensureReadable(len);
    std::string v(m_buffer.data() + m_read_pos, len);
    m_read_pos += len;
    return v;
}

}  // namespace rpc
}  // namespace sky
