# Pierwszy zapis zdarzenia — analiza 22.10.2026

Źródła użytkownika: Downloads/monitor-dpf-event.csv, monitor-dpf-drive.csv oraz Niepotwierdzony 333106.crdownload. Nie modyfikowano źródeł ani firmware.

## Zdarzenie

- Nagłówek: event=1, boot=77, reason=2 (zakończenie po temperaturze poniżej 450°C przez 120 s).
- 306 próbek, ms 505855–2033290: 25 min 27,435 s. Maksymalna przerwa 5030 ms, wszystkie maski świeżości 7F. Nie ma widocznych luk w próbkowaniu; maska nie stanowi niezależnego dowodu poprawności ECU.
- EGT3: początek 153,8°C, maksimum 638,7°C, koniec 243,6°C.
- Sadza obliczona: 21,01 → 6,73 g (ubytek 14,28 g); maksimum 21,05 g, minimum 6,71 g.
- Sadza zmierzona: 19,18 → 5,85 g (ubytek 13,33 g).
- Wszystkie 306 odpowiedzi: 115B=00, 12E7=00000000, 134F=00, 11C1=00. Nie ma zmiany pozwalającej dekodować aktywną regenerację. Nie dowodzi to nieistnienia statusu w ECU ani niepoprawnego adresu DID.
- Utrzymywanie wysokiej temperatury wraz z dużym spadkiem obu mas sadzy jest mocnym dowodem procesu spalania sadzy, zgodnego z aktywną regeneracją. Nie jest to potwierdzenie flagą ECU.
- Odtworzenie reguły estymatora na próbkach co 5 s daje ON przy ms 756287, OFF przy ms 1973200 (około 20 min 17 s pomiędzy). To przybliżenie; firmware pracuje co 1 s, CSV nie zawiera faktycznych przełączeń alarmu ani dowodu wyświetlenia komunikatu.

## Po ponownym uruchomieniu

Przejazd zawiera 14 próbek ms 5015–70115 (65,1 s pomiędzy skrajnymi), nie pełne 10 minut. EGT3 około 180°C, sadza obliczona 6,77 g, zmierzona 5,83 g. Pierwsza próbka ma brak części kanałów, kolejne 7F. Nie łączymy osi millis z osią poprzedniego bootu ani nie przypisujemy daty zdarzenia z daty pobrania.

Plik .crdownload zawiera czytelny log tekstowy: uptime 82156 ms, dane do około 39456 ms, ostatnia linia urwana. W obserwowanym fragmencie liczniki CAN zerowe. DID 1156 raw 0000001A oznacza 26 m według stosowanego dekodera; to dodatkowo wspiera niedawne wyzerowanie dystansu od regeneracji. Log pochodzi z nowego uruchomienia i nie dokumentuje alarmu podczas wcześniejszego zdarzenia.

## Wnioski i następny krok

Rejestrator automatycznie zebrał prehistorię, gorący etap oraz schłodzenie i udostępnił trwałe zdarzenie w późniejszym uruchomieniu. Fizycznego odłączenia między tymi konkretnymi plikami nie obserwowano; dowód NVS po utracie zasilania pochodzi z wcześniejszego TESTU.

Nie zmieniamy progów po jednym zdarzeniu. Potrzebne potwierdzenie użytkownika, czy komunikat alarmowy pojawił się i zniknął. Obecne statusy pozostają diagnostyczne; nie wdrażamy reguły raw!=0 ani nie uznajemy zera za brak wypalania. Wykres: dpf-event-analysis-20261022.svg.

## Potwierdzenie ekranu
Użytkownik potwierdził 22.10.2026, że podczas tego zdarzenia komunikat REGENERACJA DPF / NIE GAŚ pojawił się i następnie zniknął. Potwierdza to działanie alarmu szacowanego na ekranie podczas pierwszego rzeczywistego zdarzenia. Dokładne czasy przełączeń nie są znane; powyższe czasy z odtworzenia CSV pozostają przybliżeniem. Następny krok: dalsze zbieranie zdarzeń, w tym obserwacji bez regeneracji, aby ocenić pominięcia i fałszywe alarmy. Jedna udana próba nie potwierdza ogólnej skuteczności ani dekodowania flag ECU.
