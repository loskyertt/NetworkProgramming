# RPC 模块学习笔记

这个目录实现了一个轻量级 RPC 框架。它不是完整的工业级框架，但包含 RPC 最核心的几个环节：

- 客户端把“本地函数调用”包装成网络请求。
- 服务端根据服务名和方法名找到真正的处理函数。
- 参数和返回值通过二进制序列化在 TCP 连接上传输。
- 请求和响应用 `call_id` 配对，并用状态码表达成功或失败。

示例代码在 `examples/rpc_calculator_server.cpp`、`examples/rpc_calculator_client.cpp` 和 `examples/rpc_calculator_stub.h`。

## 什么是 RPC

RPC 是 Remote Procedure Call，远程过程调用。

普通本地调用是：

```cpp
int result = add(3, 5);
```

RPC 希望让远程调用看起来也像本地调用：

```cpp
CalculatorStub calc("127.0.0.1", 8080);
int result = calc.add(3, 5);
```

但这行代码背后实际发生的是：

1. `CalculatorStub::add` 把 `3` 和 `5` 序列化成二进制参数。
2. `RpcChannel` 创建 TCP 连接。
3. 客户端发送一个 RPC Request，里面包含服务名 `Calculator`、方法名 `add` 和参数 payload。
4. 服务端收到请求后，根据 `Calculator.add` 找到注册的 handler。
5. handler 反序列化参数，执行真实逻辑，得到结果 `8`。
6. 服务端把 `8` 序列化成响应 payload 发回客户端。
7. 客户端反序列化响应 payload，返回给调用方。

所以 RPC 的本质是：把函数调用拆成“序列化 + 网络传输 + 服务端分发 + 结果返回”。

## 目录结构

```text
src/rpc
├── CMakeLists.txt
├── README.md
├── include/rpc
│   ├── rpc_protocol.h    # 协议头、魔数、版本、状态码
│   ├── rpc_message.h     # RpcRequest/RpcResponse 和收发函数声明
│   ├── rpc_serializer.h  # 简易二进制序列化器
│   ├── rpc_channel.h     # 客户端调用通道
│   └── rpc_server.h      # 服务端注册和分发
└── impl
    ├── rpc_message.cpp
    ├── rpc_serializer.cpp
    ├── rpc_channel.cpp
    └── rpc_server.cpp
```

## 整体调用流程

```text
客户端业务代码
  |
  v
CalculatorStub::add(3, 5)
  |
  v
RpcSerializer 写入参数: int32(3), int32(5)
  |
  v
RpcChannel::call("Calculator", "add", params)
  |
  v
sendRequest(fd, RpcRequest)
  |
  v
TCP 网络
  |
  v
RpcServer epoll 收到连接可读事件
  |
  v
recvRequest(fd, RpcRequest)
  |
  v
查找 handler: "Calculator.add"
  |
  v
handler 反序列化参数并执行真实 add
  |
  v
sendResponse(fd, RpcResponse)
  |
  v
客户端 recvResponse(fd, RpcResponse)
  |
  v
Stub 反序列化返回值并返回 int
```

## 协议格式

协议定义在 `include/rpc/rpc_protocol.h`。

RPC 传输的数据由两部分组成：

```text
定长 Header + 变长 Body
```

Header 使用 `RpcHeader`：

```cpp
struct RpcHeader {
  uint32_t magic;
  uint8_t version;
  uint8_t msg_type;
  uint8_t status;
  uint32_t call_id;
  uint32_t svc_len;
  uint32_t meth_len;
  uint32_t payload_len;
};
```

字段含义：

| 字段 | 含义 |
| --- | --- |
| `magic` | 魔数，用来识别这是本 RPC 协议的数据包 |
| `version` | 协议版本，当前是 `1` |
| `msg_type` | 消息类型，`0` 是请求，`1` 是响应 |
| `status` | 响应状态，请求中不使用 |
| `call_id` | 调用 ID，用来匹配请求和响应 |
| `svc_len` | 服务名长度，只在请求中使用 |
| `meth_len` | 方法名长度，只在请求中使用 |
| `payload_len` | 参数或返回值 payload 的长度 |

请求包格式：

```text
RpcHeader
ServiceName bytes
MethodName bytes
Payload bytes
```

响应包格式：

```text
RpcHeader
Payload bytes
```

例如调用 `Calculator.add(3, 5)`：

```text
service_name = "Calculator"
method_name  = "add"
payload      = int32(3) + int32(5)
```

响应成功时：

```text
status  = OK
payload = int32(8)
```

## 为什么需要 Header

TCP 是字节流协议，不保留消息边界。

客户端调用一次 `send`，服务端不一定对应一次完整 `recv`。服务端可能出现这些情况：

- 一次只收到半个 Header。
- 一次收到 Header 加一半 Body。
- 一次收到多个 RPC 包的数据。

因此接收方必须知道“一个完整 RPC 消息到底有多少字节”。本项目用 Header 里的长度字段解决这个问题。

`impl/rpc_message.cpp` 中的 `recvFull` 会循环读取，直到读满指定字节数：

```text
先读满 RpcHeader
再根据 svc_len、meth_len、payload_len 读满 Body
```

发送方也类似，`sendFull` 会循环发送，直到全部字节写入 socket。

## 序列化

序列化器定义在 `include/rpc/rpc_serializer.h`，实现在 `impl/rpc_serializer.cpp`。

网络只能传输字节，不能直接传输 C++ 对象。序列化就是把对象转成字节数组，反序列化就是把字节数组还原成对象。

当前支持的类型：

| 类型 | 编码方式 |
| --- | --- |
| `int8_t` | 1 字节 |
| `int32_t` | 4 字节 |
| `uint32_t` | 4 字节 |
| `int64_t` | 8 字节 |
| `double` | 8 字节 |
| `bool` | 1 字节，`0` 或 `1` |
| `std::string` | `uint32_t(length)` + 字符串内容 |

写参数示例：

```cpp
RpcSerializer params;
params.writeInt32(3);
params.writeInt32(5);
```

读参数示例：

```cpp
RpcSerializer reader;
reader.reset(params_payload);
int a = reader.readInt32();
int b = reader.readInt32();
```

注意：写入顺序和读取顺序必须完全一致。写的是 `int32, int32`，读的时候也必须按 `int32, int32` 读。

## 客户端：Stub 和 Channel

客户端分两层。

第一层是业务友好的 Stub，例如 `examples/rpc_calculator_stub.h`：

```cpp
int add(int a, int b) {
  RpcSerializer params;
  params.writeInt32(a);
  params.writeInt32(b);

  RpcResponse resp = m_channel.call("Calculator", "add", params);
  // 检查状态码，反序列化返回值
}
```

Stub 的作用是隐藏 RPC 细节，让业务调用者只看到普通方法。

第二层是通用的 `RpcChannel`：

```cpp
RpcResponse call(const std::string &service,
                 const std::string &method,
                 RpcSerializer &params);
```

`RpcChannel` 做这些事情：

1. 创建 `ClientSocket` 并连接服务端。
2. 生成递增的 `call_id`。
3. 构造 `RpcRequest`。
4. 调用 `sendRequest` 发送请求。
5. 调用 `recvResponse` 等待响应。
6. 校验响应的 `call_id` 是否匹配请求。

当前实现中，每次 RPC 调用都会创建一个新的 TCP 连接。这样实现最简单，适合学习和演示。缺点是频繁创建连接会有额外开销。

## 服务端：注册和分发

服务端入口是 `RpcServer`。

业务代码通过 `registerHandler` 注册方法：

```cpp
server.registerHandler("Calculator", "add", [](const std::vector<char> &params) {
  RpcSerializer reader;
  reader.reset(params);
  int a = reader.readInt32();
  int b = reader.readInt32();

  RpcSerializer writer;
  writer.writeInt32(a + b);
  return writer.data();
});
```

服务端内部把服务名和方法名拼成 key：

```text
Calculator.add
```

请求到达后，`RpcServer::handleRequest` 会：

1. 从请求中取出 `service_name` 和 `method_name`。
2. 拼出 key。
3. 在 `m_handlers` 中查找对应 handler。
4. 找不到则返回 `NOT_FOUND`。
5. 找到则执行 handler，把返回的字节数组放进响应 payload。
6. handler 抛异常时返回 `ERROR`，并把错误信息作为字符串写入 payload。

## 服务端网络模型

`RpcServer` 使用项目已有的 `socket` 和 `thread` 模块：

- `ServerSocket` 负责创建、绑定、监听 socket。
- `EPoller` 负责等待连接事件。
- `ThreadPool` 负责并发执行请求处理逻辑。

大致流程：

```text
listen_fd 加入 epoll
  |
  v
epoll_wait 等待事件
  |
  v
listen_fd 可读: accept 新连接，把 conn_fd 加入 epoll
  |
  v
conn_fd 可读: recvRequest 读取完整请求
  |
  v
从 epoll 移除 conn_fd
  |
  v
提交到线程池处理
  |
  v
线程池执行 handler，发送响应，关闭连接
```

当前实现采用“一个连接处理一个请求”的模型。服务端处理完响应后关闭连接。

## 错误处理

状态码定义在 `RpcStatus`：

| 状态码 | 含义 |
| --- | --- |
| `OK` | 调用成功 |
| `ERROR` | 网络失败、handler 异常或其他错误 |
| `NOT_FOUND` | 服务或方法未注册 |
| `BAD_REQUEST` | 请求格式错误，预留状态 |

协议层会校验：

- `magic` 是否正确。
- `version` 是否匹配。
- `msg_type` 是否符合当前读取函数的预期。
- body 总大小是否超过 `RPC_MAX_BODY_SIZE`。

客户端还会校验响应 `call_id` 是否等于请求 `call_id`，避免把不属于当前请求的响应当成正确结果。

## 一次 add 调用的完整数据变化

调用代码：

```cpp
int result = calc.add(3, 5);
```

客户端 Stub 生成参数 payload：

```text
03 00 00 00 05 00 00 00
```

请求语义：

```text
call_id      = 1
service_name = Calculator
method_name  = add
payload      = 两个 int32 参数
```

服务端 handler 读取 payload：

```cpp
int a = reader.readInt32(); // 3
int b = reader.readInt32(); // 5
```

服务端生成响应 payload：

```text
08 00 00 00
```

客户端读取响应：

```cpp
int result = reader.readInt32(); // 8
```

## 当前实现的边界

这个 RPC 模块适合学习网络 RPC 的基本原理，但仍然是教学级实现。

已具备：

- 简单二进制协议。
- TCP 粘包/半包处理。
- 基础序列化和反序列化。
- 服务注册和方法分发。
- 请求响应 `call_id` 匹配。
- 基础状态码和错误信息返回。
- epoll + 线程池服务端模型。

尚未实现：

- 长连接复用和连接池。
- 单连接多路复用，也就是同一连接同时发多个请求。
- 超时控制。
- 自动代码生成。
- 接口描述语言，例如 protobuf IDL。
- 跨大小端机器的标准网络字节序转换。
- 认证、加密、限流、熔断、重试等生产级能力。

## 如果继续完善

可以按这个顺序演进：

1. 给 `RpcChannel::call` 增加超时参数。
2. 使用长连接，减少每次调用创建 TCP 连接的开销。
3. 给响应错误 payload 设计固定格式，而不是约定为字符串。
4. 引入 IDL 和代码生成，自动生成 Stub 和服务端注册代码。
5. 使用 protobuf、flatbuffers 或 msgpack 替换手写序列化。
6. 增加压测和单元测试，覆盖协议边界、错误包和并发请求。
