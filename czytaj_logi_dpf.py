#!/usr/bin/env python3
"""Read the ESPHome web_server event stream using HTTP Digest authentication.

Read-only helper for this project. Credentials are loaded from secrets.yaml and
are never printed. Ctrl+C stops the stream; --seconds sets a bounded capture.
"""

from __future__ import annotations

import argparse
import re
import sys
import time
from datetime import datetime
from pathlib import Path

import requests
import yaml
from requests.auth import HTTPDigestAuth

# Windows PowerShell may use a legacy console code page that cannot represent
# every character sent by ESPHome. Replace unsupported glyphs instead of
# crashing the live stream.
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(errors="replace")


def load_credentials(project_dir: Path) -> tuple[str, str]:
    secrets_path = project_dir / "secrets.yaml"
    if not secrets_path.is_file():
        raise FileNotFoundError(f"Nie znaleziono pliku {secrets_path}")
    with secrets_path.open("r", encoding="utf-8") as stream:
        secrets = yaml.safe_load(stream) or {}
    username = secrets.get("web_server_username")
    password = secrets.get("web_server_password")
    if not isinstance(username, str) or not isinstance(password, str):
        raise ValueError("secrets.yaml musi zawierać web_server_username i web_server_password")
    if not username or not password:
        raise ValueError("Dane logowania web_server w secrets.yaml nie mogą być puste")
    return username, password


def main() -> int:
    parser = argparse.ArgumentParser(description="Odczyt logów ESPHome przez HTTP Digest (tylko odczyt).")
    parser.add_argument("--device", default="audi-dpf-monitor.local", help="Adres IP lub nazwa mDNS (domyślnie audi-dpf-monitor.local).")
    parser.add_argument("--port", type=int, default=80, help="Port web_server (domyślnie 80).")
    parser.add_argument("--seconds", type=float, default=0, help="Czas przechwytywania; 0 oznacza do Ctrl+C.")
    parser.add_argument("--output", type=Path, help="Zapis logu UTF-8 z czasem lokalnym; istniejący plik nie jest nadpisywany.")
    args = parser.parse_args()
    if args.port < 1 or args.port > 65535 or args.seconds < 0:
        parser.error("port musi być 1–65535, a seconds nie może być ujemne")

    project_dir = Path(__file__).resolve().parent
    try:
        username, password = load_credentials(project_dir)
    except (OSError, ValueError, yaml.YAMLError) as exc:
        print(f"Błąd konfiguracji: {exc}", file=sys.stderr)
        return 2

    output = None
    if args.output:
        output_path = args.output if args.output.is_absolute() else project_dir / args.output
        try:
            output_path.parent.mkdir(parents=True, exist_ok=True)
            output = output_path.open("x", encoding="utf-8", buffering=1)
        except OSError as exc:
            print(f"Nie można utworzyć logu: {exc}", file=sys.stderr)
            return 5

    def emit(message: str) -> None:
        print(message, flush=True)
        if output is not None:
            timestamp = datetime.now().astimezone().isoformat(timespec="milliseconds")
            output.write(f"{timestamp} {message}\n")

    url = f"http://{args.device}:{args.port}/events"
    deadline = time.monotonic() + args.seconds if args.seconds else None
    emit(f"Łączenie z {url} (HTTP Digest, tylko odczyt). Zatrzymaj Ctrl+C.")
    stream_connected = False
    read_timeout = max(1, min(15, args.seconds)) if args.seconds else 15
    try:
        with requests.get(
            url,
            auth=HTTPDigestAuth(username, password),
            headers={"Accept": "text/event-stream", "Cache-Control": "no-cache"},
            stream=True,
            timeout=(5, read_timeout),
        ) as response:
            if response.status_code == 401:
                emit("HTTP 401: serwer odrzucił dane logowania. Sprawdź web_server_username/password w secrets.yaml.")
                return 3
            response.raise_for_status()
            # Event-stream is UTF-8; requests may otherwise infer Latin-1.
            response.encoding = "utf-8"
            emit(f"Połączono: HTTP {response.status_code}.")
            stream_connected = True
            current_event = ""
            deadline_reached = False
            for line in response.iter_lines(decode_unicode=True, chunk_size=1):
                if deadline is not None and time.monotonic() >= deadline:
                    deadline_reached = True
                    break
                if not line:
                    continue
                if line.startswith("event:"):
                    current_event = line[6:].strip()
                elif line.startswith("data:") and current_event == "log":
                    message = line[5:].strip()
                    message = re.sub(r"\x1b\[[0-9;]*m", "", message)
                    emit(message)
            if deadline is not None and time.monotonic() >= deadline:
                deadline_reached = True
            if deadline_reached:
                emit("Koniec zadanego czasu przechwytywania.")
    except KeyboardInterrupt:
        emit("Zatrzymano odczyt.")
    except requests.RequestException as exc:
        if stream_connected and deadline is not None and time.monotonic() >= deadline and "read timed out" in str(exc).lower():
            emit("Koniec czasu przechwytywania: timeout bez nowych danych SSE; stan urządzenia niepotwierdzony.")
            return 0
        emit(f"Błąd połączenia: {exc}")
        return 4
    except OSError as exc:
        print(f"Błąd zapisu logu: {exc}", file=sys.stderr)
        return 5
    finally:
        if output is not None:
            output.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
