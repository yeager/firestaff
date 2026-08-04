# Pass 1118 -- DM2 V1 mouse_cursor_graphics (c_mcursor.cpp)

## What

Port 3 functions from skproject c_mcursor.cpp into Firestaff using
callback-based architecture with receipt structs for testability.

## Source

- skproject: SKULLWIN/c_mcursor.cpp

## Implemented functions

See include/dm2_v1_mcursor_pc34_compat.h for full API.

## Verification

- `build/test_dm2_v1_mcursor_pc34_compat`: rc=0
