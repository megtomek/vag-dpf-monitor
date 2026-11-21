# Lista uruchomienia i pierwszej weryfikacji

Ten dokument prowadzi przez uruchomienie monitora. Aktualny firmware ma tryb DEMO oraz standardowe, tylko-odczytowe zapytania OBD-II na magistrali CAN. UDS dla sterownika CJCD i alarm regeneracji z ECU pozostają niezweryfikowane.

## 1. Przed podłączeniem do auta

- Rewizja PCB użytkownika nie ma czytelnego oznaczenia. Na jego prośbę przyjmujemy roboczo V1 (GPIO9), który działa na tej płytce. Używaj `Kompiluj-monitor-DPF.ps1 -BoardRevision V1`; nie wgrywaj V1.1 (GPIO46), dopóki nowe dane nie wskażą innej rewizji.
- W przewodzie od OBD-II pinu 16 (`+12 V`) do przetwornicy umieść bezpiecznik możliwie blisko wtyczki. Przetwornica musi mieć potwierdzoną ochronę odpowiednią do instalacji samochodowej, w tym przed przepięciami i odwrotną polaryzacją; nie zakładaj, że dowolny moduł step-down ją ma.
- Przed podłączeniem płytki zmierz wyjście przetwornicy: ma wynosić `5 V`. Jeśli nie znasz modelu albo parametrów przetwornicy, nie podłączaj jej jeszcze do OBD-II pinu 16; najpierw sprawdź dokumentację modułu i przetestuj go z ograniczonym prądowo źródłem zasilania.
- Moduł SN65HVD230 zasil z `3V3`; połącz `GPIO5 → D/TXD`, `GPIO6 ← R/RXD`, `CANH → OBD pin 6`, `CANL → OBD pin 14`. Nie wkładaj karty microSD: GPIO5/6 współdzielą styki gniazda TF. GPIO1 jest zajęte przez włączenie zasilania ekranu.
- Sprawdź polaryzację i ciągłość połączeń przy odłączonym zasilaniu. Nie dodawaj rezystora 120 Ω bez ustalenia, czy magistrala wymaga dodatkowego terminatora.
- Pamiętaj, że OBD pin 16 może być zasilany także po wyłączeniu samochodu. Na czas postoju odłącz urządzenie, dopóki nie ma kontrolowanego usypiania lub odcięcia zasilania.

Szczegółowe zestawienie połączeń znajduje się w [`HARDWARE_REFERENCE.md`](HARDWARE_REFERENCE.md).

## 2. Przygotuj i wgraj firmware

1. Aktywny firmware działa samodzielnie, bez Home Assistant i API ESPHome. Wi-Fi obsługuje lokalny panel WWW (tryb STA z awaryjnym AP `VAG-DPF-Setup`), a OTA aktualizację po sieci; wymagane klucze znajdują się w `secrets.yaml`.
2. Uruchom `esphome config audi-dpf-monitor.yaml`, a potem `esphome compile audi-dpf-monitor.yaml` zgodnie z [`BUILD_INSTRUCTIONS.md`](BUILD_INSTRUCTIONS.md).
3. Wgraj firmware przez USB dopiero po pomyślnej kompilacji. Pierwsza kompilacja wymaga pobrania ESP-IDF i narzędzi; sprawdź, czy komputer ma dostęp do GitHuba.
4. Zasil płytkę początkowo tylko przez USB. Potwierdź start ESPHome, poprawną orientację ekranu i działanie pięciu kart w kolejności DPF → TEMP → FILTR → HIST → USTAW. Wspólne przyciski nawigacji są w dolnym pasku. Protokół FT3168 i nawigacja po pojedynczym dotknięciu zostały już sprawdzone; kolejność po ostatnim wgraniu USB wymaga jeszcze potwierdzenia wzrokowego.
5. **Sprawdź tryb DEMO:** na karcie USTAWIENIA dotknij przycisku `DEMO: WYŁ.`. Powinna pojawić się etykieta `DEMO` na każdej karcie, zmieniające się przykładowe wartości, a alarm `NIE GAŚ!` ma być widoczny tylko w 60-sekundowej fazie regeneracji aktywnej. Następnie przez 30 s status powinien pokazywać `REGEN PASYWNA` bez alarmu, a przez kolejne 30 s chłodzenie i `DPF OK`. Po 120 s wartości pozostają na ekranie, aż DEMO zostanie wyłączone. Ponowne dotknięcie przycisku zatrzymuje symulację. Restart ESP32 uruchamia tryb normalny z wyłączonym DEMO. Te wartości są sztuczne i nie pochodzą z ECU.
6. Przy podłączonym USB uruchom `esphome logs audi-dpf-monitor.yaml --device COMx` zgodnie z [`BUILD_INSTRUCTIONS.md`](BUILD_INSTRUCTIONS.md). Przy cichej magistrali CAN firmware nie wysyła zapytań OBD. Brakujące liczby na ekranie pokazują `--`, a status regeneracji `BRAK DANYCH`; UI i dotyk powinny działać bez timeoutów TX. Odczyty OBD ruszają po wykryciu ruchu na magistrali.
7. W panelu WWW wybierz `AUTO / OBD-II`, a potem kandydata CJCD. Profil powinien przetrwać restart. Karta USTAWIENIA na ekranie nie wybiera już ECU: zawiera DEMO, jasność, AP i automatyczny widok. Ręczny wybór kandydata w panelu nie potwierdza profilu ani nie odblokowuje DID-ów.

## 2a. Historia konfiguracji Wi-Fi i prób Home Assistant

> Ta sekcja zapisuje wcześniejsze próby z 6 października 2026. Bieżąca konfiguracja korzysta z lokalnego panelu WWW i OTA, ale nie z Home Assistant ani API ESPHome. Opisane niżej timeouty i brak IP są historyczne; nie traktuj ich jako obecnego stanu urządzenia.


1. ESP32-S3 łączy się z siecią 2,4 GHz. W `secrets.yaml` w katalogu projektu sprawdź, czy `wifi_ssid` i `wifi_password` dotyczą dokładnie tej sieci; nie wklejaj tych wartości do logów ani zgłoszeń. Po zmianie tych danych ponownie skompiluj firmware i wgraj go przez USB — wartości z `secrets.yaml` są wbudowywane podczas kompilacji i samo zapisanie pliku nie aktualizuje płytki. W lokalnym sprawdzeniu 6 października YAML poprawnie się parsował, oba klucze były obecne, SSID miało prawidłową długość, a hasło prawidłowy format WPA; komputer nie miał aktywnego połączenia Wi-Fi, więc nie dało się porównać nazwy sieci. To nie potwierdza, że hasło jest zgodne z ustawieniem routera.
2. Po ponownym wgraniu firmware w logu ESPHome szukaj komunikatów o udanym połączeniu i przydzielonym adresie IP. `4-Way Handshake Timeout` oznacza, że urządzenie nie zakończyło uwierzytelniania Wi-Fi; zanim sprawdzisz API/HA, zweryfikuj SSID, hasło, zabezpieczenia punktu dostępowego i siłę sygnału. W logu z 6 października 2026 płytka nadal zgłaszała ten timeout; komputer osiągał wtedy port 8123 Home Assistant, ale ESPHome API nie było połączone. Późniejszy test 120 s pokazał 25 rozłączeń z kodem przyczyny 4 (`DISASSOC_DUE_TO_INACTIVITY` / timeout asocjacji); nie wykryto restartów. ESPHome nie zalogowało w tym oknie potwierdzenia uzyskania IP. To nadal wymaga sprawdzenia w sieci 2,4 GHz i porównania z `secrets.yaml` bez ujawniania jego zawartości.
3. Gdy urządzenie ma adres IP, sprawdź w Home Assistant integrację ESPHome i dodaj urządzenie. Jeśli automatyczne wykrywanie lub nazwa `*.local` nie działa, użyj adresu IP z logu ESPHome.
4. Port COM może być otwarty tylko przez jeden program naraz. Przed uruchomieniem kolejnego odczytu logów albo wgrywaniem zamknij aktywny monitor szeregowy w ESPHome lub innym narzędziu.

## 2b. Próby stabilności obrazu na biurku (6 października 2026)

- Prosty test panelu MIPI i osobny test LVGL z obrotem 90° wyświetlały stabilny obraz; pełny UI DEMO z wyłączonymi radiami został przez użytkownika oceniony jako stabilny.
- Test CAN-idle uruchamiał pełny UI w DEMO, odbiór CAN, wyłączone Wi-Fi i nie wysyłał zapytań OBD. Użytkownik potwierdził, że obraz jest prawidłowy i nie zawiesza się.
- Następnie skompilowano i wgrano na COM6 `audi-dpf-monitor-wifi-stability-test.yaml`: DEMO pozostaje aktywne, CAN nasłuchuje, a Wi-Fi i API są włączone. W DEMO pętla diagnostyczna blokuje żądania OBD. Weryfikacja zapisu flash przeszła.
- Około 100 s logów z testu łączonego nie pokazało restartu ESP32, watchdogu, brownoutu ani panic. Wi-Fi wielokrotnie kończyło próbę komunikatem `4-Way Handshake Timeout`, restartowało adapter i nie uzyskało IP. Sprawdź bez ujawniania wartości w `secrets.yaml`, czy wbudowane w firmware dane odpowiadają sieci 2,4 GHz; możliwe są też problemy z zabezpieczeniami AP lub samym uzgadnianiem. Po stronie Espressifa ten timeout występuje, gdy nie dochodzi do końca czterostopniowego handshake; sam komunikat nie rozstrzyga przyczyny.
- Użytkownik potwierdził prawidłowy obraz i brak zawieszeń po wgraniu testu DEMO z aktywnym Wi-Fi i nasłuchem CAN. Następnie produkcyjny `audi-dpf-monitor.yaml` skompilowano dla V1 i wgrano przez COM6; weryfikacja SHA flash przeszła. DEMO jest domyślnie wyłączone, a CAN włączony.
- Po wgraniu produkcyjnego firmware 90 s logów nie pokazało restartu ESP32, watchdogu, brownoutu, panic, błędów TWAI/CAN ani zapytań PID OBD. CAN nie był podłączony do auta, więc brak żądań jest zgodny z logiką oczekiwania na ruch magistrali. W tym oknie odnotowano 23 timeouty handshake Wi-Fi i 11 restartów adaptera, bez uzyskania IP; osiem odczytów statusu FT3168 odzyskano po pojedynczej ponownej próbie I²C. Wizualne sprawdzenie ekranu w normalnym trybie produkcyjnym pozostaje do potwierdzenia.
- Po ponownym zgłoszeniu zaśnieżenia Wi-Fi wyłączono przy starcie (`enable_on_boot: false`), a pasek obliczonej sadzy rozszerzono do 424 px (x=16) na ekranie 456 px. Firmware V1 z CAN włączonym skompilowano i wgrano przez COM6; hash flash przeszedł. Użytkownik później potwierdził nawrót śnieżenia. W 180 s logów nie było resetów, panic, brownout, błędów wyświetlacza/SPI ani CAN; wystąpiło 12 odzyskanych prób FT3168.
- Test A/B: z CAN wyłączonym użytkownik potwierdził czysty obraz przez kilka minut; logi 180 s nie pokazały błędów wyświetlacza/SPI, panic ani brownout. Z CAN włączonym, Wi-Fi wyłączonym i bez auta użytkownik znów zobaczył śnieg. Pomiar co 10 s przez 180 s nie wykazał narastania zużycia pamięci: internal free pozostał 234008 B, minimum 229848 B, PSRAM zmieniła się z 8302620 B do 8302588 B. Kolejka TX pozostała pusta, liczniki TX/RX/bus error oraz OBD queries wynosiły zero. Było 13 odzyskanych prób odczytu FT3168 bez błędów końcowych. Wyniki nie wspierają hipotezy zapełniania pamięci lub kolejki przez brak odpowiedzi ECU; wskazują na wpływ aktywnego kontrolera CAN lub współdziałanie jego ścieżki z UI. Tymczasowa telemetria RAM została usunięta ze źródeł.
- Po teście przywrócono i wgrano przez COM6 wariant V1 z CAN i Wi-Fi wyłączonymi; weryfikacja hash flash przeszła. Użytkownik potwierdził ten wariant jako wizualnie stabilny. Do użycia OBD-II po podłączeniu auta CAN trzeba ponownie włączyć i ponowić próbę stabilności.
- Pełna próba CAN, dane DPF, mapa UDS CJCD i alarm regeneracji wymagają późniejszej weryfikacji na samochodzie oraz porównania z OBDeleven.
- Po zgłoszeniu, że przycisk RESET nie przywracał obrazu, dodano wczesny cykl zasilania AMOLED przez GPIO1: panel jest wyłączony na 300 ms, włączony, a następnie inicjalizowany przez sterownik MIPI (impuls resetu GPIO21). Wariant V1 z CAN i Wi-Fi wyłączonymi skompilował się i został wgrany na COM6; weryfikacja flash przeszła. Użytkownik potwierdził, że po naciśnięciu RESET obraz wrócił bez odpinania USB. Logi po ponownym starcie potwierdziły poprawne uruchomienie ESP32 i panelu; nie stwierdzono panic, brownoutu ani restartów pętli. To potwierdza skuteczność odzyskiwania po resecie, ale stabilność wielogodzinną i przy aktywnym CAN trzeba jeszcze sprawdzić.
- Wariant z CAN aktywnym, Wi-Fi wyłączonym i tym samym cyklem zasilania AMOLED także skompilował się i został wgrany. Użytkownik ponownie zaobserwował kolorowy śnieg. W chwili zgłoszenia logi nadal regularnie raportowały FT3168 co 2 s i pojedyncze odzyskiwane ponowienia I²C; nie było komunikatu restartu, panic ani brownoutu. To wskazuje na problem w torze wyświetlania/renderowania, a nie na całkowite zawieszenie MCU. Po teście ponownie skompilowano i wgrano wariant CAN-off/Wi-Fi-off; zapis flash zweryfikowano. Cykl GPIO1 pomaga w ponownym uruchomieniu panelu po resecie, ale sam nie zapobiegł artefaktom podczas pracy CAN-on.

## 3. Próba odczytów OBD-II w samochodzie

CAN i standardowe zapytania OBD-II są aktywne. Firmware wstrzymuje zapytania, jeśli przez 2 s nie widzi ruchu CAN; po wykryciu ramek najpierw pobiera bitmapę PID-ów od `0x00`, a strony `0x20`–`0x80` tylko wtedy, gdy ECU zgłosi kontynuację. Następnie odpytuje wartości. PID niezgłoszony przez bitmapę nadal otrzymuje dwie bezpośrednie próby; brak bitmapy nie blokuje fallbacku. Po utracie łączności pamięć bitmap i oczekujące zapytanie są kasowane, by nie przenosić stanu na inne ECU. Przy pierwszej próbie sprawdzamy odpowiedź ECU oraz jednostki, nie wymuszamy żadnych funkcji serwisowych. DEMO blokuje pętlę diagnostyczną.

1. Samochód zaparkuj, zabezpiecz przed ruszeniem i włącz zapłon. Na tym etapie nie wykonuj jazdy ani regeneracji serwisowej.
2. Podłącz monitor do OBD-II i odczekaj, aż firmware wykryje ruch CAN, a następnie kilka cykli odpytywania. W lokalnym panelu WWW sprawdź encje diagnostyczne `Stan regeneracji DPF` oraz `Obsługa PID regeneracji 0x8B`: `OK` oznacza świeży, obsługiwany PID, `BRAK` jawnie nieobsługiwany PID, a `OCZEK.` brak rozstrzygającej odpowiedzi. Nie ma globalnego paska statusu OBD. Przy włączonym zapłonie i wyłączonym silniku obroty powinny wynosić zero, jeśli ECU odpowiada na standardowy PID.
3. Jeśli PID temperatury płynu odpowiada, porównaj odczyt z OBDeleven. Nie traktuj braku odpowiedzi na PID DPF `0x7A` lub `0x7C` jako awarii CAN: ECU może ich nie udostępniać.
4. Uruchom silnik na postoju i porównaj z OBDeleven tylko te wartości RPM, płynu, ciśnienia DPF, temperatury DPF, MAP i BARO, które monitor pokazuje jako świeże. MAP i BARO są ciśnieniami bezwzględnymi; bieżący ekran nie pokazuje ich różnicy jako doładowania. Zapisz jednostki i czas obserwacji; nie zakładaj obsługi PID `0x0B` ani `0x33` z góry.
   Jeśli ECU udostępnia PID `0x7C`, sprawdź też, czy jego świeża temperatura wlotowa DPF pojawia się na ekranie temperatur jako źródło zapasowe, kiedy profil UDS nie jest aktywny.
5. Przejdź między ekranami i sprawdź, czy dotyk trafia w przyciski. Nazwa ECU i CALID nie są wyświetlane na małej karcie USTAWIENIA; jeśli ECU je udostępnia, sprawdź encje `Nazwa ECU` i `Identyfikatory kalibracji` w lokalnym panelu WWW lub logu USB.
6. Po wyjęciu wtyczki OBD odczyty mają zniknąć lub zostać oznaczone jako nieaktualne. Encje identyfikacji ECU/CALID w panelu WWW powinny zostać wyczyszczone i pobrane ponownie po odzyskaniu łączności.

## 4. Warunki akceptacji pierwszego testu

- Ekran jest poziomy, teksty nie nachodzą na siebie, a dotyk działa po obrocie LVGL na wszystkich pięciu kartach.
- Wszystkie karty są dostępne ze wspólnego paska nawigacji; brak odpowiedzi Mode 09 nie może blokować standardowych odczytów OBD.
- Po wykryciu ruchu CAN firmware loguje odebrane strony bitmap OBD, odpytuje standardowe PID-y i zachowuje bezpośredni fallback dla niezgłoszonych PID-ów oraz ECU bez obsługi bitmap. Po utracie łączności następny skan rozpoczyna się od nowa.
- Status OBD reaguje na podłączenie i odłączenie ECU; stare liczby nie pozostają przedstawiane jako świeże.
- Nazwa ECU i CALID są czyszczone po utracie łączności oraz odczytywane ponownie po jej odzyskaniu, także po przełożeniu monitora do innego auta.
- Wartość MAP pojawia się tylko po poprawnej odpowiedzi PID `0x0B`, jest oznaczona w kPa abs i zanika lub staje się nieaktualna po utracie łączności.
- Encja szacowanego ciśnienia względnego jest dostępna tylko wtedy, gdy świeże są jednocześnie MAP (`0x0B`) i BARO (`0x33`); to różnica ciśnień, nie bezpośredni pomiar turbiny.
- Odczyty standardowych PID-ów są porównane z OBDeleven w podobnych warunkach pracy.
- Przy braku świeżego, potwierdzonego sygnału regeneracji interfejs pokazuje „BRAK DANYCH”, nigdy stan nieaktywny ani alarm. „DPF OK” wolno pokazać wyłącznie po świeżym odczycie obsługiwanego statusu, który jest nieaktywny; komunikat nie oznacza sprawnego filtra ani gotowości do wypalania. „NIE GAŚ!” wymaga świeżego, potwierdzonego sygnału aktywnej regeneracji.
- W lokalnym panelu WWW `Stan regeneracji DPF` ma stan `unavailable`, gdy ECU nie potwierdza świeżego statusu. Stan nieaktywny wolno pokazać wyłącznie przy świeżym, obsługiwanym polu.
- W lokalnym panelu WWW `Obsługa PID regeneracji 0x8B` pokazuje `unknown` przed odpowiedzią, a potem stan zgodny z bitami obsługi. `Źródło statusu regeneracji` wskazuje PID/DID tylko przy świeżych danych; bez nich ma stan `unavailable`.

## 5. Co jest potrzebne przed testem alarmu regeneracji

Obecny profil `CJCD / 03L906018SJ / SW 9977 / ODX 003003` nie ma zweryfikowanej mapy surowych DID-ów, skali i bitu aktywnej regeneracji. `PROFILE_MAPPING_VERIFIED` pozostaje wyłączone. Zapisz w OBDeleven nazwę oraz identyfikator IDE każdego parametru, jednostkę i próbki ze stanu przed regeneracją, w jej trakcie i po zakończeniu. Nie wymuszaj regeneracji na potrzeby testu. Instrukcje zbierania próbek znajdują się w [`DPF_PROFILE_OBDELEVEN_CAPTURE.md`](DPF_PROFILE_OBDELEVEN_CAPTURE.md).

Alarm „nie gaś silnika” może korzystać ze standardowego PID `0x8B`, jeśli ECU jawnie zgłosi obsługę zarówno statusu regeneracji, jak i jej typu. Według dekodera zgodnego z SAE J1979-DA Oct 2011 wymaga to A0=1, A1=1, B0=1 i B1=1 w świeżej odpowiedzi. Podczas testu porównaj wskazanie z OBDeleven w czasie obserwowanej regeneracji; jeżeli sterownik nie udostępnia pól lub odpowiedź jest nieświeża, alarm pozostanie w stanie „brak danych”. Odczyty RPM, temperatury płynu, ciśnienia i temperatury DPF same w sobie nie potwierdzają regeneracji. Mapa UDS dla ECU CJCD może być dodana po potwierdzeniu jej DID-ów, skali i stanu regeneracji.

## 6. Ostatnia weryfikacja UI i CAN na biurku (6 października 2026)

- Zmieniono odświeżanie LVGL tak, aby widoczność plakietek DEMO, status regeneracji i migający alert aktualizowały się tylko przy zmianie stanu. Ustawiono `data_rate: 40MHz`, zgodnie z presetem Waveshare dla kontrolera CO5300; wcześniejsze 5 MHz i 10 MHz powodowały wolniejsze pełne odrysowanie.
- Firmware V1 z CAN włączonym skompilowano i wgrano przez COM6. Weryfikacja SHA flash przeszła. Log rozruchu wykazał poprawne uruchomienie ESPHome, wykrycie FT3168 o ID `0x03` oraz brak zweryfikowanego profilu UDS; logował jedno ostrzeżenie LVGL podczas początkowego rysowania (121 ms przy 40 MHz; wcześniejszy build 10 MHz zgłaszał do 213 ms).
- Użytkownik potwierdził, że UI działa płynniej, a obraz wydaje się stabilny. Dodatkowy nasłuch szeregowy przez ponad 3 minuty nie wykazał kolejnych ostrzeżeń LVGL ani restartów; pojawił się komunikat ESPHome `Boot seems successful`.
- Na stole bez aktywności CAN poller nie wysyła zapytań OBD-II. Po wykryciu ramek ruszają standardowe, tylko-odczytowe PID-y. UDS dla CJCD pozostaje wyłączone (`PROFILE_MAPPING_VERIFIED = false`).
- To potwierdza działanie UI w teście biurkowym, ale nie długotrwałą stabilność w aucie. Nadal trzeba porównać dostępne PID-y z OBDeleven oraz potwierdzić PID `0x8B` lub zweryfikowany DID UDS podczas rzeczywistej regeneracji.
## 7. Kontrolowany test USB bez CAN (7 października 2026)

- Użytkownik potwierdził, że płytka jest zasilana wyłącznie z USB komputera; zewnętrzny transceiver CAN nie jest podłączony.
- Pierwsze polecenie `esphome upload` bez zmiennej `ESPHOME_DATA_DIR` użyło starego artefaktu z domyślnego katalogu `.esphome`; log pokazał próby łączenia Wi-Fi. Nie był to poprawny test aktualnego YAML-a.
- Wyczyściliśmy właściwy katalog builda `.esphome-data`, skompilowaliśmy aktualny YAML z `-CanBusEnabled false`, a następnie wgraliśmy obraz na COM6. ESPHome potwierdziło hash zapisu flash. Świeży build nie zawiera konfiguracji Wi-Fi/API/OTA, wyklucza `esp_wifi` w CMake i wywołuje `set_enabled(false)` dla kontrolera CAN.
- Wprowadzono `Wgraj-monitor-DPF.ps1`, który jawnie ustawia `.esphome-data`, kompiluje wariant i wgrywa dokładnie ten obraz. Dzięki temu ręczny upload nie powinien wybierać starego katalogu `.esphome`.
- Po wgraniu nie uzyskaliśmy logów startowych przez monitor szeregowy. Sam brak logów nie potwierdza poprawnego startu ani stabilności ekranu; oczekujemy na wizualny wynik testu na USB.
- Przegląd aktualnej pętli `ESP32CanRecover` potwierdza maksymalnie 16 obsłużonych ramek na obieg oraz odczyt stanu TWAI co 250 ms. W poprzednim teście CAN-on, bez auta, kolejka TX i liczniki CAN pozostawały zerowe, a pamięć nie wykazywała narastającego zużycia. To osłabia hipotezę zalewania kolejki lub wycieku pamięci; nadal trzeba sprawdzić wpływ aktywnego kontrolera CAN na obraz.
- Dodatkowo po czystym buildzie skompilowano aktualny wariant samochodowy z `CAN=true`; walidacja i kompilacja ESPHome 2026.9.1 przeszły. Obraz ma 604859 B, wykorzystuje 19,07% DIRAM i 7,4% flash. Wygenerowany kod ustawia `obd_can->set_enabled(true)`, nadal bez komponentu Wi-Fi. Obrazu CAN-on nie wgrano — płytka pozostaje na wariancie stołowym bez CAN podczas oczekiwania na wynik obserwacji.
- Odczytano przez `esptool read_flash` 614400 B z partycji aplikacji płytki podłączonej do COM6. Zidentyfikowano ESP32-S3 revision v0.2 z interfejsem USB-Serial/JTAG. Początek działającej aplikacji pasuje bajt w bajt do lokalnego `audi-dpf-monitor-V1-can-off-test.ota.bin` (603376 B) i nie pasuje do obrazu CAN-on. To bezpośrednio potwierdza, że płytka pracuje na firmware CAN-off; nie opieramy się tu na domyślnej wartości substytucji w YAML ani na nazwie artefaktu. Odczyt nie zmieniał zawartości flash; po nim esptool wykonał reset sprzętowy.
- Dokumentacja i schemat V1 Waveshare potwierdzają natywne USB ESP32-S3: przewód komputerowy łączy jednocześnie VBUS oraz USB Serial/JTAG. Dlatego wcześniejsza obserwacja PC USB kontra powerbank zmienia jednocześnie zasilacz i obecność hosta USB. Przy aktualnie potwierdzonym obrazie CAN-off należy porównać, bez zmiany firmware i jasności, USB komputera bezpośrednio z tym samym portem przez adapter blokujący linie danych; to odróżni wpływ hosta/logowania od samej jakości 5 V.
## 8. Izolacja logów USB Serial/JTAG (7 października 2026)

- Dodano substytucję logger_baud_rate (domyślnie 115200) i parametr LoggerBaudRate w skryptach kompilacji oraz wgrywania; wartość 0 wyłącza wyjście logów ESPHome.
- Czysta walidacja i kompilacja wariantu V1 z CAN=false i loggerem USB 0 przeszły. Obraz aplikacji ma 603251 B, używa 19,07% DIRAM i 7,4% flash; wygenerowany kod ustawia logger::Logger(0) i obd_can->set_enabled(false). esp_wifi nadal jest wykluczone.
- Wariant wgrano przez COM6; ESPHome zgłosiło Hash of data verified i wykonało reset. Płytka została podłączona tylko przez USB, bez transceivera CAN.
- Ten krok izoluje wyjście logów USB; nie odłącza elektrycznie linii USB D+/D− ani hosta komputera. Ocena obrazu po wgraniu jest do potwierdzenia wzrokowo. Przy braku logów serialnych przywróć standardowy wariant poleceniem LoggerBaudRate 115200.
- Po przygotowaniu cichego testu skompilowano również wariant samochodowy V1 z CAN=true i loggerem 115200 (604859 B, 19,07% DIRAM, 7,4% flash); nie wgrywano go, więc płytka pozostała na wariancie testowym CAN-off/logger-off.

## 9. Test samego kontrolera CAN bez transceivera (7 października 2026)

- Na COM6 wgrano wariant V1 z `CAN=true`, `logger_baud_rate=0`, bez Wi-Fi/API/OTA. Build użył ESPHome 2026.9.1 i ESP-IDF 5.5.5; obraz `firmware.ota.bin` ma 604976 B, a zapis flash został potwierdzony przez `Hash of data verified`.
- Transceiver SN65HVD230 i samochód nie są podłączone; urządzenie jest połączone z komputerem przez USB. Test sprawdza wpływ samego uruchomienia TWAI przy braku magistrali i bez wyjścia logów przez USB.
- W `uds_audi.h` odpytywanie OBD pozostaje wstrzymane, dopóki nie zostanie odebranych kilka poprawnych ramek standardowych w krótkim oknie. Bez aktywności CAN nie powinny być wysyłane żądania ECU.
- Obserwacja wizualna po wgraniu oraz czas do ewentualnego szumu pozostają do potwierdzenia przez użytkownika. Nie uznawać tego testu za zaliczony bez takiej obserwacji.
- Dodatkowy odczyt PnP (`pnputil /enum-devices /class Ports /connected`) potwierdził instancję `USB\VID_303A&PID_1001&MI_00` jako COM6 w stanie Started. Nie wykryto procesu ESPHome ani typowego monitora szeregowego w tej sesji. Odczyt dziennika System przez `Get-WinEvent` został odrzucony z powodu braku uprawnień; nie ma więc danych historycznych o ewentualnych rozłączeniach/resetach podczas testu.
- Pasywny monitoring `pnputil` przez 65 próbek (około 5 minut) wykazał COM6 obecny w 65/65 próbkach; nie zaobserwowano odłączenia USB w tym oknie. Test nie mierzy obrazu ani napięcia zasilania.
- Przegląd lokalnego źródła ESP-IDF 5.5.5 `components/driver/twai/twai.c`: `twai_configure_gpio()` włącza wejście RX przez `gpio_input_enable()` i nie zmienia pull mode. Ustawienie `GPIO_PULLUP_ONLY` na GPIO6 przed `twai_driver_install_v2()` nie jest nadpisywane przez ten sterownik; sama kolejność pull-upu nie wyjaśnia artefaktów.
- Właściwość PnP `DEVPKEY_Device_LastArrivalDate` dla COM6 wskazywała 7.10.2026 14:48:02. Nie było późniejszej enumeracji USB do chwili odczytu. To wyklucza fizyczne odłączenie/przeenumerowanie USB w tym okresie, ale nie wyklucza resetu ESP32, który może zachować enumerację USB Serial/JTAG.
- Przegląd wygenerowanego kodu ESPHome: `Application::loop()` wykonuje komponenty w jednej sekwencyjnej fazie; `ESP32CanRecover::loop()` jest przed LVGL, a `LvglComponent::loop()` wywołuje `lv_timer_handler()`. Nie ma tu równoległego wywoływania LVGL przez callback CAN. Odczyt TWAI używa timeoutu 0, limituje paczkę do 16 ramek, a przy pustej kolejce nie tworzy wektorów; stan kontrolera sprawdzany jest co 250 ms. To osłabia hipotezę, że bezczynna pętla CAN sama zalewa pamięć lub ściga się z LVGL; ruchliwa magistrala może nadal wydłużać pojedynczą iterację.
- Sprawdzono mechanizm `show_snow_` w wygenerowanym komponencie LVGL: losowe piksele są rysowane tylko po jawnym `set_paused(true, true)`. W aktualnym YAML i wygenerowanym `main.cpp` nie ma wywołania `lvgl.pause`, `lvgl.resume` ani `set_paused`; ten celowy efekt śniegu nie jest skonfigurowany i nie wyjaśnia zgłaszanego obrazu.
- Korekta diagnostyki COM6: próba otwarcia portu do pasywnego nasłuchu przy DTR=0/RTS=0 zakończyła się `Access is denied`, a `tasklist` odmówił wyświetlenia procesów z powodu błędu uwierzytelniania. Nie da się więc potwierdzić, że COM6 jest wolny ani ustalić, który proces go trzyma; wcześniejszy filtr nazw procesów był niepełny.

## 10. Weryfikacja konfliktu starego i nowego sterownika TWAI (7 października 2026)

- Dokumentacja migracyjna ESP-IDF 5.5 ostrzega, że nie należy równocześnie używać starego API `driver/twai.h` i nowego `esp_driver_twai`; mieszanie obu może powodować ostrzeżenia, awarie lub restarty. [ESP-IDF 5.5: peryferia i migracja TWAI](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/migration-guides/release-5.x/5.5/peripherals.html)
- Lokalny komponent projektu wywołuje `twai_driver_install_v2` ze starego API. `driver/CMakeLists.txt` ESP-IDF 5.5.5 deklaruje `esp_driver_twai` jako publiczną zależność dla zgodności wstecznej, więc sama obecność biblioteki `libesp_driver_twai.a` w mapie linkowania nie dowodzi używania nowego sterownika.
- W mapie bieżącego obrazu `audi-dpf-monitor.elf` symbol `twai_driver_install_v2` pochodzi z `esp-idf/driver/libdriver.a(twai.c.obj)`. W mapie nie znaleziono `twai_new_node_onchip` ani obiektów `esp_twai.c.obj`/`esp_twai_onchip.c.obj`; nie ma zatem dowodu, że oba API są jednocześnie aktywne w aplikacji. To osłabia hipotezę o konflikcie sterowników jako przyczynie śnieżenia.
- Potwierdzono bieżący test sprzętowy: ESP32-S3 widnieje w Windows jako COM6, płytka jest podłączona tylko przez USB, transceiver nie jest podłączony. W tej sesji Codexa nie ma terminala z logami; odczyt COM6 przez dodatkowy listener nie jest dostępny. Obserwacja samego ekranu i napięcia 5 V pozostają poza zdalną weryfikacją.
- Bezpieczny kolejny krok pozostaje kontrolowany test A/B na tym samym porcie USB, kablu i jasności: aktualny firmware z CAN włączonym kontra ten sam YAML z `can_bus_enabled=false`, oba z wyjściem logów USB wyłączonym. Wynik obrazu należy odnotować wraz z czasem do ewentualnego śnieżenia. Nie uznawać braku zjawiska na jednym krótkim uruchomieniu za dowód stabilności wielogodzinnej.

## 11. Kontrola wszystkich ścieżek wysyłania CAN bez transceivera (7 października 2026)

- Przejrzano wszystkie wywołania `bus_->send_data()` w `uds_audi.h`. OBD-II rozpoczyna zapytanie dopiero po potwierdzeniu co najmniej trzech poprawnych ramek standardowych w oknie 1 s i tylko gdy aktywność była świeża w ostatnich 2 s (`loop_obd_probe_()`). Przy całkowicie nieaktywnej, podciągniętej linii RX żądania nie powinny się uruchomić.
- W źródle lokalnego ESP-IDF 5.5.5 (`components/driver/twai/twai.c`, `twai_configure_gpio()`) sprawdzono inicjalizację RX: driver włącza wejście i łączy macierz sygnałów, ale nie zmienia pull mode. Pull-up GPIO6 ustawiony przed `twai_driver_install_v2()` nie jest kasowany przez ten driver.
- UDS jest wyłączony dla bieżącego profilu (`PROFILE_MAPPING_VERIFIED = false`, `configured_ = false`). Pozostałe wysłania to ISO-TP Flow Control, wykonywane wyłącznie po odebraniu ramki ECU typu First Frame.
- Zatem brak transceivera/ECU sam w sobie nie powinien generować kolejnych żądań ani zapychać kolejki TX. Hipoteza pozostaje możliwa tylko, jeśli niepodłączone GPIO6 zbiera zakłócenia i TWAI dekoduje z nich poprawne ramki; przy obecnym wyłączonym loggerze nie ma telemetrii runtime, by to potwierdzić.
- Jeśli kolorowy śnieg pojawi się z tym obrazem, następny test powinien porównać CAN-on i CAN-off na tym samym komputerowym porcie USB, przewodzie i poziomie jasności. Wartość samego statusu COM6 nie pozwala wnioskować o stanie panelu ani o tym, czy aplikacja wykonuje reset.

## 12. Wariant diagnostyczny CAN listen-only (7 października 2026)

- Dodano podstawienie `can_bus_mode` oraz parametr `-CanBusMode NORMAL|LISTENONLY` do skryptów kompilacji i wgrywania. Domyślny tryb pozostał `NORMAL`; wariant nasłuchowy jest opcjonalny i jego artefakt ma osobny sufiks.
- Skrypty PowerShell przeszły kontrolę składni. ESPHome 2026.9.1 zwalidowało i skompilowało `CAN=true`, `LISTENONLY`, logger USB `0`. Wygenerowany `main.cpp` jawnie zawiera `CAN_MODE_LISTEN_ONLY`, `set_enabled(true)` i `Logger(0)`.
- Obraz `firmware/V1/audi-dpf-monitor-V1-can-listenonly-test-logger-off-test.ota.bin` (604976 B) wgrano na COM6; ESPHome/esptool potwierdziły `Hash of data verified`. Płytka jest podłączona tylko przez USB, bez transceivera CAN i bez auta.
- Użytkownik potwierdził czysty obraz po 5 minutach w tym trybie. Wynik pokazuje stabilność odbiornika CAN w trybie listen-only na USB bez transceivera; nie testuje nadawania ani rzeczywistego ruchu z auta.

## 13. Wariant kontrolny: TWAI NORMAL bez pollera OBD (7 października 2026)

- Dodano osobne podstawienie `obd_polling_enabled` i parametr skryptów `-ObdPollingEnabled true|false`; domyślnie pozostaje `true`. To umożliwia test aktywnego kontrolera bez generowania żądań aplikacji.
- Skrypty PowerShell przeszły kontrolę parsera. ESPHome 2026.9.1 zwalidowało i skompilowało wariant `CAN=true`, `NORMAL`, `OBD=false`, logger USB `0`. W `main.cpp` sprawdzono `CAN_MODE_NORMAL`, `set_enabled(true)` i warunek `true && false`, który pomija `uds_manager.loop()`.
- Obraz `firmware/V1/audi-dpf-monitor-V1-obd-poll-off-test-logger-off-test.ota.bin` (603360 B) wgrano na COM6; `Hash of data verified` potwierdziło zapis.
- Użytkownik potwierdził czysty obraz po 5 minutach w wariancie NORMAL + OBD-off. Wraz z czystym obrazem w LISTENONLY wskazuje to, że samo uruchomienie kontrolera w obu trybach nie odtworzyło śnieżenia.

## 14. Kompilacja artefaktu produkcyjnego i aktualny obraz testowy (7 października 2026)

- Bez zmiany obrazu na płytce skompilowano aktualny YAML z ustawieniami domyślnymi: `CAN=true`, `NORMAL`, poller OBD `true`, logger USB `115200`.
- Walidacja i kompilacja ESPHome 2026.9.1 zakończyły się powodzeniem. Artefakty rewizji V1 zostały odświeżone; obraz aplikacji ma 604859 B, używa 19,07% DIRAM i 7,4% flash.
- Produkcyjny artefakt z loggerem USB 115200 skompilowano, ale go nie wgrywano. Po tym buildzie skompilowano i wgrano diagnostyczny wariant `CAN=true`, `NORMAL`, `OBD=true`, logger USB `0`; weryfikacja hash flash przeszła. Użytkownik potwierdził czysty obraz po 5 minutach. Płytka nie miała transceivera ani auta, więc ten test nie obejmował ruchu ani transmisji CAN.
- Następnie wgrano test CAN=false, logger USB 115200, OBD polling false; kompilacja zakończyła się powodzeniem, a esptool potwierdził hash flash. Użytkownik zgłosił, że obraz pozostaje stabilny, ale bez dokładnego czasu obserwacji.
## 15. Kontrola fail-safe statusu regeneracji (7 października 2026)

- Przegląd źródła YAML potwierdza, że interfejs wylicza alarm jako `regenerating = data_available && dpf_regen`. Sam zapamiętany stan `dpf_regen` nie wystarcza do pokazania `NIE GAŚ!`.
- `regen_data_available` może być prawdziwe tylko dla świeżego DID-u 0xF401 albo świeżego PID-u OBD 0x8B jawnie zgłoszonego jako obsługiwany. Wartości nieaktualne, nieobsługiwane albo niezweryfikowane prowadzą do `BRAK DANYCH`.
- To jest kontrola statyczna kodu. PID 0x8B, skala i bit regeneracji nie zostały jeszcze porównane z ECU CJCD/OBDeleven, więc wiarygodność alarmu pozostaje niepotwierdzona na aucie.

## 16. Porównanie CAN + OBD z loggerem USB aktywnym (7 października 2026)

- Na COM6 wgrano wariant V1: `CAN=true`, `NORMAL`, odpytywanie OBD `true`, logger USB `115200`. ESPHome 2026.9.1 poprawnie zwalidowało i skompilowało konfigurację; obraz ma 604859 B. Esptool potwierdził zapis przez `Hash of data verified`.
- Płytka jest podłączona do komputera przez USB; transceiver CAN i samochód nie są podłączone. Poller OBD rozpocznie wysyłanie dopiero po wykryciu co najmniej trzech standardowych ramek CAN w ciągu 1 s, więc ten test nie obejmuje rzeczywistej transmisji ani obciążenia ruchem z auta.
- Test porównuje aktywny kontroler CAN, poller OBD i logger USB 115200 z wcześniejszym wariantem CAN-off/logger-on. Użytkownik został poproszony o sprawdzenie ekranu po 5 minutach; wynik wizualny oczekuje na potwierdzenie.

## 17. TWAI self-test z kontrolowanym ruchem RX (7 października 2026)

- Dodano tryb diagnostyczny `SELFTEST`, pozostawiając produkcyjny `NORMAL` bez zmian. Wariant używa `TWAI_MODE_NO_ACK`, samoodbioru ramek i wspólnego GPIO5 dla TX/RX; co 5 ms nadaje ramkę testową `0x7E8`. OBD polling jest wyłączony.
- Wygenerowany kod sprawdzono przed wgraniem: tryb `SELFTEST`, RX mapowane na GPIO5, ustawiany bit self-reception, poller OBD pominięty. Transceiver CAN i auto pozostają odłączone.
- ESPHome 2026.9.1 poprawnie zwalidowało i skompilowało wariant. Obraz ma 603847 B, używa 19,09% DIRAM i 7,4% flash. Wystąpiły dwa istniejące ostrzeżenia kompilatora `-Wempty-body` w komponencie FT3168; build zakończył się poprawnie.
- Wariant wgrano na COM6; esptool potwierdził `Hash of data verified`. Następnie odczytano logi przez około 15 s i zamknięto port COM6. Trzy kolejne raporty pokazały po 1020 ramek RX/5 s, kolejki RX/TX równe 0, `RX missed=0` i `bus errors=0`.
- Po około 5 minutach pracy pobrano kolejny krótki wycinek logów: raporty pokazały 980, 1002 i 1006 ramek RX/5 s, kolejki RX/TX równe 0, `RX missed=0` i `bus errors=0`. W tym wycinku nie pojawił się komunikat restartu, panic ani błędu LVGL. Port COM6 zamknięto po około 15 s.

- Ocena wizualna ekranu i dotyku po co najmniej 5 minutach samoodbioru nadal oczekuje na potwierdzenie użytkownika. Nie podłączaj transceivera ani auta, dopóki na płytce działa `SELFTEST`.

## 18. Test obciążeniowy TWAI i przywrócenie wariantu NORMAL (7 października 2026)

- Dodano parametr `CanSelfTestInterval` do skryptów kompilacji/wgrywania; dopuszczalne wartości to `1ms` i `5ms`, domyślnie `5ms`. Skrypty zapisano jako UTF-8 z BOM, a Windows PowerShell 5.1 poprawnie przeszedł parser.
- Na COM6 skompilowano i wgrano wariant diagnostyczny V1: `CAN=true`, `SELFTEST`, interwał `1ms`, OBD polling `false`, logger USB `115200`. Walidacja ESPHome 2026.9.1, kompilacja i weryfikacja zapisu flash zakończyły się powodzeniem. Transceiver CAN i auto były odłączone.
- Autotest generował około 1000 ramek/s. Logi pokazywały ok. 4800 odebranych ramek/5 s, `queued RX=48`, `queued TX=0`, brak błędów magistrali oraz narastający licznik RX missed (od 2333 do 5556 w obserwowanym oknie). Wystąpiło ostrzeżenie LVGL o operacji 172 ms przy limicie 130 ms.
- Użytkownik potwierdził czysty obraz i działający dotyk podczas autotestu 1 ms. Duże tempo sztucznego ruchu obciąża odbiór i opóźnia pojedyncze operacje LVGL, ale w tej próbie nie odtworzyło kolorowego śniegu.
- Następnie przywrócono na COM6 wariant V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger USB `115200`; kompilacja oraz `Hash of data verified` zakończyły się powodzeniem. Użytkownik potwierdził czysty obraz i działający dotyk po przywróceniu NORMAL.
- Wyniki nie wskazują, że typowy ruch CAN/OBD powoduje śnieżenie. Test nie obejmował rzeczywistego ruchu z ECU; do weryfikacji pozostały odpowiedzi ECU, mapa DID/PID CJCD i porównanie regeneracji z OBDeleven.
## 19. Doprecyzowanie alarmu PID 0x8B (7 października 2026)

- W SAE J1979-DA PID `0x8B` rozdziela status regeneracji od jej typu: bajt A bit 0 deklaruje obsługę statusu, A bit 1 obsługę typu; bajt B bit 0 oznacza regenerację w toku, a B bit 1 odróżnia aktywną (1) od pasywnej (0). Źródło: [SAE J1979-DA, tabela PID $8B](https://www.e90post.com/forums/attachment.php?attachmentid=1509324&d=1476497126), wydanie Revised OCT2011.
- Poprawiono dekoder: alarm aktywnej regeneracji wymaga świeżego PID-u oraz jawnej obsługi obu pól. Wartość „NIE GAŚ!” jest wyliczana wyłącznie, gdy B0=1 i B1=1. Przy B0=1/B1=0 interfejs pokazuje „REGEN PASYWNA” bez migającego alertu. Gdy typ nie jest obsługiwany, stan jest „BRAK DANYCH”, nie „DPF OK”.
- Tekst diagnostyczny PID-u rozróżnia teraz „type unsupported”. Niezmieniony profil CJCD UDS pozostaje wyłączony, dopóki DID, format, skala i znaczenie nie zostaną potwierdzone. Szerokość pola statusu zwiększono do 204 px, aby mieściło „REGEN PASYWNA”.
- ESPHome 2026.9.1 zwalidowało i skompilowało wariant NORMAL z OBD; obraz aplikacji ma 606055 B. Firmware wgrano na COM6, a `Hash of data verified` potwierdził zapis. Użytkownik potwierdził po ostatnim wgraniu czysty obraz i działający dotyk.
## 20. Cykliczna diagnostyka CAN w NORMAL (7 października 2026)

- Do komponentu `esp32_can_recover` dodano raport stanu TWAI co 5 s w trybie NORMAL: stan kontrolera, kolejki RX/TX, pominięte ramki, nieudane TX, liczniki błędów RX/TX i błędy magistrali. Zmiana jest tylko obserwowalnością; nie zmienia parametrów odpytywania ani UI.
- ESPHome 2026.9.1 zwalidowało i skompilowało wariant V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger USB `115200`. Obraz aplikacji ma 606247 B. Firmware wgrano przez COM6; `Hash of data verified` potwierdził zapis. Płytka zgłosiła ESP32-S3 revision v0.2 i 8 MB PSRAM.
- Odczyt logów przez COM6 trwał około minuty. W każdym raporcie TWAI był w stanie RUNNING; kolejki RX/TX, RX missed, TX failed, błędy RX/TX i bus errors pozostały równe 0. W trakcie tego testu transceiver i auto były odłączone, więc potwierdzono jedynie stabilną pracę na bezczynnej magistrali. Dodatkowy odczyt przez około 2 min 28 s również wykazał wyłącznie zera w tych licznikach, przy stanie RUNNING.
- UI i dotyk wymagałyby ponownego potwierdzenia po tym wgraniu; firmware zawiera tę samą konfigurację UI co poprzednio, a jedyną zmianą jest cykliczny log diagnostyczny.
- Rozszerzono raport NORMAL o uptime, wolny heap, największy ciągły blok heap, wolną PSRAM i największy blok PSRAM. Firmware skompilowano i wgrano przez COM6; obraz aplikacji ma 606403 B, zapis zweryfikowano hashem.
- Po wgraniu użytkownik ponownie potwierdził czysty obraz i działający dotyk. Odczyt logów trwał 149 s: uptime wzrósł bez restartu, heap pozostał 304752 B, największy blok heap 253952 B; wolna PSRAM zmieniła się z 8302044 do 8302052 B, największy blok PSRAM pozostał 8257536 B. Wszystkie liczniki CAN pozostały zerowe. Wskazuje to, że na bezczynnej magistrali nie ma narastającego zużycia RAM/PSRAM ani błędów CAN.- Dodatkowy test bez restartu trwał 6 min 14 s (uptime wzrósł z 67527 do 441978 ms). Wszystkie raporty nadal pokazywały stan RUNNING, puste kolejki, zerowe błędy i stałą pamięć heap/PSRAM. Obserwacja wizualna użytkownika z całego tego okna oczekuje na potwierdzenie.
- Dodano do raportu SELFTEST uptime, liczniki RX/TX i heap/PSRAM, by porównać zwykły tryb z przeciążeniowym autotestem.
- SELFTEST 1 ms (transceiver/auto odłączone, OBD polling wyłączony) trwał około 108 s. Generował początkowo 4800 ramek RX/5 s; kolejka RX osiągnęła limit 48, `RX missed` wzrósł do 5802, natomiast błędy magistrali i TX pozostały 0. LVGL zgłosił pojedynczą operację 171 ms po około 57 s. Heap po pierwszym wzroście obciążenia spadł z 304736 do 304472 B i pozostał stały; PSRAM zmieniła się nieznacznie. Użytkownik potwierdził czysty obraz i działający dotyk podczas tego testu. Śnieżenia nie odtworzono, choć przeciążenie CAN wywołało zauważalne opóźnienie UI.
- Po teście wgrano wariant NORMAL z OBD; zapis flash zweryfikowano hashem. Przez około 31 s logi wróciły do stanu RUNNING z pustymi kolejkami i zerowymi licznikami CAN, a pamięć wróciła do wartości bazowej.
- Kolejny test diagnostyczny powinien sprawdzić logi na rzeczywistym ruchu CAN. Rosnące `RX missed` lub `queued RX` wskazywałyby na przeciążenie odbioru; błędy magistrali/TX wskazywałyby raczej na warstwę fizyczną, bitrate albo brak ACK.
## 21. Większy budżet obsługi RX i obserwacja UI (7 października 2026)

- Zmieniono limit pracy własnego sterownika z 16 na 32 odebrane ramki w jednym obiegu ESPHome. Cel: opróżniać kolejkę szybciej przy zachowaniu ograniczonej porcji, która pozwala wrócić do obsługi LVGL.
- SELFTEST 1 ms, OBD polling wyłączony, transceiver i auto odłączone: skompilowano i wgrano firmware na COM6, zapis flash zweryfikowano hashem. Przez około 82 s odbierano ok. 5098 ramek/5 s; kolejka RX/TX, RX missed i liczniki błędów pozostały zerowe. Heap i PSRAM były stałe. Odpowiedź użytkownika o obrazie podczas tej konkretnej wersji oczekuje na potwierdzenie.
- Po teście przywrócono firmware NORMAL z CAN i OBD włączonymi; ponowna kompilacja, flash i hash zakończyły się pomyślnie. Około 6 min 20 s logów na USB, bez transceivera i auta: brak restartu, wszystkie liczniki CAN pozostały zerowe. Heap wahał się w przybliżeniu 304468–304752 B, wolna PSRAM ok. 8301912–8302024 B. Wystąpiło jedno ostrzeżenie LVGL o operacji trwającej 172 ms przy limicie 130 ms.
- Ustalenia: zwiększenie budżetu usuwa kolejkę i zgubione ramki w sztucznym ruchu ok. 1000 ramek/s; zwykły tryb bez ruchu CAN nie wykazuje narastającego błędu magistrali. Samo śnieżenie nie zostało odtworzone ani wyjaśnione. Pojedyncze długie wywołanie LVGL wystąpiło również w normalnym trybie bez odbioru CAN, więc opóźnienie renderowania nie jest dowodem, że CAN powoduje artefakty. Brakuje jeszcze wizualnego potwierdzenia z bieżącej obserwacji i testu na prawdziwym ruchu z auta.
## 22. Ograniczenie zbędnych aktualizacji LVGL (7 października 2026)

- W `audi-dpf-monitor.yaml` dodano filtry `delta` do 12 sensorów widocznych w UI: progi odpowiadają precyzji wyświetlania (m.in. 0.1 g, 1°C, 1 kPa i 0.01 kPa). UDS nadal odświeża własne znaczniki czasu odpowiedzi niezależnie od filtrowania callbacku UI.
- ESPHome 2026.9.1 zwalidowało i skompilowało konfigurację V1 NORMAL z CAN/OBD włączonymi; obraz aplikacji 607527 B. Wgrano na COM6 i zweryfikowano zapis `Hash of data verified`.
- Użytkownik potwierdził czysty ekran i działający dotyk po wgraniu oraz poprawne przejście przez DPF/TEMP/FILTR/ECU bez artefaktów. Po pełnych 6 minutach użytkownik potwierdził stabilny obraz.
- Logi na USB trwały ponad 6 min (uptime od 11115 do 385360 ms): bez restartu, CAN RUNNING, kolejki RX/TX i liczniki błędów równe 0. Heap ustabilizował się na 304092 B, PSRAM ok. 8302008 B. Ostrzeżenie LVGL o 172 ms wystąpiło raz, około 41 s po starcie; później się nie powtórzyło.
- Test odbył się bez transceivera i samochodu, więc nie weryfikuje ruchu ECU ani odpowiedzi na aktualizacje wartości z auta. Przyczyna wcześniejszego śnieżenia nadal nie jest udowodniona; wariant ma mniej powtarzalnych invalidacji UI i pozostaje na płytce do dalszej obserwacji.

## 23. Karta historii regeneracji (7 października 2026)

- Dodano piątą kartę `HIST` z trwałym licznikiem zakończonych cykli i czasem ostatniego cyklu obserwowanego przez monitor. Nawigacja ma teraz pięć przycisków 80 × 52 px; czcionka nawigacji pozostała bez zmian.
- Historia wymaga świeżego, jawnie obsługiwanego statusu ECU. Po starcie lub utracie danych czeka na potwierdzony stan nieaktywny, a następnie zapisuje cykl dopiero po zaobserwowaniu początku i końca regeneracji. Cykle rozpoczęte przed uruchomieniem monitora, przerwane utratą danych i symulowane przez DEMO są pomijane. Zapis trwały następuje tylko po zakończonym cyklu.
- ESPHome 2026.9.1 zwalidowało i skompilowało wariant V1 `CAN=true`, `NORMAL`, OBD polling `true`. Obraz aplikacji ma 615707 B; pamięć RAM 66383 B (19,4%). Firmware wgrano przez COM6, `Hash of data verified` potwierdził zapis.
- Logi po starcie: CAN RUNNING, kolejki RX/TX oraz błędy CAN równe 0, heap 303584 B, PSRAM 8296740 B. Nie podłączono transceivera ani auta. Użytkownik nie potwierdził jeszcze wizualnie nowej karty; poprawność statusów i czasów wymaga testu z ECU i porównania z OBDeleven.

## 24. Ograniczenie niepotrzebnych odrysowań karty HIST (7 października 2026)

- Etykiety liczby cykli, czasu ostatniej regeneracji i statusu są teraz aktualizowane przez LVGL wyłącznie wtedy, gdy ich treść się zmienia. Poprzednio ta sama treść była ustawiana ponownie co sekundę.
- ESPHome 2026.9.1 poprawnie zwalidowało i skompilowało wariant V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger USB `115200`. Obraz aplikacji ma 616107 B, zajmuje 19,45% DIRAM i 7,6% dostępnej partycji aplikacji.
- Firmware wgrano na COM6; ESP32-S3 zgłosił revision v0.2, a esptool potwierdził `Hash of data verified`. Po uruchomieniu logi przez około 30 s pokazywały rosnący uptime, CAN RUNNING, puste kolejki RX/TX oraz 0 błędów i utraconych ramek. Heap 303448 B, PSRAM 8296740 B.
- Test wykonano po USB, bez transceivera i auta. Potwierdza uruchomienie i bezbłędną pracę jałowego CAN w tym krótkim oknie; nie stanowi wielogodzinnego testu stabilności, wizualnego potwierdzenia nowej karty ani walidacji odczytów ECU.

## 25. Pięciominutowa obserwacja pracy po aktualizacji HIST (7 października 2026)

- Po wgraniu nowego obrazu odczytano logi COM6; uptime zwiększył się od ok. 77 s do ok. 309 s bez restartu.
- Przez cały widoczny okres CAN pozostał RUNNING, RX/TX i kolejki były zerowe, nie narastały błędy ani utracone ramki. Heap pozostał na poziomie 303448 B, PSRAM 8296740 B.
- Raport dotyku co 60 s pokazał kilka ponowień odczytu statusu, ale `unrecovered=0`, `slow(>=20ms)=0` i maksymalnie 1 ms obsługi. Logger USB został zamknięty po teście.
- To potwierdza krótką stabilną pracę po aktualizacji na biurku bez transceivera. Nie ocenia obrazu panelu, nie zastępuje dłuższego testu ani sprawdzenia na aucie.

## 26. Ponowna obserwacja jałowego CAN po aktualizacji (7 października 2026)

- Ponownie otwarto tylko-odczytowy logger na COM6 przy firmware V1 `CAN=true`, `NORMAL`, OBD polling `true`. Widziany uptime wzrósł z ok. 77 s do ok. 309 s; nie było restartu.
- W raportach co 5 s: RX=0, batch=0, kolejki RX/TX=0, RX missed=0, TX failed=0, RX/TX errors=0, bus errors=0. Heap utrzymywał się na poziomie 303448 B, PSRAM 8296740 B.
- Raport FT3168 co 60 s odnotował 4–5 odzyskanych ponowień statusu, zero błędów nieodzyskanych, zero transakcji >=20 ms oraz maksymalnie 1 ms. Dotyk działał wcześniej; w trakcie tego odczytu nie wykonano celowego testu tapnięć.
- Transceiver CAN i auto były odłączone. To jest kolejny krótki test pracy jałowej, nie potwierdzenie zachowania przy ruchu CAN ani wizualny test ekranu.

## 27. Ograniczenie odrysowań temperatury i ponowne wgranie (7 października 2026)

- W głównej pętli odświeżania temperatura DPF z OBD/stan niedostępny jest teraz porównywana z ostatnią widoczną wartością. Pasek i etykieta zmieniają się tylko przy zmianie zaokrąglonej temperatury albo przejściu świeżość/brak danych. Świeży DID UDS nadal aktualizuje widok przez dotychczasowy sensor.
- ESPHome 2026.9.1 poprawnie zwalidowało i skompilowało V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger USB `115200`; aplikacja ma 616171 B, wykorzystuje 19,46% DIRAM i 7,6% partycji aplikacji.
- Nowy obraz wgrano na COM6; `Hash of data verified` potwierdził zapis. Przez ok. 26 s uptime rósł od 6 s do 26 s bez restartu. CAN RUNNING, kolejki RX/TX i liczniki błędów pozostały zerowe. Heap po starcie 303328–303408 B, PSRAM 8296820 B.
- Krótki log potwierdza start i stan jałowej magistrali, ale nie dowodzi widocznej poprawy płynności ani stabilności wielogodzinnej. Do oceny obrazu i karty HIST potrzebne jest obejrzenie płytki.
- Dodatkowy tylko-odczytowy odczyt loggera trwał ok. 2 min (uptime 62435–180415 ms) bez restartu. CAN pozostawał RUNNING; RX/TX, kolejki, błędy i liczniki utraconych ramek były zerowe. Heap stabilny 303408 B, PSRAM 8296820 B. FT3168: 4–5 ponowień statusu/60 s, unrecovered=0, slow=0, max=1 ms.

## 28. Alarm regeneracji na karcie HIST (7 października 2026)

- Kod alarmu regeneracji obejmuje teraz także piątą stronę HIST. Widok ma lokalny element nagłówka `regen_alert_history`, który jest pokazywany i miga naprzemiennie na czerwono/bursztynowo razem z alarmami DPF, TEMP, FILTR i ECU. Po wyjściu z aktywnej regeneracji wszystkie pięć elementów jest ukrywanych.
- ESPHome 2026.9.1 poprawnie zwalidowało i skompilowało wariant V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger USB `115200`; obraz aplikacji ma 616375 B, DIRAM 66527 B (19,5%).
- Firmware wgrano na COM6; `Hash of data verified`. Log startowy przez ok. 26 s pokazał rosnący uptime, CAN RUNNING, kolejki i liczniki błędów zerowe. Heap 303320–303400 B, PSRAM 8296556 B.
- Logika wszystkich pięciu etykiet jest sprawdzona w skompilowanym kodzie; fizyczne potwierdzenie obrazu podczas DEMO/aktywnej regeneracji wymaga obserwacji ekranu.

## 29. Jednoznaczny tekst alarmu na wszystkich kartach (7 października 2026)

- Wszystkie pięć nagłówków regeneracji pokazuje teraz `REGENERACJA DPF — NIE GAŚ!`, aby instrukcja dla kierowcy była widoczna także poza kartą DPF. Tekst zachowuje nazwę stanu, którą wcześniej wskazał użytkownik.
- Pomiar fontu Ubuntu przy 18 px dał szerokość 256 px; pole alarmowe ma 456 px. To potwierdza, że tekst mieści się w belce według metryk czcionki, ale nie zastępuje oceny czytelności na fizycznym ekranie.
- ESPHome 2026.9.1 zwalidowało i skompilowało wariant V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger USB `115200`. Obraz aplikacji ma 616375 B, DIRAM 66527 B (19,5%).
- Firmware wgrano na COM6 i potwierdzono `Hash of data verified`. Log startowy przez ok. 26 s pokazał rosnący uptime, CAN RUNNING, zerowe błędy/kolejki; heap 303320–303400 B, PSRAM 8296404 B.
- Obserwacja fizycznej belki z tym dłuższym tekstem nadal oczekuje; auto i transceiver nie były podłączone.

## 30. Uściślenie dekodowania PID 0x8B

- Przegląd `uds_audi.h` potwierdził, że ścieżka OBD rozdziela dostępność statusu (A0) od dostępności typu (A1). Stan „active” wymaga świeżej odpowiedzi, A0=A1=1 oraz B0=B1=1; B0=1/B1=0 jest klasyfikowane jako regeneracja pasywna. Przy braku obsługi któregoś pola lub nieświeżej odpowiedzi interfejs nie może pokazać „DPF OK” ani „NIE GAŚ!”.
- Poprawiono `DPF_PROFILE_OBDELEVEN_CAPTURE.md`, `DPF_PROFILES.md` i README, aby instrukcje nie opisywały starszej hipotezy A0/B0 i wyraźnie odróżniały dekodowanie według SAE J1979-DA Oct 2011 od weryfikacji w ECU `03L906018SJ / SW 9977 / ODX 003003`.
- Katalog SAE wskazuje rewizję J1979-DA `202607` jako bieżącą, lecz jej treść nie była dostępna do porównania. Zgodność bitów z nowszą rewizją i implementacja w docelowym ECU pozostają otwarte.
- Nie zmieniano firmware ani nie wgrywano nowego obrazu: ta praca uściśla wymagania i dokumentację. Potwierdzenie zachowania podczas rzeczywistej regeneracji nadal wymaga równoległej obserwacji monitora i OBDeleven.

## 31. Standardowe temperatury EGT z PID 0x78

- Dodano do pollera OBD standardowy PID `0x78`, który zawiera cztery kanały EGT Bank 1. Dekoder sprawdza maskę obsługi A0–A3 i publikuje kanał tylko po świeżej odpowiedzi, z przeliczeniem `raw / 10 - 40 °C`.
- Karta TEMP pokazuje kanały `EGT 1`–`EGT 3` oraz osobne pole temperatury wlotowej DPF z PID `0x7C`; nie przypisuje numerów EGT do położenia przed turbiną ani przed DPF. Kanał EGT 4 jest dekodowany, lecz nie jest wyświetlany. Demo zasila widoczne pola wartościami symulowanymi.
- SAE J1979-DA Oct 2011, tabela B100, potwierdza kolejność bitów A0–A3 oraz bajtów B–I. Nie potwierdza to obsługi PID-u ani lokalizacji czujników w ECU `03L906018SJ / 9977 / ODX 003003`; należy porównać je z OBDeleven.
- ESPHome 2026.9.1 poprawnie zwalidowało i skompilowało wariant V1 `CAN=true`, `NORMAL`, OBD polling `true`; rozmiar aplikacji 618663 B, DIRAM 66951 B (19,6%). Firmware nie został w tej iteracji wgrany, bo COM6 był niedostępny z tej sesji; wizualny test nowej karty TEMP na płytce pozostaje do wykonania.

## 32. Diagnostyka obsługi PID-ów na karcie ECU (7 października 2026)

- Karta ECU wyświetla teraz status PID-ów `0x7A` (różnica ciśnień DPF), `0x7C` (temperatura DPF), `0x78` (EGT) i `0x8B` (status regeneracji). Stany rozróżniają oczekiwanie, świeże dane obsługiwane, jawny brak obsługi, odpowiedź bez rozpoznanej maski, nieaktualne dane i nieobsługiwany typ regeneracji.
- DEMO opisuje podsumowanie jako `PID-y auta: DEMO`, aby nie przedstawiać symulacji jako odpowiedzi sterownika. Brak odpowiedzi nie jest klasyfikowany jako brak obsługi.
- ESPHome 2026.9.1 zwalidowało i skompilowało wariant V1 `CAN=true`, `NORMAL`, OBD polling `true`; obraz aplikacji ma 619751 B, DIRAM 66975 B (19,6%).
- Firmware wgrano na COM6; ESP32-S3 zgłosił revision v0.2, flash zweryfikowano komunikatem `Hash of data verified`. W krótkim odczycie logów po starcie przez ok. 37 s uptime rósł; CAN był `RUNNING`, RX/TX queue, utracone ramki i liczniki błędów pozostały zerowe; heap 302896 B, PSRAM 8294172 B.
- Płytka była podłączona tylko przez USB, bez transceivera i samochodu. Test potwierdza build, zapis i stabilny start w tym krótkim oknie, ale nie wizualne rozmieszczenie nowego wiersza ani obsługę PID-ów przez ECU.
- Dodatkowy odczyt logów utrzymano do ok. 5 min uptime. Uptime wzrósł z 103 s do 303 s bez restartu; heap pozostał 302896 B, PSRAM 8294172 B, CAN `RUNNING`, kolejki i błędy równe 0. FT3168 zgłaszał 4–5 ponowień odczytu statusu na 60 s, zero nieudanych odzysków, zero odczytów >=20 ms i maks. 1 ms. To test stabilności runtime, nie oględziny ekranu.

## 33. Różnica ciśnień DPF na karcie FILTR (7 października 2026)

- Wykorzystano istniejący dekoder PID `0x7A` i dodano na karcie FILTR pole różnicy ciśnień DPF. Wartość pokazuje się tylko dla świeżej odpowiedzi z bitem obsługi; po utracie świeżości ekran wraca do `--.-- kPa`.
- Tryb DEMO pokazuje syntetyczne 0,85 kPa podczas fazy regeneracji i 0,22 kPa podczas chłodzenia. Plakietka DEMO odróżnia te przykładowe liczby od odczytu ECU.
- ESPHome 2026.9.1 poprawnie zwalidowało i skompilowało V1 `CAN=true`, `NORMAL`, OBD polling `true`; aplikacja ma 620347 B, DIRAM 67023 B (19,6%).
- Firmware wgrano na COM6, a `Hash of data verified` potwierdził zapis. Krótki log startowy pokazał rosnący uptime 6–21 s, CAN `RUNNING`, puste kolejki i zero błędów; heap 302768–302848 B, PSRAM 8293488 B.
- Płytka nadal była podłączona tylko przez USB. Weryfikacja wizualna nowego pola w DEMO oraz rzeczywistej jednostki/skali PID `0x7A` w ECU CJCD pozostają otwarte.

## 34. Trzy poziomy jasności ekranu (7 października 2026)

- Karta ECU ma przycisk `JASNOŚĆ`, który przełącza wartości sterownika AMOLED 220 → 130 → 60 → 220. Globalna ESPHome ma `restore_value: yes`, więc wybór jest zapamiętywany w preferencjach i przywracany po restarcie. Flash synchronizuje preferencje domyślnie co 60 s; odłączenie zasilania bezpośrednio po zmianie może cofnąć ostatni wybór. Ustawienie fabryczne nowych urządzeń pozostaje 220.
- Po zmianie wartości polecenie jasności do CO5300 jest wysyłane tylko raz; pętla UI nie wysyła go w każdym cyklu. Zmiana nie dotyka sterownika CAN ani odczytów OBD.
- ESPHome 2026.9.1 poprawnie skompilowało V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger USB 115200. Obraz aplikacji: 621479 B; DIRAM: 67143 B (19,65%).
- Firmware wgrano przez COM6; esptool potwierdził `Hash of data verified`. Log startowy przez ok. 10 s pokazał CAN `RUNNING`, zerowe kolejki i błędy, heap 302652 B i PSRAM 8293208 B.
- Weryfikacja logu nie potwierdza fizycznego wyglądu przycisku ani skuteczności trzech poziomów jasności; do sprawdzenia wzrokowego pozostaje karta ECU i reakcja ekranu po dotknięciu `JASNOŚĆ`.

## 35. Stabilność po zmianie jasności (8 października 2026)

- Po wgraniu wariantu z wyborem jasności uruchomiono tylko-odczytowy logger USB na COM6 przez ok. 5 min 20 s. Uptime wzrósł z ok. 104 s do 422 s bez restartu.
- Heap pozostał stały na poziomie 302652 B, wolna PSRAM 8293208 B. CAN był `RUNNING`; przy płytce zasilanej z USB, bez auta i transceivera, RX/TX, kolejki, zgubione ramki i błędy pozostały równe 0.
- FT3168 raportował 4–5 ponowień odczytu statusu na 60 s; odzyskanie nieudane: 0, odczyty ≥20 ms: 0, maksymalny czas operacji: 1 ms.
- To potwierdza jedynie stabilny runtime w krótkim teście biurkowym. Nie sprawdza wzrokowo działania przycisku jasności, obrazu przez wiele godzin, rzeczywistego ruchu CAN ani danych ECU.

## 36. Dopasowanie ISO-TP First Frame do bieżącego żądania (8 października 2026)

- Przegląd parsera wykazał, że po odebraniu ISO-TP First Frame firmware wysyłało Flow Control przed sprawdzeniem, czy początkowe bajty odpowiedzi zawierają oczekiwaną usługę i PID/InfoType. Dodano walidację nagłówka `41 <PID>` lub `49 <InfoType>` przed kontynuowaniem odbioru. Obca albo spóźniona odpowiedź nie rozpoczyna teraz reassemblacji ani nie wywołuje Flow Control.
- ESPHome 2026.9.1 skompilowało V1 z CAN włączonym, trybem NORMAL, OBD pollingiem i loggerem 115200. Obraz aplikacji ma 621539 B, DIRAM 67143 B (19,65%).
- Firmware wgrano na COM6; esptool zgłosił `Hash of data verified`. Krótki log startowy przez ok. 5 s pokazał uptime rosnący z 26 do 31 s, CAN `RUNNING`, kolejki i błędy równe 0, heap 302652 B i PSRAM 8293368 B.
- Transceiver i auto były odłączone. Test potwierdza kompilację, zapis flash i start, ale nie wykonuje rzeczywistego testu wieloramkowej odpowiedzi ECU; do tego potrzebne jest auto z OBD albo kontrolowane źródło CAN.

## 37. Kompilacyjne zabezpieczenie dopasowania odpowiedzi OBD (8 października 2026)

- Przeniesiono czyste sprawdzenie prefiksu odpowiedzi OBD do funkcji `constexpr` i dodano asercje kompilatora: poprawne `41 <PID>` oraz `49 <InfoType>` są akceptowane, a obcy PID, obca usługa i prefiks krótszy niż dwa bajty są odrzucane.
- Wykorzystano tę samą funkcję w walidacji ISO-TP First Frame, więc warunek sprawdzany kompilacyjnie odpowiada warunkowi używanemu przez runtime.
- OBD i UDS odrzucają teraz Single Frame deklarujący więcej niż 7 bajtów danych, zgodnie z limitem klasycznej ramki CAN używanej przez tę konfigurację. Nadal sprawdzana jest także długość faktycznie odebranej ramki.
- ESPHome 2026.9.1 poprawnie skompilowało firmware V1 (`CAN=true`, `NORMAL`, OBD polling `true`). Rozmiar aplikacji 621543 B, DIRAM 67143 B (19,65%). Ponowna kompilacja po kontroli Single Frame również przeszła.
- Kompilacja początkowo ujawniła ograniczenie C++: asercje wewnątrz niedokończonej definicji klasy nie mogły wywołać jej funkcji `constexpr`. Przeniesienie helpera i asercji na poziom pliku rozwiązało problem. Kolejna kompilacja zakończyła się powodzeniem.
- Obraz fabryczny wgrano na COM6; `Hash of data verified`. Przy ręcznym `esphome upload --file` podany plik jest zapisywany od offsetu 0, dlatego do USB należy wskazywać `firmware.factory.bin` z bootloaderem i tablicą partycji, nie sam obraz OTA. Po przypadkowym pierwszym wskazaniu obrazu OTA natychmiast wgrano pełny factory image; weryfikacja hash i reset zakończyły się poprawnie.
- Log po starcie przez ok. 109 s: uptime rósł bez restartu, heap stabilny 302664 B, wolna PSRAM 8293368 B, kolejki RX/TX, zgubione ramki i błędy CAN równe 0. FT3168 raportował 5 odzyskanych prób statusu I²C/60 s, bez nieudanych odzysków ani wolnych operacji (maks. 1 ms).
- Płytka nie miała podłączonego transceivera ani auta. To potwierdza zapis i krótki start przy bezczynnej magistrali, ale nie zastępuje testu parsera na wieloramkowej odpowiedzi ECU ani porównania z OBDeleven.
- Zabezpieczono skrypt `Wgraj-monitor-DPF.ps1`: po kompilacji sprawdza istnienie `firmware.factory.bin` i przekazuje ten konkretny plik do ESPHome, zamiast polegać na domyślnym wyborze obrazu. Pełna komenda skryptu przeszła na COM6; zapis potwierdził hash.
- Ponowny log po flashowaniu: uptime wzrósł przez ok. 42 s; heap utrzymał się na 302664 B, PSRAM na 8293360 B, kolejki CAN i wszystkie błędy pozostały zerowe. To krótka weryfikacja startu i skryptu, nadal bez testu wizualnego ani auta.
- Ponowna kontrola katalogu SAE wskazuje `J1979DA_202607` jako bieżącą rewizję (zweryfikowaną 16 lipca 2026). Publiczna strona katalogowa nie udostępnia treści tabeli PID `0x8B`, dlatego nie zmieniano bitowego dekodera na podstawie niepełnych danych. Zgodność PID-u z ECU `03L906018SJ / 9977 / ODX 003003` nadal wymaga porównania z OBDeleven.
- Dodano `constexpr classic_can_sf_length_valid()` używaną w ścieżkach OBD i UDS oraz asercje kompilacyjne dla długości 1 i 7, zera, długości 8 i danych wykraczających poza ramkę. ESPHome ponownie skompilowało V1 (`CAN=true`, `NORMAL`, OBD polling `true`): aplikacja 621567 B, DIRAM 67143 B (19,65%).
- Poprawionym `Wgraj-monitor-DPF.ps1` wgrano obraz fabryczny na COM6; `Hash of data verified`. Log po resecie przez ok. 37 s pokazał rosnący uptime, heap 302664 B, PSRAM 8293368 B, puste kolejki oraz zerowe błędy CAN. Transceiver i auto były odłączone; zachowanie przy prawdziwych odpowiedziach ECU nadal niezweryfikowane.

## 38. Izolowany test dekodera OBD i powrót do NORMAL (8 października 2026)

- Dodano test parsera na osobnej instancji `UdsManager`: obcy ISO-TP First Frame (`41 05` zamiast oczekiwanego PID-u) ma być odrzucony przed Flow Control, Single Frame z obcym PID-em nie może ustawić odczytu, a poprawne `41 0B 64` powinno ustawić ciśnienie MAP na 100 kPa i zakończyć oczekujące żądanie. Test nie publikuje fikcyjnych danych do UI ani do właściwego pollera.
- Pierwszy przebieg ujawnił, że wewnętrzna wartość MAP była ustawiana tylko wtedy, gdy do parsera przypięto sensor ESPHome. Przeniesiono przypisanie wartości przed opcjonalną publikację, dzięki czemu dekodowanie nie zależy od obecności warstwy sensorów.
- SELFTEST na COM6 potwierdził w logu `OBD PID 0x0B MAP=100 kPa abs` oraz `isolated OBD parser self-test: PASS`. Test CAN raportował ok. 980 ramek/5 s, bez utraconych ramek, kolejek ani błędów.
- Po teście wgrano z powrotem firmware V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger USB 115200. Factory image został zweryfikowany przez esptool (`Hash of data verified`).
- Log normalnego firmware przez ok. 27 s: uptime rósł, CAN `RUNNING`, RX/TX, kolejki, utracone ramki i błędy pozostały zerowe; heap 302664 B, PSRAM 8293200 B. Transceiver i samochód były odłączone.
- To jest test filtracji/dekodowania syntetycznych ramek i krótkiego runtime na biurku. Nie potwierdza obsługi tych PID-ów ani wartości przez ECU CJCD; nie testuje pozytywnej odpowiedzi wieloramkowej z samochodu.


## 39. Wczesne odrzucanie obcych OBD Single Frame (8 października 2026)

- Poller OBD sprawdza teraz service oraz PID/InfoType w Single Frame przed skopiowaniem odpowiedzi do bufora i wywołaniem dekodera. Obcy PID lub usługa są ignorowane od razu; odpowiedź nie może zostawić przypadkowych bajtów w stanie parsera.
- Test izolowany rozszerzono o obcy PID i obcą usługę. SELFTEST zaliczył przypadki: błędny ISO-TP First Frame bez Flow Control, obce Single Frame bez zmiany stanu oraz prawidłowe `41 0B 64` → `MAP=100 kPa abs`. W logu pojawiło się `isolated OBD parser self-test: PASS`.
- Test CAN pracował bez transceivera: ok. 990 ramek/5 s, brak strat, kolejek i błędów. Kompilator zgłosił ostrzeżenie o zawężeniu typu w teście; poprawiono rzutowanie i przebudowano wariant NORMAL bez tego ostrzeżenia w diagnostyce kompilacji.
- Po teście ponownie wgrano firmware V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger 115200; hash obrazu zweryfikowany. Log po starcie przez ok. 16 s pokazał rosnący uptime, CAN `RUNNING`, puste kolejki i zerowe błędy, heap 302652 B, PSRAM 8293208 B.
- Brak auta i transceivera ogranicza dowód do zachowania parsera na sztucznych ramkach oraz startu firmware. Nie potwierdza dostępności PID-ów ani formatu odpowiedzi w ECU CJCD.


## 40. Test macierzy fail-safe alarmu regeneracji (8 października 2026)

- Rozszerzony test uruchamiany wyłącznie w SELFTEST sprawdza mapowanie stanów parsera: świeży status z obsługą obu pól i typem aktywnym → `active`; typ pasywny → `passive`; świeży brak regeneracji → `inactive`; brak obsługi typu lub upływ limitu świeżości → `unavailable`.
- Test ustawia pola stanu na odizolowanej instancji `UdsManager`, bez publikowania wartości do UI. To test logiki fail-safe, nie dekodowania surowej wieloramkowej odpowiedzi PID 0x8B ani implementacji w ECU.
- W logu płytki SELFTEST pojawiło się `isolated OBD parser self-test: PASS`; CAN odbierał około 990 ramek/5 s bez strat i błędów.
- Po teście przywrócono firmware V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger USB 115200. Hash factory image zweryfikowany; log po starcie przez ok. 11 s wykazał rosnący uptime, CAN `RUNNING`, zerowe błędy i kolejki, heap 302664 B, PSRAM 8293368 B.
- Syntetyczna macierz nie zastępuje walidacji bitów PID 0x8B i zachowania alarmu w rzeczywistym CJCD. Potrzebne porównanie z OBDeleven przy aktywnej i zakończonej regeneracji.


## 41. Jedna reguła klasyfikacji alarmu regeneracji (8 października 2026)

- Regułę fail-safe przeniesiono do wspólnej funkcji `classify_regen_status()`. Z niej korzystają klasyfikacja stanu OBD i publikacja binarnego alarmu, więc tekst UI oraz stan alarmowy nie rozjeżdżają się przy przyszłych zmianach.
- Asercje kompilatora obejmują stan aktywny, pasywny, nieaktywny, nieświeże dane oraz osobno brak obsługi statusu i typu. SELFTEST na odizolowanej instancji zaliczył rozszerzoną macierz i filtrację OBD (`isolated OBD parser self-test: PASS`).
- Po teście przywrócono V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger 115200. Flash potwierdzony hashem; log po starcie przez ok. 26 s: uptime rośnie, CAN `RUNNING`, kolejki/straty/błędy zero, heap 302664 B, PSRAM 8293200 B. Transceiver i samochód odłączone.
- Nadal niezweryfikowane pozostają znaczenie bitów PID 0x8B w konkretnej rewizji SAE i odpowiedź ECU `03L906018SJ`. Test nie jest walidacją na aucie ani dowodem prawdziwego alarmu.


## 42. Maszyna stanów historii regeneracji (8 października 2026)

- Przejścia historii wyodrębniono do `advance_regen_history()` i podłączono bezpośrednio do lambdy YAML. Asercje kompilatora sprawdzają: aktywny status przy starcie nie rozpoczyna pełnego pomiaru; po świeżym stanie nieaktywnym aktywny lub pasywny rozpoczyna śledzenie; kolejny nieaktywny kończy cykl; utrata świeżości kasuje bieżące śledzenie i nie zalicza cyklu.
- ESPHome 2026.9.1 zwalidowało i skompilowało firmware NORMAL; obraz aplikacji 622511 B, DIRAM 67143 B. Factory image wgrano na COM6 i zweryfikowano hashem.
- Log startowy przez ok. 26 s: uptime rósł, CAN `RUNNING`, kolejki, straty i błędy zerowe; heap 302652 B, PSRAM 8293208 B. Nie było transceivera ani auta.
- Asercje sprawdzają czystą maszynę stanów, a nie odczyty ECU. Na aucie trzeba nadal sprawdzić, czy rzeczywiste przejścia PID 0x8B odpowiadają temu modelowi.

## 43. Doprecyzowanie zakresu licznika historii i audyt glifów (8 października 2026)

- Etykietę karty HIST zmieniono z `ZAKOŃCZONE CYKLE` na `CYKLE OBSERWOWANE`; opis wyjaśnia, że są liczone tylko podczas pracy monitora. Chroni to przed pomyleniem lokalnej historii obserwacji z całkowitym licznikiem ECU.
- Audyt tablicy `cmap` obu lokalnych fontów Ubuntu potwierdził wymagane polskie litery oraz znaki stopni, jednostek i interpunkcji. Znaki bullet i wielokropek były zadeklarowane w YAML, ale nie istnieją w plikach fontów; usunięto je z pięciu list glifów. Nie występują w tekstach UI.
- README opisuje ograniczenia historii: liczone są pełne cykle zaobserwowane od świeżego stanu nieaktywnego; DEMO oraz cykle rozpoczęte przed startem monitora nie są uwzględniane.
- ESPHome 2026.9.1 zwalidowało i skompilowało wariant V1 (`CAN=true`, `NORMAL`, OBD polling `true`); obraz aplikacji ma 621615 B, DIRAM 67143 B (19,65%). Factory image wgrano na COM6, a esptool potwierdził `Hash of data verified`.
- Po restarcie log przez ok. 16 s pokazywał rosnący uptime, CAN `RUNNING`, puste kolejki i 0 błędów/strat; heap 302652 B, wolna PSRAM 8293208 B. RX=0, więc nie jest to test z ruchem CAN ani ECU.
- Wizualne dopasowanie nowych etykiet do ekranu pozostaje do obejrzenia na płytce.

## 44. Reassemblacja wieloramkowego OBD PID 0x7A (8 października 2026)

- Reassemblację ISO-TP First/Consecutive Frame wydzielono do helperów bez zależności od magistrali i UI; normalny poller wywołuje te same funkcje, które sprawdza SELFTEST.
- Izolowany test składa syntetyczną odpowiedź PID `0x7A` w dwóch ramkach, sprawdza payload `41 7A 01 00 64 ...` i dekodowanie ciśnienia `1,00 kPa`. Błędny numer sekwencji musi skasować częściowy payload. Test nie publikuje wartości do prawdziwego sensora.
- Na COM6 w trybie SELFTEST log pokazał `OBD PID 0x7A differential pressure=1.00 kPa` oraz `isolated OBD parser self-test: PASS`. CAN loopback uzyskał ok. 979–998 RX/5 s bez strat, kolejek ani błędów.
- Po teście wgrano z powrotem V1 NORMAL z CAN i OBD pollingiem włączonym. Kompilacja ESPHome 2026.9.1 przeszła (obraz aplikacji 622431 B, DIRAM 67143 B); esptool potwierdził `Hash of data verified`. Log po restarcie przez ok. 11 s: CAN `RUNNING`, RX=0, kolejki i błędy 0, heap 302652 B, PSRAM 8293208 B.
- SELFTEST sprawdza helper reassemblacji i dekoder, ale nie przechodzi przez rzeczywisty nadajnik Flow Control ani odpowiedź ECU. Weryfikacja ramek z auta i porównanie z OBDeleven nadal są wymagane.

## 45. Dopasowanie podpisu licznika do szerokości pola (8 października 2026)

- Metryki `hmtx` fontu Ubuntu-R przy rozmiarze 18 px wykazały, że `CYKLE OBSERWOWANE` zajmuje ok. 196,5 px w polu 200 px (margines tylko 3,5 px). Podpis skrócono do `CYKLE Z OBSERWACJI` (ok. 181,9 px, margines 18,1 px), bez zmiany znaczenia licznika.
- Dolna informacja `Liczone tylko podczas pracy monitora.` zajmuje ok. 309,5 px w polu 428 px i pozostaje czytelna z zapasem.
- ESPHome 2026.9.1 poprawnie skompilowało NORMAL V1 (obraz aplikacji 622431 B, DIRAM 67143 B); wgranie na COM6 zakończyło się `Hash of data verified`. Log startowy przez ok. 11 s wykazał CAN `RUNNING`, zerowe kolejki i błędy oraz heap 302652 B / PSRAM 8293208 B. RX=0, płytka nie odbierała danych z auta.
- Weryfikacja metrykami jest przybliżeniem szerokości advance; nie zastępuje obejrzenia wyrenderowanego ekranu. Wizualne potwierdzenie nowego podpisu pozostaje otwarte.

## 46. Audyt szerokości stałych etykiet LVGL (8 października 2026)

- Odczyt metryk TTF i rozmiarów widgetów wskazał, że `OBLICZONA MASA SADZY` zajmuje ok. 213,1 px w polu 220 px. Pole poszerzono do 224 px i przesunięto z x=12 na x=8, zachowując odstęp od statusu po prawej stronie.
- Inne stałe etykiety z jawnie określonym fontem i szerokością mają co najmniej 8 px łącznego zapasu; `CYKLE Z OBSERWACJI` ma ponad 18 px. Dynamiczne treści i odstępy kerningu mogą zmienić wynik — potrzebne jest potwierdzenie na ekranie.
- ESPHome 2026.9.1 poprawnie skompilowało NORMAL V1 (obraz aplikacji 622431 B, DIRAM 67143 B). Firmware wgrano na COM6, `Hash of data verified`; log startowy przez ok. 11 s pokazał CAN `RUNNING`, zero błędów, heap 302652 B i PSRAM 8293208 B. RX=0 — nadal bez danych z ECU.
- Wirtualny pomiar nie zastępuje oględzin głównego ekranu na panelu; wizualne potwierdzenie szerokości etykiety pozostaje otwarte.

## 47. Test surowego dekodowania PID 0x8B (8 października 2026)

- SELFTEST rozszerzono o surowe, wieloramkowe odpowiedzi PID `0x8B` składane tymi samymi helperami co poller. Macierz weryfikuje cztery przypadki: status aktywny z alarmem, pasywny bez alarmu, nieaktywny i nieobsługiwany typ jako niedostępny.
- Na COM6 log pokazał bajty A–G `03 03 ...` → `active_warning=yes`, `03 01 ...` → pasywny/bez alarmu, `03 00 ...` → nieaktywny oraz `01 03 ...` → nieobsługiwany typ. Wynik `isolated OBD parser self-test: PASS`; CAN loopback ok. 957–988 RX/5 s, bez strat, kolejek i błędów.
- SELFTEST nie mierzy ramek z ECU. Po nim przywrócono NORMAL V1 (`CAN=true`, OBD polling `true`), flash zweryfikowano hashem. Log startowy przez ok. 11 s: CAN `RUNNING`, RX=0, kolejki i błędy 0, heap 302664 B, PSRAM 8293356 B.
- Ten test potwierdza lokalną reassemblację i interpretację surowych bajtów według obecnego dekodera, ale nie potwierdza semantyki PID `0x8B` w ECU `03L906018SJ / 9977`. Nadal wymaga to porównania z OBDeleven w aucie.

## 48. Reassemblacja przez produkcyjny handler ramek (8 października 2026)

- Test PID `0x7A` i macierz PID `0x8B` przechodzą teraz przez `handle_obd_probe_response_()`, tak jak ramki odebrane przez normalny poller. W teście pominięto wysyłanie Flow Control, bo izolowany parser nie ma magistrali; kompletne syntetyczne odpowiedzi trafiają jednak do tej samej ścieżki reassemblacji i dekodera.
- SELFTEST na COM6 zaliczył test ciśnienia różnicowego `1.00 kPa`, aktywną regenerację z alarmem, pasywną bez alarmu, stan nieaktywny oraz nieobsługiwany typ; log potwierdził `isolated OBD parser self-test: PASS`. Odbiór testowy wynosił ok. 986–991 ramek/5 s, bez strat, kolejek i błędów CAN.
- Następnie wgrano firmware V1 `CAN=true`, `NORMAL`, OBD polling `true`, logger USB 115200; zapis factory image zakończył się `Hash of data verified`. Log startowy przez ok. 16 s pokazał CAN `RUNNING`, RX=0, puste kolejki, zero strat i błędów, heap 302664 B i PSRAM 8293200 B.
- Auto i transceiver pozostawały odłączone. To potwierdza tor syntetycznych ramek w oprogramowaniu, nie rzeczywiste Flow Control, zgodność PID-ów ani wartości dla ECU CJCD. Te nadal wymagają sprawdzenia z OBDeleven.

## 49. Ponowna kontrola rewizji SAE dla PID 0x8B (8 października 2026)

- Oficjalny rekord SAE dla J1979-DA wskazuje rewizję `J1979DA_202607`, zweryfikowaną 16 lipca 2026 r.; katalog potwierdza numer rewizji, ale użyty widok nie udostępnia tabeli PID `0x8B` do porównania. [Rekord SAE](https://saemobilus.sae.org/standards/j1979da_202607-j1979-da-digital-annex-e-e-diagnostic-test-modes)
- Dekoder w `uds_audi.h` nadal ma mapowanie z tabeli B118 wydania Oct 2011. Doprecyzowano komentarz w kodzie i dokumentację: nie jest to potwierdzenie zgodności z rewizją 202607 ani z ECU `03L906018SJ / 9977 / ODX 003003`.
- Kompilacja kontrolna NORMAL V1 po zmianie komentarza przeszła w ESPHome 2026.9.1: aplikacja 622699 B, DIRAM 67143 B (19,65%). Obrazu nie wgrywano, bo zachowanie programu się nie zmieniło.
- Następna walidacja wymaga dostępu do pełnej tabeli 202607 lub równoważnego wiarygodnego źródła oraz porównania na aucie z OBDeleven. Do tego czasu SELFTEST potwierdza tylko działanie parsera według obecnego dekodera.

## 51. Uzgodnienie dokumentacji ze statusem OBD widocznym w UI (8 października 2026)

- Audyt YAML wykazał, że `obd_link_available` jest wewnętrznym sensorem, a nie elementem widoku; kod nie wyświetla też globalnego paska CAN/OBD. Karta ECU pokazuje status świeżości/obsługi wybranych PID-ów, a strony danych prezentują placeholdery zamiast nieaktualnych odczytów.
- Usunięto z `DPF_PROFILES.md` nieaktualne twierdzenie, że górny pasek wskazuje świeżą odpowiedź ECU. Nie dodawano nowych elementów UI ani nie zmieniano firmware.

## 50. Krzyżowe sprawdzenie numeru ECU (8 października 2026)

- Oficjalny katalog Boscha łączy `03L 906 018 SJ` z EDC17C46 / `0281019897` oraz zastosowaniami Audi A4/A5 2.0 TDI 110 kW. Jest to spójne z podaną rodziną sterownika i mocą silnika CJCD, ale samo nie dowodzi kodu silnika w konkretnym samochodzie ani kalibracji DPF.
- Zaktualizowano opis kandydata CJCD w `DPF_PROFILES.md`, wyraźnie oddzielając zgodność numeru części/aplikacji od walidacji mapy PID/DID. Nie zmieniano firmware.
- Źródło: [Bosch Electronic Service — EDC17C46 / 03L 906 018 SJ](https://www.bosch-repair-service.com/hu/motorvezerloe-keszuelekek/bosch/edc17c46-0281019897001/).

## 52. Audyt ścieżki CAN pod kątem wpływu na wyświetlacz (8 października 2026)

- W `components/esp32_can_recover/esp32_can_recover.cpp` odbiór jest ograniczony do maksymalnie 24 ramek lub 500 µs na przebieg `loop()`. Limit czasu sprawdzany jest między ramkami, więc pojedynczy callback może go przekroczyć. Ramki standardowe służą do wykrycia aktywności magistrali, ale callback aplikacji dostaje tylko ramkę `0x7E8`; bufor payload jest ponownie używany, bez alokacji wektora dla każdej odpowiedzi.
- W wygenerowanym porządku komponentów ESPHome CAN wykonuje `loop()` przed LVGL (rejestracja odpowiednio 6 i 14). CAN nie korzysta z magistrali QSPI ekranu. Przebieg CAN ma jednak synchroniczny koszt CPU przed obsługą LVGL; zmierzony w SELFTEST maksymalny czas był wcześniej rzędu 148–162 µs przy ok. 1 000 ramkach/s. Ten wynik nie odtwarza pełnego ruchu samochodowego.
- Dotychczasowe próby: CAN NORMAL bez transceivera i z włączonym pollerem OBD pozostawał czysty przez 5 min; SELFTEST ok. 1 000 ramek/s, także z pollerem OBD, nie powodował śnieżenia ani narastania pamięci/kolejek w obserwowanych oknach. Zdarzały się jednak wcześniejsze powroty artefaktów po wgraniu CAN-on. W tych próbach nie podłączono auta ani transceivera.
- Wniosek: dostępne pomiary nie potwierdzają tezy o zapełnianiu RAM/kolejki przez brak odpowiedzi ECU ani o prostym zagłodzeniu LVGL przy przetestowanym obciążeniu. Przyczyna nie jest ustalona; pozostają do rozdzielenia rzeczywisty ruch CAN o większej gęstości, interakcja kontrolera TWAI z układem ekranu oraz wpływ zasilania/połączenia USB.
- Dodano dodatkowy limit czasu obsługi paczki CAN. ESPHome 2026.9.1 poprawnie skompilowało V1 `CAN=true`, `NORMAL`, OBD polling `true`; obraz aplikacji ma 622703 B, DIRAM 67143 B (19,65%). Nie wgrywano nowego obrazu. Próba tylko-odczytowego otwarcia COM6 nie powiodła się (`Access denied`), więc nie zebrano świeżych logów ani nie sprawdzono nowej wersji na płytce.
- Rozstrzygający test wymaga odtworzenia zjawiska na niezmienionym kablu i jasności, zebrania logów z licznikiem `max batch` / `max loop` oraz porównania źródła zasilania przy identycznej konfiguracji CAN. Test na aucie powinien obejmować normalny ruch magistrali i porównanie danych z OBDeleven; do tego czasu nie należy przypisywać winy samemu pollerowi OBD.

## 53. DEMO pokazuje regenerację aktywną i pasywną (8 października 2026)

- Tryb DEMO obejmuje teraz 60 s regeneracji aktywnej, 30 s pasywnej i 30 s chłodzenia. Symulowany status pasywny wyświetla `REGEN PASYWNA` i nie uruchamia alarmu `NIE GAŚ!`; alarm ma być widoczny tylko w fazie aktywnej.
- Nazwa stanu regeneracji jest wspólna dla ścieżki OBD i DEMO (`regen_status_name()`), dzięki czemu tekst statusu oraz flaga aktywnej regeneracji nie muszą odtwarzać mapowania enum osobno.
- Walidacja ESPHome 2026.9.1 i kompilacja NORMAL V1 (`CAN=true`, OBD polling `true`) przeszły. Obraz aplikacji: 622971 B; DIRAM: 67167 B (19,65%).
- Factory image wgrano na COM6; ESPHome/esptool potwierdziło `Hash of data verified`. Z logów po restarcie: CAN `RUNNING`, `RX=0`, kolejki TX/RX i błędy CAN równe 0; przez ok. 31 s heap pozostał na poziomie 302628 B, a wolna PSRAM 8293208 B. Transceiver i samochód były odłączone.
- Factory image i kopia w `firmware/V1/` mają zgodny SHA-256: `B6892E6A86BF98B6A0F81DEB571D17358D261C74E4D6FEA7D32226BA871F9E2E`.
- Zebrane logi potwierdzają tylko uruchomienie i krótki test bez ruchu CAN. Fazy DEMO trzeba jeszcze obejrzeć na ekranie; syntetyczne dane nie potwierdzają zachowania ECU.

## 54. Szybsze wygaszanie nieaktualnego alarmu regeneracji (8 października 2026)

- Dodano osobny limit świeżości statusu regeneracji: 10 s. Pozostałe wskazania i ogólna dostępność łącza nadal używają 15 s. Dzięki temu `NIE GAŚ!` wymaga świeższego potwierdzenia ECU, ale ma zapas na jeden wolny obieg sekwencyjnego pollera.
- Rozszerzono izolowany SELFTEST o przypadki statusu świeżego przy wieku 9 s i nieaktualnego przy 11 s, oba przy limicie 10 s. Pierwszy przebieg ujawnił błąd samego fixture testowego (pozostał stan pasywny po poprzedniej asercji); po jawnej inicjalizacji aktywnego typu test ponownie przeszedł.
- Na COM6 log SELFTEST pokazał `fresh_10s=1`, `stale_10s=1` i `isolated OBD parser self-test: PASS`. W CAN loopback uzyskano ok. 966–997 ramek/5 s bez strat, kolejek ani błędów.
- Po SELFTEST ponownie zbudowano i wgrano NORMAL V1 (`CAN=true`, OBD polling `true`). Walidacja ESPHome 2026.9.1 przeszła. Obraz aplikacji: 623507 B; DIRAM: 67167 B (19,65%). Weryfikacja flash: `Hash of data verified`.
- Po przywróceniu NORMAL log przez ok. 11 s pokazał CAN `RUNNING`, `RX=0`, brak kolejek RX/TX, strat i błędów; heap ok. 302640 B, PSRAM 8293200 B. Transceiver i samochód były odłączone.
- Aktualny factory image: `firmware/V1/audi-dpf-monitor-V1.factory.bin`, SHA-256 `AC458A3940F3DA359A0FC5FA9E1F1726AB4B4F6FED25AECA5A2F4C3B22847687`.
- To potwierdza kompilację, zapis flash i start firmware, ale nie test wzrokowy DEMO ani zgodność PID-u regeneracji z ECU CJCD. Test z samochodem i porównanie z OBDeleven pozostają otwarte.

## 55. Jedna wrap-safe reguła świeżości odczytów (8 października 2026)

- Dodano `timestamp_is_fresh(now, sample, max_age)`, używane przez świeżość DID, PID OBD, EGT, dostępność łącza OBD oraz aktywność CAN. Reguła uwzględnia naturalne przepełnienie 32-bitowego `millis()` i traktuje wiek równy limitowi jako nieaktualny.
- Asercje kompilatora sprawdzają 9 999/10 000 ms przy limicie 10 s oraz 9/10 ms po zawinięciu licznika. Izolowany SELFTEST przeszedł: `fresh_10s=1`, `stale_10s=1`, `isolated OBD parser self-test: PASS`.
- Test CAN loopback odbierał ok. 967–993 ramek/5 s bez strat, kolejek i błędów. Po teście przywrócono NORMAL V1; kompilacja i zapis na COM6 zakończyły się `Hash of data verified`.
- Log po przywróceniu NORMAL przez ok. 11 s: CAN `RUNNING`, `RX=0`, kolejki i liczniki błędów zerowe; heap 302640 B, PSRAM 8293200 B. Transceiver i samochód były odłączone.
- Aktualny factory image: `firmware/V1/audi-dpf-monitor-V1.factory.bin`, SHA-256 `F0F467622247DAA38DB9A9F5F97C74683DA931139BB0BD134FF942C6B2F69148`.
- To potwierdza logikę licznika czasu, nie prawdziwe zachowanie na ECU CJCD ani stabilność pracy w samochodzie. Dane i alarm regeneracji nadal wymagają porównania z OBDeleven.

## 56. Ponowna walidacja artefaktu NORMAL V1 (8 października 2026)

- ESPHome 2026.9.1 ponownie zwalidowało aktualny YAML dla `CAN=true`, trybu `NORMAL`, pollera OBD włączonego i loggera USB 115200. Kompilacja incremental zakończyła się powodzeniem; system zgłosił konfigurację builda jako aktualną, więc nie przebudowywał obiektów C++.
- Wygenerowano ponownie factory image 689152 B i OTA image 623616 B. Factory image w katalogu build i kopia `firmware/V1/audi-dpf-monitor-V1.factory.bin` mają zgodny SHA-256 `F0F467622247DAA38DB9A9F5F97C74683DA931139BB0BD134FF942C6B2F69148`. Raport rozmiaru: DIRAM 67167 B (19,65%), flash 623507 B (7,7%).
- Firmware nie był w tej próbie wgrywany na płytkę. Ostatnie potwierdzone wgranie i log startowy opisano w punkcie 55; test w aucie oraz porównanie z OBDeleven pozostają otwarte.

## 57. Pełna kompilacja z czystego katalogu ESPHome (8 października 2026)

- Dla wariantu V1 `CAN=true`, `NORMAL`, poller OBD `true` i logger USB 115200 wykonano osobny build z nowego katalogu danych ESPHome, bez użycia obiektów C++ z głównego cache’u projektu. CMake rozpoznał Xtensa GCC/G++ 14.2.0; pełne 1360 kroków Ninja, w tym `main.cpp`, `uds_audi.h` i własny komponent TWAI, zakończyło się `Successfully compiled program`.
- Raport rozmiaru: aplikacja 623507 B, DIRAM 67167 B (19,65%), flash 7,7%. Factory image: 689152 B, SHA-256 `F8444BA46F212BF1E989DDEE96B96AF62D44738A06C8B441670F7F34B9790405`; OTA image: 623616 B. Hash różni się od obrazu w punkcie 55 przez metadane czasu kompilacji; ustawienia i config hash są zgodne.
- ESP-IDF zgłosił niekrytyczne ostrzeżenie, że w tej izolowanej sesji nie ustawiono `ESP_ROM_ELF_DIR`; konfiguracja CMake mimo tego zakończyła się poprawnie i kompilacja obrazu przeszła.
- Świeży build nie był wgrywany na płytkę. Wymaga to odrębnej próby uruchomieniowej; testy ECU/OBDeleven w aucie pozostają otwarte.

## 58. Pełnoszeroki pasek obliczonej sadzy (8 października 2026)

- Na karcie DPF pasek poszerzono z 424 do 440 px i zmniejszono marginesy boczne do 8 px. Tło pustej części rozjaśniono, aby widoczny był cały tor, a nie tylko turkusowe wypełnienie. Skala pozostała 0–40 g, ponieważ limit dla konkretnego ECU nie jest potwierdzony.
- ESPHome 2026.9.1 poprawnie zwalidowało i skompilowało wariant V1 NORMAL (`CAN=true`, OBD polling `true`, logger 115200). Aplikacja ma 623507 B, DIRAM 67167 B (19,65%), flash 7,7%.
- Factory image (`firmware/V1/audi-dpf-monitor-V1.factory.bin`) ma SHA-256 `AEBD5B19004A6C5A3B5D13BF9B4FEFFF7982759777EC947E5DCF46A249024C3F`; hash kopii builda jest identyczny. Obraz wgrano na COM6; esptool potwierdził `Hash of data verified`. Log startowy przez ok. 32 s: CAN `state=1`, `RX=0`, puste kolejki, zero błędów i strat, heap 302628 B oraz PSRAM 8293208 B bez zmian. Transceiver i auto były odłączone.
- Statyczny przegląd prostokątów wszystkich pięciu kart względem obszaru 456 × 280 px nie wykazał elementów wychodzących poza ekran. To kontrola współrzędnych, nie zastępstwo oceny wizualnej kontrastu i czytelności.
- Log nie potwierdza jeszcze wzrokowo czytelności nowego paska; obrazu z fizycznego ekranu nie da się ocenić przez serial. Testy danych z ECU i OBDeleven pozostają otwarte.

## 59. Dodatkowe przypadki fail-safe dekodera regeneracji (8 października 2026)

- Rozszerzono surowy test ISO-TP PID `0x8B` o przypadek, gdy ECU deklaruje obsługę typu regeneracji, ale nie statusu (`A=0x02`), oraz gdy nie deklaruje obsługi żadnego pola (`A=0x00`). Oba przypadki muszą zwracać `unavailable`, nawet jeśli bajt B wygląda jak aktywna regeneracja.
- ESPHome 2026.9.1 zwalidowało i skompilowało V1 `CAN=true`, `SELFTEST`, OBD polling `true`. Rozmiar aplikacji 623583 B, DIRAM 67167 B (19,65%).
- Wariant SELFTEST wgrano na COM6; esptool potwierdził `Hash of data verified`. Log wykazał sześć poprawnych scenariuszy surowego PID (`regen_raw=1/1/1/1/1/1`), wszystkie asercje bezpieczeństwa i `isolated OBD parser self-test: PASS`. CAN loopback utrzymywał ok. 976–1003 ramek/5 s, bez strat, błędów ani wzrostu kolejek; heap i PSRAM pozostały stabilne.
- Po teście przywrócono NORMAL V1 (`CAN=true`, OBD polling `true`) na COM6; zapis flash przeszedł `Hash of data verified`. Aktualny obraz factory ma SHA-256 `B9076098EE14FC1355705B51D1A68A9B1EDB648E6AD0B56FA61233E77F373192`, aplikacja 623587 B. Krótki log potwierdził `CAN state=1`, brak ruchu (`RX=0`), puste kolejki i zero błędów; heap/PSRAM stabilne. Samochód i transceiver nie były podłączone.
- Test dotyczy tylko fail-safe parsera w syntetycznych odpowiedziach. Nie potwierdza znaczenia PID `0x8B` w konkretnym ECU CJCD ani zachowania podczas rzeczywistej regeneracji; porównanie z OBDeleven nadal jest wymagane.

## 60. Audyt glyphów UI i poprawka znaku równości (8 października 2026)

- Audyt napisów ekranowych względem jawnego zestawu glyphów LVGL wykrył brak znaku `=` w etykiecie `CJCD = kandydat; DID-y wymagają weryfikacji.`. Dodano go do wszystkich pięciu ról fontu; Ubuntu Regular i Bold zawierają używane polskie znaki i symbole, a sprawdzone napisy UI mieszczą się teraz w zestawie glyphów.
- ESPHome 2026.9.1 poprawnie zwalidowało i skompilowało wariant V1 NORMAL (`CAN=true`, OBD polling `true`, logger USB 115200). Aplikacja ma 624163 B; DIRAM 67167 B (19,65%).
- Firmware wgrano na COM6; esptool potwierdził `Hash of data verified`. Factory image w `firmware/V1/audi-dpf-monitor-V1.factory.bin` i katalogu build ma zgodny SHA-256 `F5162376DEDFF4805488858FB715B99E8A25E5ABF2FF20B55CBF31E0CFE8AAA5`.
- Log startowy potwierdził NORMAL (`CAN state=1`), brak ruchu CAN, puste kolejki i zero błędów; heap 302640 B i PSRAM 8293368 B były stabilne przez ok. 16 s. Obraz fontu wymaga jeszcze oceny wzrokowej na ekranie, zwłaszcza etykiety profilu ECU.

## 61. Bufor LVGL dla płynniejszego rysowania przy obrocie programowym (8 października 2026)

- ESPHome raportuje programowy obrót LVGL, który zmniejsza wydajność. Zmieniono `buffer_size` z 25% na 12%; dokumentacja ESPHome zaleca mały bufor przy dostępnej PSRAM, ponieważ LVGL próbuje wtedy przydzielić bufor rysowania w szybszej pamięci wewnętrznej. Sam log pamięci nie potwierdza jednak szybkości renderowania ani dokładnego miejsca każdej alokacji. [Dokumentacja LVGL](https://esphome.io/components/lvgl/#choosing-a-buffer-size)
- Wariant V1 NORMAL (`CAN=true`, OBD polling `true`) skompilował się i został wgrany na COM6; esptool potwierdził `Hash of data verified`. Aplikacja 624159 B, DIRAM 67167 B (19,65%). Factory image w buildzie i `firmware/V1/` ma SHA-256 `C08A314E81EB30F09ABEFB742A62C40D5967AE255F07826934DF798BAE7FCCBB`.
- Log przez ok. 16 s: `CAN state=1`, `RX=0`, puste kolejki i zero błędów. Heap wewnętrzny: 334532 B wolne / 278528 B największy blok; PSRAM: 8325112 B wolne / 8257536 B największy blok. Brak błędu inicjalizacji LVGL.
- Dodatkowy log przez ok. 72 s po starcie pokazał niezmienione wartości heap/PSRAM oraz zero błędów CAN. Podsumowanie FT3168: 5 odzyskanych ponowień odczytu statusu na 60 s, 0 ponowień współrzędnych, 0 błędów nieodzyskanych, 0 transakcji I²C >=20 ms, maks. 1 ms. To potwierdza tylko krótki test bez dotykania ekranu; nie jest pomiarem płynności UI.
- Oczekuje na ocenę wizualną, czy po tej zmianie odrysowanie jest szybsze i czy przejścia stron nadal są płynne. Nie wyciągać wniosku o przyroście szybkości na podstawie samej kompilacji i logu pamięci.

## 62. Porównanie bufora LVGL i przywrócenie zwykłego firmware (8 października 2026)

- Włączono na czas pomiaru `ui_perf_metrics_enabled=true`, które wymuszało pełne odrysowanie aktywnej strony co 10 s i raportowało czas od `on_draw_start` do `on_draw_end`. To przedział obejmuje render LVGL oraz zakończenie transferu wyświetlacza; nie mierzy bezpośrednio FPS ani płynności animacji.
- Na tym samym urządzeniu, z niezmienionym CAN/OBD i programowym obrotem 90°, bufor 12% dał cztery raporty `avg=184 ms max=184 ms`. Bufor 25% dał trzy raporty `avg=112 ms max=112 ms`, czyli ok. 39% krótszy czas pełnego odrysowania. Test 25% trwał ok. 32 s; CAN zgłaszał brak ramek, ale bez błędów, strat i kolejek.
- Przy 25% log pamięci wskazał heap 302412 B wolne / 249856 B największy blok i PSRAM 8293324 B wolne / 8257536 B największy blok. To krótki pomiar stołowy, nie wielogodzinny test stabilności ani ocena wizualna po zmianie.
- Przyjęto 25% jako ustawienie normalne. `ui_perf_metrics_enabled` ustawiono na `false`, więc w normalnym firmware nie ma wymuszonych odświeżeń diagnostycznych.
- ESPHome 2026.9.1 poprawnie zbudowało i wgrało wariant NORMAL V1 (`CAN=true`, OBD polling `true`) na COM6. Esptool potwierdził `Hash of data verified`. Aplikacja: 624627 B; DIRAM: 67343 B (19,7%); factory image: 690272 B; SHA-256 `67D6A23D85AD9A1304F999EAE9E23D0E97D05D44D2D78D426DB02CE14C8284A5`.
- Log po wgraniu przez ok. 11 s pokazał `CAN state=1`, `RX=0`, puste kolejki i zero błędów; heap/PSRAM pozostały stałe. Nie pojawiły się raporty `ui_perf`, zgodnie z wyłączonym trybem diagnostycznym.
- Następny test na stole: wzrokowo porównać odrysowanie i przejścia kart z poprzednim firmware. Preset `WAVESHARE-ESP32-S3-TOUCH-AMOLED-1.64` jest generowany z `HAS_HARDWARE_ROTATION=false`, dlatego LVGL używa obrotu programowego; zmiana wymagałaby odrębnej modyfikacji sterownika i nie została wprowadzona. Nie testowano danych z ECU ani zgodności z CJCD.
## 63. Pięciominutowy log NORMAL V1 z buforem 25% (8 października 2026)

- Po wgraniu końcowego obrazu NORMAL V1 pozostawiono urządzenie podłączone przez USB przez ponad 5 minut; odczyt szeregowy był otwarty przez cały test. Auto i transceiver CAN pozostawały odłączone.
- Raporty przez cały okres: CAN `state=1`, `RX=0`, kolejki RX/TX równe 0, brak strat i błędów CAN. Heap utrzymywał się na poziomie 302412 B wolne / 249856 B największy blok; PSRAM 8293324 B wolne / 8257536 B największy blok. W pokazanym logu nie było komunikatu restartu ani błędu komponentu.
- Raporty FT3168/60 s: ponowienia odczytu statusu 4–5, ponowienia współrzędnych 0, błędy nieodzyskane 0, transakcje >=20 ms 0, maksymalny czas transakcji 1 ms.
- To potwierdza tylko stan runtime i liczniki sterownika w pięciominutowym oknie z odłączonym CAN. Nie sprawdza wizualnie jakości obrazu, faktycznego dotyku podczas tego testu, obciążenia ruchu CAN ani odczytów auta.
## 64. Pomiar bufora LVGL 50% i przywrócenie wariantu NORMAL 25% (8 października 2026)

- Dla dodatkowego porównania ustawiono `buffer_size: 50%` i czasowo włączono wymuszone odrysowanie aktywnej strony co 10 s. Trzy próbki wyniosły 130, 129 i 130 ms (`avg=max` w każdym 10-sekundowym oknie).
- To wynik wolniejszy od 25% (112 ms), mimo większego bufora. Wariant 50% zużywał także PSRAM: log wskazywał 8101796 B wolne / 7995392 B największy blok; heap wewnętrzny 366256 B wolne / 311296 B największy blok. W 25% wcześniejszy log wykazał PSRAM 8293324 B wolne / 8257536 B największy blok i heap 302412 B / 249856 B. Nie wykryto błędów ani strat CAN w krótkim teście.
- Przywrócono `buffer_size: 25%`, `ui_perf_metrics_enabled: false` oraz firmware NORMAL V1 na COM6. Kompilacja i zapis factory image zakończyły się powodzeniem, esptool potwierdził `Hash of data verified`. Krótki log po starcie: CAN `state=1`, `RX=0`, puste kolejki, brak błędów; heap/PSRAM bez zauważalnej zmiany.
- Wniosek pomiarowy: w tej konfiguracji 25% daje krótszy czas pełnego odrysowania niż 12% i 50%. Nie jest to ocena płynności przejść ani wizualna inspekcja ekranu; obraz po wgraniu normalnego wariantu nadal wymaga oceny na urządzeniu.
## 65. Izolowany SELFTEST ISO-TP i przywrócenie NORMAL V1 (8 października 2026)

- Rozszerzono `UdsManager::run_obd_parser_self_test()` o dwa przypadki: wycięcie bajtów paddingu z końcowej ramki CF oraz poprawne przejście licznika sekwencji 0xF→0x0 w odpowiedzi wieloramkowej. Test sekwencji składa 118 bajtów w 16 CF; test paddingu sprawdza, że do payloadu wchodzi tylko zadeklarowana długość.
- Wariant `SELFTEST`, OBD polling wyłączony, skompilowano i wgrano na COM6. Log wykazał `padding=1`, `seq_wrap=1`, wszystkie wcześniejsze przypadki regeneracji `=1` oraz `isolated OBD parser self-test: PASS`. Wewnętrzny loopback TWAI odbierał 960–987 ramek/5 s, bez strat, błędów ani kolejek; maks. czas pętli 97 µs. To sprawdza logikę parsera i wewnętrzny loopback, nie odpowiedź ECU ani fizyczny ruch pojazdu.
- Po SELFTEST ponownie zbudowano i wgrano NORMAL V1 (`CAN=true`, OBD polling `true`, bufor LVGL 25%, pomiar UI wyłączony). Esptool potwierdził `Hash of data verified`. Factory image ma 691024 B; SHA-256 `8A5AD1AB1D06AEF4AD21AB991FEB7812DEE7C924E9082A904391413DEE53F171`.
- Log po przywróceniu NORMAL: CAN `state=1`, `RX=0`, puste kolejki, zero błędów; heap/PSRAM stabilne w krótkim oknie. SELFTEST nie pozostał aktywny.
- Wynik potwierdza zachowanie wspólnego parsera na syntetycznych ramkach klasycznego ISO-TP. Nie potwierdza tabel SAE dla aktualnego ECU, PID-u regeneracji ani wartości CJCD; do tego nadal potrzebny jest samochód i porównanie z OBDeleven.

## 66. Log NORMAL V1 podczas faz DEMO (8 października 2026)

- Odczyt tylko przez USB/COM6, bez restartu i bez wgrywania. Użytkownik poinformował, że obraz na ekranie jest stabilny; w czasie monitoringu DEMO było włączone na urządzeniu.
- Log `dpf_demo` zarejestrował fazę `active` przy 0 s, `passive` przy 60 s i `cooling` przy 90 s. Przykładowy odczyt zmieniał się: sadza obliczona 23.6 → 16.6 → 6.4 g; EGT DPF 610 → 520 → 556 °C. Są to dane symulowane DEMO, nie odczyty ECU.
- Przez ok. 113 s: CAN `state=1`, `RX=0`, kolejki RX/TX 0, brak błędów/strat CAN; maksymalny czas pętli 18–21 µs. Heap pozostawał w zakresie 302124–302344 B wolne, PSRAM 8293220–8293268 B wolne. Odczyty FT3168: ponowienia statusu 2–3, współrzędne 0, błędy nieodzyskane 0, transakcje I²C >=20 ms 0, maks. 1 ms.
- Po monitoringu zamknięto sesję logów, aby zwolnić port COM6. Test potwierdza zmianę stanów DEMO i stabilne liczniki runtime w tym oknie; nie dowodzi wizualnej poprawności każdej karty ani poprawności CAN/OBD z autem.

## 67. Uproszczenie karty ustawień i blokada kompilacji hosta (8 października 2026)

- Zmieniono kartę ECU na kartę `USTAWIENIA`. Zawiera teraz tylko wybór profilu, ostrzeżenie że profil CJCD pozostaje niezweryfikowany, DEMO i jasność; długie ECU Name/CALID oraz skrót PID usunięto z widoku. Sensory identyfikacyjne pozostają w konfiguracji, ale nie są wyświetlane na 1,64-calowym ekranie.
- ESPHome 2026.9.1 zgłosiło `Configuration is valid!` dla V1 NORMAL (`CAN=true`, OBD polling `true`). Pełny build zatrzymał się przed kompilacją C++: uruchomienie `xtensa-esp32s3-elf-g++.exe --version` zwróciło Windows `ERROR_ILLEGAL_DLL_RELOCATION` (623), a bezpośredni Ninja zgłosił `CreateProcess failed`.
- SHA-256 lokalnego archiwum toolchainu wynosi `82cbe0353c2e7c96acc8755c854aaa2d93d346c6a378fc692e60b3912c560108`, zgodnie z wpisem win64 w `C:\Espressif\frameworks\5.5.5\tools\tools.json`; SHA-256 kompilatora i jego DLL-e również odpowiadają kopiom z archiwum. To wskazuje na błąd uruchamiania narzędzia po stronie Windows, nie na wadliwy YAML ani niekompletny pakiet.
- Nie wygenerowano nowego obrazu firmware ani nie wgrywano płytki. Obrazy `firmware/V1/audi-dpf-monitor-V1.*` są starsze (2:48); nie wolno traktować ich jako wersji z uproszczoną kartą. Wzrokowa weryfikacja zmian pozostaje otwarta.

## 68. Bieżący przyrostowy build i monitoring NORMAL V1 przez USB (8 października 2026)

- ESPHome 2026.9.1 poprawnie zwalidowało konfigurację V1 NORMAL (`CAN=true`, OBD polling `true`) i zakończyło kompilację komunikatem `Successfully compiled program`. CMake uznał konfigurację za aktualną i Ninja wykonał tylko pozostałe kroki, więc nie jest to dowód ponownej kompilacji wszystkich plików C++. Utworzony obraz aplikacji ma 625375 B (7,7% flash), DIRAM 67191 B (19,66%); nie wgrywano go na płytkę.
- Tylko-odczytowy monitoring COM6 rozpoczął się przy uptime 360063 ms i zakończył przy 437031 ms; nie odnotowano restartu. Przez ok. 77 s CAN był `RUNNING`, RX=0 (brak transceivera/auta), kolejki i liczniki błędów/strat CAN wynosiły 0. Heap pozostawał 302692/249856 B, PSRAM 8294388/8257536 B; maksymalny czas pętli CAN wynosił 14–20 µs.
- FT3168 zgłosił cztery ponowienia odczytu statusu w jednym oknie 60 s, brak ponowień współrzędnych i jeden nieodzyskany odczyt statusu; log zawiera pojedyncze ostrzeżenie o błędzie rejestru `0x02`. Nie wystąpiły transakcje I²C >=20 ms, maksymalny zmierzony czas wyniósł 1 ms. Oznacza to sporadyczny błąd I²C wymagający obserwacji, ale nie tłumaczy kolorowego śnieżenia ani nie dowodzi jego przyczyny.
- W tym odczycie nie było wizualnego potwierdzenia stanu ekranu. Sesję logów zamknięto, aby zwolnić COM6; nie wgrywano firmware i nie resetowano płytki.

## 69. Powtórny odczyt FT3168 i ostrzeżenie długiej operacji LVGL (8 października 2026)

- Odczyt tylko przez COM6, bez restartu i wgrywania, od uptime 703843 ms do 791029 ms (ok. 87 s). CAN pozostawał `RUNNING`, RX=0, kolejki i błędy/straty CAN = 0; maksymalny czas pętli CAN 14–24 µs. Heap wynosił 302692 B, a po przejściu do fazy DEMO 302424 B; PSRAM zmieniła się z 8294388 B do 8294308 B.
- Dwa raporty FT3168: `retries status=4` i `3`, `coordinates=0`, `unrecovered=0`, `slow(>=20ms)=0`, `max=1 ms`. W tym oknie pojedynczy nieodzyskany błąd z poprzedniego logu nie powtórzył się.
- O 10:03:21 logger zgłosił `[W][component:419]: lvgl took a long time for an operation (181 ms), max is 140 ms`. CAN był wtedy bez ruchu i bez błędów; log nie wskazuje więc na obciążenie odbiornika CAN jako bezpośrednią przyczynę tej operacji. To pojedyncze ostrzeżenie czasowe, nie dowód śnieżenia ani zamrożenia ekranu. Osiem sekund później DEMO zgłosiło fazę `active`.
- Nie było potwierdzenia wzrokowego ekranu ani informacji o dotknięciach w trakcie testu. Warto skorelować kolejne ostrzeżenia LVGL z metryką renderowania i ręczną obserwacją UI; bieżące buforowanie 25% wcześniej dawało ok. 112 ms dla pełnego odrysowania.

## 70. Ponowny monitoring NORMAL V1 przez COM6 (8 października 2026)

- Odczyt tylko do logów przez ok. 2 minuty; bez resetu, kompilacji i wgrywania. Uptime płytki wzrósł z ok. 1 083 897 ms do 1 217 498 ms, więc podczas okna odczytu nie było restartu.
- CAN pozostawał `RUNNING`; RX=0, kolejki RX/TX, straty i liczniki błędów CAN/TX pozostawały zerowe. Maksymalny czas pętli CAN wynosił 15–19 µs. Heap utrzymywał się na 302424/249856 B, PSRAM na 8294256/8257536 B.
- Dwa raporty FT3168: ponowienia statusu 5 i 4, ponowienia współrzędnych 0, błędy nieodzyskane 0, transakcje >=20 ms 0, maks. 1 ms. W tym krótkim oknie nie powtórzył się komunikat `lvgl took a long time for an operation`.
- Log nie zawierał potwierdzenia wizualnego stanu ekranu ani wyniku dotyku. To kolejny stabilny odczyt telemetrii, ale sam nie wyklucza śnieżenia ani zamrożenia obrazu.

## 71. Karta porównawcza kandydatów DID dla CJCD (10 października 2026)

- Włączono tymczasowy, tylko-odczytowy tryb `dpf_did_diagnostic_probe`. Zestaw sześciu kandydatów: `0x114F`, `0x114E`, `0x14F5`, `0x1156`, `0x11B2`, `0x10FB`; żaden nie jest potwierdzony dla `03L906018SJ / 9977 / ODX 003003`. Kandydackie odpowiedzi nie trafiają do produkcyjnych sensorów DPF ani logiki alarmu regeneracji.
- Dodano widok `USTAW → DANE DPF DO PORÓWNANIA`: wartości kandydackie, ostrzeżenie o niepotwierdzonym mapowaniu i status ruchu CAN. Początkowo `114F`, `114E`, `14F5` były surowymi U16, a `1156` był przeliczany na km; późniejsze porównanie z OBDeleven z 11 października wstępnie dopasowało skale pierwszych trzech i wykazało błąd przeliczenia `1156`. Szczegóły znajdują się w sekcji 76.
- ESPHome 2026.9.1 zwalidowało YAML; firmware skompilował się i został wgrany na COM6, a esptool potwierdził `Hash of data verified`. Krótki odczyt po wgraniu potwierdził działające uptime i uruchomiony TWAI.
- Użytkownik poinformował, że płytka jest obecnie odłączona od samochodu. Odczyt `RX=0`, brak ACK i timeouty DID z tego okna są zgodne z brakiem auta i nie stanowią dowodu usterki wiązki ani nieobsługiwania DID-ów. Porównanie z OBDeleven i wzrokowa ocena nowego statusu na ekranie pozostają do wykonania.

## 72. Blokada sondy UDS na cichej magistrali i test stołowy (10 października 2026)

- Pierwsze wgranie V1 z 30-sekundowym odstępem między pełnymi cyklami nadal wysyłało sześć DID-ów po ok. 500 ms. Przy braku auta/TX ACK log pokazał `queued TX=1`, timeouty i szybki wzrost `bus errors` (365306 → 912530 w ok. 30 s). Ta osłona była niewystarczająca.
- Zmieniono warunek startu sondy: w tymczasowym trybie porównawczym UDS nie nadaje, dopóki kontroler nie wykryje co najmniej 3 standardowych ramek danych CAN w oknie 1 s; świeżość ruchu jest wymagana przy rozpoczęciu cyklu. Dzięki temu cicha magistrala stołowa nie dostaje żądań, a aktywny samochód może odblokować odczyt.
- Poprawiony firmware V1 NORMAL (`CAN=true`, OBD polling `true`, kandydackie DIDs, logger 115200) skompilowano i wgrano na COM6. ESPHome zgłosiło `Successfully compiled program`, esptool: `Hash of data verified`, `Successfully uploaded program`; połączenie wykryło ESP32-S3 rev v0.2.
- Log po poprawce obejmował ponad 70 s: CAN `state=1`, `RX=0`, `queued RX/TX=0`, `RX errors=0`, `TX errors=0`, `TX failed=0`, `bus errors=0`. Nie pojawiły się wysłane zapytania DID ani timeouty. Heap ok. 174–178 kB wolne, PSRAM ok. 8.29 MB wolne. FT3168: `retries status=5`, `coordinates=0`, `unrecovered=0`, `slow(>=20ms)=0`, `max=1 ms`; ESPHome zgłosiło `Boot seems successful`.
- To potwierdza ograniczenie nadawania na cichej magistrali i start firmware'u na stole. Nie potwierdza obrazu/dotyku przez obserwację wzrokową ani danych ECU; użytkownik musi ponownie ocenić ekran, a porównanie DID-ów z OBDeleven wymaga podłączenia do auta.

## 73. Dłuższy monitoring firmware'u z blokadą cichego CAN (10 października 2026)

- Ponowny odczyt COM6 trwał ok. 117 s; uptime wzrósł z 331891 do 449150 ms bez resetu. Przez wszystkie raporty CAN pozostawał `state=1`, `RX=0`, kolejki RX/TX=0, straty i błędy RX/TX/bus=0. Brak timeoutów i logów wysłanych DID-ów na cichej magistrali.
- Heap raportował ok. 173–176 kB wolne (largest block 131–135 kB), PSRAM 8291028 B wolne. Pętla CAN zwykle 47–64 µs, pojedyncze maksima 285–461 µs.
- Dwa raporty FT3168: retries status 5 i 4, coordinates 0, unrecovered 0, slow(>=20ms)=0, max 2 ms. ESPHome nie zgłosiło ponownego startu. Nadal nie było wzrokowej kontroli obrazu ani ręcznego testu dotyku po tym buildzie.

## 74. Wdrożenie ekranu porównawczego przez OTA (10 października 2026)

- Potwierdzono, że ekran `USTAW → DANE DPF DO PORÓWNANIA` prezentuje sześć kandydackich odczytów UDS, oddzielonych od produkcyjnych sensorów i logiki alarmu regeneracji.
- Ponowna kompilacja przez `Kompiluj-monitor-DPF.ps1 -BoardRevision V1` zakończyła się powodzeniem: YAML poprawny, firmware zbudowany dla V1, `CAN=true`, `OBD=true`, bufor LVGL 25%. Rozmiar obrazu aplikacji: 1 271 827 B; RAM DIRAM: 118 987 B (34,8%).
- Wgrano obraz OTA na potwierdzony adres `192.168.0.166`. ESPHome potwierdziło `Connected`, `Handshake complete`, `OTA successful` i `Successfully uploaded program`.
- Po aktualizacji odczytano strumień logów przez HTTP Digest: HTTP 200, uptime wzrósł z 41 567 do 56 879 ms bez restartu. CAN `state=1`, `RX=0`, kolejki RX/TX=0, błędy/straty=0; urządzenie odpowiadało po restarcie.
- Użytkownik potwierdził po OTA czysty obraz i widoczny ekran porównawczy. Panel WWW i start firmware są potwierdzone, ale nie potwierdzono jeszcze żadnej wartości z ECU; płytka pozostaje poza autem.

## 75. Interpretacja RX i odbiornika SN65HVD230 (10 października 2026)

- Przegląd `components/esp32_can_recover/esp32_can_recover.cpp`: licznik `RX` zwiększa się o liczbę ramek odczytanych przez TWAI przed filtrowaniem payloadów do ID `0x7E8`. Callback aktywności CAN jest również wywoływany przed filtrem. Zatem `RX=0` z wcześniejszego logu przy pracującym silniku oznacza brak ramek rozpoznanych przez kontroler TWAI w tym oknie, a nie tylko brak odpowiedzi ECU `0x7E8`. Taki wynik kieruje dalszą diagnozę do ścieżki fizycznej: OBD piny 6/14, CANH/CANL, zasilanie/masa transceivera, połączenia D/R do GPIO5/6, tryb pracy RS i bitrate.
- Dokumentacja TI podaje, że SN65HVD230 zasilany jest 3,3 V, pin `D` jest wejściem nadajnika, `R` wyjściem odbiornika, CANH/CANL liniami magistrali, a `RS` wybiera tryb. Dla HVD230 `RS` w stanie wysokim włącza standby nadajnika, ale odbiornik nadal działa; HVD231 zachowuje się inaczej i wyłącza także odbiornik. Nie zakładamy, że moduł Waveshare lub jego klon ma identyczny układ RS bez sprawdzenia oznaczenia układu i połączeń płytki.
- Rozstrzygający następny test wymaga ponownego podłączenia do auta i krótkiego logu z pracującym silnikiem: najpierw sprawdzić, czy `RX` rośnie, następnie czy pojawiają się odpowiedzi na DID. Do czasu tego pomiaru nie ma podstaw, by zmieniać mapę DID ani uznać brak wartości za problem skalowania.

## 76. Minutowy monitoring po OTA i potwierdzenie UI (10 października 2026)

- Użytkownik potwierdził, że ekran jest czysty i wyświetla stronę porównawczą po OTA.
- Strumień HTTP Digest przez 60 s: uptime wzrósł z 250488 do 306570 ms bez restartu; CAN `state=1`, `RX=0`, kolejki, straty, TX failed oraz błędy RX/TX/bus pozostawały zerowe. Płytka nie była podłączona do auta, więc brak ramek był oczekiwany.
- Heap wolny wahał się między ok. 169748 a 176000 B, największy blok 126976 B; PSRAM 8291196 B. Raport FT3168: retries status=4, coordinates=0, unrecovered=0, slow(>=20ms)=0, max=3 ms.
- To potwierdza minutowy runtime i potwierdzony wzrokowo UI poza autem, nie wielogodzinną stabilność ani odczyty/zgodność danych ECU.

## 77. Zewnętrzny zrzut kanałów VCDS jako pomoc do porównania (10 października 2026)

- Sprawdzono publiczny dokument `DPF_H1.docx`: pokazuje kanały IDE00432–IDE00436 oraz IDE01407–IDE01410 i IDE06059 dla sterownika `DRV\\03L-906-018-CGL.CLB`, protokół UDS. To rodzina pokrewna, ale nie dokładny profil `03L906018SJ / 9977 / ODX 003003`.
- Dopisano listę nazw do arkusza przygotowań OBDeleven. Oznaczenia IDE są nazwami kanałów diagnostycznych, a nie adresami surowych UDS DID; z dokumentu nie wyprowadzano żadnego adresu ani przelicznika i nie zmieniano firmware.
- Źródło: [zrzut VCDS DPF_H1](https://community.dieselschrauber.org/download.php?id=12813). Dostępna dokumentacja pomaga ustalić, jakich nazw szukać, ale mapowanie, jednostki i wartości graniczne nadal trzeba potwierdzić dla konkretnego ECU w samochodzie.

## 78. Walidacja i kompilacja bieżącego firmware'u porównawczego (10 października 2026)

- `Kompiluj-monitor-DPF.ps1 -BoardRevision V1` zwalidował YAML i skompilował aktualny firmware dla `CAN=true`, `NORMAL`, `OBD=true`, loggera USB 115200. Obraz aplikacji: 1 271 827 B; DIRAM 118 987 B (34,82%).
- Kompilator Xtensa nie uruchomił się w ograniczonym środowisku Windows z błędem relokacji DLL; ten sam lokalny skrypt wykonany poza sandboxem zakończył się `Successfully compiled program`. Nie zmieniano zabezpieczeń Windows ani plików systemowych.
- Firmware nie był ponownie wgrywany, bo bieżący build nie wnosi zmian do kodu urządzenia, a użytkownik potwierdził już ekran porównawczy po OTA. Kompilacja potwierdza bieżący workspace, lecz nadal nie weryfikuje odpowiedzi CAN/UDS ani parametrów z ECU CJCD.

## 79. Skan VCDS dla dokładnego numeru ECU znaleziony publicznie (10 października 2026)

- Publiczny skan podaje dokładnie `03L 906 018 SJ / HW 03L 906 018 JL / H25 9977 / EV_ECM20TDI01103L906018SJ 003003`, opisany jako CJCD. W freeze-frame związanym z P2452 widać ciśnienie DPF 1047 hPa, offsety 5 i 7 hPa, sadzę zmierzoną 19.29 g i obliczoną 38.21 g; inna ramka pokazuje 10 hPa.
- Są to odczyty w kontekście błędu czujnika różnicy ciśnień, nie normatywne wartości sprawnego DPF. Skan nie podaje dla tych pól IDE ani surowych DID-ów/odpowiedzi UDS. Nie zmieniono mapy ani skal firmware. Wynik obala jedynie założenie, że 24 g można bezwarunkowo traktować jako maksymalną możliwą wartość odczytu; nie dowodzi progu regeneracji ani prawidłowego zakresu CJCD.
- Źródło: [wątek MOTOR-TALK — pełny identyfikator ECU i ramki usterek](https://www.motor-talk.de/forum/drosselklappe-anlernen-t7332377.html?page=1). To źródło społecznościowe i pojedynczy przypadek; potrzebne są wartości z własnego ECU/OBDeleven.

## 80. Bieżący odczyt logów panelu po potwierdzeniu dostępności (10 października 2026)

- Skrypt `Czytaj-logi-DPF.ps1 -Device 192.168.0.166 -Seconds 12` połączył się z `/events` przez HTTP Digest; odpowiedź serwera: HTTP 200.
- Dwa raporty z okna: uptime 790679 → 795778 ms, bez resetu; CAN `state=1`, `RX=0/5s`, kolejki RX/TX=0, `RX missed=0`, `TX failed=0`, błędy RX/TX/bus=0; heap 176000 B, PSRAM 8291196 B.
- Użytkownik podał, że płytka jest odłączona od samochodu. `RX=0` nie jest więc próbą odbioru z ECU; potwierdzono tylko dostęp panelu, działanie runtime i spokojny stan CAN na stole.

## 81. Ograniczenie retransmisji TWAI w trybie sondowania DID (10 października 2026)

- Po uruchomieniu aktywnej sondy UDS bez samochodu wcześniejsze logi ponownie pokazały lawinowy wzrost `bus errors` (ok. 91 tys. na każde 5 s) i kolejkę TX 1–2. Sam 30-sekundowy odstęp aplikacji nie zatrzymywał retransmisji aktualnie nadawanej ramki przez kontroler TWAI.
- Usunięto blokadę czekającą na trzy ramki pasywne — magistrala diagnostyczna może nie nadawać niczego, dopóki tester nie wyśle żądania. Zabezpieczeniem transmisji jest teraz opcja komponentu `single_shot_when_quiet`: po 2 s bez odebranej ramki standardowej ramka jest nadawana najwyżej raz; po wykryciu ruchu CAN wraca zwykła semantyka retransmisji.
- ESPHome 2026.9.1 zwalidowało YAML, a kompilacja V1 `CAN=true`, `NORMAL`, `OBD=true` powiodła się (aplikacja 1 272 015 B, DIRAM 118 987 B / 34,82%). Firmware wgrano na `192.168.0.166`; OTA potwierdziło `OTA successful`.
- Log HTTP Digest przez ok. 35 s po OTA: uptime wzrastał, `queued TX=0` w każdym raporcie. `TX failed` i `bus errors` zmieniły się 1→2 przy pojedynczych niepotwierdzonych próbach, zamiast lawinowo narastać.
- Kolejny odczyt HTTP Digest przez 60 s: uptime wzrósł z ok. 77 s do 133 s bez restartu; `queued TX=0`, `RX=0`, `RX missed=0`; liczniki wzrosły tylko wraz z pojedynczymi próbami (TX failed/bus errors 3→5). Heap odzyskiwał poziom po krótkich wahaniach, PSRAM stabilny ok. 8,29 MB. Dotyk: raport FT3168 `unrecovered=0`, `slow(>=20ms)=0`, `max=2 ms`.
- Płytka nie była podłączona do auta, więc brak odpowiedzi DID jest oczekiwany. Potwierdzono ograniczenie retransmisji i stabilny runtime na stole, ale nie potwierdzono jeszcze odbioru CAN z samochodu, wartości DID ani ich zgodności z OBDeleven. Wcześniejsze sekcje 72–73 dokumentują kolejne, później zastąpione wersje zabezpieczenia sondy.

## 82. OBD-II i kandydackie DID-y w jednym trybie porównawczym (10 października 2026)

- Audyt wykazał, że tryb `dpf_did_diagnostic_probe` wcześniej ustawiał manager jako skonfigurowany i przez to pomijał standardowy poller OBD-II. Zmodyfikowano scheduler w `uds_audi.h`: w trybie porównawczym standardowe PID-y OBD-II i sześć kandydackich UDS DID są odpytane naprzemiennie; w każdym momencie aktywne jest najwyżej jedno żądanie. Odpowiedź jest kierowana do parsera na podstawie właściciela bieżącej transakcji, co chroni assembler ISO-TP przed pomieszaniem usług `0x22` i `0x01/0x09`.
- Kandydackie DID-y nadal publikują wyłącznie tekst diagnostyczny i nie wpływają na produkcyjne sensory ani alarm. Standardowe PID-y pokazują się na zwykłych ekranach (jeśli ECU je obsługuje); strona porównawcza nadal pokazuje sześć kandydatów.
- YAML przeszedł walidację i kompilację przez ESPHome 2026.9.1. Rozmiar obrazu aplikacji 1 272 192 B; DIRAM 118 987 B (34,82%). OTA na `192.168.0.166` zakończyło się `OTA successful`.
- Po restarcie strumień logów `/events` zwrócił HTTP 200. Przez ok. 12 s uptime wzrósł 37 458 → 42 566 ms bez restartu; `queued RX/TX=0`, `RX=0`, liczniki TX/bus wzrosły 15→16. OBD wykonało cztery żądania i cztery timeouty. To jest zgodne z testem na stole bez auta: potwierdza aktywność obu harmonogramów i brak zapełnienia kolejki, ale nie potwierdza danych ECU ani stabilności długoterminowej.
- Do sprawdzenia po powrocie do auta: potwierdzić stabilny ekran po OTA; na zwykłych kartach spisać standardowe PID-y oraz na karcie porównawczej sześć kandydatów; zachować log USB z surowymi odpowiedziami. Następnie porównać nazwy/jednostki z OBDeleven przy podobnym stanie silnika. Do tej chwili mapowanie DPF i alarm regeneracji dla `03L906018SJ / 9977 / 003003` pozostają niezweryfikowane.

## 83. Kontrola runtime po zmianie pollera (10 października 2026)

- Po OTA pobierano strumień `/events` przez HTTP Digest przez 60 s. W widocznych raportach uptime wzrósł 139 999 → 190 941 ms bez restartu; CAN `RX=0`, kolejki RX/TX=0, `RX missed=0`. W przybliżeniu 51 s obserwowanych raportów TX failed/bus errors wzrosły 52→70, RX errors pozostały 135, TX errors 0. Nie wystąpił przyrost kolejki.
- Poller OBD zgłaszał zwykle 3–4 żądania i 3–4 timeouty na raportowany interwał 10 s; sondy UDS kandydatów również kończyły się timeoutami. Płytka była poza autem, więc brak odpowiedzi jest oczekiwany. Pojedyncze próby nadawcze nie zapełniały kolejki, ale licznik błędów rośnie na cichej magistrali.
- FT3168 w raporcie okresowym: retries status=5, coordinates=0, unrecovered=0, slow(>=20ms)=0, max=3 ms. Ten raport dotyczy I²C dotyku, nie jest automatycznym dowodem wzrokowej poprawności ekranu.
- Użytkownik potwierdził stronę porównawczą po wcześniejszym OTA, ale jeszcze nie potwierdził obrazu po aktualnym buildzie z dwoma pollerami. Nie ma odpowiedzi z ECU ani porównania z OBDeleven. Kolejny rozstrzygający krok to wzrokowo potwierdzić ekran po ostatnim OTA, a po dostępie do auta sprawdzić, czy `RX` rośnie i czy na standardowych stronach pojawiają się PID-y.

## 84. Korekta opisu profilu i niedostępny cel OTA (11 października 2026)

- W panelu WWW zmieniono nazwę selektora na `Oznaczenie profilu ECU`, a wariant CJCD na `CJCD (DID niepotwierdzone)`. Wybór nadal tylko zapisuje oznaczenie i nie przełącza mapy; to zapobiega sugerowaniu, że istnieje już zweryfikowany profil dla ECU `03L906018SJ / 9977 / 003003`.
- Zaktualizowano opisy w `README.md` i `DPF_PROFILES.md`: bieżący tryb naprzemiennie odpytuje OBD-II oraz kandydackie UDS DID, a wymiana żądań jest serializowana.
- ESPHome 2026.9.1 zwalidowało YAML i skompilowało obraz V1 (`CAN=true`, `NORMAL`, `OBD=true`), rozmiar aplikacji 1 272 063 B, DIRAM 118 987 B (34,82%).
- Wysyłanie OTA pod wcześniejszy adres `192.168.0.166` zakończyło się timeoutem po trzech próbach. Odczyt `/events` na porcie 80 również nie połączył się. Odczyt portów szeregowych wykazał tylko Bluetooth COM3/COM4, bez portu płytki. Zmiana selektora została więc skompilowana lokalnie, ale nie wdrożona; aktualny stan ekranu/urządzenia nie jest w tej chwili znany.
- Komputer jest w Wi-Fi `192.168.0.66/24`, brama `192.168.0.1`; mDNS `audi-dpf-monitor.local` nie zwrócił adresu. Dwa sąsiednie hosty z otwartym portem 80 mają MAC `[MAC URZĄDZENIA USUNIĘTY]` (prefiks zarejestrowany dla Canon) oraz `[MAC URZĄDZENIA USUNIĘTY]` (prefiks zarejestrowany dla D&M/Denon). To nie potwierdza monitora; nie użyto wobec nich zapisanych poświadczeń ani nie pobierano danych. Adres monitora pozostaje nieznany.

## 77. Porównanie kandydackich DID z OBDeleven i log z jazdy (11 października 2026)

- Użytkownik porównał zdjęcie monitora (11:05:55) ze zrzutem OBDeleven (11:04:32), odstęp ok. 83 s. Monitor pokazywał `CAN: RUCH WYKRYTY`, 114F=1714, 114E=1677, 14F5=13, 1156=0.8 km, 11B2=75.3 °C, 10FB=15.0 °C.
- OBDeleven pokazywał sadzę obliczoną 17.13 g, zmierzoną 16.77 g, dystans od regeneracji 202256 m, różnicę ciśnienia 13 hPa i symulowaną temperaturę powierzchni DPF 66.3 °C.
- Dopasowanie: 114E przy 0.01 g/bit daje dokładnie 16.77 g; 114F daje 17.14 g, różnica 0.01 g wobec OBDeleven (odstęp próbek/zaokrąglenie możliwe); 14F5 zgadza się liczbowo jako 13 hPa. Wyniki wspierają te skale, lecz wymagają kolejnych porównań w innych stanach pracy.
- W późniejszym logu z jazdy odpowiedź 1156 ma bajty `00 03 A2 E9`; jako U32 big-endian to 238313. Stary ekran pokazywał 0.8–0.9 km, bo dekoder obcinał odpowiedź do trzech pierwszych bajtów i dzielił wynik przez 1000. Pełne U32 w metrach silnie pasuje do dystansu od regeneracji; firmware ma dekodować wszystkie cztery bajty i dzielić metry przez 1000.
- 11B2 i temperatura symulowana powierzchni DPF w OBDeleven nie są potwierdzone jako ten sam kanał, więc różnicy 75.3 vs 66.3 °C nie interpretujemy. 10FB nie ma odpowiednika na zrzucie.
- Kandydackie dane nadal są wyłącznie diagnostyczne: nie są publikowane do produkcyjnych sensorów i nie sterują alarmem `NIE GAŚ!`.

- Log z jazdy (14:54:29–14:54:39): CAN raportował `RX=12–13/5s`, bez strat RX i bez błędów RX/TX w tym oknie; skumulowane liczniki `TX failed` i `bus errors` wynosiły 3589 w obu raportach. Odpowiedzi kandydackich DID dochodziły: `114F=1904` (`19.04 g`), `114E=1857` (`18.57 g`), `14F5=14` (`14 hPa`), `1156=00 03 A2 E9` (`238313 m`), `11B2=146.3 °C`, `10FB=123.8 °C`.
- OBD raportował 19 żądań w interwale, 10 prawidłowych ramek ISO-TP i 10 timeoutów. To wskazuje na częściowe odpowiedzi/timeouty w standardowym pollerze, mimo że kandydackie zapytania UDS otrzymywały odpowiedzi.
- Wi-Fi w tym samym oknie nie znajdowało zapisanej sieci i ponawiało połączenie. To osobny problem łączności Wi-Fi, nie dowód awarii CAN/UDS.
- Wniosek dla 1156 jest mocny, ale wymaga jeszcze jednego porównania w zbliżonym czasie z OBDeleven, aby potwierdzić zmianę dystansu.

## Aktualizacja wdrożenia częściowego profilu CJCD (11 października 2026)

- Późniejsza zmiana względem wcześniejszych wpisów „wyłącznie diagnostycznych”: ręczny wybór profilu CJCD udostępnia teraz na zwykłych kartach `114F` (sadza obliczona), `114E` (sadza zmierzona) i `14F5` (ciśnienie), z jednostkami/skalą wspartymi pojedynczym porównaniem OBDeleven. Kolejne próbki nadal są potrzebne; profil nie steruje alarmem regeneracji.
- Konfiguracja ESPHome 2026.9.1 przeszła walidację i kompilację NORMAL V1, config hash `0xe1e6283b`, obraz 1 272 759 B, DIRAM 118 987 B / 34,82%. COM6 był zajęty, więc firmware wgrano OTA na `192.168.0.166`; transfer zakończył się `OTA successful`.
- Po OTA autoryzowany panel zwrócił HTTP 200 i opublikował stany sensorów; przy odłączonym samochodzie RPM, sadza i ciśnienie raportowały `NA`. Oczekuje na wzrokowe sprawdzenie ekranu po tym konkretnym restarcie.
- Do potwierdzenia w aucie pozostają: kolejne porównanie trzech wartości z OBDeleven, temperatury i dystans oraz przede wszystkim sygnał regeneracji/status PID `0x8B` w konkretnym ECU. Monitor nie może jeszcze traktować alarmu jako zweryfikowanego.
