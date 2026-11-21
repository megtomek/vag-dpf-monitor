# Drugie naturalne zdarzenie DPF — analiza 29.10.2026

## Wynik

Nowe dane mocno wspierają kolejne wypalanie: EGT3 maks. 642,5°C, sadza obliczona 21,03 → 6,96 g i zmierzona 21,17 → 8,22 g. Użytkownik potwierdził pojawienie się i zniknięcie komunikatu na ekranie. Estymator zapisał rzeczywiste przejścia ON=588429 ms i OFF=1694337 ms, czyli 1105,908 s (18 min 25,908 s); przyczyną wyłączenia było schłodzenie. Progi nie są zmieniane na podstawie tej analizy.

To druga udana obserwacja alarmu szacowanego; nie potwierdzenie flagi ECU ani ogólnej skuteczności. Wszystkie cztery statusy ponownie są zerowe.

## Źródła i sesja

Pliki użytkownika z Downloads/Mobile Devices: monitor-dpf-event.csv, monitor-dpf-drive.csv, monitor-dpf-logs.txt oraz monitor-dpf-TEST.csv. Kopie sprawdzone bajt w bajt zachowano lokalnie w logs/analysis-2026-10-29/, wraz z SHA256 i obliczeniami summary.json. Surowych plików nie dołączono do repozytorium. Data w nazwie raportu to data otrzymania; nie ustalono czasu kalendarzowego przejazdu z osi millis.

- Event: nr 2, boot=130, reason=1 (utrwalony checkpoint), evidence_schema=2, overflow=0.
- 276 próbek, 315505–1692943 ms, czas 1377,438 s = 22 min 57,438 s. Wszystkie maski 7F, maksymalny odstęp 5124 ms.
- Drive: 120 próbek, 1142036–1738018 ms, czas 595,982 s. Wszystkie maski 7F, ecu_link_fresh=1, maksymalny odstęp 5104 ms.
- 111 wspólnych próbek event/drive ma identyczne wartości wszystkich wspólnych pól. Po połączeniu jest 285 unikalnych próbek tej osi czasu; pliki są spójne. Zgodność nie stanowi niezależnej walidacji czujników.
- TEST: wyraźnie syntetyczny, reason=100, boot=130, dwie próbki 600°C. Nie użyto go jako dowodu wypalania ani odporności na restart.

## Temperatura, sadza i alarm

| Wartość | Wynik |
|---|---|
| EGT3 event | 307,0 → maks. 642,5 → 301,7°C |
| EGT3 na końcu drive | 260,9°C |
| Sadza obliczona event | 21,03 → 6,96 g, ubytek 14,07 g; maksimum 21,06 g |
| Sadza zmierzona event | 21,17 → 8,22 g, ubytek 12,95 g |
| Alarm ON | 588429 ms; próbka 591100 ms: 581,6°C, 19,90/19,61 g |
| Alarm OFF | 1694337 ms; reason=2, schłodzenie |
| Czas aktywnego alarmu | 18 min 25,908 s według zapisanych decyzji, z rozdzielczością pętli ~1 s |

Pierwsze chwilowe przekroczenie 550°C w CSV jest wcześniejsze niż start alarmu; nie oznacza to nieprawidłowego opóźnienia, ponieważ warunek wymaga ciągłego przekroczenia przez 30 s, a CSV ma próbki co 5 s. Nie odtwarzano dokładnego stanu pętli 1 s z samych próbek.

## Nowe dowody działania rejestratora

Trend jest niepusty we wszystkich 276 próbkach event. Trend obliczonej sadzy: od +0,3362 do minimum −1,2512 g/min; zmierzonej: minimum −1,4309 g/min. Po rozgrzaniu oba wskazują ubytek. Dla 240 próbek, dla których dostępna jest pełna wcześniejsza historia 36 próbek, ponownie policzono nachylenie z rzeczywistych mas i odstępów czasu: maksymalne różnice 0,000049992 i 0,000049841 g/min, zgodne z zaokrągleniem eksportu do czterech miejsc. To analiza danych, nie automatyczny test firmware. Nie potwierdza zachowania po lukach, DEMO lub restarcie.

Nowy CSV zawiera dystans i przejścia alarmu, w tym OFF późniejsze od ostatniej próbki: OFF=1694337 ms, ostatnia próbka event=1692943 ms. To oczekiwany skutek natychmiastowego checkpointu po decyzji alarmu między próbkami 5 s; nie niespójność pliku. Log zawiera zapis 276 próbek o 1694722 ms, po przejściu OFF. Pobranie odbywa się z utrwalonych blobów NVS: potwierdzono zapis/odczyt rozszerzonego dodatku w tej samej sesji. Brak dowodu jego zachowania po ponownym uruchomieniu.

## Dlaczego reason=1, a nie zakończone zdarzenie

Alarm wymaga 60 s schłodzenia, a rejestrator 120 s. Pierwsza próbka końcowego nieprzerwanego odcinka <450°C jest przy 1637781 ms (427,9°C). Zamknięcia rejestratora oczekiwalibyśmy około 1757781 ms lub przy kolejnym ticku. Drive kończy się 1738018 ms, około 20 s wcześniej. Log pobrano jeszcze wcześniej (uptime 1727951 ms). Dlatego przekazane pliki nie obejmują końcowego checkpointu reason=2. Brak takiego checkpointu w tym zestawie nie jest dowodem błędu zamykania; warto pobrać zdarzenie ponownie po schłodzeniu, jeśli nadal jest dostępne.

## Dystans i statusy ECU

1156: 208,116 → 210,810 km, przyrost 2,694 km w zarejestrowanym fragmencie. Od 1202112 ms wartość pozostaje 210,810 km. Bez prędkości/niezależnego odniesienia nie rozstrzygamy, czy pojazd stał, ani nie oceniamy dokładności dystansu. Nie wystąpił reset licznika; nie można nim potwierdzić zakończenia tego cyklu w dostępnym oknie.

115B=00, 12E7=00000000, 134F=00, 11C1=00 w całym event i drive. Druga obserwacja nie dostarcza mapy bitów. Kanały te nadal nie sterują alarmem; nie interpretować zera jako braku wypalania ani dowodu nieistnienia innego statusu ECU.

## Komunikacja i ograniczenia

W 68 raportach CAN wszystkie RX missed / TX failed / RX errors / TX errors / bus errors wynoszą 0. Raporty OBD podają timeouts=0. W zachowanym oknie brak błędów zapisu checkpointu i wpisów [E]. Bufor logów jest ograniczony wiekiem i liczbą wpisów; overwritten=4239 nie jest licznikiem utraconych ramek CAN. Wniosek dotyczy wyłącznie dostępnego fragmentu, nie całej eksploatacji.

Nie zmieniono firmware ani progów, nie kompilowano, nie wgrywano i nie uruchamiano testów. Nadal potrzebne: finalny reason=2, odczyt nowego dodatku po restarcie, obserwacje jazdy bez wypalania pod obciążeniem oraz kolejne porównania parametrów. Dwa pozytywne zdarzenia nie pozwalają określić odsetka pominięć ani fałszywych alarmów.
