# Odczyty odniesienia OBDeleven — 12 października 2026

Źródło: zrzuty ekranu dostarczone przez użytkownika. Pomiary nie są jednoczesne z odczytami monitora. Brak ramek UDS i numerów DID na zrzutach; nie stanowią potwierdzenia przypisania DID.

## DPF, 11:57–11:58

| Parametr | Odczyt |
|---|---|
| Różnica ciśnienia | 14 hPa |
| Zawartość popiołu w oleju (etykieta aplikacji) | 102,1 g |
| Zmierzona masa sadzy | 20,19 g |
| Obliczona masa sadzy | 20,17 g |
| Przebieg od ostatniej regeneracji | 249079 m = 249,079 km |
| Granica zapełnienia dla regeneracji podczas jazdy | 21,08 g |
| Czas od ostatniej regeneracji | 28471 s = 7 h 54 min 31 s |

Próg 21,08 g jest odczytem w tej chwili. Nie udowadnia stałej kalibracji ani aktywnej regeneracji. Nie zastępuje flagi regeneracji ECU i nie uzasadnia automatycznego alarmu NIE GAŚ.

## Temperatury

| Kanał OBDeleven | 12:03 | 12:10 |
|---|---|---|
| Czujnik temperatury spalin 1 | 15,0 °C | 19,0 °C |
| Czujnik temperatury spalin 2 | -0,0 °C | -0,0 °C |
| Czujnik temperatury spalin 3 | 79,7 °C | 87,1 °C |
| Czujnik temperatury spalin 4 | 62,9 °C | 81,7 °C |

Stałe -0,0 °C kanału 2 nie pozwala ustalić obecności czujnika ani ważności pomiaru. Numer kanału nie potwierdza lokalizacji na układzie wydechowym. Odczyt 10FB=15,0 °C z poprzedniego dnia jest podobny do EGT1, ale różne czasy nie pozwalają potwierdzić mapowania.

12:07: obliczona temperatura powietrza doładowującego na wyjściu turbosprężarki 14,4 °C; obliczona temperatura ściany sprężarki 13,7 °C. Są to osobne kanały od temperatur spalin.

## Identyfikacja

EV_ECM20TDI01103L906018SJ, wersja 003003.

## Dalsze potwierdzenie

Porównać monitor z powyższymi kanałami w zbliżonych warunkach. Dystans z porannego logu 241,5–242,9 km i późniejszy odczyt OBDeleven 249,079 km są zgodne co do rzędu wielkości, lecz nie są parą pomiarów potwierdzającą skalę. Popiół i próg wymagają ustalenia DID przed dodaniem ich do odczytów. Status regeneracji pozostaje niepotwierdzony.
## Warunki pomiaru potwierdzone przez użytkownika

Silnik pracował podczas zrzutów temperatur z 12:03 i 12:10. EGT1 15–19 °C i EGT2 -0,0 °C wymagają wyjaśnienia przed uznaniem ich za użyteczne odczyty temperatur spalin; same zrzuty nie dowodzą usterki ani lokalizacji czujników. Nie przypisywać 10FB do EGT1 wyłącznie na podstawie podobnej wartości.

## Odczyt monitora przy pracującym silniku — 12:50

Pobrano car-archive-20261012.txt przez HTTP Digest (200, 49081 B). Ostatnie próbki: 114F=20,62 g, 114E=20,19 g, 14F5=16 hPa, 1156 raw 00 03 CD 12 = 249106 m, 11B2=111,1 °C, 10FB=20,0 °C. Różnica dystansu wobec zdjęcia OBDeleven wynosi 27 m, ale czas pomiarów jest różny. Po początkowych próbach licznik timeoutów na interwał wynosi 0; kolejki i błędy CAN wynoszą 0 w zachowanym fragmencie.

ECU odpowiedział na strony obsługi PID: 00=98 3B A0 13, 20=B0 19 A0 01, 40=CC D2 00 00. Ostatnia strona nie reklamuje następnej strony 60. Próby PID 78, 7A, 7C i 8B zakończyły się timeoutem. Brak standardowego statusu regeneracji nie oznacza braku regeneracji; potrzebny właściwy parametr UDS. Nie znaleziono w aktywnej tabeli sześciu DID kandydata flagi regeneracji. Nie dodawać arbitralnego DID ani interpretacji raw!=0 bez źródła.

Ponownie sprawdzono https://github.com/pashokitsme/vagcan — projekt pozyskuje opisy kanałów z danych ODIS/VCDS; samo repozytorium nie dostarcza gotowej mapy naszego ECU. Nie pozyskano opisu ODX dla statusu regeneracji.

## Dodatkowe zrzuty użytkownika, 13:18–13:20

Sadza obliczona 20,77 g; zmierzona 20,19 g; popiół 102,1 g; dystans 249106 m; paliwo od regeneracji 15,28 l; różnica ciśnień 29 hPa; offset 4 hPa; dynamiczny offset 0 hPa; symulowana temperatura powierzchni DPF 122,8 °C; strumień spalin 54,3 m³/h. Czas od regeneracji: 33276 s na zrzucie 13:18 oraz 33377 s na zrzucie 13:20. Zmierzony współczynnik zapełniania 0,0826 g/km, obliczony 0,0623 g/km.

Dystans 249106 m jest identyczny z wcześniejszą odpowiedzią monitora 1156 raw 00 03 CD 12. To dodatkowy dowód skali U32 BE / 1000 km, nadal bez jednoczesnego pomiaru obu urządzeń.

Kanały wymagające ustalenia DID i definicji:

- Statystyka filtra cząstek stałych, polecenie uruchomienia regeneracji: 0, widoczne bity 0–7 równe 0. Nie utożsamiać żądania ze stanem aktywnej regeneracji.
- Statystyka filtra cząstek stałych, indeks danych regeneracji: 0, widoczne pola bitowe. Nazwa nie ustala znaczenia bitów.
- Filtr cząstek stałych, regeneracja podczas jazdy, warunki przerwania: 0, widoczne bity 0–9 równe 0. Pole ma co najmniej 10 bitów; dekoder musi zachować pełną długość odpowiedzi, nie zakładać U8. Zera nie potwierdzają trwania ani braku regeneracji.
- Regeneracja serwisowa, warunki przerwania: 0, bity 0–7 równe 0; osobny kanał od warunków podczas jazdy.
- Statystyka filtra cząstek stałych, czas trwania regeneracji: 0,00 s. Wymaga sprawdzenia aktualizacji i resetowania podczas naturalnego procesu.

Statystyki regeneracji zawierają też temperatury maksymalne przed/za DPF -273,1 °C oraz zerowe masy, prędkości i czas. Nie przyjmować tych wartości jako fizycznych pomiarów ani dowodu aktualnego statusu; brak opisu ważności i okresu statystyki. Wszystkie nowe zrzuty nie zawierają adresów DID. Nie dodano arbitralnego odpytywania ani nowej logiki alarmu na ich podstawie.
