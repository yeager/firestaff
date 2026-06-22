#!/usr/bin/env python3
"""Pass1073: DM1 V1 original live-capture receipt lock.

This is a redacted evidence gate for the macOS DOSBox Staging live run that
reached DM1 PC 3.4 dungeon gameplay and proved keyboard movement with a
viewport-hash change.  It intentionally stores hashes, classifications, source
anchors, and non-claims only.  The proprietary original game frames remain
operator-local and are not checked into the repository.
"""
from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "pass1073_dm1_v1_original_live_capture_receipt"
MANIFEST = ROOT / "parity-evidence" / "verification" / PASS / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

STATUS = "PASS1073_DM1_V1_ORIGINAL_LIVE_CAPTURE_RECEIPT_LOCKED"
SCHEMA = "firestaff.parity.pass1073_dm1_v1_original_live_capture_receipt.v1"

HEX64 = re.compile(r"^[0-9a-f]{64}$")
FORBIDDEN_PATH_FRAGMENTS = ("/Users/", "/tmp/", "/private/", "/Applications/")


def fail(message: str) -> None:
    raise AssertionError(message)


def load_manifest() -> dict[str, Any]:
    if not MANIFEST.is_file():
        fail(f"missing manifest: {MANIFEST}")
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    if not isinstance(data, dict):
        fail("manifest is not a JSON object")
    return data


def walk_strings(value: Any) -> list[str]:
    if isinstance(value, str):
        return [value]
    if isinstance(value, dict):
        out: list[str] = []
        for item in value.values():
            out.extend(walk_strings(item))
        return out
    if isinstance(value, list):
        out: list[str] = []
        for item in value:
            out.extend(walk_strings(item))
        return out
    return []


def require_no_operator_paths(data: dict[str, Any]) -> None:
    for text in walk_strings(data):
        if any(fragment in text for fragment in FORBIDDEN_PATH_FRAGMENTS):
            fail(f"operator-local path leaked into redacted receipt: {text}")


def require_non_claims(non_claims: list[Any]) -> None:
    lowered = " ".join(str(claim).lower() for claim in non_claims)
    for phrase in (
        "not original-vs-firestaff pixel parity",
        "proprietary frames remain operator-local",
        "not an i34e debugger observation",
        "does not close any b1 row",
    ):
        if phrase not in lowered:
            fail(f"missing non-claim phrase: {phrase}")


def require_live_conf(pins: dict[str, Any]) -> None:
    expected = {
        "machine": "svga_s3",
        "memsize": "16",
        "core": "dynamic",
        "cycles": "max",
        "frameskip": "0",
        "output": "opengl",
        "mouse_capture": "onclick",
    }
    for key, value in expected.items():
        if str(pins.get(key)) != value:
            fail(f"live conf pin {key!r} expected {value!r}, got {pins.get(key)!r}")


def require_frame_rows(rows: list[Any]) -> None:
    if len(rows) != 2:
        fail(f"expected exactly 2 redacted frame rows, got {len(rows)}")
    expected = [
        ("original/01_ingame_start.png", "dungeon_gameplay"),
        ("original/02_ingame_step_forward.png", "dungeon_gameplay"),
    ]
    viewport_hashes: list[str] = []
    normalized_hashes: list[str] = []
    file_hashes: list[str] = []
    for row, (filename, classification) in zip(rows, expected):
        if not isinstance(row, dict):
            fail("frame row is not an object")
        if row.get("file") != filename:
            fail(f"frame filename drift: expected {filename}, got {row.get('file')}")
        if row.get("classification") != classification:
            fail(f"{filename}: expected classification {classification}")
        if row.get("width") != 320 or row.get("height") != 200:
            fail(f"{filename}: expected 320x200 frame geometry")
        for key in ("frameSha256", "viewportRgbSha256", "normalizedRgbSha256"):
            value = str(row.get(key, ""))
            if not HEX64.fullmatch(value):
                fail(f"{filename}: {key} is not a 64-char sha256")
        file_hashes.append(str(row["frameSha256"]))
        viewport_hashes.append(str(row["viewportRgbSha256"]))
        normalized_hashes.append(str(row["normalizedRgbSha256"]))
    if file_hashes[0] == file_hashes[1]:
        fail("full-frame sha did not change between start and step-forward")
    if viewport_hashes[0] == viewport_hashes[1]:
        fail("viewport RGB sha did not change after Keypad-5")
    if normalized_hashes[0] == normalized_hashes[1]:
        fail("normalized full-frame sha did not change after Keypad-5")


def require_actions(actions: list[Any]) -> None:
    if len(actions) != 2:
        fail(f"expected 2 action attempts, got {len(actions)}")
    by_key = {
        str(action.get("actionKey")): action
        for action in actions
        if isinstance(action, dict)
    }
    c070 = by_key.get("dungeon_move_forward_click")
    if not c070:
        fail("missing C070 mouse diagnostic action")
    if c070.get("viewportRgbChanged") is not False:
        fail("C070 mouse diagnostic must preserve no-change result")
    if "C070_ZONE_MOVE_FORWARD" not in str(c070.get("sourceAnchor", "")):
        fail("C070 action missing ReDMCSB C070 source anchor")
    keypad = by_key.get("Keypad-5")
    if not keypad:
        fail("missing Keypad-5 action")
    if keypad.get("viewportRgbChanged") is not True:
        fail("Keypad-5 must record a viewport hash change")
    if "COMMAND.C:275-281" not in str(keypad.get("sourceAnchor", "")):
        fail("Keypad-5 action missing ReDMCSB keyboard-table source anchor")


def require_receipt(data: dict[str, Any]) -> None:
    if data.get("schema") != SCHEMA:
        fail("schema mismatch")
    if data.get("status") != STATUS:
        fail("status mismatch")
    if data.get("gapRowMovement", {}).get("to") != "PARTIAL":
        fail("gap row must remain PARTIAL")
    require_no_operator_paths(data)
    require_non_claims(list(data.get("nonClaims", [])))
    live = data.get("liveRun", {})
    if not isinstance(live, dict):
        fail("liveRun is not an object")
    if live.get("engineState") != "dungeon_gameplay":
        fail("live run did not reach dungeon_gameplay")
    if live.get("captureBackend") != "peekaboo":
        fail("capture backend drift")
    if live.get("dosboxLaunchMode") != "staging_app_binary_dm_exe_path":
        fail("DOSBox launch mode drift")
    conf_sha = str(live.get("liveConfSha256", ""))
    if not HEX64.fullmatch(conf_sha):
        fail("live conf sha is not a 64-char sha256")
    runtime_files = live.get("runtimeRequiredFiles", {})
    if runtime_files != {"DUNGEON.DAT": True, "GRAPHICS.DAT": True}:
        fail("runtime required-file summary drift")
    require_live_conf(dict(live.get("liveConfPins", {})))
    require_frame_rows(list(live.get("frames", [])))
    require_actions(list(live.get("actionAttempts", [])))


def main() -> int:
    data = load_manifest()
    require_receipt(data)
    report_text = REPORT.read_text(encoding="utf-8") if REPORT.is_file() else ""
    if STATUS not in report_text:
        fail("report does not contain pass1073 status")
    print(f"{STATUS}: redacted live-capture receipt locked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
