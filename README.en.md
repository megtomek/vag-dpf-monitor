# VAG DPF Monitor

[Polski](README.md) · [Build guide (PL)](BUILD_INSTRUCTIONS.md) · [Hardware](HARDWARE_REFERENCE.md) · [Support](SUPPORT.md)

A standalone ESP32-S3 dashboard for observing diesel particulate filter data over OBD-II/CAN. Touch AMOLED, local Wi-Fi/access-point web panel and event recording; no Home Assistant required.

**Development project with partial support for one ECU configuration.** The primary “REGENERACJA DPF / NIE GAŚ!” warning is estimated from EGT3, not an ECU-confirmed regeneration flag. Three analyzed natural-event datasets support the estimator, but do not establish false-positive or missed-event rates. It must not be the sole indication of vehicle condition.

## Compatibility

- Waveshare **ESP32-S3-Touch-AMOLED-1.64**, 16 MB Flash / 8 MB PSRAM, landscape 456 × 280.
- Working V1 pin configuration, LCD CS GPIO9. Silicon revision v0.2 does not identify PCB revision; scripts support V1 only.
- External **SN65HVD230 at 3.3 V**, TX GPIO5 → D/TXD, RX GPIO6 ← R/RXD; CAN 500 kbit/s, request 7E0 / response 7E8. Remove microSD: TF shares these GPIOs.
- Partial profile: Audi **CJCD 2.0 TDI**, SW **03L906018SJ / 9977**, HW **03L906018JL / H25**, ODX **EV_ECM20TDI01103L906018SJ / 003003**. Other ECUs/software need independent validation.
- Protected automotive input, regulated 5 V and common ground. Never connect battery voltage to board/GPIO. See [wiring](HARDWARE_REFERENCE.md).

## Features and limits

Five pages: DPF, TEMP, FILTR, HIST and USTAW. Readouts include calculated/measured soot, pressure difference, EGT3, distance since regeneration and ECU-reported voltage. Availability and evidence are listed in the [parameter reference](PARAMETER_REFERENCE.md).

The estimated warning starts at EGT3 ≥550°C for 30 s and clears below 450°C for 60 s. Stale data makes the estimate unavailable; clearing the warning does not prove a completed ECU regeneration. Trends are recorded for analysis only.

Two event snapshots are retained in NVS; approximately ten minutes of drive data and system logs are in RAM. Download CSV/TXT through the panel. Router Wi-Fi, AP-only and radio-off modes survive restart. Device readings and recording work without Wi-Fi. DEMO is simulated. Service regeneration is not initiated.

## Getting started

1. Check exact hardware and ECU identity; read the [hardware guide](HARDWARE_REFERENCE.md).
2. Clone `https://github.com/megtomek/vag-dpf-monitor.git`. Copy `secrets.example.yaml` to `secrets.yaml`, then set unique credentials.
3. Follow [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) for ESPHome **2026.9.1** / ESP-IDF **5.5.5**, bench setup, USB and OTA. Active configuration: `audi-dpf-monitor.yaml`. Fresh-machine setup has not been independently reproduced.
4. While parked, select the correct profile, verify ECU identity and compare readings with a diagnostic reference. Use NORMAL for the vehicle; bench-only SELFTEST transmits synthetic frames.
5. See the [user guide](docs/USER_GUIDE.md). AP panel: `http://192.168.4.1/`; router mode uses the current device address.

Build locally: images include local credentials and must not be shared. A build is not physical/vehicle validation. Operate the screen and download data while parked.

## Contributing and status

Polish and English reports are welcome. See [CONTRIBUTING](CONTRIBUTING.md), [SUPPORT](SUPPORT.md), [SECURITY](SECURITY.md) and [Issues](https://github.com/megtomek/vag-dpf-monitor/issues). Redact VINs, credentials, network identifiers and locations.

[PROJECT_STATUS](PROJECT_STATUS.md), [RELEASES](RELEASES.md), [CHANGELOG](CHANGELOG.md) and [documentation index](docs/INDEX.md) track evidence. PR #17/#18 are development branches, not merged `main` features.

## Licensing

Original project code and documentation are licensed under **GNU GPL version 3 only**, `SPDX-License-Identifier: GPL-3.0-only`. See the full [LICENSE](LICENSE) and scope in [COPYRIGHT](COPYRIGHT).

Use, modification and commercial distribution are permitted under the license's conditions, including corresponding-source obligations when distributing covered firmware or derivatives. The project is supplied without warranty as specified in the license. Third-party assets retain their own licenses, including MIT for the upstream frontend, BSD-3-Clause for Lit and Ubuntu Font Licence for fonts; see [THIRD_PARTY_NOTICES](THIRD_PARTY_NOTICES.md) for the remaining provenance review.

Independent community project; not affiliated with Volkswagen Group, Audi, Waveshare or OBDeleven.
