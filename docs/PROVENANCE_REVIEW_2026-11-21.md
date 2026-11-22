# Przegląd pochodzenia i danych historycznych — 21.11.2026

> Dokument historyczny dotyczący poprzedniego repozytorium. Nie opisuje bieżącej gotowości nowej historii. Aktualny stan: [checklista publikacji](PUBLICATION_CHECKLIST.md) i [zakres przeniesienia](REPOSITORY_MIGRATION.md).

## Wynik

**Przegląd wykonany; publikacja całego repozytorium z historią nadal wymaga decyzji i kilku działań.** Nie znaleziono sekretów wskazanych przez Gitleaks ani aktualnych lokalnych haseł w sprawdzonych materiałach. Potwierdzono pochodzenie CAN i fontów, dodano brakujące noty bibliotek panelu. Pozostają dokładna wersja bazy frontendu, pochodzenie lokalnego FT3168 sprzed Git oraz świadoma decyzja o metadanych autora i identyfikatorze urządzenia w archiwum. To przegląd techniczny, nie gwarancja braku ukrytych danych ani opinia prawna.

Nie zmieniono widoczności repozytorium, starych commitów, tagów ani surowych danych lokalnych. Teksty licencji i przypisania źródeł są poprawkami do przygotowania publikacji; w JS zmieniono wyłącznie komentarz licencyjny.

## Zakres i punkt odniesienia

- Kandydat dokumentacji/licencji: `codex/public-project-docs`, **2bcc102**. Main: **3050486**. Pobranie referencji origin wykonano przed skanowaniem.
- Wszystkie lokalnie pobrane gałęzie origin i cztery tagi v0.1.0–v0.1.3: **46 osiągalnych commitów**, w tym **36 bez merge**, **288 różnych blobów**. Gitleaks raportuje 36 commitów z patchami, co nie oznacza pominięcia pozostałych referencji.
- Dodatkowo pobrano do lokalnych refs/audit-dpf-pulls/* głowy wszystkich 14 PR. Ich dodanie nie zwiększyło zbioru 46 commitów — kod tych PR jest już pokryty gałęziami/tagami. Referencje audytu pozostają lokalne.
- 1 historyczny ZIP / 8 plików, 4 różne PNG, 4 wersje SVG. Zawartość ZIP odczytano i objęto skanowaniem; niczego z archiwum nie uruchamiano.
- GitHub API: **19 wpisów** — 5 Issues i 14 PR, **4 Releases**. Kolekcje komentarzy Issues i inline PR: 0; sprawdzone kolekcje reviews wszystkich 14 PR: 0. Każda pobrana lista mieściła się na jednej stronie per_page=100. W Releases brak dodatkowych assets; teksty wydań przejrzano. Lista uruchomień Actions: 0. Osobny endpoint zbiorczy artefaktów nie jest dostępny przez użyte narzędzie; nie przypisujemy mu wyniku skanowania. Wiki, Discussions i Pages są wyłączone według metadanych repozytorium.
- Nie znaleziono odnośników do załączników user-attachments/user-images/private-user-images w treści tych wpisów. Automatyczne archiwa źródeł Releases odsyłają do już sprawdzonych tagów.
- Lokalna techniczna referencja narzędzia zawierająca trzy surowe załączniki nie należy do sprawdzanych zdalnych gałęzi/tagów. Nie używać push --mirror ani wysyłania wszystkich lokalnych referencji.
- Lokalny build użyty do inwentaryzacji zależności pochodzi z wcześniejszego wdrożenia; nie jest nowym buildem ani dowodem kompletności składu przyszłego wydania.

Zakres obejmuje treści i metadane wymienione powyżej, nie prywatne dane całego komputera, niedostępne obiekty serwera, nieosiągalne dawne rewizje po ewentualnym force-push, zewnętrzne forki, historię edycji komentarzy ani nieistniejące już zasoby GitHuba.

## Narzędzia i wyniki wyszukiwania

Gitleaks **8.30.1**, oficjalne wydanie; ZIP zweryfikowany względem sumy SHA256 opublikowanej przy wydaniu. Standardowe reguły, redakcja wyników 100%, bez respektowania komentarzy gitleaks:allow, dekodowanie do głębokości 5, archiwa do głębokości 3.

Wykonano cztery odrębne przebiegi:
1. `gitleaks git --log-opts="--remotes=origin --tags --full-history"`: około 1,89 MB patchy — 0 znalezisk.
2. `gitleaks dir` po wyeksportowanych 288 historycznych blobach, także ZIP — około 4,21 MB — 0 znalezisk.
3. `gitleaks dir` po pobranym JSON treści/metadanych GitHub — około 118,94 KB — 0 znalezisk.
4. `gitleaks dir` po wiadomościach i metadanych author/committer commitów — około 13,33 KB — 0 znalezisk.

Dodatkowo porównano aktualne lokalne wartości konfiguracji w postaci jawnej, JSON, URL-encoded i base64: 0 trafień. Uzupełniające wyszukiwanie kandydatów VIN, e-maili w plikach, MAC, prywatnych IPv4 i ścieżek Windows: brak kandydatów VIN i e-maili w treści plików repozytorium/Issues; w metadanych commitów 92 wystąpienia dwóch adresów autor/committer. Znalezione identyfikatory omówiono niżej. Reguły i wyszukiwanie dosłowne mogą nie rozpoznać dawnego hasła, innego kodowania lub nieznanego formatu. Nie weryfikowano działania żadnego znalezionego identyfikatora/danych dostępowych.

Pełne materiały i wyniki zredagowane skanera pozostają lokalnie w katalogu logs/publication-review-2026-11-21, poza Gitem. Raport nie powiela wartości prywatnych.

## Pochodzenie kodu i zasobów

| Element | Ustalenie | Ocena / działanie |
|---|---|---|
| esp32_can_recover | Adaptacja esp32_can ESPHome 2026.9.1, commit 0b2b16e664ecfc0add37f3435db1c2140b8a687e. Po normalizacji nazw zachowane 147/147 linii Python, 180/188 C++, 44/45 nagłówka upstream | Potwierdzone pochodzenie. Dodano [notę komponentu](../components/esp32_can_recover/PROVENANCE.md) i pełną hybrydową licencję ESPHome; nie przypisujemy całego pliku wyłącznie autorowi monitora |
| FT3168 | Lokalny sterownik łączy API ESPHome ze schematem rejestrów Waveshare: 0x00=normal, status 0x02, współrzędne 0x03. Porównano z publicznym FT3168.cpp/h producenta, commit 21b952243d306692774f3b5bfeb3eb5cff10f851 | Kod jest strukturalnie inny; nie stwierdzono identycznych całych funkcji. Samo zgodne działanie protokołu nie dowodzi kopiowania. Git zaczyna się od gotowego zestawu źródeł, więc brak pełnego dowodu pochodzenia sprzed Git. Nie ustalono ogólnej licencji przykładu Waveshare; przed dystrybucją potrzebne potwierdzenie autorstwa adaptacji lub wyjaśnienie prawa do przejętych fragmentów, jeśli były przejmowane |
| webserver-v3.js — baza | Nagłówek wskazuje esphome/esphome-webserver; nota wilberforce MIT odpowiada licencji upstream. Lokalny plik ma też własne dodatki pobierania/logów | Nie udało się przypisać bazy do konkretnego upstream commit/release. Aktualny CDN nie jest identyczny; brak source map. Potrzebny zapis pierwotnej wersji lub odtworzenie z przypiętego źródła, zanim baza zostanie uznana za w pełni rozliczoną |
| Lit w bundlu | Markery @lit/reactive-element 2.1.2, lit-html 3.3.3, lit-element 4.2.2. Weryfikacja paczek npm i ich tekstów licencji | BSD-3-Clause, nie MIT. Dodano trzy teksty licencji w [LICENSES](../LICENSES/README.md) |
| Iconify w bundlu | Rozpoznany kod i endpointy Iconify, zależność frontend upstream; licencja projektu Iconify MIT | Zachowano notę MIT. Dokładna wersja lokalnego fragmentu i licencje ewentualnie ładowanych zestawów ikon nieustalone. Nie wykonywano runtime/network trace panelu |
| Ubuntu-B.ttf / Ubuntu-R.ttf | Ubuntu 0.83, zgodne binarnie z oficjalną paczką Canonical; w metadanych copyright 2011 Canonical / Dalton Maag, UFL 1.0 | Potwierdzone; [hash i źródło](../fonts/PROVENANCE.md). Nie modyfikowano fontów i nie relicencjonowano ich na GPL |
| Własne nagłówki UDS, estymator, rejestrator, sieć i UI | Śledzone w historii projektu, brak obcych not w tych plikach; dane mapowań pochodzą z opisanych porównań i przechwyceń | Brak wykrytej dodatkowej biblioteki vendor w tych plikach nie jest dowodem wyłącznego autorstwa. Historia rozpoczyna się gotowym importem ea4433e; właściciel powinien potwierdzić prawa do wkładu sprzed importu |
| Dane DID/PID i raporty | Są cytowania źródeł oraz własne porównania. UDS_CANDIDATES_RESEARCH odróżnia tropy v-cu od własnego potwierdzenia | Nie znaleziono plików baz ODX/ROD/PDX lub skopiowanych komercyjnych pakietów w publikowanych ścieżkach. Nie utożsamiamy referencji do ODX z prawem do bazy. Nie rozstrzygamy praw do całego standardu ani zewnętrznych stron |
| ZIP z 05.10 | 8 plików źródeł CAN/FT3168; brak secrets.yaml, BIN/ELF, logów. To starsze wersje komponentów, część zgodna z zachowanymi snapshotami | W ZIP brakuje kompletu not i licencji, w tym ESPHome. Nie dystrybuować jako samodzielnej paczki bez uzupełnienia albo wyłączyć z przyszłego publicznego snapshotu, zachowując prywatną kopię. Nowy .gitignore nie usuwa go z historii |

W tagach v0.1.0, v0.1.1, v0.1.2 i v0.1.3 nie ma głównego LICENSE/COPYING/COPYRIGHT. Dodanie GPL do nowej gałęzi nie dodaje pliku do dawnych automatycznych archiwów Releases. Stare paczki wymagają osobnej oceny kompletności not i licencji; nie zmieniano tagów.

To analiza tekstowa i inwentaryzacja, nie narzędzie ustalające autorstwo. Niedopasowanie do sprawdzonego upstream nie dowodzi oryginalności. Nie twierdzimy, że znaleziono naruszenie tam, gdzie brakuje dowodu.

## Grafiki i dane historyczne

Cztery PNG obejrzano: są schematycznymi makietami UI, nie zdjęciami pojazdu/urządzenia. Brak VIN, tablic rejestracyjnych, osób i lokalizacji w widocznym obrazie. W strukturze PNG brak eXIf/tEXt/zTXt/iTXt. Nie przedstawiać ich jako aktualnych fotografii.

Przejrzano tekst i strukturę czterech wersji SVG (trzy ścieżki, w tym dwie wersje bieżącego schematu): brak skryptów i zewnętrznych href/obrazów, brak wykrytych identyfikatorów pojazdu. Wykres zdarzenia zawiera pomiary temperatury/sadzy i czas względny. CSV porównań jest w większości formularzem i jedną próbką telemetryczną; nie zawiera VIN ani lokalizacji. Sam wygląd grafik nie jest dowodem autorstwa; brak ich zewnętrznego źródła w historii pozostaje ograniczeniem.

### Dane wymagające decyzji

- **P-01, średnie:** dwa adresy inne niż GitHub noreply występują w metadanych author/committer dawnych commitów. To dane widoczne po publikacji historii; nie są hasłami. Przed zmianą widoczności zaakceptować ich publikację albo zaplanować zmianę metadanych/snapshot. Raport nie powiela adresów.
- **P-02, średnie:** jeden konkretny MAC monitora w docs/history/ROADMAP-before-organization-2026-10-27.md. Identyfikuje egzemplarz. To nie sekret uwierzytelniający, ale do publicznej dokumentacji wystarczy zanonimizowane oznaczenie. Usunięcie z najnowszego pliku nie usunie starszych commitów/tagów.
- **P-03, niskie:** pięć różnych prywatnych IPv4, 142 wystąpienia w różnych wersjach/JSON po wyłączeniu standardowego AP 192.168.4.1. Wiele to dane laboratoryjne i przykłady; nie są publicznymi adresami dostępu. Przejrzeć, czy są potrzebne jako dowód historyczny; w instrukcjach dla nowych użytkowników używać bieżącego IP lub wartości przykładowej.
- **P-04, niskie:** 13 dopasowań do ścieżki Windows z jednym ogólnym użytkownikiem systemowym. Występują m.in. ENVIRONMENT, lokalnych skryptach i dawnym pliku metadanych USB. Nie wykryto w tym mechanizmie indywidualnego imienia/nazwiska; ścieżki nadal ujawniają strukturę środowiska.
- Brak trafień aktualnego SSID/hasła/OTA/loginu w kontrolowanych reprezentacjach. Nie ma podstaw, by ten wynik przedstawiać jako pełne sprawdzenie wszystkich haseł użytych kiedykolwiek.

## Zależności lokalnego builda

Z zachowanego dependencies.lock: ESP-IDF 5.5.5, LVGL 9.5.0 (MIT), ArduinoJson 7.4.3 (MIT), mDNS 1.12.0 (Apache-2.0). Teksty licencji sprawdzono w lokalnych managed_components. ESPHome 2026.9.1: Python MIT / C++ GPLv3. Nie kopiowano wygenerowanych obrazów, cache ani prywatnych konfiguracji do repozytorium. Nie jest to pełny SBOM wszystkich komponentów ESP-IDF/toolchainu.

## Zalecany plan przed publikacją

1. Dokończyć pochodzenie bazy panelu i lokalnego FT3168; potwierdzić prawa do materiałów sprzed Git. Do tego czasu brak pełnego potwierdzenia kompletu praw.
2. Ustalić sposób dystrybucji starego ZIP i archiwów starych tagów bez głównej licencji oraz aktualną bazę funkcjonalną. Oznaczyć makiety jako archiwalne.
3. Właściciel wybiera publikację istniejącej historii albo przejrzanego snapshotu. Snapshot ogranicza przenoszone metadane, ale nie zwalnia z licencji i przeglądu zawartości.
4. Rozstrzygnąć P-01/P-02. Dla pełnej historii ewentualna anonimizacja obejmuje także tagi, gałęzie i stare PR; wymaga osobnego uzgodnienia, kopii i planu aktualizacji klonów. Nie wykonywano rewrite/force-push.
5. Dla przyszłych commitów używać GitHub noreply, jeśli właściciel chce ograniczyć publikację adresu. Nie zmieniano globalnej konfiguracji Git.
6. Dopiero potem końcowy skan kandydata, przegląd diff i ustawień bezpieczeństwa oraz osobna zgoda na widoczność publiczną.

Praktycznie najprostszy wariant do rozważenia: zachować obecne repo jako prywatne archiwum i utworzyć osobny przejrzany snapshot publiczny z aktualnymi źródłami i kompletem not, po wyjaśnieniu pozostałego pochodzenia. Nie wykonano tego działania; wybór należy do właściciela.

Nie trzeba ponownie wgrywać firmware dla tych zmian. Nie uruchamiano testów produktu ani kompilacji; skanowanie sekretów i porównania źródeł są zakresem zleconego przeglądu.

## Źródła weryfikacyjne

- [ESPHome esp32_can, wersja 2026.9.1](https://github.com/esphome/esphome/tree/0b2b16e664ecfc0add37f3435db1c2140b8a687e/esphome/components/esp32_can)
- [Waveshare FT3168.cpp — referencja protokołu](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.64-v2/blob/21b952243d306692774f3b5bfeb3eb5cff10f851/Arduino/examples/06_LVGL_Test/FT3168.cpp)
- [Oficjalna paczka Ubuntu 0.83](https://assets.ubuntu.com/v1/0cef8205-ubuntu-font-family-0.83.zip)
- [Frontend ESPHome](https://github.com/esphome/esphome-webserver), [CDN porównania](https://oi.esphome.io/v3/www.js)
- [Gitleaks v8.30.1](https://github.com/gitleaks/gitleaks/releases/tag/v8.30.1)


## Uzupełnienie: anonimizacja przed publikacją

21.11.2026 właściciel zlecił usunięcie prywatnych e-maili i MAC. W bieżącej gałęzi zastąpiono MAC monitora w archiwalnym ROADMAP i dwa adresy MAC innych urządzeń sieci lokalnej w COMMISSIONING_CHECKLIST. Dodatkowe sprawdzenie wykryło zapis z myślnikami, którego wcześniejsze zestawienie MAC nie uwzględniało; poprzednia liczba jednego MAC nie jest kompletną inwentaryzacją. Nie usuwano not autorów zewnętrznych bibliotek.

To usuwa wartości z bieżących plików, ale nie ze starych commitów, tagów i referencji PR. Prywatne e-maile są w metadanych historii, nie w bieżących plikach. Właściciel wyraźnie zezwolił 21.11.2026 na przepisanie historii obecnego repozytorium jako wyjątek od zakazu force-push. Oczyszczanie gałęzi/tagów wymaga prywatnej kopii zapasowej i weryfikacji zdalnych referencji. Repozytorium pozostaje prywatne i nie jest jeszcze gotowe do publikacji z całą historią. Pozostałe ustalenia o pochodzeniu FT3168, bazy panelu i licencjach archiwów pozostają otwarte.


Aktualizacja po zgodzie właściciela: [wynik oczyszczenia historii i ocena gotowości](PRIVACY_CLEANUP_2026-11-21.md). Gałęzie/tagi oczyszczono, ale stare referencje PR nadal blokują uznanie całego repozytorium za anonimowe.


Uzupełnienie 21.11.2026: [potwierdzono pełną bazę panelu z oficjalnym artefaktem CDN](WEB_PANEL_PROVENANCE.md), po normalizacji dwóch opisanych różnic. Brak identyczności całego pliku wynika również z własnych dodatków. Nie ustalono commitu budowania ani pełnej wersji Iconify; pozostałe otwarte ustalenia nadal obowiązują.


Uzupełnienie dowodowe 21.11.2026: odnaleziono prywatny zapis utworzenia czterech plików FT3168 przez Codex 03.10.2026, przed pierwszym importem Git. [Nota FT3168](../components/ft3168/PROVENANCE.md) opisuje operację i ograniczenia: potwierdzono miejsce powstania integracji, zachowano wskazanie API ESPHome i protokołu Waveshare. Nie publikowano surowej rozmowy. Pozostałe problemy dawnych tagów/PR nadal obowiązują.
