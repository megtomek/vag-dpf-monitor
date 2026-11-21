#pragma once

#ifdef USE_ESP32

#include "esphome/components/canbus/canbus.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#include <driver/twai.h>

#include <vector>
#include "esphome/components/web_server_base/web_server_base.h"
#include <mutex>
#include <atomic>

namespace esphome::esp32_can_recover {

enum CanMode : uint8_t {
  CAN_MODE_NORMAL = 0,
  CAN_MODE_LISTEN_ONLY = 1,
  CAN_MODE_SELF_TEST = 2,
};

class ESP32CanRecover final : public canbus::Canbus, public web_server_idf::AsyncWebHandler {
 public:
  void set_rx(int rx) { rx_ = rx; }
  void set_tx(int tx) { tx_ = tx; }
  void set_mode(CanMode mode) { mode_ = mode; }
  void set_enabled(bool enabled) { enabled_ = enabled; }
  void set_capture_enabled(bool enabled) { capture_enabled_ = enabled; }
  bool canHandle(web_server_idf::AsyncWebServerRequest *request) const override;
  void handleRequest(web_server_idf::AsyncWebServerRequest *request) override;
  void set_single_shot_when_quiet(bool enabled) { single_shot_when_quiet_ = enabled; }
  void set_rx_filter_id(uint32_t rx_filter_id) { rx_filter_id_ = rx_filter_id; }
  template<typename F> void add_activity_callback(F &&callback) {
    this->activity_callback_manager_.add(std::forward<F>(callback));
  }
  void set_tx_queue_len(uint32_t tx_queue_len) { this->tx_queue_len_ = tx_queue_len; }
  void set_rx_queue_len(uint32_t rx_queue_len) { this->rx_queue_len_ = rx_queue_len; }
  void set_tx_enqueue_timeout_ms(uint32_t tx_enqueue_timeout_ms) {
    this->tx_enqueue_timeout_ticks_ = pdMS_TO_TICKS(tx_enqueue_timeout_ms);
  }
  // Discard frames queued before entering UI-only/demo mode. Without this,
  // TWAI may keep retrying a frame even after the OBD polling loop is paused.
  bool clear_transmit_queue();
  ESP32CanRecover(){};

  // Poll the TWAI state and restart the controller after bus-off recovery.
  void loop() override;

 protected:
  bool setup_internal() override;
  canbus::Error send_message(struct canbus::CanFrame *frame) override;
  canbus::Error read_message(struct canbus::CanFrame *frame) override;

  int rx_{-1};
  struct CaptureFrame { uint32_t ms, id; uint8_t dlc, flags, data[8]; };
  static constexpr size_t CAPTURE_CAPACITY = 65536;
  bool capture_enabled_{false}, capture_active_{false};
  CaptureFrame *capture_frames_{nullptr};
  size_t capture_count_{0};
  uint32_t capture_started_{0}, capture_dropped_{0}, capture_rx_missed_start_{0};
  std::mutex capture_mutex_;
  std::atomic<uint32_t> capture_skipped_{0};
  int tx_{-1};
  CanMode mode_{CAN_MODE_NORMAL};
  bool enabled_{true};
  TickType_t tx_enqueue_timeout_ticks_{};
  optional<uint32_t> tx_queue_len_{};
  optional<uint32_t> rx_queue_len_{};
  optional<uint32_t> rx_filter_id_{};
  LazyCallbackManager<void()> activity_callback_manager_{};
  twai_handle_t twai_handle_{nullptr};
  uint32_t last_state_check_ms_{0};
  uint32_t last_normal_report_ms_{0};
  uint32_t last_tx_error_log_ms_{0};
  uint32_t tx_errors_since_log_{0};
  uint32_t tx_retry_after_ms_{0};
  uint32_t self_test_rx_count_{0};
  uint32_t self_test_last_report_ms_{0};
  uint32_t rx_frames_since_report_{0};
  uint32_t max_rx_batch_since_report_{0};
  uint32_t max_rx_loop_us_since_report_{0};
  // Scratch storage reused for each accepted frame; callbacks consume it synchronously.
  std::vector<uint8_t> rx_payload_buffer_{};
  bool recovery_in_progress_{false};
  bool single_shot_when_quiet_{false};
  bool has_received_can_frame_{false};
  uint32_t last_can_rx_ms_{0};
};

}  // namespace esphome::esp32_can_recover

#endif
