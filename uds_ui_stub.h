#pragma once

// Tymczasowa warstwa danych dla trybu projektowania interfejsu.
// Udostępnia tylko metody używane przez YAML i zawsze zwraca brak danych.
// Nie dołącza komponentu CAN, nie tworzy sterownika magistrali i niczego
// nie wysyła do samochodu. Po zakończeniu prac nad UI można wrócić do
// pełnego menedżera UDS z pliku uds_audi.h.

#include <cstdint>
#include <string>

class UdsManager {
 public:
  template <typename... Args>
  void configure(std::nullptr_t, Args &&...) {}

  bool is_did_fresh(uint16_t, uint32_t) const { return false; }
  bool is_obd_pid_fresh(uint8_t, uint32_t) const { return false; }
  bool is_obd_pid_supported(uint8_t) const { return false; }
  bool is_obd_pid_support_known(uint8_t) const { return false; }
  bool has_fresh_regen_data(uint32_t) const { return false; }
  bool has_obd_link(uint32_t) const { return false; }

  std::string regen_data_source(uint32_t) const { return "unavailable"; }
};

// Obiekt o tej samej nazwie, której używają lambdy w YAML.
inline UdsManager uds_manager;
