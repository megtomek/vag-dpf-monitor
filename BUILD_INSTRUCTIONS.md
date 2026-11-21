# Instalacja i kompilacja

[README](README.md) · [Sprzęt](HARDWARE_REFERENCE.md) · [Obsługa](docs/USER_GUIDE.md)

## Zakres i wymagania

Aktywna konfiguracja: `audi-dpf-monitor.yaml`. Zachowaj cały checkout, w tym nagłówki, `components/`, `fonts/` i zasoby panelu. Nie kompiluj archiwów zamiast głównego YAML.

Sprawdzone środowisko autora: Windows, ESPHome **2026.9.1**, ESP-IDF **5.5.5**, sprzęt V1. Poniższa procedura od zera nie została jeszcze odtworzona na niezależnym czystym komputerze. Inne systemy i wersje nie mają tu potwierdzenia.

## 1. Źródła i hasła

Zainstaluj Git oraz Python 3.12 i otwórz PowerShell w krótkim katalogu roboczym:

```powershell
git clone https://github.com/megtomek/vag-dpf-monitor.git
Set-Location vag-dpf-monitor
py -3.12 -m venv .venv
& .\.venv\Scripts\python.exe -m pip install esphome==2026.9.1
Copy-Item secrets.example.yaml secrets.yaml
notepad secrets.yaml
```

Dopóki repozytorium jest prywatne, klonowanie wymaga dostępu. Uzupełnij SSID/hasło Wi-Fi, hasło AP (co najmniej 8 znaków), login/hasło WWW i hasło OTA. Użyj własnych wartości. Nie udostępniaj `secrets.yaml` ani BIN z lokalnymi hasłami.

## 2. Pierwsze przygotowanie ESP-IDF — bez flashowania

Skrypt kompilacji wymaga już zainstalowanych narzędzi IDF. Sama instalacja pakietu Python nie wystarcza. Pozwól ESPHome pobrać zarządzany framework i toolchain; potrzebne są internet i miejsce na dysku:

```powershell
$env:ESPHOME_DATA_DIR = Join-Path $PWD '.esphome-data'
$env:ESPHOME_ESP_IDF_PREFIX = Join-Path $PWD '.esphome-idf'
& .\.venv\Scripts\python.exe -m esphome --substitution can_bus_enabled false compile audi-dpf-monitor.yaml
```

To kompilacja na stół bez CAN, bez wgrywania. Odczytaj wersję IDF z wyniku; bazą jest 5.5.5. Gdy narzędzia wybiorą inną wersję lub konfiguracja nie przejdzie, zgłoś wynik zamiast instalować przypadkowe wersje. Nie publikuj rozwiniętego YAML z polecenia `config`, bo może ujawnić ustawienia.

Skrypt korzysta z `ESPHOME_PYTHON` lub projektowej `.venv`. Narzędzia wybiera przez `ESPRESSIF_TOOLS_PATH`, `ESPHOME_ESP_IDF_PREFIX`, cache CMake, `C:\Espressif` lub `.esphome-idf`. Nie potrzebujesz ścieżek z komputera autora. Zachowaj działający cache podczas diagnozy.

## 3. Pierwsze USB na stole

Odłącz OBD. Sprawdź PCB i [okablowanie](HARDWARE_REFERENCE.md); skrypty obsługują V1 z CS GPIO9. Użyj przewodu USB danych, zamknij programy korzystające z portu:

```powershell
Get-CimInstance Win32_SerialPort | Select-Object DeviceID, Name
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Wgraj-monitor-DPF.ps1 -BoardRevision V1 -CanBusEnabled false -Transport USB -Device COM6
```

Zastąp COM6 swoim portem. Skrypt kompiluje i wybiera kompletny `firmware.factory.bin` dla USB. **Nie wgrywaj `firmware.ota.bin` pod offset 0 przez USB** — nie zawiera bootloadera i tablicy partycji. W razie problemu z wykrywaniem USB sprawdź przewód/port, a tryb bootloadera uruchamiaj według [Waveshare](https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-1.64).

Potwierdź czysty obraz i dotyk. Brak danych ECU na stole jest normalny; DEMO jest symulacją.

## 4. Wariant do auta

Po sprawdzeniu zasilania i transceivera wgraj NORMAL z odczytami, nadal przed podłączeniem do auta:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Wgraj-monitor-DPF.ps1 -BoardRevision V1 -CanBusEnabled true -CanBusMode NORMAL -ObdPollingEnabled true -Transport USB -Device COM6
```

Dobierz profil po identyfikacji ECU. Przejdź [odbiór urządzenia](docs/USER_GUIDE.md#odbiór-po-pierwszym-uruchomieniu). Upload nie potwierdza odczytu samochodu.

## 5. Aktualizacje OTA

Ustal bieżący IP i brak aktywnego zdarzenia/wypalania. Pobierz logi i przejazd RAM przed restartem; zdarzenie CSV także zachowaj jako kopię. Włącz radio. Przykład dla AP:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Wgraj-monitor-DPF.ps1 -BoardRevision V1 -CanBusEnabled true -Transport OTA -Device 192.168.4.1
```

W trybie routera użyj aktualnego IP. Adres z raportu historycznego nie jest stałym ustawieniem. Skrypt używa natywnego OTA ESPHome i lokalnego hasła; upload przez formularz WWW jest wyłączony. Po aktualizacji sprawdź obraz, dotyk, dane ECU i zdarzenie.

## Logi i problemy

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Czytaj-logi-DPF.ps1 -Device 192.168.4.1 -Seconds 30
```

Skrypt używa HTTP Digest i lokalnych danych. ESPHome CLI `logs` przez WWW w tej wersji może zwrócić 401 przez różnicę Basic/Digest. Dostępne są też logi TXT w panelu. Logger USB zajmuje port; zamknij go przed uploadem. Nie kończ wszystkich procesów Python/Chrome — ustal właściciela konkretnego portu.

Przy błędzie zapisz wersje i komunikat. Nie reinstaluj IDF ani nie usuwaj cache bez diagnozy. [SUPPORT](SUPPORT.md) opisuje potrzebne dane. Linux/macOS i chmura nie są zweryfikowaną ścieżką tego projektu.

## Wyniki i diagnostyka

Build: `.esphome-data/build/audi-dpf-monitor/build/`; kopie: `firmware/V1/`. BIN/ELF mogą zawierać hasła i nie trafiają do Git/Releases.

`-PassiveCapture` wybiera osobny nasłuch — [CAN_CAPTURE](CAN_CAPTURE.md). SELFTEST nadaje ramki syntetyczne i wymaga `-ConfirmBenchOnly` oraz fizycznie odłączonego transceivera i auta. Nie jest częścią zwykłego uruchomienia.

Datowane komendy i wyniki zachowano w [archiwum instrukcji](docs/history/BUILD_INSTRUCTIONS-before-public-docs-2026-11-21.md); to materiały dowodowe, nie bieżący przewodnik.
