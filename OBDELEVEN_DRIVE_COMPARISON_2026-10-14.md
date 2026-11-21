# Dwa eksporty przejazdów — analiza 14 października

Źródła: Downloads/chart.csv i chart (1).csv, monitor-dpf-logs (1).txt i monitor-dpf-logs (2).txt. Data analizy nie określa daty wykonania przejazdów. Pliki nie mają wspólnego czasu absolutnego ani parowania próbek obu urządzeń.

Pierwszy CSV: 2383 próbek, time 0–1060114; EGT3 74,3–631,6 °C (koniec 596,1), sadza zmierzona 13,02 → 6,28 g. Aktualnie trwająca procedura i masa sadzy przy uruchomieniu regeneracji stale 0. Materiał sugeruje wypalanie, lecz nie identyfikuje DID ani potwierdzonej flagi ECU. Jednostka time w nagłówku nie jest podana; przy ms czas 17 min 40 s.

Drugi CSV: 291 próbek, time 0–333118; EGT1 19–258,1 °C, EGT2 stale -0,0 °C, EGT3 91,9–132,2 °C, EGT4 94,4–98 °C. Obliczona temperatura powietrza za turbosprężarką 13,4–121,8 °C, obroty 811–2086/min, temperatura zasysanego powietrza 15–17 °C. Brak sadzy i statusu regeneracji. Przy założeniu ms czas 5 min 33 s. EGT3 w całym zapisie pozostaje poniżej proponowanego progu 550 °C; to próbka niskiej temperatury, nie dowód poprawności alarmu we wszystkich obciążeniach.

Oba TXT zawierają logi systemowe, nie ramki CAN: odpowiednio uptime 293172 i 599901 ms. Firmware z 12 października 13:53:47, tryb AP; raporty FT3168 bez unrecovered errors. Brak pomiarów ECU w TXT uniemożliwia bezpośrednie porównanie temperatur i ustalenie DID. Nie wnioskować z samego braku wpisów CAN o uszkodzeniu połączeń lub pustym buforze rejestratora.

Potrzebny kolejny materiał: monitor-dpf-can.csv z sekcji CAN pasywny, zawierający żądania i odpowiedzi podczas odczytu tylko EGT3, oraz zrzut wartości OBDeleven. Nie zastępować go przyciskiem logów systemowych. W source panelu dodano jednoznaczne etykiety; zmiana nie jest jeszcze wgrana.
