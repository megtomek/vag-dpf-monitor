# Statystyka czasu trwania regeneracji — DID 12E7

Źródła: Downloads/monitor-dpf-can (2).csv i Screenshot_20261015-100100_OBDeleven.png. Pojedynczy kanał „Statystyka filtra cząstek stałych, czas trwania regeneracji”, na ekranie 0,00 s.

Zapis 79–34602 ms: 982 ramki; 491 żądań CAN 7E0 `22 12 E7` i 491 odpowiedzi CAN 7E8 `62 12 E7 00 00 00 00`. Dane wartości mają cztery bajty, stale zerowe. driver_rx_missed=0, skipped_while_locked=0, brak wykrytych problemów rekonstrukcji.

Potwierdzono adres kanału 0x12E7 oraz długość czterech bajtów. Zerowy zapis nie ustala typu liczbowego (np. integer vs float), kolejności bajtów, skali ani momentu aktualizacji i resetowania. Nazwa statystyki nie dowodzi licznika aktualnie trwającej regeneracji. Nie używać raw!=0 jako potwierdzenia aktywnego wypalania. Potrzebny odczyt niezerowy i zmiany w czasie podczas naturalnej regeneracji, wraz z referencją OBDeleven.

Raporty: capture-regen-duration-20261015-uds.csv i .md. Rejestratora na płytce nie zmieniano.
