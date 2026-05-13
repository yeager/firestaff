#!/usr/bin/env python3
"""Pass505: source-lock blocked movement side effects in DM1 V1 F0366.

This is intentionally narrow. Pass406 already locks target-square legality;
this gate locks the ordering and side effects of a blocked cardinal step:
living-champion stamina is spent before legality, wall/door/fake-wall blocks
request self damage, queued input is discarded, the PC-34 VBlank wait is
requested, and the successful-move wait/redraw path is not taken.
"""
from __future__ import annotations

from datetime import datetime, timezone
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass505_dm1_v1_blocked_movement_side_effect_source_lock"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CLIKMENU = RED / "CLIKMENU.C"
COMMAND_CORE = ROOT / "dm1_v1_movement_command_core_pc34_compat.c"
COMMAND_TEST = ROOT / "test_dm1_v1_movement_command_core_pc34_compat.c"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_order(text: str, needles: list[str], label: str) -> list[int]:
    positions: list[int] = []
    last = -1
    for needle in needles:
        pos = require(text, needle, label)
        if pos <= last:
            raise AssertionError(f"{label}: out of order {needle!r}")
        positions.append(pos)
        last = pos
    return positions


def function_range(text: str, name: str, rettype: str = r"(?:void|BOOLEAN|int|const char\*)", next_anchor: str | None = None) -> tuple[int, int, str]:
    m = re.search(r"\b(?:" + rettype + r")\s+" + re.escape(name) + r"\s*\(", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return line_no(text, m.start()), line_no(text, pos), text[m.start():pos + 1]
    if next_anchor:
        end = text.find(next_anchor, brace)
        if end > brace:
            return line_no(text, m.start()), line_no(text, end) - 1, text[m.start():end]
    raise AssertionError(f"unterminated function {name}")


def exact_span(text: str, offsets: list[int], base_line: int = 1) -> tuple[int, int]:
    return base_line + line_no(text, min(offsets)) - 1, base_line + line_no(text, max(offsets)) - 1


def run(cmd: list[str], timeout: int = 60) -> str:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    if p.returncode != 0:
        raise AssertionError(f"command failed {' '.join(cmd)}:\n{p.stdout[-3000:]}")
    return p.stdout.strip()


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    clik = CLIKMENU.read_text(encoding="latin-1")
    core = COMMAND_CORE.read_text(encoding="utf-8")
    test = COMMAND_TEST.read_text(encoding="utf-8")

    f0366_start, f0366_end, f0366 = function_range(clik, "F0366_COMMAND_ProcessTypes3To6_MoveParty", next_anchor="#include \"CLIKCHAM.C\"")
    source_order = require_order(f0366, [
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "F0325_CHAMPION_DecrementStamina(AL1118_ui_ChampionIndex",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "if (L1117_B_MovementBlocked) {",
        "F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage",
        "MASK0x0008_WOUND_TORSO | MASK0x0010_WOUND_LEGS",
        "C2_ATTACK_SELF",
        "F0357_COMMAND_DiscardAllInput();",
        "F0693_WaitVerticalBlank();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;\n                return;",
    ], "ReDMCSB F0366 blocked movement side-effect order")
    source_side_effect_span = exact_span(f0366, source_order, f0366_start)

    require_order(f0366, [
        "if (L1117_B_MovementBlocked) {",
        "F0357_COMMAND_DiscardAllInput();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;\n                return;",
        "if (L1123_B_StairsSquare) {",
        "AL1115_ui_Ticks = 1;",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
    ], "ReDMCSB blocked branch returns before accepted-move result/cooldown")

    core_start, core_end, process_one = function_range(core, "DM1_V1_MovementCommandCore_ProcessOnePc34Compat", rettype="int")
    core_order = require_order(process_one, [
        "outResult->commandHandled = 1;",
        "dm1_v1_apply_pre_step_stamina_cost(party, outResult);",
        "if (!F0702_MOVEMENT_TryMove_Compat(dungeon, party, action, &outResult->movement)) {",
        "outResult->movementBlocked = 1;",
        "dm1_v1_record_blocked_wall_or_door_damage_request(party, action, outResult);",
        "outResult->inputDiscardRequested = 1;",
        "outResult->blockedMovementVblankWaitRequested = 1;",
        "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);\n        return 1;",
    ], "Firestaff command-core blocked movement side-effect order")
    core_side_effect_span = exact_span(process_one, core_order, core_start)

    damage_start, damage_end, damage = function_range(core, "dm1_v1_record_blocked_wall_or_door_damage_request", rettype="static int|static void")
    require_order(damage, [
        "if (!party || !outResult || party->championCount <= 0)",
        "firstCell = dm1_v1_normalize_cell(movementArrowIndex + party->direction + 2);",
        "outResult->blockedByWallOrDoorDamageRequested = 1;",
        "outResult->blockedByWallOrDoorDamageAttack = 1;",
        "outResult->blockedByWallOrDoorDamageAttackTypeSelf = 2;",
        "outResult->blockedByWallOrDoorDamageAllowedWounds = 0x0018u;",
        "outResult->blockedByWallOrDoorDamageSecondCell = dm1_v1_normalize_cell(firstCell + 1);",
    ], "Firestaff blocked wall/door damage request seam")

    for needle in [
        "blocked movement reported",
        "blocked input discard requested",
        "blocked command does not release wait",
        "blocked command does not redraw viewport",
        "blocked movement still decrements stamina",
        "blocked movement flushes queued input",
        "blocked wall requests self damage seam",
        "blocked wall damage attack is one",
        "blocked wall damage attack type self",
        "blocked wall damage wounds torso legs",
        "blocked forward north first target cell",
        "blocked forward north second target cell",
    ]:
        require(test, needle, f"runtime blocked-movement assertion {needle}")

    runtime_stdout = run([str(ROOT / "build/test_dm1_v1_movement_command_core_pc34_compat")])

    status = "PASS505_DM1_V1_BLOCKED_MOVEMENT_SIDE_EFFECT_SOURCE_LOCK_PROVEN"
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "primarySource": "ReDMCSB_WIP20210206/Toolchains/Common/Source/CLIKMENU.C",
        "sourceAudit": {
            "F0366": f"CLIKMENU.C:{f0366_start}-{f0366_end}",
            "blockedSideEffects": f"CLIKMENU.C:{source_side_effect_span[0]}-{source_side_effect_span[1]}",
        },
        "firestaffGuards": {
            "DM1_V1_MovementCommandCore_ProcessOnePc34Compat": f"dm1_v1_movement_command_core_pc34_compat.c:{core_start}-{core_end}",
            "blockedBranch": f"dm1_v1_movement_command_core_pc34_compat.c:{core_side_effect_span[0]}-{core_side_effect_span[1]}",
            "blockedDamageSeam": f"dm1_v1_movement_command_core_pc34_compat.c:{damage_start}-{damage_end}",
            "runtimeExecutable": "build/test_dm1_v1_movement_command_core_pc34_compat",
        },
        "runtimeOutputFirstLine": runtime_stdout.splitlines()[0] if runtime_stdout else "",
        "notClaimed": [
            "combat RNG/wound materialization parity for the pending damage request",
            "representative original DOS overlay capture parity",
            "viewport/walls rendering completion",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    REPORT.write_text("\n".join([
        "# Pass505 - DM1 V1 blocked movement side-effect source lock",
        "",
        f"Status: {status}",
        "",
        "## ReDMCSB-first source audit",
        f"- CLIKMENU.C:{f0366_start}-{f0366_end} / F0366_COMMAND_ProcessTypes3To6_MoveParty is the primary source.",
        f"- CLIKMENU.C:{source_side_effect_span[0]}-{source_side_effect_span[1]} proves the blocked-step order: input wait is armed, living champions spend stamina, legality runs, blocked wall/door/fake-wall damage is requested, input is discarded, PC-34 waits one VBlank, input wait is re-armed by setting G0321_B_StopWaitingForPlayerInput = C0_FALSE, and the function returns before accepted-move/cooldown code.",
        "",
        "## Firestaff executable guards",
        f"- dm1_v1_movement_command_core_pc34_compat.c:{core_side_effect_span[0]}-{core_side_effect_span[1]} keeps the same command-core blocked branch order.",
        f"- dm1_v1_movement_command_core_pc34_compat.c:{damage_start}-{damage_end} records the source-locked self-damage request seam without claiming combat RNG/wound materialization.",
        "- build/test_dm1_v1_movement_command_core_pc34_compat asserts blocked movement spends stamina, flushes queued input, does not release wait/redraw, and records the wall/door self-damage request fields.",
        "",
        "## Scope guard",
        "- This gate advances movement legality/timing/source-lock completion only for blocked-step side effects. It does not touch viewport/walls or original-capture lanes.",
        "",
        f"Manifest: parity-evidence/verification/{PASS}/manifest.json",
    ]) + "\n")

    print(f"PASS {PASS}")
    print(f"- ReDMCSB: CLIKMENU.C:{f0366_start}-{f0366_end}; blocked side effects {source_side_effect_span[0]}-{source_side_effect_span[1]}")
    print(f"- Firestaff: command core {core_start}-{core_end}; blocked branch {core_side_effect_span[0]}-{core_side_effect_span[1]}; damage seam {damage_start}-{damage_end}")
    print(f"- report: {REPORT.relative_to(ROOT)}")
    print(f"- manifest: {MANIFEST.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, subprocess.SubprocessError) as exc:
        print(f"FAIL {PASS}: {exc}", file=sys.stderr)
        raise SystemExit(1)
