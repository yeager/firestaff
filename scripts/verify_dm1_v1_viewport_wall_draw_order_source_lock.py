#!/usr/bin/env python3
"""Source-lock DM1 V1 viewport wall zones and draw order against ReDMCSB.

This gate is intentionally evidence-only. It proves the current Firestaff viewport
wall work must model ReDMCSB as an ordered square replay into zones, not as a
single primitive-batching pass sorted only by depth.
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
        "id": "drawview-presents-after-dungeonview-buffer",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "range": "709-900",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ],
        "why": "Presentation is a final blit of G0296 after DUNVIEW has replayed walls/objects into the viewport buffer.",
    },
    {
        "id": "floor-ceiling-base-before-wall-replay",
        "file": "DUNVIEW.C",
        "function": "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "range": "2962-3003",
        "needles": [
            "void F0098_DUNGEONVIEW_DrawFloorAndCeiling(",
            "F0008_MAIN_ClearBytes(M772_CAST_PC(G0086_puc_Bitmap_ViewportBlackArea)",
            "F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);",
            "F0674_F0128_sub(G2108_Floor, G0087_puc_Bitmap_ViewportFloorArea);",
            "G0297_B_DrawFloorAndCeilingRequested = C0_FALSE;",
        ],
        "why": "The renderer starts from a clean floor/ceiling buffer before replaying visible wall zones and contents.",
    },
    {
        "id": "f0128-far-to-near-square-replay",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "range": "8318-8618",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G3010_i_WallSet_Wall_D3L2, C702_ZONE_WALL_D3L2);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G3072_i_WallSet_Wall_D3R2, C703_ZONE_WALL_D3R2);",
            "F0676_DrawD3L2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0677_DrawD3R2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0678_DrawD2L2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0679_DrawD2R2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "ordered_needles": [
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(F0162_DUNGEON_GetSquareFirstObject(L0224_i_MapX, L0225_i_MapY), P0183_i_Direction, L0224_i_MapX, L0225_i_MapY, M598_VIEW_SQUARE_D4L",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(F0162_DUNGEON_GetSquareFirstObject(L0224_i_MapX, L0225_i_MapY), P0183_i_Direction, L0224_i_MapX, L0225_i_MapY, M599_VIEW_SQUARE_D4R",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(F0162_DUNGEON_GetSquareFirstObject(L0224_i_MapX, L0225_i_MapY), P0183_i_Direction, L0224_i_MapX, L0225_i_MapY, M597_VIEW_SQUARE_D4C",
            "F0676_DrawD3L2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0677_DrawD3R2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0117_DUNGEONVIEW_DrawSquareD3R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0678_DrawD2L2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0679_DrawD2R2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0119_DUNGEONVIEW_DrawSquareD2L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0122_DUNGEONVIEW_DrawSquareD1L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0123_DUNGEONVIEW_DrawSquareD1R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0126_DUNGEONVIEW_DrawSquareD0R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "why": "DUNVIEW replays cells from far D4 through D0 before presentation; PC34/I34E D3L2/R2 and D2L2/R2 lanes are explicitly kept in the audited order.",
    },
    {
        "id": "d3-side-wall-occludes-by-early-return",
        "file": "DUNVIEW.C",
        "function": "F0116_DUNGEONVIEW_DrawSquareD3L / F0117_DUNGEONVIEW_DrawSquareD3R / F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
        "range": "6400-6835",
        "needles": [
            "case C00_ELEMENT_WALL:",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);",
            "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0201_ai_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL], M575_VIEW_WALL_D3L_RIGHT);",
            "if (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0201_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M577_VIEW_WALL_D3L_FRONT)) {",
            "L0200_i_Order = C0x0000_CELL_ORDER_ALCOVE;",
            "return;",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C12_WALL_D3R], C706_ZONE_WALL_D3R);",
            "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C);",
        ],
        "why": "A wall square draws its wall zone and normally returns before open-cell content; only alcove wall ornaments intentionally replay contained items.",
    },
    {
        "id": "f0115-ordered-cell-pass-not-depth-sort",
        "file": "DUNVIEW.C",
        "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "range": "4547-4910",
        "needles": [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
            "P0146_ui_OrderedViewCellOrdinals",
            "P0146_ui_OrderedViewCellOrdinals >>= 4;",
            "L0139_i_Cell = M021_NORMALIZE(AL0126_i_ViewCell + P0142_i_Direction);",
            "P0141_T_Thing = L0146_T_FirstThingToDraw;",
            "if ((AL0127_i_ThingType = M012_TYPE(P0141_T_Thing)) == C04_THING_TYPE_GROUP)",
        ],
        "why": "Within each visible square, F0115 consumes encoded cell order nibbles and rescans the thing list per cell; primitive batching would lose this source order.",
    },
    {
        "id": "door-front-two-pass-near-side-replay",
        "file": "DUNVIEW.C",
        "function": "F0116/F0117/F0118 door-front branches",
        "range": "6428-6816",
        "needles": [
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "F0111_DUNGEONVIEW_DrawDoor(L0201_ai_SquareAspect[M557_DOOR_THING_INDEX]",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
            "C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT",
            "C0x0439_CELL_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT",
        ],
        "why": "Door-front squares draw rear-side contents, then the door/frame, then replay front-side contents; this is explicit near-side replay, not a flat depth batch.",
    },
]


def slice_text(path: Path, line_range: str) -> str:
    start_s, end_s = line_range.split("-", 1)
    start = int(start_s)
    end = int(end_s)
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(lines[start - 1 : end])


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    results = []
    ok = True
    for check in CHECKS:
        path = args.source / check["file"]
        text = slice_text(path, check["range"])
        missing = [needle for needle in check["needles"] if needle not in text]
        ordered_needles = check.get("ordered_needles", [])
        out_of_order: list[str] = []
        last_position = -1
        for needle in ordered_needles:
            position = text.find(needle)
            if position < 0:
                if needle not in missing:
                    missing.append(needle)
            elif position <= last_position:
                out_of_order.append(needle)
            else:
                last_position = position
        passed = not missing and not out_of_order
        ok = ok and passed
        results.append({
            "id": check["id"],
            "passed": passed,
            "source": "{}:{}".format(check["file"], check["range"]),
            "function": check["function"],
            "why": check["why"],
            "missing": missing,
            "orderedNeedleCount": len(ordered_needles),
            "outOfOrder": out_of_order,
        })

    payload = {
        "gate": "dm1_v1_viewport_wall_draw_order_source_lock",
        "source_root": str(args.source),
        "passed": ok,
        "checks": results,
    }
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for result in results:
            status = "PASS" if result["passed"] else "FAIL"
            print(f"{status} {result['id']} {result['source']} {result['function']}")
            print(f"  {result['why']}")
            if result["missing"]:
                for needle in result["missing"]:
                    print(f"  missing: {needle}")
            if result["outOfOrder"]:
                for needle in result["outOfOrder"]:
                    print(f"  out-of-order: {needle}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
