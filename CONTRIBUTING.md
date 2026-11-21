# Współpraca / Contributing

Zgłoszenia i poprawki PL/EN są mile widziane. Zacznij od [README](README.md), [stanu](PROJECT_STATUS.md), dokumentu funkcji i [zasad społeczności](CODE_OF_CONDUCT.md).

## Licencja wkładu

Własny kod i dokumentacja projektu mają licencję **GPL-3.0-only** — [LICENSE](LICENSE), [COPYRIGHT](COPYRIGHT). Wysyłając własny wkład z przeznaczeniem do włączenia do projektu, udostępniasz go na tych samych warunkach. Musisz mieć prawo do jego udostępnienia. Zachowujesz swoje prawa autorskie; nie wymagamy przeniesienia praw ani CLA.

Przed dodaniem cudzych materiałów wskaż ich pochodzenie i licencję oraz uzgodnij zgodność. Istniejące zewnętrzne noty pozostają w mocy; nowy LICENSE ich nie zastępuje — [THIRD_PARTY_NOTICES](THIRD_PARTY_NOTICES.md).

## Potrzebne prace

Porównania z niezależnym narzędziem dla dokładnego ECU/SW/ODX, pomiary naturalnych zdarzeń i zwykłej jazdy, odtwarzalne błędy, instalacja na czystym komputerze, dokumentacja i tłumaczenia. Nie uruchamiaj wypalania serwisowego wyłącznie dla projektu. Nie kopiuj mapy DID do podobnego silnika bez dowodów.

EGT3 szacowane, potwierdzony status ECU i DEMO pozostają oddzielone. Zerowe 115B/12E7/134F/11C1 nie potwierdzają regeneracji.

## Pull request

1. Uzgodnij większą zmianę w Issue. Zewnętrzni autorzy mogą użyć forka, współpracownicy gałęzi repozytorium.
2. Zacznij od aktualnego `main` w czystym checkout. Agent używa `codex/<temat>`; pozostali krótkiej nazwy. Bez nadpisywania cudzej pracy i force-push.
3. Jeden spójny zakres. Aktywna konfiguracja: `audi-dpf-monitor.yaml`; archiwa/testowe YAML nie są produkcyjną bazą.
4. Przy zmianie zachowania uaktualnij CHANGELOG i dokumentację; mapowania utrzymuj w [tabeli](PARAMETER_REFERENCE.md).
5. Raportuj osobno konfigurację/kompilację, upload, urządzenie i porównanie ECU. Wskaż niesprawdzone scenariusze. Dokumentacja nie wymaga flashowania. Zgodnie z bieżącymi zasadami repozytorium nie dodawaj ani nie uruchamiaj testów bez uzgodnienia z maintainerem.
6. Przejrzyj diff i listę plików: bez sekretów, BIN/ELF i surowych lokalnych danych. Diagnostykę dołączaj tylko po osobnej zgodzie i anonimizacji.
7. Użyj szablonu PR. Brak potwierdzenia na urządzeniu może oznaczać draft; kompilacja nie wystarcza do uznania gotowości w aucie.

Przed OTA/resetem zachowaj potrzebne bufory RAM i sprawdź brak zdarzenia. Nie zmieniaj progów/pinów/mapy z powodu ogólnej porady z sieci.

## Nowy profil i dane

Podaj kod silnika, ECU, SW/HW, ODX/ASAM i CALID, bez VIN. Porównanie powinno zawierać parametr, DID/PID, raw, wzór/jednostkę, wartość odniesienia, czas i warunki silnika/zapłonu. Różnica czasu może tłumaczyć różnicę wskazań.

Udostępniaj wyłącznie legalnie pozyskane materiały z prawem do publikacji. Nie przesyłaj zastrzeżonych ODX/ROD i płatnych baz diagnostycznych. Oryginały pomiarów przechowuj prywatnie.

## Wydania

`main` to baza źródeł; tagi są niezmienne. Używaj [szablonu wydania](.github/RELEASE_TEMPLATE.md): zmiany, rzeczywiście wgrane, sprawdzone, pozostaje do sprawdzenia. Źródła i obraz na urządzeniu to różne wyniki. Nie dodawaj BIN z sekretami do Releases. Datowanych raportów nie przepisuj tak, aby sugerowały późniejszą walidację.

## English

English reports are welcome. Start from current `main`, discuss larger changes and keep one topic per PR. Preserve estimated/ECU-confirmed/DEMO distinctions. Include exact hardware/ECU identity and evidence, redact vehicle/personal/network data, never share credential-bearing firmware or proprietary diagnostic databases. Separate build, upload and physical checks. Tests require maintainer agreement under current policy. Original code/documentation and original contributions submitted for inclusion use GPL-3.0-only; contributors retain copyright and must have the necessary rights. Preserve third-party notices and agree compatibility before adding external material.
