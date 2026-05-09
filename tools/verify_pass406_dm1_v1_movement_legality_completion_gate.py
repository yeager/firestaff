#!/usr/bin/env python3
"""Pass406: DM1 V1 party movement target-square legality completion gate."""
from __future__ import annotations

from datetime import datetime, timezone
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass406_dm1_v1_movement_legality_completion_gate"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CLIKMENU = RED / "CLIKMENU.C"
MOVESENS = RED / "MOVESENS.C"
DUNGEON = RED / "DUNGEON.C"
FIRE_C = ROOT / "memory_movement_pc34_compat.c"
FIRE_TEST = ROOT / "test_dm1_v1_movement_core_pc34_compat.c"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def function_range(text: str, name: str, rettype: str = r"(?:void|BOOLEAN|int16_t|int|unsigned char)", next_name: str | None = None) -> tuple[int, int, str]:
    m = re.search(r"\b" + rettype + r"\s+" + re.escape(name) + r"\s*\(", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for pos in range(brace, len(text)):
        ch = text[pos]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return line_no(text, m.start()), line_no(text, pos), text[m.start():pos + 1]
    if next_name:
        n = re.search(r"^.*\b" + re.escape(next_name) + r"\s*\(", text[m.end():], re.M)
        if not n:
            raise AssertionError(f"unterminated function {name}; missing next anchor {next_name}")
        end = m.end() + n.start()
    else:
        end = len(text)
    return line_no(text, m.start()), line_no(text, end) - 1, text[m.start():end]


def require_order(text: str, needles: list[str], label: str) -> None:
    last = -1
    for needle in needles:
        pos = require(text, needle, label)
        if pos <= last:
            raise AssertionError(f"{label}: out of order {needle!r}")
        last = pos


def run(cmd: list[str], timeout: int = 60) -> str:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    if p.returncode != 0:
        raise AssertionError(f"command failed {chr(32).join(cmd)}:\n{p.stdout[-2000:]}")
    return p.stdout.strip()


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    clik = CLIKMENU.read_text(encoding="latin-1")
    moves = MOVESENS.read_text(encoding="latin-1")
    dung = DUNGEON.read_text(encoding="latin-1")
    fire = FIRE_C.read_text(encoding="utf-8")
    test = FIRE_TEST.read_text(encoding="utf-8")

    f0366_start, f0366_end, f0366 = function_range(clik, "F0366_COMMAND_ProcessTypes3To6_MoveParty", next_name="F0369_COMMAND_ProcessTypes101To108_ClickInSpellSymbolsArea_CPSE")
    require_order(f0366, [
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection",
        "L1116_i_SquareType = M034_SQUARE_TYPE(AL1115_ui_Square = F0151_DUNGEON_GetSquare",
        "if (L1116_i_SquareType == C03_ELEMENT_STAIRS)",
        "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
        "if (L1116_i_SquareType == C04_ELEMENT_DOOR)",
        "L1117_B_MovementBlocked = (L1117_B_MovementBlocked != C0_DOOR_STATE_OPEN) && (L1117_B_MovementBlocked != C1_DOOR_STATE_CLOSED_ONE_FOURTH) && (L1117_B_MovementBlocked != C5_DOOR_STATE_DESTROYED);",
        "if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)",
        "L1117_B_MovementBlocked = (!M007_GET(AL1115_ui_Square, MASK0x0004_FAKEWALL_OPEN) && !M007_GET(AL1115_ui_Square, MASK0x0001_FAKEWALL_IMAGINARY));",
        "if (L1117_B_MovementBlocked)",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
    ], "ReDMCSB CLIKMENU party target-square legality")

    f0267_start, f0267_end, f0267 = function_range(moves, "F0267_MOVE_GetMoveResult_CPSCE", rettype="BOOLEAN", next_name="F0268_SENSOR_AddEvent")
    pit_pos = require(f0267, "if ((AL0709_i_DestinationSquareType == C02_ELEMENT_PIT) && !L0713_B_ThingLevitates", "ReDMCSB MOVESENS pit gate")
    require_order(f0267[pit_pos:], [
        "if ((AL0709_i_DestinationSquareType == C02_ELEMENT_PIT) && !L0713_B_ThingLevitates",
        "F0154_DUNGEON_GetLocationAfterLevelChange",
        "F0173_DUNGEON_SetCurrentMap",
    ], "ReDMCSB MOVESENS pit consequence after accepted move")

    f0150_start, f0150_end, f0150 = function_range(dung, "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement")
    require(f0150, "G0233_ai_Graphic559_DirectionToStepEastCount", "ReDMCSB relative movement X table")
    f0151_start, f0151_end, f0151 = function_range(dung, "F0151_DUNGEON_GetSquare", rettype="unsigned char")
    require(f0151, "return M035_SQUARE(C00_ELEMENT_WALL, 0);", "ReDMCSB out-of-bounds wall")

    f0702_start, f0702_end, f0702 = function_range(fire, "F0702_MOVEMENT_TryMove_Compat", rettype="int")
    require_order(f0702, [
        "F0701_MOVEMENT_GetStepDelta_Compat(party->direction, moveAction, &dx, &dy);",
        "if (elementType == DUNGEON_ELEMENT_WALL)",
        "if (elementType == DUNGEON_ELEMENT_DOOR)",
        "if (doorState != 0 && doorState != 1 && doorState != 5)",
        "} else if (elementType == DUNGEON_ELEMENT_FAKEWALL)",
        "if (!(squareByte & 0x04) && !(squareByte & 0x01))",
        "outResult->resultCode = MOVE_OK;",
    ], "Firestaff party target-square legality")
    f0704_start, f0704_end, f0704 = function_range(fire, "F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat", rettype="int")
    require_order(f0704, [
        "if (elementType == DUNGEON_ELEMENT_PIT &&",
        "targetLevel = movement_get_location_after_level_change(",
        "outResolution->pitCount += 1;",
    ], "Firestaff post-move pit consequence")

    for needle in [
        "closed door state blocks forward",
        "one-fourth door passable",
        "destroyed door passable",
        "closed real fakewall blocks forward",
        "open fakewall passable",
        "imaginary fakewall passable",
        "pit square passable by movement dispatch",
    ]:
        require(test, needle, f"runtime movement-core case {needle}")

    source_lock_stdout = run([sys.executable, str(ROOT / "tools/verify_v1_movement_legality_source_lock.py")])
    movement_core_stdout = run([str(ROOT / "build/test_dm1_v1_movement_core_pc34_compat")])

    status = "PASS406_DM1_V1_MOVEMENT_LEGALITY_COMPLETION_GATE_PROVEN"
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "sourceAudit": {
            "CLIKMENU.F0366": f"CLIKMENU.C:{f0366_start}-{f0366_end}",
            "MOVESENS.F0267": f"MOVESENS.C:{f0267_start}-{f0267_end}",
            "DUNGEON.F0150": f"DUNGEON.C:{f0150_start}-{f0150_end}",
            "DUNGEON.F0151": f"DUNGEON.C:{f0151_start}-{f0151_end}",
        },
        "firestaffGuards": {
            "F0702_MOVEMENT_TryMove_Compat": f"memory_movement_pc34_compat.c:{f0702_start}-{f0702_end}",
            "F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat": f"memory_movement_pc34_compat.c:{f0704_start}-{f0704_end}",
            "runtimeExecutable": "build/test_dm1_v1_movement_core_pc34_compat",
        },
        "checks": [
            "tools/verify_v1_movement_legality_source_lock.py",
            "build/test_dm1_v1_movement_core_pc34_compat",
        ],
        "sourceLockOutputFirstLine": source_lock_stdout.splitlines()[0] if source_lock_stdout else "",
        "runtimeOutputFirstLine": movement_core_stdout.splitlines()[0] if movement_core_stdout else "",
        "notClaimed": [
            "representative original DOS movement overlay parity",
            "complete movement side-effect parity",
            "direct F0380 binary breakpoint proof",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    REPORT.write_text("\n".join([
        "# Pass406 — DM1 V1 movement legality completion gate",
        "",
        f"Status: `{status}`",
        "",
        "## ReDMCSB-first source audit",
        f"- `CLIKMENU.C:{f0366_start}-{f0366_end}` / `F0366_COMMAND_ProcessTypes3To6_MoveParty` — computes relative target square, blocks walls, closed-enough doors, and closed real fake-walls before accepted move dispatch.",
        f"- `MOVESENS.C:{f0267_start}-{f0267_end}` / `F0267_MOVE_GetMoveResult_CPSCE` — accepted-move consequence path, including open non-imaginary pit level changes.",
        f"- `DUNGEON.C:{f0150_start}-{f0150_end}` / `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` — source relative-step coordinate update.",
        f"- `DUNGEON.C:{f0151_start}-{f0151_end}` / `F0151_DUNGEON_GetSquare` — source square fetch, including out-of-bounds-as-wall behavior.",
        "",
        "## Firestaff executable guards",
        f"- `memory_movement_pc34_compat.c:{f0702_start}-{f0702_end}` / `F0702_MOVEMENT_TryMove_Compat` keeps the pre-step legality split.",
        f"- `memory_movement_pc34_compat.c:{f0704_start}-{f0704_end}` / `F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat` keeps pit consequences post-step.",
        "- `build/test_dm1_v1_movement_core_pc34_compat` covers closed/open/destroyed door states, fake-wall open/imaginary bits, wall blocks, and pit passability.",
        "",
        "## Verdict",
        "- Closes one completion-matrix movement source-lock gap: target-square legality is now an executable CTest gate, not just prose in the movement row.",
        "- Scope guard: no representative original DOS overlay parity, complete movement side-effect parity, or direct-F0380 binary breakpoint proof is claimed.",
        "",
        f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
    ]) + "\n")

    print(f"PASS {PASS}")
    print(f"- ReDMCSB: CLIKMENU.C:{f0366_start}-{f0366_end}; MOVESENS.C:{f0267_start}-{f0267_end}; DUNGEON.C:{f0150_start}-{f0150_end}, {f0151_start}-{f0151_end}")
    print(f"- Firestaff: memory_movement_pc34_compat.c:{f0702_start}-{f0702_end}, {f0704_start}-{f0704_end}")
    print(f"- report: {REPORT.relative_to(ROOT)}")
    print(f"- manifest: {MANIFEST.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, subprocess.SubprocessError) as exc:
        print(f"FAIL {PASS}: {exc}", file=sys.stderr)
        raise SystemExit(1)
