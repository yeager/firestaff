#!/usr/bin/env python3
"""Verify DM1 V1 projectile travel blockers are source-locked.

This gate is deliberately narrow: it binds ReDMCSB projectile flight to
Firestaff's F0811/F0814/F0816 travel blocker regression cases for walls,
closed real fake walls, stairs-to-stairs, closed doors, and pass-through doors.
"""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED_ROOT = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
PROJ = RED_ROOT / "PROJEXPL.C"
DUNGEON = RED_ROOT / "DUNGEON.C"
LOCAL_C = ROOT / "src/memory/memory_projectile_pc34_compat.c"
LOCAL_TEST = ROOT / "tests/test_dm1_v1_projectile_explosion_render_pc34_compat.c"


def block(path: Path, start: int, end: int) -> str:
    return "\n".join(path.read_text(encoding="latin-1").splitlines()[start - 1:end])


def require(label: str, text: str, needles: list[str]) -> None:
    compact = " ".join(text.split())
    for needle in needles:
        if " ".join(needle.split()) not in compact:
            raise AssertionError(f"{label}: missing {needle!r}")


def main() -> int:
    require("PROJEXPL.C:689-727 projectile source-cell impacts and wall travel blockers",
        block(PROJ, 689, 727),
        [
            "if (L0519_ps_Event->A.A.Type == C48_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS)",
            "F0217_PROJECTILE_HasImpactOccured(CM2_ELEMENT_CHAMPION",
            "F0217_PROJECTILE_HasImpactOccured(CM1_ELEMENT_CREATURE",
            "if (L0520_ps_Projectile->KineticEnergy <= (AL0516_ui_StepEnergy",
            "M034_SQUARE_TYPE(AL0516_ui_Square = F0151_DUNGEON_GetSquare",
            "C00_ELEMENT_WALL",
            "C06_ELEMENT_FAKEWALL",
            "MASK0x0001_FAKEWALL_IMAGINARY | MASK0x0004_FAKEWALL_OPEN",
            "C03_ELEMENT_STAIRS",
            "F0217_PROJECTILE_HasImpactOccured(M034_SQUARE_TYPE(AL0516_ui_Square)",
        ])
    require("PROJEXPL.C:735-745 intra-square door impact",
        block(PROJ, 735, 745),
        [
            "F0267_MOVE_GetMoveResult_CPSCE",
            "M034_SQUARE_TYPE(F0151_DUNGEON_GetSquare",
            "C04_ELEMENT_DOOR",
            "F0217_PROJECTILE_HasImpactOccured(C04_ELEMENT_DOOR",
        ])
    require("PROJEXPL.C:471-505 door pass-through/impact classifier",
        block(PROJ, 471, 505),
        [
            "case C04_ELEMENT_DOOR",
            "AL0487_i_DoorState",
            "C5_DOOR_STATE_DESTROYED",
            "C1_DOOR_STATE_CLOSED_ONE_FOURTH",
            "MASK0x0002_PROJECTILES_CAN_PASS_THROUGH",
            "L0486_T_ProjectileAssociatedThing >= C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL",
            "MASK0x0100_POUCH_AND_PASS_THROUGH_DOORS",
            "return C0_FALSE",
        ])
    require("DUNGEON.C:560-565 projectile pass-through door attribute",
        block(DUNGEON, 560, 565),
        [
            "MASK0x0002_PROJECTILES_CAN_PASS_THROUGH",
            "Portcullis",
            "Wooden door",
            "Iron door",
            "Ra door",
        ])

    local = LOCAL_C.read_text(encoding="utf-8")
    require("memory_projectile_pc34_compat.c local blocker implementation",
        local,
        [
            "F0814_PROJECTILE_InspectDestination_Compat",
            "PROJECTILE_BLOCKER_WALL",
            "PROJECTILE_BLOCKER_STAIRS",
            "PROJECTILE_BLOCKER_CLOSED_DOOR",
            "F0816_PROJECTILE_DoesPassThroughDoor_Compat",
            "in->projectileSubtype >= PROJECTILE_SUBTYPE_HARM_NON_MATERIAL",
            "in->projectileSubtype != PROJECTILE_SUBTYPE_OPEN_DOOR",
            "F0811_PROJECTILE_Advance_Compat",
            "dispatch = PROJECTILE_RESULT_HIT_WALL",
            "dispatch = PROJECTILE_RESULT_HIT_DOOR",
        ])
    test = LOCAL_TEST.read_text(encoding="utf-8")
    require("test_dm1_v1_projectile_explosion_render_pc34_compat.c regression cases",
        test,
        [
            "test_projectile_travel_blockers",
            "wall hit result",
            "closed fakewall hit result",
            "open fakewall flies",
            "stairs-to-stairs impacts as wall",
            "closed door hit result",
            "one-fourth door flies",
            "allowed magical projectile flies through",
            "open-door projectile still hits pass-through door",
        ])

    print("PASS pass563_dm1_v1_projectile_travel_blockers_source_lock")
    print("- PROJEXPL.C:689-727 projectile current-cell impact, energy gate, wall/fakewall/stairs travel blocker")
    print("- PROJEXPL.C:735-745 intra-square closed-door impact")
    print("- PROJEXPL.C:471-505 door state/pass-through classifier")
    print("- DUNGEON.C:560-565 door info pass-through attribute")
    print("- Firestaff: F0811/F0814/F0816 plus projectile travel blocker regression cases")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
