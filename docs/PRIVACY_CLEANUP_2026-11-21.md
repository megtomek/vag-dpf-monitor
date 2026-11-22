# Oczyszczenie historii i gotowość publikacji — 21.11.2026

> Dokument historyczny dotyczący poprzedniego repozytorium. Nie opisuje bieżącej gotowości nowej historii. Aktualny stan: [checklista publikacji](PUBLICATION_CHECKLIST.md) i [zakres przeniesienia](REPOSITORY_MIGRATION.md).

## Wynik: jeszcze nie udostępniać całego repozytorium

Na wyraźne zlecenie właściciela przepisano historię istniejącego repozytorium. Prywatna kopia sprzed operacji pozostaje lokalnie poza publikowanymi źródłami. Nie zmieniono widoczności ani działania firmware.

## Wykonane i zweryfikowane

- Oczyszczono wszystkie 15 gałęzi i 4 tagi na GitHub, łącznie 19 referencji. Aktualizację wysłano atomowo, z indywidualną ochroną force-with-lease przed nadpisaniem równoległych zmian.
- Dwa prywatne adresy author/committer zastąpiono adresem GitHub noreply właściciela; nie zmieniano not autorów bibliotek. Usunięto trzy konkretne MAC (monitor i dwa urządzenia LAN) z całej historii tych gałęzi/tagów i wiadomości commitów.
- Sprawdzono 308 osiągalnych blobów oraz metadane commitów: zero wystąpień usuwanych wartości. Zdalne głowy gałęzi/tagi odpowiadały oczyszczonej kopii.
- Nowy katalog roboczy: dpf-public-clean. Dawne katalogi i prywatne backupy nadal przechowują oryginalną historię; nie należy wysyłać z nich dawnych gałęzi/tagów ani używać push --mirror. Nie nadpisywano niezapisanych zmian pierwotnego katalogu. Identyfikatory commitów zmieniły się; stare hashe w raportach są historycznymi identyfikatorami dowodów.

## Pozostałości na GitHub

Po operacji 11 referencji głów starych PR nadal wskazywało nieoczyszczone commity: #5, #6, #7, #8, #9, #10, #11, #12, #14, #15, #16. PR #17/#18/#19 wskazywały nową historię. Użytkownik nie może nadpisać refs/pull zwykłym push. Nie sprawdzono wszystkich wewnętrznych merge refs/cache, więc liczba 11 nie oznacza pełnego zakresu pozostałości serwera.

[Dokumentacja GitHub](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/removing-sensitive-data-from-a-repository) opisuje kontakt ze wsparciem w sprawie cache i referencji PR. GitHub zastrzega, że nie usuwa danych uznanych za niewrażliwe; nie gwarantujemy przyjęcia zgłoszenia dotyczącego e-mail/MAC. Nie wysyłano zgłoszenia ani wiadomości w imieniu właściciela. Zwykłe zamknięcie PR nie usuwa jego historii.

## Inne blokady publikacji

- Nieustalona dokładna wersja źródeł bazy panelu i fragmentu Iconify; pochodzenie integracji FT3168 sprzed pierwszego importu Git wymaga wyjaśnienia.
- Stary ZIP komponentów i dawne tagi nadal nie zawierają pełnego kompletu licencji. Anonimizacja nie dopisuje licencji do dawnych wersji.
- Dokumentacja i GPL przygotowane w PR #19 nadal nie są scalone do main. Nie publikować main jako przygotowanej wersji dokumentacyjnej.
- Instalacja na czystym komputerze nie została sprawdzona. Przed publikacją pozostaje końcowy przegląd ustawień bezpieczeństwa i wybranej wersji źródeł.

Najpewniejszą drogą bez ujawnienia starych PR pozostaje oddzielne repozytorium z oczyszczonymi źródłami i licencjami, po wyjaśnieniu pochodzenia; wymaga to osobnej decyzji. Obecne repozytorium pozostaje prywatne. Nie uruchamiano testów produktu, kompilacji ani flashowania.


Uzupełnienie 21.11.2026: [potwierdzono pełną bazę panelu z oficjalnym artefaktem CDN](WEB_PANEL_PROVENANCE.md), po normalizacji dwóch opisanych różnic. Brak identyczności całego pliku wynika również z własnych dodatków. Nie ustalono commitu budowania ani pełnej wersji Iconify; pozostałe otwarte ustalenia nadal obowiązują.


Aktualizacja archiwum: stary ZIP wycofano z bieżącej gałęzi, zachowując lokalny oryginał i prywatną kopię; nadal istnieje w dawnych commitach/tagach. Szczegóły: [archiwalne paczki](../firmware/archive/README.md). [Pochodzenie FT3168](../components/ft3168/PROVENANCE.md) rozdziela potwierdzone fakty i oczekujące wyjaśnienie pierwszej implementacji.


Uzupełnienie dowodowe 21.11.2026: odnaleziono prywatny zapis utworzenia czterech plików FT3168 przez Codex 03.10.2026, przed pierwszym importem Git. [Nota FT3168](../components/ft3168/PROVENANCE.md) opisuje operację i ograniczenia: potwierdzono miejsce powstania integracji, zachowano wskazanie API ESPHome i protokołu Waveshare. Nie publikowano surowej rozmowy. Pozostałe problemy dawnych tagów/PR nadal obowiązują.
