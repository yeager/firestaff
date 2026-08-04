# Pass 1138 — DM2 V1 input missing functions (c_input.cpp)

## Source
skproject SKULLWIN/c_input.cpp — 3 missing functions added to existing module.

## Added functions
- DM2_ADJUST_UI_EVENT (c_input.cpp:112) — adjusts UI event idx based on hand cooldowns and item activability. For idx 116-123 (action hands): checks hero hand cooldown and IS_ITEM_HAND_ACTIVABLE. For idx 95-98 (movement arrows): checks player position and hand cooldowns. Sets idx=0 if unavailable.
- DM2_1031_03f2 (c_input.cpp:55) — recursive event table traversal. Walks table1d3ba0 entries matching the current bbw value, returns matching event index from table1d3d23/v1d39bc.
- DM2_0b36_129a (c_input.cpp:522) — draw string to button group bitmap during event execution. Uses QUERY_STR_METRICS for sizing, DRAW_STRING for rendering, ADJUST_BUTTONGROUP_RECTS for update.

## Files modified
- include/dm2_v1_event_handlers_pc34_compat.h (added callbacks, receipts, declarations)
- src/dm2/dm2_v1_event_handlers_pc34_compat.c (added implementations)

## Test
All tests pass (dm2_v1_event_handlers_pc34_compat).
