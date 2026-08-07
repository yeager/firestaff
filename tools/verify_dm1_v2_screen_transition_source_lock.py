#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
source_root = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
evidence_path = root / "parity-evidence/verification/dm1_v2_screen_transition_source_lock.json"

source_checks = [
    {
        "file": "GAMELOOP.C",
        "start": 84,
        "end": 96,
        "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)"],
        "meaning": "PC34 passes the committed party tuple directly to the dungeon-view composer",
    },
    {
        "file": "DUNVIEW.C",
        "start": 8318,
        "end": 8332,
        "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF("],
        "meaning": "F0128 is the source-owned dungeon-view composition entry point",
    },
    {
        "file": "DRAWVIEW.C",
        "start": 709,
        "end": 718,
        "needles": ["void F0097_DUNGEONVIEW_DrawViewport("],
        "meaning": "F0097 is the source-owned viewport presentation entry point",
    },
]

required = {
    "src/dm1v2/dm1_v2_screen_transition_pc34.c": [
        "No source-owned transition\n * layer was found",
        "memcpy(dst, src, (size_t)w * (size_t)h)",
        "return false;",
        "return 0.0f;",
        "return 1;",
    ],
    "include/dm1_v2_screen_transition_pc34.h": [
        "Compatibility-only API.",
        "copy source pixels unchanged",
        "void v2_screen_transition_start(int kind, float duration_ms);",
    ],
    "tests/test_dm1_v2_screen_transition_pc34.c": [
        "FADE_BLACK",
        "WIPE_LEFT",
        "copied_unchanged",
        "v22_screen_fade_alpha() == 0.0f",
    ],
}

forbidden = [
    "V2_Anim",
    "V2_EASE_IN_OUT_QUAD",
    "case FADE_BLACK:",
    "case FADE_WHITE:",
    "case WIPE_LEFT:",
    "case WIPE_DOWN:",
    "g_trans",
    "g_screen_fade",
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

transition_text = (root / "src/dm1v2/dm1_v2_screen_transition_pc34.c").read_text(encoding="utf-8")
for needle in forbidden:
    if needle in transition_text:
        errors.append(f"screen-transition bridge must not retain synthetic transition state or pixel rules: {needle}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "dm1_v2_screen_transition compatibility bridge",
    "redmcsbSourceRoot": str(source_root),
    "anchors": anchors,
    "firestaffFiles": sorted(required),
    "forbiddenSyntheticState": forbidden,
    "errors": errors,
}
evidence_path.parent.mkdir(parents=True, exist_ok=True)
evidence_path.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

if errors:
    print("dm1_v2_screen_transition_source_lock=FAIL")
    for error in errors:
        print(error)
    sys.exit(1)

print(f"dm1_v2_screen_transition_source_lock=OK evidence={evidence_path.relative_to(root)}")
