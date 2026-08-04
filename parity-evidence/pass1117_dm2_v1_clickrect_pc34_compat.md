# Pass 1117 -- DM2 V1 click_rectangle_zones (c_clickrect.cpp)

## What

Port 5 functions from skproject c_clickrect.cpp into Firestaff using
callback-based architecture with receipt structs for testability.

## Source

- skproject: SKULLWIN/c_clickrect.cpp

## Implemented functions

See include/dm2_v1_clickrect_pc34_compat.h for full API.

## Verification

- `build/test_dm2_v1_clickrect_pc34_compat`: rc=0
