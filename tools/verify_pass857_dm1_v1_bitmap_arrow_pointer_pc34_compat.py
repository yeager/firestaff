#!/usr/bin/env python3
"""pass852 DM1 V1 mandatory-graphic-indices contract."""
from __future__ import annotations

import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass857_dm1_v1_bitmap_arrow_pointer_pc34_compat"
STATUS = "PASS857_DM1_V1_BITMAP_ARROW_POINTER_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_bitmap_arrow_pointer_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/bitmap_arrow_pointer_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_bitmap_arrow_pointer_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / 'parity-evidence/verification' / PASS
MANIFEST = OUT_DIR / 'manifest.json'
REPORT = ROOT / 'parity-evidence' / f'{PASS}.md'
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

ANCHORS = [
    "DATA.C:48/362-376/120",
]

LOCAL_NEEDLES = [
    "DATA.C:48",
    "DATA.C:362",
    "DATA.C:376",
    "G0042_auc_Graphic562_Bitmap_ArrowPointer",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_bitmap_arrow_pointer_pc34_compat",
    "src/dm1/dm1_v1_bitmap_arrow_pointer_pc34_compat.c",
    "NAME dm1_v1_bitmap_arrow_pointer_pc34_compat",
    "verify_pass857_dm1_v1_bitmap_arrow_pointer_pc34_compat",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (48, "G0042_auc_Graphic562_Bitmap_ArrowPointer"),
        (362, "G0042_auc_Graphic562_Bitmap_ArrowPointer"),
        (376, "G0042_auc_Graphic562_Bitmap_ArrowPointer"),
    ],
}


def read(path):
    encoding = "latin-1" if path.is_relative_to(RED) else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def line_at(path, line_no):
    lines = read(path).splitlines()
    if line_no <= 0 or line_no > len(lines):
        return ""
    return lines[line_no - 1]


def check_needles(label, path, needles):
    text = read(path)
    missing = [n for n in needles if n not in text]
    return {
        "id": label,
        "file": str(path.relative_to(ROOT)),
        "status": "PASS" if not missing else "FAIL",
        "missing": missing,
    }


def check_redmcsb_windows():
    checks = []
    for filename, windows in REDMCSB_WINDOWS.items():
        path = RED / filename
        for line_no, needle in windows:
            lo = max(1, line_no - 3)
            hi = line_no + 3
            text = "\n".join(line_at(path, row) for row in range(lo, hi + 1))
            checks.append({
                "id": f"{filename}:{line_no}",
                "file": str(path),
                "line": line_no,
                "needle": needle,
                "status": "PASS" if needle in text else "DRIFT",
                "lineText": line_at(path, line_no),
            })
    return checks


def run(cmd):
    proc = subprocess.run(cmd, cwd=ROOT, text=True,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=180)
    return {
        "command": cmd,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-20:]),
    }


def resolve_build_dir(binary_name=""):
    candidates = [ROOT / "build", ROOT / "builds" / "nv1-build",
                  ROOT / "builds" / "n2-build"]
    for c in candidates:
        if (c / "CMakeCache.txt").exists() and (c / binary_name).exists():
            return c
    for c in candidates:
        if (c / "CMakeCache.txt").exists():
            return c
    return candidates[0]


def write_outputs(local_checks, redmcsb_checks, runs):
    ok = (all(r["status"] == "PASS" for r in local_checks)
          and all(r["passed"] for r in runs))
    drift = [r for r in redmcsb_checks if r["status"] != "PASS"]
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if ok else f"FAILED_{STATUS}",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "scope": "DM1 V1 Graphics.dat item 562 init var contract.",
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": ["Re-pin anchor line numbers when ReDMCSB tree updates."] if drift else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass784-790 (mirror-candidate C040 + wound).",
            "Not pass791+ (champion-panel/leader/mirror + chest).",
            "Not pass798+ (Graphics.dat init-table gates).",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    rl = [f"# {PASS}", "", f"- Status: {manifest['status']}", "", "## Anchors", ""]
    for a in ANCHORS:
        rl.append(f"- {a}")
    rl.append("")
    rl.append("## Verification")
    for r in runs:
        rl.append(f"- `{ ' '.join(r['command']) }`: rc={r['returncode']}")
    REPORT.write_text("\n".join(rl))


def main():
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, ["dm1_v1_bitmap_arrow_pointer_plane_pc34"]),
        check_needles("test_entry_and_assertions", TEST,
                      [
                      "test_plane_pointers",
                      "test_lookup_function",
                      "test_first_last_specific",
                      "test_run_accepted",
                      "assertions passed",
                  ]),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    binary = CMAKE_NEEDLES[0]
    build_dir = resolve_build_dir(binary)
    runs = [run([str(build_dir / binary)])]
    write_outputs(local_checks, redmcsb_checks, runs)
    ok = all(r["status"] == "PASS" for r in local_checks) and all(r["passed"] for r in runs)
    print(f"{PASS}: {'PASS' if ok else 'FAIL'}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    for r in local_checks:
        if r["status"] != "PASS":
            print(f"missing in {r['id']}: {r['missing']}")
    for r in runs:
        print(r["outputTail"])
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())