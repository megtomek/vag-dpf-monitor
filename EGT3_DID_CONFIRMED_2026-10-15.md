# EGT3: identyfikacja kanału CJCD na podstawie przechwycenia

Źródła: Downloads/monitor-dpf-can.csv i Screenshot_20261015-094530_OBDeleven.png. Użytkownik wyświetlał pojedynczy kanał „Czujnik temperatury spalin 3”, 95,9 °C.

Zapis: 1055 ramek, czas względny 5–37581 ms, 527 żądań 7E0 i 528 odpowiedzi 7E8. driver_rx_missed=0, skipped_while_locked=0; analizator nie wykrył błędów rekonstrukcji. Pierwsza odpowiedź wyprzedza pierwsze zarejestrowane żądanie, więc liczby nie muszą być równe.

Wszystkie żądania UDS: `22 11 B2`. Odpowiedzi: `62 11 B2` i dwa bajty danych U16 big-endian.

| Dane | Liczba odpowiedzi | (U16 - 2731) / 10 |
|---|---:|---:|
| 0E 66 | 207 | 95,5 °C |
| 0E 6A | 271 | 95,9 °C |
| 0E 6D | 50 | 96,2 °C |

Wniosek: dla badanego ECU DID 0x11B2 odpowiada kanałowi OBDeleven EGT3. Zrzut 95,9 °C jest zgodny z odpowiedzią 0E6A (3690). To bezpośredni dowód adresu i zgodności dekodowania przy tej temperaturze; szersza skala i zachowanie podczas wypalania wymagają dalszej próby. Nie ustalono lokalizacji fizycznej czujnika samym numerem EGT3. Wcześniejsza nazwa TEMP WLOT DPF była kandydacka; można zastąpić ją EGT3.

Raporty analizatora: capture-egt3-20261015-uds.csv i .md. Surowy CSV użytkownika pozostaje w Downloads. Ten pomiar nie potwierdza samego algorytmu regeneracji; pozwala dopiero podłączyć jego właściwe źródło temperatury.

## Integracja firmware

Dekoder publikuje temperaturę 11B2 jako EGT3 po wyborze CJCD i dopiero po zgodnym CALID. Wymaga dokładnie dwóch bajtów wartości, zakresu -40..1200 °C i świeżości odpowiedzi. Czas ostatniego zaakceptowanego odczytu jest śledzony osobno od standardowego PID 78. Dzięki temu algorytm oraz karta TEMP używają tego samego źródła i wygaszają je po utracie świeżości. Źródło alarmu pozostaje oznaczone SZACOWANA. Wersji tej nie wgrano w ramach integracji; na urządzeniu pozostaje rejestrator do zbierania kolejnych DID.
