# Opisy wydań przed ujednoliceniem

Kopia opisów GitHub Releases z 28.10.2026. To archiwum, nie bieżący stan urządzenia. Tagi i obrazy nie są zmieniane.

## v0.1.0

# Baza użytkowa CJCD — wersja rozwojowa

Pierwsze nazwane wydanie źródeł samodzielnego monitora DPF. Punkt odniesienia do dalszych zmian i odtworzenia dotychczasowej pracy.

## Zakres

- Waveshare ESP32-S3-Touch-AMOLED-1.64, roboczo PCB V1, ekran poziomy 456×280.
- CJCD: SW 03L906018SJ/9977, ODX 003003; CAN/OBD/UDS i częściowo potwierdzone odczyty.
- EGT3 DID 11B2 i podstawowy alarm szacowany: >=550°C/30 s, wyjście <450°C/60 s.
- Pięć kart, lokalny panel WWW/AP, trzy zapamiętywane tryby sieci, OTA.
- Zdarzenia NVS, około 10 minut odczytów w RAM, logi oraz oddzielny TEST pamięci.
- Aktualna dokumentacja, plan w Issues, zasady pracy, historia i szablony zgłoszeń.

## Co sprawdzono wcześniej

Pierwszy naturalny zapis otrzymany 22.10: 306 próbek, EGT3 maks. 638,7°C, sadza obliczona 21,01 → 6,73 g, zmierzona 19,18 → 5,85 g. Użytkownik potwierdził pojawienie się i zniknięcie komunikatu. TEST NVS przetrwał fizyczne odłączenie zasilania; bufor 120 próbek i jego nadpisywanie sprawdzono na urządzeniu.

## Ograniczenia

To prerelease: alarm jest szacowany, nie potwierdzony zdekodowaną flagą ECU. 115B/12E7/134F/11C1 pozostały zerowe przez całe pierwsze zdarzenie. Nie potwierdzono ogólnej skuteczności detekcji ani innych ECU. Dokładny zakres dowodów i dalsze prace: PROJECT_STATUS.md, FIRST_REAL_EVENT_2026-10-22.md, DEVELOPMENT_ROADMAP.md.

To wydanie źródeł, nie nowe wgranie na monitor. Ostatnia znana instalacja z 15.10 pozostaje na urządzeniu; zmiana organizacyjna nie zmienia firmware. Nie wykonywano nowych testów, kompilacji ani flashowania podczas porządkowania dokumentacji.

Brak załączników BIN/ELF: mogą zawierać lokalne hasła. Aby zbudować obraz, utwórz własny secrets.yaml z secrets.example.yaml i postępuj według BUILD_INSTRUCTIONS.md. Surowe lokalne logi nie są częścią kopii źródeł.

Porządki scalono w PR #5; dalsze zadania #1–#4.

## v0.1.1

## Pasek sadzy 0–24 g

- 12 g = połowa paska, 21 g = 87,5%, 24 g i więcej = pełny pasek. Etykieta liczbowa zachowuje odczyt w gramach.
- Kod w PR #8. Kompilacja i OTA zakończone powodzeniem 28.10.2026; użytkownik potwierdził poprawną kartę DPF, działający dotyk i wypełnienie przy około 19 g.
- Po restarcie odbiór ECU działał, liczniki CAN w obserwowanym fragmencie zerowe, zachowane zdarzenie identyczne.
- Raport: docs/DEPLOYMENT_2026-10-28_SOOT_BAR.md, z SHA256 i identyfikacją obrazu.
- Zawiera też dokumentację porządków lokalnych i usunięcie pięciu archiwalnych duplikatów z PR #7.

Nie zmieniono progów alarmu ani dekodowania ECU. Nadal prerelease: alarm regeneracji jest szacowany z EGT3. Draft PR #6 nie jest częścią wydania. Brak załączników BIN/ELF zawierających lokalne sekrety.

## v0.1.2

Poprawiono pustą kartę HIST w trybie szacowanym. Pokazuje do dwóch zachowanych zdarzeń, czas ostatniego zapisu i powód zakończenia. Czas obejmuje rozgrzewanie i schłodzenie; dane nie są statusem potwierdzonym ECU.

Kompilacja i OTA zakończone powodzeniem. Odzyskano starsze zdarzenie (306 próbek, 1527 s), CSV pozostał identyczny. Użytkownik potwierdził dane, czytelny układ i dotyk. Raport: docs/DEPLOYMENT_2026-10-28_HISTORY.md.

Wydanie zawiera skalę sadzy 0–24 g z v0.1.1. PR #6 pozostaje osobno. Brak automatycznych testów; nie potwierdzono pełnej skuteczności estymatora ani bitów statusu ECU. Obrazy firmware i sekrety nie są publikowane.

## v0.1.3

FILTR otrzymuje dystans od regeneracji 1156 dla zgodnego CJCD, z dokładnością 0,1 km. Główny ekran ma napięcie zasilania ECU z PID 42, samą wartość z przecinkiem i czcionką 36 px. Dane i układ potwierdzone przez użytkownika.

Rejestrator dodaje dystans, obserwacyjne trendy obu mas sadzy z okna 2–3 minut oraz rzeczywiste przełączenia alarmu i ich przyczyny. WWW/AP rozróżnia schłodzenie od utraty danych. Progi i decyzja alarmu pozostają bez zmian; nie potwierdzono flagi ECU ani ukończenia regeneracji.

Kompilacja i OTA udane. Stare zdarzenie zachowane identycznie bajt w bajt, nowe pola przejazdu i WWW potwierdzone. Użytkownik potwierdził ekran i dotyk. Późniejszą utratę sieci wyjaśnił wyłączeniem zasilania lub Wi-Fi. Pełne okno trendu oraz zapis i trwałość nowego dodatku podczas naturalnego wypalania pozostają do sprawdzenia; nie uruchamiano automatycznych testów.

Wgrany obraz 0x0f0129ae, kompilacja 28.10.2026 12:16:36 +0100; raport docs/REGEN_EVIDENCE_2026-10-28.md. PR #6 pozostaje osobno. Firmware z sekretami i surowe logi nie są publikowane.
