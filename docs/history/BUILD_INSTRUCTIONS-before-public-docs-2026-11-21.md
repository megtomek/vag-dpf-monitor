> Archiwum instrukcji sprzed przygotowania publicznej dokumentacji (21.11.2026). Aktualna instrukcja: [BUILD_INSTRUCTIONS.md](../../BUILD_INSTRUCTIONS.md). Datowane obserwacje poniżej nie są obecną deklaracją zgodności.

# Instalacja i kompilacja monitora DPF

## Bieżący etap

Aktywny YAML używa CAN, standardowych PID OBD-II oraz odpytywania DID UDS dla CJCD. Podstawowy alarm jest szacowany z EGT3 11B2, a surowe kanały statusów nie są potwierdzoną flagą wypalania. Bez podłączenia do auta brak danych jest normalny. Interfejs ma pięć kart: DPF, TEMP, FILTR, HIST i USTAW. Bieżący stan i ograniczenia: [PROJECT_STATUS.md](../../PROJECT_STATUS.md); instrukcje niżej zawierają również datowane wyniki wcześniejszych prób.

Przy podstawowym alarmie szacowanym karta HIST pokazuje dwa ostatnie zachowane zdarzenia NVS i czas najnowszego zapisu (z rozgrzewaniem/schłodzeniem), bez deklarowania potwierdzonego cyklu ECU. Zapis przerwany brakiem danych, restartem lub limitem ma osobny opis. Przy wyłączonym trybie szacowanym zachowano wcześniejszy licznik pełnych przejść statusu ECU; DEMO nie tworzy rzeczywistych zapisów.

Aktywny YAML zawiera Wi-Fi i lokalny web server do ustawień monitora, bez integracji z Home Assistant. Kafelek sieci na USTAW cyklicznie wybiera router Wi-Fi → AP `VAG-DPF-Setup` (panel `http://192.168.4.1/`) → całkowicie wyłączone radio. Każdy tryb jest zapamiętywany po restarcie; AP nie uruchamia się samoczynnie, a wyłączenie radia kończy próby łączenia. Dostępny jest też tymczasowy automatyczny widok: przełącza karty DPF → TEMP → FILTR → HIST co 5 s i pomija USTAW; po restarcie jest wyłączony. OTA przez stronę jest wyłączone; po pierwszym flashowaniu przez USB dostępne jest natywne, hasłowane OTA ESPHome. CAN jest domyślnie włączony dla pracy w samochodzie; do testu na biurku bez transceivera kompiluj i wgrywaj z `-CanBusEnabled false`.

### Odczyt logów przez sieć

ESPHome CLI 2026.9.1 używa przy `esphome logs` uwierzytelniania HTTP Basic, natomiast web server monitora wymaga HTTP Digest. Dlatego CLI może zgłaszać HTTP 401 mimo prawidłowych danych. Do odczytu użyj skryptu Digest z projektu; jest tylko do odczytu i pobiera nazwę oraz hasło z `secrets.yaml`, nie wypisując ich:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\Czytaj-logi-DPF.ps1' -Device 192.168.0.105 -Seconds 30
```

Domyślnie skrypt łączy się z `audi-dpf-monitor.local` i strumieniuje do Ctrl+C. Podczas połączenia z AP podaj `-Device 192.168.4.1`. Parametry `-Port` i `-Seconds` pozwalają zmienić port i czas przechwytywania.

Do testu UI na biurku można całkowicie wyłączyć kontroler CAN parametrem `-CanBusEnabled false`. Ten obraz jest przeznaczony wyłącznie do diagnostyki ekranu; nie odczytuje auta. Domyślne `true` przywraca normalną obsługę magistrali.

Do testu wpływu samych logów USB Serial/JTAG ustaw parametr LoggerBaudRate na 0. Pozostaw CAN wyłączony (CanBusEnabled false), ten sam przewód, port komputera i jasność ekranu. W tym trybie ESPHome nie wysyła logów przez USB; port USB nadal służy do programowania. Aby przywrócić logi, wgraj ponownie z LoggerBaudRate 115200 albo pomiń ten parametr.

Przykład wgrania testu CAN-off bez wyjścia logów: powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Wgraj-monitor-DPF.ps1 -BoardRevision V1 -CanBusEnabled false -LoggerBaudRate 0 -Transport USB -Device COM6. Aby przywrócić logi przy CAN-off, użyj tej samej komendy z LoggerBaudRate 115200.

Do kompilacji samodzielnego firmware zachowaj obok YAML-a katalogi `fonts/`, `components/ft3168/`, `components/esp32_can_recover/`, `components/dpf_network/`, `components/dpf_log_archive/`, `uds_audi.h`, `wifi_ap_control.h`, `webserver-theme.css`, `webserver-v3.js`, `webserver-v3.LICENSE.txt` i `secrets.yaml`. Pakiet JS jest lokalnym, przypiętym do ESPHome 2026.9.1 frontendem v3; CSS i JS są wgrywane do firmware, więc panel nie potrzebuje CDN. Nie ustawiaj tu `web_server.local: true`: w wersji v3 ESPHome używa wtedy wbudowanej strony, która omija `css_include`. W `secrets.yaml` muszą być klucze `wifi_ssid`, `wifi_password`, `dpf_ap_password`, `web_server_username`, `web_server_password` i `ota_password`. Sekrety są lokalne i ignorowane przez Git; nie wklejaj ich do logów ani repozytorium.

## Strona ustawień przez przeglądarkę

Tryb AP nie uruchamia się automatycznie. Na ekranie USTAW wybierz „AP: VAG-DPF-Setup” albo w panelu WWW ustaw „Tryb sieci monitora”. Połącz telefon lub komputer z tą siecią, używając hasła z `dpf_ap_password`, i otwórz ręcznie `http://192.168.4.1/`. Następnie zaloguj się danymi z `web_server_username` i `web_server_password`. Aby ograniczyć próby łączenia, wybierz „Sieć wyłączona”; tryb zostanie zapamiętany także po restarcie.

Jeżeli monitor połączył się z domowym Wi-Fi, stronę otwórz pod `http://audi-dpf-monitor.local/` lub pod adresem IP widocznym w routerze. Profil ECU wybiera się na stronie; na ekranie USTAW pozostały DEMO, jasność, przełącznik AP i automatyczny widok. Ta strona nie aktualizuje firmware. Po pierwszym wgraniu można aktualizować przez OTA ESPHome, gdy komputer jest w tej samej sieci co monitor. Przy AP połącz komputer z `VAG-DPF-Setup` i użyj adresu `192.168.4.1`. Do pierwszego wgrania, zmiany partycji/bootloadera lub odzyskiwania użyj USB.

Aktualizację OTA z komputera połączonego z AP wykonuje ten sam skrypt w trybie sieciowym. Skrypt kompiluje firmware lokalnie i wybiera właściwy obraz OTA:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\Wgraj-monitor-DPF.ps1' -BoardRevision V1 -CanBusEnabled true -Transport OTA -Device 192.168.4.1
```

ESPHome użyje hasła OTA z lokalnego `secrets.yaml`. Ta konfiguracja uwierzytelnia upload, ale nie szyfruje obrazu firmware; używaj OTA tylko w zaufanej, bezpośredniej sieci. Domyślnie skrypt używa OTA na `audi-dpf-monitor.local`, dzięki czemu aktualizacje nie zajmują COM6. Dla pierwszego flashowania lub odzyskiwania przez USB podaj jawnie `-Transport USB -Device COM6`. W trybie AP podaj `-Device 192.168.4.1`; jeśli mDNS nie działa, wpisz aktualny adres DHCP.

Przy diagnozie problemu połączenia obserwuj log USB: udane połączenie powinno zakończyć się komunikatem `Got IP address`. Powtarzający się `4-Way Handshake Timeout` oznacza, że etap uwierzytelnienia nie został zakończony. W aktualnej konfiguracji brak routera nie uruchamia fallback AP; wybierz ręcznie AP albo całkowite wyłączenie radia na karcie USTAW.

## Wymagane narzędzia i pliki

Projekt używa ESPHome 2026.9.1 oraz ESP-IDF 5.5.5. Skrypt `Kompiluj-monitor-DPF.ps1` wybiera root narzędzi kolejno z `ESPRESSIF_TOOLS_PATH`, `ESPHOME_ESP_IDF_PREFIX`, Pythona zapisanego w istniejącym `CMakeCache.txt`, a dla nowego builda z `C:\Espressif` albo `.esphome-idf` projektu. Dzięki temu interpreter IDF pozostaje zgodny z cache’em CMake. Cache builda znajduje się w `.esphome-data/`. Nie usuwaj tego katalogu — zawiera bieżące pliki kompilacji i pobrane zależności. Skrypt uruchamia ESPHome przez `python -m esphome`, korzystając z `ESPHOME_PYTHON`, `.venv` w projekcie albo instalacji w `Documents\Codex\2026-10-01\c`.

W ograniczonym procesie narzędziowym kompilator może zwrócić Windows `WinError 623` / „Nieprawidłowa relokacja biblioteki DLL systemu”. W tym środowisku ten sam Xtensa uruchomił się poza ograniczeniem, a pełna kompilacja powiodła się ze ścieżką `C:\Espressif`. Nie wymagało to reinstalacji toolchainu. ESPHome może też zgłosić brak DNS przy pobieraniu narzędzi, jeśli wybrany root nie ma kompletnego cache.

## Walidacja i kompilacja

Uruchom PowerShell w katalogu projektu:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\Kompiluj-monitor-DPF.ps1' -BoardRevision V1
```

Obraz testowy bez kontrolera CAN:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\Kompiluj-monitor-DPF.ps1' -BoardRevision V1 -CanBusEnabled false
```

Przyjęliśmy roboczo rewizję V1, bo oznaczenie na płytce jest nieczytelne, a wariant V1 działał na urządzeniu. Skrypt ogranicza kompilację do V1 z `LCD_CS` na GPIO9. Wcześniejszy obraz V1.1 pozostaje wyłącznie archiwalny i nie jest obsługiwany przez skrypt.

Skrypt wykonuje walidację konfiguracji, kompiluje firmware i zapisuje obrazy w `firmware\V1`. Wariant V1 z kartą USTAWIENIA, Wi-Fi i web serverem skompilowano i wgrano OTA 8 października 2026 r. Użytkownik potwierdził przydzielony adres `192.168.0.105` i otwieranie panelu w Chrome. Odczyt diagnostyczny `/events` zwrócił HTTP 200; interakcje z kontrolkami oraz wygląd nowego motywu nadal wymagają sprawdzenia. Jeśli kolejny build zgłosi nowy błąd, kieruj się najnowszym komunikatem, nie archiwalnymi logami w `archive/diagnostics/`.

`-ExecutionPolicy Bypass` dotyczy tylko uruchamianego procesu PowerShell; nie zmienia ustawień systemowych. Skrypt ustawia ścieżki ESP-IDF w swoim procesie i najpierw odczytuje toolchain zgodny z bieżącym cache’em CMake.

## Wgrywanie przez USB

Po kompilacji sprawdź port płytki w Menedżerze urządzeń lub poleceniem:

```powershell
Get-CimInstance Win32_SerialPort | Select-Object DeviceID, Name
```

Użyj skryptu, który najpierw kompiluje wybrany wariant, sprawdza obecność obrazu fabrycznego i jawnie wskazuje go do wgrania z katalogu `.esphome-data`. Zastąp `COM6` właściwym portem płytki:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\Wgraj-monitor-DPF.ps1' -BoardRevision V1 -CanBusEnabled false -Transport USB -Device COM6
```

`-CanBusEnabled false` wyłącza CAN na stole bez transceivera. Do pracy w aucie, po podłączeniu transceivera, jawnie użyj `-CanBusEnabled true`. Skrypt zawsze kompiluje przed uploadem i wskazuje `.esphome-data`, dzięki czemu nie wgrywa starego obrazu z `.esphome`.

Domyślnie `Wgraj-monitor-DPF.ps1` przesyła `firmware.ota.bin` przez sieć pod `audi-dpf-monitor.local`, więc nie przejmuje COM6. Z `-Transport USB -Device COM6` skrypt wgrywa kompletny `firmware.factory.bin`; tej opcji używaj do pierwszego flashowania lub odzyskiwania. Kopie obrazów dla wybranej rewizji są w `firmware\<rewizja>\`.

**Nie wgrywaj ręcznie `firmware.ota.bin` przez `esphome upload --file` na USB.** W ESPHome 2026.9.1 ta ścieżka zapisuje wskazany plik od offsetu `0x0`; OTA nie zawiera bootloadera ani tablicy partycji. Do USB używaj `Wgraj-monitor-DPF.ps1`, który uruchamia upload z katalogu właściwego projektu bez ręcznego wskazywania pliku. Jeśli musisz użyć `--file` bezpośrednio, wskaż kompletny `firmware.factory.bin` (zapisany od offsetu `0x0`).

## Logi z urządzenia

Przy podłączonym USB:

```powershell
$env:ESPHOME_DATA_DIR = Join-Path (Get-Location).Path '.esphome-data'
if ($env:ESPHOME_PYTHON) { $esphomePython = $env:ESPHOME_PYTHON }
elseif (Test-Path '.\.venv\Scripts\python.exe') { $esphomePython = (Resolve-Path '.\.venv\Scripts\python.exe').Path }
else { $esphomePython = Join-Path $env:USERPROFILE 'Documents\Codex\2026-10-01\c\.venv\Scripts\python.exe' }
& $esphomePython -m esphome logs audi-dpf-monitor.yaml --device COM6
```

W aktualnym firmware logger ma `deassert_rts_dtr: true`. ESPHome ustawia linie RTS/DTR nieaktywne przed otwarciem portu, co zapobiegało resetowi płytki podczas podłączania logów. Zwykłe polecenie `logs` nie używa `--reset`; nie dodawaj tej opcji, jeśli chcesz zachować uptime lub aktywny DEMO. Wgranie firmware nadal normalnie restartuje ESP32, a DEMO po starcie jest wyłączone.

Ograniczony do 8 godzin, tylko-odczytowy test stabilności NORMAL przez USB uruchamia skrypt:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\archive\diagnostics\Collect-NormalUsbLogs.ps1' -DurationHours 8 -Device COM6
```

Skrypt nie kompiluje ani nie wgrywa firmware. Zapisuje wyjście ESPHome, błędy i metadane SHA-256 obrazu do trzech plików `archive/diagnostics/normal-usb-<czas>.*`, a po zadanym czasie zamyka COM. W trakcie testu port COM6 pozostaje zajęty przez logger.

### Weryfikacja trybu DEMO w logach

1. Otwórz logi poleceniem powyżej i zostaw aktywne okno terminala.
2. Na płytce przejdź do karty USTAW i włącz DEMO.
3. Przy rozpoczęciu symulacji oczekuj wpisu `dpf_demo` z `phase=active`, a następnie `phase=passive` i `phase=cooling` oraz przykładowymi wartościami sadzy i temperatur.
4. Po około 90 sekundach powinien pojawić się wpis `phase=cooling`. Po ponownym dotknięciu DEMO pojawi się `DEMO disabled`.

Wpisy są emitowane tylko przy zmianie fazy, nie co sekundę. Potwierdzają logikę symulacji, ale nie zastępują wizualnego sprawdzenia obrazu na ekranie.

OBD-II działa w modelu żądanie–odpowiedź, więc cicha para diagnostyczna CAN nie musi sama generować ramek przed pierwszym zapytaniem. Firmware wysyła początkowe, tylko-odczytowe próby OBD także bez wcześniejszej aktywności; gdy magistrala i ECU nie odpowiedzą, ponawia je oszczędnie co 2,5 s. Po wykryciu ramek lub odpowiedzi ECU przechodzi do zwykłego tempa 250 ms. Dzięki ograniczonej kolejce TX i timeoutom brak auta nie powinien powodować szybkiej lawiny zapytań. Sam brak odpowiedzi konkretnego PID-u nie oznacza awarii całej magistrali. Logi z wcześniejszego debugowania są zachowane w `archive/diagnostics/`.

## Typowe problemy Windows / ESP-IDF

- Jeśli PowerShell blokuje skrypt, uruchom go z `-NoProfile -ExecutionPolicy Bypass` jak w przykładach powyżej. Nie zmieniaj trwale polityki systemowej.
- Windows `ERROR_ILLEGAL_DLL_RELOCATION` (623) przy uruchamianiu kompilatora może wynikać z ograniczonej izolacji procesu. W tym projekcie ten sam kompilator działa poza izolacją Codex, a pełny build zakończył się poprawnie 8 października 2026 r.; nie wymagało to napraw systemowych Windows. Gdy błąd pojawi się w Codex, uruchom kompilację w zwykłym PowerShellu albo pozwól środowisku uruchomić ją poza izolacją. [Microsoft opisuje kod 623 jako konflikt zakresu adresowego DLL.](https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes--500-999-)
- Jeśli Xtensa zgłasza konflikt `XTENSA_GNU_CONFIG` z `-dynconfig`, użyj skryptu projektu. Skrypt usuwa konfliktującą zmienną tylko w procesie builda i przywraca jej poprzednią wartość po zakończeniu.
- Błąd pobierania z GitHub lub rejestru komponentów oznacza problem z dostępem sieciowym do zależności. Zachowaj `.esphome-data` i `managed_components`; nie kasuj cache w ramach zwykłego debugowania.
- Gdy kompilator zgłosi `Failed to get path name` albo podobny problem długości ścieżki, użyj skryptu projektu, który kieruje ESP-IDF do `C:\Espressif`.

## Konfiguracje testowe

Konfiguracje dawnych testów ekranu, CAN, Wi-Fi i obrotu wyświetlacza są w `tests/`. Nie są to konfiguracje produkcyjne i nie należy ich wgrywać na stałe do auta. Lista plików i ich przeznaczenie: `tests/README.md`.

## Profil ECU i CAN

Warstwa CAN i standardowe odczyty OBD-II są aktywne. Mapa UDS DID dla ECU CJCD / `03L906018SJ` oraz bit regeneracji wymagają jeszcze potwierdzenia na samochodzie przez porównanie z OBDeleven. Szczegóły są w `DPF_PROFILES.md` i `DPF_PROFILE_OBDELEVEN_CAPTURE.md`.
## Diagnostyka CAN i odpytywania OBD

Skrypty kompilacji i wgrywania zachowują ustawienia produkcyjne jako wartości domyślne: `CanBusEnabled true`, `CanBusMode NORMAL`, `ObdPollingEnabled true`, logger USB 115200. Warianty opisane niżej służą wyłącznie do testów na stole. Po diagnozie wróć do ustawień domyślnych dla auta.

Test samego odbioru CAN (sterownik w trybie nasłuch-only, bez nadawania):

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\Wgraj-monitor-DPF.ps1' -BoardRevision V1 -CanBusEnabled true -CanBusMode LISTENONLY -ObdPollingEnabled true -LoggerBaudRate 0 -Transport USB -Device COM6
```

Test CAN w trybie normalnym, ale bez wykonywania pętli żądań OBD:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\Wgraj-monitor-DPF.ps1' -BoardRevision V1 -CanBusEnabled true -CanBusMode NORMAL -ObdPollingEnabled false -LoggerBaudRate 0 -Transport USB -Device COM6
```

Test stabilności samego UI, z wyłączonym CAN:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\Wgraj-monitor-DPF.ps1' -BoardRevision V1 -CanBusEnabled false -LoggerBaudRate 0 -Transport USB -Device COM6
```

`LISTENONLY` nie wysyła ramek CAN. `ObdPollingEnabled false` pozostawia kontroler TWAI w trybie NORMAL, ale zatrzymuje okresowe wywołanie obsługi żądań OBD. `LoggerBaudRate 0` wyłącza logowanie USB podczas testu, lecz nie wyłącza USB jako portu programowania. Dla wiarygodnego porównania utrzymaj ten sam przewód, port USB i jasność ekranu; zmieniaj tylko wskazany wariant. W każdym wgraniu odczekaj co najmniej 5 minut i zanotuj, czy obraz pozostał czysty.

Test samoodbioru TWAI, który generuje kontrolowany ruch wewnętrznie:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '.\Wgraj-monitor-DPF.ps1' -BoardRevision V1 -CanBusEnabled true -CanBusMode SELFTEST -ConfirmBenchOnly -ObdPollingEnabled false -LoggerBaudRate 115200 -Transport USB -Device COM6
```

**Przed wgraniem SELFTEST fizycznie odłącz transceiver SN65HVD230 od ESP32 i samochodu.** Skrypty kompilacji i wgrywania wymagają przełącznika `-ConfirmBenchOnly` dla tego wariantu, ale przełącznik nie wykrywa fizycznych połączeń. Tryb używa `TWAI_MODE_NO_ACK`, samoodbioru i wspólnego GPIO5 dla TX/RX; nadaje syntetyczne ramki i nie jest trybem do jazdy. Domyślnie generuje ramkę co 5 ms; parametr `-CanSelfTestInterval` pozwala wybrać 1 ms lub 5 ms. Logger raportuje liczbę odebranych ramek i błędy co 5 s. Po zakończeniu wróć do `CanBusMode NORMAL` przed podłączeniem transceivera lub auta.


Wyniki testów na stole (USB, bez transceivera):

- CAN `LISTENONLY`, logowanie USB wyłączone: czysty obraz po 5 minutach.
- CAN `NORMAL`, `ObdPollingEnabled false`, logowanie USB wyłączone: czysty obraz po 5 minutach.
- CAN `NORMAL`, `ObdPollingEnabled true`, logowanie USB wyłączone: użytkownik potwierdził czysty obraz po 5 minutach; bez transceivera test nie generował ruchu do ECU.
- 2026-10-07, `SELFTEST` 5 ms, OBD wyłączone: ok. 1 020 odebranych ramek/5 s, kolejki i liczniki błędów zerowe, pamięć stabilna; użytkownik potwierdził czysty obraz.
- 2026-10-07, `SELFTEST` 1 ms, OBD wyłączone, limit odbioru 24 ramek na przebieg: ok. 5 096 ramek/5 s (~1 019/s), `queued RX=0`, `RX missed=0`, max batch 18, max czas pętli 474 µs, brak błędów i stabilna pamięć. Po ponad 2 min użytkownik potwierdził czysty obraz i działający dotyk.
- 2026-10-07, `SELFTEST` 1 ms, poller OBD włączony: logi pokazały ok. 5 106–5 109 ramek/5 s (~1 021/s), `queued RX/TX=0`, `RX missed=0`, max batch 18, max czas pętli 390–470 µs, brak błędów CAN/TX i stabilną pamięć (heap ok. 304 kB, PSRAM ok. 8,3 MB). Użytkownik potwierdził czysty ekran i działający dotyk. SELFTEST uruchamia bramkę aktywności CAN i pozwala pollerowi wysłać żądania w trybie NO_ACK; nie ma w nim odpowiedzi ECU. Dodatkowe liczniki raportują żądania, poprawne strukturalnie ramki ISO-TP i timeouty co 10 s tylko podczas świeżej aktywności CAN.
- Weryfikacja liczników w `SELFTEST` 1 ms: przed poprawką licznik błędnie nazywał wszystkie ramki odebrane podczas oczekiwania „odpowiedziami” (w tym nieprawidłowy testowy payload `0xF0`). Po poprawce próba 10 s wykazała `requests=14`, `valid ISO-TP frames=0`, `timeouts=13`; kolejny interwał `13/0/13`. Oznacza to, że ruch loopback nie jest już mylony z poprawną ramką ISO-TP. Test osiągnął ~5 103–5 104 RX/5 s bez strat, błędów CAN, zapełniania kolejek ani zmian pamięci.
- Po przywróceniu `NORMAL` na COM6, przy fizycznie odłączonym transceiverze, logi potwierdziły `RX=0`, puste kolejki, zero błędów CAN/TX i stabilną pamięć; nie pojawiły się żądania OBD bez aktywności magistrali.
- Końcowy firmware `NORMAL` z licznikami OBD skompilowano i wgrano na COM6; ESPHome zweryfikowało zapis flash. Log startowy ponownie potwierdził stan `state=1`, `RX=0`, kolejki/błędy równe 0 oraz stałe heap/PSRAM. Logowanie USB zatrzymano po weryfikacji.
- 2026-10-07, po zmianie odbioru `0x7E8` na wielokrotnie używany bufor (bez `std::vector` allocate/free dla każdej ramki), SELFTEST 1 ms z pollerem OBD działał ponad 5 min: stale ok. 5 100 RX/5 s, `RX missed=0`, kolejki i błędy 0, heap 304260 B bez zmian, PSRAM ok. 8,30 MB (zmiana 32 B), max pętli CAN 148–162 µs (wcześniej ok. 390–455 µs). OBD: 13–14 żądań/10 s, 0 poprawnych strukturalnie ramek ISO-TP i 13–14 timeoutów, zgodnie z brakiem ECU. W trakcie testu FT3168 raportował 4–5 ponowień statusu/min, 0 nieudanych odzysków i max I²C 1 ms. Po teście przywrócono i wgrano `NORMAL`; log potwierdził `RX=0`, puste kolejki, brak błędów, heap 304276 B i PSRAM 8302028 B.
- Przy wcześniejszym limicie 8 ramek test 1 ms przetwarzał ok. 480 ramek/s i przepełniał kolejkę; limit 16 poprawiał wynik do ok. 960 ramek/s, ale pozostawiało to niewielkie straty. Obecny limit 24 usuwa straty w teście ok. 1 000 ramek/s.
- 2026-10-08, wersja V1 z poprawką odświeżania EGT w DEMO oraz większym komunikatem regeneracji została skompilowana i wgrana na COM6; ESPHome zweryfikowało hash flash. Log po restarcie przez ok. 78 s: CAN bez transceivera `RX=0`, kolejki i błędy 0, heap 302524 B, PSRAM 8294228 B bez zmian; FT3168: 4 ponowienia statusu/60 s, 0 nieudanych odzysków, 0 wolnych transakcji, max 1 ms. Logi USB nie pokazują obrazu; oczekuje na wizualne potwierdzenie wartości EGT w DEMO i rozmiaru napisu „NIE GAŚ!”.

Wynik SELFTEST potwierdza przepustowość wewnętrznego samoodbioru TWAI na stole, ale nie odtwarza elektrycznie magistrali samochodu ani nie potwierdza stabilności przy rzeczywistym ruchu CAN.

W testach z 2026-10-07 poller czekał na trzy ramki CAN przed pierwszym zapytaniem. Późniejszy test w samochodzie wykazał `RX=0` także przy pracującym silniku; przegląd kodu ujawnił, że takie oczekiwanie mogło blokować diagnostykę na cichej parze OBD. Bieżąca poprawka wprowadza powolne próby startowe bez wcześniejszej aktywności. Wymaga to ponownego sprawdzenia na samochodzie oraz osobnego krótkiego sprawdzenia stabilności na stole.
