# Automatyczny zapis zdarzeń DPF

Normalny firmware odczytuje EGT3 11B2, sadzę 114F/114E oraz surowe kanały 115B, 12E7, 134F, 11C1. Dane są przekazywane do rejestratora tylko przy wybranym CJCD i zgodnym CALID. Nie jest potrzebne równoczesne OBDeleven. Statusów bitowych i czterobajtowego czasu nie interpretuje się bez dalszej weryfikacji.

## Alarm i zapis są osobnymi mechanizmami

- Szacowany alarm: EGT3 >=550 °C przez 30 sekund; wyłączenie poniżej 450 °C przez 60 sekund; utrata świeżości daje brak danych. To nie flaga ECU.
- Rejestrator rozpoczyna zdarzenie przy EGT3 >=450 °C przez 15 sekund, aby zachować rozgrzewanie i możliwe fałszywe alarmy. Przed zdarzeniem przechowuje do 3 minut próbek w RAM.
- Próbka co 5 sekund: EGT3, sadza obliczona i zmierzona, surowe cztery statusy oraz maska świeżości. Brak aktualnego kanału daje puste pole; nie jest zamieniany na zero.
- Zakończenie po temperaturze poniżej 450 °C przez 120 sekund, po utracie EGT3 lub po zapełnieniu 900 próbek. Limit wynosi około 75 minut łącznie z buforem poprzedzającym. Utrata odczytu i schłodzenie są różnymi powodami zakończenia; żaden nie potwierdza ukończenia regeneracji ECU.

## Pamięć po wyłączeniu zasilania

Przy rozpoczęciu, co 60 sekund oraz przy zatrzymaniu zdarzenie jest zapisywane jako blob NVS. Dwa naprzemienne sloty ograniczają liczbę przechowywanych zdarzeń; panel udostępnia najnowszy zapis. Nie zmieniano tabeli partycji ani nie kasowano NVS. Nagłe odłączenie może utracić okres od ostatniego udanego checkpointu (nominalnie do 60 s). Jeśli zapis NVS zawiedzie, log zgłosi błąd i nowsze próbki pozostaną tylko w RAM. Restart podczas zdarzenia nie jest oznaczany jako jego poprawne zakończenie.

Nie zapisuje się stale na flash całego przejazdu. Próbki przed zdarzeniem, które nie wywołało zapisu, znikają po odłączeniu. Jazda bez przekroczenia progu może więc dać „brak zapisanego zdarzenia” — to oczekiwane działanie.

## Pobieranie

Karta HIST w trybie szacowanym odczytuje metadane utrwalonych event0/event1: liczbę zajętych slotów (0–2), czas najnowszego zapisu i jego powód zakończenia. Podpis „HISTORIA DPF — SZACOWANA” odróżnia je od cykli potwierdzonych ECU. Czas liczy się od pierwszej do ostatniej próbki i zawiera rozgrzewanie/schłodzenie; nie jest czasem pracy samego alarmu. Zapis przerwany nie jest oznaczany jako udana regeneracja. Metadane odtwarzają się przy starcie i aktualizują dopiero po udanym zapisie NVS. Oddzielny TEST nie wchodzi do tej historii. Format blobów i klucze nie zmieniły się, więc starsze zapisy pozostają czytelne.

W panelu WWW i AP: „Sprawdź zapisane zdarzenie” oraz „Pobierz zdarzenie DPF CSV”. Endpointy GET `/dpf/event-status` i `/dpf/event.csv` korzystają z tego samego uwierzytelnienia Digest co panel. Pobierany jest ostatni utrwalony checkpoint, także podczas zapisu; odczyt nie zatrzymuje rejestratora.

CSV zawiera czas od uruchomienia źródłowej sesji i numer bootu, nie czas kalendarzowy. Cztery statusy zachowano w hex; 12E7 ma osiem cyfr, bo dane mają cztery bajty. Rodzaj liczbowy i skala tego kanału pozostają nieznane. Etykiety pochodzą z przechwyceń OBDeleven, nie pełnego opisu ODX.

Po naturalnej regeneracji wystarczy później włączyć sieć, pobrać CSV i przesłać do analizy. Nie trzeba obsługiwać telefonu podczas jazdy ani wywoływać regeneracji serwisowej.

## Weryfikacja do wykonania

## Zwykły przejazd i osobny TEST pamięci

Przycisk „Pobierz przejazd — ostatnie 10 min” pobiera `/dpf/trip.csv`: ostatnie 120 próbek co 5 s, niezależnie od temperatury. Jest to bufor RAM, więc trzeba pobrać go przed RESET/odłączeniem zasilania. Zawiera `ecu_link_fresh` oraz maskę świeżości kanałów; brak danych to puste pole. Sam brak danych DID nie musi oznaczać uszkodzonej magistrali.

„Utwórz TEST pamięci” zleca zapis osobnego bloba `test_event` w NVS. „Pobierz TEST — dane sztuczne” pobiera `/dpf/event-test.csv`, którego nagłówek jednoznacznie opisuje dane syntetyczne, a reason wynosi 100. Test nie jest zdarzeniem ECU, nie zmienia alarmu, nie zastępuje `event0`/`event1` i nie wstrzykuje temperatur do pomiarów. Po restarcie można pobrać identyczny plik i porównać jego hash.

Endpointy POST `/dpf/event-test-create` i `/dpf/event-test-restart` mają to samo uwierzytelnienie Digest co panel. Drugi jest narzędziem diagnostycznym do sprawdzenia trwałości: wymaga istniejącego bloba TEST, zleca bezpieczny restart po odpowiedzi HTTP i kasuje bufor RAM przejazdu. Przed jego użyciem pobrać zwykły przejazd; nie restartować monitora w trakcie rejestracji rzeczywistego zdarzenia. Programowy restart nie zastępuje próby nagłego odłączenia zasilania.

Kompilacja nie potwierdza działania w aucie. Wymagane: odpowiedzi na nowe DID, świeżość EGT3, alarm i jego zanik, zapis naturalnego zdarzenia, poprawne pobranie CSV oraz odzyskanie checkpointu po wyłączeniu zasilania. Brak zdarzenia w aktualnych warunkach nie dowodzi poprawności tych funkcji.

## Rozszerzenie obserwacyjne — 28.10.2026

Nowe zdarzenia i przejazdy zawierają dystans 1156, trendy sadzy (g/min, ciągłe okno 2–3 min) i stan/czas/przyczynę przełączenia alarmu. Trend jest obserwacyjny, bez wpływu na NIE GAŚ. Dokładne przełączenia pętli 1 s są dodatkowo zapisane w komentarzach nowego CSV zdarzenia; wartości 5 s nie zastępują tych czasów. Schłodzenie wygasza alarm, utrata danych przerywa ocenę; żaden stan nie potwierdza skutecznego zakończenia regeneracji ECU. Nowy opis WWW/AP: „Ocena alarmu szacowanego”.

Starszy format Event pozostaje bez zmian; nowe dane mają osobny wersjonowany blob evidence0/evidence1, z kontrolą numeru zdarzenia, bootu i liczby próbek. Stare zdarzenia i TEST są eksportowane w dotychczasowej postaci. To zastępuje wcześniejszą informację o niezmienionym zestawie kluczy/CSV wyłącznie dla nowych rozszerzonych zapisów. Szczegóły kodów i ograniczeń: docs/REGEN_EVIDENCE_2026-10-28.md.
