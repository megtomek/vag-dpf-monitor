# Wdrożenie skali paska sadzy 0–24 g

Data: 28.10.2026. Zlecenie: pełny pasek przy 24 g zamiast 40 g. Zmiana źródeł: commit `49b4342`, scalona w PR #8 do `d458f7b84008fed5effa9ebac8a170db92a50c1b`. Późniejsze zmiany dokumentacji wydania nie zmieniają tego firmware. Draft PR #6 nie jest włączony.

## Obraz i wgranie

- NORMAL V1, CAN i OBD włączone.
- ESPHome 2026.9.1; config_hash `0xf5e87fb9`; kompilacja `2026-10-28 10:53:33 +0100`.
- OTA 1 296 960 B; SHA256 `0b324105b30edcd50d67c95313151461783dc0a4d74fb562a887e1628a869273`.
- Przed aktualizacją sprawdzono hash obrazu. Natywne OTA do adresu podanego przez użytkownika zakończyło się powodzeniem i kodem procesu 0.
- BIN/ELF pozostały lokalne; nie są załącznikami wydania.

## Zabezpieczenie danych i wynik

Przed aktualizacją pobrano event-status, bufor przejazdu, logi i zapis zdarzenia do `logs/2026-10-28-before-soot-scale/`. Ostatnie zdarzenie było zakończone (reason=2), a świeże EGT3 wynosiło 26,1°C — brak przesłanek trwającej rejestracji termicznej.

Po aktualizacji log pokazał nową datę kompilacji i ponowny start. Do około 11 s uptime: odbiór CAN, TX failed/RX missed/RX errors/TX errors/bus errors = 0. Próbka przejazdu: maska 7F, ECU link świeży, EGT3 25,3°C, sadza obliczona 19,35 g, zmierzona 19,23 g. Zachowany plik zdarzenia pozostał identyczny bajt w bajt. Dowody lokalne: `logs/2026-10-28-after-soot-scale/`.

Użytkownik potwierdził poprawne wyświetlanie karty DPF, działanie dotyku i wypełnienie około 4/5 przy około 19 g. Wskaźnik LVGL nadal otrzymuje całkowite gramy: 19/24 ≈79,2%; przy dokładnie 21 g ma 87,5%, przy 24 g i wyższej wartości jest pełny. Etykieta liczbowa zachowuje dokładniejszy odczyt.

Nie zmieniono progów alarmu EGT3, rejestratora ani dekodowania ECU. Potwierdzono tę zmianę UI, nie ogólną skuteczność wykrywania regeneracji. Nie uruchamiano automatycznych testów wykonania parsera.
