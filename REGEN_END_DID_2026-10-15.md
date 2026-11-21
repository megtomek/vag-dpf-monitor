# Statystyka statusu końca regeneracji — DID 134F

Źródła: Downloads/monitor-dpf-can (3).csv oraz Screenshot_20261015-100402_OBDeleven.png. Pojedynczy kanał „Statystyka filtra cząstek stałych, status końca regeneracji”, wartość 0, bity 0–7 równe 0.

Zapis 13–37055 ms: 840 ramek, 420 żądań 7E0 `22 13 4F`, 420 odpowiedzi 7E8 `62 13 4F 00`. Wartość ma jeden bajt, stale 00. driver_rx_missed=0, skipped_while_locked=0, brak wykrytych błędów rekonstrukcji.

Potwierdzono adres 0x134F i długość wartości. Nie ustalono znaczenia bitów, trwałości statystyki, ani czy zmiana oznacza sukces, przerwanie lub inny wynik. Nie traktować zera jako dowodu braku regeneracji lub jej ukończenia. Wymagana próbka przy zakończeniu naturalnego wypalania. Nie dodano kanału do decyzji alarmowej; rejestrator pozostaje na urządzeniu.
