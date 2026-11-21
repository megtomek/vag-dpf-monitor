# Plan rozwoju

Aktualizacja: 28.10.2026. Bazą jest używalny monitor CJCD z alarmem szacowanym. Stan dowodów: [PROJECT_STATUS.md](PROJECT_STATUS.md). Zadania śledzimy w [GitHub Issues](https://github.com/megtomek/vag-dpf-monitor/issues), a wynik wdrożeń w raportach i changelogu.

Otwarte zadania: [#1 powtarzalność alarmu](https://github.com/megtomek/vag-dpf-monitor/issues/1), [#2 status ECU](https://github.com/megtomek/vag-dpf-monitor/issues/2), [#13 porównania parametrów](https://github.com/megtomek/vag-dpf-monitor/issues/13), [#4 instalacja i dłuższa praca](https://github.com/megtomek/vag-dpf-monitor/issues/4).

## P1 — powtarzalność wykrywania i zapisów

- Zebrać kolejne naturalne wypalania oraz fragmenty normalnej jazdy, zwłaszcza pod większym obciążeniem.
- Ocenić pojawienie się i wygaszenie alarmu, utratę danych oraz trwałość rzeczywistego zdarzenia.
- Porównać temperaturę i spadek sadzy. Nie zmieniać progów na podstawie samego pojedynczego wyniku. Rozszerzenie 28.10 zbiera dystans, trend 2–3 min i rzeczywiste przełączenia alarmu; trend pozostaje obserwacyjny. Nowe trwałe zdarzenie wymaga próby naturalnej.
- Warunek zamknięcia: udokumentowane kolejne przypadki, ocena pominięć i fałszywych alarmów, jasno opisany pozostały zakres niepewności. Liczba zebranych plików sama nie dowodzi skuteczności.

## P1 — rzeczywisty status ECU

- Ustalić, czy ECU CJCD/9977 udostępnia użyteczny aktualny status regeneracji i jak go interpretuje.
- Obecne 115B/12E7/134F/11C1 były zerowe przez całe pierwsze zdarzenie; nie implementować raw!=0 jako potwierdzonego alarmu.
- Warunek zamknięcia: źródło opisu zgodne z ECU oraz porównanie nieaktywny → aktywny → zakończony, z długością odpowiedzi i mapą stanów. Nie wywoływać serwisowej regeneracji tylko po to, by zebrać dane.

## P2 — dalsze porównania parametrów (#13)

- Porównać przyrost/reset dystansu, ciśnienie, obie masy sadzy (w tym wartości ujemne, jeśli wystąpią) i napięcie z odniesieniem.
- Zapisywać stan silnika, czas i jednostki oraz aktualizować [wspólną tabelę parametrów](PARAMETER_REFERENCE.md).
- Semantyka HIST i jej czytelność są wykonane: #3 zamknięto, PR #10 i #12 wdrożone. To nie zamyka weryfikacji nowych naturalnych zdarzeń (#1).

## P2 — instalacja samochodowa i dłuższa praca

- Udokumentować fizyczne zasilanie, wspólną masę i zachowanie przy uruchamianiu/wyłączaniu samochodu.
- Sprawdzić dłuższą pracę, wznowienie komunikacji i zachowanie rejestratora przy przerwaniu zdarzenia.
- Odnotować rzeczywistą rewizję PCB, jeżeli uda się ją ustalić. Nie utożsamiać jej z rewizją krzemu.

## Odłożona poprawka techniczna — draft PR #6

Nadal potrzebna dla konfiguracji bez opcjonalnych sensorów RPM/płynu/BARO, ale nie została wykazana jako przyczyna dotychczasowych problemów bieżącego YAML. Nie jest zastąpiona przez v0.1.3 i nie jest na urządzeniu. Przed wznowieniem: aktualizacja do main, zachowanie nowych odczytów/rejestratora oraz uzgodniona weryfikacja integracji. [Stan PR #6](https://github.com/megtomek/vag-dpf-monitor/pull/6). Nie scalać starej gałęzi tylko w celu zamknięcia listy.

## P3 — nowe profile i funkcje

- Dodawać profile dopiero z mapą DID, dekodowaniem i opisanym zakresem potwierdzenia.
- Zachować kontrolę zgodności ECU/CALID oraz rozróżnienie danych potwierdzonych i kandydatów.
- Nowe funkcje planować jako osobne zadania, bez pogarszania istniejącego odczytu, alarmu i rejestracji.

## Osiągnięte etapy

- CAN i profil CJCD, porównania podstawowych parametrów z OBDeleven.
- Ustalenie DID EGT3 11B2 przez pasywne przechwycenie pojedynczego kanału.
- Czytelna karta TEMP, lokalny panel WWW/AP, OTA, trzy zapamiętywane tryby sieci.
- Rejestrator zdarzeń NVS i bufor 120 próbek przejazdu.
- TEST NVS przetrwał fizyczne odłączenie zasilania.
- Pierwsze naturalne zdarzenie: EGT3 maks. 638,7°C, spadek obu mas sadzy; użytkownik potwierdził alarm i jego zanik.

Wcześniejszy dziennik prac zachowano w [historii planu](docs/history/ROADMAP-before-organization-2026-10-27.md). Nie jest listą aktualnie otwartych zadań.
