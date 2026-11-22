# Wydania i rzeczywiste wdrożenia

Nowe repozytorium rozpoczyna się od oczyszczonej kopii źródeł z 21.11.2026. W chwili przygotowania publikacji 22.11.2026 nie ma własnych tagów ani GitHub Releases. Do rozpoczęcia pracy używaj main i zapisuj hash pobranego commitu.

## Zakres pierwszej publikacji źródeł

- Baza funkcjonalna wywodzi się z wcześniejszego v0.1.3, z uzupełnioną dokumentacją, GPL-3.0-only i notami zależności.
- Nie obejmuje niescalonych rozszerzeń dawnego PR #17/#18. Nie jest identyczna z ostatnim eksperymentalnym obrazem urządzenia z 03.11.
- Przy przygotowaniu publikacji nie wykonano nowej kompilacji ani flashowania. Nie deklarujemy nowego sprawdzenia na samochodzie.
- Alarm pozostaje szacowany z EGT3; brak potwierdzonej flagi regeneracji CJCD.
- Brak gotowego BIN do dystrybucji: obrazy buduje się lokalnie z własnymi sekretami.

[Stan i pomiary](PROJECT_STATUS.md) · [historia opisów poprzednich wydań](docs/history/RELEASES-before-migration-2026-11-22.md) · [szablon kolejnych wydań](.github/RELEASE_TEMPLATE.md).
