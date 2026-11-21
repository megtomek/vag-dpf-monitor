# Noty licencyjne zasobów zewnętrznych

Teksty zachowano podczas przeglądu 21.11.2026. Nie zmieniają one GPL-3.0-only własnych wkładów. Zasoby zewnętrzne zachowują oryginalne warunki.

| Plik | Dotyczy | Źródło weryfikacji |
|---|---|---|
| [ESPHome-2026.9.1.txt](ESPHome-2026.9.1.txt) | Kod adaptowany z ESPHome: Python MIT, runtime C/C++ GPLv3 | [ESPHome 0b2b16e](https://github.com/esphome/esphome/blob/0b2b16e664ecfc0add37f3435db1c2140b8a687e/LICENSE) |
| [lit-html-LICENSE.txt](lit-html-LICENSE.txt) | lit-html 3.3.3, BSD-3-Clause | [npm lit-html 3.3.3](https://registry.npmjs.org/lit-html/3.3.3) |
| [lit-reactive-element-LICENSE.txt](lit-reactive-element-LICENSE.txt) | @lit/reactive-element 2.1.2, BSD-3-Clause | [npm 2.1.2](https://registry.npmjs.org/@lit/reactive-element/2.1.2) |
| [lit-element-LICENSE.txt](lit-element-LICENSE.txt) | lit-element 4.2.2, BSD-3-Clause | [npm lit-element 4.2.2](https://registry.npmjs.org/lit-element/4.2.2) |
| [Iconify-MIT.txt](Iconify-MIT.txt) | Kod Iconify, MIT | [Iconify — license.txt](https://github.com/iconify/iconify/blob/main/license.txt); dokładna wersja lokalnego fragmentu nieustalona |

Wersje Lit odczytano z markerów wewnątrz lokalnego bundla i zweryfikowano w paczkach npm. Pobierano metadane i teksty licencji, bez instalowania lub wykonywania kodu paczek. Licencja samego upstream frontendu jest osobno w [webserver-v3.LICENSE.txt](../webserver-v3.LICENSE.txt), fontów w [fonts/UFL.txt](../fonts/UFL.txt).

Nie jest to kompletny SBOM ani potwierdzenie dokładnej wersji całego bundla. Źródła i granice potwierdzenia: [raport przeglądu](../docs/PROVENANCE_REVIEW_2026-11-21.md). Przy dystrybucji obrazu warunki/źródła muszą odpowiadać temu konkretnemu buildowi; nie dołączaj obrazu z lokalnymi hasłami.


Ikony MDI są osobną warstwą od kodu Iconify: [Apache-2.0.txt](Apache-2.0.txt), [nota Pictogrammers](Pictogrammers-Free-License.txt). Źródło i ograniczenia: [ikony panelu](../docs/WEB_PANEL_ICONS.md).
