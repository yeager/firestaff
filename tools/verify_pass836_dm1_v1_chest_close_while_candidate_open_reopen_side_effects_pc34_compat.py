#!/usr/bin/env python3
"""Verify pass836 — DM1 V1 chest close while candidate open reopen side effects."""
import os, subprocess, sys, json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PASS = "pass836_dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat"
HDR = ROOT / "include/dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    "CHEST.C F0333:30-67 (F0333_INVENTORY_OpenAndDrawChest: materializes "
    "G0425 from the open G0426 chest); "
    "CHEST.C F0334:117-132 (F0334_INVENTORY_CloseChest: clears G0426, "
    "rewires non-empty G0425 into container Slot list); "
    "CHAMPION.C F0284:93-131 (keeps champion ownership independent from "
    "party direction updates); "
    "CHAMPION.C F0297:243-268 (leader-hand put); F0298:270-298 "
    "(leader-hand remove); F0300:511-515 (clears C30+ and champion "
    "slots); F0301:606-614 (writes C30+/champion slots); F0302:662-714 "
    "(snapshots leader hand and selected slot before remove/put exchange); "
    "PANEL.C F0344:1390-1406 (inactive food/water, eye, and "
    "resurrect-panel side routes); "
    "OBJECT.C F0033:147-212 (maps object identity to visible chest icons); "
    "BLITMASK.C F0133:30-33 (transparency blit dispatch used by reopen redraws); "
    "DEFS.H:2088 C10_COLOR_FLESH; 810-816 C30..C36; 1874-1878 C38; 2200 "
    "C040; 3001-3008 M568/M569; 3906-3913 C537..C544; 5694 G0299; "
    "5876-5881 G0425/G0426"
)
NEEDLES_T = [
    "F0333", "F0334",
    "F0284", "F0297", "F0298", "F0300", "F0301", "F0302",
    "F0344", "F0345", "F0352", "F0354",
    "F0033", "F0077", "F0078", "F0133",
    "G0425", "G0426", "G0299",
    "C025", "C040", "C10", "C30", "C34", "C36", "C38",
    "C537", "C544",
    "candidate", "reopen", "close_while"
]
NEEDLES_H = [
    "G0426", "G0299",
    "C025", "C040", "C10", "C30", "C34", "C38",
    "C537", "C544",
    "candidate", "reopen"
]
NEEDLES_C = [
    "test_dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat",
    "src/dm1/dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat.c",
    "NAME dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat",
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

    binary_name = "test_dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat"
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
