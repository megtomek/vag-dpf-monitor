# Bieżący stan projektu

Aktualizacja dokumentacji: 21.11.2026; ostatnie opisane wdrożenie: 03.11.2026. Ten plik opisuje stan obecny; [poprzedni dziennik](docs/history/PROJECT_STATUS-before-cleanup-2026-10-28.md) jest archiwum. Prywatne repozytorium: [megtomek/vag-dpf-monitor](https://github.com/megtomek/vag-dpf-monitor).

## Źródła i rzeczywiste wdrożenie

| Obszar | Stan |
|---|---|
| Ostatnia wersja funkcjonalna | **v0.1.3**, prerelease; bieżące main może zawierać późniejsze poprawki dokumentacji |
| Ostatni udokumentowany firmware urządzenia | NORMAL V1, CAN/OBD, v0.1.3 + draft PR #17/#18, źródła ac1923b, OTA 03.11.2026; nie jest to bieżący main |
| Identyfikacja obrazu z 03.11 | OTA 1310768 B; SHA256 291471e20594df2db014301092c38241c6d8fa7c998c9666b70c6b630242c01a |
| Dowody | [PR #18 i raport w jego gałęzi](https://github.com/megtomek/vag-dpf-monitor/pull/18), [wcześniejszy rejestrator](docs/REGEN_EVIDENCE_2026-10-28.md) |
| Obserwacja | 03.11 WWW odtworzyło zapis nr 4; CSV po OTA identyczny; krótki log CAN bez błędów. Ocena nowej strony HIST odłożona. Brak bieżącej obserwacji sprzętu 21.11 |
| Źródła rozwojowe | Draft #17 (opcjonalne sensory) i #18 (podsumowanie) pozostają niescalone; stan sprawdzony w GitHub 21.11 |

Urządzenie: Waveshare ESP32-S3-Touch-AMOLED-1.64, ekran poziomy 456×280. PCB V1 to założenie robocze; ESP32-S3 v0.2 oznacza rewizję krzemu. ECU: CJCD, SW 03L906018SJ/9977, HW 03L906018JL/H25, ODX EV_ECM20TDI01103L906018SJ/003003.

## Co działa i na jakiej podstawie

- Odczyty sadzy, różnicy ciśnień, EGT3 i dystansu oraz napięcia zasilania ECU. Dokładne skale, warunki i granice potwierdzenia: [wspólna tabela parametrów](PARAMETER_REFERENCE.md).
- Pięć kart DPF/TEMP/FILTR/HIST/USTAW; skala sadzy 0–24 g; napięcie samą wartością, 36 px. Ekran i dotyk potwierdzone przez użytkownika.
- HIST pokazuje do dwóch zachowanych zdarzeń szacowanych, czas całego zapisu i powód zakończenia. Nie jest licznikiem wszystkich skutecznych regeneracji.
- Panel WWW/AP, trzy zapamiętywane tryby sieci, pobieranie logów, przejazdu i zdarzenia. Brak Wi-Fi nie zatrzymuje monitorowania.
- Rejestrator: 5 s/próbkę, do 3 min prehistorii, dwa sloty NVS, do 900 próbek/zdarzenie. Bufor zwykłej jazdy: 120 próbek RAM. Osobny syntetyczny TEST przetrwał wcześniejsze odłączenie zasilania.
- v0.1.3 dodaje dystans, obserwacyjny trend sadzy z 2–3 min oraz rzeczywiste przełączenia alarmu. Po OTA potwierdzono nowe pola i zachowanie starego CSV identycznie bajt w bajt. 29.10 potwierdzono niepusty trend i jego obliczenia oraz rozszerzony checkpoint NVS podczas drugiego zdarzenia. 02.11 potwierdzono finalny zapis reason=2 oraz odczyt nowego dodatku po fizycznym odłączeniu i ponownym podłączeniu zasilania (zdarzenie nr 4). Nie sprawdzono zaniku zasilania w trakcie flashowania.

## Główna metoda wykrywania — szacowana

**EGT3 ≥550°C przez 30 s** włącza alarm; **EGT3 <450°C przez 60 s** go wygasza. Utrata świeżych danych przerywa ocenę. Trend sadzy jest wyłącznie zapisywany, nie steruje alarmem. Schłodzenie i brak danych nie potwierdzają zakończenia regeneracji ECU. DEMO jest oddzielone od rzeczywistych pomiarów i zapisów.

Pierwsze naturalne zdarzenie: 306 próbek / 25 min 27 s; maksimum EGT3 638,7°C; sadza obliczona 21,01 → 6,73 g, zmierzona 19,18 → 5,85 g. Użytkownik potwierdził alarm i jego zanik. To jedna udana obserwacja, nie pełna walidacja skuteczności. Wszystkie 115B/12E7/134F/11C1 pozostały zerowe — nie są obecnie użyteczną flagą aktywnego wypalania. [Raport pomiaru](FIRST_REAL_EVENT_2026-10-22.md).

Drugie zdarzenie otrzymane 29.10: EGT3 maks. 642,5°C; sadza 21,03 → 6,96 g i 21,17 → 8,22 g. Alarm ON/OFF zapisany, 18 min 25,908 s, użytkownik potwierdził komunikat i jego zanik. Cztery statusy nadal zerowe. Pliki pobrano około 20 s przed oczekiwanym zamknięciem rejestratora, dlatego reason=1 nie dowodzi usterki. [Raport drugiego zdarzenia](docs/SECOND_REAL_EVENT_2026-10-29.md).

Zestaw otrzymany 02.11: zamknięte zdarzenie nr 4, 180 pełnych próbek, EGT3 maks. 649,8°C, sadza 21,06 → 5,06 g i 21,45 → 0,96 g. Alarm 9 min 41,600 s; reset dystansu 232,699 → 0 km przy jeszcze gorącym EGT3. Użytkownik potwierdził pobranie po fizycznym odłączeniu zasilania — naturalny rozszerzony zapis przetrwał. Cztery statusy nadal zerowe. [Raport](docs/COMPLETED_EVENT_2026-11-02.md).

Przygotowanie publikacji: [checklista](docs/PUBLICATION_CHECKLIST.md). Repozytorium nadal prywatne, licencja własnego kodu i dokumentacji: GPL-3.0-only, wybrana przez właściciela 21.11.2026. [Przegląd pochodzenia i historii](docs/PROVENANCE_REVIEW_2026-11-21.md): brak znalezisk sekretów w opisanym zakresie; pozostają źródła bazy panelu/FT3168, noty ZIP i decyzja o metadanych autora/urządzenia. Obecna zmiana obejmuje dokumentację i komentarz licencyjny JS, bez zmiany zachowania firmware.

## Następne działania

| Praca | Co jest potrzebne | Zadanie |
|---|---|---|
| Pełne okno trendu | Potwierdzone na danych drugiego zdarzenia; pozostaje ocena luk i zwykłej jazdy | [#1](https://github.com/megtomek/vag-dpf-monitor/issues/1) |
| Końcowy zapis i trwałość | Potwierdzone reason=2 i odczyt po odłączeniu zasilania; nadal nie sprawdzono zaniku podczas zapisu | [#1](https://github.com/megtomek/vag-dpf-monitor/issues/1) |
| Fałszywe alarmy / pominięcia | Zwykłe przejazdy, także przy większym obciążeniu; bez zmiany progów po pojedynczym przypadku | [#1](https://github.com/megtomek/vag-dpf-monitor/issues/1) |
| Rzeczywisty status ECU | Opis zgodny z ECU albo przechwycenie konkretnego kanału i zmian stanów; bez losowego skanowania DID | [#2](https://github.com/megtomek/vag-dpf-monitor/issues/2) |
| Instalacja i dłuższa praca | Dokumentacja zasilania, zachowania przy rozruchu i utracie komunikacji | [#4](https://github.com/megtomek/vag-dpf-monitor/issues/4) |
| Dalsze porównania parametrów | Dystans w ruchu, ciśnienie/sadza w różnych warunkach; patrz plan | [#13](https://github.com/megtomek/vag-dpf-monitor/issues/13) |

Nie wywołujemy regeneracji serwisowej dla zebrania danych. Nie zmieniamy progów/pinów na podstawie ogólnego opisu z sieci. Cel całego projektu pozostaje otwarty.

## Nawigacja

[Parametry](PARAMETER_REFERENCE.md) · [Plan i otwarte prace](DEVELOPMENT_ROADMAP.md) · [Wydania](RELEASES.md) · [Changelog](CHANGELOG.md) · [Mapa dokumentacji](docs/INDEX.md) · [Zasady pracy](CONTRIBUTING.md).

Surowe logi, sekrety i obrazy firmware pozostają lokalnie. Raporty wdrożeń rozróżniają kompilację, wgranie i rzeczywistą obserwację urządzenia.


Aktualizacja po zgodzie właściciela: [wynik oczyszczenia historii i ocena gotowości](docs/PRIVACY_CLEANUP_2026-11-21.md). Gałęzie/tagi oczyszczono, ale stare referencje PR nadal blokują uznanie całego repozytorium za anonimowe.
