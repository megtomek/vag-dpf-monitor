# Zbieranie danych profilu DPF przez OBDeleven

Ten arkusz służy do potwierdzenia znaczenia i zachowania parametrów dla konkretnego ECU. Odczyty z aplikacji są próbkami referencyjnymi; same w sobie nie ujawniają surowych bajtów DID potrzebnych do wpisania mapy do firmware.

## Gdzie znaleźć dane

W aplikacji VAG OBDeleven połącz się z autem, otwórz **Control units / Sterowniki → Engine / Silnik → Live data / Dane bieżące** i wyszukaj hasła `particle filter`, `soot`, `regeneration`, `exhaust gas temperature` oraz `differential pressure`. Zaznacz niewielki zestaw parametrów, naciśnij aktualizację i otwórz wykres. Nazwy menu mogą się nieznacznie różnić między wersjami aplikacji.

Oficjalna pomoc OBDeleven opisuje Live Data i wykresy; przy pięciu obserwowanych wartościach odświeżanie każdej może wynosić około 2 razy na sekundę, a przy jednej około 10 razy na sekundę. Na początek wybierz przede wszystkim status regeneracji i temperatury. [Live Data](https://support.obdeleven.com/en/articles/11009883-what-is-live-data), [Live Data UDS](https://support.obdeleven.com/en/articles/5627045-live-data-uds), [Charts](https://support.obdeleven.com/en/articles/5608818-charts)

## Opcjonalna niezależna referencja: VAG DPF Free

Twórca aplikacji VAG DPF wymienia CJCD wśród silników pomyślnie przetestowanych przez użytkowników i zaleca najpierw sprawdzić zgodność wersją VAG DPF Free. Aplikacja korzysta z własnych komend CAN zamiast standardowych zapytań OBD-II; zgodność może zależeć od wersji oprogramowania ECU. To dodatkowy punkt odniesienia dla wartości DPF, a nie źródło surowych DID-ów ani zamiennik porównania z OBDeleven. [Lista silników i uwagi twórcy](https://play.google.com/store/apps/details?hl=pl&id=com.applagapp.vagdpf)

Jeśli użyjesz VAG DPF Free, zanotuj wersję aplikacji, wynik rozpoznania silnika/grupy, dokładne nazwy i jednostki widocznych parametrów oraz czas próbki. Nie kupuj pełnej aplikacji wyłącznie po to, by ustalić adresy UDS. Jej dziennik wartości, jeśli włączysz go w aplikacji, może służyć jako zapis przebiegu i czasu; nie zakładaj, że zawiera żądania CAN, surowe odpowiedzi ECU albo mapę DID. [Polityka prywatności i opis lokalnego logu](https://www.applagapp.com/VAG_DPF_privacy-policy.htm)

## Identyfikacja sterownika

Skopiuj lub sfotografuj ekran identyfikacji sterownika silnika. Zapisz:

| Pole | Wartość |
|---|---|
| Kod silnika | CJCD (do potwierdzenia) |
| Numer oprogramowania ECU | 03L906018SJ |
| Wersja oprogramowania | 9977 |
| Numer sterownika HW | 03L906018JL |
| Wersja komponentu silnika widoczna w skanie | H25 |
| Wersja HW ECU, jeśli aplikacja pokazuje osobne pole | do sprawdzenia; w jednym odczycie ECUHARD widnieje X25 |
| Nazwa ODX/ASAM i wersja | EV_ECM20TDI01103L906018SJ / 003003 |
| Identyfikacja z OBDeleven — pełny zrzut | |

Aktualny ekran monitora nie wyświetla nazwy ECU ani CALID. Jeśli po połączeniu przez OBD log ESPHome zawiera `OBD ECU name` lub `OBD calibration ID(s)`, zapisz te wpisy jako pomocnicze porównanie. Brak tych wpisów nie oznacza błędu monitora — część sterowników nie udostępnia tych informacji przez OBD Service 09. Do identyfikacji referencyjnej użyj odczytu OBDeleven; sama nazwa/CALID nie potwierdza mapy DID-ów DPF.

Zapisz dokładnie tekst z aplikacji. Nie poprawiaj samodzielnie ewentualnych różnic H25/X25.

## Próbki do zebrania

W każdej próbce zapisz nazwę parametru dokładnie jak w OBDeleven, jednostkę, wartość, stan silnika i czas. Zrzuty ekranu są przydatne, szczególnie jeśli pokazują dodatkowy identyfikator `IDE...` lub `ENG...`.

| Warunek | Data/godzina | RPM | Temp. płynu | Status/żądanie regeneracji | Temp. przed DPF | Temp. przed turbiną | Obliczona sadza | Zmierzona sadza | Inne |
|---|---|---:|---:|---|---:|---:|---:|---:|---|
| Zapłon włączony, silnik wyłączony | | | | | | | | | |
| Rozgrzany silnik, bieg jałowy, bez regeneracji | | | | | | | | | |
| Normalna jazda, regeneracja nieaktywna | | | | | | | | | |
| Obserwowana automatyczna regeneracja | | | | | | | | | |
| Po zakończeniu regeneracji | | | | | | | | | |

### Aktualna karta porównawcza w monitorze

Aktualny firmware diagnostyczny udostępnia ścieżkę **USTAW → DANE DPF DO PORÓWNANIA**. Pokazuje sześć kandydatów: `114F` (obliczona sadza), `114E` (zmierzona sadza), `14F5` (ciśnienie DPF), `1156` (kandydat U32 big-endian w metrach), `11B2` (temperatura wlotu DPF) i `10FB` (temperatura turbosprężarki). Są to DID-y z innego projektu VAG. Ręczne wybranie profilu CJCD udostępnia na zwykłych kartach DPF/FILTR tylko `114F`, `114E` i `14F5` — częściowo porównane podczas jednej sesji z OBDeleven. `1156` nadal jest diagnostyczny mimo zgodnej wartości U32 z późniejszego logu; temperatura oraz status regeneracji również nie zostały potwierdzone. Żaden kandydacki DID nie steruje alarmem.

Status u góry strony opisuje odebrane ramki CAN: `BRAK RUCHU`, `RUCH WYKRYTY` albo `CISZA`. Nie potwierdza on sam z siebie odpowiedzi na DID. `NRC xx` oznacza negatywną odpowiedź ECU; `--` oznacza brak aktualnej wartości. Surowe bajty odpowiedzi trafiają do logu USB.

#### Pierwsze porównanie z OBDeleven — 2026-10-11

Zdjęcia z jednej sesji pokazują monitor około 11:05 i OBDeleven około 11:04; urządzenia nie próbkowały w tej samej chwili.

| Parametr | Monitor / DID | OBDeleven | Wniosek |
|---|---:|---:|---|
| Sadza obliczona | `114F = 1714` | `17.13 g` | Skala `0.01 g/bit` daje `17.14 g`, różnica `0.01 g`; zgodność bardzo dobra, możliwa zmiana próbki/zaokrąglenie. |
| Sadza zmierzona | `114E = 1677` | `16.77 g` | Dokładna zgodność przy `0.01 g/bit`. |
| Różnica ciśnienia DPF | `14F5 = 13` | `13 hPa` | Dokładna zgodność liczbowa; jednostkę hPa traktujemy jako wstępnie potwierdzoną jednym porównaniem. |
| Dystans od regeneracji | na ekranie `0.8 km`; surowe bajty z późniejszej jazdy `00 03 A2 E9` | `202256 m` (`202.256 km`) wcześniej; późniejszy log monitora: `238313 m` | Wartość `0.8 km` wynikała z błędu dekodera: odpowiedź ma 4 bajty, a stary kod brał tylko 3 pierwsze (`00 03 A2` = 930, czyli `0.9 km`). Pełne U32 `00 03 A2 E9` = `238313 m` = `238.313 km`. To silnie wspiera U32 big-endian w metrach. |
| Temperatura | `11B2 = 75.3 °C` | Symulowana temperatura powierzchni DPF `66.3 °C` | Nie są to potwierdzone równoważne kanały; nie wyciągamy wniosku o skali ani błędzie. |
| Temperatura turbosprężarki | `10FB = 15.0 °C` | Brak odpowiednika na zrzucie | Kandydat bez porównania. |

Wartości `114F`, `114E`, `14F5` i `1156` są wyświetlane na stronie diagnostycznej z empirycznie dopasowanymi jednostkami. Po ręcznym wybraniu profilu CJCD pierwsze trzy trafiają również na zwykłe karty DPF/FILTR: `114F` jako obliczona sadza, `114E` jako zmierzona sadza oraz `14F5` jako ciśnienie. To częściowe mapowanie oparte na jednym porównaniu OBDeleven; należy je powtórzyć w innych warunkach i potwierdzić interpretację/kodowanie, szczególnie dla wartości ujemnych `114E`. `1156`, temperatury `11B2`/`10FB` oraz stan regeneracji nie są promowane na zwykłe karty ani używane przez alarm. W logu z jazdy zapisano także `114F=1904` (kandydat `19.04 g`), `114E=1857` (`18.57 g`), `14F5=14` (`14 hPa`), `11B2=146.3 °C` oraz `10FB=123.8 °C`.

Przy kolejnych pomiarach zanotuj każdą wartość i jej DID, a następnie odczytaj w OBDeleven odpowiadające jej nazwane parametry. Zapisz dokładne nazwy i jednostki z aplikacji oraz porównuj w podobnym stanie silnika. Bieżący tryb porównawczy odpytuje kandydackie DID-y naprzemiennie ze standardowymi PID-ami OBD-II, zawsze z najwyżej jednym żądaniem w toku. Wstępne odczyty `114F`/`114E`/`14F5` na zwykłych kartach służą do obserwacji i wymagają ponownego porównania; nie używaj żadnego DID-u jako podstawy alarmu regeneracji.

Ekran `DANE DPF DO PORÓWNANIA` pokazuje tylko kandydackie DID-y. Standardowe PID-y OBD-II (m.in. RPM, temperatury, ciśnienie i status układu oczyszczania spalin) są prezentowane na zwykłych kartach monitora, jeśli ECU je obsługuje. Każdy PID porównuj z OBDeleven według dokładnej nazwy i jednostki; samo pojawienie się liczby nie potwierdza, że odpowiada ona kanałowi o podobnej etykiecie.

W czasie jazdy nie obsługuj telefonu ani aplikacji. Dane może zapisywać pasażer; w przeciwnym razie zatrzymaj auto bezpiecznie przed wykonaniem zrzutów. Nie uruchamiaj regeneracji serwisowej ani adaptacji na potrzeby tego pomiaru.

### Gdy jest tylko jedno gniazdo OBD-II

Do wstępnego porównania nie trzeba mieć obu urządzeń podłączonych jednocześnie. Nie używaj równolegle dwóch aktywnych testerów na tym samym ECU: monitor i OBDeleven mogą wysyłać zapytania do tego samego adresu diagnostycznego, a przeplatanie odpowiedzi utrudniłoby przypisanie ich do właściwego żądania.

1. Wybierz stabilne warunki na postoju, najlepiej rozgrzany silnik na biegu jałowym. Zostaw DEMO wyłączone.
2. Z monitorem podłączonym do OBD zapisz godzinę, stan silnika i widoczne wartości. Jeśli dostępny jest log USB, zachowaj także odpowiedzi PID 0x8B A–G.
3. Zatrzymaj auto w bezpiecznym miejscu i zamień urządzenia. Jeśli do odłączenia trzeba zmienić stan zapłonu, po ponownym uruchomieniu poczekaj na ustabilizowanie obrotów i temperatury.
4. W OBDeleven wybierz te same parametry, możliwie małą ich liczbę, i zanotuj dokładną nazwę, wartość, jednostkę oraz godzinę.
5. Porównuj w pierwszej kolejności parametry powolne lub prawie stałe, np. masę sadzy i dystans od regeneracji. RPM oraz EGT są dynamiczne: próbki wykonane kolejno nie dowodzą zgodności co do tej samej chwili; dopisz odstęp czasu i warunki.

Jedno gniazdo wystarczy do wstępnej kontroli zakresów, jednostek i zachowania parametrów. Nie daje jednak równoczesnego potwierdzenia alarmu podczas tej samej regeneracji. Dla takiej walidacji potrzebny byłby kontrolowany zapis obu źródeł bez nakładających się żądań diagnostycznych; zwykły rozdzielacz OBD sam w sobie tego nie zapewnia.

## Arkusz porównania monitora z OBDeleven

Dla każdej próbki z powyższej tabeli wypełnij również jeden wiersz poniżej. Ponieważ urządzenia mogą być podłączane kolejno, wpisuj oddzielnie czas odczytu monitora i OBDeleven oraz różnicę w sekundach. Odczyty nie są wtedy równoczesne; odstęp i warunki pracy silnika są częścią wyniku, szczególnie dla szybko zmieniających się EGT.

Do wpisania próbek w arkuszu kalkulacyjnym służy [DPF_PROFILE_COMPARISON.csv](DPF_PROFILE_COMPARISON.csv). Kolumny `monitor_data_godzina`, `OBD_data_godzina` i `odstęp_czasu_s` dokumentują pomiar sekwencyjny. CSV używa średników jako separatorów, typowych dla polskich ustawień Excela. Zostaw pole puste, gdy dana wartość nie została odczytana; nie wpisuj zera jako zamiennika braku danych. Nazwy parametrów przepisuj dokładnie z OBDeleven, a przy ciśnieniu zachowaj jednostkę podaną przez aplikację.

| Warunek / czas | Parametr OBDeleven: dokładna nazwa | OBDeleven: wartość i jednostka | Monitor: ekran i wskazania | Status na monitorze | Log PID 0x8B: bajty A–G (hex) | A0/A1 obsługiwane | B0/B1 zdekodowane | Alarm „NIE GAŚ!” zgodny? |
|---|---|---|---|---|---|---|---|---|
| Zapłon włączony, silnik wyłączony | | | | | | | | |
| Rozgrzany silnik, bieg jałowy | | | | | | | | |
| Jazda, regeneracja nieaktywna | | | | | | | | |
| Obserwowana regeneracja automatyczna | | | | | | | | |
| Po zakończeniu regeneracji | | | | | | | | |

W kolumnie „Status na monitorze” wpisz dokładnie jeden z widocznych stanów, np. `REGENERACJA DPF`, `REGEN PASYWNA`, `DPF OK` albo `BRAK DANYCH`. W „Monitor: ekran i wskazania” zanotuj wyświetlone liczby wraz z jednostkami, np. RPM, temperatury i masę sadzy. Nie traktuj `BRAK DANYCH` jako zera.

Surowe bajty PID 0x8B pochodzą z logu USB firmware, a nie z OBDeleven. Jeśli logu nie ma lub sterownik odrzuca PID, wpisz `brak odpowiedzi`; nie dopisuj bajtów na podstawie wartości prezentowanej przez aplikację. `A0/A1` i `B0/B1` przepisz z logu lub wpisz `n/d`, jeśli ich nie udostępnia. Rozbieżność alarmu zapisuj jako obserwację do analizy — sam wskaźnik z aplikacji nie dowodzi, że ECU implementuje PID 0x8B zgodnie z aktualną tabelą SAE.

## Krótki zestaw na wykres

Zacznij od 3–5 wartości:

1. Status regeneracji lub żądanie regeneracji — zachowaj dokładną nazwę i wszystkie wartości tekstowe/bitowe. Podczas obserwowanej regeneracji sprawdź, czy ekran monitora zgłasza aktywny status. Po odebraniu ruchu CAN firmware odpytuje standardowy OBD-II PID `0x8B` i zapisuje w logu pełne bajty danych A–G także wtedy, gdy bity obsługi są wyzerowane. Przy ciszy CAN żądań nie wysyła się, dlatego ten test wymaga podłączonego pojazdu z aktywną magistralą. Obecny dekoder bazuje na tabeli B118 SAE J1979-DA z października 2011: A0 deklaruje obsługę statusu regeneracji, A1 obsługę typu, B0 oznacza regenerację w toku, a B1 rozróżnia aktywną (1) i pasywną (0). Oficjalny katalog SAE wskazuje nowszą rewizję `J1979DA_202607` (zweryfikowaną 16 lipca 2026 r.), ale jej treść nie jest dostępna w używanym widoku; zgodność kodu z aktualną tabelą pozostaje więc niepotwierdzona. Alarm `NIE GAŚ!` wymaga świeżej odpowiedzi oraz jawnej obsługi obu pól, a następnie jednocześnie B0=1 i B1=1. Oznaczenia bitów i ich semantykę trzeba zweryfikować dla docelowej wersji ECU; OBDeleven może pokazać wartość nazwaną przez VAG zamiast surowego PID-u, więc porównuj semantykę i moment zmiany stanu, nie zakładaj, że aplikacja ujawnia te same bajty. [Rekord rewizji SAE](https://saemobilus.sae.org/standards/j1979da_202607-j1979-da-digital-annex-e-e-diagnostic-test-modes)
2. Temperatura spalin przed DPF.
3. Temperatura spalin przed turbosprężarką lub najbliższy dostępny czujnik EGT.
4. Obliczona masa sadzy.
5. RPM albo temperatura płynu jako kontekst.

Jeśli status jest prezentowany jako maska/bitfield, zapisz pełne bajty albo ciągi w każdej fazie. Nie redukuj tego samodzielnie do `0/1`. Kod firmware interpretuje PID `0x8B` zgodnie z tabelą B118 SAE J1979-DA Oct 2011, ale obecność i zgodność tej funkcji w konkretnym ECU nadal są niepotwierdzone.

## Co wystarczy, a czego brakuje

- Nazwa IDE i wartości podczas regeneracji pomagają potwierdzić semantykę oraz zakres.
- Oficjalna instrukcja [Live Data](https://support.obdeleven.com/en/articles/5627045-live-data-uds) opisuje wybór nazwanych parametrów sterownika i prezentację ich bieżących wartości; nie opisuje eksportu surowych żądań/odpowiedzi CAN/UDS dla Live Data. Osobna funkcja [Raw Data](https://support.obdeleven.com/en/articles/5627138-what-is-one-click-apps-oca-builder-and-how-to-use-it) jest dokumentowana dla pól debugowania używanych m.in. przy kodowaniu i adaptacjach. Nie zakładaj, że włączenie tej opcji pokaże surowe DID-y parametrów DPF.
- Próbki Live Data z OBDeleven pomogą potwierdzić znaczenie, jednostki i zachowanie wartości, ale do implementacji `uds_audi.h` nadal potrzebny jest surowy DID, długość, kodowanie i skala z wiarygodnego źródła ODX/ASAM albo śladu diagnostycznego.
- PID `0x8B` jest niezależnym standardowym kandydatem dla alarmu regeneracji. Dekoder opiera się na tabeli SAE J1979-DA Oct 2011: A0/A1 deklarują obsługę pól, B0 wskazuje regenerację w toku, a B1 jej typ (aktywna/pasywna). Oficjalny katalog wskazuje rewizję `J1979DA_202607` z 16 lipca 2026 r., lecz jej tabeli PID nie porównano. Nie mamy też potwierdzenia implementacji ani semantyki dla ECU `03L906018SJ / 9977 / ODX 003003`. Jeśli monitor pokazuje „brak danych”, zapisz to razem z tym, czy OBDeleven w tym samym czasie pokazywał status regeneracji. [Katalog SAE](https://saemobilus.sae.org/standards/j1979da_202607-j1979-da-digital-annex-e-e-diagnostic-test-modes)
- Dopiero po uzyskaniu tych danych można utworzyć profil dla `03L906018SJ / 9977 / CJCD`. Nie wpisuj `IDE00xxx` w miejsce DID hex i nie używaj placeholderów `0xF401–0xF406` jako prawdziwych adresów.

## Źródła profilu dla dokładnego ECU

- Publiczny katalog procedur diagnostycznych zawiera dokładny wpis `EV_ECM20TDI01103L906018SJ 003003`, zgodny z podanym ODX. W widoku demonstracyjnym szczegóły procedur są zamaskowane, a pełny dostęp jest płatny; sam wpis nie ujawnia mapy DID, długości ani skalowania: [profil ECU 003003](https://diagnostics.vis4vag.com/de/motoren/uds-motoren-nach-teilenummer/a-4653-01-01-motor-ev-ecm20tdi01103l906018sj-003003/?pn=03L906018SJ).
- Projekt [pashokitsme/vagcan](https://github.com/pashokitsme/vagcan) opisuje pobieranie nazw, kodowania i skal kanałów z projektu ODIS (preferowane) lub instalacji VCDS, a następnie korygowanie ich danymi z własnego auta. To wskazuje praktyczną drogę do weryfikacji, ale repozytorium jest narzędziem ogólnym i samo nie potwierdza żadnego z naszych sześciu DID-ów dla tego ECU.
- Projekt [v-cu/dpf-load-monitor-wide](https://github.com/v-cu/dpf-load-monitor-wide) jest niezależnym, otwartym monitorem DPF dla VAG opartym o UDS/CAN. README wymienia podobne grupy parametrów, ale ostrzega, że dostępne DID-y różnią się między kodami silnika i mogą wymagać dopisania dla konkretnego auta. Opublikowany README nie potwierdza mapowania dla `03L906018SJ / 9977 / ODX 003003`, więc nie zmieniono na jego podstawie kandydatów ani dekodera. Projekt podaje licencję CC BY-NC-SA 4.0; nie kopiowano jego kodu ani zasobów.
- Przykładowy publiczny zrzut diagnostyczny z ECU o numerze `03L 906 018 SJ` pokazuje nazwy IDE dla ciśnienia, sadzy i dystansu regeneracji, lecz IDE to nazwy kanałów diagnostycznych, nie adresy UDS. Nie używać go do wyprowadzania DID ani skalowania: [zrzut z forum A4 Klub Polska](https://a4-klub.pl/topic/476849-cz%C4%99ste-wypalanie-dpf-b%C5%82%C4%85d-egr/?comment=5622487&do=findComment).
- Drugi zapis VCDS dla sterownika `03L 906 018 SJ` podaje `IDE00433` jako objętość popiołu olejowego (0,11 l) i `IDE06059` jako masę popiołu (45,6 g). To użyteczne potwierdzenie nazw i jednostek popiołu w kanale DPF dla tej rodziny ECU, ale nie surowych DID-ów, skali sadzy ani progu alarmu; wpis forum nie zastępuje danych ODX i pomiaru własnego auta: [wątek A4 Klub Polska, wpis z 8 czerwca 2021](https://a4-klub.pl/topic/476849-cz%C4%99ste-wypalanie-dpf-b%C5%82%C4%85d-egr/?comment=5622487&do=findComment).

## Powiązany zrzut VCDS — lista nazw, nie mapa DID

Publiczny dokument VCDS z dieselschrauber pokazuje rozszerzone wartości DPF dla sterownika opisanego jako `DRV\03L-906-018-CGL.CLB` i protokół UDS. Wymienia kanały: `IDE00432` czas od regeneracji, `IDE00433` objętość popiołu olejowego, `IDE00434` obliczona masa sadzy, `IDE00435` zmierzona masa sadzy, `IDE00436` dystans od regeneracji, `IDE01407` paliwo zużyte od regeneracji, `IDE01408` granica regeneracji podczas jazdy, `IDE01409` granica regeneracji serwisowej, `IDE01410` granica przeładowania oraz `IDE06059` masa popiołu olejowego. Przykładowe jednostki widoczne w dokumencie to g, l, s i m.

To sterownik z powiązanej rodziny `03L`/R4 2.0 TDI, ale oznaczenie CGL różni się od docelowego `03L906018SJ / 9977 / ODX 003003`. Numery IDE są nazwami kanałów VCDS/ODX, a nie surowymi adresami UDS DID. Nie mapuj ich na kandydackie DID `0x114F` itd. i nie traktuj podanych progów jako norm CJCD. Źródło służy wyłącznie do przygotowania listy nazw do wyszukania w OBDeleven/ODIS: [zrzut VCDS DPF_H1](https://community.dieselschrauber.org/download.php?id=12813).

Ten zrzut nie potwierdza skali, progów, bitu aktywnej regeneracji ani DID-ów dla naszego ECU. Kolejny krok to sprawdzenie dokładnych nazw IDE dostępnych w OBDeleven dla własnego sterownika i zebranie wartości w samochodzie. Do surowej mapy nadal potrzebne są dane ODX/ASAM dla dokładnego numeru oprogramowania albo wiarygodny ślad diagnostyczny.

## Przykład z dokładnie zgodnego ECU (źródło forum, ramka zamrożona)

Znaleziono publiczny skan VCDS z dokładnym identyfikatorem sterownika: `03L 906 018 SJ`, HW `03L 906 018 JL`, `R4 2.0l TDI H25 9977`, ASAM `EV_ECM20TDI01103L906018SJ 003003`, opisanego jako CJCD. W kontekście sporadycznego błędu P2452 czujnika różnicy ciśnień ramka zamrożona zawiera nazwy i przykładowe wskazania: ciśnienie DPF 1047 hPa, offset 5 hPa, dynamiczny offset 7 hPa, sadza zmierzona 19.29 g i obliczona 38.21 g. W innej ramce w tym samym skanie widnieje ciśnienie DPF 10 hPa. To pokazuje, że odczyt zależy od chwili/kontekstu, a uszkodzony lub nieprawidłowy tor czujnika może dać skrajny wynik.

Jest to materiał pomocniczy z forum, a nie kontrolowany pomiar sprawnego filtra: wartości DPF występują w freeze-frame związanym z błędem P2452. Nie wyznaczają normy, progu regeneracji, maksimum sadzy ani DID-u. Skan podaje nazwy opisowe VCDS bez identyfikatorów IDE dla tych pól i bez surowych żądań/odpowiedzi UDS. Zapisuję go jako potwierdzenie, że nazwy „Rußmasse gemessen/berechnet” oraz „Differenzdruck” występują przy dokładnym ECU — nie jako podstawę do zmiany mapy firmware: [wątek MOTOR-TALK, skan z 10.09.2022](https://www.motor-talk.de/forum/drosselklappe-anlernen-t7332377.html?page=1).
