#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import subprocess
from typing import Any
from zipfile import ZipFile
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
RED = ROOT / "reference/redmcsb-20210206/Toolchains/Common/Source"
DM1_ARCHIVE = Path(os.environ.get(
    "FIRESTAFF_DM1_PC34_ARCHIVE",
    Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"))
MANIFEST = ROOT / "parity-evidence/verification/pass503_dm1_v1_viewport_wall_draw_order_evidence/manifest.json"
REPORT = ROOT / "parity-evidence/pass503_dm1_v1_viewport_wall_draw_order_evidence.md"

SOURCE_CHECKS: list[dict[str, Any]] = [
    {
        "id": "f0128-far-to-near-square-replay",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "lines": "8318-8543",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "M598_VIEW_SQUARE_D4L",
            "F0676_DrawD3L2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
        ],
        "ordered": True,
        "claim": "Viewport composition replays source square functions from far to near before presentation.",
    },
    {
        "id": "f0115-cell-layering-contract",
        "file": "DUNVIEW.C",
        "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "lines": "4547-4582",
        "needles": [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
            "draw each object found",
            "Draw one creature at the cell being processed",
            "Draw only projectiles at specified cell",
            "Draw only explosions at specified cell",
            "If a Fluxcage is present, draw the fluxcage",
        ],
        "ordered": True,
        "claim": "Per-cell content order is objects, creatures, projectiles, then explosions/fluxcage.",
    },
    {
        "id": "wall-blit-routes-into-viewport-buffer",
        "file": "DUNVIEW.C",
        "function": "F0100/F0101/F0102/F0765 wall and door blits",
        "lines": "3048-3180",
        "needles": [
            "void F0100_DUNGEONVIEW_DrawWallSetBitmap(",
            "F0132_VIDEO_Blit(P0105_puc_Bitmap, G0296_puc_Bitmap_Viewport",
            "void F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency(",
            "CM1_COLOR_NO_TRANSPARENCY",
            "void F0102_DUNGEONVIEW_DrawDoorBitmap(",
            "STATICFUNCTION void F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(",
        ],
        "ordered": True,
        "claim": "Wall and door panels write into G0296 with explicit transparent or opaque routes.",
    },
    {
        "id": "d3-wall-return-and-alcove-exception",
        "file": "DUNVIEW.C",
        "function": "F0676/F0677/F0116/F0117/F0118 D3 wall branches",
        "lines": "6226-6836",
        "needles": [
            "void F0676_DrawD3L2(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C11_WALL_D3L2], C702_ZONE_WALL_D3L2);",
            "void F0677_DrawD3R2(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C10_WALL_D3R2], C703_ZONE_WALL_D3R2);",
            "STATICFUNCTION void F0116_DUNGEONVIEW_DrawSquareD3L(",
            "L0200_i_Order = C0x0000_CELL_ORDER_ALCOVE;",
            "STATICFUNCTION void F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(",
            "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C);",
        ],
        "ordered": True,
        "claim": "D3 walls occlude by drawing wall zones and returning, except front alcoves that intentionally hand contents to F0115.",
    },
    {
        "id": "d2-d1-d0-wall-return-contract",
        "file": "DUNVIEW.C",
        "function": "F0678/F0679/F0119-F0127 near wall branches",
        "lines": "6837-8308",
        "needles": [
            "STATICFUNCTION void F0678_DrawD2L2(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C06_WALL_D2L2]",
            "STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C(",
            "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C09_WALL_D2C], C709_ZONE_WALL_D2C);",
            "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0000_CELL_ORDER_ALCOVE);",
            "STATICFUNCTION void F0125_DUNGEONVIEW_DrawSquareD0L(",
            "STATICFUNCTION void F0126_DUNGEONVIEW_DrawSquareD0R(",
            "STATICFUNCTION void F0127_DUNGEONVIEW_DrawSquareD0C(",
        ],
        "ordered": True,
        "claim": "D2, D1, and D0 wall paths preserve the same wall-return/alcove exception pattern.",
    },
    {
        "id": "front-door-two-pass-occlusion",
        "file": "DUNVIEW.C",
        "function": "F0124_DUNGEONVIEW_DrawSquareD1C",
        "lines": "7873-7938",
        "needles": [
            "case C17_ELEMENT_DOOR_FRONT:",
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "F0111_DUNGEONVIEW_DrawDoor(L0218_ai_SquareAspect[M557_DOOR_THING_INDEX]",
            "L0217_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING]",
        ],
        "ordered": True,
        "claim": "Front doors draw rear contents, door/frame, then front contents.",
    },
    {
        "id": "viewport-present-boundary",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "lines": "709-858",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE",
            "M526_WaitVerticalBlank();",
            "G0296_puc_Bitmap_Viewport",
            "F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ],
        "ordered": True,
        "claim": "DUNVIEW composition buffer G0296 is presented by DRAWVIEW after vblank/present gating through the PC34 viewport zone and video-driver blit route.",
    },
]

LOCAL_CHECKS = [
    ("firestaff-draw-order-table", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "static const DM1_ViewportDrawStep s_draw_order[]"),
    ("firestaff-wall-spec-table", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "static const DM1_ViewportWallDrawSpec s_wall_draw_specs[]"),
    ("firestaff-thing-layer-table", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "static const DM1_ViewportThingLayerSpec s_thing_layers[]"),
    ("firestaff-door-front-occlusion-table", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "static const DM1_ViewportDoorFrontOcclusionSpec s_door_front_occlusion_specs[]"),
    ("pass496-matrix-gate-present", ROOT / "tools/verify_pass496_dm1_v1_wall_occlusion_spec_matrix.py", "SOURCE_CHECKS = ["),
    ("pass502-blocker-doc-present", ROOT / "parity-evidence/pass502_dm1_v1_viewport_wall_occlusion_audit.md", "Required next evidence before parity promotion:"),
]

ANCHORS = ["DATA/GRAPHICS.DAT", "DATA/DUNGEON.DAT", "TITLE"]

SECONDARY_REF_CHECKS: list[dict[str, Any]] = []


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_zip_member(path: Path, name: str) -> bytes:
    """Read in memory, tolerating the retail ZIP's DOS slash headers."""
    result = subprocess.run(
        ["unzip", "-p", str(path), name], check=False,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode not in (0, 1) or not result.stdout:
        raise RuntimeError(
            f"cannot read ZIP member {name}: rc={result.returncode}")
    return result.stdout


def slice_text(path: Path, spec: str) -> str:
    start, end = (int(part) for part in spec.split("-", 1))
    return "\n".join(path.read_text(encoding="utf-8", errors="replace").splitlines()[start - 1:end])


def line_no(text: str, needle: str) -> int | None:
    for idx, line in enumerate(text.splitlines(), 1):
        if needle in line:
            return idx
    return None


def window_needle_lines(path: Path, line_range: str, needles: list[str]) -> dict[str, int | None]:
    start, end = (int(part) for part in line_range.split("-", 1))
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    window = lines[start - 1:end]
    result: dict[str, int | None] = {}
    for needle in needles:
        result[needle] = None
        for offset, line in enumerate(window, start):
            if needle in line:
                result[needle] = offset
                break
    return result


def check_source(spec: dict[str, Any]) -> dict[str, Any]:
    path = RED / spec["file"]
    window = slice_text(path, spec["lines"])
    missing = [needle for needle in spec["needles"] if needle not in window]
    out_of_order: list[str] = []
    cursor = -1
    if spec.get("ordered"):
        for needle in spec["needles"]:
            pos = window.find(needle, cursor + 1)
            if pos == -1 and needle not in missing:
                out_of_order.append(needle)
            elif pos != -1:
                cursor = pos
    return {
        "id": spec["id"],
        "ok": not missing and not out_of_order,
        "file": spec["file"],
        "function": spec["function"],
        "lineRange": spec["lines"],
        "sha256": sha256(path),
        "claim": spec["claim"],
        "needleLines": window_needle_lines(path, spec["lines"], spec["needles"]),
        "missing": missing,
        "outOfOrder": out_of_order,
    }


def check_local(cid: str, path: Path, needle: str) -> dict[str, Any]:
    if not path.exists():
        return {"id": cid, "ok": False, "file": str(path.relative_to(ROOT)), "needle": needle, "line": None}
    text = path.read_text(encoding="utf-8", errors="replace")
    return {"id": cid, "ok": needle in text, "file": str(path.relative_to(ROOT)), "needle": needle, "line": line_no(text, needle)}


def check_secondary_ref(spec: dict[str, Any]) -> dict[str, Any]:
    path = spec["root"] / spec["file"]
    if not path.exists():
        return {
            "id": spec["id"],
            "ok": False,
            "file": str(path),
            "lineRange": spec["lines"],
            "claim": spec["claim"],
            "missing": ["missing file"],
        }
    window = slice_text(path, spec["lines"])
    missing = [needle for needle in spec["needles"] if needle not in window]
    return {
        "id": spec["id"],
        "ok": not missing,
        "file": str(path),
        "lineRange": spec["lines"],
        "sha256": sha256(path),
        "claim": spec["claim"],
        "missing": missing,
    }


def main() -> int:
    source_results = [check_source(spec) for spec in SOURCE_CHECKS]
    local_results = [check_local(*item) for item in LOCAL_CHECKS]
    secondary_results = [check_secondary_ref(spec) for spec in SECONDARY_REF_CHECKS]
    anchors = []
    if DM1_ARCHIVE.is_file():
        with ZipFile(DM1_ARCHIVE) as archive:
            members = {item.filename: item for item in archive.infolist()}
            for name in ANCHORS:
                item = members.get(name)
                data = read_zip_member(DM1_ARCHIVE, name) if item is not None else None
                anchors.append({
                    "name": name,
                    "archive": str(DM1_ARCHIVE),
                    "exists": item is not None,
                    "bytes": item.file_size if item is not None else None,
                    "sha256": sha256_bytes(data) if data is not None else None,
                })
    else:
        anchors = [{"name": name, "archive": str(DM1_ARCHIVE),
                    "exists": False, "bytes": None, "sha256": None}
                   for name in ANCHORS]

    # ReDMCSB and the selected retail PC 3.4 archive are authoritative for
    # this gate. Historical GreatStone/CSBWin mirrors remain useful context,
    # but their absence must not invalidate real-media evidence.
    problems = [row["id"] for row in source_results + local_results
                if not row["ok"]]
    problems.extend(f"missing-anchor-{row['name']}" for row in anchors if not row["exists"])
    status = "PASS_PASS503_DM1_V1_VIEWPORT_WALL_DRAW_ORDER_EVIDENCE" if not problems else "FAIL_PASS503_DM1_V1_VIEWPORT_WALL_DRAW_ORDER_EVIDENCE"

    manifest = {
        "schema": "firestaff.parity.pass503_dm1_v1_viewport_wall_draw_order_evidence.v1",
        "status": status,
        "ok": not problems,
        "redmcsbSourceRoot": str(RED),
        "dm1Archive": str(DM1_ARCHIVE),
        "dm1ArchiveSha256": sha256(DM1_ARCHIVE) if DM1_ARCHIVE.is_file() else None,
        "sourceChecks": source_results,
        "firestaffChecks": local_results,
        "dm1Anchors": anchors,
        "secondaryReferences": secondary_results,
        "nonClaims": [
            "no original-vs-Firestaff pixel parity promotion",
            "no runtime capture promotion",
            "no movement-core edits",
        ],
        "problems": problems,
    }

    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    report = ["# Pass503 - DM1 V1 viewport wall draw-order evidence", "", f"Status: {status}", "", "## ReDMCSB source locks"]
    for row in source_results:
        report.append(f"- {row['file']} {row['function']} lines {row['lineRange']} ok={row['ok']}: {row['claim']}")
        for needle, line in row["needleLines"].items():
            report.append(f"  - line {line}: {needle}")
    report += ["", "## Firestaff hooks", ""]
    for row in local_results:
        report.append(f"- {row['file']} line {row['line']} ok={row['ok']}: {row['id']}")
    report += ["", "## DM1 anchors", ""]
    for row in anchors:
        digest = row["sha256"][:12] if row["sha256"] else "missing"
        report.append(f"- {row['name']} exists={row['exists']} sha256={digest}")
    report += ["", "## Optional historical secondary references", ""]
    for row in secondary_results:
        report.append(f"- {row['file']} lines {row['lineRange']} ok={row['ok']}: {row['claim']}")
    report += ["", "## Non-claims", "", "- This is source/probe evidence only.", "- Pixel parity still needs the same-viewport original/Firestaff runtime capture described by pass502."]
    if problems:
        report += ["", "## Problems", ""]
        report.extend(f"- {problem}" for problem in problems)
    REPORT.write_text("\n".join(report) + "\n", encoding="utf-8")

    print(status)
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
