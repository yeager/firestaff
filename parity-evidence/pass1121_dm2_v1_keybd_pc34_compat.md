# Pass 1121 -- DM2 V1 keyboard_input_handling (c_keybd.cpp)

## What

Port 8 functions from skproject c_keybd.cpp into Firestaff using
callback-based architecture with receipt structs for testability.

## Source

- skproject: SKULLWIN/c_keybd.cpp

## Implemented functions

See include/dm2_v1_keybd_pc34_compat.h for full API.

## Verification

- `build/test_dm2_v1_keybd_pc34_compat`: rc=0
