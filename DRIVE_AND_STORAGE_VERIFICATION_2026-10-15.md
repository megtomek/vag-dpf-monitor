# Weryfikacja zapisu przejazdu i TESTU NVS — 15 października

Wgrano normalny firmware przez OTA na 192.168.0.105, obraz 1296800 B, kompilacja 11:02:19. Poprawiono kod odpowiedzi na zlecenie TESTU/restartu z 202 na 200: używany serwer zwracał 500 dla odpowiedzi 202 mimo wykonania operacji. Potwierdzono później HTTP 200 dla restartu.

## Zwykły przejazd

GET /dpf/trip.csv zwrócił 200 i rzeczywiste próbki co około 5 s. Po pierwszym pełnym cyklu DID maska świeżości wynosi 7F, ecu_link_fresh=1. Odczyty po restarcie: EGT3 82,2 → 81,7 °C, sadza obliczona 8,98 g, zmierzona 1,01 g; 115B/134F/11C1 = 00, 12E7 = 00000000. Wszystkie kanały odpowiedziały. Początkowe puste pola sadzy przy masce 39 ustąpiły po pierwszym pełnym odpytywaniu — nie wpisano w ich miejsce zer.

Pliki dowodowe: drive-before-test-20261015.csv, drive-before-restart-20261015.csv, drive-after-restart-20261015.csv. To próbki z postoju/podłączenia OBD, nie pełny dziesięciominutowy przejazd. Zapełnienie i nadpisywanie 120-elementowego bufora nie zostały jeszcze sprawdzone w tej próbie.

## Oddzielny TEST trwałości

Utworzono syntetyczny TEST w oddzielnym kluczu test_event; pobrano 502 B CSV, nagłówek TEST ONLY - SYNTHETIC DATA - NOT ECU REGENERATION, reason=100, dwie sztuczne próbki. Rzeczywisty event-status pozostał „brak zapisanego zdarzenia”. Dane TESTU nie zostały przekazane do sensora ani alarmu.

Pliki event-TEST-before-restart-20261015.csv, event-TEST-after-ota-20261015.csv oraz event-TEST-after-restart-20261015.csv są identyczne bajt w bajt. SHA256 każdego: 0e15aa948a4c05371fcb325b08d1563adf3835cd2b40ad0e351dfad31923d739.

TEST przetrwał restart OTA i osobny restart App.safe_reboot przez chroniony endpoint. Przed restartem pobrano bufor przejazdu, a event-status nie pokazywał rzeczywistego zdarzenia. Po restarcie uptime próbek zaczyna się ponownie około 5 s, potwierdzając ponowne uruchomienie bufora RAM. Log recorder-after-restart-20261015.txt potwierdza start komponentu oraz zerowe liczniki TX failed, RX missed, RX/TX errors i bus errors w obserwowanym fragmencie do około 47 s.

Nie wykonano fizycznego odłączenia zasilania ani przerwania zapisu flash. Nie jest to potwierdzenie całego automatycznego zdarzenia podczas naturalnej regeneracji: sprawdzono NVS, pobieranie i restart z syntetycznym plikiem. Rzeczywisty zapis/checkpoint, alarm i zachowanie przy utracie zasilania w trakcie zapisu wymagają dalszych dowodów.

Dalszy odczyt drive-buffer-current-20261015.csv: 103 próbki, ms 5011–515860 (około 8 min 31 s między skrajnymi próbkami), ostatnia maska 7F, ecu_link_fresh=1, EGT3 75,3 °C. Nadal nie osiągnięto 120 próbek, więc zawijanie bufora pozostaje niepotwierdzone. Ten odczyt nie potwierdza fizycznego odłączenia zasilania.

Potwierdzono limit i pierwsze zawinięcie bufora na urządzeniu: drive-buffer-capacity-20261015.csv zawiera 120 próbek, ms 10013–605984, ściśle rosnące czasy. Pierwsza próbka z ms 5011 została nadpisana. Zakres obejmuje 595971 ms, zgodnie ze 120 próbkami co około 5 s. Wcześniejsza uwaga o niepotwierdzonym zawijaniu jest tym samym rozstrzygnięta. Fizyczny test odłączenia nadal czeka na potwierdzenie użytkownika. W źródle YAML dodano zerowanie ecu_link rejestratora w DEMO; ta poprawka nie została jeszcze skompilowana ani wgrana.

## Fizyczne odłączenie zasilania — wynik
Użytkownik potwierdził ponowne podłączenie po uzgodnionym odłączeniu zasilania. Odczyt event-TEST-after-powercycle-20261015.csv: 502 B, identyczny bajt w bajt z event-TEST-before-restart-20261015.csv; SHA256 0e15aa948a4c05371fcb325b08d1563adf3835cd2b40ad0e351dfad31923d739. Log recorder-after-powercycle-20261015.txt pokazuje uptime 52650 ms, poprawny start komponentu, połączenie Wi-Fi i odpowiedzi ECU. Potwierdzono przetrwanie wcześniej zatwierdzonego TESTU po fizycznej utracie zasilania. Nie sprawdzano przerwania zapisu flash ani naturalnej regeneracji. Ta próba zastępuje wcześniejszy status oczekiwania na odłączenie.

Poprawkę ecu_link=false w DEMO skompilowano i wgrano OTA: config_hash 0xace8c893, kompilacja 11:19:19, obraz 1296816 B. Przed aktualizacją zachowano drive-before-demo-fix-20261015.csv i potwierdzono brak zapisanego zdarzenia. Log recorder-after-demo-fix-20261015.txt potwierdza nową kompilację, odbiór CAN 14–32 ramek/5 s i zerowe liczniki błędów do około 16,6 s. TEST pozostał identyczny. Samego przełączenia DEMO i wartości ecu_link w pobranym CSV po przełączeniu jeszcze nie sprawdzono.

Test DEMO na urządzeniu po poprawce: przełącznik początkowo OFF, włączony na około 7 s, pobrano drive-demo-fix-check-20261015.csv. Ostatnia próbka ms=150243 ma ecu_link_fresh=0. Dane kanałów pozostały ostatnimi rzeczywistymi odczytami w granicach ich 15 s ważności; symulowane wartości nie weszły do CSV. W finally wyłączono DEMO, a GET przełącznika potwierdził value=false/state=OFF. Zachowanie flagi połączenia w DEMO potwierdzone.
