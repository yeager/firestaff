#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
source_root = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
evidence_path = root / "parity-evidence/verification/dm1_v2_filter_chain_source_lock.json"

source_checks = [
    {
        "file": "PALETTE.C",
        "start": 46,
        "end": 60,
        "needles": ["void F1122_", "*L2630_pi_++ = *P2812_pi_ColorPalette++;"],
        "meaning": "source palette transfer copies owned palette entries directly",
    },
    {
        "file": "VIEWPORT.C",
        "start": 30,
        "end": 46,
        "needles": ["void F0565_VIEWPORT_SetPalette", "F0566_VIEWPORT_BlitToScreen();"],
        "meaning": "source viewport path sets its palette then presents the viewport",
    },
]

required = {
    "src/dm1v2/dm1_v2_filter_crt_scanlines_pc34.c": ["No scanline overlay", "return 0;"],
    "src/dm1v2/dm1_v2_filter_dither_cleanup_pc34.c": ["no host 3x3 mode-filter route", "return 0;"],
    "src/dm1v2/dm1_v2_filter_palette_interpolate_pc34.c": ["Do not invent intermediate", "return 0;"],
    "src/dm1v2/dm1_v2_filter_sharpen_pc34.c": ["No PC34 unsharp-mask pass", "return 0;"],
    "src/dm1v2/dm1_v2_filter_palette_correct_pc34.c": [
        "Preserve the exact authenticated VGA table",
        "memcpy(out_lut, G9010_auc_VgaPaletteAll_Compat",
    ],
    "tests/test_dm1_v2_filter_chain_pc34.c": [
        "memcmp(indexed, indexed_before",
        "memcmp(rgba, rgba_before",
        "memcmp(lut, G9010_auc_VgaPaletteAll_Compat",
    ],
}

forbidden = {
    "src/dm1v2/dm1_v2_filter_crt_scanlines_pc34.c": ["gain_num", "row[x * 4"],
    "src/dm1v2/dm1_v2_filter_dither_cleanup_pc34.c": ["scratch[", "counts[16]", "mode_count"],
    "src/dm1v2/dm1_v2_filter_palette_interpolate_pc34.c": ["scratch[", "s_canon_midpoint", "blended4"],
    "src/dm1v2/dm1_v2_filter_sharpen_pc34.c": ["scratch[", "clamp_byte", "orig - blur"],
    "src/dm1v2/dm1_v2_filter_palette_correct_pc34.c": ["pow(", "gamma curve", "contrast around"],
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
            errors.append(f"{relative} retains synthetic filter implementation: {needle}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "dm1_v2 V2.0 post-process filter chain",
    "redmcsbSourceRoot": str(source_root),
    "anchors": anchors,
    "firestaffFiles": sorted(required),
    "forbiddenSyntheticRules": forbidden,
    "errors": errors,
}
evidence_path.parent.mkdir(parents=True, exist_ok=True)
evidence_path.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

if errors:
    print("dm1_v2_filter_chain_source_lock=FAIL")
    for error in errors:
        print(error)
    sys.exit(1)

print(f"dm1_v2_filter_chain_source_lock=OK evidence={evidence_path.relative_to(root)}")
