# Przeniesienie repozytorium — 22.11.2026

Bieżące repozytorium megtomek/vag-dpf-monitor ma nową historię zaczynającą się od oczyszczonej kopii 150 plików (początkowy commit ecbf964). Poprzednie repozytorium usunięto na zlecenie właściciela. Lokalny backup kodu oraz metadanych zgłoszeń/wydań pozostał prywatny.

Nie przeniesiono dawnych PR, Issues, tagów, Releases ani starych referencji Git. Dawne numery i hashe zachowano w datowanych raportach jako identyfikatory historycznych pomiarów. Nie są odnośnikami do aktywnych obiektów nowego repozytorium. Numer nowego PR lub Issue może odpowiadać zupełnie innej sprawie.

Baza źródeł wywodzi się z dawnego v0.1.3 i dokumentacji przygotowanej do publikacji. Rozszerzenia dawnego PR #17/#18 pozostają w prywatnym backupie, poza tą bazą. Nie twierdzimy, że firmware urządzenia z 03.11 jest identyczny z main. Nie zmieniono sterowników, progów, pinów ani procedur ECU podczas przeniesienia.

Przed przeniesieniem usunięto prywatne e-maile i konkretne MAC, wycofano stary ZIP komponentów, dodano GPL oraz licencje i noty zewnętrznych zasobów. Skany nie wykryły sekretów w opisanym zakresie; nie są gwarancją wykrycia każdej nieznanej wartości. Historyczne raporty audytu odnoszą się do wcześniejszego repozytorium, a bieżący stan opisuje [checklista publikacji](PUBLICATION_CHECKLIST.md).

Projekt publikowany jest jako eksperymentalne źródła dla częściowo potwierdzonego profilu CJCD. Nie wykonano nowej walidacji urządzenia, instalacji na czystym komputerze ani pełnej oceny fałszywych alarmów. [Ograniczenia i parametry](../PARAMETER_REFERENCE.md).
