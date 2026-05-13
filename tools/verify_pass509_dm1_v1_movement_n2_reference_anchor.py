#!/usr/bin/env python3
"""Pass509: DM1 V1 movement N2 reference-anchor source lock."""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass509_dm1_v1_movement_n2_reference_anchor"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

RED = Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")))
GREATSTONE = Path(os.environ.get("FIRESTAFF_GREATSTONE_ATLAS", str(Path.home() / ".openclaw/data/firestaff-greatstone-atlas")))
ORIGINALS = Path(os.environ.get("FIRESTAFF_DM_ORIGINALS", str(Path.home() / ".openclaw/data/firestaff-original-games/DM")))

SOURCE_RANGES = [
    {"id": "pc34-key-normalization", "file": "IO2.C", "lines": "27-61", "function": "F0540_INPUT_Crawcin", "claim": "PC-34 shifted cursor input is normalized into K/L/M/P command-table codes before command enqueue.", "needles": ["IODRV_00_GetKeyboardInput", "MEDIA707_I34E_I34M", "0x1248", "L2944_ui_ = 'L'", "L2944_ui_ = 'P'", "L2944_ui_ = 'K'", "L2944_ui_ = 'M'"]},
    {"id": "movement-input-tables", "file": "COMMAND.C", "lines": "106-121,636-685", "function": "G0448/G0459 movement input tables", "claim": "Mouse movement arrows and PC-34 keyboard rows map to C001/C002 turn and C003..C006 movement commands.", "needles": ["C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C006_COMMAND_MOVE_LEFT", "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT", "MEDIA707_I34E_I34M"]},
    {"id": "queue-gate-dispatch", "file": "COMMAND.C", "lines": "2045-2156", "function": "F0380_COMMAND_ProcessQueue_CPSC", "claim": "F0380 gates disabled movement before dequeue, then dispatches turn or move commands.", "needles": ["G0435_B_CommandQueueLocked = C1_TRUE", "G0310_i_DisabledMovementTicks", "G0311_i_ProjectileDisabledMovementTicks", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
    {"id": "turn-and-step-handlers", "file": "CLIKMENU.C", "lines": "142-347", "function": "F0365/F0366 turn and movement handlers", "claim": "Turn changes party direction through sensor leave/enter; step resolves deltas, blockers, F0267 movement, and cooldown timing.", "needles": ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection", "F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0357_COMMAND_DiscardAllInput", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks"]},
    {"id": "relative-delta", "file": "DUNGEON.C", "lines": "1371-1391", "function": "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "claim": "Relative stepping applies forward deltas, then a simulated-right-turn strafe delta.", "needles": ["F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "G0233_ai_Graphic559_DirectionToStepEastCount", "G0234_ai_Graphic559_DirectionToStepNorthCount", "Simulate turning right"]},
    {"id": "movement-result-sensors", "file": "MOVESENS.C", "lines": "738-818", "function": "F0267_MOVE_GetMoveResult_CPSCE", "claim": "Accepted party movement records the result tuple, scent/timing state, and source-before-destination sensor order.", "needles": ["G0397_i_MoveResultMapX", "G0398_i_MoveResultMapY", "G0399_ui_MoveResultMapIndex", "G0362_l_LastPartyMovementTime = G0313_ul_GameTime", "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX", "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX"]},
]
EXPECTED_HASHES = {
    "dm1/DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "dm1/GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "dm1/TITLE": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
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

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def verify_originals() -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    canon = ORIGINALS / "_canonical"
    readme = read_text(canon / "dm1/README.md")
    for rel, expected in EXPECTED_HASHES.items():
        path = canon / rel
        got = sha256(path)
        if got != expected:
            raise AssertionError(f"{path} sha256 {got} != {expected}")
        if str(path) not in readme:
            raise AssertionError(f"canonical README does not name {path}")
        rows.append({"path": str(path), "sha256": got})
    return rows

def verify_greatstone() -> dict[str, object]:
    diff = ORIGINALS / "_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json"
    data = json.loads(read_text(diff))
    summary = data["summary"]
    if summary["result"] != "PASS" or summary["total_mismatches"] != 0:
        raise AssertionError(f"Greatstone PC34 diff is not clean: {summary}")
    comparisons = data["comparisons"]
    if comparisons["pc34_graphics"]["inventory_sha256"] != EXPECTED_HASHES["dm1/GRAPHICS.DAT"]:
        raise AssertionError("Greatstone graphics hash does not match canonical GRAPHICS.DAT")
    if comparisons["pc34_dungeon"]["inventory_sha256"] != EXPECTED_HASHES["dm1/DUNGEON.DAT"]:
        raise AssertionError("Greatstone dungeon hash does not match canonical DUNGEON.DAT")
    if comparisons["pc34_graphics"]["greatstone_item_count"] != 713:
        raise AssertionError("unexpected Greatstone PC34 graphics item count")
    if comparisons["pc34_dungeon"]["greatstone_map_pages_fetched"] != 14:
        raise AssertionError("unexpected Greatstone PC34 dungeon map count")
    pages = json.loads(read_text(GREATSTONE / "index/pages.json"))
    urls = {row["url"] for row in pages}
    required_urls = {"http://greatstone.free.fr/dm/g_dm.html", "http://greatstone.free.fr/dm/index.html"}
    if not required_urls.issubset(urls):
        raise AssertionError("Greatstone page index missing DM overview pages")
    return {"diff": str(diff), "summary": summary, "pc34GraphicsUrl": comparisons["pc34_graphics"]["url"], "pc34DungeonUrl": comparisons["pc34_dungeon"]["url"], "indexedPages": sorted(required_urls)}

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
    lines = ["# Pass509 - DM1 V1 movement N2 reference anchor", "", f"Status: {manifest['status']}", "", "Scope: movement/forflyttning only. This binds the input->command->movement source-lock lane to N2-local ReDMCSB, Greatstone, and original DM1 PC34 anchors.", "", "## ReDMCSB source audit", ""]
    for row in manifest["redmcsbSourceAudit"]:  # type: ignore[index]
        lines.append(f"- PASS {row['file']}:{row['lines']} - {row['function']}: {row['claim']}")
    lines += ["", "## N2-local reference anchors", ""]
    for row in manifest["originalDm1Anchors"]:  # type: ignore[index]
        lines.append(f"- PASS {row['path']} sha256 {row['sha256']}")
    g = manifest["greatstoneAnchor"]  # type: ignore[index]
    lines += [f"- PASS {g['diff']} result {g['summary']['result']} with {g['summary']['total_mismatches']} mismatches.", f"- PASS Greatstone PC34 graphics URL: {g['pc34GraphicsUrl']}", f"- PASS Greatstone PC34 dungeon URL: {g['pc34DungeonUrl']}", "", "## Firestaff evidence consumed", ""]
    for row in manifest["firestaffEvidence"]:  # type: ignore[index]
        lines.append(f"- PASS {row['path']} - {row['claim']}")
    lines += ["", "## Not claimed", "", "- original DOS keyboard-buffer transcript", "- representative original movement/HUD/viewport overlay parity", "- viewport/wall or pass435 route promotion", ""]
    REPORT.write_text("\n".join(lines), encoding="utf-8")

def main() -> int:
    manifest: dict[str, object] = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": "PASS509_DM1_V1_MOVEMENT_N2_REFERENCE_ANCHORED",
        "scope": "DM1 V1 movement input->command->movement source-lock reference anchoring",
        "redmcsbRoot": str(RED),
        "greatstoneRoot": str(GREATSTONE),
        "originalsRoot": str(ORIGINALS),
        "redmcsbSourceAudit": verify_source(),
        "originalDm1Anchors": verify_originals(),
        "greatstoneAnchor": verify_greatstone(),
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
