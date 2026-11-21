# Historia wersji

Wersje wskazują źródła w GitHubie. Nie oznaczają automatycznego wgrania na urządzenie.

## Niewydane — przegląd pochodzenia i historii, 21.11.2026

- Przejrzano zdalną historię/tagi, historyczne bloby i ZIP, grafiki oraz metadane Issues/PR/Releases. Wynik i nierozstrzygnięte kwestie: docs/PROVENANCE_REVIEW_2026-11-21.md.
- Potwierdzono adaptację CAN z ESPHome i binarną tożsamość fontów Ubuntu 0.83; dodano noty ESPHome, Lit BSD-3-Clause i Iconify MIT oraz poprawiono komentarz licencyjny panelu.
- Nie przepisano historii ani nie zmieniono widoczności. Bez zmiany wykonywalnej treści JS, testów produktu, kompilacji i wgrywania.

## Niewydane — licencja GPLv3, 21.11.2026

- Decyzją właściciela dodano pełny tekst GNU GPLv3 i informację o zakresie: własny kod i dokumentacja na GPL-3.0-only. Zasoby zewnętrzne zachowują swoje warunki.
- Ujednolicono README PL/EN, zasady wkładów, szablon PR i checklistę publikacji. Historyczny etap odłożonego wyboru poniżej pozostaje zapisem wcześniejszej decyzji.
- Bez zmiany zachowania firmware, kompilacji, flashowania i widoczności repozytorium.

## Niewydane — dokumentacja publicznego projektu, 21.11.2026

- README PL/EN, instalacja bez ścieżek autora, instrukcja użytkownika, wsparcie, bezpieczeństwo, zasady społeczności i szablony zgłoszeń.
- Uaktualniony opis sprzętu/profilu, pochodzenie zasobów i checklista publikacji; wybór licencji własnej części odłożony.
- Bez zmiany firmware, kompilacji, wgrywania, testów i widoczności repozytorium.

## Niewydane — porządek dokumentacji, 28.10.2026

- Skrócono bieżący PROJECT_STATUS; pełny poprzedni plik zachowano bez zmiany bajtów w docs/history/.
- Dodano PARAMETER_REFERENCE jako wspólny indeks DID/PID, skal, zastosowań i granic potwierdzenia.
- HIST/UI (#3) zamknięte; dalsze porównania w #13. #1/#2 uaktualnione do v0.1.3. PR #6 pozostaje potrzebny, ale odłożony do aktualizacji i poza wdrożeniem.
- Ujednolicono cztery opisy wydań w RELEASES.md i GitHub Releases; dodano szablon. Starsze opisy zachowane w docs/history/. Tagi bez zmian.
- Bez zmiany firmware, kompilacji, flashowania ani uruchamiania testów. Ostatni funkcjonalny obraz nadal v0.1.3.

## v0.1.3 — 28.10.2026 — odczyty i rozszerzenie rejestratora, prerelease

- Rozszerzony rejestrator: dystans 1156, obserwacyjny trend sadzy, czasy i przyczyny przełączeń rzeczywistego estymatora; osobna ocena schłodzenia/utraty danych w WWW/AP. Stary Event i TEST kompatybilne, nowe dane w osobnych blobach. Progi alarmu bez zmian. Kompilacja i OTA udane; stare zdarzenie identyczne, nowe kolumny i pole WWW potwierdzone, użytkownik zatwierdził ekran i dotyk. Wyłączenie zasilania lub Wi-Fi przez użytkownika wyjaśniło utratę połączenia. Pełne okno trendu pozostaje do sprawdzenia. Trwałe rozszerzenie przy nowym zdarzeniu nadal wymaga naturalnej próby. Raport: docs/REGEN_EVIDENCE_2026-10-28.md.

- Napięcie na DPF: czcionka 18 → 36 px (+100%), sama wartość z przecinkiem, np. 11,5 V, bez prefiksu ECU. Kompilacja i OTA korekty udane, zapis zdarzenia zachowany; użytkownik potwierdził ekran i dotyk.

- FILTR: dystans 1156 U32 BE / 1000 km trafia do normalnej karty przy zgodnym CALID CJCD; dokładność 0,1 km, brak danych jako --. Główny ekran i WWW: napięcie zasilania ECU z PID 42, odczyt świeży lub --. Kompilacja i OTA udane; panel potwierdził 200,8 km i 11,5 V. Zdarzenie zachowane, użytkownik potwierdził obie wartości i układ. Raport: docs/DISTANCE_VOLTAGE_2026-10-28.md.

- Kolejne zadania: DEVELOPMENT_ROADMAP.md i GitHub Issues. Poprawka opcjonalnych sensorów OBD jest rozwijana oddzielnie w PR #6.

## v0.1.2 — 28.10.2026 — historia zdarzeń szacowanych, prerelease

- Karta HIST w trybie szacowanym pokazuje rzeczywiste zapisy event0/event1 (0–2), czas najnowszego i powód zakończenia; odczytuje również starsze zdarzenia. Nie zwiększa licznika cykli potwierdzonych ECU. Kompilacja i OTA udane; odzyskano 1 zapis / 1527 s i zachowano identyczny CSV. Użytkownik potwierdził dane, układ napisów i dotyk. Raport: docs/DEPLOYMENT_2026-10-28_HISTORY.md.

## v0.1.1 — 28.10.2026 — skala sadzy 24 g, prerelease

- Zmieniono skalę paska obliczonej sadzy z 0–40 g na 0–24 g i ograniczenie wypełnienia. Liczba w gramach i logika alarmu bez zmian; 21 g =87,5%.
- Walidacja, kompilacja i OTA udane; użytkownik potwierdził poprawny ekran, dotyk i wypełnienie. CAN odbiera dane, zapisane zdarzenie zachowane. Szczegóły: docs/DEPLOYMENT_2026-10-28_SOOT_BAR.md.
- Porządki lokalne 27.10: 30 logów w archiwum lokalnym, nieużywana kopia ESP-IDF i cache w Koszu, pięć identycznych kopii `.pre-passive` usuniętych. Raport: docs/LOCAL_CLEANUP_2026-10-27.md.
- PR #6 nie jest częścią wydania. Alarm nadal szacowany; nie jest to potwierdzenie flagi ECU ani pełnej skuteczności.

## v0.1.0 — 27.10.2026 — baza użytkowa CJCD, prerelease

Zebrano dotychczasowy rozwój w prywatnym repozytorium. Stan funkcjonalny pochodzi z firmware wgranego 15.10; pierwsze realne zdarzenie przeanalizowano 22.10.

- Odczyty CAN/OBD/UDS, częściowo potwierdzony profil CJCD oraz EGT3 11B2.
- Szacowany alarm EGT3 550°C/30 s, wyjście <450°C/60 s. Jedna próba naturalnego zdarzenia oraz komunikatu na ekranie potwierdzona przez użytkownika.
- Pięć kart AMOLED, czytelna EGT3, DEMO, lokalny panel WWW/AP i trzy zapamiętywane tryby sieci.
- Automatyczne zdarzenie NVS, bufor 120 próbek przejazdu, logi systemowe i osobny TEST pamięci. TEST przetrwał odłączenie zasilania.
- Poprawka flagi połączenia w DEMO i zawijania przycisków panelu WWW.
- Uporządkowano README, plan rozwoju, indeks dokumentacji i zasady pracy. Wcześniejsze opisy zachowano w docs/history/ oraz historii Git.

Ograniczenia: alarm pozostaje szacowany, statusy ECU podczas pierwszego zdarzenia były zerowe, brak pełnej walidacji innych ECU i wszystkich warunków pracy. Zmiana organizacyjna nie zmienia kodu firmware ani progów; nie wykonywano nowego builda, flashowania ani testów.

### Prywatność dokumentacji — 21.11.2026

- Zanonimizowano MAC monitora i dwóch urządzeń LAN w bieżących dokumentach. Metadane e-mail i MAC w historii wymagają osobnego oczyszczenia historii lub nowego repozytorium; nie wykonywano force-push.

- Za zgodą właściciela oczyszczono historię 15 gałęzi i 4 tagów z prywatnych e-maili i MAC; pozostałości PR opisuje docs/PRIVACY_CLEANUP_2026-11-21.md.

- Potwierdzono pełną bazę panelu z oficjalnym artefaktem ESPHome CDN; dodano hash, manifest i opis dwóch różnic, bez zmiany wykonywalnego JS.

- Wycofano historyczny ZIP komponentów z bieżącej dystrybucji po zweryfikowaniu prywatnej kopii; plik lokalny i historia zachowane. Dodano szczegółową notę pochodzenia FT3168 z rozdzieleniem ustaleń i brakujących informacji.

- Odnaleziono lokalny zapis utworzenia integracji FT3168 przez Codex z 03.10.2026; uzupełniono pochodzenie sprzed importu Git bez zmiany kodu i bez publikowania prywatnej rozmowy.

- Uzupełniono noty ikon MDI (Apache-2.0/Pictogrammers) i opisano zewnętrzne serwery ikon w panelu. Weryfikacja statyczna/metadanych, bez zmiany zachowania ani deklaracji pełnego trybu offline.

- Poprawiono siedem odnośników w archiwalnym README wskazujących pliki sprzed przeniesienia do docs/history; bez zmiany historycznych opisów.
