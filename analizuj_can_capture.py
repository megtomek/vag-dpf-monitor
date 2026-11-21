"""Offline ISO-TP inspection of the monitor's passive CAN CSV (standard library)."""
import argparse
import csv
from collections import Counter
from pathlib import Path


def analyze(source, destination):
    sessions = {}
    messages = []
    problems = []
    ids = Counter()
    metadata = []
    last_ms = -1
    with source.open(encoding="utf-8-sig") as stream:
        lines = []
        for line in stream:
            if line.startswith("#"):
                metadata.append(line.strip())
            else:
                lines.append(line)
    for row_number, row in enumerate(csv.DictReader(lines), 2):
        try:
            ms = int(row["ms"])
            identifier = int(row["id_hex"], 16)
            extended, rtr, dlc = int(row["extended"]), int(row["rtr"]), int(row["dlc"])
            data = bytes.fromhex(row["data_hex"])
            if not 0 <= dlc <= 8 or extended not in (0, 1) or rtr not in (0, 1):
                raise ValueError("invalid flags/DLC")
            if not 0 <= identifier <= (0x1FFFFFFF if extended else 0x7FF):
                raise ValueError("invalid CAN ID")
            if ms < last_ms:
                raise ValueError("timestamps out of order")
            last_ms = ms
            if not rtr and len(data) != dlc:
                raise ValueError("payload length differs from DLC")
        except (ValueError, KeyError, TypeError) as exc:
            problems.append(f"row {row_number}: {exc}")
            continue
        ids[(identifier, extended)] += 1
        # Only known diagnostic addressing. Other traffic is counted but not
        # interpreted as ISO-TP. Extended/mixed addressing needs a separate map.
        if extended or rtr or not data or not (identifier == 0x7DF or 0x7E0 <= identifier <= 0x7EF):
            continue
        if identifier in sessions and ms - sessions[identifier][4] > 2000:
            problems.append(f"0x{identifier:03X}: incomplete ISO-TP message timed out")
            del sessions[identifier]
        kind = data[0] >> 4
        if kind == 0:
            length = data[0] & 15
            if not 1 <= length <= len(data) - 1:
                problems.append(f"{ms} ms 0x{identifier:03X}: invalid single-frame length")
                continue
            if identifier in sessions:
                problems.append(f"0x{identifier:03X}: single frame interrupted unfinished message")
                del sessions[identifier]
            messages.append((ms, identifier, data[1:1 + length]))
        elif kind == 1:
            if len(data) < 3:
                problems.append(f"{ms} ms 0x{identifier:03X}: short first frame")
                continue
            length = ((data[0] & 15) << 8) | data[1]
            if length <= 7:
                problems.append(f"{ms} ms 0x{identifier:03X}: unsupported/invalid first-frame length")
                continue
            if identifier in sessions:
                problems.append(f"0x{identifier:03X}: first frame replaced unfinished message")
            sessions[identifier] = [length, bytearray(data[2:]), 1, ms, ms]
        elif kind == 2:
            session = sessions.get(identifier)
            if session is None:
                problems.append(f"{ms} ms 0x{identifier:03X}: consecutive frame without first frame")
                continue
            if (data[0] & 15) != session[2] or len(data) < 2:
                problems.append(f"{ms} ms 0x{identifier:03X}: missing/out-of-order consecutive frame")
                del sessions[identifier]
                continue
            session[1].extend(data[1:])
            session[2] = (session[2] + 1) & 15
            session[4] = ms
            if len(session[1]) >= session[0]:
                messages.append((session[3], identifier, bytes(session[1][:session[0]])))
                del sessions[identifier]
        # Flow Control is intentionally not treated as application data.
    for identifier in sessions:
        problems.append(f"0x{identifier:03X}: recording ended before ISO-TP message completed")

    requests, responses = Counter(), Counter()
    with destination.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["ms", "can_id", "service_hex", "type", "did_hex", "payload_hex", "note"])
        for ms, identifier, payload in sorted(messages):
            service = payload[0]
            kind, did, note = "other", "", ""
            if service == 0x22 and len(payload) >= 3 and len(payload) % 2 == 1:
                dids = [int.from_bytes(payload[i:i + 2], "big") for i in range(1, len(payload), 2)]
                did = " ".join(f"{value:04X}" for value in dids)
                requests.update(dids)
                kind = "read_did_request"
                if len(dids) > 1:
                    note = "Multiple DIDs: response boundaries require each DID definition"
            elif service == 0x62 and len(payload) >= 3:
                value = int.from_bytes(payload[1:3], "big")
                responses[value] += 1
                did, kind = f"{value:04X}", "read_did_response"
                note = "First DID only; raw payload includes service and DID; no units inferred"
            elif service == 0x7F and len(payload) >= 3:
                kind = "negative_response"
                note = f"Rejected service {payload[1]:02X}; NRC {payload[2]:02X}"
            writer.writerow([ms, f"{identifier:03X}", f"{service:02X}", kind, did, payload.hex(" ").upper(), note])
    report = destination.with_suffix(".md")
    summary = ["# Passive CAN capture analysis", "", f"Source: {source.name}",
               f"Reassembled messages: {len(messages)}; detected problems: {len(problems)}", "",
               "Normal 11-bit ISO-TP supported on 7DF / 7E0–7EF. Other addressing is not decoded.",
               "DID names, units and regeneration bits are not inferred from values.", "", "## Capture metadata", *metadata,
               "", "## CAN identifiers", *[f"- {identifier:08X} extended={extended}: {count} frames" for (identifier, extended), count in sorted(ids.items())],
               "", "## DID requests / responses", *[f"- {did:04X}: requests={requests[did]}, responses={responses[did]}" for did in sorted(requests.keys() | responses.keys())],
               "", "## Reconstruction problems", *(problems or ["None detected within the supported addressing scope."])]
    report.write_text("\n".join(summary) + "\n", encoding="utf-8")
    print(f"Created {destination} and {report}; reconstruction problems: {len(problems)}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    analyze(args.capture, args.output or args.capture.with_name(args.capture.stem + "-uds.csv"))
