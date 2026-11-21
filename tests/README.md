# Testowe konfiguracje ESPHome

Pliki w tym katalogu służą do historycznych testów stołowych. Produkcyjny firmware znajduje się w głównym `audi-dpf-monitor.yaml`; przed wgrywaniem zawsze sprawdź nazwę konfiguracji i opcje kompilacji.

## Pliki

- `audi-dpf-monitor-can-*.yaml` — test magistrali CAN i zachowania przy braku ruchu.
- `audi-dpf-monitor-ui-*.yaml` — test stabilności interfejsu.
- `audi-dpf-monitor-wifi-*.yaml` — historyczne testy Wi-Fi; nie odzwierciedlają bieżącej konfiguracji produkcyjnej.
- `display-diagnostic-*.yaml` — testy obrotu LVGL i konfiguracji wyświetlacza.

Ścieżki lokalnych komponentów i fontów wskazują `../components/` oraz `../fonts/`. Archiwalne firmware odpowiadające wcześniejszym testom znajduje się w `../firmware/archive/`.

`tests/secrets.yaml` jest lokalny i ignorowany przez Git. Powstał z `secrets.example.yaml`, więc zawiera wyłącznie wartości zastępcze. Używaj go tylko przy świadomym uruchamianiu historycznych testów Wi-Fi. Aktywny `../audi-dpf-monitor.yaml` odczytuje własne sekrety z `../secrets.yaml`, który zawiera dane produkcyjne i nigdy nie powinien być kopiowany do katalogu testów ani publikowany.

Testy CAN nie zastępują porównania odczytów ECU z OBDeleven. Konfiguracje Wi-Fi są historyczne i nie powinny być używane jako baza dla firmware samochodowego.
