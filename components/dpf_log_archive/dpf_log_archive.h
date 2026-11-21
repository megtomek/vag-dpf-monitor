#pragma once
#include "esphome/core/component.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include <atomic>
#include <mutex>

namespace esphome::dpf_log_archive {
// Bounded RAM only: 10 minutes OR 1024 messages, whichever is shorter.
// Reset/power loss clears history. No flash writes and no allocations per log.
class LogArchive : public Component, public web_server_idf::AsyncWebHandler {
 public:
  void setup() override;
  float get_setup_priority() const override { return 600.0f; }
  bool canHandle(web_server_idf::AsyncWebServerRequest *request) const override;
  void handleRequest(web_server_idf::AsyncWebServerRequest *request) override;
 private:
  static constexpr size_t CAPACITY = 1024;
  static constexpr size_t LINE_SIZE = 480;
  static constexpr uint32_t RETENTION_MS = 600000;
  struct Entry { uint32_t uptime; uint16_t length; char text[LINE_SIZE]; };
  void capture_(const char *message, size_t length);
  Entry *entries_{nullptr};
  size_t head_{0}, count_{0};
  uint32_t overwritten_{0}, truncated_{0};
  std::atomic<uint32_t> dropped_{0};
  std::mutex mutex_;
};
}
