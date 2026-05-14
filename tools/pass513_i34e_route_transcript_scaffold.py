#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


KEY_ROWS: dict[str, dict[str, str]] = {
    "kp4": {"normalizedKeyCode": "0x004B", "ascii": "K", "command": "C001_COMMAND_TURN_LEFT", "handler": "F0365", "kind": "turn"},
    "left": {"normalizedKeyCode": "0x004B", "ascii": "K", "command": "C001_COMMAND_TURN_LEFT", "handler": "F0365", "kind": "turn"},
    "kp8": {"normalizedKeyCode": "0x004C", "ascii": "L", "command": "C003_COMMAND_MOVE_FORWARD", "handler": "F0366", "kind": "step"},
    "up": {"normalizedKeyCode": "0x004C", "ascii": "L", "command": "C003_COMMAND_MOVE_FORWARD", "handler": "F0366", "kind": "step"},
    "kp6": {"normalizedKeyCode": "0x004D", "ascii": "M", "command": "C002_COMMAND_TURN_RIGHT", "handler": "F0365", "kind": "turn"},
    "right": {"normalizedKeyCode": "0x004D", "ascii": "M", "command": "C002_COMMAND_TURN_RIGHT", "handler": "F0365", "kind": "turn"},
    "kp1": {"normalizedKeyCode": "0x004F", "ascii": "O", "command": "C006_COMMAND_MOVE_LEFT", "handler": "F0366", "kind": "step"},
    "kp2": {"normalizedKeyCode": "0x0050", "ascii": "P", "command": "C005_COMMAND_MOVE_BACKWARD", "handler": "F0366", "kind": "step"},
    "down": {"normalizedKeyCode": "0x0050", "ascii": "P", "command": "C005_COMMAND_MOVE_BACKWARD", "handler": "F0366", "kind": "step"},
    "kp3": {"normalizedKeyCode": "0x0051", "ascii": "Q", "command": "C004_COMMAND_MOVE_RIGHT", "handler": "F0366", "kind": "step"},
}

REQUIRED_PASS513_RUNTIME_FIELDS = [
    "m527WasNonEmpty", "f0361QueueSlot", "g0434Before", "g0434After",
    "g2153BeforeEnqueue", "g2153AfterEnqueue", "g0433Before", "g0433After",
    "g2153BeforePop", "g2153AfterPop", "partyBeforeMap", "partyBeforeX",
    "partyBeforeY", "partyBeforeDir", "partyAfterMap", "partyAfterX",
    "partyAfterY", "partyAfterDir", "blockedOrNoopReason", "f0128Direction",
    "f0128MapX", "f0128MapY", "f0097Presented",
]


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def read_tsv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="") as handle:
        return list(csv.DictReader(handle, delimiter="\t"))


def is_shot(token: str) -> bool:
    low = token.lower()
    return low in {"shot", "capture", "screenshot"} or low.startswith("shot:")


def shot_label(token: str, fallback: str) -> str:
    low = token.lower()
    return low.split(":", 1)[1] if low.startswith("shot:") else fallback


def relpath(path: Path, repo_root: Path) -> str:
    try:
        return str(path.relative_to(repo_root))
    except ValueError:
        return str(path)


def build_rows(route_events: str, raw_manifest: Path, crop_manifest: Path, shot_labels: Path, repo_root: Path) -> list[dict[str, Any]]:
    raw_rows = read_tsv(raw_manifest)
    crop_rows = read_tsv(crop_manifest)
    label_rows = read_tsv(shot_labels)
    rows: list[dict[str, Any]] = []
    pending_key: tuple[int, str, dict[str, str]] | None = None
    shot_index = 0

    for token_index, token in enumerate(token.strip() for token in route_events.split() if token.strip()):
        low = token.lower()
        if low in KEY_ROWS:
            pending_key = (token_index, token, KEY_ROWS[low])
            continue
        if not is_shot(token):
            continue
        if shot_index >= len(raw_rows) or shot_index >= len(crop_rows):
            raise SystemExit("ERROR: route contains more shot tokens than manifest rows")
        label = shot_label(token, label_rows[shot_index].get("route_label") or f"shot_{shot_index + 1:02d}")
        if pending_key is None:
            shot_index += 1
            continue
        key_index, key_token, key_row = pending_key
        raw_path = Path(raw_rows[shot_index]["path"])
        if not raw_path.is_absolute():
            raw_path = repo_root / raw_path
        crop_path = crop_manifest.parent / "viewport_224x136" / crop_rows[shot_index]["filename"]
        capture_path = crop_path if crop_path.exists() else raw_path
        row: dict[str, Any] = {
            "routeId": "dm1_v1_pc34_i34e_original_dosbox_route",
            "sampleIndex": len(rows) + 1,
            "routeTokenIndex": key_index,
            "inputSource": "dosbox-route-token",
            "routeKeyToken": key_token,
            "routeShotToken": token,
            "routeShotLabel": label,
            "rawKeyCode": key_row["normalizedKeyCode"],
            "normalizedKeyCode": key_row["normalizedKeyCode"],
            "m528Value": key_row["normalizedKeyCode"],
            "f0361Table": "G0459_as_Graphic561_SecondaryKeyboardInput_Movement",
            "f0361Command": key_row["command"],
            "f0380Command": key_row["command"],
            "dispatchHandler": key_row["handler"],
            "capturePath": relpath(capture_path, repo_root),
            "captureSha256": sha256_file(capture_path),
            "rawCapturePath": relpath(raw_path, repo_root),
            "rawCaptureSha256": raw_rows[shot_index]["sha256"],
            "scaffoldOnly": True,
            "missingOriginalRuntimeFields": list(REQUIRED_PASS513_RUNTIME_FIELDS),
        }
        for field in REQUIRED_PASS513_RUNTIME_FIELDS:
            row[field] = None
        rows.append(row)
        pending_key = None
        shot_index += 1
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description="Build a pass513 transcript scaffold from a DOSBox route capture.")
    parser.add_argument("--route-events", required=True)
    parser.add_argument("--raw-manifest", required=True, type=Path)
    parser.add_argument("--crop-manifest", required=True, type=Path)
    parser.add_argument("--shot-labels", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    args = parser.parse_args()

    rows = build_rows(args.route_events, args.raw_manifest, args.crop_manifest, args.shot_labels, args.repo_root.resolve())
    payload = {
        "schema": "pass513_i34e_route_transcript_scaffold.v1",
        "status": "SCAFFOLD_ONLY_MISSING_ORIGINAL_RUNTIME_DEBUG_FIELDS",
        "createdUtc": datetime.now(timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z"),
        "routeEvents": args.route_events,
        "sourceLocks": [
            "IO2.C:27-61 F0540_INPUT_Crawcin normalizes PC/I34E extended arrows to K/L/M/O/P/Q.",
            "COMMAND.C:636-685 G0459_as_Graphic561_SecondaryKeyboardInput_Movement maps K/L/M/O/P/Q to C001/C003/C002/C006/C005/C004.",
            "COMMAND.C:1734-1812 F0361_COMMAND_ProcessKeyPress owns enqueue/last-index/count evidence.",
            "COMMAND.C:2075-2127,2150-2156 F0380_COMMAND_ProcessQueue_CPSC owns pop/count/dispatch evidence.",
            "CLIKMENU.C:142-174,237-270,293-347 F0365/F0366 own turn/move dispatch side effects.",
            "DUNVIEW.C:8318-8611 and DRAWVIEW.C:709-858 own post-command viewport present evidence.",
        ],
        "promotionRule": "Do not pass this file to FIRESTAFF_PASS513_TRANSCRIPT until every missingOriginalRuntimeFields entry is replaced with debugger-observed original PC/I34E values.",
        "rows": rows,
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
    print(json.dumps({"out": str(args.out), "rows": len(rows), "status": payload["status"]}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
