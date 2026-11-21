#include "event_recorder.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include <esp_heap_caps.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <limits>

namespace esphome::dpf_log_archive {
void EventRecorder::setup() {
  event_=static_cast<Event *>(heap_caps_calloc(1,sizeof(Event),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  evidence_=static_cast<Evidence *>(heap_caps_calloc(1,sizeof(Evidence),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  if (!event_ || !evidence_ || nvs_open("dpf_events",NVS_READWRITE,&nvs_)!=ESP_OK) { mark_failed(); return; }
  nvs_get_u32(nvs_,"boot",&boot_); ++boot_;
  nvs_set_u32(nvs_,"boot",boot_); nvs_commit(nvs_);
  // Recover the newest committed event. NVS provides atomic blob updates.
  for (const char *key : {"event0","event1"}) {
    size_t size=sizeof(Event);
    if (nvs_get_blob(nvs_,key,event_,&size)==ESP_OK && size==sizeof(Event) && event_->magic==MAGIC && event_->count<=LIMIT) {
      if (event_->sequence>sequence_) sequence_=event_->sequence;
      update_saved_summary_(*event_);
    }
  }
  memset(event_,0,sizeof(Event));
  event_recorder=this;
  web_server_base::global_web_server_base->add_handler(this);
  ESP_LOGI("dpf_events","Event recorder ready: 3 min prebuffer, 5 s sample, two persistent events");
}
void EventRecorder::record_did(uint16_t did,const uint8_t *data,size_t length) {
  if (demo_) return;
  if (did==0x1156) {
    distance_seen_=length==4;
    if(distance_seen_) {
      distance_m_=(uint32_t(data[0])<<24)|(uint32_t(data[1])<<16)|(uint32_t(data[2])<<8)|data[3];
      distance_ms_=millis();
    }
    return;
  }
  for (size_t i=0;i<CHANNELS;++i) if (IDS[i]==did) {
    const size_t expected=i==3 || i>=5 ? 1 : (i==4 ? 4 : 2);
    if (length!=expected) { seen_[i]=false; return; }
    uint32_t raw=0; for(size_t j=0;j<length;++j) raw=(raw<<8)|data[j];
    values_[i]=raw; times_[i]=millis(); seen_[i]=true; return;
  }
}
void EventRecorder::save_(uint32_t reason) {
  event_->reason=reason;
  const char *key=(event_->sequence&1) ? "event1" : "event0";
  const esp_err_t error=nvs_set_blob(nvs_,key,event_,sizeof(Event));
  esp_err_t evidence_error=ESP_OK;
  if (error==ESP_OK && evidence_->magic==EVIDENCE_MAGIC) {
    evidence_->count=event_->count;
    evidence_error=nvs_set_blob(nvs_,(event_->sequence&1)?"evidence1":"evidence0",evidence_,sizeof(Evidence));
  }
  if (error!=ESP_OK || evidence_error!=ESP_OK || nvs_commit(nvs_)!=ESP_OK) ESP_LOGE("dpf_events","Event checkpoint failed; recent data remains RAM only");
  else {
    update_saved_summary_(*event_);
    ESP_LOGI("dpf_events","Saved event %lu: %lu samples, reason=%lu",(unsigned long)event_->sequence,(unsigned long)event_->count,(unsigned long)reason);
  }
  checkpoint_=millis();
}
void EventRecorder::update_saved_summary_(const Event &saved) {
  if (saved.magic!=MAGIC || saved.count==0 || saved.count>LIMIT || saved.sequence==0 ||
      saved.reason<1 || saved.reason>4) return;
  const size_t slot=saved.sequence&1U;
  if (!saved_slots_[slot]) { saved_slots_[slot]=true; ++stored_event_count_; }
  if (saved.sequence<summary_sequence_) return;
  summary_sequence_=saved.sequence;
  last_saved_reason_=saved.reason;
  last_saved_boot_=saved.boot;
  last_saved_duration_s_=(saved.rows[saved.count-1].ms-saved.rows[0].ms)/1000;
}
void EventRecorder::start_() {
  memset(event_,0,sizeof(Event)); event_->magic=MAGIC; event_->sequence=++sequence_; event_->boot=boot_;
  memset(evidence_,0,sizeof(Evidence));
  evidence_->magic=EVIDENCE_MAGIC; evidence_->sequence=event_->sequence; evidence_->boot=boot_;
  const size_t oldest=(pre_head_+PRE-pre_count_)%PRE;
  for(size_t i=0;i<pre_count_;++i) {
    const size_t slot=(oldest+i)%PRE;
    evidence_->rows[event_->count]=pre_evidence_[slot];
    event_->rows[event_->count++]=pre_[slot];
  }
  // Include exact 1 s alarm transitions from the prebuffer, plus its baseline state.
  const uint32_t start=event_->count ? event_->rows[0].ms : millis();
  for(size_t i=0;i<alarm_count_;++i) {
    const auto &change=alarm_changes_[(alarm_head_+CHANGES-alarm_count_+i)%CHANGES];
    if(int32_t(change.ms-start)>=0) append_change_(change);
  }
  active_=true; cooling_=false; save_(1);
}
void EventRecorder::loop() {
  if(restart_requested_.exchange(false)) restart_at_=millis()+1000;
  if(restart_at_ && int32_t(millis()-restart_at_)>=0) App.safe_reboot();
  if(test_requested_.exchange(false)) create_test_();
  if(demo_) return;
  const uint32_t now=millis(); if(now-tick_<5000 || !event_) return; tick_=now;
  Row row{}; row.ms=now;
  for(size_t i=0;i<CHANNELS;++i) { row.raw[i]=values_[i]; if(seen_[i] && now-times_[i]<=15000) row.valid|=1U<<i; }
  const EvidenceRow extra=make_evidence_row_(row);
  {
    std::lock_guard<std::mutex> lock(trip_mutex_);
    trip_[trip_head_]={row,ecu_link_,extra}; trip_head_=(trip_head_+1)%trip_.size();
    if(trip_count_<trip_.size()) ++trip_count_;
  }
  pre_evidence_[pre_head_]=extra;
  pre_[pre_head_]=row; pre_head_=(pre_head_+1)%PRE; if(pre_count_<PRE) ++pre_count_;
  const bool fresh=(row.valid&1)!=0;
  const float temperature=(float(values_[0])-2731)/10;
  if(!active_) {
    if(fresh && temperature>=450 && temperature<=1200) {
      if(!hot_) { hot_=true; hot_since_=now; }
      if(now-hot_since_>=15000) { start_(); hot_=false; }
    } else hot_=false;
    return;
  }
  if(event_->count<LIMIT) { evidence_->rows[event_->count]=extra; event_->rows[event_->count++]=row; }
  uint32_t reason=1;
  if(!fresh) reason=3; // Loss of data is not successful completion.
  else if(temperature<450) {
    if(!cooling_) { cooling_=true; cool_since_=now; }
    if(now-cool_since_>=120000) reason=2;
  } else cooling_=false;
  if(event_->count>=LIMIT) reason=4;
  if(reason!=1) { save_(reason); active_=false; hot_=false; }
  else if(now-checkpoint_>=60000) save_(1);
}
bool EventRecorder::canHandle(web_server_idf::AsyncWebServerRequest *request) const {
  char url[web_server_idf::AsyncWebServerRequest::URL_BUF_SIZE]; const auto path=request->url_to(url);
  return (request->method()==HTTP_GET && (path=="/dpf/event.csv" || path=="/dpf/event-status" || path=="/dpf/trip.csv" || path=="/dpf/event-test.csv")) ||
    (request->method()==HTTP_POST && (path=="/dpf/event-test-create" || path=="/dpf/event-test-restart"));
}
void EventRecorder::handleRequest(web_server_idf::AsyncWebServerRequest *request) {
  char url[web_server_idf::AsyncWebServerRequest::URL_BUF_SIZE]; const auto path=request->url_to(url);
  if(path=="/dpf/event-test-restart") {
    size_t size=0;
    if(nvs_get_blob(nvs_,"test_event",nullptr,&size)!=ESP_OK || size!=sizeof(Event)) { request->send(409,"text/plain","Create and download a TEST before restarting"); return; }
    restart_requested_.store(true); request->send(200,"text/plain","Restart queued; TEST remains in NVS, trip RAM buffer will clear"); return;
  }
  if(path=="/dpf/event-test-create") {
    test_requested_.store(true); request->send(200,"text/plain","Test queued. Synthetic data uses a separate storage key and never activates the alarm."); return;
  }
  if(path=="/dpf/trip.csv") { download_trip_(request); return; }
  const bool is_test=path=="/dpf/event-test.csv";
  // Read committed snapshots only: no lock in the CAN/UI path.
  auto *saved=static_cast<Event *>(heap_caps_malloc(sizeof(Event),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  if(!saved) { request->send(503,"text/plain","Insufficient memory"); return; }
  uint32_t best=0;
  if(is_test) {
    size_t size=sizeof(Event);
    if(nvs_get_blob(nvs_,"test_event",saved,&size)!=ESP_OK || size!=sizeof(Event) || saved->magic!=MAGIC || saved->reason!=100 || saved->count>LIMIT) {
      heap_caps_free(saved); request->send(404,"text/plain","No saved TEST event"); return;
    }
    best=1;
  }
  if(!is_test) {
  for(const char *key:{"event0","event1"}) {
    size_t size=sizeof(Event);
    if(nvs_get_blob(nvs_,key,saved,&size)==ESP_OK && size==sizeof(Event) && saved->magic==MAGIC && saved->count<=LIMIT)
      if(saved->sequence>best) best=saved->sequence;
  }
  if(!best) { heap_caps_free(saved); request->send(path=="/dpf/event-status"?200:404,"text/plain","Brak zapisanego zdarzenia. Zapis uruchamia sie automatycznie przy EGT3 >=450 C przez 15 s."); return; }
  size_t size=sizeof(Event); nvs_get_blob(nvs_,(best&1)?"event1":"event0",saved,&size);
  if(saved->magic!=MAGIC || saved->count>LIMIT) { heap_caps_free(saved); request->send(500,"text/plain","Invalid event snapshot"); return; }
  }
  if(path=="/dpf/event-status") {
    char text[360]; snprintf(text,sizeof(text),"Zdarzenie %lu, probki %lu. Stan %lu (1=zapis/checkpoint, 2=schlodzenie, 3=utrata danych, 4=limit). %s W pamieci: %lu/2; czas zapisu: %lu s.",
      (unsigned long)saved->sequence,(unsigned long)saved->count,(unsigned long)saved->reason,
      saved->boot!=boot_ && saved->reason==1 ? "Przerwane restartem/zasilaniem; dostepny ostatni checkpoint." : "",
      (unsigned long)stored_event_count_,(unsigned long)last_saved_duration_s_);
    heap_caps_free(saved); request->send(200,"text/plain; charset=utf-8",text); return;
  }
  auto *details=static_cast<Evidence *>(heap_caps_malloc(sizeof(Evidence),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  if(!details) { heap_caps_free(saved); request->send(503,"text/plain","Insufficient evidence memory"); return; }
  const bool extended=!is_test && load_evidence_(*saved,details);
  const size_t capacity=16384+saved->count*260;
  auto *text=static_cast<char *>(heap_caps_malloc(capacity,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  if(!text) { heap_caps_free(details); heap_caps_free(saved); request->send(503,"text/plain","Insufficient memory"); return; }
  size_t used=snprintf(text,capacity,"# %s; event=%lu; boot=%lu; reason=%lu; sampling=5s; checkpoint=60s\n"
    "# Reason 1=recording checkpoint, 2=temperature below 450C for 120s, 3=data lost, 4=capacity; none proves ECU regeneration\n"
    "# Times are milliseconds since source boot. Empty fields mean stale/unavailable. Raw values are hex bytes, not decoded status bits.\n"
    "%s",
    is_test ? "TEST ONLY - SYNTHETIC DATA - NOT ECU REGENERATION" : "DPF estimated event",
    (unsigned long)saved->sequence,(unsigned long)saved->boot,(unsigned long)saved->reason,
    extended ? "" : "ms,valid_mask,egt3_c,soot_calc_g,soot_meas_g,115B_raw,12E7_raw,134F_raw,11C1_raw\n");
  if(extended) {
    used+=snprintf(text+used,capacity-used,"# evidence_schema=2; alarm_state 0=unavailable 1=inactive 2=active; alarm_reason 0=initial 1=hot_threshold 2=cooling 3=data_lost 4=mode_change; trend=g/min over >=120s up to ~180s, observation only; overflow=%lu\n",(unsigned long)details->changes_overflow);
    for(size_t i=0;i<details->changes_count;++i) {
      const auto &c=details->changes[i];
      used+=snprintf(text+used,capacity-used,"# alarm_transition_ms=%lu; state=%u; reason=%u\n",(unsigned long)c.ms,c.state,c.reason);
    }
    used+=snprintf(text+used,capacity-used,"ms,valid_mask,egt3_c,soot_calc_g,soot_meas_g,115B_raw,12E7_raw,134F_raw,11C1_raw,distance_since_regen_km,soot_calc_trend_g_min,soot_meas_trend_g_min,alarm_state,alarm_change_ms,alarm_reason\n");
  }
  for(size_t i=0;i<saved->count;++i) {
    const auto &r=saved->rows[i]; used+=snprintf(text+used,capacity-used,"%lu,%02X",(unsigned long)r.ms,r.valid);
    for(size_t j=0;j<CHANNELS;++j) {
      if(!(r.valid&(1U<<j))) { text[used++]=','; continue; }
      if(j==0) used+=snprintf(text+used,capacity-used,",%.1f",(float(r.raw[j])-2731)/10);
      else if(j<=2) used+=snprintf(text+used,capacity-used,",%.2f",j==2 ? float(int16_t(r.raw[j]))/100 : float(r.raw[j])/100);
      else used+=snprintf(text+used,capacity-used,j==4 ? ",%08lX" : ",%02lX",(unsigned long)r.raw[j]);
    }
    if(extended) {
      const auto &x=details->rows[i];
      used+=snprintf(text+used,capacity-used,",");
      if(x.valid&1) used+=snprintf(text+used,capacity-used,"%.3f",double(x.distance_m)/1000);
      used+=snprintf(text+used,capacity-used,",");
      if(x.valid&2) used+=snprintf(text+used,capacity-used,"%.4f",x.calc_trend_g_min);
      used+=snprintf(text+used,capacity-used,",");
      if(x.valid&4) used+=snprintf(text+used,capacity-used,"%.4f",x.meas_trend_g_min);
      used+=snprintf(text+used,capacity-used,",%u,%lu,%u",x.alarm_state,(unsigned long)x.alarm_change_ms,x.alarm_reason);
    }
    text[used++]='\n';
  }
  heap_caps_free(details);
  heap_caps_free(saved);
  auto *response=request->beginResponse(200,"text/csv",reinterpret_cast<const uint8_t *>(text),used);
  response->addHeader("Content-Disposition",is_test ? "attachment; filename=monitor-dpf-TEST.csv" : "attachment; filename=monitor-dpf-event.csv"); response->addHeader("Cache-Control","no-store");
  request->send(response); heap_caps_free(text);
}
void EventRecorder::create_test_() {
  auto *test=static_cast<Event *>(heap_caps_calloc(1,sizeof(Event),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  if(!test) return;
  test->magic=MAGIC; test->boot=boot_; test->reason=100; test->count=2;
  for(size_t i=0;i<2;++i) {
    test->rows[i].ms=i*5000; test->rows[i].valid=7;
    test->rows[i].raw[0]=8731; test->rows[i].raw[1]=2000-i*100; test->rows[i].raw[2]=1900-i*100;
  }
  const esp_err_t result=nvs_set_blob(nvs_,"test_event",test,sizeof(Event));
  if(result==ESP_OK && nvs_commit(nvs_)==ESP_OK) ESP_LOGI("dpf_events","Synthetic TEST saved separately; real event and alarm unchanged");
  else ESP_LOGE("dpf_events","Synthetic TEST persistence failed");
  heap_caps_free(test);
}
void EventRecorder::download_trip_(web_server_idf::AsyncWebServerRequest *request) {
  auto *snapshot=static_cast<TripRow *>(heap_caps_malloc(sizeof(TripRow)*120,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  auto *text=static_cast<char *>(heap_caps_malloc(32768,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  if(!snapshot || !text) { heap_caps_free(snapshot); heap_caps_free(text); request->send(503,"text/plain","Insufficient memory"); return; }
  size_t count;
  {
    std::lock_guard<std::mutex> lock(trip_mutex_); count=trip_count_;
    const size_t oldest=(trip_head_+trip_.size()-count)%trip_.size();
    for(size_t i=0;i<count;++i) snapshot[i]=trip_[(oldest+i)%trip_.size()];
  }
  size_t used=snprintf(text,32768,"# Ordinary drive: last 120 samples, nominal 10 minutes; RAM only; reset clears this buffer\n"
    "# ecu_link_fresh reflects recent diagnostic communication; valid_mask reflects freshness of each recorded DID\n"
    "# evidence_schema=2; alarm state/reason codes as in event.csv; trends are observation only, not an ECU flag\n"
    "ms,ecu_link_fresh,valid_mask,egt3_c,soot_calc_g,soot_meas_g,115B_raw,12E7_raw,134F_raw,11C1_raw,distance_since_regen_km,soot_calc_trend_g_min,soot_meas_trend_g_min,alarm_state,alarm_change_ms,alarm_reason\n");
  for(size_t i=0;i<count;++i) {
    const auto &r=snapshot[i].row;
    used+=snprintf(text+used,32768-used,"%lu,%u,%02X",(unsigned long)r.ms,snapshot[i].ecu_link,r.valid);
    for(size_t j=0;j<CHANNELS;++j) {
      if(!(r.valid&(1U<<j))) { text[used++]=','; continue; }
      if(j==0) used+=snprintf(text+used,32768-used,",%.1f",(float(r.raw[j])-2731)/10);
      else if(j<=2) used+=snprintf(text+used,32768-used,",%.2f",j==2 ? float(int16_t(r.raw[j]))/100 : float(r.raw[j])/100);
      else used+=snprintf(text+used,32768-used,j==4 ? ",%08lX" : ",%02lX",(unsigned long)r.raw[j]);
    }
    const auto &x=snapshot[i].evidence;
    used+=snprintf(text+used,32768-used,",");
    if(x.valid&1) used+=snprintf(text+used,32768-used,"%.3f",double(x.distance_m)/1000);
    used+=snprintf(text+used,32768-used,",");
    if(x.valid&2) used+=snprintf(text+used,32768-used,"%.4f",x.calc_trend_g_min);
    used+=snprintf(text+used,32768-used,",");
    if(x.valid&4) used+=snprintf(text+used,32768-used,"%.4f",x.meas_trend_g_min);
    used+=snprintf(text+used,32768-used,",%u,%lu,%u\n",x.alarm_state,(unsigned long)x.alarm_change_ms,x.alarm_reason);
  }
  heap_caps_free(snapshot);
  auto *response=request->beginResponse(200,"text/csv",reinterpret_cast<const uint8_t *>(text),used);
  response->addHeader("Content-Disposition","attachment; filename=monitor-dpf-drive.csv"); response->addHeader("Cache-Control","no-store");
  request->send(response); heap_caps_free(text);
}
bool EventRecorder::load_evidence_(const Event &saved,Evidence *details) {
  size_t size=sizeof(Evidence);
  return nvs_get_blob(nvs_,(saved.sequence&1)?"evidence1":"evidence0",details,&size)==ESP_OK &&
      size==sizeof(Evidence) && details->magic==EVIDENCE_MAGIC && details->sequence==saved.sequence &&
      details->boot==saved.boot && details->count==saved.count && details->changes_count<=CHANGES;
}
void EventRecorder::append_change_(const AlarmChange &change) {
  if(evidence_->changes_count<CHANGES) evidence_->changes[evidence_->changes_count++]=change;
  else evidence_->changes_overflow=1; // Explicitly report truncation; never invent transitions.
}
void EventRecorder::record_alarm(uint32_t now,bool valid,bool active,uint8_t cause) {
  if(demo_ && cause!=4) return;
  const uint8_t state=valid ? (active?2:1) : 0;
  if(alarm_initialized_ && state==alarm_state_) return;
  uint8_t reason=cause;
  if(!reason && alarm_initialized_) reason=!valid?3:(active?1:(alarm_state_==2?2:0));
  alarm_initialized_=true; alarm_state_=state; alarm_reason_=reason; alarm_change_ms_=now;
  const AlarmChange change{now,state,reason,0};
  alarm_changes_[alarm_head_]=change; alarm_head_=(alarm_head_+1)%CHANGES;
  if(alarm_count_<CHANGES) ++alarm_count_;
  ESP_LOGI("dpf_events","Estimated alarm transition: ms=%lu state=%u reason=%u (not ECU completion)",(unsigned long)now,state,reason);
  // Capture the 1 s decision time, including a loss transition just after the last 5 s sample.
  if(evidence_ && evidence_->magic==EVIDENCE_MAGIC && event_->count &&
      (active_ || uint32_t(now-event_->rows[event_->count-1].ms)<=15000)) {
    append_change_(change); save_(active_?1:event_->reason);
  }
}
const char *EventRecorder::alarm_observation() const {
  if(demo_) return "DEMO: brak oceny ECU";
  if(alarm_state_==2) return "ALARM SZACOWANY AKTYWNY";
  if(alarm_state_==0) return "BRAK DANYCH: ocena przerwana";
  if(alarm_reason_==2) return "SCHLODZENIE: alarm wygaszony; koniec cyklu niepotwierdzony";
  return "BRAK ALARMU: koniec cyklu niepotwierdzony";
}
void EventRecorder::set_demo(bool enabled) {
  if(enabled==demo_) return;
  if(enabled) {
    record_alarm(millis(),false,false,4);
    if(active_) { save_(3); active_=false; }
  }
  demo_=enabled;
  pre_count_=pre_head_=0; hot_=cooling_=false;
  distance_seen_=false;
  for(auto &seen:seen_) seen=false;
  if(!enabled) { alarm_initialized_=false; alarm_count_=alarm_head_=0; }
}
EventRecorder::EvidenceRow EventRecorder::make_evidence_row_(const Row &row) {
  EvidenceRow x{};
  x.alarm_state=alarm_state_; x.alarm_reason=alarm_reason_; x.alarm_change_ms=alarm_change_ms_;
  if(distance_seen_ && uint32_t(row.ms-distance_ms_)<=15000) { x.distance_m=distance_m_; x.valid|=1; }
  // Oldest-to-current slope across a continuous 2-3 min window, no extrapolation.
  if(!pre_count_) return x;
  const size_t oldest=(pre_head_+PRE-pre_count_)%PRE;
  const Row &base=pre_[oldest];
  const uint32_t elapsed=row.ms-base.ms;
  if(elapsed<120000 || elapsed>200000) return x;
  for(size_t channel=1;channel<=2;++channel) {
    bool continuous=(row.valid&(1U<<channel))!=0;
    uint32_t previous=base.ms;
    for(size_t i=0;i<pre_count_ && continuous;++i) {
      const Row &prior=pre_[(oldest+i)%PRE];
      continuous=(prior.valid&(1U<<channel)) && uint32_t(prior.ms-previous)<=10000;
      previous=prior.ms;
    }
    if(!continuous || uint32_t(row.ms-previous)>10000) continue;
    const float first=channel==2 ? float(int16_t(base.raw[channel]))/100 : float(base.raw[channel])/100;
    const float last=channel==2 ? float(int16_t(row.raw[channel]))/100 : float(row.raw[channel])/100;
    const float slope=(last-first)*60000.0f/elapsed;
    if(channel==1) { x.calc_trend_g_min=slope; x.valid|=2; }
    else { x.meas_trend_g_min=slope; x.valid|=4; }
  }
  return x;
}

}
