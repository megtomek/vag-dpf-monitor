# FILTR: dystans i napięcie ECU — 28.10.2026

## Przyczyna i zmiana

1156 pozostawał RAW_PROBE_ONLY i nie publikował wartości do distance_since_regen. Przy zgodnym CALID CJCD dodano odczyt dokładnie czterech bajtów U32 BE jako metrów / 1000 km. Normalna karta i WWW pokazują 0,1 km; usunięto filtr delta 1 km, aby zachować prawidłową obsługę NaN i wyzerowania. Zmiana profilu usuwa odczyt i świeżość. Nie resetujemy dystansu według alarmu szacowanego — źródłem jest ECU.

Podstawa porównawcza: OBDELEVEN_REFERENCE_2026-10-12.md, wartość 249106 m zgodna w odczycie i późniejszym zrzucie, bez jednoczesności. Przed OTA 28.10: 1156 raw 0003103E = 200766 m. Dalszy pomiar przyrostu podczas jazdy pozostaje potrzebny.

Dodano Mode 01 PID 42, U16 BE / 1000 V, do istniejącego harmonogramu i mechanizmu obsługi bitmap/timeouts. Dokumentacja producenta narzędzia OBD: https://pelican.clutch.engineering/cars/hyundai/accent/parameters (Control module voltage, standard SAE J1979); lista standardowych PID: https://www.csselectronics.com/pages/obd2-pid-table-on-board-diagnostics-j1979 . Źródła potwierdzają definicję standardu, nie obsługę w CJCD. Odpowiedź musi mieć dokładnie dwa bajty wartości. Napięcie zasilania ECU, nie bezpośredni pomiar klem; bez nowego ADC/okablowania. Wygasza się po utracie świeżości, zmiana ECU zeruje cache. DEMO 13,8 V jest symulowane i oznaczone istniejącą plakietką. Sensor dostępny WWW/AP, label ECU: x240 y46 204×26 na DPF nad statusem. DEMO przesunięto w lewo.

Alarm EGT3, jego progi, zapis NVS, piny i skala sadzy bez zmian. PR #6 nie został włączony. Nie uruchamiano automatycznych testów.

## Korekta prezentacji napięcia

Na prośbę użytkownika osobna czcionka Ubuntu-R 36 px zamiast 18 px (+100%). Label x240 y34 204×42 kończy się przed statusem y78; na ekranie sama wartość z przecinkiem (11,5 V), bez ECU. Sensor WWW zachowuje opis źródła. Źródła korekty: a5c1495. Kompilacja i OTA udane: 0x5f1d120b, 2026-10-28 12:02:08 +0100, 1301616 B; SHA256 80a5496bb857ad702ae5a6d1e2969230cbf9693df9194d34a886ddf51bd6cdc9. Po restarcie log potwierdził datę kompilacji i odczyt PID 42. Zdarzenie zachowane bajt w bajt. Dowody: logs/2026-10-28-before-voltage-font/ i logs/2026-10-28-after-voltage-font/. Użytkownik potwierdził poprawne działanie ekranu i dotyku również po kolejnym wgraniu, z powiększonym napięciem bez prefiksu.

## Weryfikacja i wdrożenie

Źródła firmware: 8ab4cd2, PR #11. Walidacja i kompilacja NORMAL V1 udane: config_hash 0xab6fd8c0, build_time 2026-10-28 11:49:12 +0100. OTA 1298704 B, SHA256 5d02c2136b28f485ac5aee7e6badcdaf5352445dc4f942216f0dc981dd1801ec. Natywne OTA do potwierdzonego 192.168.0.105 zakończone powodzeniem (exit 0).

Po restarcie CALID 03L906018SJ 9977 zgodny. Panel SSE potwierdził sensor Dystans od regeneracji=200.766, prezentację 200.8 km oraz Napięcie zasilania ECU=11.54, prezentację 11.5 V. CAN bez błędów w zachowanym fragmencie do około 27 s uptime. Zdarzenie CSV zachowane bajt w bajt, nadal 306 próbek / reason 2 / 1527 s. Dowody lokalne: logs/2026-10-28-after-distance-voltage/. To nie jest porównanie napięcia z multimetrem ani walidacja dokładności fizycznego pomiaru.

Użytkownik potwierdził obie wartości, poprawny układ i dotyk. Kopia przed OTA: logs/2026-10-28-before-distance-voltage/. Zachowano zdarzenie 1, 306 próbek, reason 2 i świeży przejazd EGT3 14,9°C; użytkownik potwierdził brak wypalania.
