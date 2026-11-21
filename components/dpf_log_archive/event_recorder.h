#pragma once
#include "esphome/core/component.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include <nvs.h>
#include <array>
#include <atomic>
#include <mutex>

namespace esphome::dpf_log_archive {
class EventRecorder : public Component, public web_server_idf::AsyncWebHandler {
 public:
  void setup() override;
  void loop() override;
  void record_did(uint16_t did, const uint8_t *data, size_t length);
  void record_alarm(uint32_t now, bool valid, bool active, uint8_t cause=0);
  void set_demo(bool enabled);
  const char *alarm_observation() const;
  void set_ecu_link(bool fresh) { ecu_link_=fresh; }
  // Main-loop UI reads metadata from committed real events, never TEST data.
  uint32_t stored_event_count() const { return stored_event_count_; }
  uint32_t last_saved_duration_s() const { return last_saved_duration_s_; }
  uint32_t last_saved_reason() const { return last_saved_reason_; }
  bool recording() const { return active_; }
  bool saved_event_interrupted() const { return last_saved_reason_==1 && last_saved_boot_!=boot_; }
  bool canHandle(web_server_idf::AsyncWebServerRequest *request) const override;
  void handleRequest(web_server_idf::AsyncWebServerRequest *request) override;
 private:
  static constexpr size_t CHANNELS=7, PRE=36, LIMIT=900;
  struct Row { uint32_t ms; uint32_t raw[CHANNELS]; uint16_t valid; };
  struct Event { uint32_t magic, sequence, boot, reason, count; Row rows[LIMIT]; };
  static constexpr uint32_t MAGIC=0x44504631;
  static constexpr std::array<uint16_t, CHANNELS> IDS{{0x11B2,0x114F,0x114E,0x115B,0x12E7,0x134F,0x11C1}};
  // Keep legacy Event byte layout intact. Optional evidence is a versioned sidecar.
  struct EvidenceRow {
    uint32_t distance_m, alarm_change_ms;
    float calc_trend_g_min, meas_trend_g_min;
    uint8_t valid, alarm_state, alarm_reason, reserved;
  };
  struct AlarmChange { uint32_t ms; uint8_t state, reason; uint16_t reserved; };
  static constexpr size_t CHANGES=128;
  static constexpr uint32_t EVIDENCE_MAGIC=0x44504632;
  struct Evidence {
    uint32_t magic, sequence, boot, count, changes_count, changes_overflow;
    EvidenceRow rows[LIMIT];
    AlarmChange changes[CHANGES];
  };
  Evidence *evidence_{nullptr};
  std::array<EvidenceRow,PRE> pre_evidence_{};
  std::array<AlarmChange,CHANGES> alarm_changes_{};
  size_t alarm_head_{0}, alarm_count_{0};
  uint32_t distance_m_{0}, distance_ms_{0}, alarm_change_ms_{0};
  bool distance_seen_{false}, demo_{false}, alarm_initialized_{false};
  uint8_t alarm_state_{0}, alarm_reason_{0};
  EvidenceRow make_evidence_row_(const Row &row);
  void append_change_(const AlarmChange &change);
  bool load_evidence_(const Event &saved, Evidence *details);
  Event *event_{nullptr};
  std::array<Row,PRE> pre_{};
  size_t pre_head_{0}, pre_count_{0};
  uint32_t values_[CHANNELS]{}, times_[CHANNELS]{};
  bool seen_[CHANNELS]{}, active_{false}, hot_{false}, cooling_{false};
  uint32_t tick_{0}, checkpoint_{0}, hot_since_{0}, cool_since_{0}, boot_{0}, sequence_{0};
  nvs_handle_t nvs_{0};
  struct TripRow { Row row; bool ecu_link; EvidenceRow evidence; };
  std::array<TripRow,120> trip_{};
  size_t trip_head_{0}, trip_count_{0};
  bool ecu_link_{false};
  std::mutex trip_mutex_;
  std::atomic<bool> test_requested_{false};
  std::atomic<bool> restart_requested_{false};
  uint32_t restart_at_{0};
  std::array<bool,2> saved_slots_{};
  uint32_t stored_event_count_{0}, summary_sequence_{0};
  uint32_t last_saved_duration_s_{0}, last_saved_reason_{0}, last_saved_boot_{0};
  void update_saved_summary_(const Event &saved);
  void create_test_();
  void download_trip_(web_server_idf::AsyncWebServerRequest *request);
  void start_();
  void save_(uint32_t reason);
};
inline EventRecorder *event_recorder=nullptr;
}
