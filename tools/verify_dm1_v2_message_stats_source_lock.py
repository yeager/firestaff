#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
source_root = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
evidence_path = root / "parity-evidence/verification/dm1_v2_message_stats_source_lock.json"

source_checks = [
    {
        "file": "CHAMPION.C",
        "start": 882,
        "end": 904,
        "needles": ["F0303_CHAMPION_GetSkillLevel", "L0918_ps_Skill->Experience += P0638_ui_Experience"],
        "meaning": "source experience and statistics remain owned by the live champion record",
    },
    {
        "file": "PANEL.C",
        "start": 1965,
        "end": 1983,
        "needles": ["F0351_INVENTORY_DrawChampionSkillsAndStatistics", "L1094_ps_Champion"],
        "meaning": "source panel reads statistics from the selected champion rather than global totals",
    },
    {
        "file": "CHAMPION.C",
        "start": 979,
        "end": 1004,
        "needles": ["F0047_TEXT_MESSAGEAREA_PrintMessage", "G0417_apc_BaseSkillNames"],
        "meaning": "source level messages enter the V1 message-area route",
    },
]

required = {
    "src/dm1v2/dm1_v2_message_log_pc34.c": ["must not retain a duplicate log", "(void)fb;"],
    "include/dm1_v2_message_log_pc34.h": ["retain no duplicate text or pixels"],
    "src/dm1v2/dm1_v2_stat_tracker_pc34.c": [
        "static const M11_V2_GameStats k_no_stats;",
        "No host-created totals may be persisted",
        "return -1;",
    ],
    "include/dm1_v2_stat_tracker_pc34.h": ["no global V2 totals record", "fail closed"],
    "tests/test_dm1_v2_message_stats_pc34.c": [
        "memcmp(framebuffer, before",
        "v2_stats_get(M11_V2_STAT_TOTAL_KILLS) == 0",
        "v2_stats_serialize(serialized",
    ],
}

forbidden = {
    "src/dm1v2/dm1_v2_message_log_pc34.c": ["k_log_font", "log_draw_glyph", "log_entries", "bg_val", "cat_color"],
    "src/dm1v2/dm1_v2_stat_tracker_pc34.c": ["g_stats", "total_kills +=", "memcpy(buf,"],
}

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

for relative, needles in forbidden.items():
    text = (root / relative).read_text(encoding="utf-8")
    for needle in needles:
        if needle in text:
            errors.append(f"{relative} retains synthetic log/stat state: {needle}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "dm1_v2 message-log and global-stat compatibility bridges",
    "redmcsbSourceRoot": str(source_root),
    "dmwebReference": "reference/dmweb-community-docs/html/community/documentation/dungeon-master-and-chaos-strikes-back/skills-and-statistics.html",
    "anchors": anchors,
    "firestaffFiles": sorted(required),
    "forbiddenSyntheticState": forbidden,
    "errors": errors,
}
evidence_path.parent.mkdir(parents=True, exist_ok=True)
evidence_path.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

if errors:
    print("dm1_v2_message_stats_source_lock=FAIL")
    for error in errors:
        print(error)
    sys.exit(1)

print(f"dm1_v2_message_stats_source_lock=OK evidence={evidence_path.relative_to(root)}")
