#pragma once

// Trzy jawne tryby radia: połączenie z routerem, sam punkt AP albo całkowite
// wyłączenie Wi-Fi. Brak fallback AP zapobiega uruchamianiu go samoczynnie.
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/core/log.h"
#include "esphome/core/preferences.h"

namespace dpf_wifi_control {

enum class NetworkMode : int {
  ROUTER = 0,
  SETUP_AP = 1,
  OFF = 2,
};

// Osobny, stały klucz NVS. Zapis tylko po zmianie dokonanej przez użytkownika,
// natychmiastowy commit: RESET nie czeka na cykliczny zapis globals.
inline esphome::ESPPreferenceObject &mode_preference() {
  static auto preference = esphome::global_preferences->make_preference<int>(0x4450464EU, true);
  return preference;
}

inline int load_network_mode(int fallback) {
  int stored = fallback;
  const bool loaded = mode_preference().load(&stored);
  if (stored < 0 || stored > 2) stored = 0;
  ESP_LOGI("dpf_wifi", "Network preference: loaded=%s mode=%d", loaded ? "yes" : "no", stored);
  return stored;
}

inline bool save_network_mode(NetworkMode mode) {
  const int stored = static_cast<int>(mode);
  const bool queued = mode_preference().save(&stored);
  const bool committed = queued && esphome::global_preferences->sync();
  ESP_LOGI("dpf_wifi", "Network preference: mode=%d save=%s", stored, committed ? "OK" : "FAILED");
  return committed;
}

inline esphome::wifi::WiFiAP &saved_station_config() {
  static esphome::wifi::WiFiAP config;
  return config;
}

inline NetworkMode &current_mode() {
  static NetworkMode mode = NetworkMode::OFF;
  return mode;
}

inline bool &station_config_saved() {
  static bool saved = false;
  return saved;
}

// Codegen przekazuje konfigurację z secrets przed uruchomieniem komponentów.
// Nie zależymy od get_sta(), które podczas skanowania może zwrócić pusty profil.
inline void initialize_station_config(const std::string &ssid, const std::string &password) {
  saved_station_config().set_ssid(ssid);
  saved_station_config().set_password(password);
  station_config_saved() = !ssid.empty();
}

inline const char *mode_name(NetworkMode mode) {
  switch (mode) {
    case NetworkMode::ROUTER: return "Wi-Fi: router";
    case NetworkMode::SETUP_AP: return "AP: VAG-DPF-Setup";
    case NetworkMode::OFF: return "Sieć wyłączona";
  }
  return "Sieć wyłączona";
}

inline void remember_station_config(esphome::wifi::WiFiComponent *wifi) {
  if (wifi != nullptr && wifi->has_sta()) {
    const auto selected = wifi->get_sta();
    if (!selected.get_ssid().empty()) {
      saved_station_config() = selected;
      station_config_saved() = true;
    }
  }
}

inline bool set_network_mode(NetworkMode requested) {
  auto *wifi = esphome::wifi::global_wifi_component;
  if (wifi == nullptr) {
    ESP_LOGW("dpf_wifi", "Wi-Fi component is not available");
    return false;
  }
  if (requested != NetworkMode::ROUTER && requested != NetworkMode::SETUP_AP &&
      requested != NetworkMode::OFF) return false;

  // Zapamiętaj konfigurację STA zanim wyczyścimy ją na potrzeby trybu AP/OFF.
  if (current_mode() == NetworkMode::ROUTER) remember_station_config(wifi);
  // Sprawdź możliwość przełączenia przed rozłączeniem aktualnego trybu.
  if (requested == NetworkMode::ROUTER && !station_config_saved()) {
    ESP_LOGW("dpf_wifi", "No saved router credentials; Wi-Fi router mode unavailable");
    return false;
  }
  if (requested == NetworkMode::SETUP_AP && !wifi->has_ap()) {
    ESP_LOGW("dpf_wifi", "Setup AP is not configured");
    return false;
  }
  if (requested == current_mode() &&
      ((requested == NetworkMode::OFF && wifi->is_disabled()) ||
       (requested == NetworkMode::SETUP_AP && wifi->is_ap_active()) ||
       (requested == NetworkMode::ROUTER && !wifi->is_disabled() && wifi->has_sta()))) {
    return true;
  }

  wifi->disable();
  wifi->clear_sta();
  // 0 wyłącza automatyczny fallback AP w ESPHome. AP uruchamia się wyłącznie,
  // gdy użytkownik wybierze jego tryb.
  wifi->set_ap_timeout(0);

  if (requested == NetworkMode::ROUTER) {
    wifi->set_sta(saved_station_config());
    wifi->enable();
    ESP_LOGI("dpf_wifi", "Network mode: router Wi-Fi; automatic AP fallback disabled");
  } else if (requested == NetworkMode::SETUP_AP) {
    wifi->enable();  // With no STA config ESPHome starts AP-only mode.
    ESP_LOGI("dpf_wifi", "Network mode: setup AP only at 192.168.4.1");
  } else {
    ESP_LOGI("dpf_wifi", "Network mode: Wi-Fi radio completely disabled");
  }

  current_mode() = requested;
  return true;
}

inline NetworkMode next_network_mode() {
  switch (current_mode()) {
    case NetworkMode::ROUTER: return NetworkMode::SETUP_AP;
    case NetworkMode::SETUP_AP: return NetworkMode::OFF;
    case NetworkMode::OFF: return NetworkMode::ROUTER;
  }
  return NetworkMode::ROUTER;
}

inline NetworkMode network_mode() { return current_mode(); }

}  // namespace dpf_wifi_control
