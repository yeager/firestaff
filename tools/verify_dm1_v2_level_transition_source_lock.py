#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
source_root = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
evidence_path = root / "parity-evidence/verification/dm1_v2_level_transition_source_lock.json"

source_checks = [
    {
        "file": "GAMELOOP.C",
        "start": 84,
        "end": 96,
        "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)"],
        "meaning": "main loop draws the committed party tuple directly through F0128",
    },
    {
        "file": "DUNGEON.C",
        "start": 1371,
        "end": 1391,
        "needles": ["P0256_pi_MapX", "P0257_pi_MapY", "DirectionToStepEastCount"],
        "meaning": "source dungeon movement owns party-coordinate changes",
    },
]

required = {
    "src/dm1v2/dm1_v2_level_transition_pc34.c": [
        "no independent V2 layer",
        "return false;",
        "return 0.0f;",
        "return 0;",
    ],
    "include/dm1_v2_level_transition_pc34.h": [
        "retains no transition state",
        "never\n * alters a source framebuffer",
    ],
    "tests/test_dm1_v2_level_transition_pc34.c": [
        "memcmp(framebuffer, before",
        "TRANS_PIT_FALL",
        "v22_transition_duration_for_type(4) == 0.0f",
    ],
}

forbidden = [
    "g_transition",
    "g_level_trans",
    "TRANS_STAIRS_DOWN:",
    "TRANS_PIT_FALL:",
    "TRANS_TELEPORT:",
    "sqrtf(",
    "V22_TRANS_STAIRS_TICKS",
    "V22_TRANS_PIT_TICKS",
    "V22_TRANS_TELEPORT_TICKS",
    "V1_TICK_MS",
]

errors: list[str] = []
anchors = []
for check in source_checks:
    path = source_root / check["file"]
    if not path.exists():
        errors.append(f"missing ReDMCSB source {path}")
        continue
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    excerpt = "\n".join(lines[check["start"] - 1:check["end"]])
    for needle in check["needles"]:
        if needle not in excerpt:
            errors.append(f"{check['file']}:{check['start']}-{check['end']}: missing {needle!r}")
    anchors.append(check)

for relative, needles in required.items():
    path = root / relative
    if not path.exists():
        errors.append(f"missing Firestaff file {relative}")
        continue
    text = path.read_text(encoding="utf-8")
    for needle in needles:
        if needle not in text:
            errors.append(f"{relative}: missing {needle!r}")

text = (root / "src/dm1v2/dm1_v2_level_transition_pc34.c").read_text(encoding="utf-8")
for needle in forbidden:
    if needle in text:
        errors.append(f"level-transition bridge retains synthetic animation state or pixels: {needle}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "dm1_v2 level-transition compatibility bridge",
    "redmcsbSourceRoot": str(source_root),
    "anchors": anchors,
    "firestaffFiles": sorted(required),
    "forbiddenSyntheticRules": forbidden,
    "errors": errors,
}
evidence_path.parent.mkdir(parents=True, exist_ok=True)
evidence_path.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

if errors:
    print("dm1_v2_level_transition_source_lock=FAIL")
    for error in errors:
        print(error)
    sys.exit(1)

print(f"dm1_v2_level_transition_source_lock=OK evidence={evidence_path.relative_to(root)}")
