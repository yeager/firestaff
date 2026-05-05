#!/usr/bin/env python3
"""Source-lock DM1 V1 viewport visible cells to dungeon square/collision state.

This evidence gate is intentionally narrow: it proves that ReDMCSB does not draw
viewport cells from a structural/depth-only template.  The same relative square
coordinates are resolved through DUNGEON.C map data, promoted into square aspects
for DUNVIEW, and checked by the movement/collision path before party motion.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

DEFAULT_SOURCE = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
).expanduser()

CHECKS: list[dict[str, Any]] = [
    {
        "id": "relative-cell-coordinates-shared-by-viewport-and-movement",
        "file": "DUNGEON.C",
        "function": "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "range": "1371-1418",
        "needles": [
            "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(",
            "G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount",
            "G0234_ai_Graphic559_DirectionToStepNorthCount[P0253_i_Direction] * P0254_i_StepsForwardCount",
            "P0253_i_Direction += 1, P0253_i_Direction &= 3; /* Simulate turning right */",
            "G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0255_i_StepsRightCount",
            "G0234_ai_Graphic559_DirectionToStepNorthCount[P0253_i_Direction] * P0255_i_StepsRightCount",
        ],
        "why": "Viewport cells and movement targets share F0150 relative-coordinate math, so visual cells must be resolved in the same map coordinate frame as collision.",
    },
    {
        "id": "square-byte-query-is-real-map-data-with-bounds-wall",
        "file": "DUNGEON.C",
        "function": "F0151_DUNGEON_GetSquare",
        "range": "1423-1455",
        "needles": [
            "unsigned char F0151_DUNGEON_GetSquare(",
            "AL0249_B_IsMapYInBounds = (P0259_i_MapY >= 0) && (P0259_i_MapY < G0274_i_CurrentMapHeight);",
            "AL0248_B_IsMapXInBounds = (P0258_i_MapX >= 0) && (P0258_i_MapX < G0273_i_CurrentMapWidth)",
            "return G0271_ppuc_CurrentMapData[P0258_i_MapX][P0259_i_MapY];",
            "returned square type will be C00_ELEMENT_WALL",
            "return M035_SQUARE(C00_ELEMENT_WALL, MASK0x0004_WALL_EAST_RANDOM_ORNAMENT_ALLOWED);",
        ],
        "why": "The authoritative square byte comes from G0271_ppuc_CurrentMapData; out-of-bounds viewport/collision probes degrade to wall squares.",
    },
    {
        "id": "viewport-visible-cells-use-relative-map-coordinates",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "range": "8464-8544",
        "needles": [
            "L0224_i_MapX = P0184_i_MapX;",
            "L0225_i_MapY = P0185_i_MapY;",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1, &L0224_i_MapX, &L0225_i_MapY);",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(F0162_DUNGEON_GetSquareFirstObject(L0224_i_MapX, L0225_i_MapY), P0183_i_Direction, L0224_i_MapX, L0225_i_MapY, M598_VIEW_SQUARE_D4L",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 3, 0, &L0224_i_MapX, &L0225_i_MapY);",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 0, &L0224_i_MapX, &L0225_i_MapY);",
            "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
        ],
        "why": "F0128 enumerates visible cells from party coordinates through F0150, then passes concrete map X/Y into each square draw routine.",
    },
    {
        "id": "square-aspect-promotes-square-byte-to-visible-element-state",
        "file": "DUNGEON.C",
        "function": "F0172_DUNGEON_SetSquareAspect",
        "range": "2518-2720",
        "needles": [
            "L0314_T_Thing = F0161_DUNGEON_GetSquareFirstThing(P0319_i_MapX, P0320_i_MapY);",
            "AL0307_uc_Square = F0151_DUNGEON_GetSquare(P0319_i_MapX, P0320_i_MapY);",
            "switch (P0317_pui_SquareAspect[C0_ELEMENT] = M034_SQUARE_TYPE(AL0307_uc_Square))",
            "case C00_ELEMENT_WALL:",
            "case C02_ELEMENT_PIT:",
            "case C06_ELEMENT_FAKEWALL:",
            "case C04_ELEMENT_DOOR:",
            "P0317_pui_SquareAspect[M556_DOOR_STATE] = M036_DOOR_STATE(AL0307_uc_Square);",
            "P0317_pui_SquareAspect[M557_DOOR_THING_INDEX] = M013_INDEX(F0161_DUNGEON_GetSquareFirstThing(P0319_i_MapX, P0320_i_MapY));",
        ],
        "why": "DUNVIEW draw routines consume square aspects that are built from the same square byte used for walls, pits, fakewalls, doors, and door state.",
    },
    {
        "id": "movement-collision-target-uses-same-relative-square-byte",
        "file": "CLIKMENU.C",
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "range": "264-332",
        "needles": [
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection, G0465_ai_Graphic561_MovementArrowToStepForwardCount[AL1118_ui_MovementArrowIndex], G0466_ai_Graphic561_MovementArrowToStepRightCount[AL1118_ui_MovementArrowIndex], &L1121_i_MapX, &L1122_i_MapY);",
            "L1116_i_SquareType = M034_SQUARE_TYPE(AL1115_ui_Square = F0151_DUNGEON_GetSquare(L1121_i_MapX, L1122_i_MapY));",
            "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
            "if (L1116_i_SquareType == C04_ELEMENT_DOOR)",
            "L1117_B_MovementBlocked = (L1117_B_MovementBlocked != C0_DOOR_STATE_OPEN) && (L1117_B_MovementBlocked != C1_DOOR_STATE_CLOSED_ONE_FOURTH) && (L1117_B_MovementBlocked != C5_DOOR_STATE_DESTROYED);",
            "if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)",
            "L1117_B_MovementBlocked = (!M007_GET(AL1115_ui_Square, MASK0x0004_FAKEWALL_OPEN) && !M007_GET(AL1115_ui_Square, MASK0x0001_FAKEWALL_IMAGINARY));",
            "F0175_GROUP_GetThing(L1121_i_MapX, L1122_i_MapY) != C0xFFFE_THING_ENDOFLIST",
            "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
        ],
        "why": "Party movement blocks/passability are computed from the same F0150/F0151 target square used by viewport visible-cell coordinates.",
    },
    {
        "id": "move-result-chain-rechecks-destination-square-state",
        "file": "MOVESENS.C",
        "function": "F0267_MOVE_GetMoveResult_CPSCE",
        "range": "474-540",
        "needles": [
            "AL0708_i_DestinationSquare = G0271_ppuc_CurrentMapData[P0560_i_DestinationMapX][P0561_i_DestinationMapY];",
            "AL0709_i_DestinationSquareType = M034_SQUARE_TYPE(AL0708_i_DestinationSquare)",
            "if (!M007_GET(AL0708_i_DestinationSquare, MASK0x0008_TELEPORTER_OPEN))",
            "F0157_DUNGEON_GetSquareFirstThingData(P0560_i_DestinationMapX, P0561_i_DestinationMapY);",
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
            "if ((AL0709_i_DestinationSquareType == C02_ELEMENT_PIT) && !L0713_B_ThingLevitates && M007_GET(AL0708_i_DestinationSquare, MASK0x0008_PIT_OPEN) && !M007_GET(AL0708_i_DestinationSquare, MASK0x0001_PIT_IMAGINARY))",
        ],
        "why": "After collision permits a move, MOVESENS follows teleporter/pit chains by rereading destination square bytes and mutating party X/Y from that state.",
    },
    {
        "id": "viewport-present-happens-after-map-backed-draw",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "range": "709-900",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ],
        "why": "The final viewport blit presents the buffer only after DUNVIEW has resolved and drawn map-backed visible cells.",
    },
]


def slice_text(path: Path, line_range: str) -> str:
    start_s, end_s = line_range.split("-", 1)
    start = int(start_s)
    end = int(end_s)
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(lines[start - 1 : end])


def verify(source: Path) -> dict[str, Any]:
    results = []
    ok = True
    for check in CHECKS:
        path = source / check["file"]
        if not path.exists():
            missing = [f"missing source file {path}"]
        else:
            text = slice_text(path, check["range"])
            missing = [needle for needle in check["needles"] if needle not in text]
        passed = not missing
        ok = ok and passed
        results.append({
            "id": check["id"],
            "passed": passed,
            "source": "{}:{}".format(check["file"], check["range"]),
            "function": check["function"],
            "why": check["why"],
            "missing": missing,
        })
    return {
        "gate": "dm1_v1_viewport_square_collision_source_lock",
        "source_root": str(source),
        "passed": ok,
        "checks": results,
        "implication": (
            "Firestaff viewport parity must route every visible square through real "
            "dungeon square data (or a probe equivalent to F0150/F0151/F0172) and "
            "must agree with movement collision checks for walls, doors, fakewalls, "
            "groups, pits, and teleporters."
        ),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    payload = verify(args.source)
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for result in payload["checks"]:
            status = "PASS" if result["passed"] else "FAIL"
            print("{} {} {} {}".format(status, result["id"], result["source"], result["function"]))
            print("  {}".format(result["why"]))
            for needle in result["missing"]:
                print(f"  missing: {needle}")
        print("IMPLICATION {}".format(payload["implication"]))
    return 0 if payload["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
