#!/usr/bin/env python3
from __future__ import annotations

import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def redmcsb_source_root() -> Path:
    candidates: list[Path] = []
    if os.environ.get("FIRESTAFF_REDMCSB_SOURCE"):
        candidates.append(Path(os.environ["FIRESTAFF_REDMCSB_SOURCE"]).expanduser())
    candidates.append(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
    for candidate in candidates:
        if (candidate / "GAMELOOP.C").exists() and (candidate / "COMMAND.C").exists():
            return candidate
    raise SystemExit("error: ReDMCSB source root not found; set FIRESTAFF_REDMCSB_SOURCE")


SOURCE = redmcsb_source_root()
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v2_launch_smoke_source_lock.json"

REQUIRED_SOURCE = [
    (SOURCE / "GAMELOOP.C", "G0321_B_StopWaitingForPlayerInput = C0_FALSE", 164),
    (SOURCE / "GAMELOOP.C", "F0380_COMMAND_ProcessQueue_CPSC", 215),
    (SOURCE / "GAMELOOP.C", "G0301_B_GameTimeTicking", 219),
    (SOURCE / "COMMAND.C", "void F0380_COMMAND_ProcessQueue_CPSC", 2045),
    (SOURCE / "COMMAND.C", "F0365_COMMAND_ProcessTypes1To2_TurnParty", 2151),
    (SOURCE / "COMMAND.C", "F0366_COMMAND_ProcessTypes3To6_MoveParty", 2155),
    (SOURCE / "DEFS.H", "C001_COMMAND_TURN_LEFT", 238),
    (SOURCE / "DEFS.H", "C006_COMMAND_MOVE_LEFT", 243),
]

REQUIRED_FIRESTAFF = [
    (ROOT / "src/ui/menu_startup_m12.c", "M12_StartupMenu_GetLaunchIntent"),
    (ROOT / "src/ui/menu_startup_m12.c", "M12_PRESENTATION_V22_MODERN"),
    (ROOT / "src/ui/menu_startup_m12.c", "intent.presentationMode = pmode"),
    (ROOT / "src/dm1v2/dm1_v2_runtime_pc34.c", "GAMELOOP.C:215"),
    (ROOT / "src/dm1v2/dm1_v2_runtime_pc34.c", "DUNGEON.C:1371-1391"),
    (ROOT / "src/dm1v2/dm1_v2_movement_command_adapter_pc34.c", "DEFS.H:238-243"),
    (ROOT / "src/dm1v2/dm1_v2_movement_command_adapter_pc34.c", "COMMAND.C:2045-2155"),
    (ROOT / "tests/test_dm1_v2_launch_smoke_pc34.c", "M12_PRESENTATION_V21_UPSCALED"),
    (ROOT / "tests/test_dm1_v2_launch_smoke_pc34.c", "GAMELOOP.C:164-219"),
    (ROOT / "tests/test_dm1_v2_launch_smoke_pc34.c", "COMMAND.C:2045-2155"),
    (ROOT / "tests/test_dm1_v2_launch_smoke_pc34.c", "DEFS.H:238-243"),
    (ROOT / "tests/test_dm1_v2_launch_smoke_pc34.c", "DM1_V2_MOVEMENT_ROUTE_V1_SOURCE"),
    (ROOT / "CMakeLists.txt", "NAME dm1_v2_launch_smoke_pc34"),
    (ROOT / "CMakeLists.txt", "NAME dm1_v2_launch_smoke_source_lock"),
]

errors: list[str] = []
anchors: list[dict[str, object]] = []
for path, needle, line in REQUIRED_SOURCE:
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
        errors.append(f"missing Firestaff launch-smoke anchor {needle} in {path.relative_to(ROOT)}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "DM1 V2 Phase-1 headless launch-smoke: M12 V2.1 launch intent plus one runtime input tick",
    "evidenceImpact": {
        "completionMatrixGap": "Phase 1 required a deterministic smoke path for launch V2, render first frame, process one input tick, and exit without making V1 the V2 presentation route.",
        "verifiedCompletionPercent": None,
    },
    "anchors": anchors,
    "errors": errors,
}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
if errors:
    for error in errors:
        print("error:", error)
    raise SystemExit(1)
print(f"dm1_v2_launch_smoke_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}")
