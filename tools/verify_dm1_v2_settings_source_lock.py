#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v2_settings_source_lock.json"

REQUIRED_SOURCE = [
    (SOURCE / "DUNVIEW.C", "F0128_DUNGEONVIEW_Draw_CPSF", 8318),
    (SOURCE / "DUNVIEW.C", "M100_PIXEL_WIDTH(G0296_puc_Bitmap_Viewport) = G2073_C224_ViewportPixelWidth", 2999),
    (SOURCE / "DUNVIEW.C", "M101_PIXEL_HEIGHT(G0296_puc_Bitmap_Viewport) = G2074_C136_ViewportHeight", 3000),
    (SOURCE / "GAMELOOP.C", "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)", 90),
]

REQUIRED_FIRESTAFF = [
    (ROOT / "src/dm1v2/dm1_v2_settings_pc34.c", "ReDMCSB primary anchor DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF"),
    (ROOT / "src/dm1v2/dm1_v2_settings_pc34.c", "V2 settings may scale/present that picture"),
    (ROOT / "src/dm1v2/dm1_v2_settings_pc34.c", "DEFS.H:238-243 and CLIKMENU.C:142-290"),
    (ROOT / "src/dm1v2/dm1_v2_settings_pc34.c", "V1 behavior is not read from these fields"),
    (ROOT / "src/dm1v2/dm1_v2_settings_pc34.c", "Phase 1 presentation scaffold rule"),
    (ROOT / "src/dm1v2/dm1_v2_settings_pc34.c", "dm1_v2_settings_apply_v21_presentation_defaults"),
    (ROOT / "src/dm1v2/dm1_v2_settings_pc34.c", "v2_settings_save_to_file"),
    (ROOT / "src/dm1v2/dm1_v2_settings_pc34.c", "v2_settings_load_from_file"),
    (ROOT / "src/dm1v2/dm1_v2_settings_pc34.c", "DM1_V2_ASPECT_ORIGINAL_4_3"),
    (ROOT / "src/dm1v2/dm1_v2_settings_pc34.c", "DM1_V2_ASPECT_WIDESCREEN_16_9"),
    (ROOT / "tests/test_dm1_v2_settings_pc34.c", "V1 remains the default presentation mode"),
    (ROOT / "tests/test_dm1_v2_settings_pc34.c", "dm1_v2_scale_percent = 250"),
    (ROOT / "tests/test_dm1_v2_settings_pc34.c", "scale_percent=400"),
    (ROOT / "include/dm1_v2_settings_pc34.h", "v2_settings_save_to_file"),
    (ROOT / "include/dm1_v2_settings_pc34.h", "v2_settings_load_from_file"),
    (ROOT / "src/engine/config_m12.c", "config->graphicsIndex = 0"),
    (ROOT / "src/engine/config_m12.c", "config->dm1V2ScalePercent = 100"),
    (ROOT / "src/engine/config_m12.c", "dm1_v2_aspect_mode"),
    (ROOT / "CMakeLists.txt", "NAME dm1_v2_settings_pc34"),
    (ROOT / "CMakeLists.txt", "NAME dm1_v2_settings_source_lock"),
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
    text = path.read_text(encoding="utf-8", errors="replace")
    if needle not in text:
        errors.append(f"missing Firestaff settings source-lock text {needle} in {path.name}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "dm1_v2_settings_pc34 Phase-1 presentation scaffold settings persistence source-lock",
    "evidenceImpact": {
        "completionMatrixGap": "Phase 1 needs deterministic V2 presentation config carried beside V1 gameplay state; this gate ties the V2-only settings path back to ReDMCSB viewport/command invariants and V1-safe persistence.",
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
print(f"dm1_v2_settings_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}")
