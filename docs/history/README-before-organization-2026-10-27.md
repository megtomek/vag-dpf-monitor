# Samochodowy Monitor DPF

## Pobranie projektu na inny komputer

Repozytorium zawiera kod, dokumentację, lokalny frontend WWW i czcionki. Hasła oraz logi diagnostyczne pozostają poza repozytorium. Skopiuj `secrets.example.yaml` do `secrets.yaml`, uzupełnij własne dane Wi-Fi, AP, panelu i OTA, następnie postępuj według [BUILD_INSTRUCTIONS.md](../../BUILD_INSTRUCTIONS.md). Nie wysyłaj `secrets.yaml` ani gotowych obrazów firmware na GitHub — obraz może zawierać dane dostępowe.

Aktualny stan opisuje [PROJECT_STATUS.md](../../PROJECT_STATUS.md). Alarm podstawowy jest szacowany z EGT3; pierwsze zdarzenie i potwierdzenie działania ekranu opisuje [FIRST_REAL_EVENT_2026-10-22.md](../../FIRST_REAL_EVENT_2026-10-22.md). Poniższe szczegółowe opisy zawierają również wcześniejsze etapy projektu.

Projekt ESPHome dla wyświetlacza Waveshare ESP32-S3-Touch-AMOLED-1.64. Firmware odpytuje sześć DID UDS oraz standardowe PID-y OBD-II naprzemiennie, bez równoległych żądań. Ręczny profil CJCD udostępnia sadzę obliczoną, zmierzoną i ciśnienie na podstawie pojedynczego porównania z OBDeleven. Pozostałe DID-y i alarm regeneracji wymagają walidacji w ECU `03L906018SJ / 9977 / ODX 003003`. Panel pokazuje zgodność CALID i blokuje parametry CJCD przy rozpoznanym innym oprogramowaniu.

Aktualne dowody i brakujące testy: [PROJECT_STATUS.md](../../PROJECT_STATUS.md). Projekt nie jest jeszcze ukończony.

## Aktualny interfejs

Ekran pracuje poziomo w rozdzielczości 456 × 280 px i ma pięć widoków wybieranych dolnymi przyciskami:

- **DPF** — obliczona masa sadzy oraz status regeneracji.
- **TEMP** — kanały EGT 1–3 z PID `0x78` oraz osobna temperatura wlotu DPF z PID `0x7C`, jeśli ECU je jawnie udostępnia. Numer czujnika EGT nie potwierdza jego fizycznego położenia w wydechu.
- **FILTR** — sadza zmierzona, dystans od regeneracji oraz różnica ciśnień DPF z PID `0x7A`, jeżeli ECU jawnie udostępnia świeże pole. Brak lub nieaktualność danych pokazuje się jako `--.-- kPa`; wartość i obsługę PID-u trzeba potwierdzić w aucie.
- **HIST** — liczba pełnych cykli i czas ostatniego cyklu zaobserwowanego przez monitor (nie jest to licznik ECU).
- **USTAW** — ostatnia, piąta karta: DEMO, jasność, trzy tryby sieci (router Wi-Fi → sam AP konfiguracji → radio wyłączone) oraz automatyczny widok DPF → TEMP → FILTR → HIST co 5 s. Wybrany tryb sieci jest zapamiętywany po restarcie; AP nie uruchamia się samoczynnie. USTAW jest pomijany podczas automatycznego przewijania. Profil ECU pozostaje ustawieniem panelu WWW; CJCD udostępnia częściowo porównane parametry, ale sam wybór profilu nie uruchamia alarmu regeneracji.

Alarm aktywnej regeneracji `REGENERACJA DPF — NIE GAŚ!` jest obsługiwany na wszystkich pięciu widokach, także na HIST; tło miga naprzemiennie na czerwono i bursztynowo.

Historia zapisuje obserwowany przez monitor epizod statusu regeneracji ECU — aktywnej lub pasywnej — dopiero po świeżym stanie nieaktywnym, rozpoczęciu epizodu i powrocie do świeżego stanu nieaktywnego. Epizod trwający przy starcie, przerwany utratą danych albo odtworzony w DEMO nie jest zaliczany. Licznik nie dowodzi całkowitego wypalenia sadzy; to zapis przejścia statusu ECU widzianego przez monitor. Czas jest mierzony lokalnie, nie stanowi pełnego zapisu serwisowego ani licznika dystansu.

Wartości pozostają puste, gdy nie ma świeżych danych.

### Pomiar szybkości UI

Na tej płytce porównano pełne odrysowanie aktywnego ekranu: bufor LVGL 12% osiągał 184 ms, a 25% 112 ms w powtórzonych pomiarach testowych. Firmware roboczy używa teraz 25%; wymuszone odrysowanie i logowanie pomiaru pozostają wyłączone. Pomiar obejmował rysowanie LVGL i zakończenie przesłania obrazu, nie jest pomiarem płynności animacji. Po zmianie użytkownik potwierdził, że UI działa płynniej, obraz wydaje się stabilny, a dotyk reaguje; nie jest to laboratoryjny pomiar FPS.
Gdy monitor jest zasilany tylko z USB i nie jest połączony z autem przez CAN, komunikat `BRAK DANYCH` jest oczekiwany. Użytkownik potwierdził czytelność UI i działające przełączanie stron. Zielony pasek statusu został usunięty w normalnym trybie.

Karta **USTAW → DANE DPF DO PORÓWNANIA** pokazuje kandydaty `114F`, `114E`, `14F5`, `1156`, `11B2` i `10FB`. Status `CAN: BRAK RUCHU / RUCH WYKRYTY / CISZA` opisuje odebrane ramki standard CAN, nie potwierdzenie odpowiedzi na DID. Brak odpowiedzi nie oznacza nieobsługiwanego DID-u. Tryb porównawczy odpytuje kandydackie DID-y naprzemiennie ze standardowymi PID-ami OBD-II, przy jednym żądaniu w toku. Profil CJCD udostępnia `114F`, `114E` i `14F5` na zwykłych kartach po ręcznym wyborze w panelu WWW; ich dopasowanie opiera się na jednej sesji z OBDeleven i wymaga kolejnych porównań. Żaden kandydacki DID nie steruje alarmem regeneracji.

Porównania monitora i OBDeleven z 11 października 2026 wspierają `114F` i `114E` jako setne grama, a `14F5` jako hPa. Po ręcznym wybraniu profilu CJCD te trzy odczyty są udostępniane na zwykłych kartach DPF/FILTR; to wstępne dopasowanie oparte na jednej sesji porównawczej, nie pełna walidacja mapy dla wszystkich warunków. `1156` jako U32 big-endian w metrach pozostaje tylko na stronie diagnostycznej. Błędne `0.8 km` wynikało z pominięcia czwartego bajtu odpowiedzi `1156`; pełne `00 03 A2 E9` oznacza `238313 m`, czyli `238.313 km`. `11B2`/`10FB` nadal są kandydatami temperatury, a alarm regeneracji nie korzysta z tych DID-ów. Szczegóły i ograniczenia porównania: [DPF_PROFILE_OBDELEVEN_CAPTURE.md](../../DPF_PROFILE_OBDELEVEN_CAPTURE.md).

### Znaczenie komunikatów o regeneracji

- `NIE GAŚ!` pojawia się wyłącznie po otrzymaniu świeżego, obsługiwanego przez ECU sygnału, że regeneracja trwa.
- Status regeneracji ma osobny limit świeżości 10 s; pozostałe odczyty korzystają z limitu 15 s. Po przekroczeniu limitu alarm znika, a status przechodzi do `BRAK DANYCH`. Wspólna funkcja uwzględnia przepełnienie licznika `millis()`.
- Jeżeli standardowy PID `0x8B` jawnie raportuje regenerację pasywną, monitor pokazuje `REGEN PASYWNA` bez alarmu `NIE GAŚ!`. Dekoder bitów opiera się na tabeli SAE J1979-DA Oct 2011. Katalog SAE wskazuje nowszą rewizję `J1979DA_202607` z 16 lipca 2026 r., której treści nie porównano jeszcze z kodem; zgodność z aktualnym standardem i ECU CJCD nadal wymaga potwierdzenia. [Rekord rewizji SAE](https://saemobilus.sae.org/standards/j1979da_202607-j1979-da-digital-annex-e-e-diagnostic-test-modes)
- `DPF OK` oznacza wyłącznie, że świeży sygnał ECU nie zgłasza aktywnego wypalania. Nie jest oceną sprawności filtra ani informacją, że filtr kwalifikuje się do regeneracji.
- `BRAK DANYCH` oznacza, że monitor nie ma świeżego i potwierdzonego sygnału regeneracji. Nie należy interpretować tego jako „regeneracja nieaktywna”.

Poza DEMO monitor pobiera standardowe PID-y OBD-II przez CAN. Obecny tryb porównawczy odpytuje je naprzemiennie z sześcioma kandydackimi DID-ami UDS, aby przygotować porównanie. DEMO używa wartości symulowanych i nie może służyć do diagnozy auta.

## Ustawienia przez przeglądarkę

Firmware może łączyć się z lokalnym Wi-Fi i udostępnia stronę ustawień ESPHome Web Server v3. Strona działa bez Internetu, ma logowanie HTTP Digest i pozwala zmienić profil (AUTO lub kandydat CJCD), jasność, DEMO i tryb sieci. Dotknięcie kafelka sieci na USTAW przełącza kolejno: router Wi-Fi, AP `VAG-DPF-Setup` (panel pod `http://192.168.4.1/`) i całkowite wyłączenie radia. Ustawienie przetrwa restart; AP nie uruchamia się automatycznie. Odczyty i diagnostyka są opisane po polsku oraz podzielone na sekcje DPF, temperatur, silnika, diagnostyki i ustawień. Aktualizacja przez stronę jest wyłączona; po jednorazowym wgraniu USB aktualizacje można wysyłać natywnym protokołem OTA ESPHome z hasłem `ota_password`. Połączenie Wi-Fi nie jest potrzebne do odczytu CAN ani pracy ekranu.

Uzupełnij `secrets.yaml` kluczami `wifi_ssid`, `wifi_password`, `dpf_ap_password`, `web_server_username`, `web_server_password` i `ota_password`. Hasło OTA jest generowane lokalnie i nie jest wyświetlane w logach. Nie udostępniaj tego pliku — jest ignorowany przez Git. Wybierz tryb sieci na ekranie USTAW lub w panelu WWW: router Wi-Fi, AP `VAG-DPF-Setup` pod `http://192.168.4.1/` albo całkowite wyłączenie radia. Zmiana jest zapamiętywana po restarcie. Gdy urządzenie połączy się z domowym Wi-Fi, otwórz `http://audi-dpf-monitor.local/` albo adres IP przydzielony przez router. Dane logowania do strony są w tych samych lokalnych sekretach.

Użytkownik potwierdził, że strona ładuje się i można się zalogować zarówno przez fallback AP, jak i po połączeniu monitora z domowym Wi-Fi. Panel przydzielił urządzeniu adres z DHCP; adres może się zmienić po ponownym połączeniu. Kontrolki profilu, jasności i DEMO sprawdzono przez uwierzytelnione API: zmiany zadziałały, a ustawienia po próbie przywrócono. W Chrome sprawdzono ciemny motyw oraz lokalne zasoby CSS/JS; zablokowanie zewnętrznego API ikon nie uniemożliwiło uruchomienia panelu. Wygląd po tej zmianie oczekuje jeszcze na ocenę użytkownika. Ręczny wybór profilu CJCD udostępnia trzy częściowo porównane wartości na zwykłych kartach; nie włącza niepotwierdzonych temperatur, dystansu ani DID-owego alarmu regeneracji.

Aktywny YAML zawiera Wi-Fi i lokalny web server do ustawień, ale nie integruje się z Home Assistant. OTA przez web server jest wyłączone. CAN jest domyślnie włączony do pracy w samochodzie. Na biurku, bez transceivera, użyj wariantu `-CanBusEnabled false`; dostępne testy rozdzielające nasłuch CAN od odpytywania OBD opisuje [BUILD_INSTRUCTIONS.md](../../BUILD_INSTRUCTIONS.md).

Na karcie **USTAW** można włączyć **tryb DEMO**. Przez 120 sekund pokazuje kolejno 60 s regeneracji aktywnej (z alarmem `NIE GAŚ!`), 30 s regeneracji pasywnej (bez alarmu) i 30 s chłodzenia. Bursztynowa etykieta `DEMO` jest widoczna na wszystkich kartach. Dotknięcie przycisku ponownie zatrzymuje symulację; restart płytki zawsze wyłącza DEMO. Symulowane wartości nie pochodzą z ECU i nie uruchamiają CAN.

Rozmiary tekstu można zmienić w sekcji `substitutions` na początku `audi-dpf-monitor.yaml`: `ui_font_small`, `ui_font_nav`, `ui_font_label`, `ui_font_value` i `ui_font_hero`. Czcionka Ubuntu zawiera polskie znaki.

Karta **HIST** zlicza tylko pełne regeneracje, które monitor zaobserwował od potwierdzonego stanu nieaktywnego do kolejnego stanu nieaktywnego. To nie jest całkowity licznik ECU; DEMO i cykle rozpoczęte przed uruchomieniem monitora nie są doliczane.

Na karcie **USTAW** przycisk `JASNOŚĆ` przełącza poziomy 220 (dzień), 130 (przygaszony) i 60 (noc). Wybrany poziom jest zapisywany w preferencjach ESPHome i przywracany po restarcie; zapis do flash jest synchronizowany domyślnie co 60 s, więc po zmianie odczekaj minutę przed odłączeniem zasilania. Poziom 220 jest ustawieniem początkowym dla nowych urządzeń. Zmiana dotyczy komendy jasności kontrolera CO5300, nie osobnego podświetlenia LED.

## Struktura projektu

- `audi-dpf-monitor.yaml` — aktywna konfiguracja ESPHome.
- `components/ft3168/` — lokalny sterownik dotyku wymagany przez konfigurację.
- `components/esp32_can_recover/` — sterownik TWAI CAN z odzyskiwaniem po utracie magistrali.
- `components/dpf_network/` i `wifi_ap_control.h` — start zapisanego trybu sieci oraz zachowanie konfiguracji routera przy AP/OFF i braku zasięgu.
- `fonts/` — lokalne fonty Ubuntu.
- `firmware/V1/` — bieżące obrazy produkcyjne dla przyjętej roboczo rewizji V1 (GPIO9). Archiwalne buildy testowe i nieużywana V1.1 są w `firmware/archive/`.
- `tests/` — testowe i diagnostyczne konfiguracje YAML, oddzielone od aktywnego firmware.
- `Kompiluj-monitor-DPF.ps1` — waliduje i kompiluje wybrany wariant.
- `Wgraj-monitor-DPF.ps1` — kompiluje i domyślnie wgrywa przez OTA; do pierwszego flashowania użyj jawnie `-Transport USB -Device COM6`.
- `ENVIRONMENT.md` — kanoniczne ścieżki narzędzi, builda i wskazówki dla blokady COM6 / dostępu do przeglądarki.
- `BUILD_INSTRUCTIONS.md` — komendy kompilacji, wgrywania i diagnostyki.
- `DPF_PROFILES.md`, `DPF_PROFILE_OBDELEVEN_CAPTURE.md`, `DPF_PROFILE_COMPARISON.csv` — profil ECU, procedura oraz arkusz porównania z OBDeleven.
- `DEVELOPMENT_ROADMAP.md` — postęp stołowy i kolejność walidacji wymagającej auta.
- `archive/diagnostics/` — historyczne logi; nie są potrzebne do builda.
- `archive/legacy-snapshots/` — scalone, wcześniejsze kopie projektu z katalogu `Documents\Codex`; wyłącznie do wglądu/rollbacku.
- `firmware/archive/` — wcześniejsze obrazy testowe, nieużywana konfiguracja V1.1 i snapshot komponentów.

Do samodzielnej kompilacji zachowaj obok YAML-a pliki `uds_audi.h`, `wifi_ap_control.h`, katalogi `fonts/`, `components/ft3168/`, `components/esp32_can_recover/`, `components/dpf_network/` oraz lokalny `secrets.yaml` z wymaganymi kluczami. Plik z hasłami jest ignorowany przez Git i nie może być publikowany.

## Kompilacja

W PowerShellu z katalogu projektu:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\Kompiluj-monitor-DPF.ps1' -BoardRevision V1
```

Dla bieżącej płytki używaj `-BoardRevision V1` (GPIO9). Skrypt ogranicza wybór do V1, zgodnie z przyjętym roboczo wariantem; wcześniejsze obrazy V1.1 zachowujemy tylko jako archiwalne pliki. Skrypt zapisuje cache builda w `.esphome-data/` i kopiuje obrazy do `firmware/V1/`. Szczegóły są w [BUILD_INSTRUCTIONS.md](../../BUILD_INSTRUCTIONS.md).

Wariant V1 z GPIO9 został skompilowany i wgrany na płytkę 6 października 2026 r.; log startowy potwierdził uruchomienie firmware. Rewizja na PCB jest nieczytelna, dlatego V1 jest przyjętym roboczo wariantem tej płytki. Wcześniejszy obraz V1.1 z GPIO46 nie był wgrywany ani sprawdzany na sprzęcie. Żaden z tych testów nie potwierdza jeszcze odczytów DPF z ECU.

Wariant NORMAL V1 z kartą USTAWIENIA skompilowano i wgrano na COM6 8 października 2026 r.; zapis potwierdzono hashem. Użytkownik potwierdził czytelny interfejs i działający dotyk. Późniejsze logi na stole pokazały stabilne CAN i pamięć przy braku transceivera/auta. Obsługa PID-ów i alarm regeneracji nadal wymagają porównania z OBDeleven w samochodzie.

8 października poprawiono kartę TEMP: EGT 1–3 pochodzą z kanałów PID `01 78`, a „DPF WLOT” z osobnego PID `01 7C` lub DID UDS. W DEMO wszystkie te pola otrzymują wyraźnie oznaczone wartości symulowane. Status „NIE GAŚ!” ma teraz 26 px (wcześniej 24 px; osobny napis w górnym alarmie pozostaje 20 px). Wariant NORMAL V1 skompilowano i wgrano na COM6, a zapis zweryfikowano hashem. Log po wgraniu przez ok. 78 s pokazał stabilną pamięć, brak błędów CAN i brak nieodzyskanych błędów dotyku. Użytkownik potwierdził, że temperatury i rozmiar alarmu wyglądają prawidłowo.


## Pobieranie historii logów z monitora

W panelu WWW kliknij **Pobierz logi**. Funkcja działa na adresie monitora w sieci lokalnej i w AP pod `http://192.168.4.1/`. Oba tryby udostępniają jeden serwer i te same lokalne pliki `webserver-theme.css` oraz `webserver-v3.js`; przycisk korzysta z adresu względnego `/dpf/logs.txt` i obecnego logowania HTTP Digest.

Bufor przechowuje ostatnie **10 minut lub 1024 wpisy**. Przy intensywnym logowaniu dostępny okres będzie krótszy. Pojedyncza linia ma maksymalnie 479 bajtów tekstu. Nagłówek pliku podaje liczbę nadpisanych, skróconych i pominiętych wpisów. Każdy wpis zawiera czas od uruchomienia w milisekundach, nie datę kalendarzową. Bufor zaczyna działać podczas setup komponentów, więc nie zawiera najwcześniejszych komunikatów bootloadera.

Historia jest wyłącznie w PSRAM: reset, OTA i odłączenie zasilania ją kasują. Zużycie stałe to ok. 488 KiB PSRAM, pobieranie rezerwuje dodatkowo ok. 513 KiB na kopię. Przechwytywanie nie alokuje pamięci na każdy log i nie zapisuje flash; jeśli bufor jest chwilowo zajęty podczas kopiowania, log zostaje pominięty zamiast blokować UI. Plik może zawierać VIN, SSID i inne dane diagnostyczne — sprawdź zawartość przed publicznym udostępnieniem.

Po aktualizacji odśwież panel, aby załadować nowy przycisk. Nie ma osobnego motywu dla AP.
