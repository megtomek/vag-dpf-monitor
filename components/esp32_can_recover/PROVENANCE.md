# Pochodzenie komponentu CAN

Ten komponent jest adaptacją `esp32_can` z **ESPHome 2026.9.1**, commit **0b2b16e664ecfc0add37f3435db1c2140b8a687e**:

- [canbus.py](https://github.com/esphome/esphome/blob/0b2b16e664ecfc0add37f3435db1c2140b8a687e/esphome/components/esp32_can/canbus.py)
- [esp32_can.cpp](https://github.com/esphome/esphome/blob/0b2b16e664ecfc0add37f3435db1c2140b8a687e/esphome/components/esp32_can/esp32_can.cpp)
- [esp32_can.h](https://github.com/esphome/esphome/blob/0b2b16e664ecfc0add37f3435db1c2140b8a687e/esphome/components/esp32_can/esp32_can.h)

Porównanie 21.11.2026, po mechanicznym ujednoliceniu nazwy namespace/klasy: 147/147 linii upstream Python zachowanych; C++ 180/188 i nagłówek 44/45 linii upstream występują w lokalnej wersji. To miara porównania tekstowego, nie wyliczenie udziału praw autorskich. Ślad CODEOWNERS upstream zachowano, ale nie jest on sam listą właścicieli praw.

Oryginalny projekt ESPHome: Copyright (c) 2019 ESPHome, zgodnie z [zachowaną licencją upstream](../../LICENSES/ESPHome-2026.9.1.txt). Python upstream ma MIT; C/C++ runtime GPLv3. Własne modyfikacje monitora są GPL-3.0-only; noty upstream nadal obowiązują.

Modyfikacje projektu obejmują inną nazwę komponentu, obsługę odzyskiwania bus-off, ograniczone przetwarzanie RX, ponowne użycie bufora, statystyki, warianty diagnostyczne i pasywny rejestrator. Ta nota nie jest nową zmianą zachowania ani walidacją sprzętu.

Wcześniejsze kopie w archive/ i historycznym ZIP również wymagają uwzględnienia pochodzenia ESPHome. Sam ZIP nie zawiera kompletnego zestawu not licencyjnych i nie powinien być publikowany jako samodzielny pakiet bez uzupełnienia.
