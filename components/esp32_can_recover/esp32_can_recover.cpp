#ifdef USE_ESP32
#include "esp32_can_recover.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#include <driver/twai.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_heap_caps.h>

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
  // Capture builds are strictly passive, regardless of other YAML overrides.
  if (capture_enabled_) {
    mode_ = CAN_MODE_LISTEN_ONLY;
    capture_frames_ = static_cast<CaptureFrame *>(heap_caps_calloc(CAPTURE_CAPACITY, sizeof(CaptureFrame), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!capture_frames_) { this->mark_failed(); return false; }
    web_server_base::global_web_server_base->add_handler(this);
  }
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

  // Select TWAI mode based on configuration. SELFTEST uses NO_ACK and
  // requests self reception; it is a bench-only diagnostic, never a vehicle mode.
  twai_mode_t twai_mode = TWAI_MODE_NORMAL;
  if (this->mode_ == CAN_MODE_LISTEN_ONLY) {
    twai_mode = TWAI_MODE_LISTEN_ONLY;
    ESP_LOGI(TAG, "CAN bus configured in LISTEN_ONLY mode (passive, no ACKs)");
  } else if (this->mode_ == CAN_MODE_SELF_TEST) {
    twai_mode = TWAI_MODE_NO_ACK;
    ESP_LOGW(TAG, "CAN SELFTEST active: disconnect transceiver and vehicle; internal TX/RX loopback only");
  }

  const gpio_num_t tx_pin = static_cast<gpio_num_t>(this->tx_);
  // Espressif legacy TWAI self-test maps TX and RX to the same GPIO and sets
  // the self-reception request flag on each test frame.
  const gpio_num_t rx_pin = this->mode_ == CAN_MODE_SELF_TEST ? tx_pin : static_cast<gpio_num_t>(this->rx_);
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(tx_pin, rx_pin, twai_mode);
  g_config.controller_id = next_twai_ctrl_num++;
  // The legacy TWAI driver only enables the GPIO input; unlike the newer
  // esp_driver_twai node driver, it does not add a pull-up. Hold RX recessive
  // when the transceiver is not connected during USB bench work. In SELFTEST
  // the shared TX/RX pin is actively driven, so do not configure its pull mode.
  if (this->mode_ != CAN_MODE_SELF_TEST) {
    const esp_err_t rx_pull_error = gpio_set_pull_mode(rx_pin, GPIO_PULLUP_ONLY);
    if (rx_pull_error != ESP_OK) {
      ESP_LOGW(TAG, "Could not enable CAN RX pull-up on GPIO%d: %s", this->rx_, esp_err_to_name(rx_pull_error));
    } else {
      ESP_LOGI(TAG, "CAN RX pull-up enabled on GPIO%d", this->rx_);
    }
  }
  if (this->tx_queue_len_.has_value()) {
    g_config.tx_queue_len = this->tx_queue_len_.value();
  }
  if (this->rx_queue_len_.has_value()) {
    g_config.rx_queue_len = this->rx_queue_len_.value();
  }

  // Accept all frames for passive bus-activity detection. The loop below
  // applies rx_filter_id_ before allocating payloads or invoking data callbacks.
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  if (this->rx_filter_id_.has_value()) {
    ESP_LOGI(TAG, "Passive activity detection enabled; payload callbacks filtered to 0x%03" PRIX32,
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
  // Allocate the small callback payload once. Reusing it avoids a heap
  // allocation/free for every ECU response frame at CAN bus speed.
  this->rx_payload_buffer_.reserve(canbus::CAN_MAX_DATA_LENGTH);
  return true;
}

void ESP32CanRecover::loop() {
  if (!this->enabled_) return;

  // Process a small bounded batch per ESPHome loop pass. TWAI accepts all IDs
  // for passive bus-activity detection, while only 0x7E8 is forwarded to UDS.
  // Returning frequently gives LVGL and display flushing predictable CPU time
  // when the vehicle bus is busy. ISO-TP replies continue on later loop passes.
  constexpr uint8_t MAX_RX_FRAMES_PER_LOOP = 24;
  // Also bound wall time, not just frame count. The check runs between frames,
  // so one callback can exceed the budget; a sustained burst still yields to
  // later ESPHome components (including LVGL) at the next frame boundary.
  constexpr uint32_t MAX_RX_BUDGET_US = 500;
  const uint32_t rx_loop_started_us = micros();
  struct canbus::CanFrame can_message;
  uint8_t processed = 0;
  while (processed < MAX_RX_FRAMES_PER_LOOP) {
    // Do at least one nonblocking receive attempt, then stop when the time
    // budget is exhausted so a continuously busy bus cannot monopolize loop().
    if (processed != 0 && static_cast<uint32_t>(micros() - rx_loop_started_us) >= MAX_RX_BUDGET_US) break;
    if (this->read_message(&can_message) != canbus::ERROR_OK) break;
    processed++;
    if (capture_enabled_) {
      std::unique_lock<std::mutex> lock(capture_mutex_, std::try_to_lock);
      if (lock.owns_lock() && capture_active_) {
        const uint32_t now = millis();
        if (capture_count_ >= CAPTURE_CAPACITY || now - capture_started_ >= 120000) {
          capture_active_ = false;
        } else {
          auto &f = capture_frames_[capture_count_++];
          f.ms = now - capture_started_; f.id = can_message.can_id;
          f.flags = (can_message.use_extended_id ? 1 : 0) | (can_message.remote_transmission_request ? 2 : 0);
          f.dlc = std::min<uint8_t>(can_message.can_data_length_code, 8);
          memcpy(f.data, can_message.data, f.dlc);
        }
      } else if (!lock.owns_lock()) {
        capture_skipped_.fetch_add(1, std::memory_order_relaxed);
      }
      // Never invoke UDS callbacks: they can transmit ISO-TP Flow Control.
      continue;
    }
    if (this->mode_ == CAN_MODE_SELF_TEST) this->self_test_rx_count_++;
    const bool standard_data_frame = !can_message.use_extended_id && !can_message.remote_transmission_request;
    if (standard_data_frame) this->activity_callback_manager_.call();

    // Skip unrelated frames before allocating payload vectors or calling the app.
    if (this->rx_filter_id_.has_value() &&
        (!standard_data_frame || can_message.can_id != this->rx_filter_id_.value())) {
      continue;
    }

    auto &data = this->rx_payload_buffer_;
    data.clear();
    const size_t length = std::min<size_t>(can_message.can_data_length_code, canbus::CAN_MAX_DATA_LENGTH);
    for (size_t i = 0; i < length; i++) data.push_back(can_message.data[i]);
    this->callback_manager_(can_message.can_id, can_message.use_extended_id, can_message.remote_transmission_request,
                            data);
  }
  this->rx_frames_since_report_ += processed;
  if (processed > this->max_rx_batch_since_report_) this->max_rx_batch_since_report_ = processed;
  const uint32_t rx_loop_duration_us = micros() - rx_loop_started_us;
  if (rx_loop_duration_us > this->max_rx_loop_us_since_report_)
    this->max_rx_loop_us_since_report_ = rx_loop_duration_us;

  if (this->twai_handle_ == nullptr) return;

  const uint32_t now = millis();
  if (now - this->last_state_check_ms_ < 250) return;
  this->last_state_check_ms_ = now;

  twai_status_info_t status{};
  if (twai_get_status_info_v2(this->twai_handle_, &status) != ESP_OK) return;

  if (this->mode_ == CAN_MODE_SELF_TEST &&
      (this->self_test_last_report_ms_ == 0 || now - this->self_test_last_report_ms_ >= 5000)) {
    const uint32_t internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    const uint32_t internal_largest = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    const uint32_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    const uint32_t psram_largest = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "SELFTEST: up=%" PRIu32 "ms RX=%" PRIu32 "/5s, batch=%" PRIu32 ", max loop=%" PRIu32
                  " us; queued RX=%" PRIu32 ", queued TX=%" PRIu32
                  ", RX missed=%" PRIu32 ", TX failed=%" PRIu32 ", RX errors=%u, TX errors=%u, bus errors=%" PRIu32
                  "; heap=%" PRIu32 "/%" PRIu32 " B, PSRAM=%" PRIu32 "/%" PRIu32 " B",
             now, this->self_test_rx_count_, this->max_rx_batch_since_report_, this->max_rx_loop_us_since_report_,
             status.msgs_to_rx, status.msgs_to_tx, status.rx_missed_count,
             status.tx_failed_count, static_cast<unsigned>(status.rx_error_counter),
             static_cast<unsigned>(status.tx_error_counter), status.bus_error_count,
             internal_free, internal_largest, psram_free, psram_largest);
    this->self_test_rx_count_ = 0;
    this->rx_frames_since_report_ = 0;
    this->max_rx_batch_since_report_ = 0;
    this->max_rx_loop_us_since_report_ = 0;
    this->self_test_last_report_ms_ = now;
  } else if (this->mode_ == CAN_MODE_NORMAL &&
             (this->last_normal_report_ms_ == 0 || now - this->last_normal_report_ms_ >= 5000)) {
    const uint32_t internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    const uint32_t internal_largest = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    const uint32_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    const uint32_t psram_largest = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "CAN: up=%" PRIu32 "ms state=%u RX=%" PRIu32 "/5s, batch=%" PRIu32 ", max loop=%" PRIu32
                  " us; queued RX=%" PRIu32 ", queued TX=%" PRIu32
                  ", RX missed=%" PRIu32 ", TX failed=%" PRIu32 ", RX errors=%u, TX errors=%u, bus errors=%" PRIu32
                  "; heap=%" PRIu32 "/%" PRIu32 " B, PSRAM=%" PRIu32 "/%" PRIu32 " B",
             now, static_cast<unsigned>(status.state), this->rx_frames_since_report_, this->max_rx_batch_since_report_,
             this->max_rx_loop_us_since_report_, status.msgs_to_rx, status.msgs_to_tx, status.rx_missed_count,
             status.tx_failed_count, static_cast<unsigned>(status.rx_error_counter),
             static_cast<unsigned>(status.tx_error_counter), status.bus_error_count,
             internal_free, internal_largest, psram_free, psram_largest);
    this->rx_frames_since_report_ = 0;
    this->max_rx_batch_since_report_ = 0;
    this->max_rx_loop_us_since_report_ = 0;
    this->last_normal_report_ms_ = now;
  }

  if (status.state == TWAI_STATE_BUS_OFF) {
    // A queued diagnostic request cannot succeed once TWAI has gone bus-off.
    // Keeping it through recovery causes the controller to retransmit the
    // same unacknowledged frame immediately after every restart, rapidly
    // increasing bus-error counts on a disconnected/quiet bench setup.
    if (status.msgs_to_tx > 0) {
      const esp_err_t clear_error = twai_clear_transmit_queue_v2(this->twai_handle_);
      if (clear_error == ESP_OK) {
        ESP_LOGW(TAG, "Cleared %u stale CAN TX frame(s) before bus-off recovery",
                 static_cast<unsigned>(status.msgs_to_tx));
      } else if (clear_error != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "Could not clear CAN TX queue before recovery: %s", esp_err_to_name(clear_error));
      }
    }
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
  message.self = this->mode_ == CAN_MODE_SELF_TEST;
  // On a quiet/unconnected bus, normal TWAI mode retransmits an unacknowledged
  // frame indefinitely. That can flood the bus-error counter and keep a stale
  // diagnostic request active. In quiet mode, make each attempt single-shot;
  // after real traffic is observed, use normal CAN retransmission semantics.
  constexpr uint32_t CAN_QUIET_WINDOW_MS = 2000;
  const uint32_t last_rx_age_ms = millis() - this->last_can_rx_ms_;
  message.ss = this->single_shot_when_quiet_ && this->mode_ == CAN_MODE_NORMAL &&
               (!this->has_received_can_frame_ || last_rx_age_ms >= CAN_QUIET_WINDOW_MS);
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

  this->has_received_can_frame_ = true;
  this->last_can_rx_ms_ = millis();

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

bool ESP32CanRecover::canHandle(web_server_idf::AsyncWebServerRequest *request) const {
  char url[web_server_idf::AsyncWebServerRequest::URL_BUF_SIZE];
  const auto path = request->url_to(url);
  return capture_enabled_ && ((request->method() == HTTP_GET && (path == "/dpf/can.csv" || path == "/dpf/can-status")) ||
      (request->method() == HTTP_POST && (path == "/dpf/can-start" || path == "/dpf/can-stop")));
}

void ESP32CanRecover::handleRequest(web_server_idf::AsyncWebServerRequest *request) {
  char url[web_server_idf::AsyncWebServerRequest::URL_BUF_SIZE];
  const auto path = request->url_to(url);
  std::unique_lock<std::mutex> lock(capture_mutex_);
  if (capture_active_ && millis() - capture_started_ >= 120000) capture_active_ = false;
  if (path == "/dpf/can-start") {
    if (!capture_frames_ || !twai_handle_) { lock.unlock(); request->send(503, "text/plain", "CAN recorder unavailable"); return; }
    twai_status_info_t status{};
    twai_get_status_info_v2(twai_handle_, &status);
    capture_rx_missed_start_ = status.rx_missed_count;
    capture_count_ = 0; capture_started_ = millis(); capture_active_ = true;
    capture_skipped_.store(0);
    lock.unlock(); request->send(200, "text/plain", "Recording started: maximum 120 seconds or 65536 frames"); return;
  }
  if (path == "/dpf/can-stop") {
    capture_active_ = false;
    lock.unlock(); request->send(200, "text/plain", "Recording stopped; download CAN CSV before next start/reset"); return;
  }
  if (path == "/dpf/can-status") {
    char status[160];
    snprintf(status, sizeof(status), "{\"active\":%s,\"frames\":%u,\"capacity\":65536,\"limit_seconds\":120}",
        capture_active_ ? "true" : "false", static_cast<unsigned>(capture_count_));
    lock.unlock(); request->send(200, "application/json", status); return;
  }
  if (capture_active_) { lock.unlock(); request->send(409, "text/plain", "Stop recording before download"); return; }
  const size_t capacity = capture_count_ * 56 + 1024;
  auto *snapshot = static_cast<char *>(heap_caps_malloc(capacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (!snapshot) { lock.unlock(); request->send(503, "text/plain", "Insufficient PSRAM for CSV snapshot"); return; }
  twai_status_info_t status{};
  twai_get_status_info_v2(twai_handle_, &status);
  size_t used = snprintf(snapshot, capacity, "# VAG DPF passive CAN capture; bitrate=500000; LISTEN_ONLY; no ACK/TX\n"
      "# RAM only; timestamps relative to recording start; frames=%u; driver_rx_missed=%lu; skipped_while_locked=%lu; capacity=65536; limit=120s\n"
      "# Capture includes all received CAN IDs; diagnostic traffic may include vehicle identifiers\n"
      "ms,id_hex,extended,rtr,dlc,data_hex\n", static_cast<unsigned>(capture_count_),
      static_cast<unsigned long>(status.rx_missed_count - capture_rx_missed_start_), static_cast<unsigned long>(capture_skipped_.load()));
  for (size_t i = 0; i < capture_count_; ++i) {
    const auto &f = capture_frames_[i];
    used += snprintf(snapshot + used, capacity - used, "%lu,%08lX,%u,%u,%u,", static_cast<unsigned long>(f.ms),
        static_cast<unsigned long>(f.id), f.flags & 1, (f.flags >> 1) & 1, f.dlc);
    for (uint8_t j = 0; j < f.dlc && !(f.flags & 2); ++j)
      used += snprintf(snapshot + used, capacity - used, "%02X", f.data[j]);
    snapshot[used++] = '\n';
  }
  lock.unlock();
  auto *response = request->beginResponse(200, "text/csv", reinterpret_cast<const uint8_t *>(snapshot), used);
  response->addHeader("Content-Disposition", "attachment; filename=monitor-dpf-can.csv");
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
  heap_caps_free(snapshot);
}

}  // namespace esphome::esp32_can_recover

#endif
