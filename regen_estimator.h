#pragma once
#include <cmath>
#include <cstdint>

// Heuristic, not an ECU regeneration flag. Thresholds need vehicle calibration.
class RegenEstimator {
 public:
  void reset() { valid_ = active_ = hot_pending_ = cool_pending_ = false; }
  void update(uint32_t now, bool fresh, float temperature) {
    if (!fresh || !std::isfinite(temperature) || temperature < -40 || temperature > 1200) {
      reset(); return;
    }
    valid_ = true;
    if (!active_) {
      cool_pending_ = false;
      if (temperature >= 550) {
        if (!hot_pending_) { hot_pending_ = true; hot_since_ = now; }
        if (uint32_t(now - hot_since_) >= 30000) { active_ = true; hot_pending_ = false; }
      } else hot_pending_ = false;
    } else {
      hot_pending_ = false;
      if (temperature < 450) {
        if (!cool_pending_) { cool_pending_ = true; cool_since_ = now; }
        if (uint32_t(now - cool_since_) >= 60000) { active_ = false; cool_pending_ = false; }
      } else cool_pending_ = false;
    }
  }
  bool valid() const { return valid_; }
  bool active() const { return valid_ && active_; }
 private:
  bool valid_{false}, active_{false}, hot_pending_{false}, cool_pending_{false};
  uint32_t hot_since_{0}, cool_since_{0};
};
