# Archiwum projektu

Aktywny projekt znajduje się w katalogu nadrzędnym. Zawartość tego katalogu jest historyczna i nie bierze udziału w kompilacji.

- `diagnostics/` — logi i skrypty z dawnych testów.
- `codex-legacy-outputs/` — starsze wizualizacje i schematy.
- `legacy-snapshots/dpf-profile-verify-20261003/` — kopia starszej wersji źródeł przeniesiona z `Documents\Codex`.
- `legacy-snapshots/can-passive-activity-backups-20261006/` — kopia plików i wariantów „pre-passive” z `Documents\Codex`.

Snapshoty zachowano do porównania i odtworzenia. Nie używaj ich YAML-i ani plików `.esphome-data` do normalnej kompilacji. Lokalny `secrets.yaml` pozostał poza kontrolą Git dzięki regule ignorowania sekretów.

27.10.2026 usunięto z bieżącego drzewa pięć identycznych kopii `.pre-passive` w snapshocie z 6.10 — zachowano pliki bez przyrostka; mapę i SHA256 podaje [raport porządków](../docs/LOCAL_CLEANUP_2026-10-27.md). Z archiwalnego snapshotu z 3.10 usunięto wyłącznie lokalny wygenerowany cache `.esphome-data`. Źródła snapshotów pozostały.
