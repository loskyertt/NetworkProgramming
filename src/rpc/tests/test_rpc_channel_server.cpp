#include "rpc/rpc_channel.h"
#include "rpc/rpc_protocol.h"
#include "rpc/rpc_serializer.h"
#include "rpc/rpc_server.h"
#include "logger/logger.h"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace sky::rpc;

namespace {

constexpr uint16_t kTestPort = 18080;

void waitForServerReady() {
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

std::string decodeErrorPayload(const RpcResponse &resp) {
  RpcSerializer reader;
  reader.reset(resp.payload);
  return reader.readString();
}

}  // namespace

TEST(RpcChannelServerTest, CallsRegisteredHandlerAndReportsErrors) {
  std::filesystem::create_directories("log");
  sky::utility::Singleton<sky::utility::Logger>::getInstance().open("log/test_rpc_channel_server.log");

  RpcServer server("127.0.0.1", kTestPort);
  server.registerHandler("Calculator", "add", [](const std::vector<char> &params) {
    RpcSerializer reader;
    reader.reset(params);
    int a = reader.readInt32();
    int b = reader.readInt32();

    RpcSerializer writer;
    writer.writeInt32(a + b);
    return writer.data();
  });

  server.registerHandler("Calculator", "fail", [](const std::vector<char> &) {
    throw std::runtime_error("handler failed");
    return std::vector<char>{};
  });

  std::thread server_thread([&server]() { server.start(); });
  waitForServerReady();

  RpcChannel channel("127.0.0.1", kTestPort);

  RpcSerializer add_params;
  add_params.writeInt32(3);
  add_params.writeInt32(5);
  RpcResponse add_resp = channel.call("Calculator", "add", add_params);
  EXPECT_EQ(add_resp.status, static_cast<uint8_t>(RpcStatus::OK));

  RpcSerializer add_reader;
  add_reader.reset(add_resp.payload);
  EXPECT_EQ(add_reader.readInt32(), 8);

  RpcSerializer missing_params;
  RpcResponse missing_resp = channel.call("Calculator", "missing", missing_params);
  EXPECT_EQ(missing_resp.status, static_cast<uint8_t>(RpcStatus::NOT_FOUND));
  EXPECT_NE(decodeErrorPayload(missing_resp).find("handler not found"), std::string::npos);

  RpcSerializer fail_params;
  RpcResponse fail_resp = channel.call("Calculator", "fail", fail_params);
  EXPECT_EQ(fail_resp.status, static_cast<uint8_t>(RpcStatus::ERROR));
  EXPECT_EQ(decodeErrorPayload(fail_resp), "handler failed");

  server.stop();
  if (server_thread.joinable()) {
    server_thread.join();
  }
}
