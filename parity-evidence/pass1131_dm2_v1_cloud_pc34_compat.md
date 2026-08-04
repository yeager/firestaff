# Pass 1131 — DM2 V1 cloud/weather rendering (c_cloud.cpp)

## Source
skproject SKULLWIN/c_cloud.cpp — 4 functions for cloud and weather rendering.

## Ported functions
- DM2_CLOUD_INIT — initialize cloud state with default parameters
- DM2_CLOUD_UPDATE — update cloud positions per tick (scroll/drift)
- DM2_CLOUD_DRAW — render cloud layer to backbuffer with transparency
- DM2_CLOUD_SET_WEATHER — set weather type affecting cloud density/speed

## Files
- include/dm2_v1_cloud_pc34_compat.h
- src/dm2/dm2_v1_cloud_pc34_compat.c
- tests/test_dm2_v1_cloud_pc34_compat.c

## Test
All tests pass (dm2_v1_cloud_pc34_compat).
