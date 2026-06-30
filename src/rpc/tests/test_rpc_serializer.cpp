#include "rpc/rpc_serializer.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <string>

using sky::rpc::RpcSerializer;

TEST(RpcSerializerTest, RoundTripSupportedTypes) {
  RpcSerializer writer;
  writer.writeInt8(-8);
  writer.writeInt32(-123456);
  writer.writeUint32(123456u);
  writer.writeInt64(-1234567890123LL);
  writer.writeDouble(3.1415926);
  writer.writeBool(true);
  writer.writeBool(false);
  writer.writeString("hello rpc");

  RpcSerializer reader;
  reader.reset(writer.data());
  EXPECT_EQ(reader.readInt8(), -8);
  EXPECT_EQ(reader.readInt32(), -123456);
  EXPECT_EQ(reader.readUint32(), 123456u);
  EXPECT_EQ(reader.readInt64(), -1234567890123LL);
  EXPECT_DOUBLE_EQ(reader.readDouble(), 3.1415926);
  EXPECT_TRUE(reader.readBool());
  EXPECT_FALSE(reader.readBool());
  EXPECT_EQ(reader.readString(), "hello rpc");
  EXPECT_EQ(reader.remaining(), 0u);
}

TEST(RpcSerializerTest, HandlesEmptyAndBinaryString) {
  std::string binary("a\0b", 3);

  RpcSerializer writer;
  writer.writeString("");
  writer.writeString(binary);

  RpcSerializer reader;
  reader.reset(writer.data());
  EXPECT_EQ(reader.readString(), "");
  EXPECT_EQ(reader.readString(), binary);
  EXPECT_EQ(reader.remaining(), 0u);
}

TEST(RpcSerializerTest, ClearResetsBufferAndReadPosition) {
  RpcSerializer serializer;
  serializer.writeInt32(42);

  serializer.clear();
  EXPECT_TRUE(serializer.data().empty());
  EXPECT_EQ(serializer.remaining(), 0u);

  serializer.writeInt32(7);
  EXPECT_EQ(serializer.readInt32(), 7);
  EXPECT_EQ(serializer.remaining(), 0u);
}

TEST(RpcSerializerTest, ThrowsWhenReadingPastEnd) {
  RpcSerializer serializer;
  serializer.writeInt32(1);
  serializer.readInt32();

  EXPECT_THROW(serializer.readInt8(), std::out_of_range);
}

TEST(RpcSerializerTest, ThrowsOnNullRawDataWithPositiveLength) {
  RpcSerializer serializer;
  EXPECT_THROW(serializer.writeRaw(nullptr, 1), std::invalid_argument);
  EXPECT_NO_THROW(serializer.writeRaw(nullptr, 0));
}
