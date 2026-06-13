#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DATA = Path.home() / ".openclaw/data"
EXTERNAL_DATA = Path("/Volumes/Extern-disk/openclaw-data/firestaff")
STATUS = "PASS650_DM1_V1_VIEWPORT_D2L_D2R_F0098_FALLBACK_SOURCE_LOCKED"
FAILED_STATUS = "FAILED_PASS650_DM1_V1_VIEWPORT_D2L_D2R_F0098_FALLBACK_SOURCE_LOCK"
MANIFEST = ROOT / "parity-evidence/verification/pass650_dm1_v1_viewport_d2l_d2r_f0098_fallback_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass650_dm1_v1_viewport_d2l_d2r_f0098_fallback_source_lock.md"
TEST_BINARY = Path(os.environ.get("FIRESTAFF_BUILD_DIR", str(ROOT / "build"))) / "test_dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat"


def first_existing(env_name: str, candidates: list[Path]) -> Path:
    env = os.environ.get(env_name)
    if env:
        return Path(env)
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return candidates[0]


RED = first_existing("FIRESTAFF_REDMCSB_SOURCE", [
    DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
    EXTERNAL_DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
])


def read_text(path: Path) -> str:
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def slice_text(path: Path, span: str) -> tuple[int, str]:
    first, last = (int(part) for part in span.split("-", 1))
    lines = read_text(path).splitlines()
    return first, "\n".join(lines[first - 1:last])


def ordered_hits(body: str, base: int, needles: list[str]) -> tuple[list[dict[str, Any]], list[str]]:
    cursor = 0
    hits: list[dict[str, Any]] = []
    missing: list[str] = []
    for needle in needles:
        pos = body.find(needle, cursor)
        if pos < 0:
            missing.append(needle)
            continue
        hits.append({"line": base + body.count("\n", 0, pos), "needle": needle})
        cursor = pos + len(needle)
    return hits, missing


def check_region(ident: str, path: Path, span: str, claim: str, needles: list[str]) -> dict[str, Any]:
    base, body = slice_text(path, span)
    hits, missing = ordered_hits(body, base, needles)
    return {
        "id": ident,
        "file": str(path),
        "source": f"{path.name}:{span}",
        "claim": claim,
        "status": "PASS" if not missing else "FAIL",
        "hits": hits,
        "missing": missing,
    }


def check_file(ident: str, rel: str, needles: list[str]) -> dict[str, Any]:
    path = ROOT / rel
    body = read_text(path)
    hits: list[dict[str, Any]] = []
    missing: list[str] = []
    for needle in needles:
        pos = body.find(needle)
        if pos < 0:
            missing.append(needle)
        else:
            hits.append({"line": 1 + body.count("\n", 0, pos), "needle": needle})
    return {
        "id": ident,
        "file": rel,
        "status": "PASS" if not missing else "FAIL",
        "hits": hits,
        "missing": missing,
    }


def run(cmd: list[str]) -> dict[str, Any]:
    if not Path(cmd[0]).exists():
        return {
            "command": cmd,
            "returncode": 127,
            "passed": False,
            "outputTail": "missing test binary; build the target first",
        }
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {
        "command": cmd,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-12:]),
    }


REDMCSB_CHECKS = [
    (
        "f0098_definition_and_dirty_clear",
        RED / "DUNVIEW.C",
        "2962-3002",
        "F0098 owns floor/ceiling refresh and clears the dirty flag.",
        [
            "void F0098_DUNGEONVIEW_DrawFloorAndCeiling",
            "F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);",
            "F0674_F0128_sub(G2108_Floor, G0087_puc_Bitmap_ViewportFloorArea);",
            "G0297_B_DrawFloorAndCeilingRequested = C0_FALSE;",
        ],
    ),
    (
        "f0128_f0098_f0099_d2l_d2r_f0097_order",
        RED / "DUNVIEW.C",
        "8337-8610",
        "F0128 gates F0098 on the dirty flag, performs F0099 flip work, dispatches D2L/D2R, then presents with F0097.",
        [
            "if (G0297_B_DrawFloorAndCeilingRequested) {",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal(G0084_puc_Bitmap_Floor",
            "F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal(G0085_puc_Bitmap_Ceiling",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 2, -1",
            "F0119_DUNGEONVIEW_DrawSquareD2L",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 2, 1",
            "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
            "F0097_DUNGEONVIEW_DrawViewport",
        ],
    ),
    (
        "f0119_d2l_non_wall_branch_not_wall_return",
        RED / "DUNVIEW.C",
        "6900-7049",
        "D2L non-wall branch reaches floor ornament, ceiling pit, F0115, and optional field without taking the C00 wall/F0107 return.",
        [
            "case C00_ELEMENT_WALL:",
            "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
            "return;",
            "case C02_ELEMENT_PIT:",
            "case C05_ELEMENT_TELEPORTER:",
            "case C01_ELEMENT_CORRIDOR:",
            "L0207_i_Order = C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT;",
            "F0108_DUNGEONVIEW_DrawFloorOrnament",
            "F0112_DUNGEONVIEW_DrawCeilingPit",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "F0113_DUNGEONVIEW_DrawField",
        ],
    ),
    (
        "f0120_d2r_non_wall_branch_not_wall_return",
        RED / "DUNVIEW.C",
        "7051-7224",
        "D2R non-wall branch mirrors the D2L fallback order without the C00 wall/F0107 return path.",
        [
            "case C00_ELEMENT_WALL:",
            "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
            "return;",
            "case C02_ELEMENT_PIT:",
            "case C05_ELEMENT_TELEPORTER:",
            "case C01_ELEMENT_CORRIDOR:",
            "L0209_i_Order = C0x4312_CELL_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT;",
            "F0108_DUNGEONVIEW_DrawFloorOrnament",
            "F0112_DUNGEONVIEW_DrawCeilingPit",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        ],
    ),
    (
        "defs_d2l_d2r_zone_ids",
        RED / "DEFS.H",
        "2582-4223",
        "DEFS.H binds D2L/D2R view-square, floor-view, cell-order, and floor/ceiling/zone ids.",
        [
            "#define M604_VIEW_SQUARE_D2L  4",
            "#define M605_VIEW_SQUARE_D2R  5",
            "#define C0x0342_CELL_ORDER_BACKRIGHT_FRONTLEFT_FRONTRIGHT",
            "#define C0x0431_CELL_ORDER_BACKLEFT_FRONTRIGHT_FRONTLEFT",
            "#define C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT",
            "#define C0x4312_CELL_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT",
            "#define M591_VIEW_FLOOR_D2L  5",
            "#define M593_VIEW_FLOOR_D2R  7",
            "#define C700_ZONE_VIEWPORT_CEILING_AREA",
            "#define C701_ZONE_VIEWPORT_FLOOR_AREA",
            "#define C710_ZONE_WALL_D2L",
            "#define C711_ZONE_WALL_D2R",
            "#define C805_ZONE_STAIRS_UP_FRONT_D2L",
            "#define C807_ZONE_STAIRS_UP_FRONT_D2R",
            "#define C855_ZONE_FLOORPIT_D2L",
            "#define C857_ZONE_FLOORPIT_D2R",
            "#define C864_ZONE_CEILING_PIT_D2L",
            "#define C866_ZONE_CEILING_PIT_D2R",
            "#define C1500_ZONE_FLOOR_ORNAMENT",
        ],
    ),
]

LOCAL_CHECKS = [
    (
        "firestaff_new_header_accessors",
        "src/dm1/dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat.h",
        [
            "DM1_V1_D2L_D2R_F0098_OP_F0098_FLOOR_CEILING_PC34",
            "DM1_V1_D2L_D2R_F0098_OP_F0099_FLOOR_CEILING_FLIP_PC34",
            "DM1_V1_D2L_D2R_F0098_OP_F0115_THINGS_PC34",
            "DM1_V1_D2L_D2R_F0098_OP_F0097_PRESENT_PC34",
            "dm1_v1_viewport_d2l_d2r_f0098_fallback_spec_pc34",
            "dm1_v1_viewport_d2l_d2r_f0098_fallback_order_pc34",
            "dm1_v1_viewport_d2l_d2r_f0098_should_draw_pc34",
        ],
    ),
    (
        "firestaff_new_module_source_lock",
        "src/dm1/dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat.c",
        [
            "PASS650_GRAPHICS_SHA256",
            "DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2L_OPEN_PC34",
            "DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2R_OPEN_PC34",
            "DUNVIEW.C F0098 line 2962",
            "DUNVIEW.C F0128 lines 8337-8338",
            "DUNVIEW.C F0119 lines 6900-7049",
            "F0120 lines 7051-7220",
            "DEFS.H lines 2582-2583",
            "G0297_B_DrawFloorAndCeilingRequested",
            "without the C00 wall_return/F0107 branch",
        ],
    ),
    (
        "firestaff_new_test_order_and_asset",
        "tests/test_dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat.c",
        [
            "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
            "test_order_for_non_wall_corridor_and_pit",
            "dm1_v1_viewport_d2l_d2r_f0098_should_draw_pc34(true)",
            "dm1_v1_viewport_d2l_d2r_f0098_should_draw_pc34(false)",
            "DUNVIEW.C:8337-8431 F0098 before F0099 flip work",
            "DUNVIEW.C:6968-6973 F0107 belongs to wall_return branch",
            "canonical DM1 PC34 GRAPHICS.DAT real-asset fixture",
        ],
    ),
    (
        "cmake_registered",
        "CMakeLists.txt",
        [
            "test_dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat",
            "dm1_v1_viewport_d2l_d2r_f0098_fallback",
            "pass650_dm1_v1_viewport_d2l_d2r_f0098_fallback_source_lock",
        ],
    ),
]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check-only", action="store_true", help="Write evidence and fail on source-lock drift.")
    args = ap.parse_args()
    _ = args.check_only

    red_rows = [check_region(*item) for item in REDMCSB_CHECKS]
    local_rows = [check_file(*item) for item in LOCAL_CHECKS]
    runtime = run([str(TEST_BINARY)])
    problems = [row["id"] for row in red_rows + local_rows if row["status"] != "PASS"]
    if not runtime["passed"]:
        problems.append("test_dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat")
    ok = not problems

    manifest: dict[str, Any] = {
        "schema": "pass650_dm1_v1_viewport_d2l_d2r_f0098_fallback_source_lock.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else FAILED_STATUS,
        "claim": "DM1 V1 D2L/D2R non-wall F0098 floor/ceiling fallback order is source-locked against ReDMCSB.",
        "redmcsbRoot": str(RED),
        "redmcsbChecks": red_rows,
        "firestaffChecks": local_rows,
        "verificationRuns": [runtime],
        "nonClaims": [
            "Does not duplicate D2L/D2R wall table gates.",
            "Does not duplicate D2L2/D2R2 wall source-lock gates.",
            "Does not duplicate D0C F0098 row-ownership coverage.",
            "Does not claim original DOS pixel parity or capture-backed closure.",
            "Does not change renderer behavior.",
        ],
        "problems": problems,
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    report = [
        "# Pass650 DM1 V1 D2L/D2R F0098 fallback source lock",
        "",
        f"Status: {manifest['status']}",
        "",
        str(manifest["claim"]),
        "",
        "Primary evidence:",
    ]
    for row in red_rows:
        report.append(f"- {row['status']} {row['source']}: {row['claim']}")
    report += [
        "",
        "Local gates:",
    ]
    for row in local_rows:
        report.append(f"- {row['status']} {row['file']}")
    report += [
        "",
        "Verification:",
        f"- {' '.join(runtime['command'])}: rc={runtime['returncode']}",
        "",
        "Non-claims:",
    ]
    report += [f"- {claim}" for claim in manifest["nonClaims"]]
    REPORT.write_text("\n".join(report) + "\n", encoding="utf-8")

    print(manifest["statusToken"])
    print("- wrote", MANIFEST.relative_to(ROOT))
    print("- wrote", REPORT.relative_to(ROOT))
    if problems:
        print("- problems", ", ".join(problems))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
