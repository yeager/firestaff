#!/usr/bin/env python3
"""Pass509: DM1 V1 movement N2 reference-anchor source lock."""
from __future__ import annotations

import hashlib
import json
import os
import struct
import zlib
import zipfile
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass509_dm1_v1_movement_n2_reference_anchor"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

RED = ROOT / "reference/redmcsb-20210206/Toolchains/Common/Source"
DM1_ZIP = Path(os.environ.get(
    "FIRESTAFF_DM1_PC34_ZIP",
    str(Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip")))
DM1_ZIP_DISPLAY = "~/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"

SOURCE_RANGES = [
    {"id": "pc34-key-normalization", "file": "IO2.C", "lines": "27-61", "function": "F0540_INPUT_Crawcin", "claim": "PC-34 shifted cursor input is normalized into K/L/M/P command-table codes before command enqueue.", "needles": ["IODRV_00_GetKeyboardInput", "MEDIA707_I34E_I34M", "0x1248", "L2944_ui_ = 'L'", "L2944_ui_ = 'P'", "L2944_ui_ = 'K'", "L2944_ui_ = 'M'"]},
    {"id": "movement-input-tables", "file": "COMMAND.C", "lines": "106-121,636-685", "function": "G0448/G0459 movement input tables", "claim": "Mouse movement arrows and PC-34 keyboard rows map to C001/C002 turn and C003..C006 movement commands.", "needles": ["C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C006_COMMAND_MOVE_LEFT", "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT", "MEDIA707_I34E_I34M"]},
    {"id": "queue-gate-dispatch", "file": "COMMAND.C", "lines": "2045-2156", "function": "F0380_COMMAND_ProcessQueue_CPSC", "claim": "F0380 gates disabled movement before dequeue, then dispatches turn or move commands.", "needles": ["G0435_B_CommandQueueLocked = C1_TRUE", "G0310_i_DisabledMovementTicks", "G0311_i_ProjectileDisabledMovementTicks", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
    {"id": "turn-and-step-handlers", "file": "CLIKMENU.C", "lines": "142-347", "function": "F0365/F0366 turn and movement handlers", "claim": "Turn changes party direction through sensor leave/enter; step resolves deltas, blockers, F0267 movement, and cooldown timing.", "needles": ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection", "F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0357_COMMAND_DiscardAllInput", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks"]},
    {"id": "relative-delta", "file": "DUNGEON.C", "lines": "1371-1391", "function": "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "claim": "Relative stepping applies forward deltas, then a simulated-right-turn strafe delta.", "needles": ["F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "G0233_ai_Graphic559_DirectionToStepEastCount", "G0234_ai_Graphic559_DirectionToStepNorthCount", "Simulate turning right"]},
    {"id": "movement-result-sensors", "file": "MOVESENS.C", "lines": "738-818", "function": "F0267_MOVE_GetMoveResult_CPSCE", "claim": "Accepted party movement records the result tuple, scent/timing state, and source-before-destination sensor order.", "needles": ["G0397_i_MoveResultMapX", "G0398_i_MoveResultMapY", "G0399_ui_MoveResultMapIndex", "G0362_l_LastPartyMovementTime = G0313_ul_GameTime", "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX", "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX"]},
]
EXPECTED_HASHES = {
    "DATA/DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "DATA/GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "TITLE": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
}

def compact(text: str) -> str:
    return " ".join(text.split())

def read_text(path: Path, encoding: str = "utf-8") -> str:
    if not path.is_file():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")

def source_block(file_name: str, ranges: str) -> str:
    lines = read_text(RED / file_name, "latin-1").splitlines()
    chunks: list[str] = []
    for span in ranges.split(","):
        lo_s, hi_s = span.split("-", 1)
        lo, hi = int(lo_s), int(hi_s)
        if lo < 1 or hi > len(lines):
            raise AssertionError(f"{file_name}:{span} outside file length {len(lines)}")
        chunks.extend(lines[lo - 1:hi])
    return "\n".join(chunks)

def verify_source() -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for item in SOURCE_RANGES:
        block = compact(source_block(item["file"], item["lines"]))
        missing = [needle for needle in item["needles"] if compact(needle) not in block]
        if missing:
            raise AssertionError(f"{item['file']}:{item['lines']} missing {missing!r}")
        rows.append({key: item[key] for key in ("id", "file", "lines", "function", "claim")})
    return rows

def zip_member_bytes(raw: bytes, info: zipfile.ZipInfo) -> bytes:
    fields = struct.unpack(
        "<IHHHHHIIIHH", raw[info.header_offset:info.header_offset + 30])
    if fields[0] != 0x04034B50:
        raise AssertionError(f"{info.filename}: invalid local ZIP header")
    start = info.header_offset + 30 + fields[9] + fields[10]
    compressed = raw[start:start + info.compress_size]
    if info.compress_type == zipfile.ZIP_STORED:
        payload = compressed
    elif info.compress_type == zipfile.ZIP_DEFLATED:
        payload = zlib.decompress(compressed, -15)
    else:
        raise AssertionError(
            f"{info.filename}: unsupported ZIP method {info.compress_type}")
    if len(payload) != info.file_size:
        raise AssertionError(f"{info.filename}: decoded size mismatch")
    return payload

def verify_originals() -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    if not DM1_ZIP.is_file():
        raise AssertionError(f"missing authentic PC 3.4 ZIP: {DM1_ZIP}")
    raw = DM1_ZIP.read_bytes()
    with zipfile.ZipFile(DM1_ZIP) as archive:
        infos = {info.filename.replace("\\", "/"): info
                 for info in archive.infolist()}
        for member, expected in EXPECTED_HASHES.items():
            if member not in infos:
                raise AssertionError(f"authentic archive missing {member}")
            payload = zip_member_bytes(raw, infos[member])
            got = hashlib.sha256(payload).hexdigest()
            if got != expected:
                raise AssertionError(f"{member} sha256 {got} != {expected}")
            rows.append({"path": f"{DM1_ZIP_DISPLAY}::{member}",
                         "sha256": got})
    return rows

def verify_firestaff() -> list[dict[str, str]]:
    checks = [
        ("tools/verify_pass423_dm1_v1_input_command_movement_pipeline_source_lock.py", ["SOURCE_RANGES", "ORDER_CHECKS", "FIRESTAFF_EVIDENCE"]),
        ("tools/verify_pass507_dm1_v1_movement_stairs_group_timing_source_lock.py", ["src_rows", "firestaff_rows", "static_gates"]),
        ("parity-evidence/pass507_dm1_v1_movement_stairs_group_timing_source_lock.md", ["PASS507_DM1_V1_MOVEMENT_STAIRS_GROUP_TIMING_SOURCE_LOCKED", "COMMAND.C:2075-2155", "MOVESENS.C:738-779"]),
    ]
    rows: list[dict[str, str]] = []
    for rel, needles in checks:
        text = read_text(ROOT / rel)
        for needle in needles:
            if needle not in text:
                raise AssertionError(f"{rel} missing {needle!r}")
        rows.append({"path": rel, "claim": "existing movement gate/evidence remains present"})
    return rows

def write_report(manifest: dict[str, object]) -> None:
    lines = ["# Pass509 - DM1 V1 movement reference anchor", "", f"Status: {manifest['status']}", "", "Scope: DM1 V1 movement only. This binds the input-to-command-to-movement lane to repository ReDMCSB and authentic PC 3.4 ZIP members.", "", "## ReDMCSB source audit", ""]
    for row in manifest["redmcsbSourceAudit"]:  # type: ignore[index]
        lines.append(f"- PASS {row['file']}:{row['lines']} - {row['function']}: {row['claim']}")
    lines += ["", "## Authentic PC 3.4 ZIP anchors", ""]
    for row in manifest["originalDm1Anchors"]:  # type: ignore[index]
        lines.append(f"- PASS {row['path']} sha256 {row['sha256']}")
    lines += ["", "## Firestaff evidence consumed", ""]
    for row in manifest["firestaffEvidence"]:  # type: ignore[index]
        lines.append(f"- PASS {row['path']} - {row['claim']}")
    lines += ["", "## Not claimed", "", "- original DOS keyboard-buffer transcript", "- representative original movement/HUD/viewport overlay parity", "- viewport/wall or pass435 route promotion", ""]
    REPORT.write_text("\n".join(lines), encoding="utf-8")

def main() -> int:
    manifest: dict[str, object] = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": "PASS509_DM1_V1_MOVEMENT_N2_REFERENCE_ANCHORED",
        "scope": "DM1 V1 movement input->command->movement source-lock reference anchoring",
        "redmcsbRoot": str(RED.relative_to(ROOT)),
        "originalArchive": DM1_ZIP_DISPLAY,
        "redmcsbSourceAudit": verify_source(),
        "originalDm1Anchors": verify_originals(),
        "firestaffEvidence": verify_firestaff(),
        "notClaimed": ["original DOS keyboard-buffer transcript", "representative original movement/HUD/viewport overlay parity", "viewport/wall or pass435 route promotion"],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": manifest["status"], "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
