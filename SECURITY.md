# Bezpieczeństwo / Security

## Zakres

To firmware rozwojowy. Poprawki przygotowywane są dla bieżącej linii `main`; nie deklarujemy utrzymania starszych tagów ani SLA. Obserwowane zdarzenia nie są certyfikacją bezpieczeństwa samochodowego.

Panel używa **HTTP Digest bez TLS**. Natywne OTA wymaga lokalnego hasła. Uwierzytelnienie nie szyfruje całej komunikacji. Używaj zaufanej sieci i unikalnych haseł; nie wystawiaj portów WWW/OTA do internetu. BIN może zawierać sekrety i nie nadaje się do udostępniania.

NORMAL wysyła żądania odczytu OBD/UDS — nie jest pasywnym nasłuchem. Rejestrator pasywny jest osobnym trybem; SELFTEST nadaje syntetyczne ramki i wymaga fizycznie odłączonego auta/transceivera.

## Zgłoszenie

Jeśli w [Security → Advisories](https://github.com/megtomek/vag-dpf-monitor/security/advisories) jest przycisk **Report a vulnerability**, użyj prywatnego formularza. Dostępność zależy od ustawień; ten plik nie włącza funkcji.

Jeśli formularza nie ma, otwórz neutralne Issue z prośbą o prywatny kanał kontaktu, bez szczegółów exploita i danych dostępowych. Prześlij szczegóły dopiero po uzgodnieniu prywatnego kanału. Nie publikujemy niesprawdzonego adresu kontaktowego ani nie obiecujemy czasu reakcji.

W prywatnym zgłoszeniu podaj wersję, komponent, warunki, kroki odtworzenia i wpływ. Własne ujawnione hasła/tokeny zmień lub unieważnij; usunięcie pliku nie usuwa historii Git.

## English

Use **Security → Advisories → Report a vulnerability** if enabled. Otherwise open a neutral issue requesting a private channel, without exploit details, credentials or vehicle data; wait for an agreed private channel before sending details. No response SLA is promised.

HTTP has no TLS; password authentication is not full transport encryption. Do not expose HTTP/OTA to the internet or share credential-bearing firmware. NORMAL sends read requests; passive capture is separate. Keep SELFTEST disconnected from the car.
