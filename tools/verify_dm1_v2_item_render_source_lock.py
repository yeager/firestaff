#!/usr/bin/env python3
"""Gate DM1 V2 item-render bindings against ReDMCSB object draw source."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v2_item_render_source_lock.json"

REQUIRED_SOURCE = [
    (SOURCE / "DUNVIEW.C", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", 4547),
    (SOURCE / "DUNVIEW.C", "L0135_B_DrawAlcoveObjects = !(L0130_ul_RemainingViewCellOrdinalsToProcess = P0146_ui_OrderedViewCellOrdinals)", 4800),
    (SOURCE / "DUNVIEW.C", "/* Draw objects */", 4820),
    (SOURCE / "DUNVIEW.C", "AL0126_i_ViewCell = M001_ORDINAL_TO_INDEX", 4826),
    (SOURCE / "DUNVIEW.C", "M011_CELL(P0141_T_Thing) == L0139_i_Cell", 4923),
    (SOURCE / "DUNVIEW.C", "G0292_aT_PileTopObject[AL0126_i_ViewCell] = P0141_T_Thing", 5178),
    (SOURCE / "DUNVIEW.C", "/* Draw creatures */", 5201),
]

REQUIRED_FIRESTAFF = [
    (ROOT / "src/dm1v2/dm1_v2_item_render_pc34.c", "ReDMCSB DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"),
    (ROOT / "src/dm1v2/dm1_v2_item_render_pc34.c", "draws open-square objects before the"),
    (ROOT / "src/dm1v2/dm1_v2_item_render_pc34.c", "include/dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"),
    (ROOT / "src/dm1v2/dm1_v2_item_render_pc34.c", "dm1_v2_hud_interaction_pc34 bridge"),
    (ROOT / "tests/test_dm1_v2_item_render_pc34.c", "test_source_locked_cell_layer_order"),
    (ROOT / "tests/test_dm1_v2_item_render_pc34.c", "firestaff-v2-wave1-items-starter.manifest.json"),
    (ROOT / "assets-v2/items/wave1/specs/starter-icons.md", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"),
    (ROOT / "assets-v2/items/wave1/specs/starter-icons.md", "G0219"),
    (ROOT / "CMakeLists.txt", "NAME dm1_v2_item_render_pc34"),
    (ROOT / "CMakeLists.txt", "NAME dm1_v2_item_render_source_lock"),
]

errors: list[str] = []
anchors = []
for path, needle, line in REQUIRED_SOURCE:
    if not path.exists():
        errors.append(f"missing ReDMCSB source file {path}")
        continue
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    if needle not in text:
        errors.append(f"missing {needle} in {path.name}")
    if not (1 <= line <= len(lines)):
        errors.append(f"line out of range {path.name}:{line}")
    else:
        anchors.append({"file": path.name, "line": line, "needle": needle, "text": lines[line - 1].strip()})

for path, needle in REQUIRED_FIRESTAFF:
    if not path.exists():
        errors.append(f"missing Firestaff file {path.relative_to(ROOT)}")
        continue
    text = path.read_text(encoding="utf-8", errors="replace")
    if needle not in text:
        errors.append(f"missing Firestaff item-render source-lock text {needle} in {path.relative_to(ROOT)}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "dm1_v2_item_render_pc34 Phase-5 item render source-lock and CTest wiring",
    "evidenceImpact": {
        "completionMatrixGap": "test_dm1_v2_item_render_pc34 and item starter manifest existed but were not wired into CTest/source-lock ownership; dm1_v2_item_render_pc34 was still classified as an orphan V2 module.",
        "verifiedCompletionPercent": None,
    },
    "anchors": anchors,
    "errors": errors,
}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
if errors:
    for error in errors:
        print("error:", error)
    raise SystemExit(1)
print(f"dm1_v2_item_render_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}")
