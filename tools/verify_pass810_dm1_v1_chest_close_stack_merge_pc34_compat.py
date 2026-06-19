#!/usr/bin/env python3
"""Verify pass810 — DM1 V1 chest close object stack-merge."""
import os, subprocess, sys, json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PASS = "pass810_dm1_v1_chest_close_stack_merge_pc34_compat"
HDR = ROOT / "include/dm1/dm1_v1_chest_close_stack_merge_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_chest_close_stack_merge_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    "CHEST.C F0333:30-67 (F0333_INVENTORY_OpenAndDrawChest: same-open "
    "return, chain walk via F0159, eight-item cap, G0425_aT_ChestSlots "
    "writes, C0xFFFE_THING_ENDOFLIST stop, C0xFFFF_THING_NONE tail fill); "
    "CHEST.C F0334:113-132 (F0334_INVENTORY_CloseChest: no-open return, "
    "G0426 clear, Container->Slot=C0xFFFE_THING_ENDOFLIST clobber, scan "
    "eight G0425 entries, skip C0xFFFF_THING_NONE, clear slots, relink "
    "via F0163 with CM1_MAPX_NOT_ON_A_SQUARE list-append mode); "
    "DUNGEON.C F0163:1769-1838 (DUNGEON_LinkThingToList list-append: "
    "P0287_T_ThingToLink->Next forced to C0xFFFE_THING_ENDOFLIST, then "
    "F0159 walks P0288_T_ThingInList until C0xFFFE_THING_ENDOFLIST, "
    "last walked thing's Next overwritten with P0287_T_ThingToLink); "
    "DUNGEON.C F0159:1664-1681 (DUNGEON_GetNextThing: returns the Next "
    "field verbatim); "
    "CHAMPION.C F0297:243-268 (leader-hand put: store Thing, derive "
    "icon with F0033, add F0140 weight, mark load); "
    "CHAMPION.C F0298:270-298 (leader-hand remove); "
    "CHAMPION.C F0300:511-515 (G0425 remove); F0301:606-614 (G0425 "
    "write + load); F0302:688-710 (leader hand + destination slot click "
    "dispatcher)"
)
NEEDLES_T = [
    "F0333", "F0334", "F0159", "F0163",
    "F0297", "F0298", "F0300", "F0301", "F0302",
    "G0425", "G0426",
    "C0xFFFE_THING_ENDOFLIST", "C0xFFFF_THING_NONE",
    "stack-merge", "stack_merge", "close_stack_merge",
    "list-append", "chain-merge"
]
NEEDLES_H = [
    "close_stack_merge",
    "CHEST_SLOT_COUNT"
]
NEEDLES_C = [
    "test_dm1_v1_chest_close_stack_merge_pc34_compat",
    "src/dm1/dm1_v1_chest_close_stack_merge_pc34_compat.c",
    "NAME dm1_v1_chest_close_stack_merge_pc34_compat",
    f"verify_{PASS}",
]


def check_needles(label, path, needles):
    if not path.exists():
        return [f"missing {label}: {path}"]
    text = path.read_text()
    missing = [n for n in needles if n not in text]
    return [f"{label} missing: {missing}"] if missing else []


def resolve_build_dir(binary_name=""):
    candidates = [ROOT / "build", ROOT / "builds" / "nv1-build", ROOT / "builds" / "n2-build"]
    if binary_name:
        for c in candidates:
            if (c / "CMakeCache.txt").exists() and (c / binary_name).exists():
                return c
    for c in candidates:
        if (c / "CMakeCache.txt").exists():
            return c
    return candidates[0]


def main():
    failures = []
    for label, path, needles in [
        ("header", HDR, NEEDLES_H),
        ("test_source", TEST, NEEDLES_T),
        ("cmake_registration", CMAKE, NEEDLES_C),
    ]:
        failures.extend(check_needles(label, path, needles))

    binary_name = "test_dm1_v1_chest_close_stack_merge_pc34_compat"
    build_dir = resolve_build_dir(binary_name)
    binary = build_dir / binary_name
    if not binary.exists():
        failures.append(f"binary not found: {binary}")
        passes = 0
        fails = 0
    else:
        proc = subprocess.run([str(binary)], capture_output=True, text=True, timeout=60)
        passes = proc.stdout.count("PASS ")
        fails = proc.stdout.count("FAIL ")
        if proc.returncode != 0 or fails > 0:
            failures.append(f"runtime: exit={proc.returncode} passes={passes} fails={fails}")
        else:
            print(f"{PASS}: PASS passes={passes}")

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = {
        "pass": PASS,
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "redmcsb": RED,
        "tests": {"passes": passes if not failures else 0, "fails": fails if not failures else 0},
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2))
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(
        f"# {PASS}\n\nSource-locked: {RED}\n\nResult: {manifest['status']}\n"
        f"Tests: {manifest['tests']}\nFailures: {failures}\n"
    )
    return 0 if not failures else 1


if __name__ == "__main__":
    sys.exit(main())
