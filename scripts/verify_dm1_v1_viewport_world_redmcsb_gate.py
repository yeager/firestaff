#!/usr/bin/env python3
"""Verify the DM1 V1 viewport/world visual lane against local ReDMCSB source.

This is intentionally source/evidence-only: it does not touch renderer code or CMake,
and it uses only N2-local source/original-game anchors.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

DEFAULT_REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
DEFAULT_DM1_CANONICAL = Path(
    "/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1"
)

CHECKS: list[dict[str, Any]] = [
    {
        "id": "redmcsb-viewport-present-blit-palette-gate",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "range": "709-820",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE",
            "M526_WaitVerticalBlank();",
            "F0565_VIEWPORT_SetPalette(G1010_pui_DungeonViewCurrentPalette, G0347_aui_Palette_TopAndBottomScreen);",
            "F0706_GetMouseState(&L2410_i_, &L2411_i_, &L2412_i_);",
        ],
        "why": "Viewport presentation is a distinct source step after world drawing, with palette switching and screen-update gating.",
    },
    {
        "id": "redmcsb-floor-ceiling-base-copy-gate",
        "file": "DUNVIEW.C",
        "function": "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "range": "2962-3003",
        "needles": [
            "void F0098_DUNGEONVIEW_DrawFloorAndCeiling(",
            "F0008_MAIN_ClearBytes(G0086_puc_Bitmap_ViewportBlackArea",
            "F0007_MAIN_CopyBytes(G0085_puc_Bitmap_Ceiling, G0296_puc_Bitmap_Viewport",
            "F0007_MAIN_CopyBytes(G0084_puc_Bitmap_Floor, G0087_puc_Bitmap_ViewportFloorArea",
            "G0297_B_DrawFloorAndCeilingRequested = C0_FALSE;",
        ],
        "why": "The viewport/world base begins by clearing/copying ceiling and floor before square content is layered.",
    },
    {
        "id": "redmcsb-wall-door-blit-zones-gate",
        "file": "DUNVIEW.C",
        "function": "F0100_DUNGEONVIEW_DrawWallSetBitmap / F0102_DUNGEONVIEW_DrawDoorBitmap",
        "range": "3048-3110",
        "needles": [
            "void F0100_DUNGEONVIEW_DrawWallSetBitmap(",
            "F0132_VIDEO_Blit(P0105_puc_Bitmap, G0296_puc_Bitmap_Viewport",
            "void F0102_DUNGEONVIEW_DrawDoorBitmap(",
            "F0132_VIDEO_Blit(G0074_puc_Bitmap_Temporary, G0296_puc_Bitmap_Viewport",
            "void F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally(",
            "F0130_VIDEO_FlipHorizontal(P0110_puc_Bitmap",
        ],
        "why": "Walls and doors route through framed blits into G0296_puc_Bitmap_Viewport, including horizontal door-frame flipping.",
    },
    {
        "id": "redmcsb-field-teleporter-mask-cache-gate",
        "file": "DUNVIEW.C",
        "function": "F0113_DUNGEONVIEW_DrawField",
        "range": "4382-4474",
        "needles": [
            "void F0113_DUNGEONVIEW_DrawField(",
            "L2470_i_Width = M732_BYTE_WIDTH(P0135_puc_FieldAspect);",
            "F0491_CACHE_IsDerivedBitmapInCache(C000_DERIVED_BITMAP_VIEWPORT);",
            "L0119_puc_Bitmap = F0489_MEMORY_GetNativeBitmapOrGraphic(C076_GRAPHIC_FIRST_FIELD + M728_NATIVE_BITMAP_RELATIVE_INDEX(P0135_puc_FieldAspect));",
            "F0133_VIDEO_BlitBoxFilledWithMaskedBitmap",
            "F0493_CACHE_AddDerivedBitmap(C000_DERIVED_BITMAP_VIEWPORT);",
        ],
        "why": "Teleporter/field visuals are source-locked to field aspects, masks, and derived viewport bitmap cache behavior.",
    },
    {
        "id": "redmcsb-world-draw-order-near-to-far-gate",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "range": "8318-8618",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "if (G0297_B_DrawFloorAndCeilingRequested) {",
            "G0076_B_UseFlippedWallAndFootprintsBitmaps = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001",
            "F0153_DUNGEON_GetRelativeSquareType(P0183_i_Direction, 3, -2",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(F0162_DUNGEON_GetSquareFirstObject(L0224_i_MapX, L0225_i_MapY), P0183_i_Direction, L0224_i_MapX, L0225_i_MapY, M597_VIEW_SQUARE_D4C",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
        ],
        "why": "The main world visual lane controls wall flipping, far-to-near square/object draw order, final viewport presentation, and next-frame floor/ceiling anticipation.",
    },
    {
        "id": "redmcsb-side-d3-wall-field-gate",
        "file": "DUNVIEW.C",
        "function": "F0676_DrawD3L2 / F0677_DrawD3R2",
        "range": "6226-6325",
        "needles": [
            "void F0676_DrawD3L2(",
            "F0172_DUNGEON_SetSquareAspect(L2484_ai_SquareAspect",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C11_WALL_D3L2], C702_ZONE_WALL_D3L2);",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects",
            "void F0677_DrawD3R2(",
        ],
        "why": "Side D3 world visuals use square aspects, wall zones, object/creature/projectile dispatch, and teleporter field drawing.",
    },
]

DM1_ANCHORS = ["GRAPHICS.DAT", "DUNGEON.DAT", "TITLE", "README.md"]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def require_n2_local(path: Path) -> None:
    raw = str(path)
    resolved = str(path.resolve()) if path.exists() else raw
    allowed = (
        "/home/trv2/.openclaw/data/firestaff-redmcsb-source/",
        "/home/trv2/.openclaw/data/firestaff-original-games/DM/",
    )
    if "deprecated-remote-source" in raw.lower() or "deprecated-remote-source" in resolved.lower() or "<deprecated-remote-host>" in raw:
        raise SystemExit(f"refusing non-N2 path: {path}")
    if not (raw.startswith(allowed) or resolved.startswith(allowed)):
        raise SystemExit(f"refusing path outside N2-local evidence roots: {path}")


def line_slice(text: str, line_range: str) -> str:
    start, end = [int(x) for x in line_range.split("-")]
    lines = text.splitlines()
    return "\n".join(lines[start - 1 : end])


def verify(source: Path, dm1: Path) -> tuple[dict[str, Any], list[str]]:
    require_n2_local(source)
    require_n2_local(dm1)
    failures: list[str] = []
    checks_out: list[dict[str, Any]] = []
    source_hashes: dict[str, str] = {}

    for check in CHECKS:
        path = source / check["file"]
        require_n2_local(path)
        if not path.exists():
            failures.append(f"missing source file: {path}")
            continue
        text = path.read_text(errors="replace")
        excerpt = line_slice(text, check["range"])
        missing = [needle for needle in check["needles"] if needle not in excerpt]
        status = "passed" if not missing else "failed"
        if missing:
            failures.append(
                f"{check[id]} {check[file]}:{check[range]} missing "
                + "; ".join(repr(x) for x in missing)
            )
        source_hashes.setdefault(check["file"], sha256(path))
        checks_out.append(
            {
                "id": check["id"],
                "status": status,
                "source": {
                    "file": check["file"],
                    "function": check["function"],
                    "lines": check["range"],
                    "sha256": source_hashes[check["file"]],
                },
                "why": check["why"],
            }
        )

    anchors: list[dict[str, Any]] = []
    for name in DM1_ANCHORS:
        path = dm1 / name
        require_n2_local(path)
        if not path.exists():
            failures.append(f"missing DM1 canonical anchor: {path}")
            anchors.append({"name": name, "status": "missing"})
            continue
        resolved = path.resolve()
        require_n2_local(resolved)
        anchors.append(
            {
                "name": name,
                "status": "found",
                "path": str(path),
                "resolved": str(resolved),
                "bytes": resolved.stat().st_size,
                "sha256": sha256(resolved),
            }
        )

    result = {
        "gate": "dm1-v1-viewport-world-redmcsb-source-lock",
        "status": "passed" if not failures else "failed",
        "redmcsbSourceRoot": str(source),
        "dm1CanonicalRoot": str(dm1),
        "checks": checks_out,
        "dm1Anchors": anchors,
    }
    return result, failures


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=DEFAULT_REDMCSB_SOURCE)
    parser.add_argument("--dm1", type=Path, default=DEFAULT_DM1_CANONICAL)
    parser.add_argument("--json", action="store_true", help="emit full JSON evidence")
    args = parser.parse_args()

    result, failures = verify(args.source, args.dm1)
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(f"{result['status'].upper()} {result['gate']}")
        for check in result["checks"]:
            src = check["source"]
            print(
                f"{check['status'].upper()} {check['id']}: "
                f"{src['file']} {src['function']} lines {src['lines']}"
            )
        for anchor in result["dm1Anchors"]:
            print(f"{anchor['status'].upper()} dm1-anchor {anchor['name']}")
    if failures:
        for failure in failures:
            print(f"FAIL {failure}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
