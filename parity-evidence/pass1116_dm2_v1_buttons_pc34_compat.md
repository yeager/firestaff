# Pass 1116 -- DM2 V1 button_group_management (c_buttons.cpp)

## What

Port 6 functions from skproject c_buttons.cpp into Firestaff using
callback-based architecture with receipt structs for testability.

## Source

- skproject: SKULLWIN/c_buttons.cpp

## Implemented functions

See include/dm2_v1_buttons_pc34_compat.h for full API.

## Verification

- `build/test_dm2_v1_buttons_pc34_compat`: rc=0
