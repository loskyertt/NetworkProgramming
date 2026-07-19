#include "rpc/rpc_serializer.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <string>

using sky::rpc::RpcSerializer;

TEST(RpcSerializerTest, RoundTripSupportedTypes) {
  RpcSerializer writer;
  writer.write_int8(-8);
  writer.write_int32(-123456);
  writer.write_uint32(123456u);
  writer.write_int64(-1234567890123LL);
  writer.write_double(3.1415926);
  writer.write_bool(true);
  writer.write_bool(false);
  writer.write_string("hello rpc");

  RpcSerializer reader;
  reader.reset(writer.data());
  EXPECT_EQ(reader.read_int8(), -8);
  EXPECT_EQ(reader.read_int32(), -123456);
  EXPECT_EQ(reader.read_uint32(), 123456u);
  EXPECT_EQ(reader.read_int64(), -1234567890123LL);
  EXPECT_DOUBLE_EQ(reader.read_double(), 3.1415926);
  EXPECT_TRUE(reader.read_bool());
  EXPECT_FALSE(reader.read_bool());
  EXPECT_EQ(reader.read_string(), "hello rpc");
  EXPECT_EQ(reader.remaining(), 0u);
}

TEST(RpcSerializerTest, HandlesEmptyAndBinaryString) {
  std::string binary("a\0b", 3);

  RpcSerializer writer;
  writer.write_string("");
  writer.write_string(binary);

  RpcSerializer reader;
  reader.reset(writer.data());
  EXPECT_EQ(reader.read_string(), "");
  EXPECT_EQ(reader.read_string(), binary);
  EXPECT_EQ(reader.remaining(), 0u);
}

TEST(RpcSerializerTest, ClearResetsBufferAndReadPosition) {
  RpcSerializer serializer;
  serializer.write_int32(42);

  serializer.clear();
  EXPECT_TRUE(serializer.data().empty());
  EXPECT_EQ(serializer.remaining(), 0u);

  serializer.write_int32(7);
  EXPECT_EQ(serializer.read_int32(), 7);
  EXPECT_EQ(serializer.remaining(), 0u);
}

TEST(RpcSerializerTest, ThrowsWhenReadingPastEnd) {
  RpcSerializer serializer;
  serializer.write_int32(1);
  serializer.read_int32();

  EXPECT_THROW(serializer.read_int8(), std::out_of_range);
}

TEST(RpcSerializerTest, ThrowsOnNullRawDataWithPositiveLength) {
  RpcSerializer serializer;
  EXPECT_THROW(serializer.write_raw(nullptr, 1), std::invalid_argument);
  EXPECT_NO_THROW(serializer.write_raw(nullptr, 0));
}
