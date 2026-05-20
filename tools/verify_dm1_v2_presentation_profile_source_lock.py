#!/usr/bin/env python3
from __future__ import annotations

import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def redmcsb_source_root() -> Path:
    candidates = []
    if os.environ.get("FIRESTAFF_REDMCSB_SOURCE"):
        candidates.append(Path(os.environ["FIRESTAFF_REDMCSB_SOURCE"]).expanduser())
    candidates.extend([
        Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
        Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser(),
    ])
    for candidate in candidates:
        if (candidate / "GAMELOOP.C").exists() and (candidate / "DUNVIEW.C").exists():
            return candidate
    raise SystemExit("error: ReDMCSB source root not found; set FIRESTAFF_REDMCSB_SOURCE")


SOURCE = redmcsb_source_root()
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v2_presentation_profile_source_lock.json"

REQUIRED_SOURCE = [
    (SOURCE / "GAMELOOP.C", "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)", 90),
    (SOURCE / "GAMELOOP.C", "G0321_B_StopWaitingForPlayerInput", 164),
    (SOURCE / "GAMELOOP.C", "F0380_COMMAND_ProcessQueue_CPSC", 215),
    (SOURCE / "GAMELOOP.C", "G0301_B_GameTimeTicking", 219),
    (SOURCE / "DUNVIEW.C", "M100_PIXEL_WIDTH(G0296_puc_Bitmap_Viewport) = G2073_C224_ViewportPixelWidth", 2999),
    (SOURCE / "DUNVIEW.C", "M101_PIXEL_HEIGHT(G0296_puc_Bitmap_Viewport) = G2074_C136_ViewportHeight", 3000),
    (SOURCE / "DUNVIEW.C", "F0128_DUNGEONVIEW_Draw_CPSF", 8318),
    (SOURCE / "DEFS.H", "C001_COMMAND_TURN_LEFT", 238),
    (SOURCE / "DEFS.H", "C006_COMMAND_MOVE_LEFT", 243),
    (SOURCE / "COMMAND.C", "void F0380_COMMAND_ProcessQueue_CPSC", 2045),
    (SOURCE / "COMMAND.C", "F0365_COMMAND_ProcessTypes1To2_TurnParty", 2151),
    (SOURCE / "COMMAND.C", "F0366_COMMAND_ProcessTypes3To6_MoveParty", 2155),
]

REQUIRED_FIRESTAFF = [
    (ROOT / "include/dm1_v2_presentation_profile_pc34.h", "DM1_V2_GAMEPLAY_ROUTE_V1_SOURCE"),
    (ROOT / "include/dm1_v2_presentation_profile_pc34.h", "DM1_V2_PresentationProfile"),
    (ROOT / "src/dm1v2/dm1_v2_presentation_profile_pc34.c", "GAMELOOP.C:90"),
    (ROOT / "src/dm1v2/dm1_v2_presentation_profile_pc34.c", "GAMELOOP.C:164-219"),
    (ROOT / "src/dm1v2/dm1_v2_presentation_profile_pc34.c", "DUNVIEW.C:2999-3000"),
    (ROOT / "src/dm1v2/dm1_v2_presentation_profile_pc34.c", "DUNVIEW.C:8318-8338"),
    (ROOT / "src/dm1v2/dm1_v2_presentation_profile_pc34.c", "COMMAND.C:2045-2155"),
    (ROOT / "src/dm1v2/dm1_v2_presentation_profile_pc34.c", "gameplayRoute = DM1_V2_GAMEPLAY_ROUTE_V1_SOURCE"),
    (ROOT / "tests/test_dm1_v2_presentation_profile_pc34.c", "Phase 1 defaults must keep V1/off as the boot path"),
    (ROOT / "tests/test_dm1_v2_presentation_profile_pc34.c", "V2/on affects presentation/config only"),
    (ROOT / "tests/test_dm1_v2_presentation_profile_pc34.c", "memcmp(&profile.snapshot, &before"),
    (ROOT / "CMakeLists.txt", "NAME dm1_v2_presentation_profile_pc34"),
    (ROOT / "CMakeLists.txt", "NAME dm1_v2_presentation_profile_source_lock"),
]

errors: list[str] = []
anchors = []
for path, needle, line in REQUIRED_SOURCE:
    if not path.exists():
        errors.append(f"missing ReDMCSB source file {path}")
        continue
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    if needle not in text:
        errors.append(f"missing source needle {needle} in {path.name}")
    if not (1 <= line <= len(lines)):
        errors.append(f"line out of range {path.name}:{line}")
    else:
        line_text = lines[line - 1].strip()
        if needle not in line_text:
            errors.append(f"line anchor mismatch {path.name}:{line}: expected {needle!r}, got {line_text!r}")
        anchors.append({"file": path.name, "line": line, "needle": needle, "text": line_text})

for path, needle in REQUIRED_FIRESTAFF:
    text = path.read_text(encoding="utf-8", errors="replace")
    if needle not in text:
        errors.append(f"missing Firestaff presentation profile source-lock text {needle} in {path.name}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "dm1_v2_presentation_profile_pc34 Phase-1 V1 gameplay/V2 presentation boundary",
    "anchors": anchors,
    "errors": errors,
}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
if errors:
    for error in errors:
        print("error:", error)
    raise SystemExit(1)
print(f"dm1_v2_presentation_profile_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}")
