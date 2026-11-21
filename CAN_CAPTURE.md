# Pasywny zapis komunikacji OBDeleven

Osobny wariant firmware: `audi-dpf-monitor-capture.yaml`, albo kompilacja przez `Kompiluj-monitor-DPF.ps1 -PassiveCapture`. Skrypt zapisuje artefakty z nazwą zawierającą `passive-capture`, bez nadpisywania nazw normalnego firmware. Wariant nie jest przeznaczony do normalnych odczytów DPF.

## Działanie

- TWAI wymusza LISTEN_ONLY: nie nadaje ramek ani ACK. Własny poller i callbacki UDS (w tym ISO-TP Flow Control) są wyłączone.
- Zapis obejmuje wszystkie odebrane identyfikatory, ramki standardowe i rozszerzone. Nie filtruje wyłącznie odpowiedzi 0x7E8: potrzebne są też żądania OBDeleven.
- Bufor PSRAM: 65536 ramek lub 120 sekund; zatrzymanie po osiągnięciu limitu. Zapis nie uruchamia się sam po starcie urządzenia. RESET/odłączenie zasilania kasuje dane.
- CSV: czas w ms od START, ID w hex, flaga extended, RTR, DLC i dane hex. Nagłówek podaje straty kolejki sterownika i pominięcia przy zajętym buforze. Brak strat nie gwarantuje odbioru każdej fizycznej ramki. Plik może zawierać identyfikatory pojazdu.
- Pobieranie wymaga zatrzymania zapisu. START kasuje poprzedni zapis. Limit chroni pamięć; to nie rejestrator całego przejazdu.

## Procedura po otrzymaniu rozdzielacza OBD-II

Wgrywanie przez skrypt: `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Wgraj-monitor-DPF.ps1 -CanBusEnabled true -PassiveCapture -Transport OTA -Device ADRES_MONITORA`. Dla USB użyć `-Transport USB -Device COM6` (numer portu sprawdzić przed użyciem). Skrypt kompiluje wariant pasywny i przekazuje ten sam wybór do ESPHome podczas uploadu. Nie wykonywać polecenia z przykładowym adresem lub nieaktualnym portem.

1. Wgrać wariant pasywny. Podłączyć monitor i OBDeleven równocześnie przez rozdzielacz; zachować istniejące połączenia transceivera i wspólną masę. Nie dodawać terminatora CAN do instalacji auta.
2. Włączyć zapłon i połączyć OBDeleven ze sterownikiem 01. Silnik nie jest wymagany do pierwszego ustalenia DID. Nie uruchamiać regeneracji serwisowej.
3. Otworzyć panel monitora przez router lub AP. Sekcja CAN pasywny udostępnia START, STOP i Pobierz CAN CSV. Oba tryby sieci używają tych samych przycisków oraz uwierzytelnienia.
4. Rozpocząć zapis, w OBDeleven wyświetlić tylko jeden wybrany parametr przez 30 sekund, zrobić zrzut nazwy i wartości. Zatrzymać zapis i pobrać CSV. Nadać plikowi nazwę odpowiadającą parametrowi.
5. Zacząć od polecenia uruchomienia regeneracji, następnie osobno czas trwania i warunki przerwania podczas jazdy. Nowy zapis dopiero po pobraniu poprzedniego.
6. Najpierw sprawdzić, czy CSV zawiera żądania oraz odpowiedzi ECU. Ramki innych modułów lub sam licznik ruchu nie potwierdzają widoczności komunikacji OBDeleven za bramką samochodu.

Endpointy: POST `/dpf/can-start`, POST `/dpf/can-stop`, GET `/dpf/can-status`, GET `/dpf/can.csv`. Dostępne tylko w wariancie capture; chronione tym samym Digest co panel.

Przechwycenie pozwoli ustalić DID i długość odpowiedzi. Znaczenie bitów aktywnej regeneracji wymaga opisu lub porównania podczas naturalnego wypalania. Sam zapis zerowych odpowiedzi nie wystarczy do uruchomienia alarmu.

## Analiza pobranego pliku

`python analizuj_can_capture.py monitor-dpf-can.csv` tworzy `monitor-dpf-can-uds.csv` oraz raport `.md`. Skrypt używa wyłącznie biblioteki standardowej Pythona i nie łączy się z urządzeniem. Składa standardowe wiadomości ISO-TP na identyfikatorach 7DF/7E0–7EF, wypisuje żądania 22, odpowiedzi 62, odpowiedzi negatywne 7F i sygnalizuje braki/sekwencję ramek. Inne identyfikatory są zliczane, ale nie są automatycznie interpretowane jako ISO-TP. Adresowanie rozszerzone/mieszane wymaga osobnego mapowania. Odpowiedzi na wiele DID nie są dzielone bez znajomości długości każdego kanału. Brak problemów w raporcie nie potwierdza poprawności mapy ECU.

Po zakończeniu pomiarów wgrać normalny wariant z `can_capture_enabled: false`, CAN NORMAL i własnym odpytywaniem włączonym. Wgrywanie jest osobnym krokiem; przygotowanie plików nie potwierdza próby na urządzeniu.
