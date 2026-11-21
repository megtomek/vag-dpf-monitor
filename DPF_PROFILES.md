# Profile danych DPF dla sterowników VAG

Bieżące mapowanie i zakres potwierdzenia: [wspólna tabela parametrów](PARAMETER_REFERENCE.md). Historyczne tropy nie są aktywną mapą wszystkich ECU.

## Dlaczego profil musi być przypisany do sterownika

Adresy, format i skala zależą od ECU oraz SW. Identyfikuj profil po numerze ECU, SW, ODX/ASAM i CALID, nie tylko kodzie silnika.

`main` ma standardowe OBD-II i częściowy profil CJCD. Sadza/ciśnienie były porównywane z OBDeleven, EGT3 przechwycono pasywnie, dystans jest U32 BE metrów. [Tabela parametrów](PARAMETER_REFERENCE.md) określa warunki CALID i zakres potwierdzenia. Nie jest to walidacja pełnej skali ani innych ECU.

Podstawowy alarm jest szacowany z EGT3. `PROFILE_MAPPING_VERIFIED = false` blokuje niepotwierdzoną mapę jako źródło statusu ECU, a nie wszystkie dane. `dpf_did_diagnostic_probe` pozostawia osobną diagnostykę kandydatów. 115B/12E7/134F/11C1 pozostały zerowe w trzech zestawach zdarzeń; nie interpretujemy ich jako aktywnego wypalania.

## Identyfikacja kandydata CJCD

Z odczytu sterownika przekazanego przez użytkownika wynika: `03L906018SJ`, SW `9977`, HW `03L906018JL`, komponent `R4 2.0l TDI H25` oraz ASAM `EV_ECM20TDI01103L906018SJ`, wersja `003003`. Użytkownik identyfikuje silnik jako CJCD. Oficjalny katalog Boscha przypisuje numer `03L 906 018 SJ` do EDC17C46 (Bosch `0281019897`) i zastosowań Audi A4/A5 2.0 TDI o mocy 110 kW z lat 2013–2017, co jest zgodne z rodziną pojazdu i mocą przypisaną CJCD. To potwierdza zgodność numeru części z rodziną aplikacji, ale nie weryfikuje kodu konkretnego silnika ani mapy parametrów DPF, DID-ów UDS czy dekodowania regeneracji. [Bosch Electronic Service — EDC17C46 / 03L 906 018 SJ](https://www.bosch-repair-service.com/hu/motorvezerloe-keszuelekek/bosch/edc17c46-0281019897001/)

Publiczny katalog procedur Vis4VAG pokazuje odrębne warianty ASAM `003002`, `003003` i `003004` dla rodziny `EV_ECM20TDI01103L906018SJ`. Wariant `003003` jest zgodny z identyfikatorem przekazanym przez użytkownika, ale podgląd demo maskuje szczegóły danych i warunków diagnostycznych. Wariantów ASAM nie należy traktować zamiennie ani z samej listy procedur wyprowadzać DID-ów lub dekoderów DPF. [Vis4VAG — katalog dla 03L906018SJ](https://diagnostics.vis4vag.com/de/motoren/uds-motoren-nach-teilenummer/a-4653-01-01-motor-ev-ecm20tdi01103l906018sj-003003/o-555-kraftstoffsystem-entluften?pn=03L906018SJ)

Jako dodatkowy trop, komercyjna aplikacja DPF Monitor for VAG wymienia kod silnika `CJCD` na liście wspieranych silników i zaznacza, że działanie zależy od wersji oprogramowania ECU. Publiczna strona nie ujawnia użytych PID-ów/DID-ów ani dekoderów, więc nie potwierdza zgodności z konkretnym `03L906018SJ / 9977 / 003003`. Może posłużyć jako punkt odniesienia do porównania wskazań podczas testu, nie jako źródło mapy firmware. [DPF Monitor for VAG — lista wspieranych silników](https://www.dpfmonitor.com/)

## IDE a surowy DID

`IDE00427`, `IDE00433` itp. to etykiety/identyfikatory wartości pomiarowych używane w diagnostyce VAG. Nie należy zakładać, że `IDE00433` oznacza surowy DID UDS `0x208A`; surowy adres, długość danych, kodowanie i skalę trzeba potwierdzić oddzielnie dla konkretnego ECU.

## Publiczne projekty jako źródło metody, nie gotowego profilu

Publiczny projekt [`vagcan`](https://github.com/pashokitsme/vagcan) opisuje podejście oparte na danych z ODIS-Service (preferowane) lub lokalnej instalacji VCDS (uzupełniającej). Zawiera narzędzia do odkrywania i obserwacji kanałów, ale deklaruje, że nazwy, layouty bajtów i skale pochodzą z tych zewnętrznych zbiorów, a pomiary ze sprawdzonego auta mają pierwszeństwo. Repozytorium nie daje wprost gotowej mapy DPF dla `03L906018SJ / 9977 / 003003`; ponadto jego firmware wyświetlacza i sprzęt są odrębną platformą. Możemy korzystać z tej metody pracy, ale nie kopiować założeń jako potwierdzonej konfiguracji naszego ECU. OBDeleven pozostaje użyteczne do porównania nazwanych wartości na aucie; samo w sobie nie dostarcza tu surowej mapy DID.

## Identyfikacja i wybór profilu w obecnym firmware

Profil wybiera się w panelu WWW: `AUTO / OBD-II` albo `CJCD (sadza/ciśnienie porównane)`; wybór jest zapamiętywany po restarcie. Ekran USTAW zawiera DEMO, jasność, ręczny AP i automatyczne przełączanie kart; dodatkowy przycisk prowadzi do widoku porównawczego kandydackich DID-ów. Wybór CJCD jest ręczny i nie sprawdza CALID. Odblokowuje tylko trzy częściowo porównane odczyty na zwykłych kartach; temperatura, dystans i alarm regeneracji pozostają diagnostyczne. Ponieważ próba opiera się na pojedynczym zrzucie, porównaj wartości ponownie w innych warunkach przed traktowaniem ich jako w pełni potwierdzonych.

## Test łączności przed dodaniem profilu DPF

Aktywny YAML konfiguruje `esp32_can_recover` na 500 kbit/s, TX `0x7E0`, RX `0x7E8` oraz piny GPIO5/6. Callback przyjmuje ramki tylko z `0x7E8`. Para adresów `0x7E0`/`0x7E8` została potwierdzona w aucie użytkownika: ECU zwróciło nazwę `ECM-EngineControl` oraz kalibrację `03L906018SJ 9977`, zgodną z oczekiwanym sterownikiem. Monitor odbiera poprawne odpowiedzi ISO-TP, bez błędów CAN w zarejestrowanym wycinku. To potwierdza podstawową komunikację z ECU; odczyt parametrów DPF i alarm regeneracji nadal wymagają potwierdzenia. Nie skanuj ani nie zmieniaj adresów w działającym aucie bez uzasadnienia dla konkretnego sterownika.

Gdy `PROFILE_MAPPING_VERIFIED` pozostaje `false`, firmware nie odpytuje placeholderów UDS. Standardowy poller OBD-II Mode 01 zaczyna od ograniczonego, wolnego zapytania nawet wtedy, gdy para diagnostyczna CAN jest cicha — ECU może odpowiedzieć dopiero na żądanie. Po potwierdzeniu aktywności magistrali lub odpowiedzi ECU odpytywanie przyspiesza. Sprawdza PID-y `0x0C` (obroty), `0x05` (temperatura płynu), `0x7A` (ciśnienie DPF), `0x7C` (temperatura DPF), `0x8B` (stan układu oczyszczania spalin), `0x0B` (ciśnienie bezwzględne w kolektorze dolotowym, MAP) i `0x33` (ciśnienie barometryczne, BARO). Wartości ciśnienia/temperatury są publikowane tylko wtedy, gdy odpowiedź ECU potwierdza obsługę odpowiedniego pola. PID-y `0x0B` i `0x33` zwracają pojedynczy bajt w kPa absolute; dla PID `0x0B` dodatnia odpowiedź potwierdza, że ECU udostępnia odczyt, a BARO jest używane tylko po poprawnej odpowiedzi. Szacowane ciśnienie względne MAP−BARO jest dostępne jako encja diagnostyczna, ale aktualny zestaw kart nie ma osobnego ekranu doładowania; świeżość obu źródłowych PID-ów pozostaje wymagana. Dla PID `0x8B` bajt A, bity 0 i 1 deklarują obsługę statusu oraz typu regeneracji; bajt B, bity 0 i 1 określają odpowiednio regenerację w toku oraz jej typ (aktywna/pasywna). Alarm wymaga świeżej odpowiedzi z obsługą obu pól oraz B0=B1=1. Wartości są publikowane jako encje ESPHome. Ekran `USTAW` nie pokazuje szczegółowego statusu CAN/OBD; informacje o obsłudze PID-ów i identyfikacji ECU trzeba odczytywać z diagnostycznych encji/logów ESPHome, a strony danych ukrywają wartości nieaktualne. Zarejestrowana odpowiedź ECU potwierdza podstawową ścieżkę CAN/OBD; nie potwierdza jeszcze, że konkretny VAG ECU udostępnia parametry DPF zgodnie z deklaracją.

| Odczyt | Żądanie ISO-TP na CAN (domyślnie `0x7E0`) | Odpowiedź ECU (domyślnie `0x7E8`) | Dekodowanie |
|---|---|---|---|
| Obroty silnika | `02 01 0C` | `04 41 0C A B` | `((A × 256) + B) / 4` rpm |
| Temperatura płynu | `02 01 05` | `03 41 05 A` | `A − 40` °C |
| Ciśnienie w kolektorze (MAP) | `02 01 0B` | `03 41 0B A` | `A` kPa absolute |
| Ciśnienie barometryczne (BARO) | `02 01 33` | `03 41 33 A` | `A` kPa absolute |

Szacowane ciśnienie względne w kolektorze jest liczone jako `MAP − BARO` i dostępne jako encja diagnostyczna. Aktualny zestaw pięciu kart nie wyświetla tego wyniku; można rozważyć jego dodanie przy dalszej rozbudowie UI. Wartość wymaga obsługi i świeżości obu PID-ów oraz nie zastępuje pomiaru z dedykowanego czujnika doładowania ani nie potwierdza parametrów turbosprężarki.

Mode 01 jest standardową usługą odczytu bieżących danych w SAE J1979; definicje i wymagane PID-y zależą od zgodności pojazdu ze standardem. [SAE J1979](https://saemobilus.sae.org/standards/j1979_201009-e-e-diagnostic-test-modes)

Firmware próbuje standardowych PID-ów `0x7A` (ciśnienie DPF Bank 1), `0x7C` (temperatura DPF) i `0x8B` (stan układu oczyszczania spalin, w tym status regeneracji DPF). Implementowany dekoder PID `0x8B` odpowiada tabeli SAE J1979-DA z października 2011: bajt A bit 0 deklaruje obsługę statusu regeneracji, A bit 1 — obsługę jej typu, B bit 0 oznacza regenerację w toku, a B bit 1 rozróżnia regenerację aktywną (1) i pasywną (0). Firmware pokazuje `NIE GAŚ!` wyłącznie dla świeżych danych, gdy ECU jawnie deklaruje obsługę obu pól, B0=1 i B1=1. Oficjalny katalog SAE wskazuje nowszą rewizję `J1979DA_202607`, zweryfikowaną 16 lipca 2026 r.; jej treść nie jest dostępna w używanym widoku, więc zgodność dekodera z bieżącą rewizją pozostaje niepotwierdzona. Zastosowanie PID-u i dekoder trzeba też potwierdzić dla konkretnego ECU CJCD. [Rekord rewizji SAE](https://saemobilus.sae.org/standards/j1979da_202607-j1979-da-digital-annex-e-e-diagnostic-test-modes)

Ekran TEMP pokazuje kanały EGT Bank 1 sensor 1–3 z PID `0x78` oraz osobną temperaturę wlotu DPF z PID `0x7C`. Każdy EGT pojawia się tylko przy świeżej odpowiedzi z ustawionym bitem obsługi dla tego kanału; numer nie przypisuje go do turbiny, katalizatora ani DPF. PID `0x78` sensor 4 nadal jest dekodowany, lecz nie zajmuje miejsca na ekranie. Mapa UDS dla CJCD pozostaje wyłączona. RPM, temperatura płynu, EGT i DPF mają oddzielne znaczniki świeżości.

Karta FILTR prezentuje także różnicę ciśnień DPF z PID `0x7A` w kPa. Odczyt jest widoczny wyłącznie po świeżej odpowiedzi i jawnym potwierdzeniu obsługi pola; dekodowanie i dostępność w ECU `03L906018SJ / 9977 / ODX 003003` pozostają do porównania z OBDeleven.

PID `0x78` zwraca 9 bajtów danych A–I. Bajt A bity 0–3 deklarują kolejno kanały EGT Bank 1 sensor 1–4; pary B–C, D–E, F–G i H–I dekoduje się jako `raw / 10 - 40 °C`. Firmware odpytuje PID i pokazuje obsługiwane, świeże kanały jako `EGT 1`–`EGT 4`. Numery kanałów nie określają ich położenia w układzie wydechowym; standard dopuszcza, że ECU wylicza wartość na podstawie innych wejść, a nie odczytuje bezpośrednio z czujnika. Jest to definicja tabeli B100 SAE J1979-DA Oct 2011, nie potwierdzenie obsługi przez `03L906018SJ / SW 9977 / ODX 003003` ani zgodności bieżącej rewizji SAE. Dostępność i wartości trzeba porównać z OBDeleven na aucie. [Tabela B100 SAE J1979-DA Oct 2011](https://www.e90post.com/forums/attachment.php?attachmentid=1509324&d=1476497126)

Firmware odczytuje też opcjonalnie nazwę ECU przez OBD Service 09 / InfoType `0x0A` oraz identyfikator(y) kalibracji przez InfoType `0x04`. Są one dostępne jako encje tekstowe „OBD ECU Name” i „OBD Calibration IDs” i mogą pomóc rozpoznać sterownik. ECU może nie obsługiwać tych informacji; brak odpowiedzi jest obsługiwany bez blokowania odczytu standardowych PID-ów. Sama nazwa ECU/CALID nie potwierdza mapowania DPF i nie włącza placeholderów UDS.

Stan regeneracji jest dostępny jako tekstowa encja ESPHome `DPF Regeneration Status` o stanach `active`, `passive`, `inactive` i `unavailable`. `DPF Regeneration Data Source` wskazuje `OBD-II PID 0x8B`, DID z aktywnego, potwierdzonego profilu albo `unavailable`. Przykładowy `UDS DID 0xF401` pozostaje placeholderem i UDS jest wyłączone w bieżącym firmware. `OBD DPF Regen PID 0x8B Support` rozróżnia `unknown`, `supported`, `type unsupported` i `unsupported`: stan `supported` wymaga bitów A0 oraz A1 ustawionych w świeżej odpowiedzi; po utracie łączności możliwości nie są przenoszone do następnego auta. Encja `DPF Regeneration Status Available` wskazuje, czy status pochodzi ze świeżych, jawnie obsługiwanych pól. Automatyzacja powiadomień powinna reagować tylko na przejście do `active`; `unavailable` oznacza brak potwierdzonych danych, a nie brak regeneracji. Sam standardowy PID `0x8B` nadal wymaga potwierdzenia obsługi w konkretnym ECU.

W tabeli z 2011 r. bajty odpowiedzi po `41 8B` to: A — maska obsługiwanych pól; B — stan regeneracji; pozostałe bajty zawierają inne statusy/parametry układu. [Tabela B118 SAE J1979-DA Oct 2011](https://www.e90post.com/forums/attachment.php?attachmentid=1509324&d=1476497126) opisuje A bit 0 jako „DPF Regen Status Supported” oraz B bit 0 jako regenerację w toku. Bosch wymienia `DPF_REG_STAT` („Diesel Particulate Filter Regen Status”) w [słowniku parametrów PID](https://mediathek-bosch-automotive.com/files/bosch_wa/0000/569648_en.pdf); potwierdza to istnienie nazwanego parametru, ale ten słownik nie podaje PID-u ani bitów jego kodowania. Oficjalny katalog SAE potwierdza rewizję `J1979DA_202607`, zweryfikowaną 16 lipca 2026 r. ([rekord rewizji](https://saemobilus.sae.org/standards/j1979da_202607-j1979-da-digital-annex-e-e-diagnostic-test-modes)); nie mamy dostępu do treści tej rewizji. Dekoder firmware opiera się więc na starszej tabeli z 2011 r.; zgodność z bieżącym standardem i docelowym ECU pozostaje do potwierdzenia.

W dekoderze kodu pola PID `0x7A` i `0x7C` odpowiadają definicjom w tabelach B102 i B104 SAE J1979-DA z października 2011:

| PID | Pole danych | Dekodowanie w odpowiedzi `41 <PID> ...` | Znaczenie bitu obsługi |
|---|---|---|---|
| `0x7A` | Bajty B,C (`response[3]`, `response[4]`) | int16 big-endian, ze znakiem, `raw × 0.01 kPa` | Bajt A bit 0: ciśnienie różnicowe DPF Bank 1 |
| `0x7C` | Bajty B,C (`response[3]`, `response[4]`) | uint16 big-endian, `raw × 0.1 - 40 °C` | Bajt A bit 0: temperatura wlotowa DPF Bank 1 |

PID `0x7A` zwraca 7 bajtów danych, a `0x7C` 9 bajtów. Dlatego przy klasycznym CAN ich odpowiedzi mogą wymagać reassemblacji ISO-TP. Dekoder porównano z tabelami B102/B104 SAE J1979-DA Oct 2011 w przeglądzie poniżej; nie potwierdzono go jeszcze względem bieżącej rewizji J1979-DA `202607` ani w konkretnym aucie. Definicja standardu nie gwarantuje, że sterownik udostępnia dany PID ani że wykorzystuje sygnał w strategii sterowania. [SAE J1979-DA Oct 2011](https://www.e90post.com/forums/attachment.php?attachmentid=1509324&d=1476497126); aktualny status wydań: [oficjalny katalog SAE](https://saemobilus.sae.org/standards/supplements).

Przy wieloramkowej odpowiedzi OBD parser sprawdza w pierwszym ISO-TP First Frame, czy początkowe bajty zawierają oczekiwany pozytywny nagłówek `41 <PID>` albo `49 <InfoType>`, zanim wyśle Flow Control. Reassembly sprawdza też długość i numer sekwencji kolejnych ramek. To chroni przed składaniem spóźnionej lub obcej odpowiedzi, ale nie potwierdza, że ECU implementuje dany PID ani że jego wartość ma oczekiwaną semantykę. Warstwę transportową opisuje [ISO 15765-2:2024](https://www.iso.org/standard/84211.html); zgodność z konkretnym ECU wymaga nadal testu w aucie.

### Kandydaci na wspólną warstwę DPF przez OBD-II

Oprócz profili producenta warto sprawdzić standardowe Mode 01 PID-y `0x7A` (DPF Bank 1) i `0x7C` (temperatury DPF). To kandydaci do wykrywania i odczytu, nie gwarancja, że dany samochód VAG je udostępnia. Przed implementacją należy sprawdzić definicję i dekoder w aktualnym SAE J1979-DA oraz potwierdzić zastosowanie dla konkretnego pojazdu. SAE zastrzega, że dane sekcje mogą mieć różną stosowalność zależnie od pojazdu, silnika, rocznika i regionu.

Jeśli są obsługiwane, mogą uzupełnić profil producenta o pomiary DPF. Nie należy zakładać, że dostarczą bitu "regeneracja trwa"; alarm "nie gaś silnika" wymaga osobno zweryfikowanego statusu dla danego ECU. Normatywnym źródłem definicji PID jest aktualna [SAE J1979 Digital Annex](https://saemobilus.sae.org/standards/supplements); opis [SAE J1979](https://saemobilus.sae.org/standards/j1979_201009-e-e-diagnostic-test-modes) zastrzega, że zastosowanie trzeba zweryfikować dla konkretnego pojazdu.

Przed cyklicznym odczytem danych scheduler próbuje pobrać bitmapy obsługiwanych PID-ów od `0x00`, przechodząc do kolejnych stron (`0x20`, `0x40`, `0x60`, `0x80`) tylko wtedy, gdy poprzednia strona sygnalizuje kontynuację. Poprawna bitmapa ogranicza powtarzane timeouty: PID jawnie oznaczony jako nieobsługiwany jest najpierw sprawdzany bezpośrednio, a pomijany dopiero po dwóch timeoutach. Dzięki temu pojedynczy błąd lub niepełna bitmapa nie wyłącza odczytu. Gdy ECU nie odpowiada na zapytanie o bitmapę, firmware przechodzi do dotychczasowego bezpośredniego odpytywania kandydatów. Informacja o bitmapie i wynikach prób jest kasowana po wykryciu nieaktualnego łącza CAN, aby nie przenosić możliwości między pojazdami. To ogranicza zbędne zapytania, ale nadal nie potwierdza obsługi PID-ów przez konkretny sterownik CJCD.

### Przegląd dekoderów standardowych PID-ów w firmware

Porównałem indeksy bajtów i skale w `uds_audi.h` z tabelami SAE J1979-DA Oct 2011. Długości danych są zgodne: PID `0x7A` zawiera 7 bajtów A–G, PID `0x7C` zawiera 9 bajtów A–I, a PID `0x8B` zawiera 7 bajtów A–G. W odpowiedziach OBD `0x41 <PID>` poprzedza te dane, stąd kontrola długości odpowiednio co najmniej 9, 11 i 9 bajtów.

- `0x7A`: A0 deklaruje obsługę różnicy ciśnień; B–C są signed, `0.01 kPa/bit`. Indeksowanie i skala w kodzie są zgodne z tabelą.
- `0x7C`: A0 deklaruje obsługę temperatury wlotowej DPF; B–C są kodowane jako `0.1 °C/bit` z offsetem `-40 °C`. Dekoder używa tej skali.
- `0x8B`: A0 deklaruje obsługę statusu regeneracji, A1 obsługę typu; B0 oznacza regenerację w toku, a B1 rozróżnia aktywną i pasywną regenerację. Kod wymaga świeżych danych, obu deklaracji obsługi oraz B0=1/B1=1, by pokazać alarm aktywnej regeneracji. Obecność pól w normie nie potwierdza, że dany ECU je implementuje.

To potwierdza dekodery względem wydania normy, nie obsługę PID-ów przez konkretny sterownik. Weryfikacja `03L906018SJ / 9977 / ODX 003003` nadal wymaga porównania w aucie z OBDeleven. Źródło: [SAE J1979-DA, revised October 2011, tabele B102, B104 i B118](https://www.e90post.com/forums/attachment.php?attachmentid=1509324&d=1476497126); bieżący katalog wydań: [SAE Mobilus](https://saemobilus.sae.org/standards/supplements).

### Co wiemy o etykietach IDE z logów VCDS

Publiczny log VCDS z Audi A4 B8 CJCD, ECU `03L 906 018 SJ`, wyświetla `IDE00433` jako **oil ash volume** oraz `IDE06059` jako **oil ash mass**. Potwierdza to, że dla tej kombinacji `IDE00433` nie oznacza obliczonej sadzy. Log nie ujawnia jednak surowych adresów UDS, formatu ani skali.

Osobny zrzut VCDS, z etykietą sterownika `03L-906-018-CGL.clb`, pokazuje zestaw `IDE00432` (czas od regeneracji), `IDE00433` (objętość popiołu), `IDE00434` (obliczona masa sadzy), `IDE00435` (zmierzona masa sadzy) i `IDE00436` (kilometry od regeneracji). Podobne nazwy `IDE00434`/`IDE00435` występują w zrzutach VCDS dla innych ECU, m.in. w [dyskusji Ross-Tech o VCDS](https://forums.ross-tech.com/index.php?threads%2F26832%2F=), ale nie potwierdza to wariantu `003003`. Jest to dowód porównawczy dla rodziny etykiet `03L`, a nie potwierdzenie ODX `EV_ECM20TDI01103L906018SJ / 003003`. W szczególności przeczy mapowaniu `IDE00435` jako dystansu; w tym zrzucie dystans ma etykietę `IDE00436`. Żadna z tych etykiet nie podaje sama z siebie surowego DID, długości, skali ani kodowania dla firmware.

### Kandydackie DID-y znalezione w innym projekcie VAG

Otwarty projekt [DPF Load Monitor Wide](https://github.com/v-cu/dpf-load-monitor-wide) publikuje tabelę zapytań UDS oraz funkcje dekodujące w `firmware/main/can_dpf.c`. To źródło daje użyteczne punkty startowe, ale README wyraźnie ostrzega, że część DID-ów różni się między kodami silników; repozytorium nie potwierdza obsługi `CJCD / 03L906018SJ / 9977`. Traktujemy więc poniższe jako **kandydatów do testu**, a nie gotowy profil.

| Kandydat DID | Parametr w projekcie referencyjnym | Dekoder referencyjny |
|---|---|---|
| `0x114F` | Obliczona masa sadzy | U16 big-endian; jedno porównanie wspiera skalę `0.01 g/bit` |
| `0x114E` | Zmierzona masa sadzy | U16 big-endian; dokładna zgodność z `0.01 g/bit` w jednym porównaniu; kodowanie wartości ujemnych nadal nieznane |
| `0x14F5` | Różnica ciśnień DPF | U16 big-endian; jedno porównanie wspiera jednostkę hPa |
| `0x1156` | Kandydat dystansu od regeneracji | U32 big-endian w metrach; wsparty logiem `00 03 A2 E9` = 238313 m |
| `0x115E` | Czas od regeneracji | U24 big-endian, dzielone przez 60 |
| `0x11B2` | Temperatura wejściowa DPF | U16 big-endian; `((raw - 2731) / 10)` |
| `0x10F9` | Temperatura wyjściowa DPF | U16 big-endian; `((raw - 2731) / 10)` |
| `0x10FB` | Temperatura turbosprężarki | U16 big-endian; `((raw - 2731) / 10)` |
| `0x1153` | Popiół olejowy | U16 big-endian, surowa wartość |

W tym pliku nie znaleziono osobnego DID-u flagi aktywnej regeneracji. Nie wolno zatem wnioskować „regeneracja nieaktywna” z braku odpowiedzi ani uznać tych adresów za podstawę alarmu. Format i skale trzeba porównać z danymi dla właściwego sterownika i potwierdzić na pojeździe. Projekt referencyjny ma licencję CC BY-NC-SA 4.0; tabela powyżej jest opisana jako zewnętrzne źródło kandydatów, nie skopiowany kod.

Porównania na aucie, 2026-10-11, wspierają skale `114F/114E = 0.01 g/bit` oraz `14F5 = hPa`; te trzy odczyty są dostępne na zwykłych kartach po ręcznym wyborze CJCD, ale mapowanie pozostaje wstępne i częściowe. Aktualizacja 28.10: `1156 = U32 big-endian w metrach` jest również publikowany na FILTR po dopasowaniu CALID, na podstawie zgodności 249106 m opisanej w OBDELEVEN_REFERENCE_2026-10-12.md. Żaden z tych DID-ów nie steruje alarmem regeneracji. Szczegóły i ograniczenia próbek są zapisane w `DPF_PROFILE_OBDELEVEN_CAPTURE.md`.

Przykładowy zapis VCDS opublikowany dla innego sterownika 03L 906 018 JG opisuje:

| Identyfikator IDE | Znaczenie w tym przykładzie |
|---|---|
| IDE00427 | Różnica ciśnień DPF |
| IDE00428 | Offset czujnika różnicy ciśnień |
| IDE00430 | Status żądania regeneracji polowej |
| IDE00431 | Status blokady regeneracji |
| IDE00432 | Czas od ostatniej regeneracji |
| IDE00433 | Objętość popiołu olejowego |
| IDE00434 | Obliczona masa sadzy |
| IDE00435 | Zmierzona masa sadzy |
| IDE00436 | Kilometry od ostatniej regeneracji |

To jest **przykład z innego ECU**, a nie potwierdzona mapa dla `03L906018SJ`, wersji 9977, CJCD. Różne sterowniki mogą mieć inne zestawy pomiarów i identyfikatory podrzędne `ENG...`.

### Progi sadzy w innych sterownikach — wskazówka do skali, nie limit CJCD

Publiczny zapis dla Audi A4 8K z silnikiem CJCD podaje rozpoczęcie regeneracji przy około `23 g` obliczonej sadzy. Nie podaje jednak numeru ani wersji ECU, więc jest wskazówką dla kodu silnika, a nie potwierdzeniem progu dla `03L906018SJ / 9977 / ODX 003003`. [Dieselschrauber: DPF Reset Audi A4 8K CJCD](https://community.dieselschrauber.org/en/viewtopic.php?t=29987&view=previous)

Publiczne zrzuty VCDS z innych ECU pokazują również próg regeneracji polowej około `23.37 g`, limit regeneracji serwisowej `40 g` oraz przeciążenia `45–50 g`. Przykładowy zrzut z rodziny etykiet `03L-906-018-CGL.clb` pokazuje `IDE01408`/`IDE01409`/`IDE01410` jako te trzy granice, ale nie identyfikuje jednoznacznie numeru SW ECU. Log Ross-Tech z ECU `04L 906 021 FN` pokazuje podobnie `23.37 g` i `40.00 g`. Te wartości pomagają rozróżnić typowy próg rozpoczęcia regeneracji od końca skali, ale **nie potwierdzają** limitów dla docelowej wersji sterownika.

Łuk w bieżącym UI ma zakres wizualny `0–40 g`. To pojemność wskaźnika, nie zweryfikowany limit sadzy ani próg regeneracji dla CJCD. Przy wartości większej niż `40 g` łuk pozostaje pełny, a etykieta liczbowa nadal pokazuje odczyt sensora.

### Dodatkowe źródło dla dokładnego ODX `003003`

Baza procedur diagnostycznych Vis4VAG rozpoznaje dokładnie `EV_ECM20TDI01103L906018SJ 003003` i wymienia procedury sprawdzania czujnika różnicy ciśnień DPF `G505` oraz czujników temperatury spalin `G235/G495/G648`. Potwierdza to, że ten zestaw ODX obejmuje diagnostykę elementów DPF i EGT, ale nie publikuje w widoku demonstracyjnym odczytów Live Data, adresów DID, długości ani skal (wartości procedur są maskowane). Nie jest więc wystarczającym źródłem do odblokowania profilu firmware.

W tym samym indeksie jest procedura „Prüfung des Beladungszustands des Partikelfilters mit Ruß” dla dokładnego ODX `003003`. Podgląd wymienia dwie osobne rozszerzone wartości UDS: obliczone i zmierzone zapełnienie filtra sadzą. Numery IDE, zakresy i warunki są zamaskowane, dlatego potwierdza to tylko istnienie obu kanałów w tej bazie — nie ich identyfikatory, jednostki, skalę ani dostępność Live Data w OBDeleven. [Vis4VAG — kontrola obciążenia sadzą dla ODX 003003](https://diagnostics.vis4vag.com/de/motoren/uds-motoren-nach-teilenummer/a-4653-01-01-motor-ev-ecm20tdi01103l906018sj-003003/o-1116-prufung-des-beladungszustands-des-partikelfilters-mit-russ?pn=03L906018SJ)

Publiczny odczyt ECUHARD dla numeru `03L906018SJ`, SW `9977`, podaje ODX `EV_ECM20TDI01103L906018SJ / 003003`, silnik CJCD oraz wersję HW `X25`. Inne skany pokazują komponent `H25`; należy odróżniać oznaczenie komponentu od osobnego pola wersji HW i nie traktować ich zamiennie. Żadne z tych źródeł nie podaje mapy surowych DPF DID-ów.

## Dane potrzebne do dodania profilu

Przed włączeniem profilu trzeba zebrać i zweryfikować:

1. Identyfikację ECU: numer części, SW/HW, wersję ODX/ASAM i kod silnika.
2. Dla każdej wartości: nazwę z aplikacji diagnostycznej, surowy DID lub adres usługi, długość i format bajtów, signed/unsigned, endianowość, skalę, offset oraz jednostkę.
3. Kilka próbek z silnikiem wyłączonym, rozgrzanym na biegu jałowym i w trakcie obserwowanej regeneracji; szczególnie status regeneracji i temperatury.
4. Zachowanie przy braku obsługi DID, błędzie ECU i utracie łączności. Stan musi wtedy przejść do „brak/nieaktualne dane”, nigdy do „regeneracja nieaktywna”.

## Źródła i ograniczenia

- [Przykładowy log VCDS dla ECU 03L 906 018 JG](https://www.motor-talk.de/forum/motorentyp-ea-189-verbaut-abgasskandal-t5444693.html?page=220) - przykład etykiet IDE i ich opisów, nie mapa DID dla CJCD.
- [Zrzut VCDS DPF z etykietą `03L-906-018-CGL.clb`](https://community.dieselschrauber.org/download.php?id=12813) - pokazuje etykiety IDE00432–IDE00436, IDE06059 oraz progi 23.37/40/50 g; brak pełnej identyfikacji SW, więc to porównanie etykiet, a nie mapa DID dla docelowego ECU.
- [Dyskusja Ross-Tech o odczytach VCDS DPF](https://forums.ross-tech.com/index.php?threads%2F26832%2F=) — potwierdza nazwy IDE00434 (soot mass calculated) i IDE00435 (soot mass measured) w innym ECU; nie mapuje ich na surowe DID-y ani na ODX `003003`.
- [Log Ross-Tech z ECU 04L 906 021 FN](https://forums.ross-tech.com/index.php?threads%2F22958%2F=) - pokazuje próg regeneracji polowej 23.37 g i serwisowej 40 g dla innej generacji ECU.
- [Przykładowy log Ross-Tech dla ECU 03L 906 023 GL](https://forums.ross-tech.com/index.php?threads/18864/) - pokazuje dodatkowe identyfikatory `ENG...` zależne od danych ECU.
- [Log VCDS Audi A4 B8 CJCD, ECU 03L 906 018 SJ](https://a4-klub.pl/topic/476849-częste-wypalanie-dpf-błąd-egr/) — `IDE00433` objętość popiołu i `IDE06059` masa popiołu; nie podaje surowych DID-ów.
- [DPF Load Monitor Wide — kod tabeli zapytań](https://github.com/v-cu/dpf-load-monitor-wide/blob/main/firmware/main/can_dpf.c) — surowe kandydackie DID-y i dekodery; repozytorium zaznacza zależność części DID-ów od silnika i nie potwierdza CJCD.
- [Informacje o ECU 03L906018SJ 9977 / CJCD](https://solutions.ecuhard.com/vw-audi-seat-skoda-porsche/edc17c46-edc17c64/03l906018sj-9977/3021.html) — zgodność numeru ECU, wersji i kodu silnika; strona nie dostarcza mapy odczytów DPF.
- [Procedury diagnostyczne dla ODX `EV_ECM20TDI01103L906018SJ 003003`](https://diagnostics.vis4vag.com/de/motoren/uds-motoren-nach-teilenummer/a-4653-01-01-motor-ev-ecm20tdi01103l906018sj-003003/o-555-kraftstoffsystem-entluften?pn=03L906018SJ) — w widoku demo wymienia testy czujnika różnicy ciśnień DPF i czujników EGT, ale nie ujawnia DID-ów ani dekoderów.
- [Kontrola obciążenia sadzą dla dokładnego ODX `003003`](https://diagnostics.vis4vag.com/de/motoren/uds-motoren-nach-teilenummer/a-4653-01-01-motor-ev-ecm20tdi01103l906018sj-003003/o-1116-prufung-des-beladungszustands-des-partikelfilters-mit-russ?pn=03L906018SJ) — podgląd potwierdza osobne wartości obliczonej i zmierzonej sadzy; numery IDE i wartości są ukryte.

Nie uznajemy żadnego DID z bieżącego szkicu za potwierdzony. Funkcja ostrzegania o regeneracji wymaga najpierw profilu z potwierdzonym źródłem danych i sprawdzenia go w samochodzie.

## Zgodność profilu CJCD z identyfikatorem ECU
Panel pokazuje diagnostykę 'Zgodność profilu z ECU'. Odczyt CALID przez Mode 09 jest porównywany z 03L906018SJ / 9977. Zgodność identyfikatora nie potwierdza całej mapy DPF: trzy parametry mają częściowe potwierdzenie pomiarowe; dystans, temperatury i status regeneracji nadal wymagają weryfikacji.
Rozpoznany CALID w formacie numeru VAG 03L/04L z czterocyfrową wersją SW, ale inny niż oczekiwany, blokuje publikację sadzy i ciśnienia z profilu CJCD. Dotychczasowe wartości są unieważniane, a nowe odczyty pozostają diagnostyczne. Nieznany format CALID lub brak odpowiedzi pozwala na wybór ręczny; panel oznacza brak potwierdzenia. Po utracie połączenia identyfikacja jest ponawiana. Alarm regeneracji nie zależy od samego wyboru profilu ani zgodności CALID.
