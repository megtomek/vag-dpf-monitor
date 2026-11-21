# Obsługa monitora

[README](../README.md) · [Instalacja](../BUILD_INSTRUCTIONS.md) · [Pomoc](../SUPPORT.md)

## Odbiór po pierwszym uruchomieniu

1. Na stole sprawdź obraz, dotyk, karty i jasność. DEMO pokazuje symulację.
2. Przed OBD zmierz 5 V, sprawdź wspólną masę, transceiver 3,3 V, TX→D i RX←R. Wgraj NORMAL z CAN/OBD; wyjmij microSD.
3. Na postoju włącz zapłon, sprawdź identyfikację ECU/CALID i wybierz zgodny profil. Inne SW/ODX wymagają oceny.
4. Potwierdź świeżość danych i brak narastających błędów CAN. Sam „ruch wykryty” nie dowodzi dekodowania.
5. Porównaj sadzę obliczoną/zmierzoną, ciśnienie, EGT3 i dystans z narzędziem odniesienia w zbliżonym czasie. Zapisz jednostki i stan zapłonu/silnika. Napięcie pochodzi z ECU, nie z osobnego woltomierza.
6. Obserwuj naturalne zdarzenia; nie uruchamiaj regeneracji serwisowej dla projektu.

## Karty

| Karta | Przeznaczenie |
|---|---|
| DPF | Sadza obliczona, pasek 0–24 g, alarm i napięcie ECU |
| TEMP | EGT3 i dostępne kanały; numer nie ustala fizycznego miejsca czujnika |
| FILTR | Pozostałe odczyty filtra, w tym dystans |
| HIST | Do dwóch zdarzeń NVS, czas i powód zakończenia najnowszego zapisu, nie całkowity licznik regeneracji ECU |
| USTAW | Profil, jasność, DEMO, sieć, automatyczne przełączanie kart |

Kreski oznaczają brak świeżych/obsługiwanych danych, nie zero. Podsumowanie po dotknięciu HIST z PR #18 nie jest jeszcze w `main`; sprawdź wersję przed zgłoszeniem braku funkcji.

## Alarm i zapis

Alarm: EGT3 ≥550°C/30 s, wygaszenie <450°C/60 s. Utrata świeżego EGT3 przerywa ocenę; zniknięcie ostrzeżenia przy braku danych nie oznacza końca wypalania. Monitorowane statusy UDS pozostały zerowe w trzech zestawach i nie stanowią flagi aktywnej regeneracji.

Rejestracja: start ≥450°C/15 s, próbka co 5 s, do około 3 min prehistorii, koniec <450°C/120 s, brak danych lub limit 900 próbek. Czas zapisu zawiera rozgrzewanie/schłodzenie. NVS zachowuje checkpointy; utrata zasilania może stracić końcówkę od ostatniego udanego zapisu. [Format i szczegóły](../AUTOMATIC_DPF_EVENTS.md).

## Sieć i panel

Kafelek USTAW przełącza **router Wi-Fi → tylko AP → radio wyłączone**; wybór przeżywa restart. Brak routera nie włącza AP samoczynnie. Przy wyłączonym radiu wróć do ustawień z ekranu.

Router: ta sama sieć i aktualny IP, ewentualnie `http://audi-dpf-monitor.local/`. AP: połącz z `VAG-DPF-Setup` własnym hasłem i otwórz `http://192.168.4.1/`; brak internetu na tym połączeniu jest oczekiwany. Panel używa loginu/hasła WWW z lokalnej konfiguracji.

Panel jest taki sam w Wi-Fi i AP. HTTP nie zapewnia TLS; nie wystawiaj go do internetu. [SECURITY](../SECURITY.md). Odczyty i zapis działają bez radia.

## Pobieranie po zdarzeniu

| Plik | Zawartość i trwałość |
|---|---|
| Zdarzenie DPF CSV | Najnowszy zapis/checkpoint NVS; przeżywa restart, może być zastąpiony kolejnymi zdarzeniami |
| Przejazd — ostatnie 10 min | Około 120 próbek RAM; pobierz przed restartem/OTA |
| Logi systemowe TXT | 10 min, maks. 1024 wpisy RAM; pobierz przed restartem/OTA |
| TEST pamięci | Osobny zapis syntetyczny, nie wypalanie |

Pobieraj na postoju, zapisz datę, commit/wersję i warunki. Oryginały trzymaj prywatnie. Do Issue przygotuj kopię bez VIN, SSID/BSSID, lokalizacji, haseł i danych osobowych. [CONTRIBUTING](../CONTRIBUTING.md).

## Najczęstsze objawy

| Objaw | Pierwszy krok |
|---|---|
| Ekran działa, odczyty puste | Sprawdź NORMAL/CAN/OBD, profil/CALID, masę, TX→D i RX←R, CANH/CANL; nie przepinaj pod napięciem |
| Ruch CAN, pojedynczy parametr pusty | Sprawdź obsługę i świeżość konkretnego DID/PID |
| Brak WWW | Tryb radia, aktualny IP i izolacja klientów sieci |
| Hotspot nie łączy | Sprawdź 2,4 GHz, uwierzytelnienie i logi, nie tylko SSID |
| Port zajęty | Zamknij jego właściciela, sprawdź numer COM i przewód danych |
| Po restarcie nie ma TXT/przejazdu | Bufory są w RAM; trwały jest osobny zapis zdarzenia |
| Zdarzenie przerwane | Sprawdź powód; checkpoint po restarcie nie jest dowodem skutecznego zakończenia ECU |

Przy błędzie instalacji lub problemie z autem odłącz monitor i wyjaśnij przyczynę. Nie zmieniaj progów lub mapy DID na podstawie pojedynczego odczytu.
