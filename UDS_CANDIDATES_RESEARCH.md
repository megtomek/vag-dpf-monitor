# Kandydaci UDS — źródło sprawdzone 12 października 2026

Źródło: https://github.com/v-cu/dpf-load-monitor-wide/blob/main/firmware/main/can_dpf.c
Nie jest to potwierdzenie dla 03L906018SJ/9977. Zapisano definicje jako tropy, bez dodawania odpytywania do urządzenia.

| DID | Nazwa zmiennej w źródle | Uwagi |
|---|---|---|
| 10F9 | DPF_out_temp | Kandydat temperatury za DPF; wzór (U16-2731)/10 |
| 115E | Last_regen_min | Kandydat czasu od regeneracji; źródło odczytuje 24 bity i dzieli przez 60; trzeba ustalić pełną długość odpowiedzi, nie powielać truncacji |
| 1153 | Oil_Ash | Kandydat popiołu; źródło odczytuje U16, jednostka/skala wymagają sprawdzenia |
| 163B | Post_Inj2 | Kandydat dawki dotrysku, nie flaga statusu regeneracji |
| 167D | Post_Inj3 | Kandydat dawki dotrysku, nie flaga statusu regeneracji |

Tabela nie zawiera bezpośredniej flagi regeneracji. Deklaracja alertu w README nie dowodzi, że korzysta on z flagi ECU. Potrzebny przegląd logiki UI przed ustaleniem mechanizmu. Nie używać dotrysków jako potwierdzonej flagi NIE GAŚ bez danych referencyjnych.

Powiązanie kandydatów z kanałami OBDeleven jest niepotwierdzone. Obecne wartości odniesienia: popiół 102,1 g, czas 28471 s. Nie kopiowano kodu źródłowego do firmware.
## Mechanizm alertu w projekcie źródłowym

Sprawdzono firmware/main/ui_logic.c: dpf_calc_regen_process ustawia DPF_ACTIVE, gdy Post_Inj2 != 0 i Last_regen_km != 0; po 25 s bez dotrysku wraca do IDLE. dpf_calc_passive_regen wnioskuje ze spadku sadzy i temperatur wlotu/wylotu >=300 °C. To wnioskowanie z pomiarów, nie bezpośrednia flaga regeneracji ECU. Nie spełnia samo w sobie wymagania naszego celu dotyczącego statusu potwierdzonego przez sterownik. Źródło: https://github.com/v-cu/dpf-load-monitor-wide/blob/main/firmware/main/ui_logic.c
