# Audyt `uds_audi.h` (komponent UDS/CAN – Samochodowy Monitor DPF)

Plik: `C:\Users\Administrator\Documents\ChatGPT\Samochodowy Monitor DPF\uds_audi.h` (2175 linii)
Kontekst: ESPHome (ESP-IDF v5.5.5, ESP32), komponent używany z `audi-dpf-monitor.yaml`.
Zakres: przeczytany cały plik + powiązane pliki (`regen_estimator.h`, `components/dpf_log_archive/event_recorder.*`,
`components/esp32_can_recover/*`, fragmenty `audi-dpf-monitor.yaml`).

> Uwaga: plik jest **nagłówkiem** i definiuje obiekt globalny `UdsManager uds_manager;` (linia 2175),
> więc wszystkie poniższe uwagi dotyczą też ODR (multiple definition) – patrz punkt 1.4.

---

## 1. Błędy C++ / kompilowalność / jakość kodu

### 1.1 [KRYTYCZNY] Brak jakiejkolwiek synchronizacji stanu współdzielonego (reentrancy)
- **Lokalizacje:**
  - `handle_rx()` – linia 551 (wołane z pętli głównej, `audi-dpf-monitor.yaml:221`)
  - `notify_can_activity()` – linia 506 (wołane z `activity_callback_manager_.call()`, `esp32_can_recover.cpp:205`)
  - `loop()` – linia 796 (interval 50 ms, `audi-dpf-monitor.yaml:386`)
  - `update_estimated_regen()/regen_operating_state()` – linie 478, 496 (interval 1 s, `audi-dpf-monitor.yaml:539–553`)
  - `set_cjcd_profile_selected()` – linia 310 (akcja użytkownika / web API)
- **Opis:** cały stan (`parameters_`, `query_index_`, `response_`, `obd_response_`, `did_seen_`,
  `obd_*_seen_`, liczniki, bufory ISO-TP, `estimated_regen_enabled_`) jest modyfikowany bez żadnego
  mutexa/krytycznej sekcji. `can.trigger()` wykonuje callbacki **bezpośrednio w kontekście pętli głównej**
  (nie przez defer), a serwer WWW v3 wykonuje akcje przez `this->defer(...)` → dopiero w `App.loop()`.
  To znaczy, że `uds_manager.loop()` (interval) i `handle_rx()` (callback CAN) działają w tym samym
  wątku i są bezpieczne, ale **każde przyszłe przeniesienie odbioru CAN do własnego zadania FreeRTOS
  albo wywołanie API z zadania WWW natychmiast da wyścig** na `response_`, `obd_response_`,
  `query_index_`, `waiting_for_response_`.
- **Skutek potencjalny:** uszkodzenie wektora w trakcie odczytu (odczyt poza buforem), błędne przypisanie
  odpowiedzi do złego DID-a, zawieszenie w `std::min(expected_length - payload.size(), ...)`.
- **Propozycja:** dodać `std::mutex state_mutex_` i chronić wszystkie wejścia publiczne
  (`loop`, `handle_rx`, `notify_can_activity`, `update_estimated_regen`, `set_cjcd_profile_selected`,
  `cancel_pending_obd_request`, `clear_cjcd_core_readings_`), albo jawnie udokumentować kontrakt
  „tylko pętla główna” komentarzem + `assert_loop()` w wersji debug. Rozważyć `std::recursive_mutex`
  (metody wołają się wzajemnie).

### 1.2 [ISTOTNY] Definicje funkcji w nagłówku bez `inline` – ryzyko „multiple definition”
- **Lokalizacje:** `begin_isotp_reassembly()` – linia 137, `append_isotp_consecutive_frame()` – linia 154,
  `UdsManager uds_manager;` – linia 2175.
- **Opis:** funkcje nie-szablonowe zdefiniowane w nagłówku mają linkage zewnętrzne. Plik jest obecnie
  dołączany raz, więc link się udaje, ale dołączenie `uds_audi.h` w drugiej jednostce tłumaczeniowej
  (np. kolejny custom component albo test) da błąd linkera.
- **Propozycja:** oznaczyć obie funkcje `inline` (albo `static`), a obiekt globalny zadeklarować
  `extern` w nagłówku i zdefiniować w jednym `.cpp` (albo pozostawić jako `static`, jeśli ma być
  prywatny dla TU).

### 1.3 [ISTOTNY] Prawie cała klasa `UdsManager` jest publiczna
- **Lokalizacja:** linia 230 (`public:`) i brak `protected:`; wszystko poniżej `private:` (linia 872)
  ok, ale API jest bardzo szerokie.
- **Opis:** na zewnątrz wystawione są wszystkie parsery (`handle_obd_probe_response_`, `decode_response_`,
  `finish_query_`, `send_current_query_`, `obd_pid_advertised_`, …). Utrzymanie niezmienników
  (np. „`query_index_` zawsze < `parameters_.size()`”) jest wtedy niemożliwe.
- **Propozycja:** przenieść do `private:` wszystko poza `configure()`, `loop()`, `handle_rx()`,
  `notify_can_activity()`, getterami stanu i `run_obd_parser_self_test()`. Testy przenieść do
  `friend`/osobnej klasy testowej albo do `#ifdef`.

### 1.4 [DROBNY] Nieużywane pola (martwy stan)
- **Lokalizacje:** `egt3_did_seen_`/`egt3_did_ms_` (875–876, używane tylko w 396–397 i 1854–1857 –
  faktycznie używane), `PROFILE_MAPPING_VERIFIED` (920) – **zawsze `false`**, więc:
  gałęzie `if (PROFILE_MAPPING_VERIFIED)` w 299/302 są nieosiągalne, a cała „produkcyjna” ścieżka
  (bez `did_diagnostic_probe_`) nigdy nie publikuje danych DPF.
  `distance_since_regen_`, `temp_before_turbo_`, `temp_before_dpf_` (2048–2052) są ustawiane,
  ale **żaden `Parameter` nie ma odpowiedniego `ValueKind`** (wszystkie wpisy to `RAW_PROBE_ONLY`),
  więc te pola są w praktyce martwe.
- **Propozycja:** usunąć martwe gałęzie albo dodać `#if`/komentarz wyjaśniający, że ścieżka jest
  celowo wyłączona do czasu walidacji; w przeciwnym razie ESLint/`-Wunused` i recenzenci będą
  zgłaszać fałszywe „działanie” tej funkcji.

### 1.5 [DROBNY] Zbędne `#include` i brakujące include’y
- **Lokalizacje:** linia 5 (`driver/gpio.h` – GPIO używane jest w YAML, nie w tym pliku), linia 32–33
  (`<string>`, `<string_view>` – używane), brak `<cstddef>` dla `size_t` (działa przez tranzytywne
  include’y – kruche), brak `<cmath>`/`<math.h>` mimo użycia `NAN` (linia 481 – działa tranzytywnie).
- **Propozycja:** usunąć `driver/gpio.h` z tego pliku, dodać `<cstddef>` i `<cmath>` explicite.

### 1.6 [DROBNY] `std::array<uint32_t, 512> obd_timeout_log_ms_` – marnotrawna tablica indeksowana PID-em
- **Lokalizacja:** linia 2110, użycie 1128–1131.
- **Opis:** 512 × 4 B = 2 KB na tablicę, w której realnie używanych jest 8–9 komórek; dodatkowo
  tymczasowe `uint32_t` jest tworzone dla każdego PID (alokacja stosu/`size_t` template).
- **Propozycja:** `std::array<uint32_t, 10>` indeksowane `obd_data_pid_index_()`, plus osobne pole
  dla identyfikacji Mode 09 (albo 2 komórki: 0x0A/0x04).

### 1.7 [DROBNY] Niekonsekwentne formatowanie i „puste” linie
- **Lokalizacje:** 232–297 (jedna zmienna na akapit), 577–603, 996–1040, 1555–1605, 2173–2175.
- **Opis:** całe bloki są rozbite pustymi liniami i mają niespójne wcięcia (1–2 spacje), co utrudnia
  czytanie i code review, a także generuje szum w `git diff`.
- **Propozycja:** uruchomić `clang-format` z konfiguracją ESPHome/Google (2 spacje, 120 kolumn).

### 1.8 [DROBNY] Stałe i logika zduplikowane
- **Lokalizacje:** `classic_can_sf_length_valid()` (122) vs. `begin_isotp_reassembly()`/`handle_first_frame_`
  (137–152, 1639–1701); `handle_consecutive_frame_()` (1705) duplikuje `append_isotp_consecutive_frame()`
  (154) z dodatkowymi logami.
- **Opis:** dwie równoległe implementacje ISO-TP → łatwo o rozbieżność (np. `handle_first_frame_` wymaga
  `rx_expected_length_ >= 7`, helper wymaga `>= 8`).
- **Propozycja:** zostawić jedną implementację (helper) i wywoływać ją z `handle_*_frame_`.

---

## 2. ISO-TP / CAN – walidacja ramek i granice buforów

### 2.1 [ISTOTNY] Niespójne minimum długości ramki First Frame
- **Lokalizacje:** `handle_first_frame_()` – linia 1651 (`< 7` odrzucane),
  `begin_isotp_reassembly()` – linia 145 (`< 8` odrzucane).
- **Opis:** ISO-TP FF z długością 7 (np. 7-bajtowa odpowiedź) jest akceptowana przez ścieżkę UDS
  (`handle_first_frame_`), ale odrzucana przez ścieżkę OBD. Ponadto przy `length == 7` FF przenosi
  `min(frame.size()-2, 6) = 6` bajtów, więc 7. bajt przyjdzie w CF – logika jest poprawna, ale
  zachowanie obu parserów jest różne dla tego samego wejścia.
- **Propozycja:** ujednolicić warunek (`length >= 7`) w helperze i usunąć ręczną implementację.

### 2.2 [ISTOTNY] Odrzucanie całej ramki FF przy złym prefiksie, ale pozostawienie stanu `obd_waiting_`
- **Lokalizacja:** linie 1217–1226.
- **Opis:** gdy FF należy do innego PID-u/serwisu, parser zeruje `obd_rx_expected_length_` i `obd_response_`,
  co jest poprawne, ale **nie przedłuża deadline’u** – wygląda to dobrze; natomiast w linii 1237
  (`flow_control` nie wysłane) też zeruje długość, ale nie czyści `obd_response_` (niespójne
  z pozostałymi ścieżkami – patrz 1201–1203 vs 1236).
- **Propozycja:** wywołać wspólny helper `reset_obd_rx_()` czyszczący wszystkie trzy pola.

### 2.3 [ISTOTNY] Brak walidacji „czy to aby na pewno ISO-TP z naszego ECU” w `handle_rx`
- **Lokalizacja:** linie 577–603.
- **Opis:** przełącznik po typie ramki nie weryfikuje adresu źródłowego – filtrowanie po `0x7E8` jest
  w YAML (linia 221), ale klasa sama nie ma zabezpieczenia. Jeśli ktoś doda drugi filtr / test,
  `handle_rx` przyjmie dowolną ramkę z dowolnego ID jako odpowiedź na własne zapytanie.
- **Propozycja:** przekazywać `can_id` do `handle_rx(uint32_t can_id, const std::vector<uint8_t>&)`
  i walidować `can_id == tx_id_ + 8`.

### 2.4 [DROBNY] `handle_single_frame_()` nie sprawdza `response_ready_`/`waiting_for_response_`
- **Lokalizacje:** 1619–1635.
- **Opis:** ramka SF odebrana po terminie (ale zanim `loop()` zdążył wywołać `finish_query_()`) ustawi
  `response_ready_`, a `decode_response_()` użyje `parameters_[query_index_]` – czyli DID-a, który
  dopiero co został zaadresowany. Przy błędzie kolejności może to przypisać odpowiedź do złego DID-a.
- **Propozycja:** odrzucać SF gdy `!waiting_for_response_`, tak jak robi to ścieżka OBD
  (`handle_obd_probe_response_` sprawdza `obd_waiting_` w linii 1178).

### 2.5 [DROBNY] `append_isotp_consecutive_frame` – brak ochrony przed niedomkniętym `expected_length`
- **Lokalizacja:** linia 169 (`expected_length - payload.size()`).
- **Opis:** jeśli stan został zerwany (np. `payload.size() > expected_length` po zmianie konfiguracji),
  odejmowanie `size_t` da ogromną wartość, a `std::min` ograniczy ją do `frame.size()-1` – nie ma
  przepełnienia, ale wektor urośnie bardziej niż oczekiwano. Nie jest to obecnie osiągalne, ale to
  krucha niezmienniczość bez `assert`/komentarza.
- **Propozycja:** dodać `if (payload.size() > expected_length) { reset; return INVALID; }`.

### 2.6 [DROBNY] Flow Control wysyłany do `tx_id_`, także w trybie LISTEN_ONLY SELFTEST
- **Lokalizacje:** 1233–1237, 1685–1699.
- **Opis:** `bus_->send_data` zwróci `ERROR_FAIL` w trybie LISTEN_ONLY (`esp32_can_recover.cpp:313`),
  więc funkcjonalnie ok, ale w logu pojawi się ostrzeżenie o nieudanym FC przy testach pasywnych.
- **Propozycja:** sprawdzać tryb przed wysłaniem FC albo logować na poziomie DEBUG.

---

## 3. Stabilność / timeouty / watchdog

### 3.1 [ISTOTNY] Log po timeoucie OBD czyta `current_obd_pid_` spoza zakresu tablicy bitmap
- **Lokalizacja:** linie 1128–1134 (`obd_support_query_active_ ? ... : record_obd_pid_timeout_(current_obd_pid_)`
  oraz log `PID=0x%02X`).
- **Opis:** w trybie `obd_support_query_active_` `current_obd_pid_` przyjmuje wartości **stron** (`0x00,0x20,…`),
  a nie PID-ów danych. `record_obd_pid_timeout_` na szczęście zwraca `-1` z `obd_data_pid_index_()` i nic
  nie robi, ale jest to przypadkowe zabezpieczenie; jeśli ktoś zmieni `OBD_DATA_PIDS` i doda 0x00,
  indeksy się pomieszają.
- **Propozycja:** jawnie rozdzielić „stronę wsparcia” od „PID danych” (osobne pole) i nie mieszać obu w jednym
  `current_obd_pid_`.

### 3.2 [ISTOTNY] Brak Watchdog handlingu, mimo że `loop()` może blokować na czas wysyłki CAN
- **Lokalizacja:** `loop()` – 796; `send_current_query_()`/`loop_obd_probe_()` – 1589, 1163.
- **Opis:** `tx_enqueue_timeout_ms` z YAML może być > 0 (domyślnie `twai_transmit_v2` czeka na wolne
  miejsce w kolejce TX). Callback `interval: 50 ms` plus kolejka TWAI = ryzyko przekroczenia
  `CONFIG_ESP_TASK_WDT_TIMEOUT_S` przy zapchanym TX i braku ACK. Klasa nie robi `esp_task_wdt_reset()`
  ani nie ogranicza liczby bajtów wysyłanych w jednym wywołaniu `loop()`. `ESP32CanRecover` ma własny
  budżet RX, ale nie ma budżetu TX.
- **Propozycja:** ograniczyć `tx_enqueue_timeout_ms` (0–10 ms) i/lub dodać `esp_task_wdt_reset()` po
  wysyłce diagnostycznej; zweryfikować, czy `CONFIG_ESP_TASK_WDT_PANIC` jest włączone.

### 3.3 [ISTOTNY] Pętla `loop()` bez górnego limitu czasu – brak stanu „postęp”
- **Lokalizacja:** 796–868.
- **Opis:** pojedyncze wywołanie `loop()` wysyła co najwyżej jedno zapytanie i wykonuje co najwyżej jedną
  dekodację – to dobrze. Jednak `decode_response_()` w ścieżce diagnostycznej buduje stringi
  (`format_probe_candidate_`, pętla `raw_bytes`) i loguje `ESP_LOGI` na **każdy** DID (30 s throttle jest
  tylko dla części przypadków, patrz 3.4). Przy 10 DID-ach to 10 logów INFO w szybkim cyklu.
- **Propozycja:** ograniczyć logi do `ESP_LOGD` lub podnieść throttle do wspólnego licznika.

### 3.4 [DROBNY] Asymetryczny throttle logów DID
- **Lokalizacje:** 1892–1902 vs 1894–1896.
- **Opis:** dla CJCD-core log jest raz/30 s (`did_value_log_ms_`), dla pozostałych kandydatów też raz/30 s –
  ale w gałęzi `PROFILE_MAPPING_VERIFIED == false` gałąź „core” może zostać pominięta, gdy
  `cjcd_core_enabled()` jest false, więc obie ścieżki piszą do **tego samego** `did_value_log_ms_[index]`.
  Log „(diagnostic only; mapping not promoted)” nadpisuje znacznik, przez co log core nie pojawi się przez
  30 s po zmianie profilu.
- **Propozycja:** użyć dwóch tablic znaczników albo wspólnego licznika „raz na DID”.

### 3.5 [DROBNY] Brak wykrywania „utknięcia” parowania ISO-TP
- **Lokalizacja:** `loop()` – 839–857.
- **Opis:** `rx_expected_length_ != 0` po timeoucie jest czyszczone tylko w `finish_query_()`; jeśli CF-y
  przestaną przychodzić, kolejny `send_current_query_()` i tak nadpisze bufor – bezpieczne. Brakuje
  jednak licznika/logu „porzucone niedokończone odpowiedzi”, co utrudnia diagnozę.
- **Propozycja:** licznik + `ESP_LOGW` przy starcie nowego zapytania z niezerowym `rx_expected_length_`.

---

## 4. Logika UDS/OBD – DID-y, formaty, skale

### 4.1 [ISTOTNY] Rozgałęzienie `else if` w `decode_obd_response_()` gubi walidację długości
- **Lokalizacje:** 1337–1454.
- **Opis:** pierwszy warunek `if (current_obd_pid_ == 0x0C && obd_response_.size() >= 4 && engine_rpm_ != nullptr)`
  wymaga też **niepustego wskaźnika sensora**. Jeśli `engine_rpm_ == nullptr` (np. sensor usunięty z YAML),
  sterowanie wpada w kolejne `else if`, żadne nie pasuje i wykonuje się `else { return; }` – czyli
  **`finish_obd_probe_()` nie jest wołane i transakcja zostaje w `obd_waiting_`** do timeoutu.
  To samo dla PID 0x05 (`coolant_temperature_`), 0x7C (`obd_dpf_inlet_temperature_`). Sensor
  0x0B/0x33/0x7A/0x78/0x8B ma osobne sprawdzenie `!= nullptr` w środku – tam jest ok.
- **Skutek:** po usunięciu/przemianowaniu encji w YAML polling zaczyna „wisieć” po 500 ms na każdą
  odpowiedź, a log zapełnia się timeoutami.
- **Propozycja:** rozdzielić walidację od publikacji:
  ```cpp
  if (current_obd_pid_ == 0x0C && obd_response_.size() >= 4) {
     ... ; if (engine_rpm_ != nullptr) engine_rpm_->publish_state(rpm);
  }
  ```

### 4.2 [ISTOTNY] `did_seen_` ustawiane tylko dla 3 DID-ów CJCD
- **Lokalizacje:** 1890–1891 vs. 1930–1931.
- **Opis:** gdy `PROFILE_MAPPING_VERIFIED == false` (zawsze) i `did_diagnostic_probe_ == true`,
  funkcja kończy się `return;` **przed** wspólnym blokiem ustawiającym `did_seen_[query_index_]`.
  Freshness jest więc zapisywana tylko dla 0x114F/0x114E/0x14F5 **i tylko wtedy, gdy `cjcd_core_enabled()`**.
  Skutek: `is_did_fresh(0x11B2/0x10FB/0x1156)` używane w YAML (5835, 1701 itd.) zawsze zwraca `false`,
  więc UI pokazuje te wartości jako nieaktualne, mimo że przychodzą.
- **Propozycja:** przenieść aktualizację `did_seen_/last_valid_did_ms_` przed gałąź diagnostyczną tak,
  aby każda poprawnie zweryfikowana odpowiedź (0x62 + zgodny DID) świeciła freshness – publikacja
  wartości może nadal być ograniczona do profilu CJCD.

### 4.3 [ISTOTNY] `regen_data_source()` i `regen_operating_state()` mogą wskazać „active” bez wsparcia PID
- **Lokalizacje:** 470–476, 496–503.
- **Opis:** `regen_data_source()` zwraca `"OBD-II PID 0x8B"` gdy `is_obd_pid_fresh(0x8B) && is_obd_pid_supported(0x8B)`.
  `is_obd_pid_supported()` wymaga maski wsparcia, więc jest ok. Natomiast `regen_operating_state()`
  przy `!estimated_regen_enabled_` sprawdza `is_did_fresh(0xF401)` – **DID 0xF401 nie występuje w
  `parameters_`** (tablica ma 0x114F,0x114E,0x14F5,0x1156,0x11B2,0x10FB,0x115B,0x12E7,0x134F,0x11C1),
  więc `is_did_fresh(0xF401, …)` **zawsze zwraca `false`**. Gałąź `return uds_regen_active_ ? ...` jest
  martwa, a jednocześnie `has_fresh_regen_data()` i `regen_data_source()` odwołują się do nieistniejącego
  DID-a – mylące i podatne na błąd przy dodaniu 0xF401.
- **Propozycja:** usunąć martwe odwołania do 0xF401 lub dodać ten DID do tablicy parametrów.

### 4.4 [ISTOTNY] `obd_pid_support_status()`/`obd_pid_data_status()` mogą nigdy nie pokazać „unsupported”
- **Lokalizacje:** 424–441, 454–468.
- **Opis:** `obd_pid_data_status()` dla nieobsługiwanego PID-u zwraca `"OCZEK."` dopóki
  `obd_pid_fallback_timeouts_[pid_index] < 2`. `record_obd_pid_timeout_()` **inkrementuje tylko gdy
  `bitmap_known && !advertised`** (linie 968–971), a bitmapa staje się znana dopiero po udanej
  odpowiedzi na stronę wsparcia. Jeśli ECU nie odpowiada na strony (typowe dla wielu VAG), licznik
  nigdy nie rośnie → UI wiecznie pokazuje „OCZEK.” zamiast „BRAK”. To łagodna, ale myląca wada.
- **Propozycja:** zliczać timeouty również, gdy bitmapa nie jest znana (osobny licznik), i po
  N próbach pokazywać „BRAK ODP.”.

### 4.5 [ISTOTNY] Potencjalnie błędna skala/offset EGT3 (DID 0x11B2)
- **Lokalizacje:** 1846–1858 (`(raw - 2731.0f)/10.0f`) oraz `ProbeFormat::TEMP_K_CANDIDATE` – 1780–1783.
- **Opis:** dla `raw = 0x0E6A = 3690` wychodzi `(3690-2731)/10 = 95.9 °C` (zgodnie z komentarzem),
  ale `2731` nie jest ani offsetem `273.1`, ani `2731.5`. Wzór jest „empiryczny” i wygląda jak
  `(raw - 2731)/10`, gdzie `2731` to 0.1 K·2731? To nie odpowiada żadnej standardowej konwersji
  (Kelvin×10 minus offset). Przy `TEMP_K_CANDIDATE` ten sam wzór stosowany jest do 0x10FB i 0x11B2.
  Ryzyko: błąd ~0.05 °C jest nieistotny, ale **błędna interpretacja fizyczna** – jeśli ECU podaje
  Kelvin×10, poprawny wzór to `raw/10 - 273.15`.
- **Propozycja:** potwierdzić wzór kolejnym capture i zapisać go nazwaną stałą
  (`constexpr float CJCD_TEMP_OFFSET = 273.1f;`) z komentarzem źródłowym, zamiast „magicznych” 2731.0f
  (liczba bez kropki dziesiętnej w innym miejscu niż reszta).

### 4.6 [ISTOTNY] `0x114F` publikowane bez uwzględnienia znaku, `0x114E` ze znakiem – niespójność
- **Lokalizacje:** 1879–1885.
- **Opis:** `0x114F` (sadza obliczona) jest zawsze traktowane jako `uint16` (`raw16/100`), a `0x114E`
  (zmierzona) jako `int16`. Jeśli oba DID-y mają ten sam format (co sugeruje wspólny
  `ProbeFormat::SOOT_G_0_01_CANDIDATE`), jedna z konwersji jest błędna. W `format_probe_candidate_`
  (1771) znak uwzględniany jest **tylko** dla `0x114E` – czyli niespójność jest celowa, ale
  nieudokumentowana poza kodem.
- **Propozycja:** dodać do `Parameter` pole `bool is_signed` i sterować konwersją z tabeli, z komentarzem
  potwierdzającym empirycznie.

### 4.7 [DROBNY] `format_probe_candidate_` – `response_[3..4]` czytane po sprawdzeniu tylko `size() < 5`
- **Lokalizacje:** 1764–1767.
- **Opis:** granica jest poprawna (`>= 5` ⇒ indeksy 3 i 4 istnieją), ale funkcja zakłada stały layout
  `62 DID_H DID_L data...`; dla odpowiedzi bez danych (tylko `62 DID_H DID_L`) zwraca `"--"` – ok.
  Uwaga na `RAW_BYTES`: pętla od `i = 3` czyta całe `response_` bez limitu – poprawnie, ale bez
  limitu rozmiaru stringa wynikowego (patrz 4.9).

### 4.8 [DROBNY] Mylące etykiety „RAW” w `parameters_`
- **Lokalizacje:** 1547–1550 (`"POLECENIE RAW"`, `"CZAS RAW"`, `"KONIEC RAW"`, `"BLOKADA RAW"`).
- **Opis:** te DID-y mają `ProbeFormat::RAW_BYTES` i są tylko logowane – zgodne z projektem, ale nazwy
  sugerują odczyt wartości użytkowej. Nazwy `regen_command`, `regen_duration`, `regen_end`, `regen_block`
  sugerują semantykę, która **nie została zweryfikowana** (pliki `REGEN_*_DID_2026-10-15.md` to notatki
  badawcze). Brak ostrzeżenia w kodzie.
- **Propozycja:** dopisać `// UNVERIFIED` przy każdym wpisie lub przenieść `probe_label` do
  `const char*` z prefiksem `[?]`.

### 4.9 [DROBNY] Log diagnostyczny z surowymi bajtami rośnie bez limitu
- **Lokalizacje:** 1863–1867 (`raw_bytes`), 1894–1901.
- **Opis:** `MAX_ISOTP_PAYLOAD = 256`, więc `raw_bytes` może mieć ~768 znaków, a `candidate` kolejne.
  `ESP_LOGI` z takim stringiem na `std::string` = alokacje + duży log. Przy 10 DID-ach i 30 s throttle
  jest to akceptowalne, ale przy błędzie ECU (długa odpowiedź) potrafi zjeść stos/heap.
- **Propozycja:** ograniczyć `raw_bytes` do pierwszych 16–24 B (`i < std::min(response_.size(), 24U)`).

### 4.10 [DROBNY] `classify_cjcd_calibration` – dopasowanie prefiksu może zaakceptować zły CALID
- **Lokalizacje:** 41–50, 67–75.
- **Opis:** `known_format` zakłada dokładnie 15 znaków i wzór `03L`/`04L` + 6 cyfr + 2 litery + 4 cyfry.
  Dla CALID-ów VAG o innym formacie (np. 14 znaków) `known_format` = false → wynik `UNKNOWN`, co jest
  bezpieczne. Ale `cjcd_calibration_record_matches` porównuje **cały** compact string do
  `"03L906018SJ9977"` – to jest poprawne. Brak jednak obsługi CALID-ów z separatorami innymi niż `;`
  (np. `,`, `|`). W `decode_obd_response_` ID-y są łączone przez `"; "` (linia 1285) – spójne.
- **Propozycja:** dopisać komentarz o założonym separatorze i ewentualnie normalizować białe znaki
  w `classify_cjcd_calibration`.

### 4.11 [DROBNY] `obd_support_page_index_` – arytmetyka niezdefiniowana dla PID 0
- **Lokalizacja:** 929–936.
- **Opis:** dla `pid == 0` zwracany jest indeks 0 (poprawne, strona 0x00). Dla `pid < 0x20` i `pid != 0`
  `page = ((pid-1)/0x20)*0x20 = 0` → ok. Dla `pid = 0xFF` → `page = 0xE0`, indeks 7 → `-1`. Bezpieczne.
  Brakuje jednak `static_assert`/komentarza, że `OBD_SUPPORT_PAGES.size() == 5`.
- **Propozycja:** dodać `static_assert(OBD_SUPPORT_PAGES.size() == 5)` i `static_assert(OBD_DATA_PIDS.size() == obd_pid_response_seen_.size())`.

### 4.12 [DROBNY] `mark_obd_pid_response_`/`complete_data_response` mogą uznać krótką odpowiedź za poprawną
- **Lokalizacje:** 1327–1336.
- **Opis:** dla PID 0x05 (coolant) wystarczy `size() >= 3`, ale dekodowanie czyta `obd_response_[2]` – ok.
  Dla 0x0C wymaga `>= 4` – ok. Spójne, ale `complete_data_response` jest osobną listą warunków, która
  może się rozjechać z rzeczywistymi warunkami odczytu w `else if` – lepiej wyprowadzić z jednego miejsca.
- **Propozycja:** helper `bool obd_payload_complete_(uint8_t pid, size_t size)` używany w obu miejscach.

### 4.13 [DROBNY] `is_obd_egt_fresh()` – martwy warunek `cjcd_calibration_match_ == MATCHED`
- **Lokalizacje:** 395–401.
- **Opis:** najpierw `cjcd_core_enabled()` (która wymaga `cjcd_profile_selected_`), a potem dodatkowo
  `cjcd_calibration_match_ == MATCHED`. Drugi warunek jest zbędny logicznie (jest ostrzejszy, ale
  `cjcd_core_enabled` już zwraca false dla `MISMATCHED`; dla `UNKNOWN` przechodzi). Zwraca `true`
  przy `UNKNOWN` – świadome i poprawne, ale komentarz by się przydał.

---

## 5. Wydajność / alokacje w pętli

### 5.1 [ISTOTNY] Budowa `std::vector<uint8_t>` (8 B) na każde zapytanie
- **Lokalizacje:** 1160–1162, 1233, 1563–1575, 1685–1689.
- **Opis:** 4 miejsca tworzą `std::vector<uint8_t>` o rozmiarze 8 w każdej iteracji; `interval: 50 ms` +
  zapytania co ~250 ms → kilka alokacji na sekundę. `ESP32CanRecover::send_message` kopiuje do
  `twai_message_t`, więc wektor jest tylko opakowaniem.
- **Propozycja:** użyć `std::array<uint8_t, 8>` i przeciążenia `send_data` przyjmującego `std::span`/
  range, albo pola `std::array<uint8_t,8> tx_frame_` w klasie.

### 5.2 [ISTOTNY] `obd_response_`/`response_` bez `reserve()`
- **Lokalizacje:** 2117, 2088; zapisy 1631, 1673, 1737, 1195, 148.
- **Opis:** `std::vector` rośnie przez `assign`/`insert`/`push_back` – potencjalne realokacje w hot path.
  `begin_isotp_reassembly` robi `payload.assign(...)`, co przy pustym wektorze alokuje 6 B.
- **Propozycja:** `obd_response_.reserve(MAX_ISOTP_PAYLOAD); response_.reserve(MAX_ISOTP_PAYLOAD);` w
  `configure()`.

### 5.3 [DROBNY] `esphome::str_sprintf` w pętli przy każdym bajcie
- **Lokalizacja:** 1761, 1866.
- **Opis:** `for (...) result += esphome::str_sprintf(" %02X", …)` – dla 250 bajtów to 250 wywołań
  formatujących + 250 alokacji `std::string`.
- **Propozycja:** formatować raz (`%02X` w pętli z ręcznym zapisem do `char buf[...]`) lub użyć
  `std::array<char, 3>` i `append`.

### 5.4 [DROBNY] `std::string` zwracane przez API (kopie)
- **Lokalizacje:** 414, 444, 454, 470, 496, 532, 1757.
- **Opis:** YAML używa ich w `publish_state(std::string)` i `lv_label_set_text(c_str())`, więc
  zwracanie `const std::string&`/`std::string_view` lub `const char*` dla stałych statusów
  (`"OK"`, `"BRAK"`, …) ograniczyłoby alokacje (te zwracają literały w `std::string` – kopiowanie).
- **Propozycja:** zwracać `const char *` dla stałych i `std::string` tylko dla składanych.

### 5.5 [DROBNY] Kopiowanie `Parameter` przez `const auto &` w `send_current_query_` (ok) i przez wartość
- **Lokalizacja:** 1757 (`format_probe_candidate_(const Parameter &)` – ok).
- **Uwaga:** brak miejsc kopiujących duże obiekty; `parameters_` jest `static constexpr` – dobrze
  (choć `const char*` zamiast `std::string_view` w `Parameter` oszczędza inicjalizację).

---

## 6. Styl ESPHome

### 6.1 [ISTOTNY] Klasa nie dziedziczy po `Component` i nie używa `register_*`
- **Lokalizacja:** 228.
- **Opis:** `UdsManager` jest czystym obiektem globalnym, sterowanym z `interval: 50ms` w YAML.
  ESPHome ma `Component::loop()`/`update()` i `PollingComponent`. Brak dziedziczenia nie jest błędem
  (custom component może być używany tak, jak tutaj), ale:
  - `intervals` w YAML mają własny harmonogram i nie są priorytetyzowane względem innych komponentów;
  - nie ma `setup()`/`dump_config()` (konfiguracja jest w `configure()` wołanym ręcznie z `on_boot` priority 600),
  co jest nietypowe i utrudnia `dump_config`.
- **Propozycja:** rozważyć `class UdsManager : public Component` z `setup()/loop()/dump_config()`,
  zarejestrować przez `register_component()`; przynajmniej dodać `dump_config_` (poziom INFO)
  z aktywnym `tx_id_`, `PROFILE_MAPPING_VERIFIED`, `obd_link_stale_timeout_ms_`.

### 6.2 [ISTOTNY] Konfiguracja w `on_boot` priority 600 zamiast `setup()`
- **Lokalizacja:** `audi-dpf-monitor.yaml:177–235`.
- **Opis:** `configure()` zależy od `id(ecu_profile_selection)` i `PROFILE_MAPPING_VERIFIED`, więc kolejność
  ma znaczenie; przy tym wcześniejszy callback priority 600 zakłada, że encje już istnieją.
  `add_callback`/`add_activity_callback` są rejestrowane raz, dobrze, ale nie ma strażnika przed
  podwójnym `configure()` (np. po restarcie/reloadzie) – `configure()` nie sprawdza `configured_`.
- **Propozycja:** dodać `if (bus_ != nullptr && configured_) return;` lub reset stanu na początku
  `configure()`.

### 6.3 [DROBNY] Brak `dump_config()` / brak raportu konfiguracji
- **Propozycja:** dodać log przy starcie (TAG, `tx_id_`, `PROFILE_MAPPING_VERIFIED`,
  `did_diagnostic_probe_`, `cjcd_profile_selected_`, `obd_link_stale_timeout_ms_`).

### 6.4 [DROBNY] `TAG` zadeklarowany w środku klasy po metodach
- **Lokalizacja:** 885.
- **Opis:** styl ESPHome zwykle umieszcza `TAG` na górze klasy lub tuż pod `public:`, żeby logi
  wcześniej zdefiniowanych metod go „widziały”. Tutaj `configure()` (linia 232) używa `TAG` zdefiniowanego
  niżej – legalne w definicji klasy (klasa jest kompletna), ale mylące.
- **Propozycja:** przenieść `static constexpr const char *TAG` na początek klasy.

### 6.5 [DROBNY] `run_obd_parser_self_test()` tworzy 5 instancji `UdsManager` na stosie
- **Lokalizacje:** 610–790.
- **Opis:** `UdsManager parser; multi; regen; raw_regen` – każda instancja zawiera m.in.
  `std::array<uint32_t,512> obd_timeout_log_ms_` (2 KB) i wektory. Razem ~10 KB stosu w wywołaniu,
  które może trafić w kontekst pętli głównej (ESPHome task ma domyślnie 8 KB stosu!). To realne ryzyko
  przepełnienia stosu / crash.
- **Propozycja:** użyć `static UdsManager` (poza stosem) w selftest, albo zmniejszyć
  `obd_timeout_log_ms_` do `std::array<uint32_t, 10>` (punkt 1.6), co od razu rozwiązuje problem.

### 6.6 [DROBNY] Selftest loguje przez `ESP_LOGI("obd_parser_test", …)` z `int` zamiast `%d` dla `bool`
- **Lokalizacja:** 764–778.
- **Opis:** `%d` z `bool` jest promowane do `int`, więc formalnie ok, ale `-Wformat` przy `bool`
  bywa zgłaszany przy `-Wconversion`. Wartości są konwertowane do `int` niejawnie.
- **Propozycja:** `static_cast<int>(...)` w liście argumentów.

---

## 7. Uwagi dodatkowe / ryzyka projektowe

- **[ISTOTNY] Zapis zdarzeń poza kolejnością:** `decode_response_()` zapisuje do rejestratora
  (`record_did`, 1844) **tuż po** sprawdzeniu nagłówka 0x62 i zgodności DID, ale **przed** walidacją
  długości danych. `EventRecorder::record_did` sam weryfikuje długość per kanał (event_recorder.cpp:28),
  więc jest ok – warto to zaznaczyć w komentarzu, bo kontrakt jest niejawny.
- **[DROBNY] `event_recorder` jest surowym globalnym wskaźnikiem** ustawianym w `setup()` rejestratora;
  jeśli `EventRecorder` nie zostanie skonfigurowany, wskaźnik jest `nullptr` i kod go sprawdza – dobrze.
- **[DROBNY] Brak kontroli, że `did_probe_sensors_.size() == parameters_.size()`** (6 vs 10).
  W `decode_response_` sprawdzane jest `query_index_ < did_probe_sensors_.size()` (1806, 1869) – bezpieczne,
  ale tracone są dane dla DID-ów 6–9. Warto dodać `static_assert` lub rozszerzyć tablicę.
- **[DROBNY] `parameters_.size()` = 10 użyte jako literał w `std::array<…,10>`** (1535, 2091–2092, 2111) –
  zmiana liczby parametrów wymaga ręcznej synchronizacji 4 miejsc. `static_assert(parameters_.size() == did_seen_.size())`
  byłby tani i skuteczny.
- **[DROBNY] `cancel_pending_obd_request()` nie czyści kolejki sprzętowej** – komentarz w 540–541 mówi,
  że robi to komponent CAN; w YAML (2808) widać `uds_manager.cancel_pending_obd_request()` bez
  `id(obd_can).clear_transmit_queue()`. Warto zweryfikować, czy demo mode naprawdę czyści TWAI TX.
- **[DROBNY] Brak limitu długości odpowiedzi w `handle_single_frame_`** – `payload_length <= frame.size()-1`
  sprawdzone w `classic_can_sf_length_valid`, więc bezpieczne. Podkreślam, że `sprintf` w projekcie
  nie występuje – wszystkie formatowania idą przez bezpieczne `str_sprintf`/`snprintf` – to na plus.

---

## Podsumowanie

**Mocne strony:** konsekwentna walidacja długości ramek ISO-TP, brak `sprintf`/`strcpy` w kodzie
(wszędzie `str_sprintf`, `assign`, `insert` z policzonymi iteratorami), rozbudowane testy compile-time
(`static_assert`) i self-test parsera, jawny „fail-safe” dla alarmu regeneracji, poprawne użycie
`std::min` przy docinaniu buforów, brak wycieków pamięci (żadnych `new`/`malloc` w klasie).

**Najpoważniejsze znaleziska (do naprawy w pierwszej kolejności):**
1. **[KRYTYCZNY] Brak synchronizacji współdzielonego stanu** (1.1) – dziś działa tylko dzięki
   przypadkowi (callbacki w pętli głównej); każda zmiana architektury = wyścig i możliwy odczyt poza buforem.
2. **[ISTOTNY] Gubiona transakcja OBD, gdy sensor towarzyszący jest `nullptr`** (4.1) – realne zawieszanie
   pollingu na 500 ms po zmianie YAML.
3. **[ISTOTNY] `did_seen_`/freshness nigdy nie ustawiane dla 6 z 10 DID-ów w trybie diagnostycznym** (4.2)
   – UI pokazuje świeże dane jako nieaktualne.
4. **[ISTOTNY] Martwe odwołania do DID 0xF401** (4.3) – logika regeneracji nigdy nie używa tej ścieżki,
   a kod sugeruje, że działa.
5. **[ISTOTNY] Ryzyko przepełnienia stosu w `run_obd_parser_self_test()`** (6.5) – 5 instancji klasy
   z tablicą 2 KB każda na stosie 8 KB taska.
6. **[ISTOTNY] Niespójne warunki FF/`PROFILE_MAPPING_VERIFIED=false`/brak `dump_config`** (2.1, 1.4, 6.2–6.3).

**Zalecenia przekrojowe:** wprowadzić jeden mutex/kontekst wykonania i udokumentować kontrakt wątkowy;
zredukować ślad pamięciowy instancji (tablice 512 → 10) i dodać `static_assert`-y spójności rozmiarów;
przenieść walidację poza warunki publikacji; ujednolicić implementacje ISO-TP; dodać `reserve()` i
`std::array` zamiast `std::vector` dla ramek 8-bajtowych; uruchomić `clang-format`.
