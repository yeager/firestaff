#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
checks = {
    "src/dm1v2/dm1_v2_hud_interaction_pc34.c": [
        "COMMAND.C:375-395",
        "COMMAND.C:461-471",
        "COMMAND.C:484-497",
        "CLIKCHAM.C:24-35",
        "TOUCHCLICK_Compat_HitTestWithButton",
        "action_area_routes_GetTouchMatrixInvariant",
        "champion_name_hand_routes_GetInvariant",
    ],
    "include/dm1_v2_hud_interaction_pc34.h": [
        "M11_V2_HUD_TOUCH_CHAMPION_FOCUS_PC34",
        "M11_V2_HUD_TOUCH_ACTION_ICON_PC34",
        "v2_hud_interaction_dispatch_scaled_click",
    ],
    "tests/test_dm1_v2_hud_interaction_pc34.c": [
        "champion0.toggle_box",
        "champion2.name",
        "champion3.action_hand",
        "action.icon1",
        "action.row1",
    ],
}

errors = []
for rel, needles in checks.items():
    text = (root / rel).read_text(encoding="utf-8")
    for needle in needles:
        if needle not in text:
            errors.append(f"{rel}: missing {needle!r}")

# Guard the narrow V2-only slice: V1 route files are read as source locks, not modified by this gate.
for rel in ["src/dm1/dm1_v1_click_routing_pc34_compat.c", "src/shared/touch_click_zone_matrix_pc34_compat.c", "src/shared/champion_name_hand_routes_pc34_compat.c", "src/shared/action_area_routes_pc34_compat.c"]:
    if not (root / rel).exists():
        errors.append(f"missing source-lock dependency {rel}")

if errors:
    print("dm1_v2_hud_interaction_source_lock=FAIL")
    for err in errors:
        print(err)
    sys.exit(1)

print("dm1_v2_hud_interaction_source_lock=OK")
