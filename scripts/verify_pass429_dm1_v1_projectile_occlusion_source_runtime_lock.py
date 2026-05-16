#!/usr/bin/env python3
"""Verify pass429 DM1 V1 viewport projectile occlusion source/runtime lock."""
from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
LOCAL_C = ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c"
LOCAL_H = ROOT / "include/dm1_v1_viewport_3d_pc34_compat.h"
LOCAL_TEST = ROOT / "tests/test_dm1_v1_viewport_3d_pc34_compat.c"

CHECKS: list[dict[str, Any]] = [
    {
        "id": "media720-view-square-row-map",
        "file": "DUNVIEW.C",
        "range": "370-377",
        "contains": [
            "char G2027_ac_ViewSquareIndexToViewDepth[23]",
            "char G2028_ac_ViewSquareIndexTo[23] = { 11, -1, -1, 8, 9, 10, 5, 6, 7, -1, -1, 0, 1, 2, 3, 4, -1, -1, -1, -1, -1, -1, -1 };",
        ],
        "why": "PC34/I34E projectile row zoning is driven by the MEDIA720 G2028 view-square row map.",
    },
    {
        "id": "media720-projectile-occlusion-and-zone",
        "file": "DUNVIEW.C",
        "range": "5645-5885",
        "ordered": [
            "T0115129_DrawProjectiles:",
            "if ((L2479_i_ = G2028_ac_ViewSquareIndexTo[P0145_i_ViewSquareIndex]) < 0)",
            "if ((L2475_i_ViewDepth == 3) && (AL0126_i_ViewCell <= C01_VIEW_CELL_FRONT_RIGHT))",
            "if ((L2475_i_ViewDepth == 0) && (AL0126_i_ViewCell >= C02_VIEW_CELL_BACK_RIGHT))",
            "L2474_i_ZoneIndex = C2900_ZONE_ + ((unsigned int16_t)L2479_i_ * 4) + AL0126_i_ViewCell;",
            "AL0150_ui_ProjectileScaleIndex = (L2475_i_ViewDepth << 1) - (AL0126_i_ViewCell >> 1);",
            "F0791_DUNGEONVIEW_DrawBitmapXX(F0675_DUNGEONVIEW_GetScaledBitmap",
            "G0296_puc_Bitmap_Viewport, L2474_i_ZoneIndex",
        ],
        "why": "Projectiles are not depth-sorted globally: F0115 clips unsupported cells, maps survivors to C2900 row zones, scales by depth/cell, and blits into the viewport.",
    },
]
LOCAL_NEEDLES = [
    (LOCAL_H, "DM1_ViewportProjectileOcclusionSpec"),
    (LOCAL_H, "dm1_viewport_3d_projectile_zone_for_cell"),
    (LOCAL_C, "s_projectile_occlusion_specs"),
    (LOCAL_C, "DUNVIEW.C:373,5672-5673,5683,5710-5715,5881-5883"),
    (LOCAL_C, "return 2900 + ((int)spec->g2028_row * 4) + view_cell;"),
    (LOCAL_C, "return (spec->view_depth << 1) - (view_cell >> 1);"),
    (LOCAL_TEST, "test_projectile_occlusion_zone_mapping"),
    (LOCAL_TEST, "projectile_occlusion.d0l_unsupported"),
]

def read_range(source: Path, file: str, range_: str) -> str:
    a, b = map(int, range_.split("-"))
    return "\n".join((source / file).read_text(errors="replace").splitlines()[a - 1:b])

def ordered_missing(text: str, needles: list[str]) -> list[str]:
    pos = -1
    missing = []
    for n in needles:
        i = text.find(n, pos + 1)
        if i < 0:
            missing.append(n)
        else:
            pos = i
    return missing

def run(cmd: list[str]) -> dict[str, Any]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
    return {"cmd": cmd, "returncode": proc.returncode, "passed": proc.returncode == 0, "output_tail": proc.stdout.splitlines()[-30:]}

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    ap.add_argument("--run-runtime", action="store_true")
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()
    ok = True
    results = []
    for check in CHECKS:
        text = read_range(args.source, check["file"], check["range"])
        missing = []
        if "contains" in check:
            missing += [n for n in check["contains"] if n not in text]
        if "ordered" in check:
            missing += ordered_missing(text, check["ordered"])
        passed = not missing
        ok = ok and passed
        results.append({"id": check["id"], "passed": passed, "source": f"{check['file']}:{check['range']}", "missing": missing, "why": check["why"]})
    for path, needle in LOCAL_NEEDLES:
        passed = needle in path.read_text(errors="replace")
        ok = ok and passed
        results.append({"id": f"local:{path.name}:{needle}", "passed": passed, "source": str(path.relative_to(ROOT)), "missing": [] if passed else [needle], "why": "Firestaff runtime metadata exposes the ReDMCSB projectile occlusion contract."})
    runtime = None
    if args.run_runtime:
        runtime = run([str(ROOT / "build" / "test_dm1_v1_viewport_3d_pc34_compat")])
        ok = ok and runtime["passed"]
    payload = {"gate": "pass429_dm1_v1_projectile_occlusion_source_runtime_lock", "source_root": str(args.source), "passed": ok, "checks": results, "runtime": runtime}
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for r in results:
            print(("PASS" if r["passed"] else "FAIL"), r["id"], r["source"])
            print(" ", r["why"])
            for m in r["missing"]:
                print("  missing/order:", m)
        if runtime:
            print(("PASS" if runtime["passed"] else "FAIL"), "runtime", " ".join(runtime["cmd"]))
            for line in runtime["output_tail"]:
                print(" ", line)
    return 0 if ok else 1

if __name__ == "__main__":
    raise SystemExit(main())
