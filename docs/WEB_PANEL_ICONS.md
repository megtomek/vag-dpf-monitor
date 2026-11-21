# Ikony i połączenia panelu WWW

Sprawdzono statyczny kod i oficjalne metadane 21.11.2026. Nie wykonywano śledzenia ruchu na urządzeniu.

## Rozdzielenie kodu i obrazów

Biblioteka renderująca Iconify ma MIT. Znalezione stałe panelu używają prefiksu mdi: (m.in. circle, circle-off-outline, theme-light-dark i dynamiczna nazwa checkbox). Własny YAML nie zawiera stałych mdi:. Nazwy ikon dostarczane przez API ESPHome mogą rozszerzyć ten zbiór; to nie pełna lista wszystkich możliwych ikon.

[Oficjalne API kolekcji Iconify](https://api.iconify.design/collections?prefixes=mdi) wskazało Material Design Icons, autora Pictogrammers i SPDX Apache-2.0. Zachowano [tekst Apache-2.0](../LICENSES/Apache-2.0.txt) oraz [notę Pictogrammers](../LICENSES/Pictogrammers-Free-License.txt), zweryfikowaną w repozytorium producenta na commicie 2424e748e0cc63ab7b9c095a099b9fe239b737c0. Nota wspomina także ikony redystrybuowane na ich własnych licencjach; nie oznacza to, że każdy możliwy zestaw lub ikona w przyszłej konfiguracji ma automatycznie Apache-2.0. Nie dołączano kopii całej kolekcji, fontu ikon ani paczki SVG.

## Dostęp do sieci

W bazie JS znajdują się zasoby Iconify: api.iconify.design oraz serwery alternatywne api.simplesvg.com i api.unisvg.com. Są to zależności przeglądarkowego renderowania ikon. Kod zawiera też odnośnik dokumentacyjny esphome.io/web-api; obecność URL nie dowodzi wykonania zapytania.

Lokalne pochodzenie pliku JS nie oznacza, że panel nigdy nie łączy się z internetem. Przy pracy AP bez internetu brakująca ikona może nie zostać pobrana; rzeczywiste zachowanie cache i wszystkich kontrolek wymaga próby w przeglądarce. Nie sprawdzano tego obecnie na monitorze i nie deklarujemy pełnego trybu offline panelu. Samodzielne odczyty DPF, wyświetlacz urządzenia i zapis nie korzystają z API ikon.

## Zakres pozostały

Potwierdzono dystrybuowaną bazę panelu i nazwę/licencję znanej kolekcji; dokładna wersja fragmentu Iconify w bundlu nie została przypisana. Nie jest to pełny SBOM ani inspekcja wszystkich dynamicznych ikon. Nowe prefiksy ikon należy sprawdzać osobno.
