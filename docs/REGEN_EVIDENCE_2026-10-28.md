# Rozszerzone dowody działania alarmu — 28.10.2026

## Zakres

Alarm pozostaje szacowany: 550°C przez 30 s; wyjście poniżej 450°C przez 60 s; brak świeżego EGT3 przerywa ocenę. regen_estimator.h nie zmieniono. Trend sadzy nie wpływa na alarm, nie ma nowej interpretacji 115B/12E7/134F/11C1 ani automatycznego rozpoznania udanego cyklu. Zachowano progi rejestratora (450°C/15 s, chłodzenie 120 s, limit 900, checkpoint 60 s).

## Nowe dane

DID 1156 z dopasowanego CJCD dodaje dystans (U32 BE metry) do zapisu i bufora przejazdu. Osobna świeżość 15 s; brak odpowiedzi daje puste pole. Legacy valid_mask nadal oznacza siedem starych kanałów; nowe pola mają niezależną walidację.

Na każdej próbce 5 s obliczany jest trend obu mas sadzy, g/min, jako różnica ostatniej i najstarszej wartości / czas. Wymagane co najmniej 120 s, normalnie okno do ~180 s, świeży kanał we wszystkich próbkach, odstęp do 10 s. Brak ciągłości lub zbyt krótkie okno daje puste pole. Ujemny trend to spadek, dodatni wzrost, zero brak zmiany. Nie dodano progu trendu ani klasyfikacji „potwierdzona regeneracja”; wartości są materiałem do kalibracji i mogą obejmować korekty modelu ECU.

Przełączenia są przekazywane z rzeczywistego estymatora w pętli UI 1 s, a nie odtwarzane z CSV 5 s. Zapis ms oznacza chwilę decyzji względem bootu; dokładność ograniczona harmonogramem pętli. Początkowa obserwacja też jest oznaczona, nie udaje startu alarmu. Nowe CSV zdarzenia zawierają komentarze # alarm_transition_ms=... oraz w próbkach ostatni stan, ms zmiany i przyczynę.

- alarm_state: 0=brak danych, 1=nieaktywny, 2=aktywny.
- alarm_reason: 0=początek obserwacji/powrót danych, 1=próg gorący, 2=schłodzenie, 3=utrata danych, 4=zmiana trybu.
- reason w nagłówku zdarzenia pozostaje osobnym powodem zamknięcia rejestratora (1–4), a nie alarmu.

Pojedynczy stan po schłodzeniu oznacza wyłącznie wygaszenie alarmu. Nowy sensor tekstowy WWW/AP „Ocena alarmu szacowanego” rozróżnia aktywność, schłodzenie, brak danych i DEMO, jawnie zaznaczając brak potwierdzenia końca cyklu. Nowe pola CSV: distance_since_regen_km, soot_calc_trend_g_min, soot_meas_trend_g_min, alarm_state, alarm_change_ms, alarm_reason.

## Trwałość i zgodność

Nie zmieniono struktury starego Event, magic DPF1, kluczy event0/event1 ani partycji NVS. Dodatkowe rekordy w evidence0/evidence1 mają własny magic DPF2, numer zdarzenia, boot, liczbę próbek i do 128 zmian alarmu (przekroczenie jawnie oznaczone overflow). Oba bloby zapisywane przy dotychczasowych checkpointach oraz przy zmianie alarmu podczas zdarzenia. Nie zakładamy atomowości pary blobów: eksport rozszerzony wymaga zgodności numeru, bootu, liczby próbek i formatu. Brak zgodnego dodatku daje stary CSV bez wymyślania danych. Stare zdarzenie i osobny TEST zachowują identyczną zawartość eksportu. TEST nie został rozbudowany i nie sprawdza nowego dodatku.

DEMO nie dostarcza pomiarów ani trendów do rejestratora; wejście przerywa trwający zapis jako utratę oceny, a bufory wstępne i świeżość są czyszczone. Po wyjściu wymagane są nowe odpowiedzi ECU oraz ponowne zebranie okna trendu. Sam alarm symulowany pozostaje poza dowodami z samochodu.

NVS 0x70000, bez zmiany partycji. Dwa dotychczasowe sloty i ich limity pozostają. Nagły zanik zasilania nadal może utracić dane od checkpointu; zapisy przejść ograniczają tę lukę przy zmianie alarmu, nie stanowią gwarancji odporności na zanik zasilania w trakcie flashowania.

## Weryfikacja

Źródła c9d475a, PR #12 (bazuje na PR #11). Przegląd kodu, walidacja i kompilacja NORMAL V1 udane: config_hash 0x0f0129ae, build_time 2026-10-28 12:16:36 +0100, OTA 1305152 B, SHA256 7109f3622f942a35195a2610a26a61d8a863ef9f152b6f947072a36cf088f337. Natywne OTA do potwierdzonego 192.168.0.105 zakończone powodzeniem (exit 0).

Po restarcie stare zdarzenie wyeksportowane identycznie bajt w bajt (306 próbek, reason 2). Nowy trip.csv schema 2 zawiera dystans 200,766 km, alarm_state=1, alarm_change_ms=2981 i reason=0 (początek obserwacji, nie start wypalania). Krótkie okno poprawnie eksportuje puste trendy. Endpoint sensora WWW odpowiedział 200: BRAK ALARMU: koniec cyklu niepotwierdzony. W logach odbiór CAN bez błędów w obserwowanym fragmencie do ~27 s. Dowody lokalne: logs/2026-10-28-after-event-evidence/. Do 85 s uptime bufor zawierał świeże kanały 7F, dystans i stan alarmu, a trendy były prawidłowo puste przed zebraniem wymaganego okna. Późniejszy odczyt pełnego okna zakończył się timeoutem połączenia TCP; krótka ponowna próba również. Użytkownik potwierdził wyłączenie zasilania lub Wi-Fi, co wyjaśnia utratę dostępności. Potwierdził również poprawny ekran, dotyk, nowe pole WWW oraz powiększone napięcie bez prefiksu. Nie potwierdzono jeszcze niepustego trendu, trwałości nowego dodatku ani naturalnych przejść alarmu. Te próby pozostają otwarte. Wydanie źródeł v0.1.3 obejmuje to rozszerzenie i PR #11; późniejsze zmiany dokumentacyjne nie zmieniają wgranego obrazu. Nie dodawano ani nie uruchamiano automatycznych testów ani sztucznego wypalania. Przed OTA zachowano lokalnie logs/2026-10-28-before-event-evidence/: zdarzenie 1, 306 próbek, reason 2; świeże EGT3 9,8°C. Brak przesłanek aktywnego zapisu termicznego. Nowe zdarzenie z przejściami i trwały dodatek wymagają naturalnego zdarzenia; samo uruchomienie i zwykły przejazd tego nie potwierdzają.
