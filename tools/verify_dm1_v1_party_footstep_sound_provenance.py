#!/usr/bin/env python3
"""Verify ReDMCSB provenance for the absence of a DM1 V1 party footstep SFX.

This is intentionally a source-audit probe, not an emulator/runtime-audio probe.
It proves the narrow claim that the successful party movement path does not
request a footstep sound and separates that absence from footprint graphics,
footprint events, and creature movement sounds.
"""
from __future__ import annotations

import argparse
import re
from pathlib import Path

DEFAULT_REDMCSB_SOURCE = Path(
    "/Users/bosse/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
AUDITED_FILES = [
    "COMMAND.C",
    "MOVESENS.C",
    "GROUP.C",
    "SOUND.C",
    "TIMELINE.C",
    "DUNGEON.C",
    "DATA.C",
    "DEFS.H",
]
SUPPLEMENTAL_FILES = ["CLIKMENU.C"]
SOUND_CALL_RE = re.compile(
    r"\b(F0064_SOUND_RequestPlay_CPSD|F0065_SOUND_PlayPendingSound_CPSD|"
    r"F0060_SOUND_Play_CPSX|F0709_StartSound)\b"
)
FOOTSTEP_SOUND_CONSTANT_RE = re.compile(
    r"\b(?:C|M)\d+_SOUND_[A-Z0-9_]*(?:FOOT|FOOTPRINT|STEP|WALK)[A-Z0-9_]*\b"
)
SEARCH_RE = re.compile(r"foot|footprint|step|walk|move|sound", re.IGNORECASE)


def load_lines(source: Path, filename: str) -> list[str]:
    path = source / filename
    if not path.exists():
        raise SystemExit(f"missing required source file: {path}")
    return path.read_text(encoding="latin-1").splitlines()


def block(source: Path, filename: str, start: int, end: int) -> list[str]:
    lines = load_lines(source, filename)
    return lines[start - 1 : end]


def compact(text: str) -> str:
    return " ".join(text.split())


def require(citation: str, lines: list[str], needles: list[str]) -> None:
    haystack = compact("\n".join(lines))
    for needle in needles:
        if compact(needle) not in haystack:
            raise SystemExit(f"FAIL {citation}: missing expected text: {needle}")
    print(f"PASS {citation}")


def require_no_sound_call(citation: str, lines: list[str]) -> None:
    hits = [(offset, line) for offset, line in enumerate(lines, start=1) if SOUND_CALL_RE.search(line)]
    if hits:
        detail = "; ".join(f"+{offset}: {line.strip()}" for offset, line in hits)
        raise SystemExit(f"FAIL {citation}: unexpected sound call(s): {detail}")
    print(f"PASS {citation}: no sound request/play call")


def require_no_footstep_sound_constant(source: Path, filenames: list[str]) -> None:
    hits: list[str] = []
    for filename in filenames:
        for lineno, line in enumerate(load_lines(source, filename), start=1):
            if FOOTSTEP_SOUND_CONSTANT_RE.search(line):
                hits.append(f"{filename}:{lineno}:{line.strip()}")
    if hits:
        raise SystemExit("FAIL footstep sound constant absence:\n" + "\n".join(hits))
    print("PASS audited source has no SOUND_*FOOT/FOOTPRINT/STEP/WALK constant")


def require_no_sound_call_with_foot_terms(source: Path, filenames: list[str]) -> None:
    hits: list[str] = []
    for filename in filenames:
        lines = load_lines(source, filename)
        for idx, line in enumerate(lines):
            if not SOUND_CALL_RE.search(line):
                continue
            context = "\n".join(lines[max(0, idx - 3) : min(len(lines), idx + 4)])
            if re.search(r"foot|footprint|walk", context, re.IGNORECASE):
                hits.append(f"{filename}:{idx + 1}:{line.strip()}")
    if hits:
        raise SystemExit("FAIL sound call near foot/footprint/walk term:\n" + "\n".join(hits))
    print("PASS no sound request/play call is coupled to foot/footprint/walk terms")


def summarize_search(source: Path, filenames: list[str]) -> None:
    term_hits = 0
    sound_calls = 0
    for filename in filenames:
        for line in load_lines(source, filename):
            if SEARCH_RE.search(line):
                term_hits += 1
            if SOUND_CALL_RE.search(line):
                sound_calls += 1
    print(f"INFO searched {len(filenames)} source files for foot/footprint/step/walk/move/sound patterns")
    print(f"INFO term-hit lines={term_hits} sound-call lines={sound_calls}")


def verify(source: Path) -> None:
    files = AUDITED_FILES + SUPPLEMENTAL_FILES
    for filename in AUDITED_FILES:
        load_lines(source, filename)

    require(
        "COMMAND.C:2150-2156 party command dispatch",
        block(source, "COMMAND.C", 2150, 2156),
        [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command)",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command)",
        ],
    )
    require_no_sound_call("COMMAND.C:2150-2156 dispatch", block(source, "COMMAND.C", 2150, 2156))

    require(
        "CLIKMENU.C:256-329 legal party movement path",
        block(source, "CLIKMENU.C", 256, 329),
        [
            "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "L1116_i_SquareType == C00_ELEMENT_WALL",
            "M036_DOOR_STATE(AL1115_ui_Square)",
            "MASK0x0004_FAKEWALL_OPEN",
            "MASK0x0001_FAKEWALL_IMAGINARY",
            "F0175_GROUP_GetThing",
            "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY",
        ],
    )
    require(
        "CLIKMENU.C:291-309 blocked-party-damage sound exception",
        block(source, "CLIKMENU.C", 291, 309),
        ["F0064_SOUND_RequestPlay_CPSD(M562_SOUND_PARTY_DAMAGED"],
    )
    require_no_sound_call(
        "CLIKMENU.C:325-347 successful movement cooldown tail",
        block(source, "CLIKMENU.C", 325, 347),
    )
    require(
        "CLIKMENU.C:330-347 movement cooldown only",
        block(source, "CLIKMENU.C", 330, 347),
        [
            "F0310_CHAMPION_GetMovementTicks",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks",
            "G0311_i_ProjectileDisabledMovementTicks = 0",
        ],
    )

    require(
        "MOVESENS.C:441-451 party coordinate update",
        block(source, "MOVESENS.C", 441, 451),
        [
            "P0557_T_Thing == C0xFFFF_THING_PARTY",
            "G0306_i_PartyMapX = P0560_i_DestinationMapX",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY",
            "L0716_ui_Direction = G0308_i_PartyDirection",
        ],
    )
    require_no_sound_call("MOVESENS.C:441-451 party coordinate update", block(source, "MOVESENS.C", 441, 451))
    require(
        "MOVESENS.C:493-498 audible teleporter buzz",
        block(source, "MOVESENS.C", 493, 498),
        ["F0064_SOUND_RequestPlay_CPSD(M560_SOUND_BUZZ"],
    )
    require(
        "MOVESENS.C:575-603 pit-fall scream",
        block(source, "MOVESENS.C", 575, 603),
        ["F0064_SOUND_RequestPlay_CPSD(M561_SOUND_SCREAM"],
    )
    require(
        "MOVESENS.C:764-783 scent/footprint state",
        block(source, "MOVESENS.C", 764, 783),
        [
            "G0407_s_Party.Event79Count_Footprints",
            "G0407_s_Party.LastScentIndex = G0407_s_Party.ScentCount",
            "F0317_CHAMPION_AddScentStrength",
        ],
    )
    require_no_sound_call("MOVESENS.C:764-783 scent/footprint state", block(source, "MOVESENS.C", 764, 783))
    require(
        "MOVESENS.C:811-819 party arrival sensors",
        block(source, "MOVESENS.C", 811, 819),
        ["F0276_SENSOR_ProcessThingAdditionOrRemoval", "C0xFFFF_THING_PARTY"],
    )
    require_no_sound_call("MOVESENS.C:811-819 party arrival sensors", block(source, "MOVESENS.C", 811, 819))

    require(
        "MOVESENS.C:847-853 creature movement sound request",
        block(source, "MOVESENS.C", 847, 853),
        [
            "F0514_MOVE_GetSound",
            "F0064_SOUND_RequestPlay_CPSD(L1638_ui_MovementSoundIndex",
            "F0064_SOUND_RequestPlay_CPSD(F0514_MOVE_GetSound",
        ],
    )
    require(
        "MOVESENS.C:984-995 I34E creature sound lookup",
        block(source, "MOVESENS.C", 984, 995),
        [
            "G0300_B_PartyIsResting",
            "G0243_as_Graphic559_CreatureInfo[P2099_i_Type].AttackSoundOrdinal",
            "G2003_aauc_CreatureSounds[M001_ORDINAL_TO_INDEX(L3025_i_SoundOrdinal)][C1_MOVEMENT_SOUND]",
            "return -1",
        ],
    )

    require(
        "GROUP.C:267-280 creature animation movement sound",
        block(source, "GROUP.C", 267, 280),
        [
            "L0331_ui_CreatureType == C13_CREATURE_COUATL",
            "F0514_MOVE_GetSound",
            "F0064_SOUND_RequestPlay_CPSD(F0514_MOVE_GetSound",
        ],
    )

    require(
        "DEFS.H:855-865 party footprint/scent fields",
        block(source, "DEFS.H", 855, 865),
        ["Event79Count_Footprints", "ScentCount", "FirstScentIndex", "LastScentIndex", "SCENT Scents[24]"],
    )
    require(
        "DUNGEON.C:2631-2719 footprint graphics path",
        block(source, "DUNGEON.C", 2631, 2719),
        [
            "AL0307_uc_FootprintsAllowed",
            "T0172049_Footprints",
            "F0315_CHAMPION_GetScentOrdinal",
            "MASK0x8000_FOOTPRINTS",
            "M558_FLOOR_ORNAMENT_ORDINAL",
        ],
    )
    require_no_sound_call("DUNGEON.C:2631-2719 footprint graphics path", block(source, "DUNGEON.C", 2631, 2719))
    require(
        "TIMELINE.C:1998-1999 footprint event expiry",
        block(source, "TIMELINE.C", 1998, 1999),
        ["C79_EVENT_FOOTPRINTS", "--G0407_s_Party.Event79Count_Footprints"],
    )
    require_no_sound_call("TIMELINE.C:1998-1999 footprint event expiry", block(source, "TIMELINE.C", 1998, 1999))

    require(
        "DEFS.H:58-59 I34E-family sound count",
        block(source, "DEFS.H", 58, 59),
        ["MEDIA485_P20JB_I34E_I34M", "#define M513_SOUND_COUNT", "35"],
    )
    require(
        "DEFS.H:100-128 I34E game sound constants",
        block(source, "DEFS.H", 100, 128),
        [
            "M562_SOUND_PARTY_DAMAGED",
            "C28_SOUND_MOVE_ANIMATED_ARMOUR_DETH_KNIGHT",
            "C34_SOUND_MOVE_SKELETON",
        ],
    )
    require(
        "DATA.C:1264-1310 I34E-family sound table",
        block(source, "DATA.C", 1264, 1310),
        ["MEDIA719_I34E_I34M", "7 creature movement sounds"],
    )
    require(
        "DEFS.H:6834-6844 sound request prototype",
        block(source, "DEFS.H", 6834, 6844),
        ["F0064_SOUND_RequestPlay_CPSD", "P0088_SoundIndex", "P0091_ui_Mode"],
    )
    require(
        "SOUND.C:1476-1544 sound request/schedule",
        block(source, "SOUND.C", 1476, 1544),
        ["F0064_SOUND_RequestPlay_CPSD", "C20_EVENT_PLAY_SOUND", "F0238_TIMELINE_AddEvent_GetEventIndex_CPSE"],
    )
    require(
        "SOUND.C:1608-1642 immediate/pending sound path",
        block(source, "SOUND.C", 1608, 1642),
        ["Play the sound immediately", "F0709_StartSound", "F0060_SOUND_Play_CPSX"],
    )
    require(
        "SOUND.C:1756-1850 pending sound flush",
        block(source, "SOUND.C", 1756, 1850),
        ["F0065_SOUND_PlayPendingSound_CPSD", "F0709_StartSound", "G0583_i_PendingSoundIndex"],
    )
    require(
        "TIMELINE.C:1903-1905 delayed sound event",
        block(source, "TIMELINE.C", 1903, 1905),
        ["C20_EVENT_PLAY_SOUND", "F0064_SOUND_RequestPlay_CPSD"],
    )

    require_no_footstep_sound_constant(source, files)
    require_no_sound_call_with_foot_terms(source, files)
    summarize_search(source, files)
    print("PASS dm1_v1_party_footstep_sound_provenance: no source-backed party footstep sound trigger found")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--redmcsb-source", type=Path, default=DEFAULT_REDMCSB_SOURCE)
    args = parser.parse_args()
    verify(args.redmcsb_source)


if __name__ == "__main__":
    main()
