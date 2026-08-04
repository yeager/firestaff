# Pass 1134 — DM2 V1 segment 1031 UI logic (c_1031.cpp)

## Source
skproject SKULLWIN/c_1031.cpp — 14 functions for UI click-rect management and mode switching.

## Ported functions
- dm2_v1_1031_gate — gate condition evaluation (8 cases: always, dialog2, v1e0204, hero alive, etc.)
- dm2_v1_1031_query_rect_with_offset — rect query with viewport/panel offset flags (0x8000/0x4000)
- dm2_v1_1031_mark_visible — mark click-rect entries visible for current mode
- dm2_v1_1031_hit_test — hit-test point against click-rect tables
- dm2_v1_1031_hit_test_clickrects — hit-test with gate filtering
- dm2_v1_1031_update_state — update UI state (mark visible, refresh)
- dm2_v1_1031_switch_mode — save current mode and switch to new mode
- dm2_v1_1031_restore_mode — restore previously saved mode
- dm2_v1_1031_reset_state — reset UI state variables

## Files
- include/dm2_v1_1031_pc34_compat.h
- src/dm2/dm2_v1_1031_pc34_compat.c
- tests/test_dm2_v1_1031_pc34_compat.c

## Test
All tests pass (dm2_v1_1031_pc34_compat).
