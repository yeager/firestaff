#!/usr/bin/env python3
"""Pass628: source-lock and guard DM1 V1 wall-frame bitmap global use."""
from __future__ import annotations

import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
BUILD = Path(os.environ.get("FIRESTAFF_BUILD_DIR", ROOT / "build"))
TEST_BINARY = BUILD / "test_dm1_v1_viewport_3d_pc34_compat"
MANIFEST = ROOT / "parity-evidence/verification/pass628_dm1_v1_wall_frame_bitmap_global_gate/manifest.json"
REPORT = ROOT / "parity-evidence/pass628_dm1_v1_wall_frame_bitmap_global_gate.md"
STATUS = "PASS628_DM1_V1_WALL_FRAME_BITMAP_GLOBAL_GUARDED"

SOURCE_CHECKS = [
    ("pc34-wallset-doorframe-offsets", "DUNVIEW.C", "125-158", [
        "int16_t G2108_Floor = -1;",
        "int16_t G3015_i_WallSet_Wall_D0R = -17;",
        "int16_t G2115_DoorFrameTopD2LCR = -36;",
        "int16_t G2120_DoorFrameLeftD3L = -29;",
        "int16_t G2119_DoorFrameLeftD3C = -30;",
        "int16_t G2117_DoorFrameLeftD1C = -32;",
        "int16_t G2116_DoorFrameFrontD0C = -34;",
    ]),
    ("pc34-g2107-wallset-array-order", "DUNVIEW.C", "183-205", [
        "int16_t G2107_WallSet[15]",
        "-17,  /* Wall D0R */",
        "-16,  /* Wall D0L */",
        "-15,  /* Wall D1R */",
        "-14,  /* Wall D1L */",
        "-13,  /* Wall D1C */",
    ]),
    ("pc34-draw-indexed-bitmaps", "DUNVIEW.C", "3048-3105", [
        "void F0100_DUNGEONVIEW_DrawWallSetBitmap(",
        "F0132_VIDEO_Blit(P0105_puc_Bitmap, G0296_puc_Bitmap_Viewport",
        "void F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally(",
    ]),
    ("pc34-door-frame-index-calls", "DUNVIEW.C", "6446-6454", [
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2120_DoorFrameLeftD3L, C718_ZONE_DOOR_FRAME_LEFT_D3L);",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2121_, C719_ZONE_DOOR_FRAME_RIGHT_D3L);",
    ]),
    ("pc34-front-door-frame-index-calls", "DUNVIEW.C", "6725-6739", [
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2119_DoorFrameLeftD3C, C722_ZONE_DOOR_FRAME_LEFT_D3C);",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2119_DoorFrameLeftD3C, C723_ZONE_DOOR_FRAME_RIGHT_D3C);",
    ]),
    ("pc34-parity-wallset-switch", "DUNVIEW.C", "8396-8413", [
        "G3011_i_WallSet_Wall_D3C = G3049_i_WallSetFlipped_Wall_D3C;",
        "G3015_i_WallSet_Wall_D0R = G3059_i_WallSetFlipped_Wall_D0R;",
        "F0007_MAIN_CopyBytes(M772_CAST_PC(G3048_WallSetFlipped), M772_CAST_PC(G2107_WallSet), sizeof(G2107_WallSet));",
    ]),
]

LOCAL_CHECKS = [
    ("global-definition-null", "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "const uint8_t *g_dm1_wall_frame_bitmaps = NULL;"),
    ("draw-frame-base-read", "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "const uint8_t *bm_base = g_dm1_wall_frame_bitmaps;"),
    ("door-frame-null-guard", "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "if (fr && bm_base)"),
    ("wall-loop-null-guard", "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "if (!bm_base) continue;"),
    ("csb-back-null-guard", "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "if (!bm_base) return;"),
    ("draw-wall-null-safe", "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "if (!state || !frame || frame->byte_width == 0 || frame->height == 0 || !wall_bitmap) return;"),
    ("c-regression-default-null", "tests/test_dm1_v1_viewport_3d_pc34_compat.c", "g_dm1_wall_frame_bitmaps.default_null"),
    ("c-regression-no-write", "tests/test_dm1_v1_viewport_3d_pc34_compat.c", "g_dm1_wall_frame_bitmaps.null_guard_no_viewport_write"),
    ("cmake-registration", "CMakeLists.txt", "NAME pass628_dm1_v1_wall_frame_bitmap_global_gate"),
]


def source_window(path: Path, span: str) -> tuple[int, str]:
    first, last = [int(part) for part in span.split("-", 1)]
    text = path.read_text(encoding="latin-1", errors="replace")
    return first, "\n".join(text.splitlines()[first - 1:last])


def ordered_hits(path: Path, span: str, needles: list[str]) -> dict:
    first, text = source_window(path, span)
    cursor = 0
    hits = []
    missing = []
    for needle in needles:
        pos = text.find(needle, cursor)
        if pos < 0:
            missing.append(needle)
            continue
        hits.append({"line": first + text.count("\n", 0, pos), "needle": needle})
        cursor = pos + len(needle)
    return {"status": "PASS" if not missing else "FAIL", "hits": hits, "missing": missing}


def run(cmd: list[str]) -> dict:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {
        "command": cmd,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-18:]),
    }


def main(check_only: bool = False) -> int:
    red_rows = []
    for ident, filename, span, needles in SOURCE_CHECKS:
        row = ordered_hits(RED / filename, span, needles)
        row.update({"id": ident, "sourceFile": filename, "lines": span})
        red_rows.append(row)

    local_rows = []
    for ident, rel, needle in LOCAL_CHECKS:
        path = ROOT / rel
        text = path.read_text(encoding="utf-8", errors="replace") if path.exists() else ""
        local_rows.append({
            "id": ident,
            "file": rel,
            "needle": needle,
            "status": "PASS" if needle in text else "FAIL",
        })

    failures = [row["id"] for row in red_rows + local_rows if row["status"] != "PASS"]
    if check_only:
        print("PASS pass628 check-only" if not failures else "FAIL pass628 check-only: " + ",".join(failures))
        return 0 if not failures else 1

    runtime = run([str(TEST_BINARY)])
    check_run = run([sys.executable, str(Path(__file__).resolve()), "--check-only"])
    ok = not failures and runtime["passed"] and check_run["passed"]
    manifest = {
        "schema": "pass628_dm1_v1_wall_frame_bitmap_global_gate.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else "FAILED_PASS628_DM1_V1_WALL_FRAME_BITMAP_GLOBAL_GATE",
        "redmcsbRoot": str(RED),
        "buildDir": str(BUILD),
        "claim": "The DM1 V1 wall/door-frame bitmap base pointer is source-locked to the PC34 G2107/G2110-G2120 offset model and remains null-guarded until assets are wired.",
        "redmcsbChecks": red_rows,
        "firestaffChecks": local_rows,
        "verificationRuns": [runtime, check_run],
        "nonClaims": [
            "No original DOS framebuffer parity claim.",
            "No GRAPHICS.DAT asset extraction or publication claim.",
            "No renderer output change; this pass adds a guard regression and evidence gate.",
            "No CSB, DM2, Nexus, Theron, or DM1 V2 behavior claim.",
        ],
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass628 DM1 V1 wall-frame bitmap global gate",
        "",
        f"Status: {manifest['status']}",
        "",
        manifest["claim"],
        "",
        "## ReDMCSB evidence",
    ]
    for row in red_rows:
        lines.append(f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})")
        for hit in row["hits"]:
            lines.append(f"  - line {hit['line']}: {hit['needle']}")
        for missing in row["missing"]:
            lines.append(f"  - missing: {missing}")
    lines += ["", "## Firestaff evidence"]
    for row in local_rows:
        lines.append(f"- {row['status']} {row['id']} ({row['file']})")
    lines += ["", "## Verification"]
    for row in manifest["verificationRuns"]:
        lines += [f"- {' '.join(row['command'])}: rc={row['returncode']}", "~~~", row["outputTail"], "~~~"]
    lines += ["", "## Non-claims"] + [f"- {item}" for item in manifest["nonClaims"]]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(manifest["statusToken"])
    print("- wrote", MANIFEST.relative_to(ROOT))
    print("- wrote", REPORT.relative_to(ROOT))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main("--check-only" in sys.argv))
