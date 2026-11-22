# Stan przygotowania publikacji

Aktualizacja: 22.11.2026. Dotyczy nowego repozytorium z oczyszczoną historią, nie usuniętego poprzednika. Właściciel zlecił zmianę widoczności na publiczną po poprawkach i kontroli.

## Źródła i prywatność

- [x] Nowa historia zaczyna się od przejrzanej kopii źródeł; dawne PR, Issues, tagi i Releases nie zostały przeniesione.
- [x] Nie ma secrets.yaml, obrazów BIN/ELF, surowych logów ani dawnych załączników. Usuwane prywatne e-maile i MAC nie występowały w kopii źródeł.
- [x] Stary ZIP komponentów wycofano; jego prywatna kopia pozostała lokalnie. Nie ma go w nowej historii.
- [x] GPL-3.0-only własnych wkładów oraz noty ESPHome, panelu, Lit, Iconify, MDI i fontów. Szczegóły: [THIRD_PARTY_NOTICES](../THIRD_PARTY_NOTICES.md).
- [x] Pochodzenie FT3168 uzupełniono na podstawie lokalnego zapisu tworzenia plików; surowej rozmowy nie publikowano. Baza panelu porównana z oficjalnym artefaktem CDN.

## Dokumentacja

- [x] README PL/EN opisuje eksperymentalny profil CJCD i szacowany alarm EGT3.
- [x] Usunięto odnośniki do dawnych PR/Issues udające obiekty bieżącego repozytorium. [Zakres przeniesienia](REPOSITORY_MIGRATION.md) wyjaśnia dawne numery i hashe.
- [x] Instrukcje budowania, obsługi, zgłoszeń, bezpieczeństwa, sprzęt i licencje są dostępne w źródłach.
- [x] Wydanie źródeł rozdzielono od ostatniego obrazu urządzenia. Dawne v0.1.x są opisami historycznymi, nie tagami obecnego repozytorium.

## Kontrole końcowe

Skany sekretów, przegląd listy plików i metadanych Git oraz kontrola lokalnych odnośników są wykonywane przed zmianą widoczności. Wynik i zakres zostaną opisane w PR przygotowującym publikację. Skan nie jest gwarancją wykrycia dowolnego dawnego sekretu; nie przeprowadzamy audytu całego komputera.

## Pozostały zakres rozwoju

- Instalacja na niezależnym czystym komputerze pozostaje niesprawdzona.
- Nie wykonano nowego builda, flashowania ani obserwacji monitora przy przygotowaniu publikacji.
- Inne ECU, ogólna skuteczność alarmu oraz potwierdzona flaga regeneracji wymagają dalszych danych.
- Dokładny commit budowania bazy JS i wersja fragmentu Iconify nie zostały odtworzone; zachowano źródło artefaktu, hash i licencje.
- Zachowanie panelu i wszystkich dynamicznych ikon bez internetu wymaga próby; nie deklarujemy pełnego trybu offline panelu.

Te ograniczenia są jawne i nie są przedstawiane jako zakończona walidacja urządzenia. Projekt może być udostępniony jako eksperymentalne źródła; nie jako uniwersalny, certyfikowany monitor ECU. Prywatne zgłaszanie podatności i dostępne zabezpieczenia GitHuba są sprawdzane przy publikacji; sama dokumentacja ich nie włącza.
