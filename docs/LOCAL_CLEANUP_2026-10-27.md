# Porządkowanie plików lokalnych — 27.10.2026

Zakres: wyłącznie katalog projektu Samochodowy Monitor DPF. Operację zlecił użytkownik. Nie opróżniano Kosza ani nie usuwano plików poza projektem.

## Wykonane działania

| Obszar | Działanie |
|---|---|
| 30 luźnych eksportów CSV/TXT z katalogu głównego | Przeniesiono do `logs/archive/2026-10-15/`; każdą kopię sprawdzono SHA256 przed i po przeniesieniu |
| `.esphome-idf/` | Nieużywana lokalna kopia ESP-IDF wysłana do Kosza; bieżący CMakeCache wskazuje `C:\Espressif` |
| Pięć kopii `*.pre-passive` | Identyczne bajtowo pliki z archiwum wysłano do Kosza; zachowano odpowiedniki bez tego przyrostka |
| Cache wewnątrz archiwalnego snapshotu z 3.10 | Wygenerowany `.esphome-data/` wysłany do Kosza; historyczne źródła pozostały |
| Osiem katalogów `__pycache__` | Zbędny, odtwarzalny bytecode wysłano do Kosza |

Łącznie 15 pozycji wysłanych do Kosza, obejmujących 2 116 277 982 B (około 2,12 GB / 1,97 GiB). To zmniejszenie katalogu projektu, **nie deklaracja odzyskania wolnego miejsca na woluminie** — pliki pozostają w Koszu.

Liczba plików bezpośrednio w katalogu głównym spadła z 83 do 53. Nie przenoszono aktywnych źródeł ani nie zmieniano ścieżek w konfiguracji kompilacji.

## Zachowane materiały

- `.git/`, historia i wydania GitHub.
- Bieżący `.esphome-data/` (około 2,42 GB) — aktywna kompilacja i cache zależności; pozostawiono, aby nie wydłużać kolejnych buildów.
- Systemowe `C:\Espressif`, aktywne narzędzia Python i GitHub CLI.
- `secrets.yaml`, fonty, źródła, schemat, konfiguracje.
- Wszystkie unikatowe pomiary i załączniki. Identyczne pliki TEST z różnych etapów próby pozostawiono jako zapis pochodzenia pomiarów, a nie traktowano jako niepotrzebne dane.
- Unikatowe warianty firmware, także pasywny rejestrator, i znaczące snapshoty historyczne.

## Duplikaty archiwalne

Katalog: `archive/legacy-snapshots/can-passive-activity-backups-20261006/`.

| Usunięta kopia | Zachowany odpowiednik | SHA256 obu przed operacją |
|---|---|---|
| `audi-dpf-monitor.yaml.pre-passive` | `audi-dpf-monitor.yaml` | `d6f050e9fc583829934adc606e0fbdea07f5bb9df163b21c9988a1b0cd3991cf` |
| `components_esp32_can_recover_canbus.py.pre-passive` | `components_esp32_can_recover_canbus.py` | `471c570fe9afe998fb7c042986be2e3a2455b53e16954d50c5a719e0e7e85375` |
| `components_esp32_can_recover_esp32_can_recover.cpp.pre-passive` | `components_esp32_can_recover_esp32_can_recover.cpp` | `7599a932010554fce2397eabeea00951932b27442f459402b6a71118aab4b0cf` |
| `components_esp32_can_recover_esp32_can_recover.h.pre-passive` | `components_esp32_can_recover_esp32_can_recover.h` | `d26bf6baff81b328a46d0c9280ea2ae30e2dae4c4183e4b5a604cce3af0a8c1d` |
| `uds_audi.h.pre-passive` | `uds_audi.h` | `58fdce58f01b11bccfefb3ba6fd75e2f832e862ad518547e2714de7f764f9652` |

## Odtworzenie i odnajdywanie plików

Lokalne mapy (ignorowane przez Git):

- `logs/maintenance/cleanup-plan-2026-10-27.json` — plan z oryginalnymi ścieżkami, rozmiarami i hashami przenoszonych plików;
- `logs/maintenance/cleanup-2026-10-27.json` — wykonane operacje.

Log wymieniony w starszym raporcie samą nazwą pliku znajduje się teraz pod tą samą nazwą w `logs/archive/2026-10-15/`. Pierwsze naturalne zdarzenie pozostaje w `archive/diagnostics/first-event-2026-10-22/`. Nie przenoszono plików z katalogu Pobrane.

Przeniesione logi można zwrócić do pierwotnych ścieżek na podstawie mapy, po sprawdzeniu braku kolizji. Usunięte foldery można przywrócić z Kosza. Kopie `.pre-passive` można dodatkowo odtworzyć z zachowanych identycznych odpowiedników lub historii Git sprzed tego porządku.

## Weryfikacja

Po operacji: hashe wszystkich 55 chronionych plików (źródła, skrypty, fonty, konfiguracje i lokalne sekrety) bez zmian; 30 przeniesień zweryfikowanych; żaden z 15 usuwanych celów nie pozostaje pod starą ścieżką. Sprawdzono istnienie systemowego frameworka, Pythona i kompilatora wskazanych przez bieżący build. Nie wykonywano testów firmware, kompilacji ani wgrywania.
