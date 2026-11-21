#include "ft3168_touchscreen.h"

#include "esphome/core/log.h"

namespace esphome::ft3168 {

static const char *const TAG = "ft3168.touchscreen";
static constexpr uint8_t FT3168_MODE_REG = 0x00;
static constexpr uint8_t FT3168_TOUCH_STATUS_REG = 0x02;
static constexpr uint8_t FT3168_TOUCH_DATA_REG = 0x03;

void FT3168Touchscreen::setup() {
  if (this->display_ != nullptr) {
    if (this->x_raw_max_ == this->x_raw_min_)
      this->x_raw_max_ = this->display_->get_native_width();
    if (this->y_raw_max_ == this->y_raw_min_)
      this->y_raw_max_ = this->display_->get_native_height();
  }

  // FT3168 operating mode: 0 is normal mode, as in Waveshare's reference driver.
  const uint8_t mode = 0;
  if (this->write_register(FT3168_MODE_REG, &mode, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to set normal operating mode");
    this->mark_failed();
  }
}

void FT3168Touchscreen::update_touches() {
  uint8_t touch_count = 0;
  if (!this->read_byte(FT3168_TOUCH_STATUS_REG, &touch_count)) {
    ESP_LOGW(TAG, "Failed to read touch status");
    return;
  }
  if (touch_count == 0)
    return;

  uint8_t data[4]{};
  if (!this->read_bytes(FT3168_TOUCH_DATA_REG, data, sizeof(data))) {
    ESP_LOGW(TAG, "Failed to read touch coordinates");
    return;
  }

  // The Waveshare FT3168 reference driver reports one touch as two 12-bit values.
  uint16_t x = static_cast<uint16_t>(((data[0] & 0x0F) << 8) | data[1]);
  uint16_t y = static_cast<uint16_t>(((data[2] & 0x0F) << 8) | data[3]);
  this->add_raw_touch_position_(0, x, y);
}

void FT3168Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "FT3168 Touchscreen:\n  Address: 0x%02X", this->address_);
}

}  // namespace esphome::ft3168
