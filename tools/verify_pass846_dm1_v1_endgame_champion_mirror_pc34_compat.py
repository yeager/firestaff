#!/usr/bin/env python3
"""DM1 V1 endgame_champion_mirror contract (PASS846)."""
from __future__ import annotations

import json, subprocess, sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass846_dm1_v1_endgame_champion_mirror_pc34_compat"
STATUS = "PASS846_DM1_V1_ENDGAME_CHAMPION_MIRROR_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_endgame_champion_mirror_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/endgame_champion_mirror_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_endgame_champion_mirror_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / (PASS + ".md")
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

ANCHORS = [
    "DATA.C:21/149/569",
    "ENDGAME.C:347/389/390",
]

LOCAL_NEEDLES = [
    "dm1_v1_endgame_champion_mirror_table_pc34",
    "dm1_v1_endgame_champion_mirror_get_pc34",
    "dm1_v1_endgame_champion_mirror_x_pc34",
    "dm1_v1_endgame_champion_mirror_y_pc34",
    "dm1_v1_endgame_champion_mirror_w_pc34",
    "dm1_v1_endgame_champion_mirror_h_pc34",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_endgame_champion_mirror_pc34_compat",
    "src/dm1/dm1_v1_endgame_champion_mirror_pc34_compat.c",
    "NAME dm1_v1_endgame_champion_mirror_pc34_compat",
    "verify_" + PASS,
]

def read(p):
    enc = 'latin-1' if str(p).startswith(str(RED)) else 'utf-8'
    return p.read_text(encoding=enc, errors='replace')

def check_needles(label, p, needles):
    text = read(p)
    missing = [n for n in needles if n not in text]
    return {'id': label, 'file': str(p.relative_to(ROOT)), 'status': 'PASS' if not missing else 'FAIL', 'missing': missing}

def run(cmd):
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
    return {'command': cmd, 'returncode': p.returncode, 'passed': p.returncode == 0, 'outputTail': chr(10).join(p.stdout.strip().splitlines()[-10:])}

def main():
    checks = [
        check_needles('source', SRC, ANCHORS + LOCAL_NEEDLES),
        check_needles('header', HDR, ['dm1_v1_endgame_champion_mirror_table_pc34']),
        check_needles('test', TEST, ['test_table_values', 'test_components_non_negative']),
        check_needles('cmake', CMAKE, CMAKE_NEEDLES),
    ]
    runs = [run([str(ROOT / "build" / "test_dm1_v1_endgame_champion_mirror_pc34_compat")])]
    ok = all(c['status'] == 'PASS' for c in checks) and all(r['passed'] for r in runs)
    manifest = {
        'schema': 'firestaff.parity.' + PASS + '.v1',
        'status': STATUS if ok else 'FAILED_' + STATUS,
        'timestampUtc': datetime.now(timezone.utc).isoformat(),
        'scope': 'DM1 V1 endgame_champion_mirror contract.',
        'anchors': ANCHORS,
        'redmcsbRoot': str(RED),
        'sourceChecks': checks,
        'verificationRuns': runs,
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + chr(10))
    REPORT.write_text('# ' + PASS + chr(10) + chr(10) + '- Status: ' + manifest['status'] + chr(10) + '- Anchors: ' + ', '.join(ANCHORS) + chr(10))
    print(PASS + ': ' + ('PASS' if ok else 'FAIL'))
    print('manifest=' + str(MANIFEST.relative_to(ROOT)))
    for r in runs: print(r['outputTail'])
    return 0 if ok else 1

if __name__ == "__main__": raise SystemExit(main())
