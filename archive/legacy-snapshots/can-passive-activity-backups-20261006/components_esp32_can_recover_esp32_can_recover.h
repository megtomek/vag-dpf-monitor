#pragma once

#ifdef USE_ESP32

#include "esphome/components/canbus/canbus.h"
#include "esphome/core/component.h"

#include <driver/twai.h>

namespace esphome::esp32_can_recover {

enum CanMode : uint8_t {
  CAN_MODE_NORMAL = 0,
  CAN_MODE_LISTEN_ONLY = 1,
};

class ESP32CanRecover final : public canbus::Canbus {
 public:
  void set_rx(int rx) { rx_ = rx; }
  void set_tx(int tx) { tx_ = tx; }
  void set_mode(CanMode mode) { mode_ = mode; }
  void set_enabled(bool enabled) { enabled_ = enabled; }
  void set_rx_filter_id(uint32_t rx_filter_id) { rx_filter_id_ = rx_filter_id; }
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
  int tx_{-1};
  CanMode mode_{CAN_MODE_NORMAL};
  bool enabled_{true};
  TickType_t tx_enqueue_timeout_ticks_{};
  optional<uint32_t> tx_queue_len_{};
  optional<uint32_t> rx_queue_len_{};
  optional<uint32_t> rx_filter_id_{};
  twai_handle_t twai_handle_{nullptr};
  uint32_t last_state_check_ms_{0};
  uint32_t last_tx_error_log_ms_{0};
  uint32_t tx_errors_since_log_{0};
  uint32_t tx_retry_after_ms_{0};
  bool recovery_in_progress_{false};
};

}  // namespace esphome::esp32_can_recover

#endif
