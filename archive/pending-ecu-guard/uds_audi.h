#pragma once

// GPIO API is used for an early power-cycle of the AMOLED rail during boot.
#include "driver/gpio.h"



#include "esphome/components/binary_sensor/binary_sensor.h"

#include "esphome/components/canbus/canbus.h"

#include "esphome/components/sensor/sensor.h"

#include "esphome/components/text_sensor/text_sensor.h"

#include "esphome/core/hal.h"

#include "esphome/core/log.h"



#include <algorithm>

#include <array>

#include <cstdint>

#include <limits>

#include <string>
#include <string_view>

#include <vector>

enum class CjcdCalibrationMatch : uint8_t { UNKNOWN, MATCHED, MISMATCHED };

// Mode 09 can return several CALID records separated by ';'. Match the whole
// part number AND software version. Whitespace padding is insignificant.
constexpr bool cjcd_calibration_record_matches(std::string_view record) {
  constexpr std::string_view expected = "03L906018SJ9977";
  size_t index = 0;
  for (char ch : record) {
    if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') continue;
    if (index >= expected.size() || ch != expected[index]) return false;
    ++index;
  }
  return index == expected.size();
}

constexpr CjcdCalibrationMatch classify_cjcd_calibration(std::string_view records) {
  bool has_record = false;
  while (!records.empty()) {
    const size_t separator = records.find(';');
    const auto record = records.substr(0, separator);
    if (cjcd_calibration_record_matches(record)) return CjcdCalibrationMatch::MATCHED;
    for (char ch : record) {
      if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') has_record = true;
    }
    if (separator == std::string_view::npos) break;
    records.remove_prefix(separator + 1);
  }
  return has_record ? CjcdCalibrationMatch::MISMATCHED : CjcdCalibrationMatch::UNKNOWN;
}

// Compile-time regression cases: exact ECU, other SW, substring collisions,
// padded records, several CALIDs, and absent identity.
static_assert(classify_cjcd_calibration("03L906018SJ 9977") == CjcdCalibrationMatch::MATCHED);
static_assert(classify_cjcd_calibration(" 03L906018SJ  9977 ") == CjcdCalibrationMatch::MATCHED);
static_assert(classify_cjcd_calibration("OTHER; 03L906018SJ9977") == CjcdCalibrationMatch::MATCHED);
static_assert(classify_cjcd_calibration("03L906018SJ 9976") == CjcdCalibrationMatch::MISMATCHED);
static_assert(classify_cjcd_calibration("03L906018JL 9977") == CjcdCalibrationMatch::MISMATCHED);
static_assert(classify_cjcd_calibration("03L906018SJ99770") == CjcdCalibrationMatch::MISMATCHED);
static_assert(classify_cjcd_calibration("X03L906018SJ9977") == CjcdCalibrationMatch::MISMATCHED);
static_assert(classify_cjcd_calibration(" ; \t") == CjcdCalibrationMatch::UNKNOWN);
static_assert(classify_cjcd_calibration("") == CjcdCalibrationMatch::UNKNOWN);

// Use wrap-safe unsigned elapsed time for sensor freshness. ESPHome millis()
// wraps approximately every 49 days; a fresh reading expires at the exact
// configured boundary (elapsed < max_age_ms).
constexpr bool timestamp_is_fresh(uint32_t now_ms, uint32_t sample_ms,
                                  uint32_t max_age_ms) {
  return static_cast<uint32_t>(now_ms - sample_ms) < max_age_ms;
}
static_assert(timestamp_is_fresh(20000U, 10001U, 10000U));   // age 9,999 ms
static_assert(!timestamp_is_fresh(20000U, 10000U, 10000U));  // age 10,000 ms
static_assert(timestamp_is_fresh(3U, 0xFFFFFFFAU, 10U));    // wrap, age 9 ms
static_assert(!timestamp_is_fresh(4U, 0xFFFFFFFAU, 10U));   // wrap, age 10 ms

// Pure prefix check used before accepting a multi-frame OBD response.
constexpr bool obd_response_prefix_matches(
    const std::array<uint8_t, 2> &prefix, size_t payload_size,
    uint8_t expected_service, uint8_t expected_identifier) {
  return payload_size >= 2 && prefix[0] == expected_service &&
         prefix[1] == expected_identifier;
}

static_assert(obd_response_prefix_matches(std::array<uint8_t, 2>{{0x41, 0x7A}}, 2, 0x41, 0x7A));
static_assert(obd_response_prefix_matches(std::array<uint8_t, 2>{{0x49, 0x0A}}, 2, 0x49, 0x0A));
static_assert(!obd_response_prefix_matches(std::array<uint8_t, 2>{{0x41, 0x7B}}, 2, 0x41, 0x7A));
static_assert(!obd_response_prefix_matches(std::array<uint8_t, 2>{{0x49, 0x0A}}, 2, 0x41, 0x0A));
static_assert(!obd_response_prefix_matches(std::array<uint8_t, 2>{{0x41, 0x7A}}, 1, 0x41, 0x7A));

// This project uses classic CAN frames: ISO-TP Single Frames carry 1–7 bytes.
constexpr bool classic_can_sf_length_valid(size_t payload_length, size_t frame_size) {
  return payload_length >= 1 && payload_length <= 7 && frame_size >= 2 &&
         payload_length <= frame_size - 1;
}

static_assert(classic_can_sf_length_valid(1, 8));
static_assert(classic_can_sf_length_valid(7, 8));
static_assert(!classic_can_sf_length_valid(0, 8));
static_assert(!classic_can_sf_length_valid(8, 9));
static_assert(!classic_can_sf_length_valid(7, 7));

// Shared ISO-TP reassembly helpers. Keeping these free of CAN/UI dependencies lets
// SELFTEST exercise the exact frame assembly path used by the live OBD poller.
enum class IsoTpAssemblyResult : uint8_t { INVALID, IN_PROGRESS, COMPLETE };

bool begin_isotp_reassembly(const std::vector<uint8_t> &frame, size_t max_payload_length,
                            size_t &expected_length, uint8_t &next_sequence,
                            std::vector<uint8_t> &payload) {
  expected_length = 0;
  payload.clear();
  if (frame.size() < 2 || (frame[0] >> 4) != 0x1) return false;

  const size_t length = (static_cast<size_t>(frame[0] & 0x0F) << 8) | frame[1];
  if (length < 8 || length > max_payload_length) return false;

  const size_t initial_length = std::min<size_t>(frame.size() - 2, 6);
  payload.assign(frame.begin() + 2, frame.begin() + 2 + initial_length);
  expected_length = length;
  next_sequence = 1;
  return true;
}

IsoTpAssemblyResult append_isotp_consecutive_frame(
    const std::vector<uint8_t> &frame, size_t &expected_length,
    uint8_t &next_sequence, std::vector<uint8_t> &payload) {
  if (expected_length == 0 || frame.size() < 2 || (frame[0] >> 4) != 0x2) {
    return IsoTpAssemblyResult::INVALID;
  }

  const uint8_t sequence = frame[0] & 0x0F;
  if (sequence != next_sequence) {
    expected_length = 0;
    payload.clear();
    return IsoTpAssemblyResult::INVALID;
  }

  next_sequence = (next_sequence + 1) & 0x0F;
  const size_t remaining = expected_length - payload.size();
  const size_t count = std::min<size_t>(frame.size() - 1, remaining);
  payload.insert(payload.end(), frame.begin() + 1, frame.begin() + 1 + count);
  if (payload.size() < expected_length) return IsoTpAssemblyResult::IN_PROGRESS;
  payload.resize(expected_length);
  return IsoTpAssemblyResult::COMPLETE;
}

// One fail-safe rule is shared by runtime status, the UI alarm, and compile-time checks.
enum class RegenStatus : uint8_t { UNAVAILABLE, INACTIVE, PASSIVE, ACTIVE };
constexpr const char *regen_status_name(RegenStatus status) {
  switch (status) {
    case RegenStatus::ACTIVE: return "active";
    case RegenStatus::PASSIVE: return "passive";
    case RegenStatus::INACTIVE: return "inactive";
    case RegenStatus::UNAVAILABLE: default: return "unavailable";
  }
}
constexpr RegenStatus classify_regen_status(bool fresh, bool status_supported,
                                             bool type_supported, bool in_progress,
                                             bool active_type) {
  if (!fresh || !status_supported || !type_supported) return RegenStatus::UNAVAILABLE;
  if (!in_progress) return RegenStatus::INACTIVE;
  return active_type ? RegenStatus::ACTIVE : RegenStatus::PASSIVE;
}
static_assert(classify_regen_status(true, true, true, true, true) == RegenStatus::ACTIVE);
static_assert(classify_regen_status(true, true, true, true, false) == RegenStatus::PASSIVE);
static_assert(classify_regen_status(true, true, true, false, true) == RegenStatus::INACTIVE);
static_assert(classify_regen_status(false, true, true, true, true) == RegenStatus::UNAVAILABLE);
static_assert(classify_regen_status(true, false, true, true, true) == RegenStatus::UNAVAILABLE);
static_assert(classify_regen_status(true, true, false, true, true) == RegenStatus::UNAVAILABLE);

struct RegenHistoryTransition {
  bool ready;
  bool tracking;
  bool started;
  bool completed;
};

// Count a cycle only when an inactive ECU state was seen before its start,
// and keep a data gap from looking like a successful completion.
constexpr RegenHistoryTransition advance_regen_history(bool ready, bool tracking,
                                                        RegenStatus status) {
  if (status == RegenStatus::UNAVAILABLE) return {false, false, false, false};
  if (status == RegenStatus::INACTIVE) return {true, false, false, tracking};
  if (!ready) return {false, false, false, false};
  if (!tracking) return {true, true, true, false};
  return {true, true, false, false};
}

static_assert(!advance_regen_history(false, false, RegenStatus::ACTIVE).tracking);
static_assert(advance_regen_history(false, false, RegenStatus::INACTIVE).ready);
static_assert(advance_regen_history(true, false, RegenStatus::ACTIVE).started);
static_assert(advance_regen_history(true, false, RegenStatus::PASSIVE).started);
static_assert(advance_regen_history(true, true, RegenStatus::INACTIVE).completed);
static_assert(!advance_regen_history(true, true, RegenStatus::UNAVAILABLE).completed &&
              !advance_regen_history(true, true, RegenStatus::UNAVAILABLE).tracking &&
              !advance_regen_history(true, true, RegenStatus::UNAVAILABLE).ready);

class UdsManager {

public:

void configure(esphome::canbus::Canbus *bus,

esphome::sensor::Sensor *soot_calculated,

esphome::sensor::Sensor *soot_measured,

esphome::sensor::Sensor *distance_since_regen,

esphome::sensor::Sensor *temp_before_turbo,

esphome::sensor::Sensor *temp_before_dpf,

esphome::binary_sensor::BinarySensor *regen_active,
esphome::sensor::Sensor *engine_rpm,
esphome::sensor::Sensor *coolant_temperature,
esphome::sensor::Sensor *obd_dpf_delta_pressure,
esphome::sensor::Sensor *obd_dpf_inlet_temperature,
esphome::sensor::Sensor *obd_intake_manifold_pressure,
esphome::sensor::Sensor *obd_barometric_pressure,
esphome::sensor::Sensor *obd_estimated_boost_pressure,
esphome::sensor::Sensor *obd_egt_1,
esphome::sensor::Sensor *obd_egt_2,
esphome::sensor::Sensor *obd_egt_3,
esphome::sensor::Sensor *obd_egt_4,
esphome::text_sensor::TextSensor *obd_ecu_name,
esphome::text_sensor::TextSensor *obd_calibration_ids,
esphome::text_sensor::TextSensor *did_probe_f401,
esphome::text_sensor::TextSensor *did_probe_f402,
esphome::text_sensor::TextSensor *did_probe_f403,
esphome::text_sensor::TextSensor *did_probe_f404,
esphome::text_sensor::TextSensor *did_probe_f405,
esphome::text_sensor::TextSensor *did_probe_f406,
uint32_t tx_id = 0x7E0,
uint32_t obd_link_stale_timeout_ms = 15000,
bool did_diagnostic_probe = false,
bool cjcd_profile_selected = false) {

bus_ = bus;

soot_calculated_ = soot_calculated;

soot_measured_ = soot_measured;

distance_since_regen_ = distance_since_regen;

temp_before_turbo_ = temp_before_turbo;

temp_before_dpf_ = temp_before_dpf;

regen_active_ = regen_active;
engine_rpm_ = engine_rpm;
coolant_temperature_ = coolant_temperature;
obd_dpf_delta_pressure_ = obd_dpf_delta_pressure;
obd_dpf_inlet_temperature_ = obd_dpf_inlet_temperature;
obd_intake_manifold_pressure_ = obd_intake_manifold_pressure;
obd_barometric_pressure_ = obd_barometric_pressure;
obd_estimated_boost_pressure_ = obd_estimated_boost_pressure;
obd_egt_sensors_ = {{obd_egt_1, obd_egt_2, obd_egt_3, obd_egt_4}};
obd_ecu_name_ = obd_ecu_name;
obd_calibration_ids_ = obd_calibration_ids;
did_probe_sensors_ = {{did_probe_f401, did_probe_f402, did_probe_f403,
                       did_probe_f404, did_probe_f405, did_probe_f406}};
tx_id_ = tx_id;
obd_link_stale_timeout_ms_ = obd_link_stale_timeout_ms;
did_diagnostic_probe_ = did_diagnostic_probe;
cjcd_profile_selected_ = cjcd_profile_selected;

configured_ = (bus_ != nullptr) && (PROFILE_MAPPING_VERIFIED || did_diagnostic_probe_);
if (bus_ != nullptr && !PROFILE_MAPPING_VERIFIED && !did_diagnostic_probe_) {
ESP_LOGW(TAG, "UDS DID polling disabled: no verified profile or diagnostic probe selected");
} else if (bus_ != nullptr && !PROFILE_MAPPING_VERIFIED && did_diagnostic_probe_) {
ESP_LOGW(TAG, "UDS DID diagnostic probe enabled: candidate DIDs only; normal cards require explicit CJCD selection, no DID-based regen alarm");
}
}

// The selected CJCD profile enables only the three candidate mappings that
// were compared against OBDeleven. Regen state, distance and temperatures
// remain diagnostic-only until they are separately verified.
void set_cjcd_profile_selected(bool selected) {
  if (cjcd_profile_selected_ == selected) return;
  cjcd_profile_selected_ = selected;
  clear_cjcd_core_readings_();
}

void clear_cjcd_core_readings_() {
  for (size_t i = 0; i < parameters_.size(); ++i) {
    if (is_cjcd_core_did_(parameters_[i].did)) {
      did_seen_[i] = false;  // wait for a fresh response after profile change
    }
  }
  const float no_data = std::numeric_limits<float>::quiet_NaN();
  if (soot_calculated_ != nullptr) soot_calculated_->publish_state(no_data);
  if (soot_measured_ != nullptr) soot_measured_->publish_state(no_data);
  if (obd_dpf_delta_pressure_ != nullptr) obd_dpf_delta_pressure_->publish_state(no_data);
}

bool cjcd_profile_selected() const { return cjcd_profile_selected_; }

bool cjcd_core_enabled() const {
  return cjcd_profile_selected_ && cjcd_calibration_match_ != CjcdCalibrationMatch::MISMATCHED;
}

const char *profile_identity_status() const {
  if (!cjcd_profile_selected_) return "OBD-II: profil uniwersalny";
  if (cjcd_calibration_match_ == CjcdCalibrationMatch::MATCHED)
    return "CJCD: CALID zgodny; mapa DPF częściowa";
  if (cjcd_calibration_match_ == CjcdCalibrationMatch::MISMATCHED)
    return "CJCD: inny CALID; dane CJCD zablokowane";
  return "CJCD: brak CALID; wybór ręczny";
}

static bool is_cjcd_core_did_(uint16_t did) {
  return did == 0x114F || did == 0x114E || did == 0x14F5;
}

// Fresh means at least one recognized DID returned a valid response recently.
bool has_fresh_data(uint32_t max_age_ms) const {
const uint32_t now = esphome::millis();
for (size_t i = 0; i < parameters_.size(); i++) {
if (did_seen_[i] && timestamp_is_fresh(now, last_valid_did_ms_[i], max_age_ms)) return true;
}
return false;
}

// Track the regeneration flag separately so the UI can mark only that value stale.
bool is_did_fresh(uint16_t did, uint32_t max_age_ms) const {
const uint32_t now = esphome::millis();
for (size_t i = 0; i < parameters_.size(); i++) {
if (parameters_[i].did == did) {
return did_seen_[i] && timestamp_is_fresh(now, last_valid_did_ms_[i], max_age_ms);
}
}
return false;
}

// The standard OBD-II fallback confirms ECU communication without relying on
// vehicle-specific DPF DID. It also probes generic DPF pressure/temperature PIDs.
bool has_obd_link(uint32_t max_age_ms) const {
return obd_seen_ && timestamp_is_fresh(esphome::millis(), last_obd_ms_, max_age_ms);
}

bool is_obd_pid_fresh(uint8_t pid, uint32_t max_age_ms) const {
const uint32_t now = esphome::millis();
if (pid == 0x0C) return obd_rpm_seen_ && timestamp_is_fresh(now, obd_rpm_ms_, max_age_ms);
if (pid == 0x05) return obd_coolant_seen_ && timestamp_is_fresh(now, obd_coolant_ms_, max_age_ms);
if (pid == 0x7A) return obd_dpf_pressure_seen_ && timestamp_is_fresh(now, obd_dpf_pressure_ms_, max_age_ms);
if (pid == 0x7C) return obd_dpf_temperature_seen_ && timestamp_is_fresh(now, obd_dpf_temperature_ms_, max_age_ms);
if (pid == 0x78) return obd_egt_seen_ && timestamp_is_fresh(now, obd_egt_ms_, max_age_ms);
if (pid == 0x8B) return obd_dpf_aftertreatment_seen_ && timestamp_is_fresh(now, obd_dpf_aftertreatment_ms_, max_age_ms);
if (pid == 0x0B) return obd_intake_pressure_seen_ && timestamp_is_fresh(now, obd_intake_pressure_ms_, max_age_ms);
if (pid == 0x33) return obd_barometric_pressure_seen_ && timestamp_is_fresh(now, obd_barometric_pressure_ms_, max_age_ms);
return false;
}

bool is_obd_pid_supported(uint8_t pid) const {
if (pid == 0x7A) return obd_dpf_pressure_supported_known_ && obd_dpf_pressure_supported_;
if (pid == 0x7C) return obd_dpf_temperature_supported_known_ && obd_dpf_temperature_supported_;
if (pid == 0x8B) return obd_dpf_aftertreatment_supported_known_ && obd_dpf_regen_supported_ && obd_dpf_regen_type_supported_;
if (pid == 0x0B) return obd_intake_pressure_supported_;
if (pid == 0x78) return obd_egt_support_known_ && obd_egt_support_mask_ != 0;
return false;
}

bool is_obd_egt_fresh(uint8_t sensor_number, uint32_t max_age_ms) const {
if (sensor_number < 1 || sensor_number > 4 || !obd_egt_seen_ || !obd_egt_support_known_) return false;
const uint8_t bit = static_cast<uint8_t>(1U << (sensor_number - 1U));
return timestamp_is_fresh(esphome::millis(), obd_egt_ms_, max_age_ms) &&
       (obd_egt_support_mask_ & bit) != 0;
}

bool is_obd_pid_support_known(uint8_t pid) const {
if (pid == 0x7A) return obd_dpf_pressure_supported_known_;
if (pid == 0x7C) return obd_dpf_temperature_supported_known_;
if (pid == 0x8B) return obd_dpf_aftertreatment_supported_known_;
if (pid == 0x78) return obd_egt_support_known_;
return false;
}

// Human-readable support/freshness state for the standalone ECU screen.
// No response is never treated as proof that a PID is unsupported.
std::string obd_pid_data_status(uint8_t pid, uint32_t max_age_ms) const {
bool seen = false;
switch (pid) {
case 0x7A: seen = obd_dpf_pressure_seen_; break;
case 0x7C: seen = obd_dpf_temperature_seen_; break;
case 0x78: seen = obd_egt_seen_; break;
case 0x8B: seen = obd_dpf_aftertreatment_seen_; break;
default: return "?";
}

if (is_obd_pid_fresh(pid, max_age_ms)) {
if (pid == 0x8B) {
const std::string status = obd_pid_support_status(pid);
if (status == "supported") return "OK";
if (status == "type unsupported") return "CZESC";
if (status == "unsupported" || status == "not advertised") return "BRAK";
}
if (is_obd_pid_support_known(pid)) return is_obd_pid_supported(pid) ? "OK" : "BRAK";
return "ODP";
}
if (seen) return "STARE";

const int pid_index = obd_data_pid_index_(pid);
bool bitmap_known = false;
const bool advertised = obd_pid_advertised_(pid, &bitmap_known);
if (pid_index >= 0 && bitmap_known && !advertised &&
    obd_pid_fallback_timeouts_[pid_index] >= OBD_BITMAP_FALLBACK_TIMEOUTS) return "BRAK";
return "OCZEK.";
}

std::string obd_dpf_pid_summary(uint32_t max_age_ms) const {
return "PID 7A " + obd_pid_data_status(0x7A, max_age_ms) +
       "   7C " + obd_pid_data_status(0x7C, max_age_ms) +
       "   78 " + obd_pid_data_status(0x78, max_age_ms) +
       "   8B " + obd_pid_data_status(0x8B, max_age_ms);
}

// Distinguish an unanswered PID from one absent in a valid support bitmap.
// The bitmap is only reported as "not advertised" after the direct fallback
// has also timed out twice; a real PID response always takes precedence.
std::string obd_pid_support_status(uint8_t pid) const {
if (pid != 0x8B) return "unknown";
if (obd_dpf_aftertreatment_supported_known_) {
if (!obd_dpf_regen_supported_) return "unsupported";
return obd_dpf_regen_type_supported_ ? "supported" : "type unsupported";
}
const int pid_index = obd_data_pid_index_(pid);
bool bitmap_known = false;
const bool advertised = obd_pid_advertised_(pid, &bitmap_known);
if (pid_index >= 0 && bitmap_known && !advertised &&
    obd_pid_fallback_timeouts_[pid_index] >= OBD_BITMAP_FALLBACK_TIMEOUTS) {
return "not advertised";
}
return "unknown";
}

std::string regen_data_source(uint32_t max_age_ms) const {
if (is_did_fresh(0xF401, max_age_ms)) return "UDS DID 0xF401";
if (is_obd_pid_fresh(0x8B, max_age_ms) && is_obd_pid_supported(0x8B)) return "OBD-II PID 0x8B";
return "unavailable";
}

// Regeneration is available only from a fresh, explicitly supported standard
// OBD field or a verified vehicle-specific UDS DID.
bool has_fresh_regen_data(uint32_t max_age_ms) const {
return is_did_fresh(0xF401, max_age_ms) ||
       obd_regen_state_(max_age_ms) != RegenStatus::UNAVAILABLE;
}



// Return an explicit operating state. PID 0x8B alone is useful for the active
// warning only when the ECU supports both the regen-status and regen-type bits.
std::string regen_operating_state(uint32_t max_age_ms) const {
if (is_did_fresh(0xF401, max_age_ms)) return uds_regen_active_ ? "active" : "inactive";
return regen_status_name(obd_regen_state_(max_age_ms));
}
// Called from canbus.on_frame for RX ID 0x7E8.

void notify_can_activity() {
const uint32_t now = esphome::millis();

// A single decoded frame on a floating or noisy RX line is not enough to
// classify the vehicle bus as active for fast polling. Initial OBD probes
// are still sent at a slow, bounded cadence so a quiet diagnostic bus can
// receive the request that causes the ECU to reply.
if (can_activity_window_start_ms_ == 0 ||
    static_cast<uint32_t>(now - can_activity_window_start_ms_) > CAN_ACTIVITY_CONFIRM_WINDOW_MS) {
can_activity_window_start_ms_ = now;
can_activity_frames_in_window_ = 0;
}
if (can_activity_frames_in_window_ < CAN_ACTIVITY_CONFIRM_FRAMES) {
++can_activity_frames_in_window_;
}
if (can_activity_frames_in_window_ >= CAN_ACTIVITY_CONFIRM_FRAMES) {
last_can_activity_ms_ = now;
can_activity_seen_ = true;
// Fresh bus traffic means a vehicle/network peer is present. Resume the
// candidate poller early if it backed off after a silent cycle.
if (did_diagnostic_probe_) cycle_delay_until_ms_ = now;
}
}

// Keep the diagnostic page explicit about bus traffic. This reports only
// observed standard CAN activity; it does not claim that the ECU answered a DID.
std::string can_activity_status() const {
  if (!can_activity_seen_) return "CAN: BRAK RUCHU";
  if (timestamp_is_fresh(esphome::millis(), last_can_activity_ms_, CAN_ACTIVITY_TIMEOUT_MS)) {
    return "CAN: RUCH WYKRYTY";
  }
  return "CAN: CISZA";
}

// Cancel software state when switching to UI-only/demo mode. The CAN component
// also clears the hardware TX queue so a frame cannot keep retrying in TWAI.
void cancel_pending_obd_request() {
obd_waiting_ = false;
obd_rx_expected_length_ = 0;
obd_response_.clear();
obd_identity_active_type_ = 0;
obd_tx_retry_after_ms_ = 0;
obd_next_query_ms_ = esphome::millis();
}

void handle_rx(const std::vector<uint8_t> &data) {

if (data.empty()) return;

// Route the one in-flight transaction by its owner. Diagnostic comparison
// mode alternates UDS candidate DIDs and standard Mode 01/09 OBD requests;
// never let an OBD reply enter the UDS ISO-TP assembler (or vice versa).
if (configured_ && did_diagnostic_probe_ && obd_waiting_) {
  did_probe_cycle_received_frame_ = true;
  handle_obd_probe_response_(data);
  return;
}

if (!configured_) {
handle_obd_probe_response_(data);
return;
}

if (did_diagnostic_probe_) {
  if (!waiting_for_response_) return;
  did_probe_cycle_received_frame_ = true;
  did_probe_query_received_frame_ = true;
}



switch (data[0] >> 4) {

case 0x0: // ISO-TP Single Frame

handle_single_frame_(data);

break;

case 0x1: // ISO-TP First Frame

handle_first_frame_(data);

break;

case 0x2: // ISO-TP Consecutive Frame

handle_consecutive_frame_(data);

break;

case 0x3: // Flow Control from ECU; not used by this tester

default:

break;

}

}


// Run against an isolated manager instance so a bench test never publishes
// synthetic values to the live UI or changes the production parser state.
static bool run_obd_parser_self_test() {
UdsManager parser;
parser.obd_waiting_ = true;
parser.current_obd_pid_ = 0x0B;

// A stale multi-frame reply for a different PID must be rejected before the
// parser tries to send Flow Control (the isolated instance has no CAN bus).
parser.handle_obd_probe_response_(std::vector<uint8_t>{0x10, 0x08, 0x41, 0x05, 0x00, 0x00, 0x00, 0x00});
const bool unrelated_first_frame_rejected = parser.obd_rx_expected_length_ == 0 &&
                                            parser.obd_response_.empty();

// A Single Frame for an unrelated PID or service must be rejected before
// it can leave stale bytes in the response buffer or populate a sensor.
parser.handle_obd_probe_response_(std::vector<uint8_t>{0x03, 0x41, 0x05, 0x64});
const bool unrelated_pid_rejected = !parser.obd_intake_pressure_seen_ &&
                                    parser.obd_response_.empty() && parser.obd_waiting_;
parser.handle_obd_probe_response_(std::vector<uint8_t>{0x03, 0x49, 0x0B, 0x64});
const bool unrelated_service_rejected = !parser.obd_intake_pressure_seen_ &&
                                        parser.obd_response_.empty() && parser.obd_waiting_;

// A valid response for the pending PID should decode 100 kPa and finish the
// request without a physical sensor, display, or transceiver attached.
parser.handle_obd_probe_response_(std::vector<uint8_t>{0x03, 0x41, 0x0B, 0x64});
const bool valid_single_frame_accepted = parser.obd_intake_pressure_seen_ &&
    parser.obd_intake_pressure_supported_ && parser.obd_intake_pressure_kpa_ == 100.0f &&
    !parser.obd_waiting_;

// Feed a real-shaped multi-frame PID 0x7A response through the same frame
// handler and decoder used by the CAN poller, without a bus or live sensors.
// A=0x01 advertises delta pressure; B:C=0x0064 represents 1.00 kPa.
UdsManager multi;
multi.obd_waiting_ = true;
multi.current_obd_pid_ = 0x7A;
const std::vector<uint8_t> dpf_first_frame{0x10, 0x09, 0x41, 0x7A, 0x01, 0x00, 0x64, 0x00};
multi.handle_obd_probe_response_(dpf_first_frame, false);
const bool dpf_first_frame_accepted = multi.obd_rx_expected_length_ == 9 &&
    multi.obd_response_.size() == 6 && multi.obd_response_[0] == 0x41 &&
    multi.obd_response_[1] == 0x7A;
multi.handle_obd_probe_response_(
    std::vector<uint8_t>{0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, false);
const bool dpf_multiframe_reassembled =
    dpf_first_frame_accepted &&
    multi.obd_rx_expected_length_ == 0 &&
    multi.obd_response_.empty() && multi.obd_dpf_pressure_seen_;
const bool dpf_multiframe_decoded = multi.obd_dpf_pressure_seen_ &&
    multi.obd_dpf_pressure_supported_ && multi.obd_dpf_delta_pressure_kpa_ == 1.0f &&
    !multi.obd_waiting_;

// A bad sequence number must discard partial data instead of decoding it.
size_t bad_sequence_expected = 9;
uint8_t bad_sequence_next = 1;
std::vector<uint8_t> bad_sequence_payload{0x41, 0x7A, 0x01, 0x00, 0x64, 0x00};
const IsoTpAssemblyResult bad_sequence_result = append_isotp_consecutive_frame(
    std::vector<uint8_t>{0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    bad_sequence_expected, bad_sequence_next, bad_sequence_payload);
const bool bad_sequence_discarded = bad_sequence_result == IsoTpAssemblyResult::INVALID &&
    bad_sequence_expected == 0 && bad_sequence_payload.empty();
// A final Consecutive Frame may be padded; bytes beyond the declared ISO-TP
// payload must never leak into the decoded OBD response.
size_t padded_expected = 0;
uint8_t padded_next = 1;
std::vector<uint8_t> padded_payload;
const bool padded_started = begin_isotp_reassembly(
    std::vector<uint8_t>{0x10, 0x08, 0x41, 0x7A, 0x01, 0x00, 0x64, 0x00},
    MAX_ISOTP_PAYLOAD, padded_expected, padded_next, padded_payload);
const IsoTpAssemblyResult padded_result = append_isotp_consecutive_frame(
    std::vector<uint8_t>{0x21, 0x7F, 0x01, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA},
    padded_expected, padded_next, padded_payload);
const bool final_padding_trimmed = padded_started &&
    padded_result == IsoTpAssemblyResult::COMPLETE && padded_payload.size() == 8 &&
    padded_payload[6] == 0x7F && padded_payload[7] == 0x01;

// Exercise the 4-bit ISO-TP sequence rollover 0xF -> 0x0 across 16 CFs.
size_t wrapped_expected = 0;
uint8_t wrapped_next = 0;
std::vector<uint8_t> wrapped_payload;
const bool wrapped_started = begin_isotp_reassembly(
    std::vector<uint8_t>{0x10, 0x76, 0x41, 0x8B, 0x01, 0x03, 0x00, 0x00},
    MAX_ISOTP_PAYLOAD, wrapped_expected, wrapped_next, wrapped_payload);
IsoTpAssemblyResult wrapped_result = IsoTpAssemblyResult::INVALID;
for (uint8_t frame_index = 0; frame_index < 16; ++frame_index) {
  std::vector<uint8_t> consecutive_frame(8, 0xAA);
  consecutive_frame[0] = static_cast<uint8_t>(0x20U | ((frame_index + 1U) & 0x0FU));
  wrapped_result = append_isotp_consecutive_frame(
      consecutive_frame, wrapped_expected, wrapped_next, wrapped_payload);
  if (wrapped_result == IsoTpAssemblyResult::INVALID) break;
}
const bool sequence_rollover_reassembled = wrapped_started &&
    wrapped_result == IsoTpAssemblyResult::COMPLETE && wrapped_payload.size() == 118 &&
    wrapped_next == 1;

// Decode raw multi-frame PID 0x8B bytes through the production decoder. This
// checks byte A support flags and byte B active/passive/inactive bits together.
const auto check_multiframe_regen = [](uint8_t support_mask, uint8_t status_byte,
                                       const char *expected_state) {
  UdsManager raw_regen;
  raw_regen.obd_waiting_ = true;
  raw_regen.current_obd_pid_ = 0x8B;
  const std::vector<uint8_t> first_frame{
      0x10, 0x09, 0x41, 0x8B, support_mask, status_byte, 0x00, 0x00};
  raw_regen.handle_obd_probe_response_(first_frame, false);
  if (raw_regen.obd_rx_expected_length_ != 9 || raw_regen.obd_response_.size() != 6) {
    return false;
  }
  raw_regen.handle_obd_probe_response_(
      std::vector<uint8_t>{0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, false);
  if (raw_regen.obd_rx_expected_length_ != 0 || raw_regen.obd_response_.size() != 0) return false;
  return raw_regen.regen_operating_state(15000) == expected_state;
};
const bool raw_multiframe_active = check_multiframe_regen(0x03, 0x03, "active");
const bool raw_multiframe_passive = check_multiframe_regen(0x03, 0x01, "passive");
const bool raw_multiframe_inactive = check_multiframe_regen(0x03, 0x00, "inactive");
const bool raw_multiframe_unsupported = check_multiframe_regen(0x01, 0x03, "unavailable");
const bool raw_multiframe_status_unsupported = check_multiframe_regen(0x02, 0x03, "unavailable");
const bool raw_multiframe_all_fields_unsupported = check_multiframe_regen(0x00, 0x03, "unavailable");

// Exercise the fail-safe state mapping without publishing to real sensors:
// only a fresh PID with both support bits and an active type may alarm.
UdsManager regen;
const uint32_t regen_now = esphome::millis();
regen.obd_dpf_aftertreatment_seen_ = true;
regen.obd_dpf_aftertreatment_ms_ = regen_now;
regen.obd_dpf_aftertreatment_supported_known_ = true;
regen.obd_dpf_regen_supported_ = true;
regen.obd_dpf_regen_type_supported_ = true;
regen.obd_dpf_regen_in_progress_ = true;
regen.obd_dpf_regen_type_active_ = true;
const bool active_requires_confirmed_fields =
    regen.regen_operating_state(15000) == "active";
regen.obd_dpf_regen_type_active_ = false;
const bool passive_is_not_active_alarm =
    regen.regen_operating_state(15000) == "passive";
regen.obd_dpf_regen_in_progress_ = false;
const bool fresh_idle_is_inactive =
    regen.regen_operating_state(15000) == "inactive";
regen.obd_dpf_regen_in_progress_ = true;
regen.obd_dpf_regen_type_supported_ = false;
const bool unsupported_type_fails_closed =
    regen.regen_operating_state(15000) == "unavailable";
regen.obd_dpf_regen_type_supported_ = true;
// Restore the active type after the preceding passive/unsupported assertions.
regen.obd_dpf_regen_type_active_ = true;
regen.obd_dpf_aftertreatment_ms_ = regen_now - 9000;
const bool fresh_status_within_10s =
    regen.regen_operating_state(10000) == "active" &&
    regen.has_fresh_regen_data(10000);
regen.obd_dpf_aftertreatment_ms_ = regen_now - 11000;
const bool stale_status_over_10s_fails_closed =
    regen.regen_operating_state(10000) == "unavailable" &&
    !regen.has_fresh_regen_data(10000);
regen.obd_dpf_aftertreatment_ms_ = regen_now - 15000;
const bool stale_status_fails_closed =
    regen.regen_operating_state(15000) == "unavailable" &&
    !regen.has_fresh_regen_data(15000);
ESP_LOGI("obd_parser_test",
         "checks: unrelated=%d/%d/%d single=%d dpf=%d/%d iso_tp=%d padding=%d seq_wrap=%d regen_raw=%d/%d/%d/%d/%d/%d",
         unrelated_first_frame_rejected, unrelated_pid_rejected,
         unrelated_service_rejected, valid_single_frame_accepted,
         dpf_multiframe_reassembled, dpf_multiframe_decoded,
         bad_sequence_discarded, final_padding_trimmed, sequence_rollover_reassembled,
         raw_multiframe_active, raw_multiframe_passive,
         raw_multiframe_inactive, raw_multiframe_unsupported,
         raw_multiframe_status_unsupported, raw_multiframe_all_fields_unsupported);
ESP_LOGI("obd_parser_test",
         "regen safety: active=%d passive=%d inactive=%d unsupported=%d fresh_10s=%d stale_10s=%d stale_15s=%d",
         active_requires_confirmed_fields, passive_is_not_active_alarm,
         fresh_idle_is_inactive, unsupported_type_fails_closed,
         fresh_status_within_10s, stale_status_over_10s_fails_closed,
         stale_status_fails_closed);
return unrelated_first_frame_rejected && unrelated_pid_rejected &&
       unrelated_service_rejected && valid_single_frame_accepted &&
       dpf_multiframe_reassembled && dpf_multiframe_decoded && bad_sequence_discarded &&
       final_padding_trimmed && sequence_rollover_reassembled &&
       raw_multiframe_active && raw_multiframe_passive && raw_multiframe_inactive &&
       raw_multiframe_unsupported && raw_multiframe_status_unsupported &&
       raw_multiframe_all_fields_unsupported &&
       active_requires_confirmed_fields && passive_is_not_active_alarm &&
       fresh_idle_is_inactive && unsupported_type_fails_closed &&
       fresh_status_within_10s && stale_status_over_10s_fails_closed &&
       stale_status_fails_closed;
}



// Called periodically by the YAML interval automation.

void loop() {

if (bus_ == nullptr) return;

if (!configured_) {
loop_obd_probe_();
return;
}



const uint32_t now = esphome::millis();

// The diagnostic comparison build also polls generic OBD-II PIDs so that
// values such as RPM, EGT, DPF inlet temperature and differential pressure
// can be compared on their normal screens. Keep strict request/response
// serialization: finish an active transaction before starting another one.
if (did_diagnostic_probe_) {
  if (obd_waiting_) {
    loop_obd_probe_();  // handles the OBD timeout; RX is routed to this parser
    return;
  }

  const bool obd_retry_ready = obd_tx_retry_after_ms_ == 0 ||
      static_cast<int32_t>(now - obd_tx_retry_after_ms_) >= 0;
  const bool obd_query_due = static_cast<int32_t>(now - obd_next_query_ms_) >= 0;
  if (!waiting_for_response_ && obd_query_due && obd_retry_ready) {
    loop_obd_probe_();
    return;  // reserve this scheduler turn for the OBD attempt
  }
}

// A diagnostic OBD pair can stay silent until a tester requests data. Start
// the DID probe without waiting for unsolicited CAN frames, but strictly
// pace it: one unanswered candidate per 30 seconds. A response confirms a
// peer and the remaining candidates can then be read at a short interval.
if (did_diagnostic_probe_ && !waiting_for_response_ && now < cycle_delay_until_ms_) return;

// Leave a 2-second pause between completed six-query cycles in normal mode.
if (!did_diagnostic_probe_ && query_index_ == 0 && now < cycle_delay_until_ms_) return;



if (waiting_for_response_) {

if (response_ready_) {

decode_response_();

finish_query_();

} else if (static_cast<int32_t>(now - deadline_ms_) >= 0) {

ESP_LOGW(TAG, "Timeout for DID 0x%04X", parameters_[query_index_].did);

finish_query_();

}

return;

}



did_probe_query_received_frame_ = false;
send_current_query_();

waiting_for_response_ = true;

deadline_ms_ = now + RESPONSE_TIMEOUT_MS;

}



private:
RegenStatus obd_regen_state_(uint32_t max_age_ms) const {
const bool fresh = is_obd_pid_fresh(0x8B, max_age_ms);
const bool status_supported = obd_dpf_aftertreatment_supported_known_ && obd_dpf_regen_supported_;
const bool type_supported = obd_dpf_aftertreatment_supported_known_ && obd_dpf_regen_type_supported_;
return classify_regen_status(fresh, status_supported, type_supported,
                             obd_dpf_regen_in_progress_, obd_dpf_regen_type_active_);
}

static constexpr const char *TAG = "uds_audi";

uint32_t tx_id_{0x7E0};

static constexpr uint32_t RESPONSE_TIMEOUT_MS = 500;
static constexpr uint32_t DID_PROBE_NORMAL_CYCLE_DELAY_MS = 2000;
static constexpr uint32_t DID_PROBE_NO_RESPONSE_BACKOFF_MS = 30000;
static constexpr uint32_t DID_PROBE_QUERY_GAP_MS = 250;
static constexpr uint32_t OBD_QUERY_GAP_MS = 250;
// Keep unanswered probes sparse on a disconnected/quiet CAN bus while still
// allowing the tester to wake an ECU that only responds to diagnostic queries.
static constexpr uint32_t OBD_QUIET_BUS_QUERY_GAP_MS = 2500;
static constexpr uint32_t OBD_TX_RETRY_BACKOFF_MS = 2000;
static constexpr uint32_t CAN_ACTIVITY_TIMEOUT_MS = 2000;
static constexpr uint32_t CAN_ACTIVITY_CONFIRM_WINDOW_MS = 1000;
static constexpr uint8_t CAN_ACTIVITY_CONFIRM_FRAMES = 3;
// Recheck ECU name and CALID periodically in case the device is moved to another car.
static constexpr uint32_t OBD_IDENTITY_REFRESH_MS = 60000;

static constexpr size_t MAX_ISOTP_PAYLOAD = 256;
static constexpr std::array<uint8_t, 5> OBD_SUPPORT_PAGES{{0x00, 0x20, 0x40, 0x60, 0x80}};
static constexpr std::array<uint8_t, 8> OBD_DATA_PIDS{{0x0C, 0x05, 0x7A, 0x7C, 0x8B, 0x0B, 0x33, 0x78}};
static constexpr uint8_t OBD_BITMAP_FALLBACK_TIMEOUTS = 2;

uint32_t next_obd_query_gap_ms_() const {
const uint32_t now = esphome::millis();
const bool can_active = can_activity_seen_ &&
    timestamp_is_fresh(now, last_can_activity_ms_, CAN_ACTIVITY_TIMEOUT_MS);
const bool ecu_active = has_obd_link(obd_link_stale_timeout_ms_);
return (can_active || ecu_active) ? OBD_QUERY_GAP_MS : OBD_QUIET_BUS_QUERY_GAP_MS;
}

// Keep polling disabled while parameters_ below contains illustrative values.
// Set true only after the DID, byte layout, scaling and meaning are verified
// for the exact ECU software/profile selected in the device.
static constexpr bool PROFILE_MAPPING_VERIFIED = false;

static int obd_data_pid_index_(uint8_t pid) {
for (size_t i = 0; i < OBD_DATA_PIDS.size(); i++) {
if (OBD_DATA_PIDS[i] == pid) return static_cast<int>(i);
}
return -1;
}

static int obd_support_page_index_(uint8_t pid) {
// A page request uses its exact base PID (00/20/40/60/80), while a data PID
// belongs to the preceding 32-PID range (for example 8B belongs to page 80).
if (pid <= 0x80 && (pid % 0x20U) == 0) return pid / 0x20U;
const uint8_t page = static_cast<uint8_t>(((pid - 1U) / 0x20U) * 0x20U);
const int index = page / 0x20U;
return index >= 0 && index < static_cast<int>(OBD_SUPPORT_PAGES.size()) ? index : -1;
}

bool obd_pid_advertised_(uint8_t pid, bool *known = nullptr) const {
const int page_index = obd_support_page_index_(pid);
if (page_index < 0 || !obd_support_bitmap_known_[page_index]) {
if (known != nullptr) *known = false;
return false;
}
if (known != nullptr) *known = true;
const uint8_t page = OBD_SUPPORT_PAGES[page_index];
const uint8_t offset = static_cast<uint8_t>(pid - page - 1U);
const uint8_t byte_index = offset / 8U;
const uint8_t bit_index = static_cast<uint8_t>(7U - (offset % 8U));
return (obd_support_bitmap_[page_index][byte_index] & (1U << bit_index)) != 0;
}

bool should_skip_obd_pid_(uint8_t pid) const {
const int pid_index = obd_data_pid_index_(pid);
if (pid_index < 0 || obd_pid_response_seen_[pid_index]) return false;
bool bitmap_known = false;
const bool advertised = obd_pid_advertised_(pid, &bitmap_known);
// A single fallback query tolerates vehicles whose support bitmap is incomplete.
// Skip only after two missing responses to a PID the ECU explicitly did not advertise.
return bitmap_known && !advertised &&
       obd_pid_fallback_timeouts_[pid_index] >= OBD_BITMAP_FALLBACK_TIMEOUTS;
}

void record_obd_pid_timeout_(uint8_t pid) {
const int pid_index = obd_data_pid_index_(pid);
if (pid_index < 0 || obd_pid_response_seen_[pid_index]) return;
bool bitmap_known = false;
const bool advertised = obd_pid_advertised_(pid, &bitmap_known);
if (bitmap_known && !advertised &&
    obd_pid_fallback_timeouts_[pid_index] < OBD_BITMAP_FALLBACK_TIMEOUTS) {
obd_pid_fallback_timeouts_[pid_index]++;
}
}

void mark_obd_pid_response_(uint8_t pid) {
const int pid_index = obd_data_pid_index_(pid);
if (pid_index < 0) return;
obd_pid_response_seen_[pid_index] = true;
obd_pid_fallback_timeouts_[pid_index] = 0;
}

void advance_obd_pid_() {
const int current_index = obd_data_pid_index_(current_obd_pid_);
const size_t start = current_index >= 0 ? static_cast<size_t>(current_index) : 0;
for (size_t step = 1; step <= OBD_DATA_PIDS.size(); step++) {
const uint8_t candidate = OBD_DATA_PIDS[(start + step) % OBD_DATA_PIDS.size()];
if (!should_skip_obd_pid_(candidate)) {
current_obd_pid_ = candidate;
return;
}
}
current_obd_pid_ = OBD_DATA_PIDS[0];
}



enum class ValueKind : uint8_t {

REGEN_FLAG,

SOOT_CALCULATED,

SOOT_MEASURED,

DISTANCE_KM,

TEMP_BEFORE_DPF,

TEMP_BEFORE_TURBO,

RAW_PROBE_ONLY,

};

enum class ProbeFormat : uint8_t {
  SOOT_G_0_01_CANDIDATE,
  PRESSURE_HPA_CANDIDATE,
  U32_METERS_CANDIDATE,
  TEMP_K_CANDIDATE
};



struct Parameter {

uint16_t did;

ValueKind kind;

float scale;

float offset;

const char *name;

ProbeFormat probe_format;

const char *probe_label;

};

void loop_obd_probe_() {
const uint32_t now = esphome::millis();
const bool can_activity_fresh = can_activity_seen_ &&
    timestamp_is_fresh(now, last_can_activity_ms_, CAN_ACTIVITY_TIMEOUT_MS);
// Periodic INFO counters also show quiet-bus probe attempts. OBD-II is
// request/response: a diagnostic CAN bus may stay silent until the tester
// sends the first request, so waiting for unsolicited traffic would deadlock.
if ((can_activity_fresh || obd_probe_requests_since_report_ > 0) &&
    (last_obd_probe_report_ms_ == 0 || now - last_obd_probe_report_ms_ >= OBD_PROBE_REPORT_INTERVAL_MS)) {
ESP_LOGI(TAG, "OBD probe: requests=%u/interval, valid ISO-TP frames=%u, timeouts=%u",
         static_cast<unsigned>(obd_probe_requests_since_report_),
         static_cast<unsigned>(obd_probe_reply_frames_since_report_),
         static_cast<unsigned>(obd_probe_timeouts_since_report_));
obd_probe_requests_since_report_ = 0;
obd_probe_reply_frames_since_report_ = 0;
obd_probe_timeouts_since_report_ = 0;
last_obd_probe_report_ms_ = now;
}
// A failed TWAI enqueue usually means no active CAN peer is acknowledging
// traffic. Pause the whole OBD poller so it cannot hammer the UI/logging loop.
if (obd_tx_retry_after_ms_ != 0 && static_cast<int32_t>(now - obd_tx_retry_after_ms_) < 0) return;
const bool link_fresh = has_obd_link(obd_link_stale_timeout_ms_);
if (link_fresh != obd_link_was_fresh_) {
// A monitor may be moved between cars. Clear cached identity when the link
// goes stale; after it returns, query identity values again if still missing.
obd_link_was_fresh_ = link_fresh;
if (!link_fresh) {
// Drop any in-flight query as well. Otherwise its timeout could advance the
// newly reset page-0 discovery state as if it belonged to the old ECU.
obd_waiting_ = false;
obd_identity_active_type_ = 0;
obd_response_.clear();
obd_rx_expected_length_ = 0;
obd_ecu_name_seen_ = false;
obd_calibration_ids_seen_ = false;
// Forget the old ECU guard only when its link expires. A periodic refresh
// must preserve a known mismatch while waiting for the next CALID response.
cjcd_calibration_match_ = CjcdCalibrationMatch::UNKNOWN;
obd_identity_name_attempted_ = false;
obd_identity_calid_attempted_ = false;
obd_identity_refresh_due_ = false;
// A monitor may move to another ECU. Do not carry DPF PID capability bits
// from the previous vehicle across a stale-link boundary.
obd_dpf_pressure_supported_known_ = false;
obd_dpf_pressure_supported_ = false;
obd_dpf_temperature_supported_known_ = false;
obd_dpf_temperature_supported_ = false;
obd_egt_support_known_ = false;
obd_egt_support_mask_ = 0;
obd_dpf_aftertreatment_supported_known_ = false;
obd_dpf_regen_supported_ = false;
obd_dpf_regen_type_supported_ = false;
obd_dpf_regen_in_progress_ = false;
obd_dpf_regen_type_active_ = false;
obd_support_discovery_done_ = false;
obd_support_query_active_ = false;
obd_support_response_valid_ = false;
obd_support_bitmap_known_.fill(false);
for (auto &page : obd_support_bitmap_) page.fill(0);
obd_pid_fallback_timeouts_.fill(0);
obd_pid_response_seen_.fill(false);
current_obd_pid_ = OBD_SUPPORT_PAGES[0];
if (obd_ecu_name_ != nullptr) obd_ecu_name_->publish_state("");
if (obd_calibration_ids_ != nullptr) obd_calibration_ids_->publish_state("");
} else {
if (!obd_ecu_name_seen_) obd_identity_name_attempted_ = false;
if (!obd_calibration_ids_seen_) obd_identity_calid_attempted_ = false;
}
obd_identity_next_probe_ms_ = now + OBD_QUERY_GAP_MS;
}
if (obd_identity_refresh_due_ && static_cast<int32_t>(now - obd_identity_next_probe_ms_) >= 0) {
// Refresh identity periodically so a quick vehicle/ECU swap is eventually
// detected even if the gap between the two OBD connections was shorter than
// the stale timeout.
obd_ecu_name_seen_ = false;
obd_calibration_ids_seen_ = false;
obd_identity_name_attempted_ = false;
obd_identity_calid_attempted_ = false;
obd_identity_refresh_due_ = false;
if (obd_ecu_name_ != nullptr) obd_ecu_name_->publish_state("");
if (obd_calibration_ids_ != nullptr) obd_calibration_ids_->publish_state("");
}
if (obd_waiting_) {
if (static_cast<int32_t>(now - obd_deadline_ms_) >= 0) {
ESP_LOGD(TAG, "Timeout waiting for OBD PID 0x%02X", current_obd_pid_);
obd_probe_timeouts_since_report_++;
if (!obd_support_query_active_ && obd_identity_active_type_ == 0) {
record_obd_pid_timeout_(current_obd_pid_);
}
finish_obd_probe_();
}
return;
}
// Do not gate the first OBD request on received CAN traffic. Many ECUs only
// answer diagnostic requests and do not broadcast on the OBD diagnostic pair.
// Start with one bounded request; use a slow cadence until the bus or a valid
// ECU response confirms that a peer is present.
const bool can_bus_active = can_activity_fresh || link_fresh;
if (static_cast<int32_t>(now - obd_next_query_ms_) < 0) return;

uint8_t identity_type = 0;
if (can_bus_active && current_obd_pid_ == 0x0C && static_cast<int32_t>(now - obd_identity_next_probe_ms_) >= 0 &&
    !(obd_ecu_name_seen_ && obd_calibration_ids_seen_)) {
if (!obd_ecu_name_seen_ && !obd_identity_name_attempted_) identity_type = 0x0A;
else if (!obd_calibration_ids_seen_ && !obd_identity_calid_attempted_) identity_type = 0x04;
}
obd_identity_active_type_ = identity_type;
obd_support_query_active_ = identity_type == 0 && !obd_support_discovery_done_;
obd_support_response_valid_ = false;
const std::vector<uint8_t> request = identity_type != 0
    ? std::vector<uint8_t>{0x02, 0x09, identity_type, 0x00, 0x00, 0x00, 0x00, 0x00}
    : std::vector<uint8_t>{0x02, 0x01, current_obd_pid_, 0x00, 0x00, 0x00, 0x00, 0x00};
const auto result = bus_->send_data(tx_id_, false, false, request);
if (result != esphome::canbus::ERROR_OK) {
obd_tx_retry_after_ms_ = now + OBD_TX_RETRY_BACKOFF_MS;
report_send_failure_(identity_type != 0 ? "OBD identity InfoType" : "OBD PID",
                     identity_type != 0 ? identity_type : current_obd_pid_);
finish_obd_probe_();
return;
}
obd_probe_requests_since_report_++;
obd_waiting_ = true;
obd_deadline_ms_ = now + RESPONSE_TIMEOUT_MS;
}

void handle_obd_probe_response_(const std::vector<uint8_t> &frame,
                                bool send_flow_control = true) {
if (!obd_waiting_ || frame.empty()) return;
const uint8_t frame_type = frame[0] >> 4;
if (frame_type == 0) {
const size_t payload_length = frame[0] & 0x0F;
if (!classic_can_sf_length_valid(payload_length, frame.size())) return;
const uint8_t expected_service = obd_identity_active_type_ != 0 ? 0x49 : 0x41;
const uint8_t expected_identifier = obd_identity_active_type_ != 0
    ? obd_identity_active_type_ : current_obd_pid_;
const std::array<uint8_t, 2> response_prefix{{frame[1],
    static_cast<uint8_t>(payload_length > 1 ? frame[2] : 0U)}};
if (!obd_response_prefix_matches(response_prefix, payload_length,
                                 expected_service, expected_identifier)) {
  ESP_LOGD(TAG, "Ignoring unrelated OBD Single Frame (service=0x%02X, id=0x%02X; expected 0x%02X/0x%02X)",
           response_prefix[0], response_prefix[1], expected_service, expected_identifier);
  return;
}
obd_probe_reply_frames_since_report_++;
obd_response_.assign(frame.begin() + 1, frame.begin() + 1 + payload_length);
decode_obd_response_();
} else if (frame_type == 1) {
if (frame.size() < 2) return;
if (!begin_isotp_reassembly(frame, MAX_ISOTP_PAYLOAD, obd_rx_expected_length_,
                            obd_rx_next_sequence_, obd_response_)) {
  obd_rx_expected_length_ = 0;
  obd_response_.clear();
  return;
}
obd_probe_reply_frames_since_report_++;
// A classic-CAN OBD First Frame carries the service and PID/InfoType in its
// first two payload bytes. Reject stale or unrelated replies before sending
// Flow Control; otherwise an old multi-frame response could consume bus time
// and be assembled while a different request is pending.
const uint8_t expected_service = obd_identity_active_type_ != 0 ? 0x49 : 0x41;
const uint8_t expected_identifier = obd_identity_active_type_ != 0
    ? obd_identity_active_type_ : current_obd_pid_;
const std::array<uint8_t, 2> response_prefix{{
    static_cast<uint8_t>(obd_response_.size() > 0 ? obd_response_[0] : 0),
    static_cast<uint8_t>(obd_response_.size() > 1 ? obd_response_[1] : 0),
}};
if (!obd_response_prefix_matches(response_prefix, obd_response_.size(),
                                 expected_service, expected_identifier)) {
  ESP_LOGD(TAG, "Ignoring unrelated OBD First Frame (service=0x%02X, id=0x%02X; expected 0x%02X/0x%02X)",
           obd_response_.size() > 0 ? obd_response_[0] : 0,
           obd_response_.size() > 1 ? obd_response_[1] : 0,
           expected_service, expected_identifier);
obd_rx_expected_length_ = 0;
obd_response_.clear();
return;
}
if (!send_flow_control) return;
if (bus_ == nullptr) {
  obd_rx_expected_length_ = 0;
  obd_response_.clear();
  return;
}
const std::vector<uint8_t> flow_control{0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
if (bus_->send_data(tx_id_, false, false, flow_control) != esphome::canbus::ERROR_OK) {
report_send_failure_("ISO-TP flow control", 0);
obd_rx_expected_length_ = 0;
}
} else if (frame_type == 2) {
const IsoTpAssemblyResult result = append_isotp_consecutive_frame(
    frame, obd_rx_expected_length_, obd_rx_next_sequence_, obd_response_);
if (result == IsoTpAssemblyResult::INVALID) return;
obd_probe_reply_frames_since_report_++;
if (result == IsoTpAssemblyResult::COMPLETE) {
obd_rx_expected_length_ = 0;
decode_obd_response_();
}
}
}

void decode_obd_response_() {
const uint32_t now = esphome::millis();
if (obd_identity_active_type_ == 0x0A) {
if (obd_response_.size() < 4 || obd_response_[0] != 0x49 || obd_response_[1] != 0x0A || obd_response_[2] == 0) return;
std::string ecu_name;
const size_t end = std::min<size_t>(obd_response_.size(), 23);  // 49 0A, item count, up to 20 ASCII characters.
for (size_t i = 3; i < end; i++) {
const uint8_t ch = obd_response_[i];
if (ch >= 0x20 && ch <= 0x7E) ecu_name.push_back(static_cast<char>(ch));
}
while (!ecu_name.empty() && ecu_name.back() == ' ') ecu_name.pop_back();
if (ecu_name.empty()) return;
obd_ecu_name_seen_ = true;
if (obd_ecu_name_ != nullptr) obd_ecu_name_->publish_state(ecu_name);
ESP_LOGI(TAG, "OBD ECU name: %s", ecu_name.c_str());
obd_seen_ = true;
last_obd_ms_ = now;
obd_rx_expected_length_ = 0;
obd_response_.clear();
finish_obd_probe_();
return;
} else if (obd_identity_active_type_ == 0x04) {
if (obd_response_.size() < 19 || obd_response_[0] != 0x49 || obd_response_[1] != 0x04 || obd_response_[2] == 0) return;
const size_t item_count = obd_response_[2];
if (item_count > (obd_response_.size() - 3) / 16) return;
std::string calibration_ids;
for (size_t item = 0; item < item_count; item++) {
std::string calibration_id;
const size_t start = 3 + item * 16;
for (size_t i = start; i < start + 16; i++) {
const uint8_t ch = obd_response_[i];
if (ch >= 0x20 && ch <= 0x7E) calibration_id.push_back(static_cast<char>(ch));
}
while (!calibration_id.empty() && calibration_id.back() == ' ') calibration_id.pop_back();
if (calibration_id.empty()) continue;
if (!calibration_ids.empty()) calibration_ids += "; ";
calibration_ids += calibration_id;
}
if (calibration_ids.empty()) return;
obd_calibration_ids_seen_ = true;
const auto identity_match = classify_cjcd_calibration(calibration_ids);
if (identity_match != cjcd_calibration_match_) {
  cjcd_calibration_match_ = identity_match;
  if (cjcd_profile_selected_) clear_cjcd_core_readings_();
  if (identity_match == CjcdCalibrationMatch::MISMATCHED && cjcd_profile_selected_)
    ESP_LOGW(TAG, "CJCD profile blocked: ECU CALID differs from 03L906018SJ 9977");
}
if (obd_calibration_ids_ != nullptr) obd_calibration_ids_->publish_state(calibration_ids);
ESP_LOGI(TAG, "OBD calibration ID(s): %s", calibration_ids.c_str());
obd_seen_ = true;
last_obd_ms_ = now;
obd_rx_expected_length_ = 0;
obd_response_.clear();
finish_obd_probe_();
return;
}
if (obd_support_query_active_) {
if (obd_response_.size() < 6 || obd_response_[0] != 0x41 ||
    obd_response_[1] != current_obd_pid_) return;
const int page_index = obd_support_page_index_(current_obd_pid_);
if (page_index < 0) return;
for (size_t i = 0; i < 4; i++) {
obd_support_bitmap_[page_index][i] = obd_response_[i + 2];
}
obd_support_bitmap_known_[page_index] = true;
obd_support_response_valid_ = true;
obd_seen_ = true;
last_obd_ms_ = now;
ESP_LOGI(TAG, "OBD supported-PID page 0x%02X: %02X %02X %02X %02X",
         current_obd_pid_, obd_response_[2], obd_response_[3],
         obd_response_[4], obd_response_[5]);
obd_rx_expected_length_ = 0;
obd_response_.clear();
finish_obd_probe_();
return;
}
if (obd_response_.size() < 3 || obd_response_[0] != 0x41 || obd_response_[1] != current_obd_pid_) return;
const bool complete_data_response =
    (current_obd_pid_ == 0x0C && obd_response_.size() >= 4) ||
    (current_obd_pid_ == 0x05 && obd_response_.size() >= 3) ||
    (current_obd_pid_ == 0x0B && obd_response_.size() >= 3) ||
    (current_obd_pid_ == 0x33 && obd_response_.size() >= 3) ||
    (current_obd_pid_ == 0x7A && obd_response_.size() >= 9) ||
    (current_obd_pid_ == 0x7C && obd_response_.size() >= 11) ||
    (current_obd_pid_ == 0x78 && obd_response_.size() >= 11) ||
    (current_obd_pid_ == 0x8B && obd_response_.size() >= 9);
if (complete_data_response) mark_obd_pid_response_(current_obd_pid_);
if (current_obd_pid_ == 0x0C && obd_response_.size() >= 4 && engine_rpm_ != nullptr) {
const float rpm = ((static_cast<uint16_t>(obd_response_[2]) << 8) | obd_response_[3]) / 4.0f;
obd_rpm_seen_ = true;
obd_rpm_ms_ = now;
engine_rpm_->publish_state(rpm);
ESP_LOGD(TAG, "OBD PID 0x0C RPM=%.0f", rpm);
} else if (current_obd_pid_ == 0x05 && coolant_temperature_ != nullptr) {
obd_coolant_seen_ = true;
obd_coolant_ms_ = now;
const float temperature_c = static_cast<float>(obd_response_[2]) - 40.0f;
coolant_temperature_->publish_state(temperature_c);
ESP_LOGD(TAG, "OBD PID 0x05 coolant=%.0f C", temperature_c);
} else if (current_obd_pid_ == 0x0B && obd_response_.size() >= 3) {
// SAE J1979 Mode 01 PID 0B: one byte, intake manifold absolute pressure in kPa.
obd_intake_pressure_seen_ = true;
obd_intake_pressure_supported_ = true;
obd_intake_pressure_ms_ = now;
obd_intake_pressure_kpa_ = static_cast<float>(obd_response_[2]);
if (obd_intake_manifold_pressure_ != nullptr) {
obd_intake_manifold_pressure_->publish_state(obd_intake_pressure_kpa_);
}
ESP_LOGI(TAG, "OBD PID 0x0B MAP=%.0f kPa abs", obd_intake_pressure_kpa_);
publish_estimated_boost_if_fresh_(now);
} else if (current_obd_pid_ == 0x33 && obd_response_.size() >= 3) {
// SAE J1979 Mode 01 PID 33: one byte, absolute barometric pressure in kPa.
obd_barometric_pressure_seen_ = true;
obd_barometric_pressure_ms_ = now;
if (obd_barometric_pressure_ != nullptr) {
obd_barometric_pressure_kpa_ = static_cast<float>(obd_response_[2]);
obd_barometric_pressure_->publish_state(obd_barometric_pressure_kpa_);
}
ESP_LOGI(TAG, "OBD PID 0x33 BARO=%.0f kPa abs", obd_barometric_pressure_kpa_);
publish_estimated_boost_if_fresh_(now);
} else if (current_obd_pid_ == 0x7A && obd_response_.size() >= 9) {
// PID 7A: Byte A advertises supported pressure fields; delta pressure is bit 0.
obd_dpf_pressure_seen_ = true;
obd_dpf_pressure_ms_ = now;
obd_dpf_pressure_supported_known_ = true;
obd_dpf_pressure_supported_ = (obd_response_[2] & 0x01) != 0;
if (obd_dpf_pressure_supported_) {
const uint16_t raw = (static_cast<uint16_t>(obd_response_[3]) << 8) | obd_response_[4];
const int32_t signed_raw = (raw & 0x8000U) != 0 ? static_cast<int32_t>(raw) - 0x10000 : raw;
obd_dpf_delta_pressure_kpa_ = static_cast<float>(signed_raw) / 100.0f;
if (obd_dpf_delta_pressure_ != nullptr) {
  obd_dpf_delta_pressure_->publish_state(obd_dpf_delta_pressure_kpa_);
}
ESP_LOGI(TAG, "OBD PID 0x7A differential pressure=%.2f kPa", obd_dpf_delta_pressure_kpa_);
} else {
ESP_LOGI(TAG, "OBD PID 0x7A received; differential-pressure field unsupported (A=0x%02X)", obd_response_[2]);
}
} else if (current_obd_pid_ == 0x7C && obd_response_.size() >= 11) {
// PID 7C: Byte A bit 0 advertises DPF Bank 1 inlet temperature.
obd_dpf_temperature_seen_ = true;
obd_dpf_temperature_ms_ = now;
obd_dpf_temperature_supported_known_ = true;
obd_dpf_temperature_supported_ = (obd_response_[2] & 0x01) != 0;
if (obd_dpf_temperature_supported_ && obd_dpf_inlet_temperature_ != nullptr) {
const uint16_t raw = (static_cast<uint16_t>(obd_response_[3]) << 8) | obd_response_[4];
const float temperature_c = static_cast<float>(raw) / 10.0f - 40.0f;
obd_dpf_inlet_temperature_->publish_state(temperature_c);
ESP_LOGI(TAG, "OBD PID 0x7C DPF inlet temperature=%.1f C", temperature_c);
} else {
ESP_LOGI(TAG, "OBD PID 0x7C received; DPF inlet-temperature field unsupported (A=0x%02X)", obd_response_[2]);
}
} else if (current_obd_pid_ == 0x78 && obd_response_.size() >= 11) {
// SAE J1979-DA PID 78: A0-A3 advertise EGT sensors 1-4;
// temperatures use uint16 big-endian, 0.1 C/bit, with a -40 C offset.
obd_egt_seen_ = true;
obd_egt_ms_ = now;
obd_egt_support_known_ = true;
obd_egt_support_mask_ = static_cast<uint8_t>(obd_response_[2] & 0x0FU);
ESP_LOGI(TAG, "OBD PID 0x78 EGT support mask=%02X", obd_egt_support_mask_);
for (size_t i = 0; i < obd_egt_sensors_.size(); ++i) {
const uint8_t bit = static_cast<uint8_t>(1U << i);
if ((obd_egt_support_mask_ & bit) == 0 || obd_egt_sensors_[i] == nullptr) continue;
const size_t data_offset = 3U + i * 2U;
const uint16_t raw = (static_cast<uint16_t>(obd_response_[data_offset]) << 8) |
                     obd_response_[data_offset + 1U];
const float temperature_c = static_cast<float>(raw) / 10.0f - 40.0f;
obd_egt_sensors_[i]->publish_state(temperature_c);
ESP_LOGI(TAG, "OBD PID 0x78 EGT sensor %u=%.1f C", static_cast<unsigned>(i + 1U), temperature_c);
}
} else if (current_obd_pid_ == 0x8B && obd_response_.size() >= 9) {
// Legacy mapping from SAE J1979-DA Oct 2011 PID 8B table B118:
// byte A bits 0/1 advertise regen status/type;
// byte B bit 0 means regeneration in progress; bit 1 means active (1) or passive (0).
// SAE lists a newer 202607 revision; compare its table and validate with the
// target ECU before treating this mapping as confirmed for CJCD.
obd_dpf_aftertreatment_seen_ = true;
obd_dpf_aftertreatment_ms_ = now;
// Keep the complete seven-byte data record in the log, including when the
// ECU does not set the support bit. This makes future profile validation
// possible without changing the display decoder or issuing write requests.
ESP_LOGI(TAG, "OBD PID 0x8B data A-G: %02X %02X %02X %02X %02X %02X %02X",
         obd_response_[2], obd_response_[3], obd_response_[4], obd_response_[5],
         obd_response_[6], obd_response_[7], obd_response_[8]);
obd_dpf_aftertreatment_supported_known_ = true;
obd_dpf_regen_supported_ = (obd_response_[2] & 0x01) != 0;
obd_dpf_regen_type_supported_ = (obd_response_[2] & 0x02) != 0;
obd_dpf_regen_in_progress_ = obd_dpf_regen_supported_ && (obd_response_[3] & 0x01) != 0;
obd_dpf_regen_type_active_ = obd_dpf_regen_type_supported_ && (obd_response_[3] & 0x02) != 0;
const bool active_regen = classify_regen_status(
    true, obd_dpf_regen_supported_, obd_dpf_regen_type_supported_,
    obd_dpf_regen_in_progress_, obd_dpf_regen_type_active_) == RegenStatus::ACTIVE;
if (regen_active_ != nullptr) regen_active_->publish_state(active_regen);
if (!obd_dpf_regen_supported_) {
ESP_LOGI(TAG, "OBD PID 0x8B regen-status field unsupported (A=0x%02X)", obd_response_[2]);
} else if (!obd_dpf_regen_type_supported_) {
ESP_LOGI(TAG, "OBD PID 0x8B reports regen in-progress but not active/passive type (A=0x%02X)", obd_response_[2]);
} else {
ESP_LOGI(TAG, "OBD PID 0x8B regen=%s type=%s active_warning=%s",
         obd_dpf_regen_in_progress_ ? "in-progress" : "inactive",
         obd_dpf_regen_in_progress_ ? (obd_dpf_regen_type_active_ ? "active" : "passive") : "n/a",
         active_regen ? "yes" : "no");
}
} else {
return;
}
obd_seen_ = true;
last_obd_ms_ = now;
obd_rx_expected_length_ = 0;
obd_response_.clear();
finish_obd_probe_();
}

void finish_obd_probe_() {
obd_waiting_ = false;
obd_rx_expected_length_ = 0;
obd_response_.clear();
if (obd_identity_active_type_ != 0) {
if (obd_identity_active_type_ == 0x0A) obd_identity_name_attempted_ = true;
if (obd_identity_active_type_ == 0x04) obd_identity_calid_attempted_ = true;
obd_identity_active_type_ = 0;
const bool another_identity_needed =
    (!obd_ecu_name_seen_ && !obd_identity_name_attempted_) ||
    (!obd_calibration_ids_seen_ && !obd_identity_calid_attempted_);
if (another_identity_needed) {
obd_identity_refresh_due_ = false;
obd_identity_next_probe_ms_ = esphome::millis() + OBD_QUERY_GAP_MS;
} else {
if (!obd_ecu_name_seen_) obd_identity_name_attempted_ = false;
if (!obd_calibration_ids_seen_) obd_identity_calid_attempted_ = false;
obd_identity_next_probe_ms_ = esphome::millis() + OBD_IDENTITY_REFRESH_MS;
obd_identity_refresh_due_ = true;
}

obd_next_query_ms_ = esphome::millis() + next_obd_query_gap_ms_();
return;
}
if (obd_support_query_active_) {
const bool valid_page = obd_support_response_valid_;
const int page_index = obd_support_page_index_(current_obd_pid_);
obd_support_query_active_ = false;
obd_support_response_valid_ = false;
if (valid_page && page_index >= 0) {
const bool has_next_page = (obd_support_bitmap_[page_index][3] & 0x01U) != 0;
if (has_next_page && page_index + 1 < static_cast<int>(OBD_SUPPORT_PAGES.size())) {
current_obd_pid_ = OBD_SUPPORT_PAGES[page_index + 1];
} else {
if (!has_next_page) {
for (size_t i = static_cast<size_t>(page_index + 1); i < obd_support_bitmap_known_.size(); i++) {
obd_support_bitmap_[i].fill(0);
obd_support_bitmap_known_[i] = true;
}
}
obd_support_discovery_done_ = true;
current_obd_pid_ = OBD_DATA_PIDS[0];
}
} else {
// Some ECUs do not implement Mode 01 support pages. Keep the direct-PID
// fallback so a valid individual response can still enable that value.
obd_support_discovery_done_ = true;
current_obd_pid_ = OBD_DATA_PIDS[0];
}
obd_next_query_ms_ = esphome::millis() + next_obd_query_gap_ms_();
return;
}
advance_obd_pid_();
obd_next_query_ms_ = esphome::millis() + next_obd_query_gap_ms_();
}

void publish_estimated_boost_if_fresh_(uint32_t now) {
if (!obd_intake_pressure_seen_ || !obd_barometric_pressure_seen_) return;
if (!timestamp_is_fresh(now, obd_intake_pressure_ms_, obd_link_stale_timeout_ms_) ||
    !timestamp_is_fresh(now, obd_barometric_pressure_ms_, obd_link_stale_timeout_ms_)) return;
if (obd_estimated_boost_pressure_ != nullptr) {
// MAP and BARO are absolute pressures; their difference estimates gauge pressure.
obd_estimated_boost_pressure_->publish_state(obd_intake_pressure_kpa_ - obd_barometric_pressure_kpa_);
}
}


// Read-only candidates sourced from another public VAG DPF project. The IDs
// and candidate conversions are NOT verified for ECU 03L906018SJ/CJCD.
// Diagnostic mode displays candidate values for comparison with OBDeleven.
// The raw bytes remain available in serial logs. Only the three DID mappings
// with a first OBDeleven comparison are copied to normal cards after explicit
// CJCD selection; no candidate is used for the regeneration warning.
static constexpr std::array<Parameter, 6> parameters_{{
// Empirical comparison on 2026-10-11 supports 0.01 g/bit for both soot DIDs.
// Keep them diagnostic-only until repeat captures confirm the mapping.
{0x114F, ValueKind::RAW_PROBE_ONLY, 1.0f, 0.0f, "soot_calc_candidate", ProbeFormat::SOOT_G_0_01_CANDIDATE, "SADZA OBLICZ"},
{0x114E, ValueKind::RAW_PROBE_ONLY, 1.0f, 0.0f, "soot_meas_candidate", ProbeFormat::SOOT_G_0_01_CANDIDATE, "SADZA ZMIERZ"},
// U16 matched the OBDeleven differential pressure number in hPa in one capture.
{0x14F5, ValueKind::RAW_PROBE_ONLY, 1.0f, 0.0f, "dpf_pressure_candidate", ProbeFormat::PRESSURE_HPA_CANDIDATE, "CISN. DPF"},
// On-road response bytes 00 03 A2 E9 equal 238313, matching 238313 m.
// Treat as a strong candidate for U32 big-endian meters; keep diagnostic-only.
{0x1156, ValueKind::RAW_PROBE_ONLY, 1.0f, 0.0f, "regen_distance_candidate", ProbeFormat::U32_METERS_CANDIDATE, "OD REGEN"},
{0x11B2, ValueKind::RAW_PROBE_ONLY, 1.0f, 0.0f, "dpf_inlet_temp_candidate", ProbeFormat::TEMP_K_CANDIDATE, "TEMP WLOT DPF"},
{0x10FB, ValueKind::RAW_PROBE_ONLY, 1.0f, 0.0f, "turbo_temp_candidate", ProbeFormat::TEMP_K_CANDIDATE, "TEMP TURBO"},
}};



void send_current_query_() {

const auto &parameter = parameters_[query_index_];



// ISO-TP Single Frame carrying UDS ReadDataByIdentifier (0x22).

const std::vector<uint8_t> request{

0x03,

0x22,

static_cast<uint8_t>((parameter.did >> 8) & 0xFF),

static_cast<uint8_t>(parameter.did & 0xFF),

0x00, 0x00, 0x00, 0x00,

};



response_.clear();

response_ready_ = false;

rx_expected_length_ = 0;

rx_next_sequence_ = 1;



const auto result = bus_->send_data(tx_id_, false, false, request);

if (result != esphome::canbus::ERROR_OK) {

report_send_failure_("UDS DID", parameter.did);

} else {

if (did_diagnostic_probe_ && !PROFILE_MAPPING_VERIFIED) {
  ESP_LOGI(TAG, "DID probe sent: 0x%04X candidate=%s (read-only)", parameter.did, parameter.name);
} else {
  ESP_LOGD(TAG, "Sent DID 0x%04X (%s)", parameter.did, parameter.name);
}

}

}

void report_send_failure_(const char *request_type, uint16_t identifier) {
const uint32_t now = esphome::millis();
this->tx_send_failures_since_log_++;
if (this->last_tx_send_failure_log_ms_ == 0 || now - this->last_tx_send_failure_log_ms_ >= 10000) {
ESP_LOGW(TAG, "%u diagnostic CAN request(s) failed to send; latest: %s 0x%04X",
         static_cast<unsigned>(this->tx_send_failures_since_log_), request_type,
         static_cast<unsigned>(identifier));
this->last_tx_send_failure_log_ms_ = now;
this->tx_send_failures_since_log_ = 0;
}
}

void handle_single_frame_(const std::vector<uint8_t> &frame) {

if (frame.size() < 2) return;



const size_t payload_length = frame[0] & 0x0F;

if (!classic_can_sf_length_valid(payload_length, frame.size())) return;



response_.assign(frame.begin() + 1, frame.begin() + 1 + payload_length);

response_ready_ = true;

}



void handle_first_frame_(const std::vector<uint8_t> &frame) {

if (frame.size() < 2) return;



rx_expected_length_ =

(static_cast<size_t>(frame[0] & 0x0F) << 8) | frame[1];



if (rx_expected_length_ < 7 || rx_expected_length_ > MAX_ISOTP_PAYLOAD) {

ESP_LOGW(TAG, "Unsupported ISO-TP payload length: %u",

static_cast<unsigned>(rx_expected_length_));

rx_expected_length_ = 0;

response_.clear();

return;

}



response_.clear();



const size_t initial_bytes = std::min<size_t>(frame.size() - 2, 6);

response_.insert(response_.end(), frame.begin() + 2,

frame.begin() + 2 + initial_bytes);



rx_next_sequence_ = 1;



// Flow Control: Continue To Send, block size 0, separation time 0 ms.

const std::vector<uint8_t> flow_control{

0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};



const auto result = bus_->send_data(tx_id_, false, false, flow_control);

if (result != esphome::canbus::ERROR_OK) {

ESP_LOGW(TAG, "Failed to send ISO-TP Flow Control");

}

}



void handle_consecutive_frame_(const std::vector<uint8_t> &frame) {

if (rx_expected_length_ == 0 || frame.size() < 2) return;



const uint8_t sequence = frame[0] & 0x0F;

if (sequence != rx_next_sequence_) {

ESP_LOGW(TAG, "Bad ISO-TP sequence: expected %u, got %u",

rx_next_sequence_, sequence);

response_.clear();

rx_expected_length_ = 0;

return;

}



rx_next_sequence_ = (rx_next_sequence_ + 1) & 0x0F;



const size_t remaining = rx_expected_length_ - response_.size();

const size_t count = std::min<size_t>(frame.size() - 1, remaining);

response_.insert(response_.end(), frame.begin() + 1,

frame.begin() + 1 + count);



if (response_.size() >= rx_expected_length_) {

response_.resize(rx_expected_length_);

rx_expected_length_ = 0;

response_ready_ = true;

}

}



std::string format_probe_candidate_(const Parameter &parameter) const {
  if (response_.size() < 5) {
      return esphome::str_sprintf("%s: --", parameter.probe_label);
  }
  const uint16_t raw16 = (static_cast<uint16_t>(response_[3]) << 8) | response_[4];
  switch (parameter.probe_format) {
    case ProbeFormat::SOOT_G_0_01_CANDIDATE: {
      const int32_t signed_raw = parameter.did == 0x114E && (raw16 & 0x8000U) != 0
          ? static_cast<int32_t>(raw16) - 0x10000
          : static_cast<int32_t>(raw16);
      const float grams = static_cast<float>(signed_raw) / 100.0f;
      return esphome::str_sprintf("%s: %.2f g", parameter.probe_label, grams);
    }
    case ProbeFormat::PRESSURE_HPA_CANDIDATE:
      return esphome::str_sprintf("%s: %u hPa", parameter.probe_label,
                                  static_cast<unsigned>(raw16));
    case ProbeFormat::TEMP_K_CANDIDATE: {
      const float celsius = (static_cast<float>(raw16) - 2731.0f) / 10.0f;
      return esphome::str_sprintf("%s: %.1f C", parameter.probe_label, celsius);
    }
    case ProbeFormat::U32_METERS_CANDIDATE: {
      if (response_.size() < 7) {
        return esphome::str_sprintf("%s: --", parameter.probe_label);
      }
      const uint32_t meters = (static_cast<uint32_t>(response_[3]) << 24) |
                              (static_cast<uint32_t>(response_[4]) << 16) |
                              (static_cast<uint32_t>(response_[5]) << 8) |
                              static_cast<uint32_t>(response_[6]);
      const float km = static_cast<float>(meters) / 1000.0f;
      return esphome::str_sprintf("%s: %.1f km", parameter.probe_label, km);
    }
  }
  return esphome::str_sprintf("%s: --", parameter.probe_label);
}

void decode_response_() {

// Positive ReadDataByIdentifier response: 0x62, DID_H, DID_L, value...

 if (response_.size() >= 3 && response_[0] == 0x7F && response_[1] == 0x22) {
 const uint16_t did = parameters_[query_index_].did;
 const size_t index = query_index_;
 if (did_diagnostic_probe_ && index < did_probe_sensors_.size() && did_probe_sensors_[index] != nullptr) {
   did_probe_sensors_[index]->publish_state(esphome::str_sprintf("NRC %02X", response_[2]));
 }
 ESP_LOGW(TAG, "UDS DID 0x%04X negative response NRC=0x%02X",
          did, response_[2]);
 return;
 }

 if (response_.size() < 4 || response_[0] != 0x62) {

ESP_LOGW(TAG, "Response is not a positive UDS 0x62 response");

return;

}



const uint16_t did =

(static_cast<uint16_t>(response_[1]) << 8) | response_[2];

const auto &parameter = parameters_[query_index_];



if (did != parameter.did) {

ESP_LOGW(TAG, "Got DID 0x%04X, expected 0x%04X", did, parameter.did);

return;

}



 // Diagnostic probing confirms raw responses but never publishes values under
 // unverified soot/temperature/regeneration labels.
 if (!PROFILE_MAPPING_VERIFIED && did_diagnostic_probe_) {
   std::string raw_bytes;
   for (size_t i = 3; i < response_.size(); ++i) {
     if (!raw_bytes.empty()) raw_bytes += ' ';
     raw_bytes += esphome::str_sprintf("%02X", response_[i]);
   }
   const std::string candidate = format_probe_candidate_(parameter);
   if (query_index_ < did_probe_sensors_.size() && did_probe_sensors_[query_index_] != nullptr) {
     did_probe_sensors_[query_index_]->publish_state(candidate);
   }
   // Three CJCD candidates matched the OBDeleven capture from 2026-10-11:
   // soot DIDs 114F/114E (0.01 g/bit) and pressure 14F5 (hPa). Only expose
   // these on the normal cards after the user explicitly selects the CJCD
   // profile. Keep every other candidate diagnostic-only and keep regen
   // alarm logic independent of these values.
   if (cjcd_core_enabled() && is_cjcd_core_did_(did) && response_.size() >= 5) {
     const uint16_t raw16 = (static_cast<uint16_t>(response_[3]) << 8) | response_[4];
     if (did == 0x114F && soot_calculated_ != nullptr) {
       soot_calculated_->publish_state(static_cast<float>(raw16) / 100.0f);
     } else if (did == 0x114E && soot_measured_ != nullptr) {
       const int32_t signed_raw = (raw16 & 0x8000U) != 0
           ? static_cast<int32_t>(raw16) - 0x10000
           : static_cast<int32_t>(raw16);
       soot_measured_->publish_state(static_cast<float>(signed_raw) / 100.0f);
     } else if (did == 0x14F5 && obd_dpf_delta_pressure_ != nullptr) {
       // 1 hPa = 0.1 kPa; the common UI sensor uses kPa.
       obd_dpf_delta_pressure_->publish_state(static_cast<float>(raw16) / 10.0f);
     }
     did_seen_[query_index_] = true;
     last_valid_did_ms_[query_index_] = esphome::millis();
     ESP_LOGI(TAG, "CJCD core DID matched with OBDeleven once: 0x%04X %s; raw=[%s]",
              did, candidate.c_str(), raw_bytes.c_str());
   } else {
     ESP_LOGI(TAG, "DID candidate 0x%04X: %s; raw=[%s] (diagnostic only; mapping not promoted)",
              did, candidate.c_str(), raw_bytes.c_str());
   }
   return;
 }

 // PLACEHOLDER: example unsigned 16-bit big-endian value.

// Adapt byte count, offset, signedness and formula to confirmed ECU/ODX data.

if (response_.size() < 5) {

ESP_LOGW(TAG, "Response value too short for DID 0x%04X", did);

return;

}



const uint16_t raw =

(static_cast<uint16_t>(response_[3]) << 8) | response_[4];

const float value =

static_cast<float>(raw) * parameter.scale + parameter.offset;

// Record freshness only after the response and requested DID passed validation.
did_seen_[query_index_] = true;
last_valid_did_ms_[query_index_] = esphome::millis();



switch (parameter.kind) {

case ValueKind::REGEN_FLAG:

uds_regen_active_ = raw != 0;
if (regen_active_ != nullptr) regen_active_->publish_state(uds_regen_active_);

break;

case ValueKind::SOOT_CALCULATED:

if (soot_calculated_ != nullptr) soot_calculated_->publish_state(value);

break;

case ValueKind::SOOT_MEASURED:

if (soot_measured_ != nullptr) soot_measured_->publish_state(value);

break;

case ValueKind::DISTANCE_KM:

if (distance_since_regen_ != nullptr)

distance_since_regen_->publish_state(value);

break;

case ValueKind::TEMP_BEFORE_DPF:

if (temp_before_dpf_ != nullptr) temp_before_dpf_->publish_state(value);

break;

case ValueKind::TEMP_BEFORE_TURBO:

if (temp_before_turbo_ != nullptr)

temp_before_turbo_->publish_state(value);

break;

case ValueKind::RAW_PROBE_ONLY:
// Never publish a candidate mapping as a confirmed production sensor.
break;

}

ESP_LOGD(TAG, "%s: raw=%u, value=%.2f", parameter.name, raw, value);

}



void finish_query_() {

const bool completed_full_cycle = (query_index_ == parameters_.size() - 1);



query_index_ = (query_index_ + 1) % parameters_.size();

if (did_diagnostic_probe_) {
  const uint32_t now = esphome::millis();
  const uint32_t delay_ms = did_probe_query_received_frame_
      ? DID_PROBE_QUERY_GAP_MS
      : DID_PROBE_NO_RESPONSE_BACKOFF_MS;
  cycle_delay_until_ms_ = now + delay_ms;
  if (!did_probe_query_received_frame_) {
    ESP_LOGW(TAG, "No ECU frame for candidate DID; next probe in %u ms",
             static_cast<unsigned>(delay_ms));
  }
}



if (completed_full_cycle && query_index_ == 0) {
const uint32_t now = esphome::millis();
const bool ecu_replied = did_probe_cycle_received_frame_;
const uint32_t delay_ms = did_diagnostic_probe_ && !ecu_replied
    ? DID_PROBE_NO_RESPONSE_BACKOFF_MS
    : DID_PROBE_NORMAL_CYCLE_DELAY_MS;
cycle_delay_until_ms_ = now + delay_ms;
if (did_diagnostic_probe_ && !ecu_replied) {
  ESP_LOGW(TAG, "No ECU response during DID probe cycle; backing off for %u ms",
           static_cast<unsigned>(delay_ms));
}
did_probe_cycle_received_frame_ = false;
}



waiting_for_response_ = false;

response_ready_ = false;

rx_expected_length_ = 0;

response_.clear();

}



esphome::canbus::Canbus *bus_{nullptr};



esphome::sensor::Sensor *soot_calculated_{nullptr};

esphome::sensor::Sensor *soot_measured_{nullptr};

esphome::sensor::Sensor *distance_since_regen_{nullptr};

esphome::sensor::Sensor *temp_before_turbo_{nullptr};

esphome::sensor::Sensor *temp_before_dpf_{nullptr};

esphome::binary_sensor::BinarySensor *regen_active_{nullptr};
esphome::sensor::Sensor *engine_rpm_{nullptr};
esphome::sensor::Sensor *coolant_temperature_{nullptr};
esphome::sensor::Sensor *obd_dpf_delta_pressure_{nullptr};
esphome::sensor::Sensor *obd_dpf_inlet_temperature_{nullptr};
esphome::sensor::Sensor *obd_intake_manifold_pressure_{nullptr};
esphome::sensor::Sensor *obd_barometric_pressure_{nullptr};
esphome::sensor::Sensor *obd_estimated_boost_pressure_{nullptr};
std::array<esphome::sensor::Sensor *, 4> obd_egt_sensors_{{nullptr, nullptr, nullptr, nullptr}};



bool configured_{false};
bool did_diagnostic_probe_{false};
bool cjcd_profile_selected_{false};
bool did_probe_cycle_received_frame_{false};
bool did_probe_query_received_frame_{false};

bool waiting_for_response_{false};

bool response_ready_{false};



size_t query_index_{0};

size_t rx_expected_length_{0};

uint8_t rx_next_sequence_{1};

uint32_t deadline_ms_{0};

uint32_t cycle_delay_until_ms_{0};

std::vector<uint8_t> response_;

// Per-DID timestamps let the UI clear values after CAN/UDS updates stop.
std::array<uint32_t, 6> last_valid_did_ms_{};
std::array<bool, 6> did_seen_{};
bool obd_waiting_{false};
bool obd_seen_{false};
bool obd_link_was_fresh_{false};
bool obd_rpm_seen_{false};
bool obd_coolant_seen_{false};
uint32_t obd_link_stale_timeout_ms_{15000};
uint32_t obd_rpm_ms_{0};
uint32_t obd_coolant_ms_{0};
uint8_t current_obd_pid_{0x00};
uint32_t obd_deadline_ms_{0};
uint32_t obd_next_query_ms_{0};
uint32_t obd_tx_retry_after_ms_{0};
static constexpr uint32_t OBD_PROBE_REPORT_INTERVAL_MS = 10000;
uint32_t last_obd_probe_report_ms_{0};
uint32_t obd_probe_requests_since_report_{0};
uint32_t obd_probe_reply_frames_since_report_{0};
uint32_t obd_probe_timeouts_since_report_{0};
uint32_t last_can_activity_ms_{0};
uint32_t can_activity_window_start_ms_{0};
uint8_t can_activity_frames_in_window_{0};
bool can_activity_seen_{false};
uint32_t last_obd_ms_{0};
std::vector<uint8_t> obd_response_;
size_t obd_rx_expected_length_{0};
uint8_t obd_rx_next_sequence_{1};
bool obd_dpf_pressure_seen_{false};
bool obd_dpf_temperature_seen_{false};
float obd_dpf_delta_pressure_kpa_{0.0f};
bool obd_dpf_pressure_supported_known_{false};
bool obd_dpf_temperature_supported_known_{false};
bool obd_dpf_pressure_supported_{false};
bool obd_dpf_temperature_supported_{false};
bool obd_egt_seen_{false};
bool obd_egt_support_known_{false};
uint8_t obd_egt_support_mask_{0};
uint32_t obd_egt_ms_{0};
uint32_t obd_dpf_pressure_ms_{0};
uint32_t obd_dpf_temperature_ms_{0};
bool obd_dpf_aftertreatment_seen_{false};
uint32_t obd_dpf_aftertreatment_ms_{0};
bool obd_dpf_aftertreatment_supported_known_{false};
bool obd_dpf_regen_supported_{false};
bool obd_dpf_regen_type_supported_{false};
bool obd_dpf_regen_in_progress_{false};
bool obd_dpf_regen_type_active_{false};
bool uds_regen_active_{false};
bool obd_intake_pressure_seen_{false};
bool obd_intake_pressure_supported_{false};
uint32_t obd_intake_pressure_ms_{0};
float obd_intake_pressure_kpa_{0.0f};
bool obd_barometric_pressure_seen_{false};
uint32_t obd_barometric_pressure_ms_{0};
float obd_barometric_pressure_kpa_{0.0f};
esphome::text_sensor::TextSensor *obd_ecu_name_{nullptr};
esphome::text_sensor::TextSensor *obd_calibration_ids_{nullptr};
std::array<esphome::text_sensor::TextSensor *, 6> did_probe_sensors_{};
bool obd_ecu_name_seen_{false};
bool obd_calibration_ids_seen_{false};
CjcdCalibrationMatch cjcd_calibration_match_{CjcdCalibrationMatch::UNKNOWN};
bool obd_identity_name_attempted_{false};
bool obd_identity_calid_attempted_{false};
bool obd_identity_refresh_due_{false};
uint8_t obd_identity_active_type_{0};
uint32_t obd_identity_next_probe_ms_{5000};
std::array<std::array<uint8_t, 4>, 5> obd_support_bitmap_{};
std::array<bool, 5> obd_support_bitmap_known_{};
std::array<uint8_t, 8> obd_pid_fallback_timeouts_{};
std::array<bool, 8> obd_pid_response_seen_{};
bool obd_support_discovery_done_{false};
bool obd_support_query_active_{false};
bool obd_support_response_valid_{false};

// Keep repeated send failures visible without flooding the ESPHome log.
uint32_t last_tx_send_failure_log_ms_{0};
uint32_t tx_send_failures_since_log_{0};

};



UdsManager uds_manager;
