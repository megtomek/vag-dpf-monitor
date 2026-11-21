# Pochodzenie bazy panelu WWW

## Potwierdzenie z 21.11.2026

Baza lokalnego webserver-v3.js odpowiada całemu oficjalnemu artefaktowi [ESPHome Web Server v3](https://oi.esphome.io/v3/www.js), pobranemu podczas przeglądu. Plik upstream ma 77 692 bajty, SHA256:

`7d94fdcfbbb0782db3c7cadd577855879df103f1b88f546771fca75a36c4ca0d`

Porównano pełną treść, a nie wybrane funkcje. Po pominięciu naszego pierwszego komentarza licencyjnego i normalizacji CRLF/LF znaleziono w bazie wyłącznie dwie różnice:

1. Jeden znak < wewnątrz szablonu komentarza HTML zapisano jako sekwencję JavaScript \x3C.
2. Jeden znak nowej linii przed osadzonym SVG zastąpiono spacją.

Po normalizacji tych dwóch miejsc cała baza jest identycznym prefiksem lokalnego pliku. Dalej występują nasze dodatki, od komentarza Local download control. Nie zmieniono kodu podczas tego porównania. Pierwotny wynik „CDN nie jest identyczny” dotyczył surowych plików wraz z dodatkami; nie świadczył o nieznanej bazie.

## Zakres i granice

Potwierdzono tożsamość dystrybuowanego artefaktu z powyższymi dwiema różnicami. CDN jest adresem zmiennym; hash identyfikuje sprawdzoną treść. Nie ustalono przypisania tego artefaktu do konkretnego commitu źródłowego ani nie odtwarzano jego procesu budowania. Nie należy nazywać tego odtwarzalnym buildem.

Źródła frontendu: [esphome/esphome-webserver](https://github.com/esphome/esphome-webserver), MIT wilberforce. Obowiązują [nota upstream](../webserver-v3.LICENSE.txt), [noty Lit i Iconify](../LICENSES/README.md) oraz GPL-3.0-only własnych dodatków. Wersje Lit potwierdzono z markerów i paczek npm. Wersja lokalnego Iconify i zakres ewentualnych ikon pobieranych z sieci nadal nie są w pełni zinwentaryzowane. Weryfikacja nie jest obserwacją działania panelu na urządzeniu.


Kolejne ustalenie: [ikony MDI i zależności sieciowe](WEB_PANEL_ICONS.md); oficjalne metadane kolekcji wskazują Apache-2.0, osobno od MIT kodu Iconify.
