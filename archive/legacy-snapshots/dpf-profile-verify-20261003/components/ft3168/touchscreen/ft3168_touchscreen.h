#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/component.h"

namespace esphome::ft3168 {

class FT3168Touchscreen final : public touchscreen::Touchscreen, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void update_touches() override;
};

}  // namespace esphome::ft3168
