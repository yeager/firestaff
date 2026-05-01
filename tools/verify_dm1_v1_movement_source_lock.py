#!/usr/bin/env python3
"""Verify the DM1 V1 movement legality/timing source lock against ReDMCSB.

This is a provenance gate for the compat movement layer.  It intentionally
checks a narrow set of source facts from ReDMCSB before asserting Firestaff
implementation snippets: command queue dispatch, relative step math, door /
fake-wall movement legality, and movement cooldown ticks.
"""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
SRC = {
    "COMMAND.C": REDMCSB_SOURCE / "COMMAND.C",
    "CLIKMENU.C": REDMCSB_SOURCE / "CLIKMENU.C",
    "DUNGEON.C": REDMCSB_SOURCE / "DUNGEON.C",
    "MOVESENS.C": REDMCSB_SOURCE / "MOVESENS.C",
    "GAMELOOP.C": REDMCSB_SOURCE / "GAMELOOP.C",
    "CHAMPION.C": REDMCSB_SOURCE / "CHAMPION.C",
}
COMPAT_C = ROOT / "memory_movement_pc34_compat.c"
COMPAT_H = ROOT / "memory_movement_pc34_compat.h"
ORCH_C = ROOT / "memory_tick_orchestrator_pc34_compat.c"
PROBE_C = ROOT / "firestaff_m10_tick_orchestrator_probe.c"
GROUP_PROBE_C = ROOT / "firestaff_m11_pass44_party_group_collision_probe.c"
DEFAULT_OUT = ROOT / "parity-evidence/verification/dm1_v1_movement_source_lock.json"


def lines(path: Path) -> list[str]:
    return path.read_text(encoding="latin-1").splitlines()


def block(path: Path, start: int, end: int) -> str:
    ls = lines(path)
    return "\n".join(ls[start - 1:end])


def require(cite: str, text: str, needles: list[str]) -> dict[str, Any]:
    compact = " ".join(text.split())
    for needle in needles:
        if " ".join(needle.split()) not in compact:
            raise SystemExit(f"{cite} missing expected text: {needle}")
    return {"citation": cite, "verified": True, "needles": needles}


def verify_redmcsb() -> list[dict[str, Any]]:
    checks: list[dict[str, Any]] = []
    checks.append(require("COMMAND.C:396-405", block(SRC["COMMAND.C"], 396, 405), [
        "C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT",
        "C006_COMMAND_MOVE_LEFT", "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT",
        "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "C083_COMMAND_TOGGLE_INVENTORY_LEADER",
    ]))
    checks.append(require("COMMAND.C:1403-1431", block(SRC["COMMAND.C"], 1403, 1431), [
        "while (L1107_Command = P0721_ps_MouseInput->Command)",
        "P0724_i_ButtonsStatus & P0721_ps_MouseInput->Button",
        "F0798_COMMAND_IsPointInZone",
    ]))
    checks.append(require("COMMAND.C:2045-2156", block(SRC["COMMAND.C"], 2045, 2156), [
        "G0310_i_DisabledMovementTicks",
        "G0311_i_ProjectileDisabledMovementTicks",
        "G0312_i_LastProjectileDisabledMovementDirection",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty",
    ]))
    checks.append(require("CLIKMENU.C:224-233", block(SRC["CLIKMENU.C"], 224, 233), [
        "1,   /* Forward */", "0,   /* Right */", "-1,  /* Backward */", "-1 }; /* Left */",
    ]))
    checks.append(require("DUNGEON.C:35-44", block(SRC["DUNGEON.C"], 35, 44), [
        "0,    /* North */", "1,    /* East */", "-1 }; /* South */",
        "-1,  /* North */", "1,   /* West */",
    ]))
    checks.append(require("DUNGEON.C:1371-1391", block(SRC["DUNGEON.C"], 1371, 1391), [
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "P0253_i_Direction += 1", "Simulate turning right",
    ]))
    checks.append(require("CLIKMENU.C:256-347", block(SRC["CLIKMENU.C"], 256, 347), [
        "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "L1116_i_SquareType == C00_ELEMENT_WALL",
        "L1117_B_MovementBlocked = M036_DOOR_STATE(AL1115_ui_Square)",
        "L1117_B_MovementBlocked != C1_DOOR_STATE_CLOSED_ONE_FOURTH",
        "MASK0x0004_FAKEWALL_OPEN", "MASK0x0001_FAKEWALL_IMAGINARY",
        "F0175_GROUP_GetThing", "G0305_ui_PartyChampionCount == 0",
        "F0357_COMMAND_DiscardAllInput", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks",
        "G0311_i_ProjectileDisabledMovementTicks = 0",
    ]))
    checks.append(require("MOVESENS.C:315-385", block(SRC["MOVESENS.C"], 315, 385), [
        "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE",
        "P0558_i_SourceMapX",
        "P0560_i_DestinationMapX",
        "L0713_B_ThingLevitates",
        "L0724_B_DestinationIsTeleporterTarget",
    ]))
    checks.append(require("MOVESENS.C:752-775", block(SRC["MOVESENS.C"], 752, 775), [
        "L0725_B_PartySquare",
        "P0557_T_Thing == C0xFFFF_THING_PARTY",
        "G0308_i_PartyDirection == L0716_ui_Direction",
        "G0362_l_LastPartyMovementTime = G0313_ul_GameTime",
    ]))
    checks.append(require("GAMELOOP.C:150-155", block(SRC["GAMELOOP.C"], 150, 155), [
        "G0310_i_DisabledMovementTicks--", "G0311_i_ProjectileDisabledMovementTicks--",
    ]))
    checks.append(require("CHAMPION.C:93-130", block(SRC["CHAMPION.C"], 93, 130), [
        "F0284_CHAMPION_SetPartyDirection", "L0834_i_Delta", "G0308_i_PartyDirection = P0600_i_Direction",
    ]))
    checks.append(require("CHAMPION.C:1180-1214", block(SRC["CHAMPION.C"], 1180, 1214), [
        "F0310_CHAMPION_GetMovementTicks", "L0933_ui_Ticks = 2", "L0933_ui_Ticks = 4 +",
        "MASK0x0020_WOUND_FEET", "C194_ICON_ARMOUR_BOOT_OF_SPEED", "return L0933_ui_Ticks",
    ]))
    return checks


def verify_firestaff() -> list[dict[str, Any]]:
    c = COMPAT_C.read_text()
    h = COMPAT_H.read_text()
    orch = ORCH_C.read_text()
    probe = PROBE_C.read_text()
    group_probe = GROUP_PROBE_C.read_text()
    checks: list[dict[str, Any]] = []
    impl_checks = [
        ("memory_movement_pc34_compat.c:F0700", c, ["return (currentDir + 1) & 3", "return (currentDir + 3) & 3"]),
        ("memory_movement_pc34_compat.c:F0701", c, ["case MOVE_FORWARD:  stepDir = direction", "case MOVE_RIGHT:    stepDir = (direction + 1) & 3", "case MOVE_BACKWARD: stepDir = (direction + 2) & 3", "case MOVE_LEFT:     stepDir = (direction + 3) & 3"]),
        ("memory_movement_pc34_compat.c:F0706 door/fakewall", c, ["case DUNGEON_ELEMENT_FAKEWALL", "squareByte & 0x04", "squareByte & 0x01", "doorState == 0 || doorState == 1 || doorState == 5"]),
        ("memory_movement_pc34_compat.c:F0702 door/fakewall", c, ["doorState != 0 && doorState != 1 && doorState != 5", "MOVE_BLOCKED_DOOR", "!(squareByte & 0x04) && !(squareByte & 0x01)", "MOVE_BLOCKED_WALL"]),
        ("memory_movement_pc34_compat.h documentation", h, ["closed one-fourth (1)", "OPEN (0x04) or IMAGINARY (0x01)"]),
        ("memory_movement_pc34_compat.c:F0708 party/group collision", c, ["F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat", "party->championCount <= 0", "THING_GET_TYPE(thing) == THING_TYPE_GROUP", "F0702_MOVEMENT_TryMove_Compat"]),
        ("memory_movement_pc34_compat.h:F0708 documentation", h, ["F0175_GROUP_GetThing", "championCount == 0 skips", "Turn-only commands never report group blocking"]),
    ]
    for cite, text, needles in impl_checks:
        checks.append(require(cite, text, needles))
    checks.append(require("memory_tick_orchestrator_pc34_compat.c:F0888 disabled movement gate", orch, [
        "movement_command_disabled_redmcsb_compat",
        "COMMAND.C:2095-2100 / 2104-2110",
        "world->disabledMovementTicks > 0",
        "world->projectileDisabledMovementTicks > 0",
        "lastProjectileDisabledMovementDirection",
        "if (movement_command_disabled_redmcsb_compat(world, mv)) return 0",
        "F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat",
        "CLIKMENU.C:291-318",
        "leave party/cooldowns",
    ]))
    checks.append(require("memory_tick_orchestrator_pc34_compat.c:F0888 successful movement cooldown", orch, [
        "redmcsb_party_move_cooldown_ticks_compat",
        "CLIKMENU.C:330-346",
        "F0841_LIFECYCLE_ComputeMoveTicks_Compat",
        "world->disabledMovementTicks = redmcsb_party_move_cooldown_ticks_compat(&world->party)",
        "world->projectileDisabledMovementTicks = 0",
    ]))
    checks.append(require("memory_tick_orchestrator_pc34_compat.c:F0890 cooldown decrement", orch, [
        "if (world->disabledMovementTicks > 0) world->disabledMovementTicks--",
        "if (world->projectileDisabledMovementTicks > 0) world->projectileDisabledMovementTicks--",
    ]))
    checks.append(require("firestaff_m10_tick_orchestrator_probe.c:disabled movement cooldown invariant", probe, [
        "disabledMovementTicks blocks cardinal movement dispatch without consuming cooldown",
        "disabledMovementTicks does not block turn dispatch",
        "periodic effects decrement disabledMovementTicks once per tick",
        "projectileDisabledMovementTicks blocks only matching absolute movement direction",
        "projectileDisabledMovementTicks allows non-matching absolute movement direction",
        "periodic effects decrement projectileDisabledMovementTicks once per tick",
        "successful move sets disabledMovementTicks to max living champion movement ticks and clears projectile cooldown",
        "passable destination group blocks movement before move result/cooldown/emissions",
        "empty party preserves ReDMCSB BUG0_85 and skips group collision in orchestrator",
    ]))
    checks.append(require("firestaff_m11_pass44_party_group_collision_probe.c:party/group collision invariant", group_probe, [
        "P44_F0708_GROUP_BLOCKS_PASSABLE_TARGET",
        "P44_F0708_EMPTY_PARTY_SKIPS_GROUP",
        "P44_F0708_TURNS_IGNORE_GROUPS",
        "P44_F0708_WALL_LEGALITY_WINS",
    ]))
    return checks


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT)
    args = ap.parse_args()
    result = {
        "status": "PASS",
        "redmcsb_source_root": str(REDMCSB_SOURCE),
        "redmcsb_checks": verify_redmcsb(),
        "firestaff_checks": verify_firestaff(),
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(f"PASS dm1_v1_movement_source_lock wrote {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
