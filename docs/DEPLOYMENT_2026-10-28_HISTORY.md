# Wdrożenie historii zapisanych zdarzeń — 28.10.2026

Źródła zachowania: commit 165718b, PR #10. NORMAL V1; draft PR #6 nie jest częścią firmware.

## Obraz i wdrożenie

- ESPHome 2026.9.1, config_hash 0x4b5f5284, kompilacja 2026-10-28 11:32:04 +0100.
- OTA 1297728 B; SHA256 7d08a7e69a2ba202f436654ab82b90ad0fe400c43c2660a12ff888f424034b6f.
- Wgranie natywnym OTA do potwierdzonego 192.168.0.105 zakończone powodzeniem, proces exit 0.
- Przed OTA zachowano logi, przejazd i zdarzenie w logs/2026-10-28-before-history-final/. Reason=2, świeże EGT3 18,8°C: brak przesłanek trwającego zapisu termicznego.

## Sprawdzenie po restarcie

Panel zwrócił 1/2 zachowanych zdarzeń, 306 próbek oraz 1527 s czasu najnowszego zapisu. Zdarzenie CSV identyczne bajt w bajt przed/po. Log potwierdził datę kompilacji i odbiór CAN 25 ramek/5 s, bez błędów w tym krótkim fragmencie. Pierwsza próbka po starcie miała maskę 39 i świeże EGT3; nie traktowano niepełnej maski po 5 s jako potwierdzenia wszystkich kanałów. Dowody lokalne: logs/2026-10-28-after-history/.

HIST przedstawia istniejące sloty (maksymalnie dwa), nie licznik wszystkich zakończonych regeneracji. Czas obejmuje pełny zapis, również rozgrzewanie i schłodzenie; nie jest dokładnym czasem aktywnej regeneracji. Status pozostaje szacowany. Układ pamięci, CSV, progi i mapy DID bez zmian.

Kompilacja i odczyt urządzenia wykonane; nie uruchamiano automatycznych testów. Użytkownik potwierdził 1 zdarzenie, około 25 min, status zakończenia po schłodzeniu, mieszczące się napisy oraz działający dotyk. Wydanie źródeł: v0.1.2; późniejsze commity dokumentacyjne nie zmieniają wgranego obrazu.
