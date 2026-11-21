#pragma once

#include <cstdint>

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/component.h"

namespace esphome::ft3168 {

class FT3168Touchscreen final : public touchscreen::Touchscreen, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void update_touches() override;

 private:
  bool initialize_controller_();

  // Rate-limit temporary diagnostics so touch polling does not flood ESPHome logs.
  uint32_t last_status_log_ms_{0};
  uint32_t last_touch_log_ms_{0};
  uint32_t last_i2c_error_log_ms_{0};
  uint32_t last_init_attempt_ms_{0};
  uint32_t last_init_log_ms_{0};
  uint32_t last_i2c_stats_log_ms_{0};
  uint32_t status_retries_in_window_{0};
  uint32_t coordinate_retries_in_window_{0};
  uint32_t i2c_failures_in_window_{0};
  uint32_t slow_i2c_transactions_in_window_{0};
  uint32_t max_i2c_transaction_ms_in_window_{0};
  bool controller_initialized_{false};
  bool touch_was_detected_{false};
};

}  // namespace esphome::ft3168
