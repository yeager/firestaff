# Firestaff v3.0.34

Firestaff v3.0.34 packages the latest startup hardening on `main` after
v3.0.33.

## Highlights since v3.0.33

- **Nexus direct media startup**: Nexus startup now accepts direct ISO/BIN/CUE
  media requests through the same hash-verified scanner path used by normal
  data-directory launches.

- **Theron's Quest descriptor proof**: the Track 02 descriptor semantic probe
  is now wired into CMake and verifies the current descriptor-role binding
  entry point.

- **Hash-first direct file startup**: direct file requests now try the normal
  hash-verified scanner path before legacy explicit-file fallback, including
  DM1, CSB and DM2 ZIP/ISO/BIN cache materialization.

## Verification

- Local release-prep verification covered version synchronization before tag
  push.
- The GitHub release workflow builds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, Linux arm64 and Steam Deck x86_64 artifacts.
