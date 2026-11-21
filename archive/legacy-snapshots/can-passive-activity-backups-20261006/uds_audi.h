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

#include <string>

#include <vector>



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
esphome::text_sensor::TextSensor *obd_ecu_name,
esphome::text_sensor::TextSensor *obd_calibration_ids,
uint32_t tx_id = 0x7E0,
uint32_t obd_link_stale_timeout_ms = 15000) {

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
obd_ecu_name_ = obd_ecu_name;
obd_calibration_ids_ = obd_calibration_ids;
tx_id_ = tx_id;
obd_link_stale_timeout_ms_ = obd_link_stale_timeout_ms;

configured_ = (bus_ != nullptr) && PROFILE_MAPPING_VERIFIED;
if (bus_ != nullptr && !PROFILE_MAPPING_VERIFIED) {
ESP_LOGW(TAG, "UDS polling disabled: no verified vehicle profile is configured");
}
}

// Fresh means at least one recognized DID returned a valid response recently.
bool has_fresh_data(uint32_t max_age_ms) const {
const uint32_t now = esphome::millis();
for (size_t i = 0; i < parameters_.size(); i++) {
if (did_seen_[i] && static_cast<uint32_t>(now - last_valid_did_ms_[i]) < max_age_ms) return true;
}
return false;
}

// Track the regeneration flag separately so the UI can mark only that value stale.
bool is_did_fresh(uint16_t did, uint32_t max_age_ms) const {
const uint32_t now = esphome::millis();
for (size_t i = 0; i < parameters_.size(); i++) {
if (parameters_[i].did == did) {
return did_seen_[i] && static_cast<uint32_t>(now - last_valid_did_ms_[i]) < max_age_ms;
}
}
return false;
}

// The standard OBD-II fallback confirms ECU communication without relying on
// vehicle-specific DPF DID. It also probes generic DPF pressure/temperature PIDs.
bool has_obd_link(uint32_t max_age_ms) const {
return obd_seen_ && static_cast<uint32_t>(esphome::millis() - last_obd_ms_) < max_age_ms;
}

bool is_obd_pid_fresh(uint8_t pid, uint32_t max_age_ms) const {
const uint32_t now = esphome::millis();
if (pid == 0x0C) return obd_rpm_seen_ && static_cast<uint32_t>(now - obd_rpm_ms_) < max_age_ms;
if (pid == 0x05) return obd_coolant_seen_ && static_cast<uint32_t>(now - obd_coolant_ms_) < max_age_ms;
if (pid == 0x7A) return obd_dpf_pressure_seen_ && static_cast<uint32_t>(now - obd_dpf_pressure_ms_) < max_age_ms;
if (pid == 0x7C) return obd_dpf_temperature_seen_ && static_cast<uint32_t>(now - obd_dpf_temperature_ms_) < max_age_ms;
if (pid == 0x8B) return obd_dpf_aftertreatment_seen_ && static_cast<uint32_t>(now - obd_dpf_aftertreatment_ms_) < max_age_ms;
if (pid == 0x0B) return obd_intake_pressure_seen_ && static_cast<uint32_t>(now - obd_intake_pressure_ms_) < max_age_ms;
if (pid == 0x33) return obd_barometric_pressure_seen_ && static_cast<uint32_t>(now - obd_barometric_pressure_ms_) < max_age_ms;
return false;
}

bool is_obd_pid_supported(uint8_t pid) const {
if (pid == 0x7A) return obd_dpf_pressure_supported_known_ && obd_dpf_pressure_supported_;
if (pid == 0x7C) return obd_dpf_temperature_supported_known_ && obd_dpf_temperature_supported_;
if (pid == 0x8B) return obd_dpf_aftertreatment_supported_known_ && obd_dpf_regen_supported_;
if (pid == 0x0B) return obd_intake_pressure_supported_;
return false;
}

bool is_obd_pid_support_known(uint8_t pid) const {
if (pid == 0x7A) return obd_dpf_pressure_supported_known_;
if (pid == 0x7C) return obd_dpf_temperature_supported_known_;
if (pid == 0x8B) return obd_dpf_aftertreatment_supported_known_;
return false;
}

// Distinguish an unanswered PID from one absent in a valid support bitmap.
// The bitmap is only reported as "not advertised" after the direct fallback
// has also timed out twice; a real PID response always takes precedence.
std::string obd_pid_support_status(uint8_t pid) const {
if (pid != 0x8B) return "unknown";
if (obd_dpf_aftertreatment_supported_known_) {
return obd_dpf_regen_supported_ ? "supported" : "unsupported";
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
       (is_obd_pid_fresh(0x8B, max_age_ms) && is_obd_pid_supported(0x8B));
}



// Called from canbus.on_frame for RX ID 0x7E8.

void notify_can_activity() {
last_can_activity_ms_ = esphome::millis();
can_activity_seen_ = true;
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

if (!configured_) {
handle_obd_probe_response_(data);
return;
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



// Called periodically by the YAML interval automation.

void loop() {

if (bus_ == nullptr) return;

if (!configured_) {
loop_obd_probe_();
return;
}



const uint32_t now = esphome::millis();



// Leave a 2-second pause between completed six-query cycles.

if (query_index_ == 0 && now < cycle_delay_until_ms_) return;



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



send_current_query_();

waiting_for_response_ = true;

deadline_ms_ = now + RESPONSE_TIMEOUT_MS;

}



private:

static constexpr const char *TAG = "uds_audi";

uint32_t tx_id_{0x7E0};

static constexpr uint32_t RESPONSE_TIMEOUT_MS = 500;
static constexpr uint32_t OBD_QUERY_GAP_MS = 250;
static constexpr uint32_t OBD_TX_RETRY_BACKOFF_MS = 2000;
static constexpr uint32_t CAN_ACTIVITY_TIMEOUT_MS = 2000;
static constexpr uint32_t CAN_IDLE_PROBE_INTERVAL_MS = 5000;
// Recheck ECU name and CALID periodically in case the device is moved to another car.
static constexpr uint32_t OBD_IDENTITY_REFRESH_MS = 60000;

static constexpr size_t MAX_ISOTP_PAYLOAD = 256;
static constexpr std::array<uint8_t, 5> OBD_SUPPORT_PAGES{{0x00, 0x20, 0x40, 0x60, 0x80}};
static constexpr std::array<uint8_t, 7> OBD_DATA_PIDS{{0x0C, 0x05, 0x7A, 0x7C, 0x8B, 0x0B, 0x33}};
static constexpr uint8_t OBD_BITMAP_FALLBACK_TIMEOUTS = 2;

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

};



struct Parameter {

uint16_t did;

ValueKind kind;

float scale;

float offset;

const char *name;

};

void loop_obd_probe_() {
const uint32_t now = esphome::millis();
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
obd_identity_name_attempted_ = false;
obd_identity_calid_attempted_ = false;
obd_identity_refresh_due_ = false;
// A monitor may move to another ECU. Do not carry DPF PID capability bits
// from the previous vehicle across a stale-link boundary.
obd_dpf_pressure_supported_known_ = false;
obd_dpf_pressure_supported_ = false;
obd_dpf_temperature_supported_known_ = false;
obd_dpf_temperature_supported_ = false;
obd_dpf_aftertreatment_supported_known_ = false;
obd_dpf_regen_supported_ = false;
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
if (!obd_support_query_active_ && obd_identity_active_type_ == 0) {
record_obd_pid_timeout_(current_obd_pid_);
}
finish_obd_probe_();
}
return;
}
const bool can_bus_active = can_activity_seen_ &&
    static_cast<uint32_t>(now - last_can_activity_ms_) < CAN_ACTIVITY_TIMEOUT_MS;
// The hardware filter admits only ECU replies on 0x7E8. No frame can wake the
// poller before its first request, so bootstrap with one bounded OBD probe
// every 5 seconds. After a reply, use the normal query cadence.
if (!can_bus_active) {
  if (static_cast<int32_t>(now - obd_idle_probe_after_ms_) < 0) return;
} else if (static_cast<int32_t>(now - obd_next_query_ms_) < 0) {
  return;
}

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
if (!can_bus_active) obd_idle_probe_after_ms_ = now + CAN_IDLE_PROBE_INTERVAL_MS;
const auto result = bus_->send_data(tx_id_, false, false, request);
if (result != esphome::canbus::ERROR_OK) {
obd_tx_retry_after_ms_ = now + OBD_TX_RETRY_BACKOFF_MS;
report_send_failure_(identity_type != 0 ? "OBD identity InfoType" : "OBD PID",
                     identity_type != 0 ? identity_type : current_obd_pid_);
finish_obd_probe_();
return;
}
obd_waiting_ = true;
obd_deadline_ms_ = now + RESPONSE_TIMEOUT_MS;
}

void handle_obd_probe_response_(const std::vector<uint8_t> &frame) {
if (!obd_waiting_ || frame.empty()) return;
const uint8_t frame_type = frame[0] >> 4;
if (frame_type == 0) {
const size_t payload_length = frame[0] & 0x0F;
if (payload_length == 0 || payload_length > frame.size() - 1) return;
obd_response_.assign(frame.begin() + 1, frame.begin() + 1 + payload_length);
decode_obd_response_();
} else if (frame_type == 1) {
if (frame.size() < 2) return;
obd_rx_expected_length_ = (static_cast<size_t>(frame[0] & 0x0F) << 8) | frame[1];
if (obd_rx_expected_length_ < 8 || obd_rx_expected_length_ > MAX_ISOTP_PAYLOAD) {
obd_rx_expected_length_ = 0;
return;
}
obd_response_.assign(frame.begin() + 2, frame.begin() + 2 + std::min<size_t>(frame.size() - 2, 6));
obd_rx_next_sequence_ = 1;
const std::vector<uint8_t> flow_control{0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
if (bus_->send_data(tx_id_, false, false, flow_control) != esphome::canbus::ERROR_OK) {
report_send_failure_("ISO-TP flow control", 0);
obd_rx_expected_length_ = 0;
}
} else if (frame_type == 2) {
if (obd_rx_expected_length_ == 0 || frame.size() < 2) return;
const uint8_t sequence = frame[0] & 0x0F;
if (sequence != obd_rx_next_sequence_) {
obd_rx_expected_length_ = 0;
obd_response_.clear();
return;
}
obd_rx_next_sequence_ = (obd_rx_next_sequence_ + 1) & 0x0F;
const size_t remaining = obd_rx_expected_length_ - obd_response_.size();
const size_t count = std::min<size_t>(frame.size() - 1, remaining);
obd_response_.insert(obd_response_.end(), frame.begin() + 1, frame.begin() + 1 + count);
if (obd_response_.size() >= obd_rx_expected_length_) {
obd_response_.resize(obd_rx_expected_length_);
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
if (obd_intake_manifold_pressure_ != nullptr) {
obd_intake_pressure_kpa_ = static_cast<float>(obd_response_[2]);
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
if (obd_dpf_pressure_supported_ && obd_dpf_delta_pressure_ != nullptr) {
const uint16_t raw = (static_cast<uint16_t>(obd_response_[3]) << 8) | obd_response_[4];
const int32_t signed_raw = (raw & 0x8000U) != 0 ? static_cast<int32_t>(raw) - 0x10000 : raw;
obd_dpf_delta_pressure_->publish_state(static_cast<float>(signed_raw) / 100.0f);
ESP_LOGI(TAG, "OBD PID 0x7A differential pressure=%.2f kPa", static_cast<float>(signed_raw) / 100.0f);
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
} else if (current_obd_pid_ == 0x8B && obd_response_.size() >= 9) {
// SAE J1979-DA PID 8B: byte A bit 0 advertises DPF regen status;
// byte B bit 0 is 1 while DPF regeneration is in progress.
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
if (obd_dpf_regen_supported_ && regen_active_ != nullptr) {
const bool regen_active = (obd_response_[3] & 0x01) != 0;
regen_active_->publish_state(regen_active);
ESP_LOGI(TAG, "OBD PID 0x8B regen-status supported; byte A=0x%02X byte B=0x%02X active=%s",
         obd_response_[2], obd_response_[3], regen_active ? "yes" : "no");
} else {
ESP_LOGI(TAG, "OBD PID 0x8B received; regen-status field unsupported (A=0x%02X)", obd_response_[2]);
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

obd_next_query_ms_ = esphome::millis() + OBD_QUERY_GAP_MS;
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
obd_next_query_ms_ = esphome::millis() + OBD_QUERY_GAP_MS;
return;
}
advance_obd_pid_();
obd_next_query_ms_ = esphome::millis() + OBD_QUERY_GAP_MS;
}

void publish_estimated_boost_if_fresh_(uint32_t now) {
if (!obd_intake_pressure_seen_ || !obd_barometric_pressure_seen_) return;
if (static_cast<uint32_t>(now - obd_intake_pressure_ms_) >= obd_link_stale_timeout_ms_ ||
    static_cast<uint32_t>(now - obd_barometric_pressure_ms_) >= obd_link_stale_timeout_ms_) return;
if (obd_estimated_boost_pressure_ != nullptr) {
// MAP and BARO are absolute pressures; their difference estimates gauge pressure.
obd_estimated_boost_pressure_->publish_state(obd_intake_pressure_kpa_ - obd_barometric_pressure_kpa_);
}
}


// PLACEHOLDERS: DID codes and scale/offset are not verified for the CJCD ECU.

// OBDeleven can confirm named values; raw DID mapping/format still needs
// confirmation from ECU/ODX data or a raw diagnostic trace before replacement.

static constexpr std::array<Parameter, 6> parameters_{{

{0xF401, ValueKind::REGEN_FLAG, 1.0f, 0.0f, "regen"},

{0xF402, ValueKind::SOOT_CALCULATED, 0.1f, 0.0f, "soot_calculated"},

{0xF403, ValueKind::SOOT_MEASURED, 0.1f, 0.0f, "soot_measured"},

{0xF404, ValueKind::DISTANCE_KM, 1.0f, 0.0f, "distance_since_regen"},

{0xF405, ValueKind::TEMP_BEFORE_DPF, 0.1f, -40.0f, "temp_before_dpf"},

{0xF406, ValueKind::TEMP_BEFORE_TURBO, 0.1f, -40.0f, "temp_before_turbo"},

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

ESP_LOGD(TAG, "Sent DID 0x%04X (%s)", parameter.did, parameter.name);

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

if (payload_length == 0 || payload_length > frame.size() - 1) return;



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



void decode_response_() {

// Positive ReadDataByIdentifier response: 0x62, DID_H, DID_L, value...

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

if (regen_active_ != nullptr) regen_active_->publish_state(raw != 0);

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

}



ESP_LOGD(TAG, "%s: raw=%u, value=%.2f", parameter.name, raw, value);

}



void finish_query_() {

const bool completed_full_cycle = (query_index_ == parameters_.size() - 1);



query_index_ = (query_index_ + 1) % parameters_.size();



if (completed_full_cycle && query_index_ == 0) {

cycle_delay_until_ms_ = esphome::millis() + 2000;

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



bool configured_{false};

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
uint32_t last_can_activity_ms_{0};
uint32_t obd_idle_probe_after_ms_{0};
bool can_activity_seen_{false};
uint32_t last_obd_ms_{0};
std::vector<uint8_t> obd_response_;
size_t obd_rx_expected_length_{0};
uint8_t obd_rx_next_sequence_{1};
bool obd_dpf_pressure_seen_{false};
bool obd_dpf_temperature_seen_{false};
bool obd_dpf_pressure_supported_known_{false};
bool obd_dpf_temperature_supported_known_{false};
bool obd_dpf_pressure_supported_{false};
bool obd_dpf_temperature_supported_{false};
uint32_t obd_dpf_pressure_ms_{0};
uint32_t obd_dpf_temperature_ms_{0};
bool obd_dpf_aftertreatment_seen_{false};
uint32_t obd_dpf_aftertreatment_ms_{0};
bool obd_dpf_aftertreatment_supported_known_{false};
bool obd_dpf_regen_supported_{false};
bool obd_intake_pressure_seen_{false};
bool obd_intake_pressure_supported_{false};
uint32_t obd_intake_pressure_ms_{0};
float obd_intake_pressure_kpa_{0.0f};
bool obd_barometric_pressure_seen_{false};
uint32_t obd_barometric_pressure_ms_{0};
float obd_barometric_pressure_kpa_{0.0f};
esphome::text_sensor::TextSensor *obd_ecu_name_{nullptr};
esphome::text_sensor::TextSensor *obd_calibration_ids_{nullptr};
bool obd_ecu_name_seen_{false};
bool obd_calibration_ids_seen_{false};
bool obd_identity_name_attempted_{false};
bool obd_identity_calid_attempted_{false};
bool obd_identity_refresh_due_{false};
uint8_t obd_identity_active_type_{0};
uint32_t obd_identity_next_probe_ms_{5000};
std::array<std::array<uint8_t, 4>, 5> obd_support_bitmap_{};
std::array<bool, 5> obd_support_bitmap_known_{};
std::array<uint8_t, 7> obd_pid_fallback_timeouts_{};
std::array<bool, 7> obd_pid_response_seen_{};
bool obd_support_discovery_done_{false};
bool obd_support_query_active_{false};
bool obd_support_response_valid_{false};

// Keep repeated send failures visible without flooding the ESPHome log.
uint32_t last_tx_send_failure_log_ms_{0};
uint32_t tx_send_failures_since_log_{0};

};



UdsManager uds_manager;


