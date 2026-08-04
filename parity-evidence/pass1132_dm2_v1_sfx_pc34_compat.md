# Pass 1132 — DM2 V1 sound effects processing (c_sfx.cpp)

## Source
skproject SKULLWIN/c_sfx.cpp — 9 functions for positional sound effects.

## Ported functions
- sfx_state_init — initialize SFX state (queues, counters)
- sfx_calc_volume_pan — compute volume and stereo pan from position
- sfx_calc_sound_distance — distance/occlusion calculation via view data (R_1FB7D)
- DM2_QUEUE_NOISE_GEN1 — queue positional sound with direction-relative panning
- DM2_QUEUE_NOISE_GEN2 — queue sound with fallback type routing

## Files
- include/dm2_v1_sfx_pc34_compat.h
- src/dm2/dm2_v1_sfx_pc34_compat.c
- tests/test_dm2_v1_sfx_pc34_compat.c

## Test
All tests pass (dm2_v1_sfx_pc34_compat).
