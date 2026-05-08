#!/usr/bin/env python3
"""Verify DM1 V1 viewport projection/draw-order anchors in ReDMCSB."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

DEFAULT_SOURCE = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
).expanduser()

CHECKS: list[dict[str, Any]] = [
    {
        "id": "relative-coordinate-projection-source",
        "file": "DUNGEON.C",
        "range": "1371-1510",
        "ordered": [
            "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(",
            "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount",
            "P0253_i_Direction += 1, P0253_i_Direction &= 3; /* Simulate turning right */",
            "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0255_i_StepsRightCount",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0260_i_Direction, P0261_i_StepsForwardCount, P0262_i_StepsRightCount",
            "return M034_SQUARE_TYPE(F0152_DUNGEON_GetRelativeSquare(P0265_i_Direction, P0266_i_StepsForwardCount, P0267_i_StepsRightCount",
        ],
        "why": "Projection is relative (forward/right) map-coordinate mutation before square lookup/type tests.",
    },
    {
        "id": "viewport-bounds-and-zone-vocabulary",
        "file": "DEFS.H",
        "range": "2568-4234",
        "contains": [
            "#define M597_VIEW_SQUARE_D4C -3",
            "#define M598_VIEW_SQUARE_D4L -2",
            "#define M609_VIEW_SQUARE_D0C  9",
            "#define M597_VIEW_SQUARE_D4C 16",
            "#define M598_VIEW_SQUARE_D4L 17",
            "#define C007_ZONE_VIEWPORT                                        7",
            "#define C700_ZONE_VIEWPORT_CEILING_AREA                         700",
            "#define C701_ZONE_VIEWPORT_FLOOR_AREA                           701",
            "#define C704_ZONE_WALL_D3C                                      704",
            "#define C705_ZONE_WALL_D3L                                      705",
            "#define C2500_ZONE_                                            2500",
            "#define C2548_ZONE_                                            2548",
        ],
        "why": "The viewport uses named visible-square indices plus fixed viewport/wall/object zone IDs.",
    },
    {
        "id": "viewport-buffer-size-and-present",
        "files": [
            {"file": "COORD.C", "range": "1693-1731"},
            {"file": "DRAWVIEW.C", "range": "709-858"},
        ],
        "contains": [
            "int16_t G2073_C224_ViewportPixelWidth = 224;",
            "int16_t G2074_C136_ViewportHeight = 136;",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT",
            "F0638_GetZone(C007_ZONE_VIEWPORT",
        ],
        "why": "Rendering targets a 224x136 viewport bitmap and presents it through C007_ZONE_VIEWPORT.",
    },
    {
        "id": "f0128-far-to-near-draw-pipeline",
        "file": "DUNVIEW.C",
        "range": "8318-8618",
        "ordered": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0153_DUNGEON_GetRelativeSquareType(P0183_i_Direction, 3, -2",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1",
            "M598_VIEW_SQUARE_D4L",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, 1",
            "M599_VIEW_SQUARE_D4R",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, 0",
            "M597_VIEW_SQUARE_D4C",
            "F0116_DUNGEONVIEW_DrawSquareD3L",
            "F0117_DUNGEONVIEW_DrawSquareD3R",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
            "F0119_DUNGEONVIEW_DrawSquareD2L",
            "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
            "F0121_DUNGEONVIEW_DrawSquareD2C",
            "F0122_DUNGEONVIEW_DrawSquareD1L",
            "F0123_DUNGEONVIEW_DrawSquareD1R",
            "F0124_DUNGEONVIEW_DrawSquareD1C",
            "F0125_DUNGEONVIEW_DrawSquareD0L",
            "F0126_DUNGEONVIEW_DrawSquareD0R",
            "F0127_DUNGEONVIEW_DrawSquareD0C",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "why": "F0128 paints base floor/ceiling, far D4 objects, D3/D2/D1/D0 squares, then presents viewport.",
    },
    {
        "id": "wall-object-handoff-and-door-two-pass",
        "file": "DUNVIEW.C",
        "range": "6400-6488",
        "ordered": [
            "case C00_ELEMENT_WALL:",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);",
            "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
            "L0200_i_Order = C0x0000_CELL_ORDER_ALCOVE;",
            "return;",
            "case C17_ELEMENT_DOOR_FRONT:",
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "F0111_DUNGEONVIEW_DrawDoor",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        ],
        "why": "Wall squares draw/return unless alcove; door-front squares replay contents before and after the door draw.",
    },
    {
        "id": "f0115-cell-order-object-creature-projectile-handoff",
        "file": "DUNVIEW.C",
        "range": "4547-5215",
        "ordered": [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
            "P0146_ui_OrderedViewCellOrdinals",
            "P0146_ui_OrderedViewCellOrdinals >>= 4;",
            "L0139_i_Cell = M021_NORMALIZE(AL0126_i_ViewCell + P0142_i_Direction);",
            "P0141_T_Thing = L0146_T_FirstThingToDraw;",
            "if ((AL0127_i_ThingType = M012_TYPE(P0141_T_Thing)) == C04_THING_TYPE_GROUP)",
            "if (L2475_i_ViewDepth > 3)",
            "break; /* End of processing if square is too far away at D4 */",
        ],
        "why": "F0115 consumes encoded view-cell order and stops creature/projectile detail past D4.",
    },
    {
        "id": "d0c-near-square-handoff",
        "file": "DUNVIEW.C",
        "range": "8200-8312",
        "contains": [
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0222_ai_SquareAspect[M550_FIRST_THING], P0180_i_Direction, P0181_i_MapX, P0182_i_MapY, M609_VIEW_SQUARE_D0C, C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT);",
            "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[M609_VIEW_SQUARE_D0C]",
            "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M609_VIEW_SQUARE_D0C]]",
        ],
        "why": "Near D0C hands objects/creatures/projectiles/explosions to F0115 before optional field overlay.",
    },
]


def read_slice(source: Path, spec: dict[str, str]) -> tuple[str, str]:
    path = source / spec["file"]
    start_s, end_s = spec["range"].split("-", 1)
    start, end = int(start_s), int(end_s)
    lines = path.read_text(errors="replace").splitlines()
    text = "\n".join(lines[start - 1 : end])
    return f"{spec['file']}:{spec['range']}", text


def check_order(text: str, needles: list[str]) -> list[str]:
    missing: list[str] = []
    pos = -1
    for needle in needles:
        found = text.find(needle, pos + 1)
        if found == -1:
            missing.append(needle)
        else:
            pos = found
    return missing


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    results = []
    passed_all = True
    for check in CHECKS:
        specs = check.get("files") or [{"file": check["file"], "range": check["range"]}]
        slices = [read_slice(args.source, spec) for spec in specs]
        combined = "\n".join(text for _, text in slices)
        missing = []
        if "ordered" in check:
            missing.extend(check_order(combined, check["ordered"]))
        if "contains" in check:
            missing.extend(n for n in check["contains"] if n not in combined)
        digest = hashlib.sha256(combined.encode("utf-8", "replace")).hexdigest()
        passed = not missing
        passed_all = passed_all and passed
        results.append({
            "id": check["id"],
            "passed": passed,
            "source": [src for src, _ in slices],
            "sha256": digest,
            "why": check["why"],
            "missing": missing,
        })

    payload = {
        "gate": "pass391_dm1_v1_viewport_projection_draw_order_source_lock",
        "source_root": str(args.source),
        "passed": passed_all,
        "checks": results,
    }
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for result in results:
            status = "PASS" if result["passed"] else "FAIL"
            print(f"{status} {result['id']} {'; '.join(result['source'])}")
            print(f"  sha256={result['sha256']}")
            print(f"  {result['why']}")
            for needle in result["missing"]:
                print(f"  missing/order: {needle}")
    return 0 if passed_all else 1


if __name__ == "__main__":
    raise SystemExit(main())
