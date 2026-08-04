# Pass 1136 — DM2 V1 gfx_main graphics operations (c_gfx_main.cpp)

## Source
skproject SKULLWIN/c_gfx_main.cpp — 16 functions for main graphics operations.

## Ported functions
- c_gfx_system::init — graphics system state init (backbuffer 224x136, screen 320x200)
- DM2_INIT_BACKBUFF — initialize back buffer header
- DM2_DRAW_GAMELOAD_DIALOGUE_TO_SCREEN — draw game load dialog
- DM2_gfxmain_3929_0914 — message line management (scroll on overflow)
- DM2_gfxmain_3929_0929 — draw text to message area
- DM2_gfxmain_0b36_0cbe — blit button group rects to screen
- DM2_FILL_BACKBUFF_RECT — fill backbuffer rectangle
- DM2_FILL_SCREEN_RECT — fill screen rectangle by query
- DM2_FILL_FULLSCREEN — fill full screen rectangle
- DM2_FILL_ENTIRE_PICT — fill entire bitmap
- DM2_FILL_HALFTONE_RECTV — checkerboard halftone pattern fill
- DM2_FILL_HALFTONE_RECTI — halftone fill by rect query
- DM2_FADE_SCREEN — fade screen (palette set select)
- blit_toscreen — blit to screen with disable_video counter
- _specialblit — special blit with optional stretch effect
- DM2_DRAWINGS_COMPLETED — finalize viewport drawing

## Files
- include/dm2_v1_gfx_main_pc34_compat.h
- src/dm2/dm2_v1_gfx_main_pc34_compat.c
- tests/test_dm2_v1_gfx_main_pc34_compat.c

## Test
All tests pass (dm2_v1_gfx_main_pc34_compat).
