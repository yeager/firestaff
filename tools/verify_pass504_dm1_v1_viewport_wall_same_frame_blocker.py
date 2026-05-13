#!/usr/bin/env python3
"""Pass504: DM1 V1 viewport/wall same-frame parity blocker."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT_DIR = ROOT / "parity-evidence/verification/pass504_dm1_v1_viewport_wall_same_frame_blocker"
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass504_dm1_v1_viewport_wall_same_frame_blocker.md"
STATUS = "PASS504_DM1_V1_VIEWPORT_WALL_SAME_FRAME_BLOCKER_LOCKED"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {"id": "f0128_builds_viewport_far_to_near", "file": "DUNVIEW.C", "function": "F0128_DUNGEONVIEW_Draw_CPSF", "lines": "8318-8543", "ordered": True, "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "F0098_DUNGEONVIEW_DrawFloorAndCeiling();", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(F0162_DUNGEON_GetSquareFirstObject(L0224_i_MapX, L0225_i_MapY), P0183_i_Direction, L0224_i_MapX, L0225_i_MapY, M598_VIEW_SQUARE_D4L", "F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);", "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);", "F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);", "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);", "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);"], "claim": "The comparison frame must be tied to the authoritative DUNVIEW far-to-near composition pass."},
    {"id": "wall_and_door_blit_paths_into_g0296", "file": "DUNVIEW.C", "function": "F0100/F0101/F0102/F0765 viewport blits", "lines": "3048-3180", "needles": ["void F0100_DUNGEONVIEW_DrawWallSetBitmap(", "F0132_VIDEO_Blit(P0105_puc_Bitmap, G0296_puc_Bitmap_Viewport", "void F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency(", "CM1_COLOR_NO_TRANSPARENCY", "void F0102_DUNGEONVIEW_DrawDoorBitmap(", "STATICFUNCTION void F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency("], "claim": "Wall/door occlusion proof must bind to the routes that write into G0296, not only local metadata."},
    {"id": "cell_content_order_inside_wall_squares", "file": "DUNVIEW.C", "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", "lines": "4547-5885", "ordered": True, "needles": ["STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(", "P0146_ui_OrderedViewCellOrdinals", "/* Draw objects */", "/* Draw creatures */", "T0115129_DrawProjectiles:", "F0791_DUNGEONVIEW_DrawBitmapXX"], "claim": "Same-frame proof must keep object/creature/projectile layering attached to the source cell-order pass."},
    {"id": "d3_d2_d1_wall_return_and_alcove_exceptions", "file": "DUNVIEW.C", "function": "F0116-F0124 wall branches", "lines": "6361-7959", "needles": ["STATICFUNCTION void F0116_DUNGEONVIEW_DrawSquareD3L(", "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);", "STATICFUNCTION void F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(", "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C);", "STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C(", "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C09_WALL_D2C], C709_ZONE_WALL_D2C);", "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(", "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C);", "C0x0000_CELL_ORDER_ALCOVE"], "claim": "Center/side walls occlude normal content, with explicit alcove replay exceptions that the frame must identify."},
    {"id": "f0097_presents_composed_viewport", "file": "DRAWVIEW.C", "function": "F0097_DUNGEONVIEW_DrawViewport", "lines": "709-858", "ordered": True, "needles": ["void F0097_DUNGEONVIEW_DrawViewport(", "G0324_B_DrawViewportRequested = C1_TRUE;", "M526_WaitVerticalBlank();", "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT", "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);"], "claim": "A promotable screenshot/crop must be after this present boundary for the same F0128-built buffer."},
]

LOCAL_LOCKS = [
    ("pass502_precise_blocker", ROOT / "parity-evidence/pass502_dm1_v1_viewport_wall_occlusion_audit.md", "Capture one canonical DM1 V1 original viewport frame for a wall/door occlusion case."),
    ("pass500_source_gate", ROOT / "tools/verify_pass500_dm1_v1_viewport_walls_blocker_cleanup_source_lock.py", "pass500-dm1-v1-viewport-walls-blocker-cleanup-source-lock"),
    ("pass499_runtime_predicate", ROOT / "tools/verify_pass499_dm1_v1_wall_occlusion_runtime_evidence_gate.py", "same viewport"),
    ("wall_spec_matrix", ROOT / "tools/verify_pass496_dm1_v1_wall_occlusion_spec_matrix.py", "SOURCE_CHECKS"),
    ("wall_contract_probe", ROOT / "probes/dm1/firestaff_dm1_v1_wall_composition_contract_probe.c", "wallCompositionMatrix source="),
]

PROMOTION_PREDICATE = [
    "canonical original DM1 V1 viewport crop for a wall/door occlusion case",
    "matching Firestaff crop from the same map/x/y/direction and wall/door state",
    "runtime trace or manifest proving Firestaff reached F0128 then F0097/VIDRV for that exact frame",
    "hash/region evidence showing the crop is not a repeated static gameplay frame",
    "explicit identification of wall return versus alcove/door two-pass exception when applicable",
]


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing file: {path}")
    return path.read_text(encoding="latin-1", errors="replace")


def slice_lines(text: str, span: str) -> str:
    start_s, end_s = span.split("-", 1)
    start, end = int(start_s), int(end_s)
    return "\n".join(text.splitlines()[start - 1 : end])


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def check_source(row: dict[str, Any]) -> dict[str, Any]:
    excerpt = slice_lines(read(RED / row["file"]), row["lines"])
    missing = [needle for needle in row["needles"] if needle not in excerpt]
    out_of_order: list[str] = []
    if row.get("ordered"):
        cursor = -1
        for needle in row["needles"]:
            pos = excerpt.find(needle, cursor + 1)
            if pos >= 0:
                cursor = pos
            elif needle not in missing:
                out_of_order.append(needle)
    return {"id": row["id"], "ok": not missing and not out_of_order, "source": {"file": row["file"], "function": row["function"], "lines": row["lines"], "sliceSha256": sha256_text(excerpt)}, "claim": row["claim"], "missing": missing, "outOfOrder": out_of_order}


def check_local(item: tuple[str, Path, str]) -> dict[str, Any]:
    cid, path, needle = item
    text = path.read_text(encoding="utf-8", errors="replace") if path.exists() else ""
    line = text[: text.find(needle)].count("\n") + 1 if needle in text else None
    return {"id": cid, "ok": path.exists() and needle in text, "file": str(path.relative_to(ROOT)), "line": line, "needle": needle}


def main() -> int:
    source = [check_source(row) for row in SOURCE_LOCKS]
    local = [check_local(item) for item in LOCAL_LOCKS]
    problems = [f"source lock failed: {row['id']}" for row in source if not row["ok"]]
    problems.extend(f"local lock failed: {row['id']}" for row in local if not row["ok"])
    manifest = {
        "schema": "firestaff.parity.pass504_dm1_v1_viewport_wall_same_frame_blocker.v1",
        "status": STATUS if not problems else "FAIL_PASS504_DM1_V1_VIEWPORT_WALL_SAME_FRAME_BLOCKER",
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceLocks": source,
        "localLocks": local,
        "preciseBlocker": "Viewport/walls source and Firestaff spec/probe evidence are locked, but parity still lacks one same-frame original DM1 V1 and Firestaff wall/door occlusion capture tied to the F0128 composition and F0097 present boundary.",
        "promotionPredicate": PROMOTION_PREDICATE,
        "nonClaims": ["no new original runtime capture", "no original-vs-Firestaff pixel parity promotion", "no movement-core change"],
        "problems": problems,
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = ["# Pass504 - DM1 V1 viewport/wall same-frame blocker", "", "Status: " + manifest["status"], "", "## Decision", "", manifest["preciseBlocker"], "", "## ReDMCSB source locks"]
    for row in source:
        src = row["source"]
        lines.append(f"- {src['file']}:{src['lines']} / {src['function']} ok={row['ok']} - {row['claim']}")
    lines += ["", "## Firestaff evidence locks"]
    for row in local:
        lines.append(f"- {row['id']} ok={row['ok']} file={row['file']} line={row['line']}")
    lines += ["", "## Required next proof"]
    lines.extend(f"- {item}" for item in PROMOTION_PREDICATE)
    lines += ["", "## Non-claims"]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    lines += ["", "Manifest: parity-evidence/verification/pass504_dm1_v1_viewport_wall_same_frame_blocker/manifest.json"]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(manifest["status"])
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    if problems:
        for problem in problems:
            print(f"FAIL {problem}")
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
