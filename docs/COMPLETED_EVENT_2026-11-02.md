# Zamknięte zdarzenie nr 4 i odtworzenie po utracie zasilania

Analiza 02.11.2026 — trzeci przeanalizowany zestaw naturalnego wypalania. Numer event=4 nie jest liczbą potwierdzonych przez nas udanych regeneracji. Data raportu jest datą otrzymania plików, nie czasem kalendarzowym zdarzenia.

## Wynik

Event=4, boot=158, reason=2, evidence_schema=2, overflow=0. Rejestrator zamknął zdarzenie po wymaganym schłodzeniu. Użytkownik potwierdził, że plik pobrano po fizycznym odłączeniu i ponownym podłączeniu zasilania. Potwierdza to odzyskanie tego konkretnego naturalnego zdarzenia wraz z nowym dodatkiem: dystansem, trendami i przełączeniami alarmu. Nie jest to próba zaniku zasilania w trakcie zapisu flash.

| Parametr | Wynik |
|---|---|
| Próbki event | 180, wszystkie valid_mask=7F |
| Zakres czasu | 50061–946614 ms od uruchomienia źródłowej sesji |
| Czas zapisu | 896,553 s = 14 min 56,553 s, włącznie z rozgrzewaniem/schłodzeniem |
| Maksymalny odstęp próbek | 5131 ms |
| EGT3 | 106,4 → maks. 649,8 → 270,5°C |
| Sadza obliczona | 21,06 → 5,06 g, ubytek 16,00 g; minimum 5,05 g, maksimum 21,12 g |
| Sadza zmierzona | 21,45 → 0,96 g, ubytek 20,49 g; maksimum 21,60 g |
| Alarm ON | 304243 ms, reason=1 (gorący próg) |
| Alarm OFF | 885843 ms, reason=2 (schłodzenie) |
| Czas alarmu | 581,600 s = 9 min 41,600 s; czas decyzji pętli ~1 s |

Utrzymana wysoka temperatura, duży spadek obu mas sadzy i reset dystansu mocno wspierają pełne wypalenie. Nadal nie odczytano jawnej flagi aktywnej regeneracji ECU. W tym zestawie osobno nie pytano o obserwację komunikatu na ekranie; CSV potwierdza stan algorytmu, a wcześniejsze dwa zestawy miały też potwierdzenie wyświetlenia przez użytkownika.

## Dystans: nowy dowód resetu

Dystans rośnie z 223,808 do 232,699 km, po czym spada do 0,000 km pomiędzy próbkami 776411 i 781419 ms. Reset jest widoczny przy EGT3 613,3°C i sadzy 5,22 / 0,96 g. Alarm jest wtedy nadal aktywny i wygasa 104,424 s po pierwszej próbce z zerowym dystansem.

To wspiera identyfikację DID 1156 jako dystansu od regeneracji i daje kandydacką informację o zakończeniu cyklu. Nie wolno jednak traktować pojedynczego resetu jako flagi aktywności ani używać go od razu do wyłączania NIE GAŚ: temperatura po resecie nadal przekracza 600°C. Do dalszej analizy należy zachować reset jako dodatkowy dowód, bez zmiany obecnych progów.

## Trend i statusy

Trendy obu kanałów są obecne w 152 z 180 próbek; początkowe puste pola odpowiadają zbieraniu okna. Minimum obliczonego trendu −3,4617 g/min; zmierzonego −6,0418 g/min. Dla 144 próbek możliwe było niezależne ponowne obliczenie z wcześniejszych 36 próbek: maksymalne różnice 0,000049319 i 0,000049881 g/min, zgodne z zaokrągleniem CSV. Trend pozostaje obserwacyjny, bez wpływu na alarm. Nie dowodzi to odporności na wszystkie luki danych.

115B=00, 12E7=00000000, 134F=00, 11C1=00 we wszystkich 180 próbkach zdarzenia. Kolejny komplet nie daje mapy bitów; nie traktować zera jako braku regeneracji. Nie dodano nowych DID ani interpretacji.

## Przejazd i logi po ponownym uruchomieniu

Przejazd ma 96 próbek od 5001 do 480860 ms (475,859 s), maksimum przerwy 5106 ms. Pierwsza maska 39 oznacza jeszcze niekompletne odczyty po starcie, kolejne 95 masek to 7F; ecu_link_fresh=1 przez cały bufor. EGT3 216,4 → 132,5°C (maks. 266,2°C); obliczona sadza 5,07 → 5,25 g, zmierzona 0,96 → 0,84 g. Dystans od 0,006 do 1,531 km. Brak ponownego alarmu w tym fragmencie. Dostępnych 59 trendów na kanał pokazuje ponowne zebranie okna.

Nie łączymy osi millis ze zdarzeniem boot=158: log pokazuje pełny nowy start i kompilację 28.10.2026 12:16:36 +0100, zgodną z v0.1.3. Sam log nie podaje nowego numeru boot. Nie ma zapisu tego gorącego zdarzenia w nowym logu, temperatury i masy są już po wypaleniu. Potwierdzenie użytkownika rozstrzyga kolejność pobrania względem fizycznego odłączenia zasilania.

94 raporty CAN: RX missed, TX failed, RX errors, TX errors i bus errors wszędzie 0. Brak [E] i błędów checkpointu. Po starcie wystąpiły timeouty prób PID 7A/7C/8B/78 (jeden raport interwału timeouts=8); 47 pozostałych raportów OBD ma timeouts=0. To rozpoznane bezskuteczne próby standardowych parametrów, nie błędy fizycznego CAN. Nie opisujemy tej sesji jako całkowicie pozbawionej timeoutów. CALID 03L906018SJ 9977 zgodny. Preferencja sieci odtworzona jako AP-only 192.168.4.1; nie zakładamy bieżącej dostępności urządzenia.

## Źródła i ograniczenia

Załączniki monitor-dpf-event _1_.csv, monitor-dpf-drive _1_.csv, monitor-dpf-logs _1_.txt. Lokalne kopie w logs/analysis-2026-11-02/ oraz summary.json z SHA256. Surowych plików nie dołączono do GitHub. Nie kompilowano, nie flashowano, nie zmieniono firmware ani nie uruchamiano testów; wykonano analizę danych użytkownika.

Domknięte: finalny zapis reason=2 i odczyt naturalnego rozszerzonego zdarzenia po fizycznym odłączeniu zasilania. Nadal otwarte: odporność na zanik w trakcie zapisu, luki komunikacji, fałszywe alarmy/pominięcia w innych warunkach, dokładność względem niezależnego odczytu i rzeczywista flaga ECU. Nie uznajemy całego celu projektu za ukończony.
