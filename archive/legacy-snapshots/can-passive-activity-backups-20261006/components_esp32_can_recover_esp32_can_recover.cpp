#ifdef USE_ESP32
#include "esp32_can_recover.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#include <driver/twai.h>
#include <driver/gpio.h>
#include <esp_err.h>

// WORKAROUND, because CAN_IO_UNUSED is just defined as (-1) in this version
// of the framework which does not work with -fpermissive
#undef CAN_IO_UNUSED
#define CAN_IO_UNUSED ((gpio_num_t) -1)

namespace esphome::esp32_can_recover {

static const char *const TAG = "esp32_can_recover";

static bool get_bitrate(canbus::CanSpeed bitrate, twai_timing_config_t *t_config) {
  switch (bitrate) {
#if defined(USE_ESP32_VARIANT_ESP32C3) || defined(USE_ESP32_VARIANT_ESP32C6) || defined(USE_ESP32_VARIANT_ESP32C61) || \
    defined(USE_ESP32_VARIANT_ESP32H2) || defined(USE_ESP32_VARIANT_ESP32P4) || defined(USE_ESP32_VARIANT_ESP32S2) || \
    defined(USE_ESP32_VARIANT_ESP32S3)
    case canbus::CAN_1KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_1KBITS();
      return true;
    case canbus::CAN_5KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_5KBITS();
      return true;
    case canbus::CAN_10KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_10KBITS();
      return true;
    case canbus::CAN_12K5BPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_12_5KBITS();
      return true;
    case canbus::CAN_16KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_16KBITS();
      return true;
    case canbus::CAN_20KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_20KBITS();
      return true;
#endif
    case canbus::CAN_25KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_25KBITS();
      return true;
    case canbus::CAN_50KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_50KBITS();
      return true;
    case canbus::CAN_100KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_100KBITS();
      return true;
    case canbus::CAN_125KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_125KBITS();
      return true;
    case canbus::CAN_250KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_250KBITS();
      return true;
    case canbus::CAN_500KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_500KBITS();
      return true;
    case canbus::CAN_800KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_800KBITS();
      return true;
    case canbus::CAN_1000KBPS:
      *t_config = (twai_timing_config_t) TWAI_TIMING_CONFIG_1MBITS();
      return true;
    default:
      return false;
  }
}

bool ESP32CanRecover::setup_internal() {
  if (!this->enabled_) {
    ESP_LOGI(TAG, "CAN controller disabled by configuration (bench/UI test mode)");
    return true;
  }

  static int next_twai_ctrl_num = 0;
  if (static_cast<unsigned>(next_twai_ctrl_num) >= SOC_TWAI_CONTROLLER_NUM) {
    ESP_LOGW(TAG, "Maximum number of esp32_can components created already");
    this->mark_failed();
    return false;
  }

  // Select TWAI mode based on configuration
  twai_mode_t twai_mode = (this->mode_ == CAN_MODE_LISTEN_ONLY) ? TWAI_MODE_LISTEN_ONLY : TWAI_MODE_NORMAL;

  if (this->mode_ == CAN_MODE_LISTEN_ONLY) {
    ESP_LOGI(TAG, "CAN bus configured in LISTEN_ONLY mode (passive, no ACKs)");
  }

  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t) this->tx_, (gpio_num_t) this->rx_, twai_mode);
  g_config.controller_id = next_twai_ctrl_num++;
  // The legacy TWAI driver only enables the GPIO input; unlike the newer
  // esp_driver_twai node driver, it does not add a pull-up. Hold RX recessive
  // when the transceiver is not connected during USB bench work.
  const esp_err_t rx_pull_error = gpio_set_pull_mode(static_cast<gpio_num_t>(this->rx_), GPIO_PULLUP_ONLY);
  if (rx_pull_error != ESP_OK) {
    ESP_LOGW(TAG, "Could not enable CAN RX pull-up on GPIO%d: %s", this->rx_, esp_err_to_name(rx_pull_error));
  } else {
    ESP_LOGI(TAG, "CAN RX pull-up enabled on GPIO%d", this->rx_);
  }
  if (this->tx_queue_len_.has_value()) {
    g_config.tx_queue_len = this->tx_queue_len_.value();
  }
  if (this->rx_queue_len_.has_value()) {
    g_config.rx_queue_len = this->rx_queue_len_.value();
  }

  // In the selected physical OBD mode we only need the ECU response ID.
  // Filtering in TWAI keeps unrelated vehicle traffic out of the RX queue.
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  if (this->rx_filter_id_.has_value()) {
    const uint32_t standard_id_mask = 0x7FFU << 21;
    f_config.acceptance_code = (this->rx_filter_id_.value() & 0x7FFU) << 21;
    // In the legacy ESP-IDF TWAI filter, mask bits set to 1 are don't-care.
    f_config.acceptance_mask = ~standard_id_mask;
    f_config.single_filter = true;
    ESP_LOGI(TAG, "Hardware RX filter enabled for standard CAN ID 0x%03" PRIX32,
             this->rx_filter_id_.value());
  }
  twai_timing_config_t t_config;

  if (!get_bitrate(this->bit_rate_, &t_config)) {
    // invalid bit rate
    this->mark_failed();
    return false;
  }

  // Install TWAI driver
  if (twai_driver_install_v2(&g_config, &t_config, &f_config, &(this->twai_handle_)) != ESP_OK) {
    // Failed to install driver
    this->mark_failed();
    return false;
  }

  // Start TWAI driver
  if (twai_start_v2(this->twai_handle_) != ESP_OK) {
    // Failed to start driver
    this->mark_failed();
    return false;
  }
  return true;
}

void ESP32CanRecover::loop() {
  if (!this->enabled_) return;

  // Process only a bounded batch per ESPHome loop pass. The generic ESPHome
  // implementation drains until the RX queue is empty; on a continuously busy
  // vehicle bus it can monopolize the main loop and starve LVGL/display work.
  // Keep callback and trigger semantics, but return to ESPHome after 8 frames.
  constexpr uint8_t MAX_RX_FRAMES_PER_LOOP = 8;
  struct canbus::CanFrame can_message;
  for (uint8_t processed = 0;
       processed < MAX_RX_FRAMES_PER_LOOP && this->read_message(&can_message) == canbus::ERROR_OK;
       processed++) {
    std::vector<uint8_t> data;
    const size_t length = std::min<size_t>(can_message.can_data_length_code, canbus::CAN_MAX_DATA_LENGTH);
    data.reserve(length);
    for (size_t i = 0; i < length; i++) data.push_back(can_message.data[i]);

    this->callback_manager_(can_message.can_id, can_message.use_extended_id, can_message.remote_transmission_request,
                            data);
  }

  if (this->twai_handle_ == nullptr) return;

  const uint32_t now = millis();
  if (now - this->last_state_check_ms_ < 250) return;
  this->last_state_check_ms_ = now;

  twai_status_info_t status{};
  if (twai_get_status_info_v2(this->twai_handle_, &status) != ESP_OK) return;

  if (status.state == TWAI_STATE_BUS_OFF) {
    if (twai_initiate_recovery_v2(this->twai_handle_) == ESP_OK) {
      this->recovery_in_progress_ = true;
      ESP_LOGW(TAG, "CAN entered bus-off; starting automatic recovery");
    }
    return;
  }

  // Recovery finishes in STOPPED state. Restart the controller so it can
  // transmit as soon as a valid CAN bus becomes available again.
  if (this->recovery_in_progress_ && status.state == TWAI_STATE_STOPPED) {
    if (twai_start_v2(this->twai_handle_) == ESP_OK) {
      this->recovery_in_progress_ = false;
      ESP_LOGI(TAG, "CAN bus recovery complete; controller restarted");
    } else {
      ESP_LOGW(TAG, "CAN controller restart after bus recovery failed");
    }
  }
}

canbus::Error ESP32CanRecover::send_message(struct canbus::CanFrame *frame) {
  if (!this->enabled_) return canbus::ERROR_FAIL;

  // In listen-only mode, we cannot transmit
  if (this->mode_ == CAN_MODE_LISTEN_ONLY) {
    ESP_LOGW(TAG, "Cannot send messages in LISTEN_ONLY mode");
    return canbus::ERROR_FAIL;
  }

  if (this->twai_handle_ == nullptr) {
    // not setup yet or setup failed
    return canbus::ERROR_FAIL;
  }

  if (frame->can_data_length_code > canbus::CAN_MAX_DATA_LENGTH) {
    return canbus::ERROR_FAILTX;
  }

  const uint32_t now = millis();
  if (this->tx_retry_after_ms_ != 0 && static_cast<int32_t>(now - this->tx_retry_after_ms_) < 0) {
    return canbus::ERROR_ALLTXBUSY;
  }

  uint32_t flags = TWAI_MSG_FLAG_NONE;
  if (frame->use_extended_id) {
    flags |= TWAI_MSG_FLAG_EXTD;
  }
  if (frame->remote_transmission_request) {
    flags |= TWAI_MSG_FLAG_RTR;
  }

  twai_message_t message = {
      .flags = flags,
      .identifier = frame->can_id,
      .data_length_code = frame->can_data_length_code,
      .data = {},  // to suppress warning, data is initialized properly below
  };
  if (!frame->remote_transmission_request) {
    memcpy(message.data, frame->data, frame->can_data_length_code);
  }

  const esp_err_t tx_error =
      twai_transmit_v2(this->twai_handle_, &message, this->tx_enqueue_timeout_ticks_);
  if (tx_error == ESP_OK) {
    return canbus::ERROR_OK;
  }

  // Report the original ESP-IDF error and TWAI counters periodically. The
  // generic ESPHome canbus layer only reports ERROR_ALLTXBUSY for this path,
  // which hides whether the TX queue timed out or the controller was stopped.
  this->tx_errors_since_log_++;
  if (tx_error == ESP_ERR_TIMEOUT) {
    // If the bus has no ACKing peer, TWAI can keep queued OBD requests waiting
    // indefinitely. They are stale by the time a vehicle appears, so discard
    // pending requests and briefly back off instead of filling the TX queue.
    const esp_err_t clear_error = twai_clear_transmit_queue_v2(this->twai_handle_);
    this->tx_retry_after_ms_ = millis() + 1000;
    if (clear_error != ESP_OK && clear_error != ESP_ERR_INVALID_STATE) {
      ESP_LOGD(TAG, "Could not clear pending TWAI TX queue: %s", esp_err_to_name(clear_error));
    }
  }
  const uint32_t log_now = millis();
  if (this->last_tx_error_log_ms_ == 0 || log_now - this->last_tx_error_log_ms_ >= 5000) {
    twai_status_info_t status{};
    if (twai_get_status_info_v2(this->twai_handle_, &status) == ESP_OK) {
      ESP_LOGW(TAG,
               "TWAI TX failed: %s (0x%X); %u failure(s) since last report; state=%u, "
               "tx_err=%u, rx_err=%u, tx_failed=%u, bus_err=%u, queued=%u",
               esp_err_to_name(tx_error), static_cast<unsigned>(tx_error),
               static_cast<unsigned>(this->tx_errors_since_log_), static_cast<unsigned>(status.state),
               static_cast<unsigned>(status.tx_error_counter), static_cast<unsigned>(status.rx_error_counter),
               static_cast<unsigned>(status.tx_failed_count), static_cast<unsigned>(status.bus_error_count),
               static_cast<unsigned>(status.msgs_to_tx));
    } else {
      ESP_LOGW(TAG, "TWAI TX failed: %s (0x%X); %u failure(s) since last report; status unavailable",
               esp_err_to_name(tx_error), static_cast<unsigned>(tx_error),
               static_cast<unsigned>(this->tx_errors_since_log_));
    }
    this->last_tx_error_log_ms_ = log_now;
    this->tx_errors_since_log_ = 0;
  }
  return canbus::ERROR_ALLTXBUSY;
}

canbus::Error ESP32CanRecover::read_message(struct canbus::CanFrame *frame) {
  if (this->twai_handle_ == nullptr) {
    // not setup yet or setup failed
    return canbus::ERROR_FAIL;
  }

  twai_message_t message;

  if (twai_receive_v2(this->twai_handle_, &message, 0) != ESP_OK) {
    return canbus::ERROR_NOMSG;
  }

  frame->can_id = message.identifier;
  frame->use_extended_id = message.flags & TWAI_MSG_FLAG_EXTD;
  frame->remote_transmission_request = message.flags & TWAI_MSG_FLAG_RTR;
  frame->can_data_length_code = message.data_length_code;

  if (!frame->remote_transmission_request) {
    size_t dlc =
        message.data_length_code < canbus::CAN_MAX_DATA_LENGTH ? message.data_length_code : canbus::CAN_MAX_DATA_LENGTH;
    memcpy(frame->data, message.data, dlc);
  }

  return canbus::ERROR_OK;
}

bool ESP32CanRecover::clear_transmit_queue() {
  if (!this->enabled_) return true;
  if (this->twai_handle_ == nullptr) return false;
  const esp_err_t error = twai_clear_transmit_queue_v2(this->twai_handle_);
  if (error == ESP_OK || error == ESP_ERR_INVALID_STATE) {
    this->tx_retry_after_ms_ = 0;
    return true;
  }
  ESP_LOGW(TAG, "Could not clear TWAI TX queue: %s", esp_err_to_name(error));
  return false;
}

}  // namespace esphome::esp32_can_recover

#endif
