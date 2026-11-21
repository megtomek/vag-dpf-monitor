# Pochodzenie i licencje zasobów

Stan: 21.11.2026. Własny kod i dokumentacja mają **GPL-3.0-only** — [LICENSE](LICENSE), [COPYRIGHT](COPYRIGHT). Zewnętrzne elementy zachowują oryginalne prawa. Wyniki technicznego przeglądu i ograniczenia: [raport](docs/PROVENANCE_REVIEW_2026-11-21.md).

## Potwierdzone i nierozstrzygnięte pochodzenie

| Element | Ustalenie i warunki |
|---|---|
| esp32_can_recover | Adaptacja esp32_can ESPHome 2026.9.1, commit 0b2b16e664ecfc0add37f3435db1c2140b8a687e. Python upstream MIT, C/C++ GPLv3. [Nota komponentu](components/esp32_can_recover/PROVENANCE.md), [pełna licencja upstream](LICENSES/ESPHome-2026.9.1.txt) |
| FT3168 | Integracja utworzona podczas pracy Codex 03.10.2026, potwierdzona lokalnym zapisem dodania czterech plików. Wykorzystuje API ESPHome i protokół Waveshare; nie wykazano identycznych całych funkcji referencji. [Dowód i ograniczenia](components/ft3168/PROVENANCE.md); cudza referencja zachowuje swoje prawa |
| webserver-v3.js | Frontend upstream esphome/esphome-webserver: [MIT / wilberforce](webserver-v3.LICENSE.txt). Zawiera Lit BSD-3-Clause i Iconify MIT; własne dodatki DPF są GPL-3.0-only. Pełną bazę potwierdzono z oficjalnym artefaktem CDN; [hash i dwie różnice](docs/WEB_PANEL_PROVENANCE.md). Commit budowania i wersja fragmentu Iconify nieustalone |
| Lit w bundlu | reactive-element 2.1.2, lit-html 3.3.3, lit-element 4.2.2: wersje z markerów, licencje potwierdzone w npm. Teksty BSD-3-Clause w [LICENSES](LICENSES/README.md) |
| Iconify | Zachowano [MIT](LICENSES/Iconify-MIT.txt). Nie ustalono wersji lokalnego fragmentu; nie badano zapytań runtime. Ikony MDI mają osobne noty Apache-2.0/Pictogrammers — [zakres](docs/WEB_PANEL_ICONS.md) |
| Ubuntu-B.ttf / Ubuntu-R.ttf | Tożsame binarnie z oficjalną paczką Ubuntu 0.83. Canonical 2011 / Dalton Maag, Ubuntu Font Licence 1.0. [Źródło i hashe](fonts/PROVENANCE.md), [licencja](fonts/UFL.txt). Nie są relicencjonowane na GPL |
| Schematy i PNG | Przejrzane makiety i diagramy, bez widocznych identyfikatorów pojazdu ani metadanych tekstowych PNG. To archiwalne wizualizacje, nie aktualne zdjęcia urządzenia; sam wygląd nie dowodzi autorstwa |
| Source-bundle ZIP | Wycofany z bieżącej dystrybucji; lokalny oryginał i prywatna kopia zachowane. Dawne commity/tagi nadal zawierają paczkę bez kompletu not. [Stan archiwum](firmware/archive/README.md) |

## Zależności budowania

ESPHome 2026.9.1 rozdziela Python MIT i runtime GPLv3. Z zachowanego lokalnego dependencies.lock: ESP-IDF 5.5.5 / Apache-2.0, LVGL 9.5.0 / MIT, ArduinoJson 7.4.3 / MIT, mDNS 1.12.0 / Apache-2.0. Sprawdzono teksty licencji managed_components. To inwentaryzacja wcześniejszego builda, nie pełny SBOM całego toolchainu ani nowego wydania.

Przy dystrybucji firmware trzeba zachować wymagane noty i zapewnić źródła odpowiadające konkretnemu obrazowi zgodnie z GPL. Nie udostępniaj BIN zawierających lokalne hasła. Uzupełnione noty nie rozwiązują automatycznie pochodzenia nieprzypiętego bundla lub materiałów sprzed Git.

## Dane i prawa

Nie znaleziono dystrybuowanych baz ODX/ROD/PDX lub komercyjnych pakietów diagnostycznych w sprawdzonych ścieżkach. Źródła tropów DID są cytowane w raportach i oddzielane od własnych pomiarów. Referencja do identyfikatora ODX nie jest prawem do bazy. Właściciel powinien potwierdzić prawa do własnego wkładu sprzed pierwszego importu; Git sam tego nie rozstrzyga.

Bazę panelu i powstanie integracji FT3168 potwierdzono opisanymi dowodami. Przed publikacją całej historii pozostają stare ZIP/tagi bez kompletu not i dane zachowane w referencjach PR; gałęzie/tagi już zanonimizowano. Wybór GPL własnej części jest zakończony.
