#!/usr/bin/env python3
"""Verify DM1 V1 movement legality stays source-locked to ReDMCSB.

This is a narrow static gate for the party movement legality contract around
walls, doors, stairs, and pits.  It deliberately does not cover the already
separate cooldown gate or party/group collision gate.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
RED_ROOT = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()

CLIKMENU = RED_ROOT / "CLIKMENU.C"
MOVESENS = RED_ROOT / "MOVESENS.C"
DUNGEON = RED_ROOT / "DUNGEON.C"
COMMAND = RED_ROOT / "COMMAND.C"
FIRE_C = ROOT / "memory_movement_pc34_compat.c"
FIRE_H = ROOT / "memory_movement_pc34_compat.h"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_re(text: str, pattern: str, label: str) -> re.Match[str]:
    m = re.search(pattern, text, re.S)
    if not m:
        raise AssertionError(f"{label}: missing pattern {pattern!r}")
    return m


def require_in_order(text: str, markers: list[str], label: str) -> list[int]:
    out: list[int] = []
    last = -1
    for marker in markers:
        pos = require(text, marker, label)
        if pos <= last:
            raise AssertionError(f"{label}: marker out of order: {marker!r}")
        out.append(pos)
        last = pos
    return out


def find_function(text: str, name: str) -> tuple[int, str]:
    m = re.search(r"\b(?:static\s+)?(?:int|void|BOOLEAN)\s+" + re.escape(name) + r"\s*\(", text)
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
                return m.start(), text[m.start():pos + 1]
    raise AssertionError(f"unterminated function {name}")


def main() -> int:
    clik = CLIKMENU.read_text(encoding="latin-1")
    moves = MOVESENS.read_text(encoding="latin-1")
    dung = DUNGEON.read_text(encoding="latin-1")
    command = COMMAND.read_text(encoding="latin-1")
    fire_c = FIRE_C.read_text(encoding="utf-8")
    fire_h = FIRE_H.read_text(encoding="utf-8")

    citations: list[str] = []

    # ReDMCSB source-first contract: CLIKMENU owns party target-square legality.
    for needle, label in [
        ("if (L1123_B_StairsSquare && (AL1118_ui_MovementArrowIndex == 2))", "CLIKMENU backward-on-stairs takes stairs"),
        ("if (L1116_i_SquareType == C03_ELEMENT_STAIRS)", "CLIKMENU entering stairs is a consequence square"),
        ("if (L1116_i_SquareType == C00_ELEMENT_WALL)", "CLIKMENU wall blocks"),
        ("if (L1116_i_SquareType == C04_ELEMENT_DOOR)", "CLIKMENU door gate"),
        ("L1117_B_MovementBlocked = (L1117_B_MovementBlocked != C0_DOOR_STATE_OPEN) && (L1117_B_MovementBlocked != C1_DOOR_STATE_CLOSED_ONE_FOURTH) && (L1117_B_MovementBlocked != C5_DOOR_STATE_DESTROYED);", "CLIKMENU allowed door states"),
        ("if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)", "CLIKMENU fakewall gate"),
        ("L1117_B_MovementBlocked = (!M007_GET(AL1115_ui_Square, MASK0x0004_FAKEWALL_OPEN) && !M007_GET(AL1115_ui_Square, MASK0x0001_FAKEWALL_IMAGINARY));", "CLIKMENU fakewall open/imaginary bits"),
        ("F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);", "CLIKMENU normal move dispatch"),
    ]:
        citations.append(f"{label}: {CLIKMENU.name}:{line_no(clik, require(clik, needle, label))}")

    require_in_order(clik, [
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection",
        "L1116_i_SquareType = M034_SQUARE_TYPE(AL1115_ui_Square = F0151_DUNGEON_GetSquare(L1121_i_MapX, L1122_i_MapY));",
        "if (L1116_i_SquareType == C03_ELEMENT_STAIRS)",
        "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
        "if (L1116_i_SquareType == C04_ELEMENT_DOOR)",
        "if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)",
        "if (L1117_B_MovementBlocked)",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
    ], "CLIKMENU party movement legality order")

    # ReDMCSB pit chain belongs to post-move resolution, not pre-step blocking.
    for needle, label in [
        ("if ((AL0709_i_DestinationSquareType == C02_ELEMENT_PIT) && !L0713_B_ThingLevitates && M007_GET(AL0708_i_DestinationSquare, MASK0x0008_PIT_OPEN) && !M007_GET(AL0708_i_DestinationSquare, MASK0x0001_PIT_IMAGINARY))", "MOVESENS open non-imaginary pit fall"),
        ("L0715_ui_MapIndexDestination = F0154_DUNGEON_GetLocationAfterLevelChange(L0715_ui_MapIndexDestination, 1, &P0560_i_DestinationMapX, &P0561_i_DestinationMapY);", "MOVESENS pit level change"),
        ("F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination);", "MOVESENS pit current-map switch"),
        ("G0402_B_UseRopeToClimbDownPit = C0_FALSE;", "MOVESENS pit rope reset"),
    ]:
        citations.append(f"{label}: {MOVESENS.name}:{line_no(moves, require(moves, needle, label))}")

    # DUNGEON coordinate/bounds semantics feed CLIKMENU target-square legality.
    for needle, label in [
        ("*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount", "DUNGEON relative step X/Y"),
        ("return G0271_ppuc_CurrentMapData[P0258_i_MapX][P0259_i_MapY];", "DUNGEON in-bounds square fetch"),
        ("return M035_SQUARE(C00_ELEMENT_WALL, 0);", "DUNGEON out-of-bounds wall square"),
        ("return ((((L0256_i_SquareType = M034_SQUARE_TYPE(F0151_DUNGEON_GetSquare(P0274_i_MapX, P0275_i_MapY))) == C00_ELEMENT_WALL) || (L0256_i_SquareType == C03_ELEMENT_STAIRS)) << 1) + L0257_B_NorthSouthOrientedStairs;", "DUNGEON stairs exit direction"),
    ]:
        citations.append(f"{label}: {DUNGEON.name}:{line_no(dung, require(dung, needle, label))}")

    # COMMAND is only cited to prove dispatch/gate is outside this static legality gate.
    for needle, label in [
        ("if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT) && (G0310_i_DisabledMovementTicks", "COMMAND cooldown cardinal gate"),
        ("F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "COMMAND turn dispatch"),
        ("F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);", "COMMAND movement dispatch"),
    ]:
        citations.append(f"{label}: {COMMAND.name}:{line_no(command, require(command, needle, label))}")

    # Firestaff implementation must keep the exact legality split:
    # F0702 blocks pre-step walls/closed doors/closed fakewalls; pits and stairs pass.
    _, f0702 = find_function(fire_c, "F0702_MOVEMENT_TryMove_Compat")
    require_in_order(f0702, [
        "F0701_MOVEMENT_GetStepDelta_Compat(party->direction, moveAction, &dx, &dy);",
        "if (nx < 0 || nx >= map->width || ny < 0 || ny >= map->height)",
        "if (elementType == DUNGEON_ELEMENT_WALL)",
        "if (elementType == DUNGEON_ELEMENT_DOOR)",
        "doorState = squareByte & 0x07;",
        "if (doorState != 0 && doorState != 1 && doorState != 5)",
        "} else if (elementType == DUNGEON_ELEMENT_FAKEWALL)",
        "if (!(squareByte & 0x04) && !(squareByte & 0x01))",
        "outResult->newMapX = nx;",
        "outResult->resultCode = MOVE_OK;",
    ], "Firestaff F0702 source-order legality")

    _, f0706 = find_function(fire_c, "F0706_MOVEMENT_IsSquarePassable_Compat")
    for needle in [
        "case DUNGEON_ELEMENT_PIT:",
        "case DUNGEON_ELEMENT_TELEPORTER:",
        "case DUNGEON_ELEMENT_STAIRS:",
        "return ((squareByte & 0x04) || (squareByte & 0x01)) ? 1 : 0;",
        "return (doorState == 0 || doorState == 1 || doorState == 5) ? 1 : 0;",
    ]:
        require(f0706, needle, "Firestaff F0706 passability")

    _, f0707 = find_function(fire_c, "F0707_MOVEMENT_IsSquarePassableForContext_Compat")
    require(f0707, "return (passContext == MOVEMENT_PASS_CTX_CREATURE) ? 0 : 1;", "Firestaff F0707 stairs context split")

    _, f0704 = find_function(fire_c, "F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat")
    require_in_order(f0704, [
        "if (elementType == DUNGEON_ELEMENT_PIT)",
        "int targetLevel = cursor.mapIndex + 1;",
        "cursor.mapIndex = targetLevel;",
        "outResolution->transitioned = 1;",
        "outResolution->pitCount += 1;",
    ], "Firestaff F0704 post-move pit resolution")

    # Header documentation must cite this as party-side legality and keep group/cooldown separate.
    for needle in [
        "source-semantic square passability (wall, door state, fake-wall bits,",
        "stairs consequence square",
        "No thing-list checks — those are for sensor/group processing.",
        "Source-locked party/group collision gate.",
    ]:
        require(fire_h, needle, "Firestaff movement header source contract")

    # Emit compact citations so heartbeat/task logs have exact source anchors.
    print("DM1 V1 movement legality source-lock gate passed")
    for c in citations:
        print(f"- {c}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
