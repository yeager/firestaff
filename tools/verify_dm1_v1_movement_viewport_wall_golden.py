#!/usr/bin/env python3
"""Build/check broad DM1 V1 movement + viewport + wall golden evidence.

This is a consolidated source-first gate for implementation lanes that need
movement/orientation/viewport/wall facts without guessing.  Runtime capture is
not produced here; instead this verifier locks ReDMCSB source facts, the
canonical DM1 DUNGEON.DAT entry header, and the existing pass127 Firestaff
runtime probe into one reusable JSON artifact.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB_SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DM1_DUNGEON_DAT = Path(
    "~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT"
).expanduser()
PASS127 = ROOT / "parity-evidence/verification/pass127_turn_viewport_orientation_probe.json"
DEFAULT_OUT = ROOT / "parity-evidence/verification/dm1_v1_movement_viewport_wall_golden.json"

DIRS = {
    0: {"name": "NORTH", "stepEast": 0, "stepNorth": -1},
    1: {"name": "EAST", "stepEast": 1, "stepNorth": 0},
    2: {"name": "SOUTH", "stepEast": 0, "stepNorth": 1},
    3: {"name": "WEST", "stepEast": -1, "stepNorth": 0},
}
ELEMENTS = {
    0: "Wall", 1: "Corridor", 2: "Pit", 3: "Stairs", 4: "Door", 5: "Teleporter", 6: "FakeWall", 7: "Invalid/asset-high-bit",
}
VIEWPORT_ORDER = [
    ("D4L", 4, -1), ("D4R", 4, 1), ("D4C", 4, 0),
    ("D3L2", 3, -2), ("D3R2", 3, 2), ("D3L", 3, -1), ("D3R", 3, 1), ("D3C", 3, 0),
    ("D2L2", 2, -2), ("D2R2", 2, 2), ("D2L", 2, -1), ("D2R", 2, 1), ("D2C", 2, 0),
    ("D1L", 1, -1), ("D1R", 1, 1), ("D1C", 1, 0),
    ("D0L", 0, -1), ("D0R", 0, 1), ("D0C", 0, 0),
]


def lines(path: Path) -> list[str]:
    if not path.is_file():
        raise SystemExit(f"missing required file: {path}")
    return path.read_text(encoding="latin-1", errors="replace").splitlines()


def block(rel: str, start: int, end: int) -> str:
    ls = lines(REDMCSB_SOURCE / rel)
    return "\n".join(ls[start - 1:end])


def require(citation: str, text: str, needles: list[str], why: str) -> dict[str, Any]:
    compact = " ".join(text.split())
    for needle in needles:
        if " ".join(needle.split()) not in compact:
            raise SystemExit(f"{citation} missing expected source text: {needle}")
    return {"citation": citation, "verified": True, "why": why, "needles": needles}


def verify_redmcsb() -> list[dict[str, Any]]:
    checks: list[dict[str, Any]] = []
    checks.append(require(
        "DEFS.H:989-1013",
        block("DEFS.H", 989, 1013),
        [
            "unsigned int16_t InitialPartyLocation;",
            "Bits 11-10: Direction, Bits 9-5: Y, Bits 4-0: X",
            "#define M034_SQUARE_TYPE(square)   ((square) >> 5)",
            "#define C00_ELEMENT_WALL          0",
            "#define C04_ELEMENT_DOOR          4",
            "#define C06_ELEMENT_FAKEWALL      6",
        ],
        "Header start bits and square element decoding are the root asset contracts.",
    ))
    checks.append(require(
        "LOADSAVE.C:1940-1945",
        block("LOADSAVE.C", 1940, 1945),
        [
            "G0306_i_PartyMapX = (AL1353_i_InitialPartyLocation = G0278_ps_DungeonHeader->InitialPartyLocation) & 0x001F;",
            "G0307_i_PartyMapY = (AL1353_i_InitialPartyLocation >>= 5) & 0x001F;",
            "G0308_i_PartyDirection = (AL1353_i_InitialPartyLocation >> 5) & 0x0003;",
            "G0309_i_PartyMapIndex = 0;",
        ],
        "New-game entry state is decoded directly from DUNGEON_HEADER.InitialPartyLocation.",
    ))
    checks.append(require(
        "DUNGEON.C:35-44 and DUNGEON.C:1371-1391",
        block("DUNGEON.C", 35, 44) + "\n" + block("DUNGEON.C", 1371, 1391),
        [
            "G0233_ai_Graphic559_DirectionToStepEastCount",
            "G0234_ai_Graphic559_DirectionToStepNorthCount",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "P0253_i_Direction += 1, P0253_i_Direction &= 3; /* Simulate turning right */",
            "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction]",
        ],
        "Relative forward/right movement rotates facing, then applies source direction vectors.",
    ))
    checks.append(require(
        "DUNGEON.C:1435-1499",
        block("DUNGEON.C", 1435, 1499),
        [
            "If processing goes below this line then either P0258_i_MapX or P0259_i_MapY is out of the map bounds and the returned square type will be C00_ELEMENT_WALL",
            "return M035_SQUARE(C00_ELEMENT_WALL, 0);",
            "F0152_DUNGEON_GetRelativeSquare",
            "F0153_DUNGEON_GetRelativeSquareType",
        ],
        "Viewport/off-map probes source-lock as wall squares, which matters at the DM1 entrance edge.",
    ))
    checks.append(require(
        "COMMAND.C:1513-1660 and COMMAND.C:1744-1792",
        block("COMMAND.C", 1513, 1660) + "\n" + block("COMMAND.C", 1744, 1792),
        [
            "if ((L1108_i_CommandQueueIndex = G0434_i_CommandQueueLastIndex + 2) > M529_COMMAND_QUEUE_SIZE)",
            "if (G2153_i_QueuedCommandsCount >= L2287_i_MaximumRegularCommandsInQueue)",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command;",
            "G0432_as_CommandQueue[L1108_i_CommandQueueIndex].X = P0725_i_X;",
            "G0432_as_CommandQueue[L1108_i_CommandQueueIndex].Y = P0726_i_Y;",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
        ],
        "Mouse/key input is normalized into the circular command queue before game-loop dispatch.",
    ))
    checks.append(require(
        "COMMAND.C:2045-2156",
        block("COMMAND.C", 2045, 2156),
        [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G0310_i_DisabledMovementTicks || (G0311_i_ProjectileDisabledMovementTicks",
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "L1162_i_CommandY = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Y;",
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "Game-loop command dequeue gates movement cooldowns, preserves X/Y payload, and dispatches turns/moves.",
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
            "F0175_GROUP_GetThing",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        ],
        "Accepted movement commands enter the wall/door/fakewall/group legality gate before cooldown/state changes.",
    ))
    checks.append(require(
        "MOVESENS.C:315-385 and MOVESENS.C:752-775",
        block("MOVESENS.C", 315, 385) + "\n" + block("MOVESENS.C", 752, 775),
        [
            "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE",
            "P0558_i_SourceMapX",
            "P0560_i_DestinationMapX",
            "P0557_T_Thing == C0xFFFF_THING_PARTY",
            "G0362_l_LastPartyMovementTime = G0313_ul_GameTime",
        ],
        "Movement result routing is centralized and party moves are timestamped on success.",
    ))
    checks.append(require(
        "GAMELOOP.C:90 and GAMELOOP.C:150-155 and GAMELOOP.C:215-219",
        block("GAMELOOP.C", 90, 90) + "\n" + block("GAMELOOP.C", 150, 155) + "\n" + block("GAMELOOP.C", 215, 219),
        [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "G0310_i_DisabledMovementTicks--",
            "G0311_i_ProjectileDisabledMovementTicks--",
            "F0380_COMMAND_ProcessQueue_CPSC();",
        ],
        "Viewport redraw reads current party state; movement cooldowns tick in the game loop.",
    ))
    checks.append(require(
        "DUNVIEW.C:2962-3110 and DRAWVIEW.C:709-724",
        block("DUNVIEW.C", 2962, 3110) + "\n" + block("DRAWVIEW.C", 709, 724),
        [
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
            "F0100_DUNGEONVIEW_DrawWallSetBitmap",
            "F0102_DUNGEONVIEW_DrawDoorBitmap",
            "F0097_DUNGEONVIEW_DrawViewport",
            "G0324_B_DrawViewportRequested = C1_TRUE",
        ],
        "The viewport is a 224x136 backing bitmap filled with floor/ceiling then walls/doors and presented separately.",
    ))
    checks.append(require(
        "DUNVIEW.C:6253-8164",
        block("DUNVIEW.C", 6253, 8164),
        [
            "STATICFUNCTION void F0116_DUNGEONVIEW_DrawSquareD3L",
            "STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C",
            "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C",
            "STATICFUNCTION void F0127_DUNGEONVIEW_DrawSquareD0C",
            "case C00_ELEMENT_WALL:",
            "case C16_ELEMENT_DOOR_SIDE:",
            "case C17_ELEMENT_DOOR_FRONT:",
        ],
        "Representative D3/D2/D1/D0 draw functions branch on wall/door/corridor square aspects.",
    ))
    checks.append(require(
        "DUNVIEW.C:8318-8542",
        block("DUNVIEW.C", 8318, 8542),
        [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "F0153_DUNGEON_GetRelativeSquareType(P0183_i_Direction, 3, -2",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1",
            "F0116_DUNGEONVIEW_DrawSquareD3L",
            "F0121_DUNGEONVIEW_DrawSquareD2C",
            "F0124_DUNGEONVIEW_DrawSquareD1C",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
        ],
        "Viewport traversal is source-ordered from far D4/D3 rows through near D0/current square.",
    ))
    return checks


def decode_dm1_entry() -> dict[str, Any]:
    data = DM1_DUNGEON_DAT.read_bytes()
    initial = int.from_bytes(data[8:10], "little")
    decoded = {"mapIndex": 0, "mapX": initial & 31, "mapY": (initial >> 5) & 31, "direction": (initial >> 10) & 3}
    expected = {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2}
    if decoded != expected:
        raise SystemExit(f"DM1 entry mismatch: {decoded} != {expected}")
    return {
        "citation": "DM1 canonical DUNGEON.DAT offset 8 + DEFS.H:989-998 + LOADSAVE.C:1940-1945",
        "path": str(DM1_DUNGEON_DAT),
        "initialPartyLocationLE": f"0x{initial:04X}",
        "decoded": decoded,
        "verified": True,
    }


def rel_coord(direction: int, x: int, y: int, forward: int, right: int) -> tuple[int, int]:
    d = direction
    if right:
        d = (d + (1 if right > 0 else 3)) & 3
        for _ in range(abs(right)):
            x += DIRS[d]["stepEast"]
            y += DIRS[d]["stepNorth"]
    d = direction
    if forward < 0:
        d = (d + 2) & 3
    for _ in range(abs(forward)):
        x += DIRS[d]["stepEast"]
        y += DIRS[d]["stepNorth"]
    return x, y


def load_pass127() -> dict[str, Any]:
    doc = json.loads(PASS127.read_text(encoding="utf-8"))
    if doc.get("schema") != "pass127_turn_viewport_orientation_probe.v4":
        raise SystemExit(f"unexpected pass127 schema: {doc.get(schema)!r}")
    expected_order = ",".join(name for name, _, _ in VIEWPORT_ORDER)
    if doc.get("viewportRowOrder") != expected_order:
        raise SystemExit("pass127 viewport row order mismatch")
    return doc


def summarize_cell(cell: dict[str, Any]) -> dict[str, Any]:
    typ = int(cell.get("elementType", -1))
    return {
        "mapX": cell.get("mapX"),
        "mapY": cell.get("mapY"),
        "valid": cell.get("valid", 1),
        "squareHex": f"0x{int(cell.get('square', 0)) & 0xFF:02X}",
        "elementType": typ,
        "elementName": ELEMENTS.get(typ, "Unknown"),
        "door": cell.get("door", 0),
        "pit": cell.get("pit", 0),
        "teleporter": cell.get("teleporter", 0),
    }


def verify_snapshot_geometry(snapshot: dict[str, Any]) -> dict[str, Any]:
    x, y, direction = int(snapshot["mapX"]), int(snapshot["mapY"]), int(snapshot["direction"])
    rows = snapshot.get("viewportRows", [])
    if len(rows) != len(VIEWPORT_ORDER):
        raise SystemExit(f"{snapshot[name]} row count mismatch")
    for row, (name, forward, right) in zip(rows, VIEWPORT_ORDER):
        if row.get("row") != name:
            raise SystemExit(f"{snapshot[name]} expected row {name}, got {row.get(row)}")
        exp_x, exp_y = rel_coord(direction, x, y, forward, right)
        if (row.get("mapX"), row.get("mapY")) != (exp_x, exp_y):
            raise SystemExit(f"{snapshot[name]} {name} coord mismatch: {row}")
    return {"name": snapshot["name"], "verifiedRowGeometry": True}


def golden_from_pass127() -> dict[str, Any]:
    doc = load_pass127()
    snapshots = {row["name"]: row for row in doc["snapshots"]}
    required = ["start_south", "turn_right_west", "move_forward_west", "turn_left_east", "blocked_forward_south_wall"]
    missing = [name for name in required if name not in snapshots]
    if missing:
        raise SystemExit(f"pass127 missing snapshots: {missing}")

    expected_states = {
        "start_south": (0, 1, 3, 2),
        "turn_right_west": (0, 1, 3, 3),
        "move_forward_west": (0, 0, 3, 3),
        "turn_left_east": (0, 1, 3, 1),
        "blocked_forward_south_wall": (0, 1, 3, 2),
    }
    geometry_checks = []
    for name, state in expected_states.items():
        s = snapshots[name]
        observed = (s["mapIndex"], s["mapX"], s["mapY"], s["direction"])
        if observed != state:
            raise SystemExit(f"{name} state mismatch: {observed} != {state}")
        geometry_checks.append(verify_snapshot_geometry(s))

    start = snapshots["start_south"]
    right = snapshots["turn_right_west"]
    moved = snapshots["move_forward_west"]
    left = snapshots["turn_left_east"]
    blocked = snapshots["blocked_forward_south_wall"]

    assertions = [
        ("start_south front-center is blocking wall", start["front"][1]["elementType"] == 0),
        ("start_south front-left is stairs entry side sample", start["front"][0]["elementType"] == 3),
        ("start_south front-right is corridor side sample", start["front"][2]["elementType"] == 1),
        ("turn_right_west front-center fakewall/teleporter sample", right["front"][1]["elementType"] == 6 and right["front"][1]["teleporter"] == 1),
        ("turn_right_west front-right wall with door thing sample", right["front"][2]["elementType"] == 0 and right["front"][2]["door"] == 1),
        ("move_forward_west moves to x=0 and sees off-map D1C as invalid wall", moved["mapX"] == 0 and moved["viewportRows"][15]["valid"] == 0 and moved["viewportRows"][15]["elementType"] == 0),
        ("turn_left_east has D1C front wall blocker", left["viewportRows"][15]["elementType"] == 0),
        ("blocked_forward_south preserves party state after wall collision", (blocked["mapX"], blocked["mapY"], blocked["direction"]) == (1, 3, 2)),
    ]
    for label, ok in assertions:
        if not ok:
            raise SystemExit(f"golden assertion failed: {label}")

    cases = []
    for name in required:
        s = snapshots[name]
        cases.append({
            "name": name,
            "party": {
                "mapIndex": s["mapIndex"], "mapX": s["mapX"], "mapY": s["mapY"],
                "direction": s["direction"], "directionName": DIRS[s["direction"]]["name"], "tick": s["tick"],
            },
            "frontLeftCenterRight": [summarize_cell(c) for c in s["front"]],
            "viewportRows": [{"row": r["row"], **summarize_cell(r)} for r in s["viewportRows"]],
            "wallVisibilityNotes": [
                "Rows are sampled in ReDMCSB far-to-near order; near opaque wall/door blits overwrite farther bitmap content.",
                "valid=0 rows are off-map and source-lock as wall via DUNGEON.C:1435-1499.",
            ],
        })

    return {
        "citation": str(PASS127),
        "schema": doc["schema"],
        "verifiedGeometry": geometry_checks,
        "representativeCases": cases,
        "assertions": [label for label, _ in assertions],
    }


def source_asset_metadata() -> dict[str, Any]:
    return {
        "limitations": [
            "This gate reuses existing pass127 runtime probe output; it does not drive DOSBox/original capture itself.",
            "Wall occlusion is represented as source draw-order + representative wall/door rows, not pixel-diffed original screenshots.",
        ],
        "directions": [{"id": k, **v} for k, v in DIRS.items()],
        "commandPipeline": {
            "inputNormalization": "COMMAND.C:1513-1660 and 1744-1792 append mouse/key commands to G0432_as_CommandQueue with X/Y payloads.",
            "gameLoopDispatch": "GAMELOOP.C:215 calls F0380_COMMAND_ProcessQueue_CPSC; COMMAND.C:2045-2156 dequeues, applies disabled-movement gates, then dispatches turns/moves.",
            "movementLegality": "CLIKMENU.C:256-347 applies wall/door/fakewall/group blockers before movement commits.",
        },
        "movementCommands": {
            "turnLeft": "COMMAND.C dispatches F0365; direction=(direction+3)&3; no coordinate change",
            "turnRight": "COMMAND.C dispatches F0365; direction=(direction+1)&3; no coordinate change",
            "moveForward": "COMMAND.C dispatches F0366; step along current direction after CLIKMENU legality checks",
            "moveRight/moveLeft/moveBackward": "COMMAND.C dispatches F0366; rotate relative step direction then apply DUNGEON.C vectors",
        },
        "movementBlockers": {
            "wall": "M034_SQUARE_TYPE(square)==C00_ELEMENT_WALL blocks party movement",
            "door": "C04 door blocks unless M036_DOOR_STATE is open, one-fourth closed, or destroyed",
            "fakewall": "C06 fakewall blocks unless MASK0x0004_FAKEWALL_OPEN or MASK0x0001_FAKEWALL_IMAGINARY",
            "group": "F0175_GROUP_GetThing target group blocks unless champion collision/damage path clears it",
            "offMap": "F0151/F0152 return C00 wall for out-of-bounds viewport/movement probes",
        },
        "viewportRowOrder": [{"row": n, "forward": f, "right": r} for n, f, r in VIEWPORT_ORDER],
        "viewportSurface": {"width": 224, "height": 136, "source": "DUNVIEW/DRAWVIEW source constants and pass127 probe scope"},
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    args = parser.parse_args()
    result = {
        "status": "PASS",
        "scope": "DM1 V1 movement/orientation/viewport/wall source+asset+runtime-golden evidence",
        "redmcsb_source_root": str(REDMCSB_SOURCE),
        "redmcsb_checks": verify_redmcsb(),
        "dm1_entry_asset_check": decode_dm1_entry(),
        "golden": golden_from_pass127(),
        "source_asset_metadata": source_asset_metadata(),
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"PASS dm1_v1_movement_viewport_wall_golden wrote {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
