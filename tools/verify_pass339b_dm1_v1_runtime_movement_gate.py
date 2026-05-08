#!/usr/bin/env python3
"""Verify pass339b DM1 V1 runtime movement gate evidence."""
from pathlib import Path
import json
import sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
EVIDENCE = ROOT / "parity-evidence" / "pass339b_dm1_v1_runtime_movement_gate.md"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / "pass339b_dm1_v1_runtime_movement_gate"
MANIFEST = VERIFY_DIR / "manifest.json"
RUNTIME_JSON = VERIFY_DIR / "runtime_hall_walkaround_route.json"
RUNTIME_MD = VERIFY_DIR / "runtime_hall_walkaround_route.md"
RUNTIME_LOG = VERIFY_DIR / "runtime_probe_command.log"
PIPELINE_LOG = VERIFY_DIR / "pipeline_test.log"

REQUIRED_FILES = [
    EVIDENCE, MANIFEST, RUNTIME_JSON, RUNTIME_MD, RUNTIME_LOG, PIPELINE_LOG,
    ROOT / "main_loop_m11.c", ROOT / "m11_game_view.c",
    ROOT / "probes" / "m11" / "firestaff_m11_hall_walkaround_runtime_probe.c",
    RED / "COMMAND.C", RED / "CLIKMENU.C", RED / "MOVESENS.C", RED / "INPUT.C", RED / "IO2.C",
]

EVIDENCE_NEEDLES = [
    "Status: **MOVEMENT_PROVED**", "COMMAND.C:106-121", "COMMAND.C:678-683",
    "COMMAND.C:1709-1813", "COMMAND.C:2029-2042", "COMMAND.C:2045-2158",
    "CLIKMENU.C:223-233", "CLIKMENU.C:264-328", "MOVESENS.C:315-545",
    "INPUT.C:531-568", "IO2.C:47-59",
    "route-token-equivalent `left,up,left,up,left,up,right,right`",
    "BLOCKED_FULL_LAUNCHER_SCRIPT_HANDOFF",
]

RANGE_MARKERS = [
    (RED / "COMMAND.C", 106, 121, "G0448_as_Graphic561_SecondaryMouseInput_Movement"),
    (RED / "COMMAND.C", 678, 683, "{ C003_COMMAND_MOVE_FORWARD,  0x004C }"),
    (RED / "COMMAND.C", 1709, 1813, "void F0361_COMMAND_ProcessKeyPress"),
    (RED / "COMMAND.C", 2029, 2042, "void F1053_Pre_F0380_COMMAND_ProcessQueue_CPSC"),
    (RED / "COMMAND.C", 2045, 2158, "void F0380_COMMAND_ProcessQueue_CPSC"),
    (RED / "COMMAND.C", 2150, 2156, "F0366_COMMAND_ProcessTypes3To6_MoveParty"),
    (RED / "CLIKMENU.C", 223, 233, "G0465_ai_Graphic561_MovementArrowToStepForwardCount"),
    (RED / "CLIKMENU.C", 264, 328, "F0267_MOVE_GetMoveResult_CPSCE"),
    (RED / "MOVESENS.C", 315, 545, "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE"),
    (RED / "MOVESENS.C", 441, 443, "G0306_i_PartyMapX = P0560_i_DestinationMapX"),
    (RED / "INPUT.C", 531, 568, "F1097_StoreKeyInBuffer"),
    (RED / "IO2.C", 47, 59, "L2944_ui_ = 'L'"),
]

FIRESTAFF_NEEDLES = {
    "main_loop_m11.c": [
        'strncmp(token, "up", len) == 0', "return M12_MENU_INPUT_UP;",
        'strncmp(token, "left", len) == 0', "return M12_MENU_INPUT_LEFT;",
        'strncmp(token, "right", len) == 0', "return M12_MENU_INPUT_RIGHT;",
    ],
    "m11_game_view.c": [
        "case M12_MENU_INPUT_UP:", 'label = "FORWARD";', "case M12_MENU_INPUT_LEFT:",
        "command = CMD_TURN_LEFT;", "case M12_MENU_INPUT_RIGHT:", "command = CMD_TURN_RIGHT;",
        "return M11_GAME_INPUT_REDRAW;",
    ],
    "probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c": [
        "M11_GameView_OpenSelectedMenuEntry(game, menu)",
        "M11_GameView_HandleInput(&game, M12_MENU_INPUT_LEFT)",
        "M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP)",
        "M11_GameView_HandleInput(&game, M12_MENU_INPUT_RIGHT)",
        "west step moves x", "redraw/refresh",
    ],
}


def read(path: Path) -> str:
    enc = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=enc)


def window(path: Path, start: int, end: int) -> str:
    lines = read(path).splitlines()
    if end > len(lines):
        raise AssertionError(f"{path.name}:{start}-{end} exceeds file length {len(lines)}")
    return "\n".join(lines[start - 1:end])


def main() -> int:
    missing = [str(p) for p in REQUIRED_FILES if not p.exists()]
    if missing:
        raise AssertionError("missing required files: " + ", ".join(missing))

    ev = read(EVIDENCE)
    for needle in EVIDENCE_NEEDLES:
        if needle not in ev:
            raise AssertionError(f"evidence missing {needle!r}")

    for path, start, end, marker in RANGE_MARKERS:
        if marker not in window(path, start, end):
            raise AssertionError(f"{path.name}:{start}-{end} missing {marker!r}")

    for rel, needles in FIRESTAFF_NEEDLES.items():
        text = read(ROOT / rel)
        for needle in needles:
            if needle not in text:
                raise AssertionError(f"{rel} missing {needle!r}")

    manifest = json.loads(read(MANIFEST))
    if manifest.get("status") != "MOVEMENT_PROVED":
        raise AssertionError("manifest status is not MOVEMENT_PROVED")
    if manifest.get("fullLauncherStatus") != "BLOCKED_FULL_LAUNCHER_SCRIPT_HANDOFF":
        raise AssertionError("manifest missing launcher blocker")
    if manifest.get("routeTokens") != ["up", "left", "right"]:
        raise AssertionError("manifest routeTokens must be exactly up,left,right")

    runtime = json.loads(read(RUNTIME_JSON))
    if runtime.get("status") != "PASS":
        raise AssertionError("runtime probe did not PASS")
    steps = {step["name"]: step for step in runtime.get("steps", [])}
    for name in ["turn_left_east_view_changes", "step_west_moves_in_hall", "turn_right_east_front_second_mirror"]:
        if name not in steps:
            raise AssertionError(f"runtime missing step {name}")
        if steps[name].get("result") != 1:
            raise AssertionError(f"runtime step {name} did not request redraw")
    if steps["turn_left_east_view_changes"].get("lastOutcome") != "FACING UPDATED":
        raise AssertionError("turn did not update facing")
    moved = steps["step_west_moves_in_hall"]
    if (moved.get("mapX"), moved.get("mapY"), moved.get("lastOutcome")) != (0, 3, "PARTY MOVED"):
        raise AssertionError("west step movement proof changed")
    for step in runtime.get("steps", []):
        if step.get("championCount") != 0 or step.get("candidateMirrorPanelActive") != 0:
            raise AssertionError("movement route opened champion/candidate state")

    if "PASS dm1 v1 hall walkaround runtime probe" not in read(RUNTIME_LOG):
        raise AssertionError("runtime command log missing PASS")
    if "131 passed, 0 failed" not in read(PIPELINE_LOG):
        raise AssertionError("pipeline test log missing 131 passed, 0 failed")

    print("pass339b_dm1_v1_runtime_movement_gate=pass status=MOVEMENT_PROVED full_launcher=BLOCKED_FULL_LAUNCHER_SCRIPT_HANDOFF route_tokens=up,left,right")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"pass339b_dm1_v1_runtime_movement_gate=fail {exc}", file=sys.stderr)
        raise SystemExit(1)
