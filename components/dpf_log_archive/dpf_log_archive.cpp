#include "dpf_log_archive.h"
#include "esphome/components/logger/logger.h"
#include "esphome/core/hal.h"
#include <esp_heap_caps.h>
#include <cstdio>
#include <cstring>

namespace esphome::dpf_log_archive {
void LogArchive::setup() {
  entries_ = static_cast<Entry *>(heap_caps_calloc(CAPACITY, sizeof(Entry), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (!entries_) { this->mark_failed(); return; }
  web_server_base::global_web_server_base->add_handler(this); // Same Digest auth in LAN and AP.
  logger::global_logger->add_log_callback(this,
      [](void *self, uint8_t, const char *, const char *message, size_t length) {
        static_cast<LogArchive *>(self)->capture_(message, length);
      });
  ESP_LOGI("dpf_log_archive", "Log history ready: 10 minutes / 1024 entries in PSRAM");
}

void LogArchive::capture_(const char *message, size_t length) {
  if (!entries_ || !message) return;
  // Never wait on a download in the logger/rendering path.
  std::unique_lock<std::mutex> lock(mutex_, std::try_to_lock);
  if (!lock.owns_lock()) { dropped_.fetch_add(1, std::memory_order_relaxed); return; }
  Entry &entry = entries_[head_];
  entry.uptime = millis();
  size_t output = 0;
  bool escape = false;
  for (size_t i = 0; i < length && message[i]; ++i) {
    const unsigned char ch = message[i];
    if (ch == 0x1b) { escape = true; continue; }
    if (escape) { if (ch >= '@' && ch <= '~' && ch != '[') escape = false; continue; }
    if (output == LINE_SIZE - 1) { ++truncated_; break; }
    entry.text[output++] = (ch == '\r' || ch == '\n') ? ' ' : static_cast<char>(ch);
  }
  // Do not end a truncated UTF-8 line in the middle of a multi-byte character.
  if (output == LINE_SIZE - 1) {
    size_t start = output;
    while (start && (static_cast<unsigned char>(entry.text[start - 1]) & 0xc0) == 0x80) --start;
    if (start && static_cast<unsigned char>(entry.text[start - 1]) >= 0xc0) output = start - 1;
  }
  entry.text[output] = 0;
  entry.length = output;
  head_ = (head_ + 1) % CAPACITY;
  if (count_ < CAPACITY) ++count_; else ++overwritten_;
}

bool LogArchive::canHandle(web_server_idf::AsyncWebServerRequest *request) const {
  char url[web_server_idf::AsyncWebServerRequest::URL_BUF_SIZE];
  return request->method() == HTTP_GET && request->url_to(url) == "/dpf/logs.txt";
}

void LogArchive::handleRequest(web_server_idf::AsyncWebServerRequest *request) {
  // IDF sends this response synchronously; the PSRAM snapshot stays alive until send returns.
  constexpr size_t SNAPSHOT_SIZE = CAPACITY * (LINE_SIZE + 32) + 1024;
  auto *snapshot = static_cast<char *>(heap_caps_malloc(SNAPSHOT_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (!snapshot) { request->send(503, "text/plain", "Not enough PSRAM for log download"); return; }
  size_t used = 0;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    const uint32_t now = millis();
    used = snprintf(snapshot, 1024,
        "VAG DPF Monitor log history\nUptime: %lu ms\nRetention: last 600 seconds, maximum 1024 entries\n"
        "Times below: milliseconds since boot, not wall clock\nRAM only: reset clears history\n"
        "Overwritten since boot: %lu; truncated: %lu; skipped while busy: %lu\n\n",
        static_cast<unsigned long>(now), static_cast<unsigned long>(overwritten_),
        static_cast<unsigned long>(truncated_), static_cast<unsigned long>(dropped_.load()));
    const size_t oldest = (head_ + CAPACITY - count_) % CAPACITY;
    for (size_t i = 0; i < count_; ++i) {
      const Entry &entry = entries_[(oldest + i) % CAPACITY];
      if (static_cast<uint32_t>(now - entry.uptime) >= RETENTION_MS) continue;
      used += snprintf(snapshot + used, SNAPSHOT_SIZE - used, "[%lu ms] %.*s\n",
          static_cast<unsigned long>(entry.uptime), entry.length, entry.text);
    }
  }
  auto *response = request->beginResponse(200, "text/plain; charset=utf-8",
      reinterpret_cast<const uint8_t *>(snapshot), used);
  response->addHeader("Content-Disposition", "attachment; filename=monitor-dpf-logs.txt");
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
  heap_caps_free(snapshot);
}
}
