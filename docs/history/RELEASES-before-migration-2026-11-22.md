# Archiwum opisów wydań poprzedniego repozytorium

Ten dokument zachowuje stan sprzed przeniesienia. Numery PR, tagi i hashe są historyczne; nie wskazują wydań obecnego repozytorium. [Bieżące wydania](../../RELEASES.md).

# Wydania i rzeczywiste wdrożenia

Ten sam układ opisów obowiązuje tutaj i w GitHub Releases. Tagi pozostają bez zmian; opisy ujednolicono 28.10.2026. Bieżące wdrożenie: [PROJECT_STATUS](../../PROJECT_STATUS.md). Poprzednie opisy zachowano w [archiwum](../../docs/history/RELEASE_NOTES-before-standardization-2026-10-28.md).

## v0.1.3

### Zmiany

FILTR: dystans 1156 przy zgodnym CALID CJCD, 0,1 km. Główny ekran: napięcie zasilania ECU z PID 42, sama wartość z przecinkiem, 36 px. Rejestrator: dystans, obserwacyjny trend sadzy z 2–3 min, czasy i przyczyny rzeczywistych przełączeń alarmu; WWW/AP rozróżnia schłodzenie i utratę danych. Starsze zdarzenia zachowują zgodność. PR #11 i #12; PR #6 pozostaje poza wydaniem.

### Rzeczywiście wgrane

28.10.2026, NORMAL V1: źródła finalnego zachowania c9d475a, config_hash 0x0f0129ae, kompilacja 12:16:36 +0100, OTA 1305152 B. SHA256 i szczegóły: docs/REGEN_EVIDENCE_2026-10-28.md. Wcześniejsze kroki odczytów/UI: docs/DISTANCE_VOLTAGE_2026-10-28.md. Późniejsze commity dokumentacyjne nie zmieniają obrazu.

### Sprawdzone

Walidacja, kompilacja i OTA udane. Stary CSV zdarzenia identyczny, nowe pola przejazdu i sensor oceny WWW potwierdzone. Odczyty 200,8 km i około 11,5 V; użytkownik potwierdził ekran, dotyk i nowe pole. Późniejszą utratę połączenia wyjaśnił wyłączeniem zasilania lub Wi-Fi.

### Pozostaje do sprawdzenia

Niepusty trend po pełnym oknie; naturalne przełączenia alarmu zapisane nową wersją; trwałość nowego dodatku po restarcie i zaniku zasilania. Brak porównania napięcia z multimetrem i ogólnej walidacji detekcji. Trend nie steruje alarmem, progi bez zmian. Nie uruchamiano automatycznych testów. Nadal brak potwierdzonej flagi ECU.

Wersja rozwojowa (prerelease). Bez publikowania BIN/ELF z lokalnymi sekretami i surowych logów. Aktualny stan: PROJECT_STATUS.md; parametry: PARAMETER_REFERENCE.md.

## v0.1.2

### Zmiany

HIST odczytuje do dwóch zachowanych zdarzeń szacowanych, czas całego zapisu i powód zakończenia (PR #10). Odtwarza starsze zdarzenia. Nie jest całkowitym licznikiem udanych regeneracji ECU. Zachowano skalę sadzy 0–24 g.

### Rzeczywiście wgrane

28.10.2026, NORMAL V1: źródła zachowania 165718b, config_hash 0x4b5f5284, kompilacja 11:32:04 +0100, OTA 1297728 B. SHA256 i przebieg: docs/DEPLOYMENT_2026-10-28_HISTORY.md. PR #6 nie jest częścią obrazu.

### Sprawdzone

Walidacja, kompilacja i OTA udane. Odtworzono 1 zapis, 306 próbek, 1527 s; CSV identyczny bajt w bajt. Użytkownik potwierdził dane, mieszczące się napisy i dotyk. CAN odbierał bez błędów w krótkim zachowanym fragmencie.

### Pozostaje do sprawdzenia

Kolejne zdarzenia i ogólna skuteczność estymatora. Czas zawiera rozgrzewanie i schłodzenie, nie jest dokładnym czasem aktywnego wypalania. Alarm i historia pozostają szacowane; brak potwierdzonej flagi ECU. Bez automatycznych testów.

Wersja rozwojowa (prerelease). Bez publikowania BIN/ELF z lokalnymi sekretami i surowych logów. Aktualny stan: PROJECT_STATUS.md; parametry: PARAMETER_REFERENCE.md.

## v0.1.1

### Zmiany

Pasek obliczonej sadzy 0–24 g (PR #8), bez zmiany wartości liczbowej, progów alarmu czy dekodowania ECU. 21 g = 87,5% skali. Uwzględniono porządki lokalnych duplikatów i dokumentację z PR #7.

### Rzeczywiście wgrane

28.10.2026, NORMAL V1: źródła zachowania 49b4342, config_hash 0xf5e87fb9, kompilacja 10:53:33 +0100, OTA 1296960 B. SHA256 i przebieg: docs/DEPLOYMENT_2026-10-28_SOOT_BAR.md. PR #6 nie jest częścią obrazu.

### Sprawdzone

Walidacja, kompilacja i OTA udane. Użytkownik potwierdził kartę DPF, dotyk i wypełnienie około 4/5 przy około 19 g. Odbiór CAN bez błędów w obserwowanym fragmencie, stare zdarzenie CSV identyczne bajt w bajt.

### Pozostaje do sprawdzenia

Ogólna skuteczność alarmu szacowanego i rzeczywisty status ECU. Ta korekta skali nie potwierdza kolejnego wypalania. Nie uruchamiano automatycznych testów wykonania parsera.

Wersja rozwojowa (prerelease). Bez publikowania BIN/ELF z lokalnymi sekretami i surowych logów. Aktualny stan: PROJECT_STATUS.md; parametry: PARAMETER_REFERENCE.md.

## v0.1.0

### Zmiany

Pierwsza nazwana baza źródeł CJCD: samodzielny CAN/OBD/UDS, EGT3 11B2, alarm szacowany, pięć kart, WWW/AP, trzy tryby sieci, zdarzenia NVS, bufor przejazdu i TEST. Uporządkowano dokumentację i zadania (PR #5).

### Rzeczywiście wgrane

Przy tworzeniu tego wydania 27.10 nie kompilowano ani nie wgrywano nowego firmware. Wydanie opisuje bazę funkcjonalną zainstalowaną wcześniej, 15.10. Tag źródeł nie jest dowodem nowego OTA.

### Sprawdzone

Pierwsze naturalne zdarzenie otrzymane 22.10: 306 próbek, EGT3 maks. 638,7°C, sadza obliczona 21,01 → 6,73 g i zmierzona 19,18 → 5,85 g. Użytkownik potwierdził alarm i jego zanik. Wcześniej potwierdzono zawijanie 120 próbek oraz trwałość osobnego TESTU po odłączeniu zasilania. Raporty: FIRST_REAL_EVENT_2026-10-22.md i DRIVE_AND_STORAGE_VERIFICATION_2026-10-15.md.

### Pozostaje do sprawdzenia

Powtarzalność i fałszywe alarmy, rzeczywista flaga ECU, inne ECU oraz warunki pracy. Statusy 115B/12E7/134F/11C1 były zerowe w pierwszym zdarzeniu. Alarm pozostaje szacowany (550°C/30 s, wyjście <450°C/60 s). Nie wykonywano nowych testów przy wydaniu źródeł.

Wersja rozwojowa (prerelease). Bez publikowania BIN/ELF z lokalnymi sekretami i surowych logów. Aktualny stan: PROJECT_STATUS.md; parametry: PARAMETER_REFERENCE.md.
