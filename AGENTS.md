# Instrukcje pracy nad monitorem DPF

- Odpowiadaj użytkownikowi po polsku. Korzystaj z repozytorium megtomek/vag-dpf-monitor i zasad CONTRIBUTING.md.
- Przed zmianą przeczytaj README.md, bieżącą sekcję PROJECT_STATUS.md i dokument dotyczący zadania. Kod oraz rzeczywiste pomiary są ważniejsze od historycznych notatek.
- Aktywna konfiguracja: audi-dpf-monitor.yaml. Nie buduj przypadkowo źródeł z archive/, docs/history/ lub tests/.
- Rozwój prowadź w gałęziach codex/<temat>, zapisuj commity i PR. Main pozostaw spójną bazą; bez force-push i nadpisywania zmian użytkownika.
- Użytkownik upoważnił do rozwijania kodu i umieszczania kolejnych wersji w tym repozytorium w zakresie jego zleceń. Nie zmieniaj widoczności ani uprawnień repozytorium bez osobnego zlecenia.
- Zachowuj rozróżnienie: szacowany alarm EGT3, status potwierdzony ECU, DEMO. Pierwsze realne zdarzenie nie potwierdziło bitów 115B/12E7/134F/11C1.
- Nie zmieniaj progów ani pinów tylko z powodu ogólnego opisu z sieci. Płytka V1 jest założeniem roboczym, a ESP32-S3 v0.2 to rewizja krzemu.
- Nie dodawaj ani nie uruchamiaj testów, jeśli użytkownik o to nie prosi. Raportuj dokładnie zakres wykonanej weryfikacji. Nie traktuj kompilacji jako sprawdzenia urządzenia.
- Zmiana dokumentacji nie wymaga flashowania. Przed wgraniem potwierdź aktualny adres/port i stan rejestracji; zachowaj potrzebne dane RAM.
- Nigdy nie commituj secrets.yaml, gotowych obrazów z sekretami ani surowych lokalnych logów/załączników bez osobnego zlecenia i przeglądu danych. Nie usuwaj lokalnych plików diagnostycznych w ramach porządków.
- Zaktualizuj CHANGELOG.md i odpowiedni raport/dokumentację przy zmianie zachowania. Wydanie źródeł i wersję rzeczywiście wgraną dokumentuj osobno.
