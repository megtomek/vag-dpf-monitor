# Środowisko pracy monitora DPF

## Jedno źródło projektu i narzędzi

- Źródła, konfiguracja i dokumentacja: `C:\Users\Administrator\Documents\ChatGPT\Samochodowy Monitor DPF`.
- ESPHome: `C:\Users\Administrator\Documents\Codex\2026-10-01\c\.venv\Scripts\python.exe` (wersja sprawdzona: 2026.9.1). Skrypty projektu używają tej instalacji, jeśli `ESPHOME_PYTHON` ani lokalne `.venv` nie wskazują innej.
- ESP-IDF i Xtensa: `C:\Espressif` (ESP-IDF 5.5.5). Skrypt ustawia ścieżki narzędzi tylko dla procesu kompilacji i przywraca środowisko po zakończeniu.
- Aktywny katalog builda: `.esphome-data\build\audi-dpf-monitor`; skrypty ustawiają `ESPHOME_DATA_DIR` na `.esphome-data`, więc nie korzystają z drugiej domyślnej bazy `.esphome`.
- Dnia 11 października 2026 usunięto stary, nieużywany cache `.esphome` (ok. 649 MB / 12 364 pliki). Aktywny build i obrazy factory/OTA w `.esphome-data` pozostały zachowane. Uruchamiaj kompilację skryptem projektu, który jawnie ustawia właściwy katalog cache.
- Obrazy wydania: `firmware\V1`. Nie kopiuj buildów do osobnego katalogu ChatGPT/Codex.

27.10.2026 przeniesiono do Kosza nieużywaną kopię `.esphome-idf/` (około 2,11 GB). Aktywny CMakeCache wskazuje `C:\Espressif`, a framework, Python i kompilator tej instalacji pozostały na miejscu. Bieżący `.esphome-data/` zachowano. Skrypty nadal mają fallback `.esphome-idf` dla innej instalacji/nowego komputera — jego brak na tym komputerze jest zamierzony. Nie usuwaj aktywnego cache w ramach rutynowych porządków.

Luźne historyczne logi z katalogu głównego przeniesiono do `logs/archive/2026-10-15/`, zachowując nazwy i hashe. Mapa odtworzenia: `logs/maintenance/cleanup-2026-10-27.json`. Opis działań: [LOCAL_CLEANUP_2026-10-27.md](docs/LOCAL_CLEANUP_2026-10-27.md).

Uruchamiaj skrypty z katalogu projektu przez `powershell.exe -NoProfile -ExecutionPolicy Bypass -File ...`. `Bypass` obowiązuje tylko dla tego procesu; nie zmieniaj globalnej polityki wykonywania skryptów. `Kompiluj-monitor-DPF.ps1` waliduje YAML i kompiluje, a `Wgraj-monitor-DPF.ps1` robi obie czynności i przesyła obraz.

## Aktualizacja bez zajmowania COM6

`Wgraj-monitor-DPF.ps1` domyślnie wybiera OTA dla `audi-dpf-monitor.local`. Dzięki temu zwykłe aktualizacje nie otwierają portu USB. W razie braku mDNS wpisz aktualny adres urządzenia, np.:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Wgraj-monitor-DPF.ps1 -BoardRevision V1 -CanBusEnabled true -Transport OTA -Device 192.168.0.166
```

W trybie AP użyj `-Device 192.168.4.1`. Pierwszy flash lub odzyskiwanie firmware wymagają kompletnego obrazu przez USB:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Wgraj-monitor-DPF.ps1 -BoardRevision V1 -CanBusEnabled true -Transport USB -Device COM6
```

Jeśli USB zwraca `Access is denied` albo `port is busy`, typową przyczyną jest otwarty logger ESPHome, Arduino Serial Monitor, PuTTY lub inny terminal szeregowy. Zamknij jego okno/połączenie, po czym ponów upload. Nie kończ procesów systemowych na ślepo. W tej sesji Windows udostępnił COM6 jako `USB\VID_303A&PID_1001&MI_00`, ale sandbox odmówił zapytań CIM/listowania procesów, więc właściciela otwartego uchwytu nie dało się ustalić z Codex.

## Diagnostyka ograniczeń Windows/Codex

Podczas bieżącej kontroli wystąpiły dwa różne ograniczenia środowiska:

- Kompilator Xtensa uruchamiany z sandboxa zwrócił błąd relokacji systemowej DLL (Windows error 623). Ta sama konfiguracja została uruchomiona poza sandboxem i skompilowała się poprawnie. Nie reinstaluj ESP-IDF z powodu tego komunikatu.
- Klient HTTP uruchomiony w sandboxie nie mógł połączyć się z lokalnym adresem monitora (`WinError 10013`, zabronione gniazdo). To ograniczenie dostępu sieciowego procesu Codex, a nie dowód awarii urządzenia.
- Narzędzie pulpitu widziało tylko okno ChatGPT; Chrome nie zwrócił dostępnego okna. To oznacza, że Codex nie może w tej chwili przejąć istniejącej karty, choć użytkownik może normalnie korzystać z Chrome.
- Odczyt listy procesów Windows był niedostępny (`Get-CimInstance` — odmowa CIM; `tasklist` w kontekście sandboxa — błąd danych logowania). Niczego nie zakończono i nie zmieniano ustawień zabezpieczeń.

Jeśli potrzebny jest log z urządzenia, użyj lokalnie `Czytaj-logi-DPF.ps1` albo otwórz panel w zwykłym Chrome. Jeżeli wyłącznie Codex dostaje `WinError 10013`, firmware nie wymaga z tego powodu zmiany. Nie przyznawaj Codexowi szerszych uprawnień Windows ani nie wyłączaj zapory/antywirusa, aby obejść ograniczenie.

## Zapis testu z samochodu

Połącz laptop z AP monitora albo wspólną siecią. Zapisz 10 minut logów:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Czytaj-logi-DPF.ps1 -Device 192.168.4.1 -Seconds 600 -OutputFile logs\test-auto-01.txt
```

Dla routera/hotspotu zmień `-Device` na bieżący IP. Przy kolejnym teście zmień nazwę pliku: istniejący log nie zostanie nadpisany. Ścieżki względne wskazują katalog projektu. Log jest zapisywany jako UTF-8 z czasem lokalnym i strefą czasową; zawiera odebrane komunikaty, nie hasła z secrets. Czas oznacza odbiór przez laptop, nie dokładny moment pomiaru ECU. Utrata połączenia kończy zapis błędem; wcześniejsze wiersze pozostają w pliku. Katalog `logs` jest pomijany przez Git.

Do porównania zanotuj czas, stan silnika oraz nazwę i jednostkę parametru OBDeleven. Szczególnie potrzebne są CALID, dystans 1156, odpowiadające kanały temperatur oraz status PID 0x8B podczas rzeczywistej regeneracji. Samo DEMO nie potwierdza alarmu z ECU.

### Wynik wcześniejszych zmian sieciowych

Poprawka uruchamiania sieci ustawia `wifi.enable_on_boot: false` i przywraca wybrany tryb po setup komponentów. OFF/AP nie wykonują początkowej próby połączenia z routerem. Lokalny komponent `dpf_network` przekazuje konfigurację z sekretów do helpera przed startem radia; brak wyniku skanowania nie usuwa konfiguracji potrzebnej do powrotu AP/OFF → router. Użytkownik zgłosił, że OFF nie przetrwał RESET mimo testu po 70 sekundach. Dodano osobny stały klucz NVS i natychmiastowy zapis/sync przy zmianie trybu z kafelka lub panelu WWW. Logi dpf_wifi pokazują wynik zapisu oraz odczytu po starcie. Użytkownik potwierdził poprawny test OFF po RESET i powrót do Wi-Fi po wgraniu poprawki 0x80f9dbbe. Przyczyna poprzedniego nieudanego zapisu nie została jednoznacznie ustalona.

Konfiguracja trzech trybów sieci (router Wi-Fi, tylko AP, radio wyłączone) przeszła walidację i kompilację 11 października 2026: ESPHome 2026.9.1, ESP-IDF 5.5.5, config hash `0xfb1092ed`, obraz aplikacji 1 273 363 B, DIRAM 119 123 B (34,86%). Wgrano ją OTA na `192.168.0.166`; po restarcie `/events` zwróciło HTTP 200, uptime wynosił 16 s. Użytkownik potwierdził przejście kafelkiem po wszystkich trzech trybach i działanie przełączania. Zachowanie wybranego trybu po restarcie nadal wymaga osobnej próby.
