# Lokalne obrazy firmware

Aktywne źródła znajdują się w katalogu głównym projektu. Kompiluj przez Kompiluj-monitor-DPF.ps1.

V1/ zawiera lokalnie wygenerowane obrazy dla roboczo przyjętej płytki V1 (CS GPIO9). Archiwalne warianty, szczególnie V1.1 (GPIO46), nie są właściwym obrazem dla obecnego monitora bez nowych danych o sprzęcie.

BIN/ELF nie trafiają do nowych commitów ani załączników Releases: mogą zawierać dane z lokalnego secrets.yaml. Wydanie GitHub wskazuje źródła, nie gotowy bezpieczny obraz do dystrybucji. Historyczny source-bundle ZIP zawiera snapshot źródeł, nie bieżący firmware.

Datę i wynik faktycznego wgrania sprawdzaj w PROJECT_STATUS.md, a wersje źródeł w CHANGELOG.md. Obecność pliku w V1/ ani tagu Git nie potwierdza, że ta wersja działa na urządzeniu.
