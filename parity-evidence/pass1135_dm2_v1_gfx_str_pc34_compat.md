# Pass 1135 — DM2 V1 gfx_str text/string rendering (c_gfx_str.cpp)

## Source
skproject SKULLWIN/c_gfx_str.cpp — 16+ functions for text rendering.

## Ported functions
- c_stringdata::init — string state initialization (defaults: gfxstrw1=6, gfxstrw2=1, gfxstrw3=1, gfxstrw4=6, strxplus=7)
- DM2_QUERY_FONT — decode 6x3 font glyph into pixel array
- DM2_QUERY_STR_METRICS — compute string pixel width and height
- DM2_DRAW_STRING — draw text to pixel buffer with palette
- DM2_DRAW_STRONG_TEXT — outlined text (3x DRAW_STRING: shadow, shadow+1, foreground)
- DM2_DRAW_BUTTON_STR — text on button via blit rect query
- DM2_DRAW_NAME_STR — name text with outlined style
- DM2_DRAW_VP_STR — viewport text with palette color 12
- DM2_DRAW_GUIDED_STR — word-wrapped text with line spacing
- DM2_PRINT_SYSERR_TEXT — direct screen text
- DM2_DRAW_VP_RC_STR — viewport text with rect query
- DM2_DRAW_LOCAL_TEXT — local strong text with palette
- DM2_FORMAT_SKSTR — recursive SK-encoded string format
- DM2_QUERY_GDAT_TEXT — GDAT text query with optional XOR decode
- DM2_DRAW_TEXT_TO_BACKBUFF — backbuffer text with palette setup
- DM2_gfxstr_3929_04e2 — word-wrap line extraction helper
- DM2_gfxstr_24a5_0732 — text with character translation (A-Z/a-z mapping)
- DM2_DISPLAY_HINT_TEXT — scrolling hint text with fill/wrap
- DM2_SCROLLBOX_MESSAGE — scroll message box with blit_within_screen

## Files
- include/dm2_v1_gfx_str_pc34_compat.h
- src/dm2/dm2_v1_gfx_str_pc34_compat.c
- tests/test_dm2_v1_gfx_str_pc34_compat.c

## Test
All tests pass (dm2_v1_gfx_str_pc34_compat).
