# Pochodzenie integracji FT3168

Stan przeglądu: 21.11.2026, bez zmiany sterownika.

## Co potwierdzono

- Cztery pliki komponentu występują już w pierwszym imporcie projektu do Git (po anonimizacji historii: a707798). Nie znaleziono wcześniejszych etapów ich powstawania w osiągalnej historii Git.
- Integracja wykorzystuje API ESPHome: touchscreen, i2c, codegen i Component. Schemat Python ustawia domyślny adres 0x38 i rejestruje urządzenie I2C/ekran dotykowy.
- Sekwencja protokołu jest opisana w komentarzach jako zgodna z przykładem Waveshare: tryb normalny 0x00, status 0x02, współrzędne od 0x03 jako dwie wartości 12-bitowe. Lokalna integracja zawiera ponawianie inicjalizacji i odczytów oraz statystyki czasu/błędów.
- Porównana referencja producenta: [FT3168.cpp](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.64-v2/blob/21b952243d306692774f3b5bfeb3eb5cff10f851/Arduino/examples/06_LVGL_Test/FT3168.cpp). Kod referencji jest strukturalnie inny; nie wykazano identycznych całych funkcji. To referencja protokołu, nie potwierdzenie rewizji PCB monitora.

## Uzupełnienie: odnaleziony zapis utworzenia

Po odpowiedzi właściciela przejrzano lokalny zapis pierwotnej pracy. Znaleziono wywołanie apply_patch z 03.10.2026, 09:00:17 UTC, tworzące wszystkie cztery pliki components/ft3168: oba __init__.py, touchscreen/ft3168_touchscreen.h oraz touchscreen/ft3168_touchscreen.cpp. Ta sama operacja przełączała YAML z ft5x06 na lokalny ft3168. Zapis zawiera pełną początkową implementację integracji i jawne komentarze o sekwencji rejestrów Waveshare. Dalsze zapisy z 03.10 zawierają poprawki sterownika. To dowód tworzenia integracji podczas pracy nad projektem przed importem do Git z 08.10.2026.

Surowy zapis rozmowy pozostaje prywatny, poza repozytorium; nie kopiowano go ani nie publikowano załączników. Wcześniejsza informacja o braku dowodu sprzed Git została uzupełniona tym znaleziskiem. Nie należy dalej opisywać samego miejsca powstania początkowej integracji jako nieustalonego.

## Zakres potwierdzenia i ograniczenia

Git nie dowodzi autorstwa plików sprzed pierwszego importu. Zgodność sekwencji rejestrów nie jest dowodem kopiowania kodu. Nie ustalono ogólnej licencji przykładu producenta i nie przypisujemy jej samodzielnie. Właściciel początkowo nie pamiętał źródła. Później odnaleziony zapis utworzenia plików potwierdził ich powstanie w toku pracy Codex nad projektem. Nie wykazano importu gotowego zewnętrznego sterownika w odnalezionej operacji; korzystano z API ESPHome i opisu protokołu Waveshare. Zapis generowania kodu nie jest gwarancją jego oryginalności ani pełną analizą praw autorskich.

Własne wkłady projektu są objęte [GPL-3.0-only](../../LICENSE); nie uchyla to praw do ewentualnych cudzych fragmentów. [Licencję ESPHome](../../LICENSES/ESPHome-2026.9.1.txt) zachowano osobno. Nie zastępowano działającego sterownika innym kodem tylko w celu deklarowania gotowości publikacji.
