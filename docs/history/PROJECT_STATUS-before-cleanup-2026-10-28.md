# Stan realizacji celu — aktualizacja 28 października 2026

## Zmiana UI w źródłach — 28.10

Na życzenie użytkownika pasek obliczonej sadzy ma zakres 0–24 g zamiast 0–40 g. 21 g oznacza 87,5% skali. Ograniczenie dotyczy tylko wypełnienia paska; liczba w gramach pozostaje pełnym odczytem. Nie zmieniono estymatora, rejestratora ani mapowania DID. Kompilacja NORMAL V1 i OTA 28.10 zakończone powodzeniem: config_hash 0xf5e87fb9, build_time_str 2026-10-28 10:53:33 +0100, obraz 1296960 B. Użytkownik potwierdził kartę DPF, dotyk i prawidłowe wypełnienie. Odbiór ECU wrócił, zachowane zdarzenie jest nienaruszone. Raport i SHA256: docs/DEPLOYMENT_2026-10-28_SOOT_BAR.md. To wdrożony zakres wersji v0.1.1; draft PR #6 nie został wgrany.

## Organizacja i wersje

Prywatne repozytorium megtomek/vag-dpf-monitor jest głównym miejscem zapisu źródeł i raportów. Gałąź main przechowuje bieżącą bazę, zadania mają gałęzie codex/<temat> i PR; reguły w CONTRIBUTING.md. Wersja źródeł v0.1.0 opisuje używalną bazę CJCD jako prerelease. Nie jest nowym wgraniem na urządzenie: porządkowanie dokumentacji 27.10 nie zmienia kodu firmware. Wcześniejsze README i dziennik planu zachowano w docs/history/. Surowe lokalne dane i sekrety wymagają własnej kopii poza GitHubem.

## Stan bieżący — nadrzędny wobec historycznych notatek poniżej

Wgrano rozszerzenie dowodów alarmu (0x0f0129ae, 12:16:36 +0100, 1305152 B): dystans, trend sadzy i rzeczywiste przełączenia z pętli 1 s. Trend nie steruje alarmem, schłodzenie i utrata danych są rozdzielone bez uznawania zakończenia za potwierdzone ECU. Osobny wersjonowany zapis dodatkowy zachowuje starsze zdarzenia — po OTA eksport starego zdarzenia identyczny, nowe pola przejazdu/WWW potwierdzone. Użytkownik potwierdził ekran, dotyk, nowe pole WWW i wyłączenie zasilania lub Wi-Fi — wyjaśnia to utratę połączenia. Pełne okno trendu i nowe naturalne zdarzenie pozostają do sprawdzenia. Zakres wydania v0.1.3. Raport: docs/REGEN_EVIDENCE_2026-10-28.md.

Poprawka FILTR/napięcia wgrana 28.10: 1156 był odczytywany diagnostycznie, ale nie publikowany do sensora dystansu. Bieżący log przed zmianą pokazuje 200,766 km (0003103E). Dodano publikację tylko przy zgodnym CALID CJCD i dokładnie czterech bajtach wartości; brak świeżego odczytu pozostaje --. Napięcie ECU z PID 42 na głównym ekranie i WWW, bez ADC i zmian okablowania. Nie jest to bezpośredni pomiar na klemach. Kompilacja i OTA udane (0xab6fd8c0, 11:49:12 +0100, 1298704 B); panel potwierdził 200,8 km i 11,5 V. Zapisane zdarzenie identyczne. Użytkownik potwierdził obie wartości, układ i dotyk. Kolejna korekta wgrana (0x5f1d120b, 12:02:08 +0100, 1301616 B): napięcie 36 px zamiast 18 px, bez ECU, przecinek dziesiętny. Raport: docs/DISTANCE_VOLTAGE_2026-10-28.md.

Poprawka HIST wgrana OTA 28.10: prezentacja utrwalonych zdarzeń NVS w trybie szacowanym, zamiast wiecznie pustego licznika ECU. Liczba 0–2 oznacza zajęte sloty, czas obejmuje cały zapis, a powód zakończenia odróżnia schłodzenie od przerwania. Starszy licznik ECU nie jest zmieniany. Kompilacja NORMAL V1 udana (0x4b5f5284, 28.10 11:32:04 +0100); OTA udane; urządzenie odtworzyło 1 zapis, 306 próbek i 1527 s. CSV identyczny przed/po; użytkownik potwierdził poprawne dane, mieszczące się napisy i działanie dotyku. Wersja v0.1.2. Raport: docs/DEPLOYMENT_2026-10-28_HISTORY.md.

Normalny firmware ma EGT3 DID 11B2, podstawowy alarm szacowany 550°C/30 s (wyjście <450°C/60 s), trwały rejestrator zdarzeń oraz bufor przejazdu RAM 120 próbek. Ostatnie wgranie 28.10 dodaje historię zachowanych zdarzeń szacowanych oraz zachowuje skalę paska 0–24 g do wcześniejszej bazy z responsywnym panelem WWW/AP i poprawką flagi połączenia ECU w DEMO. Nie jest to firmware pasywnego przechwytywania.

22.10 otrzymano pierwsze naturalne zdarzenie: 306 kompletnych próbek, 25 min 27 s, EGT3 maks. 638,7°C, sadza obliczona 21,01 → 6,73 g, zmierzona 19,18 → 5,85 g. Użytkownik potwierdził pojawienie się i zniknięcie komunikatu REGENERACJA DPF / NIE GAŚ. Jest to udana próba alarmu szacowanego, nie potwierdzenie flagi ECU. 115B/12E7/134F/11C1 pozostały zerowe przez cały zapis. Nie uznawać ich za użyteczny detektor na podstawie obecnych danych.

Potwierdzono wcześniej zawijanie bufora 120 próbek oraz identyczny TEST NVS po fizycznym odłączeniu zasilania. Pierwsze rzeczywiste zdarzenie pobrano w późniejszym uruchomieniu (boot zdarzenia 77, bieżące próbki od 5 s). Nie sprawdzano odłączenia podczas zapisu flash ani wszystkich scenariuszy utraty danych. Szczegóły: FIRST_REAL_EVENT_2026-10-22.md i DRIVE_AND_STORAGE_VERIFICATION_2026-10-15.md. Kopie źródeł: archive/diagnostics/first-event-2026-10-22/.

Pozostają: ocena powtarzalności i fałszywych alarmów w kolejnych przejazdach; znaczenie/obsługa rzeczywistego statusu ECU; pełne potwierdzenie pozostałych kanałów i profili; fizyczna ocena instalacji zasilania. Cel nie jest oznaczony jako ukończony. Nie zmieniamy progów po pojedynczej udanej próbie.

## Historyczne etapy (datowane obserwacje; nie opisują bieżącego firmware)

Projekt nie jest jeszcze ukończony. Aktualny firmware: normalny CAN/OBD, EGT3 11B2, szacowany alarm i automatyczny rejestrator zdarzeń NVS, hash `0x4c16cad9`, wgrany OTA na 192.168.0.105 (1296800 B), z zapisem zwykłego przejazdu i oddzielnym TESTEM NVS. Poniższe wyniki dotyczą różnych zakresów: kompilacja nie potwierdza działania w aucie, a DEMO nie potwierdza statusu ECU.

## Automatyczna rejestracja — 15 października

Aktualizacja 11:02:19: wgrano zapis ostatnich 120 próbek zwykłej jazdy (RAM, 5 s) i oddzielny TEST NVS, obraz OTA 1296800 B. TEST pobrano przed i po restarcie OTA oraz restarcie programowym: identyczny SHA256. Rzeczywiste odczyty EGT3, sadzy i czterech statusów potwierdzono w CSV po restarcie; CAN bez błędów w obserwowanym fragmencie. Pełne dowody i ograniczenia: DRIVE_AND_STORAGE_VERIFICATION_2026-10-15.md. To zastępuje wcześniejszą informację o nieprzetestowanej trwałości na poziomie próby syntetycznej, ale nie dowodzi jeszcze odzyskania naturalnego zdarzenia ani odporności na odłączenie zasilania podczas flashowania checkpointu.

Po aktualizacji użytkownik potwierdził: ekran i panel działają (odpowiedź na pytanie obejmujące czysty obraz, dotyk i przyciski zdarzeń). Potwierdzenie nie obejmuje naturalnej regeneracji, niepustego zapisu zdarzenia ani odzyskania checkpointu po wyłączeniu zasilania.

Użytkownik zatwierdził wdrożenie automatycznego zbierania danych, aby nie musiał ręcznie trafić w naturalną regenerację. Wgrano normalny firmware; pasywny rejestrator został zastąpiony. Endpoint event-status zwrócił 200 i brak zdarzenia, event.csv zwrócił oczekiwane 404 bez zdarzeń, log potwierdził Event recorder ready. Nie przetestowano jeszcze rzeczywistego zdarzenia, pobrania niepustego CSV, zapisu checkpointu ani jego odzyskania po odłączeniu zasilania.

Odczytywane dodatkowe DID: 115B, 12E7, 134F, 11C1; są przechowywane jako raw, nie decydują o alarmie. Próbek po OTA nie otrzymano: RX=0, rosną RX errors/bus errors i są timeouty. Użytkownik pytał o zgaszenie silnika, więc stan zapłonu/zasilania ECU wymaga potwierdzenia przed rozpoznaniem nowej usterki. Poprawne CAN po zamianie TX/RX potwierdza wcześniejszy can-wiring-fixed-20261015.txt, nie bieżący brak danych.

Opis działania i ograniczeń: AUTOMATIC_DPF_EVENTS.md. Próbka co 5 s, bufor do 3 min przed zdarzeniem, start zapisu >=450 °C/15 s, stop <450 °C/120 s lub utrata świeżości/limit 900 próbek. Checkpoint NVS na starcie, co 60 s i na końcu. Dwa sloty, pobierany najnowszy. Do 60 s ostatnich danych może zniknąć przy nagłym odłączeniu, o ile wcześniejsze checkpointy się powiodły. Sam alarm nadal szacowany: >=550 °C/30 s, stop <450 °C/60 s. Potwierdzenie znaczenia bitów oraz kalibracja progów pozostają otwarte.

## Pasywny rejestrator — stan 12 października

### Nowy materiał i propozycja wykrywania szacowanego — 14 października

Użytkownik zatwierdził wykrywanie szacowane jako podstawowe. Dodano RegenEstimator i podłączono stan do UI oraz źródła alarmu w panelu; domyślnie estimated_regen_enabled=true. Aktualna implementacja opiera alarm na świeżym EGT3: >=550 °C przez 30 s, wyjście <450 °C przez 60 s, reset po utracie danych. Trend sadzy nie jest jeszcze używany w decyzji. Cykle szacowane nie są dopisywane do historii jako potwierdzone przez ECU. Brak odczytu EGT3 daje unavailable, nie DPF OK. ECU CJCD nie odpowiedział na PID 78 w zachowanych logach; właściwy DID EGT3 nadal trzeba ustalić. Dotychczasowy rejestrator na urządzeniu pozostaje potrzebny i nie został zastąpiony tą implementacją.

Użytkownik przesłał eksport OBDeleven chart.csv: 2383 próbki, EGT3 74,3–631,6 °C, sadza zmierzona 13,02 → 6,28 g. Kanały „Aktualnie trwająca procedura” i „masa sadzy przy uruchomieniu regeneracji” wynoszą stale 0. Zapis sugeruje wypalanie, ale nie ustala statusu aktywnej regeneracji ani DID EGT3. Jednostka kolumny time nie jest podana w nagłówku; przy założeniu ms czas wynosi 17 min 40 s.

Użytkownik zaproponował wykrywanie po temperaturze zamiast statusu DID. Omówiono alternatywę: próg 550 °C przez 30 s, dodatkowy trend spadku sadzy, wyjście poniżej 450 °C przez 60 s i wygaszenie po utracie świeżości. To niekalibrowana propozycja, nie zaimplementowany ani potwierdzony algorytm CJCD. Wymaga decyzji o wdrożeniu trybu szacowanego i ustalenia źródła EGT3. Oryginalny cel statusu potwierdzonego przez ECU pozostaje niezrealizowany; tryb szacowany nie zastępuje jego weryfikacji.

Wgrano obraz 1285824 B, ESPHome potwierdziło OTA successful. Po restarcie GET /dpf/can-status zwrócił HTTP 200 i active=false. POST START i STOP zwróciły 200; pobrano CSV 331 B, 0 ramek, driver_rx_missed=0. Ta krótka próba potwierdza endpointy i format nagłówka, nie odbiór komunikacji auta ani odtwarzanie ISO-TP. Użytkownik potwierdził, że rejestrator jest wgrany i wygląda poprawnie.

Firmware wymusza LISTEN_ONLY, wyłącza poller i callbacki UDS. Normalne odczyty DPF są w tej wersji wyłączone. Zapis w PSRAM: 120 sekund lub 65536 ramek; START kasuje poprzedni zapis, RESET kasuje cały bufor. Instrukcja CAN_CAPTURE.md; analizator analizuj_can_capture.py nie został jeszcze sprawdzony na rzeczywistym zapisie OBDeleven.

Następny dowód: CSV po równoczesnym podłączeniu OBDeleven i monitora przez zamawiany przez użytkownika rozdzielacz OBD-II. Najpierw ustalić widoczność żądań oraz odpowiedzi ECU, następnie DID wybranego kanału. Status regeneracji i znaczenie bitów pozostają niepotwierdzone; alarm wymaga dalszej weryfikacji.

| Wymaganie | Obecny dowód | Co pozostaje |
|---|---|---|
| Samodzielne działanie bez Home Assistant | YAML nie zawiera `api:`; lokalny ekran, dotyk i ustawienia działają według potwierdzeń użytkownika | Dłuższa próba w samochodzie |
| Poziomy czytelny ekran 1,64 cala | LVGL 90°, pięć kart, fonty z polskimi znakami; użytkownik potwierdził czytelność i działanie dotyku | Zdjęcie najnowszej wersji i ocena wszystkich kart z rzeczywistymi danymi |
| Dane DPF z ECU | Wcześniejsze zdjęcia: 114F 17,14 g wobec OBDeleven 17,13 g; 114E 16,77 g zgodnie; 14F5 13 hPa zgodnie | Powtórzenie przy innych stanach silnika; ujemna sadza zmierzona |
| Dystans i temperatury | 1156 poprawiono na U32 BE/metrach; 11B2 i 10FB mają odpowiedzi z ECU | Porównanie dystansu po poprawce; identyfikacja odpowiednich kanałów temperatur, bez utożsamiania temperatury powierzchni z wlotem |
| Alarm „REGENERACJA DPF” / „NIE GAŚ!” | Kod wymaga świeżego PID 0x8B i obsługi pól statusu/typu; DEMO sprawdziło komunikaty | Odczyt aktywnej regeneracji z własnego ECU i porównanie z OBDeleven; zanik danych i zakończenie procesu |
| Wybór profilu ECU/silnika | Panel udostępnia profil uniwersalny i częściowy CJCD; nowy status zgodności CALID odczytany przez HTTP | CALID 03L906018SJ9977 potwierdzono w logu przejazdu; inne profile wymagają własnych map i pomiarów |
| Ustawienia przez Wi-Fi/AP | Panel HTTP działa; trzy tryby przełączają się; użytkownik potwierdził OFF po RESET | AP po RESET i zapis po całkowitym odłączeniu zasilania |
| Nowoczesny i czytelny panel WWW | Lokalne zasoby panelu i funkcjonalne ustawienia istnieją | Bieżąca ocena wizualna: Computer Use zatrzymało podgląd z powodu nierozpoznanego URL; potrzebny zrzut panelu |
| Zasilanie samochodowe 5 V i wspólna masa | Istnieje schemat; urządzenie uruchomiło się w samochodzie | Weryfikacja fizycznego montażu i zasilania podczas pracy; samo uruchomienie nie potwierdza odporności układu zasilania |

## Następny test, który rozstrzyga brakujące punkty

1. Podłączyć monitor do auta, wyłączyć DEMO, wybrać CJCD. Zachować CALID i log z rzeczywistymi odpowiedziami.
2. Porównać sadzę, ciśnienie, dystans i odpowiadające kanały temperatur z OBDeleven przy możliwie podobnym stanie silnika. Dwie kolejne próbki nie są jednoczesnym pomiarem.
3. Ustalić właściwy parametr UDS statusu regeneracji dla tego ECU. W logu car-archive-20261012.txt próba PID 0x8B zakończyła się timeoutem; nie zakładać, że kolejne przejazdy dostarczą odpowiedzi tego PID. Podczas naturalnej regeneracji zachować status OBDeleven i log monitora. Po ustaleniu dekodera zweryfikować alarm na każdej karcie, zanik danych i zakończenie procesu. Nie uruchamiać regeneracji serwisowej wyłącznie na potrzeby monitora.
4. Zebrać zdjęcia ekranu i panelu WWW. To domknie ocenę czytelności, której nie zastępują logi HTTP.

Zapis logów: instrukcja i polecenie w `ENVIRONMENT.md`. Bez tych pomiarów alarm i pozostała mapa DPF są niepotwierdzone; projektu nie należy oznaczać jako ukończonego.

## Przejazd 12 października i przygotowana diagnostyka

Log potwierdza odpowiedzi ECU: sadza obliczona 19,39–19,51 g, zmierzona 19,33–19,65 g, różnica ciśnień 18–93 hPa. Kandydat dystansu pokazuje 241,5–242,9 km; dystans i temperatury nadal wymagają porównania z OBDeleven. CALID wynosi 03L906018SJ9977. Nie ma dowodu aktywnej regeneracji.

Kolejki CAN pozostają puste, liczniki TX failed i bus errors nie rosną w zachowanym fragmencie. Bufor 1024 wpisów zachował około 3 min 48 s, mimo limitu wieku 10 minut. Przygotowano i skompilowano ograniczenie powtarzanych wpisów DID oraz logowanie numeru PID i trybu przy timeoutach. Poprawkę wgrano 12 października przez OTA na 192.168.0.105 (1285904 B, OTA successful). Po aktualizacji endpoint /events zwrócił HTTP 200; w 15-sekundowym przechwyceniu nie otrzymano wpisów logów, więc nie potwierdza ono pracy CAN ani wyglądu ekranu.

Użytkownik podał aktualny adres 192.168.0.105. Stały adres 192.168.0.166 najlepiej zapewnić rezerwacją DHCP w routerze dla MAC monitora; dotyczy ona tej konkretnej sieci. Nie wpisano statycznego adresu w firmware, ponieważ monitor korzysta także z hotspotu, a jego podsieć może być inna. Sprawdzenie OTA z bieżącego środowiska nie powiodło się i zgłosiło ograniczenie dostępu CIM — nie dowodzi to, że monitor jest wyłączony.
## Aktualizacja etykiety temperatury — 12 października

Firmware 0x76ce7287 (1286032 B OTA) wgrano na 192.168.0.105; po pierwszym zerwanym transferze drugie ponowienie zakończyło się OTA successful. TEMP pokazuje TEMP 11B2? dla niepotwierdzonego źródła UDS. Wygląd po tej aktualizacji i mapa temperatur wymagają potwierdzenia. Krótki odczyt czytnika SSE przed aktualizacją zakończył się poprawnie (HTTP 200, kod 0).

## Potwierdzenie użytkownika i dane porównawcze

Po RESET użytkownik potwierdził powrót UI i panelu. Zrzuty OBDeleven z 12 października zapisano w OBDELEVEN_REFERENCE_2026-10-12.md. Podczas odczytów temperatur silnik pracował. Nie ma jeszcze pary odczytów monitora i OBDeleven w tym samym stanie potwierdzającej temperatury ani dystans. Kod zachowuje pełną odpowiedź PID 0x8B A–G na poziomie INFO; następny log z auta pozwoli ustalić odpowiedź albo konkretny timeout PID. Nie należy wnioskować o regeneracji z samej masy sadzy lub progu 21,08 g.
