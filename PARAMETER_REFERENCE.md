# Wspólna tabela parametrów

Stan kodu: v0.1.3 / aktualne main, 28.10.2026. Tabela jest indeksem aktualnej implementacji i dowodów; historyczne raporty zachowują obserwacje z daty pomiaru. „Odczyt działa” nie oznacza walidacji całego zakresu ani innych ECU.

## Zakres i zapis liczb

CJCD: SW 03L906018SJ/9977, HW 03L906018JL/H25, ODX EV_ECM20TDI01103L906018SJ/003003. CAN 500 kbit/s, żądania 7E0, odpowiedzi 7E8. U16/U32 BE = liczba bez znaku, starszy bajt pierwszy; S16 BE = liczba ze znakiem. Wzory odnoszą się do bajtów wartości, bez nagłówka 62/DID lub 41/PID.

## UDS — aktywne parametry CJCD

| Parametr | DID | Kodowanie → jednostka | Zakres potwierdzenia | Wykorzystanie obecnie |
|---|---|---|---|---|
| Sadza obliczona | 114F | U16 BE / 100 → g | 17,14 g vs 17,13 g OBDeleven; spadek w naturalnym zdarzeniu. Pełna skala niezwalidowana | DPF, pasek 0–24 g, WWW, zapis, trend obserwacyjny |
| Sadza zmierzona | 114E | S16 BE / 100 → g | 16,77 g zgodnie z OBDeleven; spadek w zdarzeniu. Interpretacja wartości ujemnych wymaga pomiaru | FILTR, WWW, zapis, trend obserwacyjny |
| Różnica ciśnień DPF | 14F5 | U16 BE → hPa; /10 → kPa | 13 hPa zgodnie z OBDeleven; brak pełnej walidacji charakterystyki | FILTR/WWW jako kPa; nie decyduje o alarmie |
| Dystans od regeneracji | 1156 | Dokładnie 4 bajty U32 BE metrów /1000 → km | 0003CD12=249106 m zgodne z późniejszym zrzutem; nie jednoczesny pomiar. Po zdarzeniu 26 m; 28.10 odczyt 200,766 km | FILTR/WWW 0,1 km, rozszerzony zapis 0,001 km; wymaga zgodnego CALID. Przyrost/reset do dalszej oceny |
| Czujnik temperatury spalin 3 (EGT3) | 11B2 | Dokładnie 2 bajty U16 BE: (raw−2731)/10 → °C | Pasywne przechwycenie OBDeleven: 0E6A=95,9°C. Naturalne zdarzenie do 638,7°C. Nie ustalono fizycznego miejsca czujnika ani dokładności pełnej skali | TEMP, alarm szacowany, rejestrator; wymaga zgodnego CALID |
| Kandydat temperatury opisany dawniej jako turbo | 10FB | (U16 BE−2731)/10 → °C, wzór kandydata | ECU odpowiada; znaczenie fizyczne i zgodność kanału niepotwierdzone | Diagnostyka; nie przedstawiać jako ustalonej temperatury turbosprężarki |
| Status polecenia regeneracji podczas jazdy | 115B | 1 bajt, surowy hex | Nazwa i DID przechwycone z OBDeleven; 00 przez pierwsze zdarzenie | Zapis diagnostyczny; bez interpretacji bitów |
| Statystyka czasu regeneracji | 12E7 | 4 bajty, surowy hex | Nazwa/DID przechwycone; 00000000 przez zdarzenie. Typ, kolejność i skala nieznane | Zapis diagnostyczny; nie przeliczać arbitralnie na czas |
| Statystyka statusu końca regeneracji | 134F | 1 bajt, surowy hex | Nazwa/DID przechwycone; stale 00 w zdarzeniu | Zapis diagnostyczny; nie potwierdza końca cyklu |
| Status blokowania regeneracji podczas jazdy | 11C1 | 1 bajt, surowy hex | Nazwa/DID przechwycone; stale 00 w zdarzeniu | Zapis diagnostyczny; zero nie oznacza braku regeneracji |

Sadza i ciśnienie wymagają ręcznie wybranego profilu CJCD i nie mogą mieć znanego konfliktu CALID; ich kod dopuszcza jeszcze CALID niepotwierdzony. EGT3 i nowa publikacja dystansu oraz przekazywanie DID do rejestratora wymagają **dopasowanego** CALID. Nie rozszerzać zakresu na inne sterowniki przez sam kod silnika.

Dowody: [porównanie OBDeleven](DPF_PROFILE_OBDELEVEN_CAPTURE.md), [odczyty 12.10](OBDELEVEN_REFERENCE_2026-10-12.md), [przechwycone kanały](CJCD_CAPTURED_DIDS.md), [EGT3](EGT3_DID_CONFIRMED_2026-10-15.md), [pierwsze zdarzenie](FIRST_REAL_EVENT_2026-10-22.md). Dawne określenia „tylko diagnostycznie” dla 1156 lub „temperatura wlotu” dla 11B2 nie opisują bieżącej prezentacji.

## Standardowe OBD — PID trybu 01

A/B oznaczają kolejne bajty danych; w PID 78/7A/7C/8B A jest maską obsługi pól. Poniższe wzory opisują kod, nie gwarantują obsługi przez każdy samochód.

| Parametr | PID | Dekodowanie w kodzie | Dowód dla CJCD / użycie |
|---|---|---|---|
| Napięcie zasilania ECU | 42 | Dokładnie 2 bajty: (256A+B)/1000 V | 28.10 odpowiedź 11,54 V; UI/WWW i ekran potwierdzone. Bez porównania z multimetrem; nie jest bezpośrednim pomiarem klem |
| Ciśnienie w kolektorze MAP | 0B | A kPa absolutnego | Odpowiedzi np. 98 kPa w logach; tło diagnostyczne / wejście wyliczanego doładowania, nie alarmu |
| Ciśnienie atmosferyczne BARO | 33 | A kPa absolutnego | Odpowiedzi np. 98 kPa; tło diagnostyczne. Aktualizacja wewnętrzna przy pominiętym sensorze jest przedmiotem draft PR #6 |
| Obroty | 0C | (256A+B)/4 obr./min | Dekoder i sensor wewnętrzny; brak tutaj osobnego potwierdzenia porównawczego. Obsługa braku sensora: PR #6 |
| Temperatura płynu | 05 | A−40 °C | Dekoder i sensor wewnętrzny; brak tutaj osobnego potwierdzenia porównawczego. Obsługa braku sensora: PR #6 |
| EGT 1–4 | 78 | Pola wskazane A; każda para U16/10−40 °C | W zachowanych próbach CJCD brak odpowiedzi. Obecne EGT3 pochodzi z DID 11B2, nie z tego PID |
| Różnica ciśnień DPF, fallback | 7A | A bit0; S16(B,C)/100 kPa | W zachowanych próbach brak odpowiedzi; nie mylić z działającym 14F5 |
| Temperatura wlotu DPF, fallback | 7C | A bit0; U16(B,C)/10−40 °C | W zachowanych próbach brak odpowiedzi; nie utożsamiać z potwierdzonym miejscem EGT3 |
| Status/typ regeneracji, fallback | 8B | Kod wymaga obsługi A bit0/1; B bit0=w toku, bit1=typ aktywny, według dotychczasowego dekodera | W zachowanych próbach CJCD brak odpowiedzi. Mapowanie standardowe w kodzie nie zostało zwalidowane jako detektor CJCD; podstawą alarmu jest EGT3 |

Puste pole/brak świeżości nie jest zerem ani dowodem nieobsługi PID. Harmonogram uwzględnia bitmapy obsługi i ograniczanie bezskutecznych prób. Identyfikacja korzysta też z trybu 09 (nazwa ECU i CALID), nie są to DID regeneracji. W kodzie pozostały ścieżki historycznych map (np. F401); nie należą do bieżącej dziesięcioelementowej tabeli odpytywanej dla CJCD.

## Wartości wyliczane i zapisy

| Wartość | Reguła / źródło | Znaczenie i ograniczenie |
|---|---|---|
| Alarm szacowany | EGT3 ≥550°C/30 s, wygaszenie <450°C/60 s; utrata danych przerywa ocenę | Główne ostrzeżenie użytkowe. Nie jest flagą ECU; nie ma potwierdzonej ogólnej skuteczności |
| Trend sadzy | Zmiana masy / czas w ciągłym oknie co najmniej 120 s, normalnie ~180 s → g/min | Ujemny=spadek; oba kanały osobno. Brak okna/ciągłości=puste. Nie wpływa na alarm; niepusty trend jeszcze niepotwierdzony na urządzeniu |
| Pasek sadzy | 0–24 g, wizualnie ograniczony do pełnej skali | Nie próg ECU; etykieta zachowuje pełny odczyt. LVGL otrzymuje całkowite gramy |
| HIST | Do dwóch zachowanych zdarzeń; czas od pierwszej do ostatniej próbki | Czas obejmuje rozgrzewanie i schłodzenie, nie czas samego alarmu; nie całkowity licznik udanych regeneracji |
| Szacowane doładowanie | Świeże MAP−BARO | Wartość wyliczana z ciśnień absolutnych, nie nowy czujnik; nie steruje DPF |
| Stan alarmu w CSV v2 | 0=brak danych, 1=nieaktywny, 2=aktywny | Czas przejścia z pętli 1 s, osobno od próbek 5 s; przyczyny i format w raporcie |

Szczegóły rejestratora, kodów przyczyn i nowych blobów: [REGEN_EVIDENCE_2026-10-28](docs/REGEN_EVIDENCE_2026-10-28.md). Zmiana dokumentacji nie rozszerza mapowania ani zakresu potwierdzenia.
