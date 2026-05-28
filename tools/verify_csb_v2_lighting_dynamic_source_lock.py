#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
EVIDENCE = ROOT / "parity-evidence/verification/csb_v2_lighting_dynamic_source_lock.json"

REQUIRED_SOURCE = [
    (SOURCE / "PANEL.C", "G0304_i_DungeonViewPaletteIndex = 0", (367, 367)),
    (SOURCE / "PANEL.C", "Get torch light power from both hands of each champion in the party", (370, 387)),
    (SOURCE / "PANEL.C", "G0407_s_Party.MagicalLightAmount", (417, 417)),
    (SOURCE / "PANEL.C", "G0304_i_DungeonViewPaletteIndex = AL1039_ui_PaletteIndex", (418, 428)),
    (SOURCE / "DATA.C", "G0040_ai_Graphic562_PaletteIndexToLightAmount[6] = { 99, 75, 50, 25, 1, 0 }", (359, 360)),
]

REQUIRED_FIRESTAFF = [
    (ROOT / "include/csb_v2_lighting_dynamic.h", "CSB_V2_SourcePaletteLighting"),
    (ROOT / "src/csb/csb_v2_lighting_dynamic.c", "presentation-only"),
    (ROOT / "src/csb/csb_v2_lighting_dynamic.c", "PANEL.C:367-428"),
    (ROOT / "src/csb/csb_v2_lighting_dynamic.c", "PANEL.C:370-405"),
    (ROOT / "src/csb/csb_v2_lighting_dynamic.c", "DATA.C:359-360"),
    (ROOT / "src/csb/csb_v2_viewport_renderer.c", "ReDMCSB PANEL.C:367-428"),
    (ROOT / "tests/test_csb_v2_lighting_dynamic.c", "squared distance falloff"),
    (ROOT / "tests/test_csb_v2_lighting_dynamic.c", "radius boundary is exclusive"),
]

errors = []
anchors = []

for path, needle, line_range in REQUIRED_SOURCE:
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    if needle not in text:
        errors.append(f"missing {needle} in {path.name}")
    start, end = line_range
    if not (1 <= start <= end <= len(lines)):
        errors.append(f"line range out of range {path.name}:{start}-{end}")
    else:
        anchors.append({
            "file": path.name,
            "lineRange": f"{start}-{end}",
            "needle": needle,
            "text": lines[start - 1].strip(),
        })

for path, needle in REQUIRED_FIRESTAFF:
    text = path.read_text(encoding="utf-8", errors="replace")
    if needle not in text:
        errors.append(f"missing Firestaff CSB V2 lighting source-lock text {needle} in {path.name}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "csb_v2_lighting_dynamic presentation-only source-lock",
    "anchors": anchors,
    "errors": errors,
}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
if errors:
    for error in errors:
        print("error:", error)
    raise SystemExit(1)
print(f"csb_v2_lighting_dynamic_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}")
