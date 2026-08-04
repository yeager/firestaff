# Pass 1129 — DM2 V1 xrect extended rectangle operations (c_xrect.cpp)

## Source
skproject SKULLWIN/c_xrect.cpp — 7 functions for extended rectangle operations.

## Ported functions
- DM2_QUERY_EXPANDED_RECT — expand compact rect with offset/size tables
- DM2_QUERY_TOPLEFT_OF_RECT — extract top-left corner from expanded rect
- DM2_FILL_BACKBUFF_RECT — fill backbuffer rectangle with pixel value
- DM2_BLIT_BACKBUFF_RECT — blit between backbuffer rectangles
- DM2_DRAW_RECT_BORDER — draw rectangle border (outline)
- DM2_DRAW_BACKBUFF_LINE — draw line in backbuffer
- DM2_DRAW_FILLED_RECT — draw filled rectangle with border

## Files
- include/dm2_v1_xrect_pc34_compat.h
- src/dm2/dm2_v1_xrect_pc34_compat.c
- tests/test_dm2_v1_xrect_pc34_compat.c

## Test
All tests pass (dm2_v1_xrect_pc34_compat).
