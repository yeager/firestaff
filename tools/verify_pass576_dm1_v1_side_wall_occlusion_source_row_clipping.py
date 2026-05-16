#!/usr/bin/env python3
"""Focused DM1 V1 side-wall occlusion and source-row clipping gate.

This pass intentionally avoids front-wall/front-cell promotion.  It source-locks
only side-wall lanes and the wall blit source-row clipping helper they share.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
DM1 = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1")
REPORT = ROOT / "parity-evidence/pass576_dm1_v1_side_wall_occlusion_source_row_clipping.md"
MANIFEST = ROOT / "parity-evidence/verification/pass576_dm1_v1_side_wall_occlusion_source_row_clipping/manifest.json"
STATUS = "PASS576_DM1_V1_SIDE_WALL_OCCLUSION_SOURCE_ROW_CLIPPING_LOCKED"

DM1_HASH_EXPECTED = {
    "GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
}

ALLOWED_ROOTS = [
    ROOT.resolve(),
    Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source").resolve(),
    Path("/home/trv2/.openclaw/data/firestaff-original-games/DM").resolve(),
]

SOURCE_CHECKS = [
    {
        "id": "redmcsb_pc34_side_wall_zone_ids_are_not_front_cells",
        "path": RED / "DEFS.H",
        "lines": "4042-4057",
        "claim": "PC34 side-wall zones are distinct D3/D2/D1/D0 side lanes; this gate excludes D3C/D2C/D1C/D0C front/center cells.",
        "ordered": [
            "#define C702_ZONE_WALL_D3L2",
            "#define C703_ZONE_WALL_D3R2",
            "#define C705_ZONE_WALL_D3L",
            "#define C706_ZONE_WALL_D3R",
            "#define C707_ZONE_WALL_D2L2",
            "#define C708_ZONE_WALL_D2R2",
            "#define C710_ZONE_WALL_D2L",
            "#define C711_ZONE_WALL_D2R",
            "#define C713_ZONE_WALL_D1L",
            "#define C714_ZONE_WALL_D1R",
            "#define C716_ZONE_WALL_D0L",
            "#define C717_ZONE_WALL_D0R",
        ],
        "forbidden": [],
    },
    {
        "id": "redmcsb_f0128_draws_side_rows_before_centers",
        "path": RED / "DUNVIEW.C",
        "lines": "8478-8542",
        "claim": "F0128 traverses side lanes in row order before each same-depth center/front cell.",
        "ordered": [
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
        ],
        "forbidden": [],
    },
    {
        "id": "redmcsb_far_side_wall_cases_return_before_field_paths",
        "path": RED / "DUNVIEW.C",
        "lines": "6846-6896",
        "claim": "D2L2/D2R2 wall cases draw side zones and return before teleporter field paths.",
        "ordered": [
            "case C00_ELEMENT_WALL:",
            "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C05_WALL_D2R2], C707_ZONE_WALL_D2L2);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C06_WALL_D2L2]",
            ", C707_ZONE_WALL_D2L2);",
            "return;",
            "case C05_ELEMENT_TELEPORTER:",
            "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[C09_VIEW_SQUARE_D2L2]], C707_ZONE_WALL_D2L2);",
            "STATICFUNCTION void F0679_DrawD2R2(",
            "case C00_ELEMENT_WALL:",
            "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C06_WALL_D2L2], C708_ZONE_WALL_D2R2);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C05_WALL_D2R2]",
            ", C708_ZONE_WALL_D2R2);",
            "return;",
            "case C05_ELEMENT_TELEPORTER:",
            "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[C10_VIEW_SQUARE_D2R2]], C708_ZONE_WALL_D2R2);",
        ],
        "forbidden": [],
    },
    {
        "id": "redmcsb_d1_side_wall_cases_return_before_open_lane_paths",
        "path": RED / "DUNVIEW.C",
        "lines": "7436-7628",
        "claim": "D1L/D1R wall cases draw side zones, apply only side ornament checks, then return.",
        "ordered": [
            "case C00_ELEMENT_WALL:",
            "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C02_WALL_D1R], C713_ZONE_WALL_D1L);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C03_WALL_D1L], C713_ZONE_WALL_D1L);",
            "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0214_ai_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL], M585_VIEW_WALL_D1L_RIGHT);",
            "return;",
            "case C00_ELEMENT_WALL:",
            "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C03_WALL_D1L], C714_ZONE_WALL_D1R);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C02_WALL_D1R], C714_ZONE_WALL_D1R);",
            "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0216_ai_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL], M586_VIEW_WALL_D1R_LEFT);",
            "return;",
        ],
        "forbidden": [],
    },
    {
        "id": "redmcsb_d0_side_wall_cases_return_before_source_row_field_paths",
        "path": RED / "DUNVIEW.C",
        "lines": "7999-8159",
        "claim": "D0L/D0R wall cases draw side zones and return before the source-row field/open-lane paths.",
        "ordered": [
            "case C00_ELEMENT_WALL:",
            "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C00_WALL_D0R], C716_ZONE_WALL_D0L);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L);",
            "return;",
            "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M610_VIEW_SQUARE_D0L]], C716_ZONE_WALL_D0L);",
            "case C00_ELEMENT_WALL:",
            "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C01_WALL_D0L], C717_ZONE_WALL_D0R);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R);",
            "return;",
            "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M611_VIEW_SQUARE_D0R]], C717_ZONE_WALL_D0R);",
        ],
        "forbidden": [],
    },
    {
        "id": "redmcsb_shared_bitmap_source_row_clipping_seam",
        "path": RED / "DUNVIEW.C",
        "lines": "3394-3472",
        "claim": "F0791 resolves source/destination clipping through F0635 before blitting side-wall-adjacent bitmap zones.",
        "ordered": [
            "STATICFUNCTION void F0791_DUNGEONVIEW_DrawBitmapXX(",
            "if (P2081_i_ZoneIndex == CM1_UNKNOWN) {",
            "F0635_(P0101_puc_Bitmap_Source, G2032_ai_XYZ, P2081_i_ZoneIndex, &L2447_i_Width, &L2448_i_Height)",
            "if ((L2449_i_ > L2450_i_) && M007_GET(P2082_i_Flip, MASK0x0001_FLIP_HORIZONTAL))",
            "L2447_i_Width += L2449_i_;",
            "if ((L2449_i_ > L2450_i_) && M007_GET(P2082_i_Flip, MASK0x0002_FLIP_VERTICAL))",
            "L2448_i_Height += L2449_i_;",
            "F0132_VIDEO_Blit(P0101_puc_Bitmap_Source, P0102_puc_Bitmap_Destination, G2032_ai_XYZ, L2447_i_Width, L2448_i_Height",
        ],
        "forbidden": [],
    },
]

FIRESTAFF_CHECKS = [
    {
        "id": "firestaff_side_wall_metadata_has_returning_side_lanes_only",
        "path": ROOT / "dm1_v1_viewport_3d_pc34_compat.c",
        "lines": "291-300",
        "claim": "Firestaff metadata encodes side wall returns for far-side, D1, and D0 side lanes without center/front cells.",
        "ordered": [
            "DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2",
            "DM1_PC34_ZONE_WALL_D2L2",
            "DUNVIEW.C:6848-6862 wall case returns",
            "DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2",
            "DM1_PC34_ZONE_WALL_D2R2",
            "DUNVIEW.C:6882-6893 wall case returns",
            "DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R",
            "DM1_PC34_ZONE_WALL_D1L",
            "DUNVIEW.C:7459-7460 side ornament then return",
            "DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L",
            "DM1_PC34_ZONE_WALL_D1R",
            "DUNVIEW.C:7627-7628 side ornament then return",
            "DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R",
            "DM1_PC34_ZONE_WALL_D0L",
            "DUNVIEW.C:8036-8038 wall case returns",
            "DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L",
            "DM1_PC34_ZONE_WALL_D0R",
            "DUNVIEW.C:8142-8144 wall case returns",
        ],
        "forbidden": [],
    },
    {
        "id": "firestaff_wall_clip_gate_retains_source_offsets_and_occlusion",
        "path": ROOT / "dm1_v1_viewport_3d_pc34_compat.c",
        "lines": "729-768",
        "claim": "The local wall clip gate preserves source X/Y offsets, clips to source and viewport bounds, and can mark fully occluded rows invisible.",
        "ordered": [
            "DM1_ViewportBlitClipGate dm1_viewport_3d_resolve_wall_blit_clip_gate",
            "int src_x = frame->blit_x;",
            "int src_y = frame->blit_y;",
            "if (dst_x < 0) { src_x -= dst_x; width += dst_x; dst_x = 0; }",
            "if (src_x + width > source_width) width = source_width - src_x;",
            "if (width <= 0 || height <= 0) return gate;",
            "gate.visible = true;",
            "gate.src_x = (int16_t)src_x;",
            "gate.src_y = (int16_t)src_y;",
        ],
        "forbidden": [],
    },
    {
        "id": "firestaff_narrow_runtime_assertions_cover_side_walls_and_clip_rows",
        "path": ROOT / "test_dm1_v1_viewport_3d_pc34_compat.c",
        "lines": "222-248",
        "claim": "Existing narrow runtime assertions cover side wall zones/returns; the same file also asserts source-row clipping edge cases.",
        "ordered": [
            "DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2",
            "DM1_PC34_ZONE_WALL_D2L2",
            "\"6862\"",
            "DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2",
            "DM1_PC34_ZONE_WALL_D2R2",
            "\"6893\"",
            "DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R",
            "DM1_PC34_ZONE_WALL_D1L",
            "\"7460\"",
            "DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L",
            "DM1_PC34_ZONE_WALL_D1R",
            "\"7628\"",
            "DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R",
            "DM1_PC34_ZONE_WALL_D0L",
            "\"8038\"",
            "DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L",
            "DM1_PC34_ZONE_WALL_D0R",
            "\"8144\"",
        ],
        "forbidden": [],
    },
    {
        "id": "firestaff_clip_row_runtime_assertions_are_registered",
        "path": ROOT / "test_dm1_v1_viewport_3d_pc34_compat.c",
        "lines": "799-855",
        "claim": "Source-row clipping has explicit visible, source-occluded, viewport-occluded, and draw-copy assertions.",
        "ordered": [
            "static void test_wall_source_row_clip_occlusion_gate(void)",
            "wall_clip_gate.151713.src_x",
            "wall_clip_gate.151713.src_y",
            "wall_clip_gate.occluded_source_row",
            "wall_clip_gate.occluded_viewport",
            "static void test_wall_draw_uses_clip_gate_source_offsets(void)",
            "wall_clip_draw.source_offset_next",
            "wall_clip_draw.opaque_copies_transparent_color",
        ],
        "forbidden": [],
    },
]


def safe_path(path: Path) -> Path:
    resolved = path.resolve()
    if not any(str(resolved).startswith(str(root)) for root in ALLOWED_ROOTS):
        raise AssertionError(f"refusing evidence path outside N2-local allowlist: {path}")
    return resolved


def sha256(path: Path) -> str:
    path = safe_path(path)
    h = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def read_text(path: Path) -> str:
    path = safe_path(path)
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def slice_lines(text: str, spec: str) -> tuple[int, str]:
    start, end = (int(part) for part in spec.split("-", 1))
    return start, "\n".join(text.splitlines()[start - 1:end])


def audit(checks: list[dict[str, object]]) -> list[dict[str, object]]:
    results = []
    for check in checks:
        path = safe_path(check["path"])  # type: ignore[arg-type]
        base, excerpt = slice_lines(read_text(path), check["lines"])  # type: ignore[arg-type]
        cursor = 0
        hits = []
        missing = []
        for needle in check["ordered"]:  # type: ignore[index]
            pos = excerpt.find(needle, cursor)
            if pos < 0:
                missing.append(needle)
            else:
                hits.append({"line": base + line_no(excerpt, pos) - 1, "needle": needle})
                cursor = pos + len(needle)
        forbidden_hits = []
        for needle in check.get("forbidden", []):  # type: ignore[union-attr]
            pos = excerpt.find(needle)
            if pos >= 0:
                forbidden_hits.append({"line": base + line_no(excerpt, pos) - 1, "needle": needle})
        results.append({
            "id": check["id"],
            "status": "PASS" if not missing and not forbidden_hits else "FAIL",
            "path": str(path),
            "sourceFile": path.name,
            "lines": check["lines"],
            "sha256": sha256(path),
            "claim": check["claim"],
            "hits": hits,
            "missing": missing,
            "forbiddenHits": forbidden_hits,
        })
    return results


def dm1_anchors() -> list[dict[str, object]]:
    anchors = []
    for name, expected in DM1_HASH_EXPECTED.items():
        path = safe_path(DM1 / name)
        actual = sha256(path)
        anchors.append({
            "variant": "DM1 canonical PC34/V1",
            "name": name,
            "path": str(path),
            "sha256": actual,
            "expectedSha256": expected,
            "bytes": path.stat().st_size,
            "status": "PASS" if actual == expected else "FAIL",
        })
    return anchors


def write_report(manifest: dict[str, object]) -> None:
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass576 DM1 V1 side-wall occlusion source-row clipping",
        "",
        f"Status: {manifest['status']}",
        "",
        "## Claim",
        "",
        "Side-wall lanes are source-locked separately from front-wall/front-cell gates. ReDMCSB draws D3/D2/D1/D0 side lanes in F0128 row order, side-wall cases draw their side zones and return before open-lane/field paths, and the shared bitmap path retains source-row clipping before blit.",
        "",
        "## Primary ReDMCSB Evidence",
    ]
    for row in manifest["redmcsbPrimaryChecks"]:  # type: ignore[index]
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})", f"  - {row['claim']}"]
        lines += [f"  - line {hit['line']}: {hit['needle']}" for hit in row["hits"]]
        lines += [f"  - missing: {needle}" for needle in row["missing"]]
        lines += [f"  - forbidden hit line {hit['line']}: {hit['needle']}" for hit in row["forbiddenHits"]]
    lines += ["", "## Firestaff Evidence"]
    for row in manifest["firestaffChecks"]:  # type: ignore[index]
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})", f"  - {row['claim']}"]
        lines += [f"  - line {hit['line']}: {hit['needle']}" for hit in row["hits"]]
        lines += [f"  - missing: {needle}" for needle in row["missing"]]
    lines += ["", "## DM1 Hash Locks"]
    for row in manifest["dm1HashLocks"]:  # type: ignore[index]
        lines += [f"- {row['status']} {row['variant']} {row['name']}: {row['path']} sha256={row['sha256']} bytes={row['bytes']}"]
    lines += ["", "## Non-Claims"]
    lines += [f"- {item}" for item in manifest["nonClaims"]]  # type: ignore[index]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    redmcsb = audit(SOURCE_CHECKS)
    firestaff = audit(FIRESTAFF_CHECKS)
    anchors = dm1_anchors()
    failed = [row["id"] for row in redmcsb + firestaff if row["status"] != "PASS"]
    failed += [f"dm1-hash-{row['name']}" for row in anchors if row["status"] != "PASS"]
    ok = not failed
    manifest = {
        "schema": "pass576_dm1_v1_side_wall_occlusion_source_row_clipping.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else "FAILED_PASS576_DM1_V1_SIDE_WALL_OCCLUSION_SOURCE_ROW_CLIPPING",
        "redmcsbRoot": str(RED),
        "claim": "DM1 V1 side-wall occlusion and shared wall source-row clipping are source-locked without promoting front-wall/front-cell behavior.",
        "redmcsbPrimaryChecks": redmcsb,
        "firestaffChecks": firestaff,
        "dm1HashLocks": anchors,
        "nonClaims": [
            "No front-wall/front-cell behavior is promoted by this pass.",
            "No renderer behavior, release artifact, tag, or external state is changed.",
            "No original runtime capture or pixel-parity claim is made.",
            "Only N2-local ReDMCSB and canonical DM1 references are used.",
        ],
        "failedChecks": failed,
    }
    write_report(manifest)
    print(manifest["statusToken"])
    print(f"- wrote {MANIFEST.relative_to(ROOT)}")
    print(f"- wrote {REPORT.relative_to(ROOT)}")
    for row in redmcsb:
        print(f"- ReDMCSB {row['sourceFile']}:{row['lines']} {row['id']} {row['status']}")
    for row in firestaff:
        print(f"- Firestaff {row['sourceFile']}:{row['lines']} {row['id']} {row['status']}")
    return 0 if ok else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
