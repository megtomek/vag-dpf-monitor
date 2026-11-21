#include "ft3168_touchscreen.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::ft3168 {

static const char *const TAG = "ft3168.touchscreen";
static constexpr uint8_t FT3168_MODE_REG = 0x00;
static constexpr uint8_t FT3168_TOUCH_STATUS_REG = 0x02;
static constexpr uint8_t FT3168_DEVICE_ID_REG = 0xA0;
static constexpr uint8_t FT3168_DEVICE_ID = 0x03;
static constexpr uint8_t FT3168_MODE_NORMAL = 0x00;
static constexpr uint32_t FT3168_RETRY_INTERVAL_MS = 2000;

void FT3168Touchscreen::setup() {
  if (this->display_ != nullptr) {
    if (this->x_raw_max_ == this->x_raw_min_)
      this->x_raw_max_ = this->display_->get_native_width();
    if (this->y_raw_max_ == this->y_raw_min_)
      this->y_raw_max_ = this->display_->get_native_height();
  }

  // FT3168 datasheet allows up to 70 ms after reset. Use a little margin and
  // follow Waveshare's driver: write only normal mode (register 0x00 = 0).
  // Do not force Monitor mode; it is unnecessary for this always-on UI and
  // can make shared-bus communication fragile.
  delay(150);
  this->last_init_attempt_ms_ = millis();
  this->last_i2c_stats_log_ms_ = millis();
  if (!this->initialize_controller_()) {
    this->status_set_warning("FT3168 not responding; retrying over I2C");
    ESP_LOGW(TAG, "FT3168 did not acknowledge address 0x%02X; will retry every %u ms",
             static_cast<unsigned>(this->address_), static_cast<unsigned>(FT3168_RETRY_INTERVAL_MS));
  }
}

bool FT3168Touchscreen::initialize_controller_() {
  // The official Waveshare example only performs this mode-register write.
  if (!this->write_byte(FT3168_MODE_REG, FT3168_MODE_NORMAL))
    return false;

  // The ID read is diagnostic only. A controller that accepted the normal
  // mode command is considered ready even if this optional read is absent.
  uint8_t chip_id = 0xFF;
  if (this->read_byte(FT3168_DEVICE_ID_REG, &chip_id)) {
    ESP_LOGI(TAG, "FT3168 device ID 0xA0 = 0x%02X%s", static_cast<unsigned>(chip_id),
             chip_id == FT3168_DEVICE_ID ? " (FT3168)" : " (unexpected)");
  }

  this->controller_initialized_ = true;
  this->status_clear_warning();
  ESP_LOGI(TAG, "FT3168 ready in normal mode at I2C address 0x%02X", static_cast<unsigned>(this->address_));
  return true;
}

void FT3168Touchscreen::update_touches() {
  if (!this->controller_initialized_) {
    const uint32_t now = millis();
    if (now - this->last_init_attempt_ms_ < FT3168_RETRY_INTERVAL_MS)
      return;

    this->last_init_attempt_ms_ = now;
    if (!this->initialize_controller_() && now - this->last_init_log_ms_ >= 10000) {
      ESP_LOGW(TAG, "FT3168 still not responding at 0x%02X; automatic retry continues",
               static_cast<unsigned>(this->address_));
      this->last_init_log_ms_ = now;
    }
    return;
  }

  // Summarize rare I2C recoveries at INFO level without logging every poll.
  // The I2C transaction timeout can block LVGL, so these counts help correlate
  // touchscreen bus errors with occasional long UI-loop operations.
  const uint32_t stats_now = millis();
  if (stats_now - this->last_i2c_stats_log_ms_ >= 60000) {
    ESP_LOGI(TAG, "I2C/60s: retries status=%u coordinates=%u unrecovered=%u; slow(>=20ms)=%u max=%u ms",
             static_cast<unsigned>(this->status_retries_in_window_),
             static_cast<unsigned>(this->coordinate_retries_in_window_),
             static_cast<unsigned>(this->i2c_failures_in_window_),
             static_cast<unsigned>(this->slow_i2c_transactions_in_window_),
             static_cast<unsigned>(this->max_i2c_transaction_ms_in_window_));
    this->status_retries_in_window_ = 0;
    this->coordinate_retries_in_window_ = 0;
    this->i2c_failures_in_window_ = 0;
    this->slow_i2c_transactions_in_window_ = 0;
    this->max_i2c_transaction_ms_in_window_ = 0;
    this->last_i2c_stats_log_ms_ = stats_now;
  }

  // Measure I2C wall time to determine whether rare recovered reads are NACKs
  // or long transactions that can stall LVGL's main loop.
  const auto timed_i2c_read = [this](auto &&operation) -> bool {
    const uint32_t started = millis();
    const bool success = operation();
    const uint32_t elapsed = millis() - started;
    if (elapsed >= 20)
      this->slow_i2c_transactions_in_window_++;
    if (elapsed > this->max_i2c_transaction_ms_in_window_)
      this->max_i2c_transaction_ms_in_window_ = elapsed;
    return success;
  };

  // Zgodnie z przykładem Waveshare najpierw odczytujemy sam rejestr 0x02.
  // Producent traktuje każdą wartość różną od zera jako aktywne dotknięcie;
  // nie maskujemy statusu do dolnych bitów, bo inne flagi też mogą oznaczać kontakt.
  uint8_t status = 0;
  bool status_retried = false;
  if (!timed_i2c_read([&]() { return this->read_byte(FT3168_TOUCH_STATUS_REG, &status); })) {
    // Recover from a single transient bus error without changing the normal
    // 10 ms polling cadence or dropping a short contact report.
    status_retried = true;
    this->status_retries_in_window_++;
    delay(1);
  }
  if (status_retried &&
      !timed_i2c_read([&]() { return this->read_byte(FT3168_TOUCH_STATUS_REG, &status); })) {
    this->i2c_failures_in_window_++;
    const uint32_t now = millis();
    if (now - this->last_i2c_error_log_ms_ >= 2000) {
      ESP_LOGW(TAG, "I2C read of FT3168 status at register 0x%02X failed",
               static_cast<unsigned>(FT3168_TOUCH_STATUS_REG));
      this->last_i2c_error_log_ms_ = now;
    }
    return;
  }
  if (status_retried) {
    ESP_LOGD(TAG, "FT3168 status read recovered on one I2C retry");
  }
  if (status == 0) {
    const uint32_t now = millis();
    if (this->touch_was_detected_) {
      ESP_LOGD(TAG, "Touch released (status register is now zero)");
      this->touch_was_detected_ = false;
    }
    // Periodic idle status proves the device is being polled even when it
    // reports no active contact.
    if (now - this->last_status_log_ms_ >= 2000) {
      ESP_LOGD(TAG, "Touch status register 0x%02X = 0 (no contact)",
               static_cast<unsigned>(FT3168_TOUCH_STATUS_REG));
      this->last_status_log_ms_ = now;
    }
    return;
  }

  // Współrzędne są w czterech kolejnych rejestrach: X high/low, Y high/low.
  // Osobna transakcja odpowiada sekwencji odczytu z przykładu producenta.
  uint8_t coordinates[4]{};
  bool coordinates_retried = false;
  if (!timed_i2c_read([&]() { return this->read_bytes(0x03, coordinates, sizeof(coordinates)); })) {
    coordinates_retried = true;
    this->coordinate_retries_in_window_++;
    delay(1);
  }
  if (coordinates_retried &&
      !timed_i2c_read([&]() { return this->read_bytes(0x03, coordinates, sizeof(coordinates)); })) {
    this->i2c_failures_in_window_++;
    const uint32_t now = millis();
    if (now - this->last_i2c_error_log_ms_ >= 2000) {
      ESP_LOGW(TAG, "I2C coordinate read of FT3168 failed at register 0x03");
      this->last_i2c_error_log_ms_ = now;
    }
    return;
  }
  if (coordinates_retried) {
    ESP_LOGD(TAG, "FT3168 coordinate read recovered on one I2C retry");
  }

  // The Waveshare FT3168 reference driver reports one touch as two 12-bit values.
  uint16_t x = static_cast<uint16_t>(((coordinates[0] & 0x0F) << 8) | coordinates[1]);
  uint16_t y = static_cast<uint16_t>(((coordinates[2] & 0x0F) << 8) | coordinates[3]);
  const uint32_t now = millis();
  if (!this->touch_was_detected_ || now - this->last_touch_log_ms_ >= 250) {
    ESP_LOGD(TAG, "Touch detected: status=0x%02X raw=(%u,%u), bytes=%02X %02X %02X %02X",
             static_cast<unsigned>(status), static_cast<unsigned>(x), static_cast<unsigned>(y),
             static_cast<unsigned>(coordinates[0]), static_cast<unsigned>(coordinates[1]),
             static_cast<unsigned>(coordinates[2]), static_cast<unsigned>(coordinates[3]));
    this->last_touch_log_ms_ = now;
  }
  this->touch_was_detected_ = true;
  this->add_raw_touch_position_(0, x, y);
}

void FT3168Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "FT3168 Touchscreen: Address 0x%02X",
                static_cast<unsigned>(this->address_));
}

}  // namespace esphome::ft3168
