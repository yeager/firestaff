#!/usr/bin/env python3
"""Pass 495 DM1 V1 viewport walls/items/occlusion source lock.

Evidence-only gate for the wall/viewport lane.  It locks Firestaff follow-up work to
local ReDMCSB rows for draw order, cell-content layering, wall-zone clipping, and
front/side wall occlusion.  It also hash-locks the canonical DM1 V1 anchors used by
this pass.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

DEFAULT_SOURCE = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DEFAULT_DM1 = Path("~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1").expanduser()

CHECKS: list[dict[str, Any]] = [
    {
        "id": "main-viewport-far-to-near-replay",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "range": "8318-8543",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(F0162_DUNGEON_GetSquareFirstObject(L0224_i_MapX, L0225_i_MapY), P0183_i_Direction, L0224_i_MapX, L0225_i_MapY, M598_VIEW_SQUARE_D4L",
            "F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
        ],
        "ordered": True,
        "why": "The authoritative viewport pass replays visible squares from D4/D3 through D0, not a flat depth-sorted primitive batch.",
    },
    {
        "id": "wall-field-content-zone-clipping",
        "file": "DUNVIEW.C",
        "function": "F0791_DUNGEONVIEW_DrawBitmapXX",
        "range": "3394-3472",
        "needles": [
            "STATICFUNCTION void F0791_DUNGEONVIEW_DrawBitmapXX(",
            "if (P2081_i_ZoneIndex == CM1_UNKNOWN)",
            "MASK0x8000_SHIFT_OBJECTS_AND_CREATURES",
            "F0635_(P0101_puc_Bitmap_Source, G2032_ai_XYZ, P2081_i_ZoneIndex",
            "F0132_VIDEO_Blit(P0101_puc_Bitmap_Source, P0102_puc_Bitmap_Destination",
        ],
        "why": "Objects/creatures/projectiles are shifted/clipped through source zones before the final viewport blit.",
    },
    {
        "id": "opaque-wall-and-door-blit-routes",
        "file": "DUNVIEW.C",
        "function": "F0100/F0101/F0102/F0765 wall+door blits",
        "range": "3048-3180",
        "needles": [
            "void F0100_DUNGEONVIEW_DrawWallSetBitmap(",
            "F0132_VIDEO_Blit(P0105_puc_Bitmap, G0296_puc_Bitmap_Viewport",
            "void F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency(",
            "CM1_COLOR_NO_TRANSPARENCY",
            "void F0102_DUNGEONVIEW_DrawDoorBitmap(",
            "STATICFUNCTION void F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(",
            "F0635_(L2436_puc_Bitmap, L2438_ai_XYZ, P2071_i_ZoneIndex",
        ],
        "why": "Wall panels and doors are framed/zone-clipped into G0296; opaque panels use no-transparency routes for occlusion.",
    },
    {
        "id": "cell-content-items-creatures-projectiles-order",
        "file": "DUNVIEW.C",
        "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "range": "4547-5885",
        "needles": [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
            "P0146_ui_OrderedViewCellOrdinals",
            "P0146_ui_OrderedViewCellOrdinals >>= 4;",
            "/* Draw objects */",
            "G0292_aT_PileTopObject[AL0126_i_ViewCell] = P0141_T_Thing;",
            "/* Draw creatures */",
            "F0176_GROUP_GetCreatureOrdinalInCell(L0152_ps_Group, L0139_i_Cell)",
            "T0115129_DrawProjectiles:",
            "L2474_i_ZoneIndex = C2900_ZONE_ + ((unsigned int16_t)L2479_i_ * 4) + AL0126_i_ViewCell;",
        ],
        "ordered": True,
        "why": "F0115 consumes encoded view-cell nibbles and layers items, creatures, then projectiles/explosions per source cell order.",
    },
    {
        "id": "d3-front-side-wall-occlusion",
        "file": "DUNVIEW.C",
        "function": "F0116/F0117/F0118 D3 wall branches",
        "range": "6361-6832",
        "needles": [
            "STATICFUNCTION void F0116_DUNGEONVIEW_DrawSquareD3L(",
            "case C00_ELEMENT_WALL:",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);",
            "return;",
            "STATICFUNCTION void F0117_DUNGEONVIEW_DrawSquareD3R(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C12_WALL_D3R], C706_ZONE_WALL_D3R);",
            "STATICFUNCTION void F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(",
            "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C);",
            "L0204_i_Order = C0x0000_CELL_ORDER_ALCOVE;",
        ],
        "why": "D3 side/front wall branches draw wall zones and return; only alcove-front walls intentionally replay contained objects.",
    },
    {
        "id": "d2-d1-d0-side-front-wall-returns",
        "file": "DUNVIEW.C",
        "function": "F0678/F0679/F0122-F0127 wall branches",
        "range": "6837-8308",
        "needles": [
            "STATICFUNCTION void F0678_DrawD2L2(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C06_WALL_D2L2]",
            "STATICFUNCTION void F0122_DUNGEONVIEW_DrawSquareD1L(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C03_WALL_D1L], C713_ZONE_WALL_D1L);",
            "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(",
            "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C);",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0000_CELL_ORDER_ALCOVE);",
            "STATICFUNCTION void F0125_DUNGEONVIEW_DrawSquareD0L(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L);",
            "STATICFUNCTION void F0127_DUNGEONVIEW_DrawSquareD0C(",
        ],
        "why": "Nearer D2/D1/D0 wall cases keep the same source contract: wall zone first, return/occlude normal content, with D1C alcove as the explicit exception.",
    },
    {
        "id": "front-door-two-pass-content-occlusion",
        "file": "DUNVIEW.C",
        "function": "F0124_DUNGEONVIEW_DrawSquareD1C door branch",
        "range": "7873-7938",
        "needles": [
            "case C17_ELEMENT_DOOR_FRONT:",
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2112_DoorFrameTopD1LCR, C733_ZONE_DOOR_FRAME_TOP_D1C);",
            "F0111_DUNGEONVIEW_DrawDoor(L0218_ai_SquareAspect[M557_DOOR_THING_INDEX]",
            "L0217_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING]",
        ],
        "ordered": True,
        "why": "Front doors render rear contents, then frame/panel, then replay front contents; front-side occlusion is source-ordered.",
    },
    {
        "id": "viewport-final-present-after-buffer",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "range": "709-858",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE",
            "M526_WaitVerticalBlank();",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ],
        "why": "After DUNVIEW builds G0296, DRAWVIEW presents that viewport buffer as the final step.",
    },
]

ANCHORS = ["GRAPHICS.DAT", "DUNGEON.DAT", "TITLE", "README.md"]
ALLOWED_PREFIXES = [
    Path("~/.openclaw/data/firestaff-redmcsb-source").expanduser().resolve(),
    Path("~/.openclaw/data/firestaff-original-games/DM").expanduser().resolve(),
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def ensure_local(path: Path) -> Path:
    resolved = path.expanduser().resolve()
    if not any(str(resolved).startswith(str(prefix)) for prefix in ALLOWED_PREFIXES):
        raise SystemExit(f"refusing non-local evidence path: {path}")
    return resolved


def slice_lines(text: str, spec: str) -> str:
    start_s, end_s = spec.split("-", 1)
    start = int(start_s)
    end = int(end_s)
    return "\n".join(text.splitlines()[start - 1 : end])


def verify(source: Path, dm1: Path) -> tuple[dict[str, Any], list[str]]:
    source = ensure_local(source)
    dm1 = ensure_local(dm1)
    failures: list[str] = []
    source_hashes: dict[str, str] = {}
    checks: list[dict[str, Any]] = []

    for check in CHECKS:
        path = ensure_local(source / check["file"])
        text = path.read_text(errors="replace")
        excerpt = slice_lines(text, check["range"])
        missing = [needle for needle in check["needles"] if needle not in excerpt]
        out_of_order: list[str] = []
        if check.get("ordered"):
            last = -1
            for needle in check["needles"]:
                pos = excerpt.find(needle, last + 1)
                if pos >= 0:
                    last = pos
                elif needle not in missing:
                    out_of_order.append(needle)
        status = "passed" if not missing and not out_of_order else "failed"
        if status != "passed":
            failures.append(f"{check['id']} missing={missing!r} out_of_order={out_of_order!r}")
        source_hashes.setdefault(check["file"], sha256(path))
        checks.append({
            "id": check["id"],
            "status": status,
            "source": {"file": check["file"], "function": check["function"], "lines": check["range"], "sha256": source_hashes[check["file"]]},
            "why": check["why"],
            "missing": missing,
            "outOfOrder": out_of_order,
        })

    anchors: list[dict[str, Any]] = []
    for name in ANCHORS:
        path = ensure_local(dm1 / name)
        if not path.exists():
            failures.append(f"missing canonical dm1 anchor {path}")
            anchors.append({"name": name, "status": "missing", "path": str(path)})
            continue
        anchors.append({"name": name, "status": "found", "path": str(path), "bytes": path.stat().st_size, "sha256": sha256(path)})

    result = {
        "gate": "pass495-dm1-v1-viewport-walls-occlusion-source-lock",
        "status": "passed" if not failures else "failed",
        "redmcsbSourceRoot": str(source),
        "dm1CanonicalRoot": str(dm1),
        "checks": checks,
        "dm1Anchors": anchors,
    }
    return result, failures


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--dm1", type=Path, default=DEFAULT_DM1)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    result, failures = verify(args.source, args.dm1)
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(f"{result['status'].upper()} {result['gate']}")
        for check in result["checks"]:
            src = check["source"]
            print(f"{check['status'].upper()} {check['id']}: {src['file']} {src['function']} lines {src['lines']}")
        for anchor in result["dm1Anchors"]:
            print(f"{anchor['status'].upper()} dm1-anchor {anchor['name']} sha256={anchor.get('sha256', 'missing')}")
    for failure in failures:
        print(f"FAIL {failure}")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
