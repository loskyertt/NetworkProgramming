#include "rpc/rpc_message.h"
#include "rpc/rpc_protocol.h"

#include <gtest/gtest.h>

#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <string>
#include <vector>

using namespace sky::rpc;

class SocketPair {
 public:
  SocketPair() {
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, fds_) != 0) {
      throw std::runtime_error("socketpair failed");
    }
  }

  ~SocketPair() {
    ::close(fds_[0]);
    ::close(fds_[1]);
  }

  int first() const { return fds_[0]; }

  int second() const { return fds_[1]; }

 private:
  int fds_[2]{-1, -1};
};

static void write_header(int fd, RpcHeader header) {
  ASSERT_EQ(::send(fd, &header, sizeof(header), 0), static_cast<ssize_t>(sizeof(header)));
}

TEST(RpcMessageTest, RequestRoundTrip) {
  SocketPair sockets;
  RpcRequest req;
  req.call_id = 7;
  req.service_name = "Calculator";
  req.method_name = "add";
  req.payload = {'a', 'b', 'c'};

  ASSERT_TRUE(send_request(sockets.first(), req));

  RpcRequest decoded;
  ASSERT_TRUE(recv_request(sockets.second(), decoded));
  EXPECT_EQ(decoded.call_id, req.call_id);
  EXPECT_EQ(decoded.service_name, req.service_name);
  EXPECT_EQ(decoded.method_name, req.method_name);
  EXPECT_EQ(decoded.payload, req.payload);
}

TEST(RpcMessageTest, ResponseRoundTrip) {
  SocketPair sockets;
  RpcResponse resp;
  resp.call_id = 11;
  resp.status = static_cast<uint8_t>(RpcStatus::OK);
  resp.payload = {'o', 'k'};

  ASSERT_TRUE(send_response(sockets.first(), resp));

  RpcResponse decoded;
  ASSERT_TRUE(recv_response(sockets.second(), decoded));
  EXPECT_EQ(decoded.call_id, resp.call_id);
  EXPECT_EQ(decoded.status, resp.status);
  EXPECT_EQ(decoded.payload, resp.payload);
}

TEST(RpcMessageTest, RejectsInvalidMagic) {
  SocketPair sockets;
  RpcHeader header{};
  header.magic = 0;
  header.version = k_rpc_version;
  header.msg_type = static_cast<uint8_t>(RpcMsgType::REQUEST);
  header.svc_len = 1;
  header.meth_len = 1;
  write_header(sockets.first(), header);

  RpcRequest req;
  EXPECT_FALSE(recv_request(sockets.second(), req));
}

TEST(RpcMessageTest, RejectsWrongMessageType) {
  SocketPair sockets;
  RpcHeader header{};
  header.magic = k_rpc_magic;
  header.version = k_rpc_version;
  header.msg_type = static_cast<uint8_t>(RpcMsgType::RESPONSE);
  header.svc_len = 1;
  header.meth_len = 1;
  write_header(sockets.first(), header);

  RpcRequest req;
  EXPECT_FALSE(recv_request(sockets.second(), req));
}

TEST(RpcMessageTest, RejectsOversizedBody) {
  SocketPair sockets;
  RpcHeader header{};
  header.magic = k_rpc_magic;
  header.version = k_rpc_version;
  header.msg_type = static_cast<uint8_t>(RpcMsgType::REQUEST);
  header.svc_len = 1;
  header.meth_len = 1;
  header.payload_len = k_rpc_max_body_size;
  write_header(sockets.first(), header);

  RpcRequest req;
  EXPECT_FALSE(recv_request(sockets.second(), req));
}

TEST(RpcMessageTest, RejectsEmptyRequestNames) {
  SocketPair sockets;
  RpcHeader header{};
  header.magic = k_rpc_magic;
  header.version = k_rpc_version;
  header.msg_type = static_cast<uint8_t>(RpcMsgType::REQUEST);
  write_header(sockets.first(), header);

  RpcRequest req;
  EXPECT_FALSE(recv_request(sockets.second(), req));
}

TEST(RpcMessageTest, RejectsResponseWithRequestNameLengths) {
  SocketPair sockets;
  RpcHeader header{};
  header.magic = k_rpc_magic;
  header.version = k_rpc_version;
  header.msg_type = static_cast<uint8_t>(RpcMsgType::RESPONSE);
  header.status = static_cast<uint8_t>(RpcStatus::OK);
  header.svc_len = 1;
  write_header(sockets.first(), header);

  RpcResponse resp;
  EXPECT_FALSE(recv_response(sockets.second(), resp));
}

TEST(RpcMessageTest, SendRejectsRequestBodyOverLimit) {
  SocketPair sockets;
  RpcRequest req;
  req.service_name = "s";
  req.method_name = "m";
  req.payload.resize(k_rpc_max_body_size - req.service_name.size() - req.method_name.size() + 1);

  EXPECT_FALSE(send_request(sockets.first(), req));
}

TEST(RpcMessageTest, SendRejectsResponseBodyOverLimit) {
  SocketPair sockets;
  RpcResponse resp;
  resp.payload.resize(k_rpc_max_body_size + 1);

  EXPECT_FALSE(send_response(sockets.first(), resp));
}
