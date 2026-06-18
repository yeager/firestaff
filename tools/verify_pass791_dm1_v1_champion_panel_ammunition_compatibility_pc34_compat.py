#!/usr/bin/env python3
"""
verify_pass791_dm1_v1_champion_panel_ammunition_compatibility_pc34_compat.py

Run the dm1_v1_champion_panel_ammunition_compatibility_pc34_compat ctest
binary, parse its PASS/FAIL output, and verify the source-locked
contract:
  - Contract-only runtime evidence (no graphics.dat/dungeon.dat).
  - Source-locked against CHAMPION.C F0294:400-450 (G0303 ammo type
    comparison) + DEFS.H G0303/G0304/G0307 (ammo type + count) +
    ITEMS.DAT ammo definitions.
  - Disjoint from pass784/785/786/787/789 (mirror-candidate C040 gates)
    and pass790 (wound probability).
"""
import os
import subprocess
import sys
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PASS = "pass791_dm1_v1_champion_panel_ammunition_compatibility_pc34_compat"
HDR = ROOT / "include/dm1_v1_champion_panel_ammunition_compatibility_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_champion_panel_ammunition_compatibility_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

RED = (
    "CHAMPION.C F0294:400-450 (G0303 ammo type comparison); "
    "DEFS.H G0303/G0304/G0307 (ammo type + count + clip size); "
    "ITEMS.DAT ammo definitions"
)

C_NEEDLES = [
    "F0294",
    "ammunition_compatibility",
    "ammo_type",
    "BOW_AMMUNITION",
    "SLING_AMMUNITION",
    "FIRST_BOW",
    "LAST_BOW",
    "FIRST_SLING",
    "LAST_SLING",
    "crossbow",
    "sling",
    "bow",
    "quarrel",
    "bolt",
    "arrow",
    "stone",
    "DEFS.H",
]
LOCAL_NEEDLES = [
    "ammunition-compatibility",
    "F0294:1-81",
    "C010_CLASS_BOW_AMMUNITION",
    "C011_CLASS_SLING_AMMUNITION",
    "ammo_type",
    "non-overlap: this gate is F0294 only",
    "ammo_compatibility",
    "pass791",
]
CMAKE_NEEDLES = [
    "test_dm1_v1_champion_panel_ammunition_compatibility_pc34_compat",
    "src/dm1/dm1_v1_champion_panel_ammunition_compatibility_pc34_compat.c",
    "NAME dm1_v1_champion_panel_ammunition_compatibility_pc34_compat",
    f"verify_{PASS}",
]

def check_needles(label, path, needles):
    if not path.exists():
        return [f"missing {label}: {path}"]
    text = path.read_text()
    missing = [n for n in needles if n not in text]
    if missing:
        return [f"{label} missing: {missing}"]
    return []

def resolve_build_dir():
    candidates = [
        Path.cwd(),
        ROOT / "builds" / "nv1-build",
        ROOT / "build",
    ]
    for c in candidates:
        if (c / "test_dm1_v1_champion_panel_ammunition_compatibility_pc34_compat").exists():
            return c
    return candidates[1]  # default to nv1-build

def run_test(build_dir):
    binary = build_dir / "test_dm1_v1_champion_panel_ammunition_compatibility_pc34_compat"
    if not binary.exists():
        return None, [f"binary not found: {binary}"]
    proc = subprocess.run(
        [str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    return proc, [proc.stdout, proc.stderr]

def parse_output(text):
    passes = text.count("PASS ")
    fails = text.count("FAIL ")
    final = ""
    for line in text.splitlines():
        if "assertions" in line:
            final = line.strip()
    return passes, fails, final

def main():
    failures = []
    file_checks = []
    for label, path in [
        ("header", HDR),
        ("test_source", TEST),
        ("cmake_registration", CMAKE),
    ]:
        needles = {
            "header": ["F0294", "ammunition_compatibility", "BOW_AMMUNITION", "SLING_AMMUNITION", "ammo_type", "DEFS.H"],
            "test_source": C_NEEDLES,
            "cmake_registration": CMAKE_NEEDLES,
        }[label]
        file_checks.append((label, check_needles(label, path, needles)))
    for label, missings in file_checks:
        for m in missings:
            failures.append(m)
    build_dir = resolve_build_dir()
    proc, output = run_test(build_dir)
    if proc is None:
        failures.append(output[0])
    else:
        text = "\n".join(output)
        passes, fails, final = parse_output(text)
        if proc.returncode != 0 or fails > 0:
            failures.append(f"runtime: exit={proc.returncode} passes={passes} fails={fails} final='{final}'")
        else:
            print(f"{PASS}: PASS passes={passes} final='{final}'")
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = {
        "pass": PASS,
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "redmcsb": RED,
        "tests": {"passes": proc and passes or 0, "fails": proc and fails or 0},
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2))
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(
        f"# {PASS}\n\n"
        f"Source-locked: {RED}\n\n"
        f"Result: {manifest['status']}\n"
        f"Tests: {manifest['tests']}\n"
        f"Failures: {failures}\n"
    )
    return 0 if not failures else 1

if __name__ == "__main__":
    sys.exit(main())
