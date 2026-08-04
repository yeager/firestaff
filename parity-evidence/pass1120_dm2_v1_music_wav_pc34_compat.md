# Pass 1120 -- DM2 V1 wav_music_playback (c_music_wav.cpp)

## What

Port 2 functions from skproject c_music_wav.cpp into Firestaff using
callback-based architecture with receipt structs for testability.

## Source

- skproject: SKULLWIN/c_music_wav.cpp

## Implemented functions

See include/dm2_v1_music_wav_pc34_compat.h for full API.

## Verification

- `build/test_dm2_v1_music_wav_pc34_compat`: rc=0
