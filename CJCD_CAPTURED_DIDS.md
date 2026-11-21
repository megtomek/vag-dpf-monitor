# CJCD — kanały przechwycone z OBDeleven

Aktualne zestawienie wszystkich parametrów, skal i ich użycia: [PARAMETER_REFERENCE](PARAMETER_REFERENCE.md). Poniżej pozostają datowane dowody przechwyceń, nie pełna aktualna lista funkcji firmware.

ECU: 03L906018SJ / 9977, ODX EV_ECM20TDI01103L906018SJ / 003003. Pomiary 15 października 2026. Powiązanie nazwy z DID pochodzi z pojedynczego kanału wyświetlanego w OBDeleven i równoczesnego pasywnego zapisu żądań 7E0/odpowiedzi 7E8.

| Kanał | DID | Bajty wartości | Odczyt i zakres potwierdzenia |
|---|---|---:|---|
| Czujnik temperatury spalin 3 | 11B2 | 2 | U16 BE: (raw-2731)/10; 0E6A = 95,9 °C zgodne ze zrzutem. Próbki 95,5–96,2 °C. Nie ustalono fizycznej lokalizacji czujnika ani pełnej skali w wysokiej temperaturze. |
| Status polecenia regeneracji podczas jazdy | 115B | 1 | Wszystkie odpowiedzi 00; zgodność z ekranem 0. Bity i znaczenie stanu niepotwierdzone. |
| Statystyka czasu trwania regeneracji | 12E7 | 4 | Wszystkie odpowiedzi 00000000; ekran 0,00 s. Typ liczby, skala, kolejność bajtów i okres statystyki niepotwierdzone. |
| Statystyka statusu końca regeneracji | 134F | 1 | Wszystkie odpowiedzi 00; ekran 0. Znaczenie bitów i trwałość wyniku niepotwierdzone. |
| Status blokowania regeneracji podczas jazdy | 11C1 | 1 | Wszystkie odpowiedzi 00; ekran 0 i bity 0–7 równe 0. Znaczenie bitów niepotwierdzone. |

W każdym przechwyceniu liczniki driver_rx_missed i skipped_while_locked wynosiły 0; analizator nie wykrył problemów rekonstrukcji w obsługiwanym adresowaniu. Szczegóły oraz liczby ramek: EGT3_DID_CONFIRMED_2026-10-15.md, REGEN_COMMAND_DID_2026-10-15.md, REGEN_DURATION_DID_2026-10-15.md i REGEN_END_DID_2026-10-15.md.

Zera statusów nie są mapą stanów i nie stanowią podstawy do interpretacji raw!=0 jako aktywnej regeneracji. Nazwy kanałów statystycznych nie oznaczają automatycznie bieżącego statusu procesu.

## Dystans — uzupełnienie 28.10.2026

DID 1156: dokładnie cztery bajty U32 big-endian, metry / 1000 = km. Podstawa: OBDELEVEN_REFERENCE_2026-10-12.md — 0003CD12 = 249106 m zgodne z późniejszym zrzutem OBDeleven; pomiary nie były jednoczesne. Po regeneracji w pierwszym zapisie nowego uruchomienia: 0000001A = 26 m. Odczyt 28.10 przed poprawką: 0003103E = 200766 m. Nowa publikacja na FILTR wymaga zgodnego CALID CJCD. Pozostaje porównanie zmiany dystansu podczas jazdy; nie jest to nowa flaga statusu regeneracji. Wcześniejsze wzmianki „tylko diagnostycznie” opisują poprzedni firmware.

## Wynik naturalnego zdarzenia otrzymanego 22.10.2026

W 306 próbkach kompletnego zdarzenia (EGT3 do 638,7°C, spadek obu mas sadzy) 115B=00, 12E7=00000000, 134F=00 i 11C1=00 przez cały fragment. Użytkownik potwierdził działanie alarmu szacowanego na ekranie. Te kanały nie potwierdziły aktywnej regeneracji; nie traktować zera jako dowodu braku wypalania. Raport FIRST_REAL_EVENT_2026-10-22.md. Normalny firmware został wgrany; poniższe punkty pochodzą z wcześniejszego etapu przechwytywania.

## Wcześniejszy plan pomiarów

1. Status blokowania zidentyfikowano: zapis monitor-dpf-can (4).csv, 19–37274 ms, 331 żądań `22 11 C1` i 331 odpowiedzi `62 11 C1 00`. Zrzut Screenshot_20261015-100654_OBDeleven.png. Bez zgłoszonych strat i problemów rekonstrukcji; raport capture-regen-block-20261015-uds.md. Pozostała interpretacja podczas zmian stanu, nie samo ustalenie adresu.
2. Niezerowe odczyty 115B/12E7/134F podczas naturalnego wypalania i jego zakończenia, zestawione z EGT3 oraz sadzą. Nie uruchamiać regeneracji serwisowej tylko na potrzeby badania.
3. Po zakończeniu zbierania DID: wgranie przygotowanego normalnego firmware i porównanie EGT3 na ekranie monitora z OBDeleven. Obecnie użytkownik wybrał pozostawienie rejestratora na urządzeniu.

Temperaturowy alarm szacowany jest implementacją zaakceptowaną przez użytkownika, ale wymaga próby w aucie. Ustalenie DID EGT3 nie jest potwierdzeniem skuteczności wykrywania regeneracji.
