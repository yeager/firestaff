#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
EVIDENCE = ROOT / "parity-evidence/verification/csb_v2_phase7_verification_source_lock.json"

REQUIRED_SOURCE = [
    (SOURCE / "GAMELOOP.C", "G0318_i_WaitForInputMaximumVerticalBlankCount = 12", (47, 50)),
    (SOURCE / "VBLANK.C", "G0317_i_WaitForInputVerticalBlankCount++", (60, 63)),
    (SOURCE / "PRIM1.C", "#define M526_WaitVerticalBlank()        Vsync()", (745, 745)),
    (SOURCE / "COMMAND.C", "F0380_COMMAND_ProcessQueue", (2045, 2156)),
    (SOURCE / "CLIKMENU.C", "F0365_COMMAND_ProcessTypes1To2_TurnParty", (142, 179)),
    (SOURCE / "CLIKMENU.C", "F0366_COMMAND_ProcessTypes3To6_MoveParty", (180, 390)),
    (SOURCE / "PANEL.C", "G0304_i_DungeonViewPaletteIndex", (367, 428)),
    (SOURCE / "DATA.C", "G0040_ai_Graphic562_PaletteIndexToLightAmount", (359, 360)),
]

REQUIRED_FIRESTAFF = [
    (ROOT / "tests/test_csb_v2_phase7_verification.c", "test_boot_viewport_clock"),
    (ROOT / "tests/test_csb_v2_phase7_verification.c", "test_smooth_movement_verification"),
    (ROOT / "tests/test_csb_v2_phase7_verification.c", "test_minimap_verification"),
    (ROOT / "tests/test_csb_v2_phase7_verification.c", "test_chaos_lighting_verification"),
    (ROOT / "src/csb/csb_v2_smooth_movement.c", "ReDMCSB COMMAND.C F0380"),
    (ROOT / "src/csb/csb_v2_lighting_dynamic.c", "PANEL.C:367-428"),
    (ROOT / "src/csb/csb_v2_viewport_renderer.c", "V2_AnimClock"),
]

errors = []
anchors = []

for path, needle, line_range in REQUIRED_SOURCE:
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    if needle not in text:
        errors.append(f"missing {needle} in {path.name}")
    start, end = line_range
    if not (1 <= start <= end <= len(lines)):
        errors.append(f"line range out of range {path.name}:{start}-{end}")
    else:
        anchors.append({
            "file": path.name,
            "lineRange": f"{start}-{end}",
            "needle": needle,
            "text": lines[start - 1].strip(),
        })

for path, needle in REQUIRED_FIRESTAFF:
    text = path.read_text(encoding="utf-8", errors="replace")
    if needle not in text:
        errors.append(f"missing Firestaff CSB V2 phase7 source-lock text {needle} in {path.name}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "csb_v2_phase7_verification_suite source-lock",
    "anchors": anchors,
    "errors": errors,
}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
if errors:
    for error in errors:
        print("error:", error)
    raise SystemExit(1)
print(f"csb_v2_phase7_verification_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}")
