# ESPAsyncWebServer

[![Latest Release](https://img.shields.io/github/release/mathieucarbou/ESPAsyncWebServer.svg)](https://GitHub.com/mathieucarbou/ESPAsyncWebServer/releases/)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/mathieucarbou/library/ESPAsyncWebServer.svg)](https://registry.platformio.org/libraries/mathieucarbou/ESPAsyncWebServer)

[![License: LGPL 3.0](https://img.shields.io/badge/License-LGPL%203.0-yellow.svg)](https://opensource.org/license/lgpl-3-0/)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](code_of_conduct.md)

[![Build](https://github.com/mathieucarbou/ESPAsyncWebServer/actions/workflows/ci.yml/badge.svg)](https://github.com/mathieucarbou/ESPAsyncWebServer/actions/workflows/ci.yml)
[![GitHub latest commit](https://badgen.net/github/last-commit/mathieucarbou/ESPAsyncWebServer)](https://GitHub.com/mathieucarbou/ESPAsyncWebServer/commit/)
[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/mathieucarbou/ESPAsyncWebServer)

Asynchronous HTTP and WebSocket Server Library for ESP32, ESP8266 and RP2040
Supports: WebSocket, SSE, Authentication, Arduino Json 7, File Upload, Static File serving, URL Rewrite, URL Redirect, etc.

This fork is based on [yubox-node-org/ESPAsyncWebServer](https://github.com/yubox-node-org/ESPAsyncWebServer) and includes all the concurrency fixes.

## Coordinate and dependencies:

**WARNING** The library name was changed from `ESP Async WebServer` to `ESPAsyncWebServer` as per the Arduino Lint recommendations.

```
mathieucarbou/ESPAsyncWebServer @ 3.2.4
```

Dependency:

- **ESP32**: `mathieucarbou/AsyncTCP @ 3.2.5` (Arduino IDE: [https://github.com/mathieucarbou/AsyncTCP#v3.2.5](https://github.com/mathieucarbou/AsyncTCP/releases))
- **ESP8266**: `esphome/ESPAsyncTCP-esphome @ 2.0.0` (Arduino IDE: [https://github.com/mathieucarbou/esphome-ESPAsyncTCP#v2.0.0](https://github.com/mathieucarbou/esphome-ESPAsyncTCP/releases/tag/v2.0.0))
- **RP2040**: `khoih-prog/AsyncTCP_RP2040W @ 1.2.0` (Arduino IDE: [https://github.com/khoih-prog/AsyncTCP_RP2040W#v1.2.0](https://github.com/khoih-prog/AsyncTCP_RP2040W/releases/tag/v1.2.0))

## Changes in this fork

- `char*` overloads to avoid using `String`
- `DEFAULT_MAX_WS_CLIENTS` to change the number of allows WebSocket clients and use `cleanupClients()` to help cleanup resources about dead clients
- `setCloseClientOnQueueFull(bool)` which can be set on a client to either close the connection or discard messages but not close the connection when the queue is full
- `SSE_MAX_QUEUED_MESSAGES` to control the maximum number of messages that can be queued for a SSE client
- `StreamConcat` example to show how to stream multiple files in one response
- `WS_MAX_QUEUED_MESSAGES`: control the maximum number of messages that can be queued for a Websocket client
- A lot of bug fixes
- Arduino 3 / ESP-IDF 5 compatibility
- Arduino Json 7 compatibility and backward compatible with 6
- Better CI with a complete matrix of Arduino versions and boards
- Code size improvements
- Deployed in PlatformIO registry and Arduino IDE library manager
- ESP32 / ESP8266 / RP2040 support
- Lot of code cleanup and optimizations
- MessagePack support
- Performance improvements in terms of memory, speed and size
- Removed ESPIDF Editor (this is not the role of a web server library to do that - get the source files from the original repos if required)
- Support overriding default response headers
- Support resumable downloads using HEAD and bytes range

## Documentation

Usage and API stays the same as the original library.
Please look at the original libraries for more examples and documentation.

- [https://github.com/me-no-dev/ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) (original library)
- [https://github.com/yubox-node-org/ESPAsyncWebServer](https://github.com/yubox-node-org/ESPAsyncWebServer) (fork of the original library)

## `AsyncWebSocketMessageBuffer` and `makeBuffer()`

The fork from `yubox-node-org` introduces some breaking API changes compared to the original library, especially regarding the use of `std::shared_ptr<std::vector<uint8_t>>` for WebSocket.

This fork is compatible with the original library from `me-no-dev` regarding WebSocket, and wraps the optimizations done by `yubox-node-org` in the `AsyncWebSocketMessageBuffer` class.
So you have the choice of which API to use.

Here are examples for serializing a Json document in a websocket message buffer:

```cpp
void send(JsonDocument& doc) {
  const size_t len = measureJson(doc);

  // original API from me-no-dev
  AsyncWebSocketMessageBuffer* buffer = _ws->makeBuffer(len);
  assert(buffer); // up to you to keep or remove this
  serializeJson(doc, buffer->get(), len);
  _ws->textAll(buffer);
}
```

```cpp
void send(JsonDocument& doc) {
  const size_t len = measureJson(doc);

  // this fork (originally from yubox-node-org), uses another API with shared pointer
  auto buffer = std::make_shared<std::vector<uint8_t>>(len);
  assert(buffer); // up to you to keep or remove this
  serializeJson(doc, buffer->data(), len);
  _ws->textAll(std::move(buffer));
}
```

I recommend to use the official API `AsyncWebSocketMessageBuffer` to retain further compatibility.

## Important recommendations

Most of the crashes are caused by improper configuration of the library for the project.
Here are some recommendations to avoid them.

1. Set the running core to be on the same core of your application (usually core 1) `-D CONFIG_ASYNC_TCP_RUNNING_CORE=1`
2. Set the stack size appropriately with `-D CONFIG_ASYNC_TCP_STACK_SIZE=16384`.
   The default value of `16384` might be too much for your project.
   You can look at the [MycilaTaskMonitor](https://oss.carbou.me/MycilaTaskMonitor) project to monitor the stack usage.
3. You can change **if you know what you are doing** the task priority with `-D CONFIG_ASYNC_TCP_PRIORITY=10`.
   Default is `10`.
4. You can increase the queue size with `-D CONFIG_ASYNC_TCP_QUEUE_SIZE=128`.
   Default is `64`.
5. You can decrease the maximum ack time `-D CONFIG_ASYNC_TCP_MAX_ACK_TIME=3000`.
   Default is `5000`.

I personally use the following configuration in my projects because my WS messages can be big (up to 4k).
If you have smaller messages, you can increase `WS_MAX_QUEUED_MESSAGES` to 128.

```c++
  -D CONFIG_ASYNC_TCP_MAX_ACK_TIME=3000
  -D CONFIG_ASYNC_TCP_PRIORITY=10
  -D CONFIG_ASYNC_TCP_QUEUE_SIZE=128
  -D CONFIG_ASYNC_TCP_RUNNING_CORE=1
  -D CONFIG_ASYNC_TCP_STACK_SIZE=4096
  -D WS_MAX_QUEUED_MESSAGES=64
```
