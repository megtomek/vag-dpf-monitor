# VAG DPF Monitor

[English](README.en.md) · [Instalacja](BUILD_INSTRUCTIONS.md) · [Sprzęt](HARDWARE_REFERENCE.md) · [Pomoc](SUPPORT.md)

Samodzielny monitor filtra cząstek stałych do eksperymentów z diagnostyką VAG. ESP32-S3 odczytuje ECU przez OBD-II/CAN, pokazuje dane na dotykowym AMOLED i zapisuje zdarzenia do analizy. Działa bez Home Assistant; lokalny panel WWW jest dostępny przez Wi-Fi lub własny AP.

**Wersja rozwojowa dla jednego zestawu sprzętu i ECU.** Komunikat „REGENERACJA DPF / NIE GAŚ!” jest **szacowany z EGT3**. Nie mamy potwierdzonej flagi aktywnej regeneracji ECU CJCD. Trzy przeanalizowane zestawy naturalnych zdarzeń wspierają działanie estymatora, ale nie ustalają częstości fałszywych alarmów ani pominięć.

## Możliwości

- Pięć kart: **DPF, TEMP, FILTR, HIST, USTAW**, ekran poziomy 456 × 280 px.
- Sadza obliczona/zmierzona, różnica ciśnień, EGT3, dystans od regeneracji i napięcie raportowane przez ECU. [Tabela parametrów](PARAMETER_REFERENCE.md) opisuje zakres potwierdzenia.
- Pasek sadzy **0–24 g**; to skala prezentacji, nie próg sterownika.
- Szacowany alarm: EGT3 ≥550°C przez 30 s; wygaszenie po <450°C przez 60 s. Utrata świeżych danych oznacza brak oceny.
- Dwa trwałe zdarzenia w NVS; około 10 minut przejazdu i logów w RAM; pobieranie CSV/TXT przez panel.
- Zapamiętywane tryby router Wi-Fi → własny AP → radio wyłączone. Odczyty i zapis nie wymagają Wi-Fi.
- Wybór profilu, jasność, DEMO i natywne OTA ESPHome po pierwszym wgraniu USB.

DEMO jest symulacją. Schłodzenie filtra, spadek sadzy i reset dystansu są obserwacjami; nie potwierdzają flagą ECU skutecznego zakończenia regeneracji. Monitor nie uruchamia wypalania serwisowego i nie zastępuje wskazań pojazdu ani diagnostyki warsztatowej. Obsługuj ekran i pobieraj dane podczas postoju.

## Zgodność

| Element | Zakres sprawdzenia |
|---|---|
| Płytka | Waveshare **ESP32-S3-Touch-AMOLED-1.64**, Flash 16 MB / PSRAM 8 MB, CO5300, FT3168 |
| Rewizja | Roboczo PCB V1, działający LCD_CS GPIO9; skrypty obsługują V1. Krzem ESP32-S3 v0.2 nie określa PCB |
| CAN | Zewnętrzny SN65HVD230 3,3 V, GPIO5 TX / GPIO6 RX, 500 kbit/s, 7E0 → 7E8 |
| Profil częściowy | Audi **CJCD 2.0 TDI**, SW **03L906018SJ / 9977**, HW **03L906018JL / H25**, ODX **EV_ECM20TDI01103L906018SJ / 003003** |
| Auto / standard OBD-II | Zależne od odpowiedzi ECU; nie jest uniwersalnym profilem DPF |
| Inne ECU, SW i płytki | Niepotwierdzone; sam kod silnika nie wystarcza do przeniesienia DID |

## Pierwsze uruchomienie

1. Sprawdź zgodność i [połączenia sprzętu](HARDWARE_REFERENCE.md). Potrzebujesz transceivera CAN i odpowiedniego samochodowego zasilania 5 V; nie podawaj 12 V na płytkę/GPIO.
2. Pobierz źródła wybranego tagu lub `main`. Skopiuj `secrets.example.yaml` do `secrets.yaml` i wpisz własne dane. Nie publikuj tego pliku ani obrazu z hasłami.
3. Przygotuj środowisko i wgraj przez USB według [instrukcji instalacji](BUILD_INSTRUCTIONS.md). Aktywna konfiguracja to `audi-dpf-monitor.yaml`.
4. Na postoju wybierz profil zgodny z ECU/CALID. Do auta podłączaj poprawnie okablowany monitor z firmware NORMAL.
5. Wykonaj [odbiór urządzenia](docs/USER_GUIDE.md#odbiór-po-pierwszym-uruchomieniu) i porównaj dane z narzędziem diagnostycznym.

Nie udostępniamy uniwersalnego BIN. Obrazy kompilowane lokalnie zawierają sekrety. Wydanie źródeł, kompilacja i obraz wgrany na urządzenie są dokumentowane osobno.

## Dokumentacja

| Potrzeba | Dokument |
|---|---|
| Lista części i okablowanie | [Sprzęt](HARDWARE_REFERENCE.md), [schemat SVG](schemat-polaczen-dpf.svg) |
| Środowisko, USB, OTA | [Instalacja](BUILD_INSTRUCTIONS.md) |
| Obsługa, logi i problemy | [Instrukcja użytkownika](docs/USER_GUIDE.md), [pomoc](SUPPORT.md) |
| Zgodność i dekodowanie | [Profile](DPF_PROFILES.md), [DID/PID](PARAMETER_REFERENCE.md) |
| Rejestracja i rozwój | [Zdarzenia](AUTOMATIC_DPF_EVENTS.md), [plan](DEVELOPMENT_ROADMAP.md) |
| Wyniki i wydania | [Stan](PROJECT_STATUS.md), [wydania](RELEASES.md), [changelog](CHANGELOG.md) |
| Współpraca | [CONTRIBUTING](CONTRIBUTING.md), [zasady społeczności](CODE_OF_CONDUCT.md), [SECURITY](SECURITY.md) |
| Licencje i publikacja | [Pochodzenie zasobów](THIRD_PARTY_NOTICES.md), [checklista](docs/PUBLICATION_CHECKLIST.md) |
| Cała dokumentacja | [Indeks](docs/INDEX.md) |

`archive/`, `tests/` i `docs/history/` zachowują wcześniejsze eksperymenty. Nie są instrukcją aktualnego firmware. PR #17 i #18 pozostają osobnymi gałęziami rozwojowymi; podsumowanie po dotknięciu HIST z PR #18 nie jest jeszcze funkcją `main`.

## Jak pomóc

Przydatne są odtwarzalne zgłoszenia, porównania z dokładną identyfikacją ECU, dokumentacja budowy i pomiary naturalnych zdarzeń. Zacznij od [CONTRIBUTING](CONTRIBUTING.md) i [Issues](https://github.com/megtomek/vag-dpf-monitor/issues). Nie publikuj VIN, haseł, lokalizacji ani nieprzejrzanych logów. Nie wywołuj regeneracji serwisowej dla projektu.

## Licencja i autorstwo

Własny kod i dokumentacja projektu są udostępniane na **GNU GPLv3, wyłącznie wersja 3** — `SPDX-License-Identifier: GPL-3.0-only`. Pełny tekst: [LICENSE](LICENSE); zakres i informacja o prawach: [COPYRIGHT](COPYRIGHT).

Licencja pozwala używać, modyfikować i sprzedawać projekt. Rozpowszechnianie objętego nią firmware i wersji pochodnych wymaga spełnienia GPLv3, w tym udostępnienia odpowiednich źródeł. Projekt jest dostarczany bez gwarancji w zakresie określonym licencją. Zasoby zewnętrzne zachowują własne warunki, w tym MIT upstream panelu, BSD-3-Clause bibliotek Lit i Ubuntu Font Licence fontów. [Pochodzenie i pozostały przegląd](THIRD_PARTY_NOTICES.md).

Niezależny projekt społecznościowy, niepowiązany z Volkswagen Group, Audi, Waveshare ani OBDeleven. Nazwy produktów służą identyfikacji zgodności.
