#!/usr/bin/env python3
"""Source-lock DM1 V1 dungeon entry, movement, and viewport evidence.

This verifier is deliberately narrow and source-first.  It anchors the expected
original behaviour in ReDMCSB line ranges before checking that the current
Firestaff compatibility/evidence files still expose the same contracts for:
new-game party entry state, deterministic turn/relative movement, movement
blocking/cooldown, and viewport wall/object presentation into the 224x136
viewport bitmap.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
DEFAULT_OUT = ROOT / "parity-evidence/verification/dm1_v1_entry_movement_viewport_source_lock.json"
DM1_DUNGEON_DAT = Path(
    "/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT"
)
ROUTE_EVIDENCE = ROOT / "verification-m11/capture-route-state/pass76_capture_route_state_probe.json"



def lines(path: Path) -> list[str]:
    if not path.is_file():
        raise SystemExit(f"missing required file: {path}")
    return path.read_text(encoding="latin-1", errors="replace").splitlines()


def block(rel: str, start: int, end: int) -> str:
    ls = lines(REDMCSB_SOURCE / rel)
    return "\n".join(ls[start - 1:end])


def require(citation: str, text: str, needles: list[str], why: str) -> dict[str, Any]:
    compact = " ".join(text.split())
    missing = [n for n in needles if " ".join(n.split()) not in compact]
    if missing:
        raise SystemExit(f"{citation} missing expected text: {missing[0]}")
    return {"citation": citation, "verified": True, "why": why, "needles": needles}


def verify_redmcsb() -> list[dict[str, Any]]:
    checks: list[dict[str, Any]] = []
    checks.append(require(
        "DEFS.H:989-998",
        block("DEFS.H", 989, 998),
        [
            "unsigned char MapCount;",
            "unsigned int16_t InitialPartyLocation;",
            "Bits 11-10: Direction, Bits 9-5: Y, Bits 4-0: X",
        ],
        "DUNGEON_HEADER encodes the deterministic new-game party X/Y/direction field.",
    ))
    checks.append(require(
        "LOADSAVE.C:1940-1945",
        block("LOADSAVE.C", 1940, 1945),
        [
            "if (G0298_B_NewGame)",
            "G0306_i_PartyMapX = (AL1353_i_InitialPartyLocation = G0278_ps_DungeonHeader->InitialPartyLocation) & 0x001F;",
            "G0307_i_PartyMapY = (AL1353_i_InitialPartyLocation >>= 5) & 0x001F;",
            "G0308_i_PartyDirection = (AL1353_i_InitialPartyLocation >> 5) & 0x0003;",
            "G0309_i_PartyMapIndex = 0;",
        ],
        "New games derive party start state from the header and force map index 0.",
    ))
    checks.append(require(
        "DUNGEON.C:35-44",
        block("DUNGEON.C", 35, 44),
        [
            "G0233_ai_Graphic559_DirectionToStepEastCount",
            "0,    /* North */",
            "1,    /* East */",
            "G0234_ai_Graphic559_DirectionToStepNorthCount",
            "-1,  /* North */",
            "1,   /* West */",
        ],
        "Direction vectors define deterministic north/east/south/west coordinate deltas.",
    ))
    checks.append(require(
        "DUNGEON.C:1371-1391",
        block("DUNGEON.C", 1371, 1391),
        [
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "P0253_i_Direction += 1, P0253_i_Direction &= 3; /* Simulate turning right */",
            "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction]",
            "*P0257_pi_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[P0253_i_Direction]",
        ],
        "Relative movement is resolved by rotating the facing direction before adding source vectors.",
    ))
    checks.append(require(
        "CHAMPION.C:93-130",
        block("CHAMPION.C", 93, 130),
        [
            "void F0284_CHAMPION_SetPartyDirection",
            "L0834_i_Delta = P0600_i_Direction - G0308_i_PartyDirection",
            "G0308_i_PartyDirection = P0600_i_Direction;",
        ],
        "Turning updates champion cells/party things, then stores the new party direction.",
    ))
    checks.append(require(
        "CLIKMENU.C:256-347",
        block("CLIKMENU.C", 256, 347),
        [
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "L1116_i_SquareType == C00_ELEMENT_WALL",
            "L1117_B_MovementBlocked = M036_DOOR_STATE(AL1115_ui_Square);",
            "L1117_B_MovementBlocked != C1_DOOR_STATE_CLOSED_ONE_FOURTH",
            "MASK0x0004_FAKEWALL_OPEN",
            "MASK0x0001_FAKEWALL_IMAGINARY",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
            "G0311_i_ProjectileDisabledMovementTicks = 0;",
        ],
        "Party movement applies wall/door/fake-wall legality and movement cooldowns.",
    ))
    checks.append(require(
        "COMMAND.C:2150-2156",
        block("COMMAND.C", 2150, 2156),
        [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
            "goto T0380042;",
        ],
        "Command queue dispatch sends turn/step commands through the movement handlers before returning to the input wait loop.",
    ))
    checks.append(require(
        "CLIKMENU.C:156-173 and CLIKMENU.C:237-347",
        block("CLIKMENU.C", 156, 173) + "\n" + block("CLIKMENU.C", 237, 347),
        [
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0284_CHAMPION_SetPartyDirection",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        ],
        "Successful turns/steps set the stop-wait flag and mutate party direction/position before the next viewport draw.",
    ))
    checks.append(require(
        "GAMELOOP.C:90 and GAMELOOP.C:215-219",
        block("GAMELOOP.C", 90, 90) + "\n" + block("GAMELOOP.C", 215, 219),
        [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        ],
        "The main loop redraws the dungeon view from current party state, then processes input until a command sets StopWaitingForPlayerInput.",
    ))
    checks.append(require(
        "DUNVIEW.C:8318-8616 and DRAWVIEW.C:709-724",
        block("DUNVIEW.C", 8318, 8616) + "\n" + block("DRAWVIEW.C", 709, 724),
        [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
            "G0324_B_DrawViewportRequested = C1_TRUE",
        ],
        "Viewport redraw derives visible cells from updated direction/map coordinates and explicitly requests presentation.",
    ))
    checks.append(require(
        "MOVESENS.C:315-385",
        block("MOVESENS.C", 315, 385),
        [
            "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE",
            "P0558_i_SourceMapX",
            "P0560_i_DestinationMapX",
            "L0713_B_ThingLevitates",
            "L0724_B_DestinationIsTeleporterTarget",
        ],
        "Movement result calculation is centralized before state mutation and sensor routing.",
    ))
    checks.append(require(
        "MOVESENS.C:752-775",
        block("MOVESENS.C", 752, 775),
        [
            "L0725_B_PartySquare",
            "P0557_T_Thing == C0xFFFF_THING_PARTY",
            "G0308_i_PartyDirection == L0716_ui_Direction",
            "G0362_l_LastPartyMovementTime = G0313_ul_GameTime",
        ],
        "Party moves are special-cased and timestamped on successful movement.",
    ))
    checks.append(require(
        "GAMELOOP.C:150-155",
        block("GAMELOOP.C", 150, 155),
        [
            "G0310_i_DisabledMovementTicks--",
            "G0311_i_ProjectileDisabledMovementTicks--",
        ],
        "Movement/projection input cooldowns are decremented once per game-loop tick.",
    ))
    checks.append(require(
        "DUNVIEW.C:2962-3003",
        block("DUNVIEW.C", 2962, 3003),
        [
            "void F0098_DUNGEONVIEW_DrawFloorAndCeiling",
            "F0008_MAIN_ClearBytes(G0086_puc_Bitmap_ViewportBlackArea",
            "F0007_MAIN_CopyBytes(G0085_puc_Bitmap_Ceiling, G0296_puc_Bitmap_Viewport",
            "F0007_MAIN_CopyBytes(G0084_puc_Bitmap_Floor",
            "G0297_B_DrawFloorAndCeilingRequested = C0_FALSE;",
        ],
        "Viewport drawing starts from a cleared/filled floor+ceiling base.",
    ))
    checks.append(require(
        "DUNVIEW.C:3048-3110",
        block("DUNVIEW.C", 3048, 3110),
        [
            "void F0100_DUNGEONVIEW_DrawWallSetBitmap",
            "F0132_VIDEO_Blit(P0105_puc_Bitmap, G0296_puc_Bitmap_Viewport",
            "void F0102_DUNGEONVIEW_DrawDoorBitmap",
            "F0132_VIDEO_Blit(G0074_puc_Bitmap_Temporary, G0296_puc_Bitmap_Viewport",
            "void F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally",
        ],
        "Walls/doors are blitted into the same viewport bitmap, including flipped door frames.",
    ))
    checks.append(require(
        "DUNVIEW.C:4382-4474",
        block("DUNVIEW.C", 4382, 4474),
        [
            "void F0113_DUNGEONVIEW_DrawField",
            "F0635_(NULL, L2472_ai_XYZ, P2086_i_ZoneIndex",
            "C076_GRAPHIC_FIRST_FIELD + M728_NATIVE_BITMAP_RELATIVE_INDEX(P0135_puc_FieldAspect)",
            "F0133_VIDEO_BlitBoxFilledWithMaskedBitmap(L0119_puc_Bitmap, G0296_puc_Bitmap_Viewport",
            "G2073_C224_ViewportPixelWidth",
            "G2074_C136_ViewportHeight",
        ],
        "Field rows/aspects resolve through source zones and masked blits into the 224x136 viewport bitmap.",
    ))
    checks.append(require(
        "DUNVIEW.C:4547-5113",
        block("DUNVIEW.C", 4547, 5113),
        [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "AL0127_i_ThingType >= C05_THING_TYPE_WEAPON",
            "AL0127_i_ThingType <= C10_THING_TYPE_JUNK",
            "M011_CELL(P0141_T_Thing) == L0139_i_Cell",
            "F0791_DUNGEONVIEW_DrawBitmapXX(AL0128_puc_Bitmap, G0296_puc_Bitmap_Viewport",
            "if (L0144_B_DrawingGrabbableObject)",
        ],
        "Visible floor/alcove objects are filtered by view cell and drawn into the viewport bitmap.",
    ))
    checks.append(require(
        "DRAWVIEW.C:709-724",
        block("DRAWVIEW.C", 709, 724),
        [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "G0324_B_DrawViewportRequested = C1_TRUE",
            "M526_WaitVerticalBlank();",
        ],
        "Viewport presentation is a separate palette/vblank-gated presentation step.",
    ))
    checks.append(require(
        "DEFS.H:2575-2614 and DEFS.H:4215-4230",
        block("DEFS.H", 2575, 2614) + "\n" + block("DEFS.H", 4215, 4230),
        [
            "#define M597_VIEW_SQUARE_D4C",
            "#define C14_VIEW_SQUARE_D3L2",
            "#define C15_VIEW_SQUARE_D3R2",
            "#define C2500_ZONE_",
            "#define C2540_ZONE_ALCOVE_OBJECT",
            "#define C2900_ZONE_",
        ],
        "Viewport view-square ids and object/projectile zone families are stable source constants.",
    ))
    checks.append(require(
        "DUNVIEW.C:8446-8542",
        block("DUNVIEW.C", 8446, 8542),
        [
            "F0153_DUNGEON_GetRelativeSquareType(P0183_i_Direction, 3, -2",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 3, 0",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 2, 0",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 0",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
        ],
        "Viewport rows are traversed source-order far-to-near using relative coordinates from current party state.",
    ))
    return checks



def verify_original_dm1_header() -> dict[str, Any]:
    if not DM1_DUNGEON_DAT.is_file():
        raise SystemExit(f"missing required DM1 DUNGEON.DAT: {DM1_DUNGEON_DAT}")
    data = DM1_DUNGEON_DAT.read_bytes()
    if len(data) < 10:
        raise SystemExit(f"DM1 DUNGEON.DAT too short for header: {DM1_DUNGEON_DAT}")
    initial = int.from_bytes(data[8:10], "little")
    decoded = {
        "mapIndex": 0,
        "mapX": initial & 0x001F,
        "mapY": (initial >> 5) & 0x001F,
        "direction": (initial >> 10) & 0x0003,
    }
    expected = {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2}
    if decoded != expected:
        raise SystemExit(f"DM1 header start mismatch: decoded {decoded}, expected {expected}")
    return {
        "citation": "DM1 PC34 DUNGEON.DAT header byte offset 8, interpreted via DEFS.H:989-998 and LOADSAVE.C:1940-1945",
        "verified": True,
        "path": str(DM1_DUNGEON_DAT),
        "initialPartyLocationLE": f"0x{initial:04X}",
        "decoded": decoded,
        "why": "The canonical DM1 V1 asset starts the party at map 0, x=1, y=3, direction=2 (south).",
    }


def verify_route_evidence() -> list[dict[str, Any]]:
    if not ROUTE_EVIDENCE.is_file():
        raise SystemExit(f"missing required route evidence: {ROUTE_EVIDENCE}")
    doc = json.loads(ROUTE_EVIDENCE.read_text(encoding="utf-8"))
    snapshots = {row["capture"]: row for row in doc.get("snapshots", [])}
    expected = [
        ("01_ingame_start_latest", "start", {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2, "tick": 0}),
        ("02_ingame_turn_right_latest", "right", {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 3, "tick": 1}),
        ("03_ingame_move_forward_latest", "up", {"mapIndex": 0, "mapX": 0, "mapY": 3, "direction": 3, "tick": 2}),
    ]
    checks: list[dict[str, Any]] = []
    for capture, action, fields in expected:
        row = snapshots.get(capture)
        if row is None:
            raise SystemExit(f"route evidence missing capture {capture}")
        for key, value in fields.items():
            if row.get(key) != value:
                raise SystemExit(f"{capture} {key}={row.get(key)!r}, expected {value!r}")
        if row.get("action") != action or row.get("result") != 1:
            raise SystemExit(f"{capture} action/result mismatch: {row}")
        checks.append({
            "citation": f"verification-m11/capture-route-state/pass76_capture_route_state_probe.json:{capture}",
            "verified": True,
            "observed": {k: row[k] for k in ["action", "result", "tick", "mapIndex", "mapX", "mapY", "direction"]},
            "why": "Deterministic Firestaff route evidence matches the ReDMCSB-derived entry/turn/relative-step contract.",
        })
    return checks

def source_derived_golden_metadata() -> dict[str, Any]:
    """Reusable source/asset-derived facts for implementation lanes.

    This is not a runtime capture. Every value is either decoded from canonical
    DM1 DUNGEON.DAT or transcribed from the cited ReDMCSB source ranges checked
    by this verifier.
    """
    return {
        "label": "source_asset_derived_golden_metadata_not_runtime_capture",
        "citations": [
            "DEFS.H:2575-2614 view-square indexes",
            "DEFS.H:4215-4230 viewport/thing zone constants",
            "DUNGEON.C:35-44 direction step vectors",
            "DUNGEON.C:1371-1391 relative movement rotation",
            "CLIKMENU.C:256-347 movement blockers/cooldowns",
            "DUNVIEW.C:8318-8542 viewport far-to-near row traversal",
            "DUNVIEW.C:4929-5075 object zone derivation from C2500/C2548",
            "DUNVIEW.C:5683-5889 projectile zone derivation from C2900",
        ],
        "directions": [
            {"id": 0, "name": "NORTH", "stepEast": 0, "stepNorth": -1},
            {"id": 1, "name": "EAST", "stepEast": 1, "stepNorth": 0},
            {"id": 2, "name": "SOUTH", "stepEast": 0, "stepNorth": 1},
            {"id": 3, "name": "WEST", "stepEast": -1, "stepNorth": 0},
        ],
        "entry_state": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2, "directionName": "SOUTH"},
        "turn_step_front_samples": [
            {"snapshot": "start_south", "party": [0, 1, 3, 2], "left": [2, 4], "center": [1, 4], "right": [0, 4]},
            {"snapshot": "turn_right_west", "party": [0, 1, 3, 3], "left": [0, 4], "center": [0, 3], "right": [0, 2]},
            {"snapshot": "turn_left_east", "party": [0, 1, 3, 1], "left": [2, 2], "center": [2, 3], "right": [2, 4]},
            {"snapshot": "move_forward_west", "party": [0, 0, 3, 3], "left": [-1, 4], "center": [-1, 3], "right": [-1, 2]},
            {"snapshot": "blocked_forward_south_wall", "party": [0, 1, 3, 2], "left": [2, 4], "center": [1, 4], "right": [0, 4]},
        ],
        "movement_blockers": {
            "wall": "blocked when M034_SQUARE_TYPE(square) == C00_ELEMENT_WALL",
            "door": "blocked unless door state is open, one-fourth closed, or destroyed (states 0, 1, 5 pass)",
            "fakewall": "blocked unless MASK0x0004_FAKEWALL_OPEN or MASK0x0001_FAKEWALL_IMAGINARY is set",
            "group": "living party checks target group and blocks/reactions before state mutation",
            "successfulMoveCooldown": "G0310_i_DisabledMovementTicks = max champion movement ticks; projectile movement cooldown reset to 0",
        },
        "viewport_rows_far_to_near": [
            {"viewSquare": "M598_VIEW_SQUARE_D4L", "relativeForward": 4, "relativeRight": -1, "role": "far object lane"},
            {"viewSquare": "M599_VIEW_SQUARE_D4R", "relativeForward": 4, "relativeRight": 1, "role": "far object lane"},
            {"viewSquare": "M597_VIEW_SQUARE_D4C", "relativeForward": 4, "relativeRight": 0, "role": "far object lane"},
            {"viewSquare": "C14_VIEW_SQUARE_D3L2", "relativeForward": 3, "relativeRight": -2, "role": "far side wall"},
            {"viewSquare": "C15_VIEW_SQUARE_D3R2", "relativeForward": 3, "relativeRight": 2, "role": "far side wall"},
            {"viewSquare": "M601_VIEW_SQUARE_D3L", "relativeForward": 3, "relativeRight": -1, "role": "D3 left square"},
            {"viewSquare": "M602_VIEW_SQUARE_D3R", "relativeForward": 3, "relativeRight": 1, "role": "D3 right square"},
            {"viewSquare": "M600_VIEW_SQUARE_D3C", "relativeForward": 3, "relativeRight": 0, "role": "D3 center square"},
            {"viewSquare": "C09_VIEW_SQUARE_D2L2", "relativeForward": 2, "relativeRight": -2, "role": "D2 side wall"},
            {"viewSquare": "C10_VIEW_SQUARE_D2R2", "relativeForward": 2, "relativeRight": 2, "role": "D2 side wall"},
            {"viewSquare": "M604_VIEW_SQUARE_D2L", "relativeForward": 2, "relativeRight": -1, "role": "D2 left square"},
            {"viewSquare": "M605_VIEW_SQUARE_D2R", "relativeForward": 2, "relativeRight": 1, "role": "D2 right square"},
            {"viewSquare": "M603_VIEW_SQUARE_D2C", "relativeForward": 2, "relativeRight": 0, "role": "D2 center square"},
            {"viewSquare": "M607_VIEW_SQUARE_D1L", "relativeForward": 1, "relativeRight": -1, "role": "D1 left square"},
            {"viewSquare": "M608_VIEW_SQUARE_D1R", "relativeForward": 1, "relativeRight": 1, "role": "D1 right square"},
            {"viewSquare": "M606_VIEW_SQUARE_D1C", "relativeForward": 1, "relativeRight": 0, "role": "D1 center square"},
            {"viewSquare": "M610_VIEW_SQUARE_D0L", "relativeForward": 0, "relativeRight": -1, "role": "current-row left"},
            {"viewSquare": "M611_VIEW_SQUARE_D0R", "relativeForward": 0, "relativeRight": 1, "role": "current-row right"},
            {"viewSquare": "M609_VIEW_SQUARE_D0C", "relativeForward": 0, "relativeRight": 0, "role": "current square"},
        ],
        "zone_families": {
            "fieldWallBase": "C700/C701 floor+ceiling and C702+ wall field zones feed F0113/F0791 clipping",
            "objectBase": "C2500_ZONE_ + viewSquare*4 + viewCell, with MASK0x8000_SHIFT_OBJECTS_AND_CREATURES for floor objects/creatures",
            "alcoveObjectBase": "C2540/C2548 zone families for alcove objects",
            "projectileBase": "C2900_ZONE_ + viewSquare*4 + viewCell",
        },
    }


def verify_firestaff() -> list[dict[str, Any]]:
    files = {
        "memory_dungeon_dat_pc34_compat.c": ROOT / "memory_dungeon_dat_pc34_compat.c",
        "memory_champion_state_pc34_compat.c": ROOT / "memory_champion_state_pc34_compat.c",
        "memory_movement_pc34_compat.c": ROOT / "memory_movement_pc34_compat.c",
        "m11_game_view.c": ROOT / "m11_game_view.c",
        "tools/verify_dm1_v1_movement_source_lock.py": ROOT / "tools/verify_dm1_v1_movement_source_lock.py",
        "scripts/verify_dm1_v1_viewport_world_redmcsb_gate.py": ROOT / "scripts/verify_dm1_v1_viewport_world_redmcsb_gate.py",
        "probes/m11/firestaff_m11_turn_viewport_orientation_probe.c": ROOT / "probes/m11/firestaff_m11_turn_viewport_orientation_probe.c",
    }
    text = {name: path.read_text(encoding="utf-8", errors="replace") for name, path in files.items()}
    checks: list[dict[str, Any]] = []
    checks.append(require(
        "memory_dungeon_dat_pc34_compat.c:header reader",
        text["memory_dungeon_dat_pc34_compat.c"],
        [
            "read_u16_le(file, &state->header.rawMapDataByteCount)",
            "read_u8(file, &state->header.mapCount)",
            "read_u16_le(file, &state->header.initialPartyLocation)",
        ],
        "Firestaff reads the source-defined dungeon header fields, including InitialPartyLocation.",
    ))
    checks.append(require(
        "memory_champion_state_pc34_compat.c:initial party decode",
        text["memory_champion_state_pc34_compat.c"],
        [
            "dungeon->header.initialPartyLocation",
            "&dir, &py, &px",
        ],
        "Firestaff decodes initial party state from the same header field used by ReDMCSB.",
    ))
    checks.append(require(
        "memory_movement_pc34_compat.c:relative movement and blocking",
        text["memory_movement_pc34_compat.c"],
        [
            "case MOVE_FORWARD:  stepDir = direction",
            "case MOVE_RIGHT:    stepDir = (direction + 1) & 3",
            "case MOVE_BACKWARD: stepDir = (direction + 2) & 3",
            "case MOVE_LEFT:     stepDir = (direction + 3) & 3",
            "doorState != 0 && doorState != 1 && doorState != 5",
            "!(squareByte & 0x04) && !(squareByte & 0x01)",
        ],
        "Firestaff movement helper preserves ReDMCSB relative movement and door/fake-wall rules.",
    ))
    checks.append(require(
        "m11_game_view.c:viewport render surface/world draw hooks",
        text["m11_game_view.c"],
        [
            "M11_VIEWPORT_W",
            "M11_VIEWPORT_H",
            "M11_GameView_Draw",
            "M11_GameView_HandleInput",
        ],
        "The M11 renderer exposes the world/viewport path this source-lock is meant to guard.",
    ))
    checks.append(require(
        "tools/verify_dm1_v1_movement_source_lock.py:existing movement gate",
        text["tools/verify_dm1_v1_movement_source_lock.py"],
        [
            "COMMAND.C:2045-2156",
            "CLIKMENU.C:256-347",
            "MOVESENS.C:315-385",
            "CHAMPION.C:1180-1214",
        ],
        "Existing movement source-lock gate remains wired and covers command/cooldown details.",
    ))
    checks.append(require(
        "scripts/verify_dm1_v1_viewport_world_redmcsb_gate.py:existing viewport gate",
        text["scripts/verify_dm1_v1_viewport_world_redmcsb_gate.py"],
        [
            "redmcsb-floor-ceiling-base-copy-gate",
            "redmcsb-wall-door-blit-zones-gate",
            "redmcsb-f0115-thing-pass-order-gate",
        ],
        "Existing viewport source-lock gate remains wired for draw stack/world visual facts.",
    ))
    checks.append(require(
        "probes/m11/firestaff_m11_turn_viewport_orientation_probe.c:post-command redraw gate",
        text["probes/m11/firestaff_m11_turn_viewport_orientation_probe.c"],
        [
            "COMMAND.C:2150-2156 dispatches turn/step commands",
            "CLIKMENU.C:156-173/237-347 sets StopWaitingForPlayerInput",
            "rows[0].result != M11_GAME_INPUT_REDRAW",
            "rows[3].result != M11_GAME_INPUT_REDRAW",
            "move_forward_west",
            "blocked_forward_south_wall",
            "rows[2].mapX != 0 || rows[2].mapY != 3",
            "rows[4].mapX != 1 || rows[4].mapY != 3",
        ],
        "Firestaff gates that successful turn and step inputs request redraw and resample viewport cells from post-command party state.",
    ))
    return checks


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    args = parser.parse_args()
    result = {
        "status": "PASS",
        "redmcsb_source_root": str(REDMCSB_SOURCE),
        "scope": "DM1 V1 entry + movement + viewport source-locked evidence",
        "redmcsb_checks": verify_redmcsb(),
        "original_dm1_asset_check": verify_original_dm1_header(),
        "firestaff_checks": verify_firestaff(),
        "route_evidence_checks": verify_route_evidence(),
        "golden_metadata": source_derived_golden_metadata(),
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"PASS dm1_v1_entry_movement_viewport_source_lock wrote {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
