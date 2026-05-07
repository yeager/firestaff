#!/usr/bin/env python3
"""Lock the exact DM1 V2 entry bitmap-materialization blocker.

This is intentionally a blocker/evidence gate, not a renderer. It names the
original GRAPHICS.DAT assets and ReDMCSB draw/decode route that must replace the
current symbolic flat viewport before a pixel-parity claim is allowed.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source")
GRAPHICS_DAT = (Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT")
JSON_OUT = ROOT / "parity-evidence/verification/pass298_dm1_v2_entry_bitmap_materialization_blocker.json"
MD_OUT = ROOT / "parity-evidence/pass298_dm1_v2_entry_bitmap_materialization_blocker.md"
EXPECTED_GRAPHICS_SHA256 = "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"

CHECKS = [
    ("DEFS.H", 2348, "M644_GRAPHIC_FIRST_FLOOR_SET", "floor-set base graphic family starts at GRAPHICS.DAT index 78"),
    ("DEFS.H", 2349, "M650_GRAPHIC_FLOOR_SET_0_FLOOR", "entry map floor set 0 floor is GRAPHICS.DAT index 78"),
    ("DEFS.H", 2350, "M651_GRAPHIC_FLOOR_SET_0_CEILING", "entry map floor set 0 ceiling is GRAPHICS.DAT index 79"),
    ("DEFS.H", 2351, "M646_GRAPHIC_FIRST_WALL_SET", "wall-set 0 graphic family starts at GRAPHICS.DAT index 86"),
    ("DEFS.H", 2373, "C107_GRAPHIC_WALLSET_0_D3C", "blocking D3C front wall is GRAPHICS.DAT index 107"),
    ("DEFS.H", 3437, "C14_WALL_D3C", "runtime G2107 wall-set slot for D3C is slot 14"),
    ("DUNVIEW.C", 126, "G2108_Floor = -1", "I34E native bitmap handle for floor"),
    ("DUNVIEW.C", 127, "G2109_Ceiling = -2", "I34E native bitmap handle for ceiling"),
    ("DUNVIEW.C", 183, "G2107_WallSet[15]", "I34E runtime wall-set native bitmap table"),
    ("DUNVIEW.C", 200, "-5 }; /* Wall D3C */", "G2107[C14_WALL_D3C] native bitmap handle for blocking front wall"),
    ("DUNVIEW.C", 2043, "M644_GRAPHIC_FIRST_FLOOR_SET", "floor/ceiling GRAPHICS.DAT index calculation"),
    ("DUNVIEW.C", 2053, "F0490_MEMORY_LoadDecompressAndExpandGraphic(AP0098_i_GraphicIndex", "floor bitmap decode from GRAPHICS.DAT"),
    ("DUNVIEW.C", 2054, "AP0098_i_GraphicIndex + 1", "ceiling bitmap decode from GRAPHICS.DAT"),
    ("DUNVIEW.C", 2153, "M646_GRAPHIC_FIRST_WALL_SET", "wall-set GRAPHICS.DAT index calculation"),
    ("DUNVIEW.C", 2215, "L0070_i_WallSetLastGraphicIndex < 15", "loads 15 wall-set bitmaps"),
    ("DUNVIEW.C", 2216, "G2107_WallSet[L0070_i_WallSetLastGraphicIndex]", "wall-set bitmap decode into native table"),
    ("DUNVIEW.C", 3288, "F0792_DUNGEONVIEW_DrawBitmapYYY", "I34E bitmap-zone blit/clipping function"),
    ("DUNVIEW.C", 3300, "F0635_(L2443_puc_Bitmap", "zone clipping/dimensions before blit"),
    ("DUNVIEW.C", 3301, "F0132_VIDEO_Blit", "bitmap materialization into G0296_puc_Bitmap_Viewport"),
    ("DUNVIEW.C", 8337, "G0297_B_DrawFloorAndCeilingRequested", "viewport redraw begins with floor/ceiling request"),
    ("DUNVIEW.C", 8367, "F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport)", "ceiling bitmap copied into viewport on flipped entry route"),
    ("DUNVIEW.C", 8368, "F0792_DUNGEONVIEW_DrawBitmapYYY(G2108_Floor", "floor bitmap blitted/clipped into viewport floor zone"),
    ("DUNVIEW.C", 8490, "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "D3/D2/D1/D0 draw walk starts after base"),
    ("DUNVIEW.C", 8542, "F0127_DUNGEONVIEW_DrawSquareD0C", "near/current square draw terminates walk"),
    ("DUNVIEW.C", 6708, "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C14_WALL_D3C]", "I34E blocking front wall draws bitmap slot C14/D3C"),
    ("DUNVIEW.C", 6720, "return", "front blocking wall returns after wall materialization"),
]

ASSETS = [
    {"role": "floor.base", "sourceFile": "GRAPHICS.DAT", "graphicIndex": 78, "symbol": "M650_GRAPHIC_FLOOR_SET_0_FLOOR", "runtimeNativeBitmap": "G2108_Floor", "runtimeNativeBitmapInitialValue": -1, "viewportZone": "C701_ZONE_VIEWPORT_FLOOR_AREA", "requiredDecode": "F0490_MEMORY_LoadDecompressAndExpandGraphic(78, F0631_GetBitmapPointer(G2108_Floor))", "requiredBlit": "F0792_DUNGEONVIEW_DrawBitmapYYY(G2108_Floor, C701_ZONE_VIEWPORT_FLOOR_AREA, flip)"},
    {"role": "ceiling.base", "sourceFile": "GRAPHICS.DAT", "graphicIndex": 79, "symbol": "M651_GRAPHIC_FLOOR_SET_0_CEILING", "runtimeNativeBitmap": "G2109_Ceiling", "runtimeNativeBitmapInitialValue": -2, "viewportZone": "C700_ZONE_VIEWPORT_CEILING_AREA", "requiredDecode": "F0490_MEMORY_LoadDecompressAndExpandGraphic(79, F0631_GetBitmapPointer(G2109_Ceiling))", "requiredBlit": "F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport) or F0792_DUNGEONVIEW_DrawBitmapYYY(G2109_Ceiling, C700_ZONE_VIEWPORT_CEILING_AREA, flip)"},
    {"role": "wall.front.blocking.d3c", "sourceFile": "GRAPHICS.DAT", "graphicIndex": 107, "symbol": "C107_GRAPHIC_WALLSET_0_D3C", "runtimeNativeBitmap": "G2107_WallSet[C14_WALL_D3C]", "runtimeNativeBitmapInitialValue": -5, "viewportZone": "C704_ZONE_WALL_D3C", "requiredDecode": "F0490_MEMORY_LoadDecompressAndExpandGraphic(107, F0631_GetBitmapPointer(G2107_WallSet[C14_WALL_D3C]))", "requiredBlit": "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C, flip)"},
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def line_has(path: Path, line: int, needle: str) -> bool:
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    return 1 <= line <= len(lines) and needle in lines[line - 1]


def main() -> int:
    errors: list[str] = []
    if not GRAPHICS_DAT.exists():
        errors.append(f"missing canonical GRAPHICS.DAT: {GRAPHICS_DAT}")
        graphics_sha = None
    else:
        graphics_sha = sha256(GRAPHICS_DAT)
        if graphics_sha != EXPECTED_GRAPHICS_SHA256:
            errors.append(f"GRAPHICS.DAT sha mismatch: {graphics_sha}")

    anchors = []
    for filename, line, needle, assertion in CHECKS:
        path = SOURCE / filename
        ok = path.exists() and line_has(path, line, needle)
        if not ok:
            errors.append(f"missing anchor {filename}:{line} {needle!r}")
        anchors.append({"file": filename, "line": line, "needle": needle, "assertion": assertion, "ok": ok})

    repro = [
        "cmake -S . -B build-pass298",
        "cmake --build build-pass298 --target dm1_v2_export_entry_viewport_png",
        "ctest --test-dir build-pass298 --output-on-failure -R \"dm1_v2_(entry_viewport_png_export_gate|entry_viewport_png_comparator_gate|completion_matrix)\"",
        "python3 tools/verify_dm1_v2_entry_bitmap_materialization_blocker.py",
    ]
    result = {
        "status": "failed" if errors else "passed",
        "pass": "pass298_dm1_v2_entry_bitmap_materialization_blocker",
        "scope": "Exact blocker manifest for DM1 V2 entry viewport map=0 x=1 y=3 dir=2 bitmap-backed materialization.",
        "pixelParityClaim": False,
        "currentMismatch": {"mismatchedPixels": 16405, "totalPixels": 30464},
        "canonicalGraphicsDat": {"path": str(GRAPHICS_DAT), "sha256": graphics_sha, "expectedSha256": EXPECTED_GRAPHICS_SHA256},
        "missingImplementationSeam": "Decode GRAPHICS.DAT graphic indexes 78, 79, and 107 into the I34E native bitmap handles and blit/clip them through the ReDMCSB F0674/F0792/F0132 viewport path instead of dm1_v2_vp_render_composition_flat symbolic rectangles.",
        "requiredOriginalAssets": ASSETS,
        "sourceAnchors": anchors,
        "repro": repro,
        "errors": errors,
    }
    JSON_OUT.parent.mkdir(parents=True, exist_ok=True)
    JSON_OUT.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass298 — DM1 V2 entry bitmap-materialization blocker",
        "",
        "Pixel parity is **not** claimed. This pass locks the exact missing bitmap seam for the current `16405/30464` entry viewport mismatch.",
        "",
        "## Required original GRAPHICS.DAT assets",
        "",
    ]
    for asset in ASSETS:
        lines.append(f"- `{asset['role']}`: `GRAPHICS.DAT` index `{asset['graphicIndex']}` / `{asset['symbol']}` -> `{asset['runtimeNativeBitmap']}`; zone `{asset['viewportZone']}`; decode `{asset['requiredDecode']}`; blit `{asset['requiredBlit']}`.")
    lines += [
        "",
        "## Source-locked route",
        "",
        "- `DUNVIEW.C:8337-8368` starts viewport rebuild and materializes ceiling/floor bitmaps into `G0296_puc_Bitmap_Viewport` / floor area.",
        "- `DUNVIEW.C:8490-8542` walks D3/D2/D1/D0 visible squares for route state `map=0 x=1 y=3 dir=2`.",
        "- `DUNVIEW.C:6708-6720` draws the entry front blocking wall through `F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C, ...)` and returns.",
        "- `DUNVIEW.C:3288-3301` is the I34E bitmap-zone clipping/blit function; it must replace the current two-color symbolic rectangle path.",
        "",
        "## Repro",
        "",
        "```sh",
        *repro,
        "```",
        "",
        f"Evidence JSON: `{JSON_OUT.relative_to(ROOT)}`.",
    ]
    MD_OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    if errors:
        for error in errors:
            print(f"error: {error}")
        return 1
    print(f"dm1_v2_entry_bitmap_materialization_blocker: ok evidence={JSON_OUT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
