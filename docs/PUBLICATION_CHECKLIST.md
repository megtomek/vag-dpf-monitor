# Przygotowanie publicznej publikacji

Stan: **21.11.2026 — dokumentacja przygotowana, publikacja niezatwierdzona**. Ta zmiana nie przełącza widoczności repozytorium ani ustawień GitHuba. Nie jest kompletnym audytem prawnym lub bezpieczeństwa.

## Decyzje właściciela

- [x] Właściciel wybrał **GPLv3, wyłącznie wersja 3 (GPL-3.0-only)** dla własnego kodu i dokumentacji. Dodano [LICENSE](../LICENSE), zakres [COPYRIGHT](../COPYRIGHT) i zasady wkładów w CONTRIBUTING. Nadal wymagany jest osobny przegląd pochodzenia cudzych materiałów.
- [x] Wykonano [przegląd pochodzenia i danych](PROVENANCE_REVIEW_2026-11-21.md); potwierdzono CAN/fonty i uzupełniono noty.
- [x] Potwierdzono bazę panelu przez porównanie pełnego artefaktu CDN i odnaleziono zapis utworzenia FT3168 sprzed Git; szczegóły w notach komponentów.
- [ ] Zamknąć pozostałe ustalenia: stare tagi/ZIP bez kompletu not, pozostałości danych w referencjach PR i końcowy zakres zależności panelu.
- [ ] Zdecydować, czy udostępniamy istniejące repo z całą historią i dyskusjami, czy oddzielny przejrzany snapshot. Osobne repo/snapshot też wymaga przeglądu, nie omija licencji.
- [ ] Zatwierdzić widoczność dopiero po końcowym przeglądzie. Nie wykonywano zmiany widoczności.

## Stan dokumentacji

- [x] README PL/EN: cel, sprzęt, zakres zgodności, ograniczenia i start.
- [x] Instrukcja budowy, obsługi, pobierania danych i diagnozy.
- [x] CONTRIBUTING, SUPPORT, SECURITY i zasady społeczności.
- [x] Szablony błędów, zgodności ECU, propozycji i PR.
- [x] Jawne rozróżnienie main/tagów, rozwoju w PR i obrazu na urządzeniu.
- [ ] Odtworzyć instrukcję instalacji na czystym komputerze; dokumentacja nie jest dowodem takiej próby.
- [ ] Zatwierdzić aktualne zdjęcie urządzenia i ew. film; historyczne wizualizacje nie są takim dowodem.
- [ ] Uzgodnić bazę funkcjonalną publikacji: PR #17/#18 nie są scalone do main; nie deklarować ich jako funkcji wydania.

## Przegląd danych i historii

Aktualny [raport z 21.11.2026](PROVENANCE_REVIEW_2026-11-21.md) obejmuje 46 commitów z gałęzi origin/tagów i 14 głów PR (bez dodatkowych commitów), 288 historycznych blobów, ZIP, grafiki oraz treści/metadane GitHub. Gitleaks i porównanie aktualnych wartości konfiguracji nie wskazały sekretów. To nie gwarancja braku wszystkich dawnych lub zakodowanych sekretów.

Wykryto dane do decyzji właściciela: adresy autorów w commitach, jeden MAC, prywatne IPv4 i ścieżki środowiska. Osobne lokale referencje techniczne z surowymi załącznikami nie należą do publikowanych gałęzi/tagów. Nie przepisywano historii ani nie usuwano danych. Materiały szczegółowe pozostają poza Gitem. Stary ZIP i stare tagi wymagają rozliczenia licencji — raport opisuje zakres.

- [x] Gitleaks 8.30.1: wszystkie pobrane gałęzie origin/tagi oraz 288 historycznych blobów/ZIP — 0 znalezisk; dodatkowe wyszukiwanie bieżących sekretów w kilku reprezentacjach — 0. Zakres i ograniczenia w raporcie.
- [x] Przejrzano 4 PNG, 4 wersje SVG, ZIP, raporty i dane porównań; nie znaleziono VIN/zdjęć pojazdu/lokalizacji. Ustalono historyczny MAC, adresy commitów, prywatne IPv4 i ścieżki.
- [ ] Rozstrzygnąć publikację metadanych autora i MAC; nowe .gitignore lub korekta najnowszego pliku nie czyszczą historii.
- [x] Przejrzano 19 wpisów Issues/PR i 4 Releases; 0 komentarzy/reviews, 0 załączonych assets, 0 wskazanych załączników user-content, 0 uruchomień Actions. Osobny endpoint zbiorczy artefaktów niedostępny; szczegóły w raporcie.
- [ ] Dla faktycznie ujawnionych sekretów: zmiana/unieważnienie i uzgodnione oczyszczenie historii. Sam nowy .gitignore nie usuwa dawnych danych.
- [ ] Sprawdzić dokładny końcowy diff i zdalne referencje bezpośrednio przed publikacją.

Szczegółowy wynik wstępnego skanu pozostaje lokalny, poza repozytorium. Nie jest certyfikatem braku sekretów.

## Ustawienia GitHub — do wykonania przy publikacji

- [ ] Włączyć Private vulnerability reporting i sprawdzić działanie przycisku w SECURITY. Do tego czasu dokument opisuje awaryjną prośbę o prywatny kontakt.
- [ ] Włączyć dostępne dla repo secret scanning / push protection i alerty zależności. Nie zakładać, że ten PR je włącza.
- [ ] Ustalić ochronę main, przegląd PR i sensowne wymagane kontrole. Nie wymagać nieistniejącego CI ani nie odcinać właściciela od pracy.
- [ ] Uzupełnić opis i topics, np. esp32-s3, esphome, obd2, can-bus, dpf, automotive. Nie deklarować uniwersalnej zgodności.
- [ ] Przygotować prerelease źródeł z zakresem walidacji według [szablonu](../.github/RELEASE_TEMPLATE.md); bez BIN/ELF z hasłami.
- [ ] Sprawdzić repo jako niezalogowany odbiorca po zatwierdzonej zmianie widoczności.

## Weryfikacja wcześniejszego przygotowania dokumentacji

Przejrzano 22 zmienione/dodane pliki: 136 odnośników do lokalnych plików bez brakujących celów, trzy frontmatter szablonów Issue (name/about), YAML konfiguracji zgłoszeń i git diff --check. Wymaganie Python >=3.12 potwierdzono w metadanych zainstalowanego ESPHome 2026.9.1. To przegląd dokumentacji i metadanych, bez testów firmware, instalowania zależności, kompilacji, flashowania i nowej obserwacji auta. Nie weryfikowano wszystkich odnośników zewnętrznych ani renderowania GitHub.

Po decyzji o GPLv3 sprawdzono dodatkowo 78 odnośników w 10 zmienionych/dodanych plikach, bez brakujących celów, spójność oznaczenia GPL-3.0-only i diff whitespace. LICENSE pobrano bez zmian ze źródła SPDX; tekst i hash źródłowy zachowano w lokalnym raporcie. Nie zmieniano istniejących licencji panelu/fontów ani firmware.

## Podstawa

Układ odpowiada zaleceniom [GitHub o README i bezpieczeństwie repozytoriów](https://docs.github.com/en/repositories/creating-and-managing-repositories/best-practices-for-repositories) oraz [plikach społeczności](https://docs.github.com/en/communities/setting-up-your-project-for-healthy-contributions/about-community-profiles-for-public-repositories). Prywatne zgłaszanie podatności wymaga osobnego [włączenia w ustawieniach](https://docs.github.com/en/code-security/how-tos/report-and-fix-vulnerabilities/configure-vulnerability-reporting/configure-for-a-repository).

Własna część projektu otrzymała GPL-3.0-only. Dokumentacja i ten przegląd nie zatwierdzają sprzętu ani automatycznie nie rozstrzygają pochodzenia cudzych materiałów i zgody na publikację danych historycznych.


Aktualizacja po zgodzie właściciela: [wynik oczyszczenia historii i ocena gotowości](PRIVACY_CLEANUP_2026-11-21.md). Gałęzie/tagi oczyszczono, ale stare referencje PR nadal blokują uznanie całego repozytorium za anonimowe.


Uzupełnienie 21.11.2026: [potwierdzono pełną bazę panelu z oficjalnym artefaktem CDN](WEB_PANEL_PROVENANCE.md), po normalizacji dwóch opisanych różnic. Brak identyczności całego pliku wynika również z własnych dodatków. Nie ustalono commitu budowania ani pełnej wersji Iconify; pozostałe otwarte ustalenia nadal obowiązują.


Aktualizacja archiwum: stary ZIP wycofano z bieżącej gałęzi, zachowując lokalny oryginał i prywatną kopię; nadal istnieje w dawnych commitach/tagach. Szczegóły: [archiwalne paczki](../firmware/archive/README.md). [Pochodzenie FT3168](../components/ft3168/PROVENANCE.md) rozdziela potwierdzone fakty i oczekujące wyjaśnienie pierwszej implementacji.


Uzupełnienie dowodowe 21.11.2026: odnaleziono prywatny zapis utworzenia czterech plików FT3168 przez Codex 03.10.2026, przed pierwszym importem Git. [Nota FT3168](../components/ft3168/PROVENANCE.md) opisuje operację i ograniczenia: potwierdzono miejsce powstania integracji, zachowano wskazanie API ESPHome i protokołu Waveshare. Nie publikowano surowej rozmowy. Pozostałe problemy dawnych tagów/PR nadal obowiązują.
