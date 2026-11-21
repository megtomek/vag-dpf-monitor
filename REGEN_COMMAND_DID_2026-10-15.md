# Status polecenia regeneracji podczas jazdy — DID 115B

Źródła: Downloads/monitor-dpf-can (1).csv oraz Screenshot_20261015-095353_OBDeleven.png. Wybrany pojedynczy kanał: „Filtr cząstek stałych, status polec. regeneracji podczas jazdy”. Wartość na zrzucie 0, bity 0–7 równe 0.

Przechwycenie 3–34530 ms: 1257 ramek, 628 żądań CAN 7E0 `22 11 5B`, 629 odpowiedzi CAN 7E8 `62 11 5B 00`. Wszystkie odpowiedzi mają jeden bajt wartości 00. driver_rx_missed=0, skipped_while_locked=0; brak wykrytych problemów rekonstrukcji.

Potwierdzono adres kanału 0x115B i długość wartości jednego bajtu dla tego ECU. Nie potwierdzono znaczenia poszczególnych bitów ani powiązania żądania z faktycznie trwającym wypalaniem. Nie implementować reguły raw!=0 -> aktywna regeneracja na podstawie wyłącznie tego zapisu. Nie utożsamiać automatycznie ze statystycznym kanałem „polecenie uruchomienia regeneracji” z wcześniejszych zrzutów.

Następna próba rozstrzygająca: zachowanie 115B podczas naturalnej regeneracji, zestawione z EGT3 11B2, trendem sadzy i odczytem OBDeleven. Przydatny również osobny zapis kanału czasu trwania regeneracji. Firmware pozostaje rejestratorem zgodnie z decyzją użytkownika.
