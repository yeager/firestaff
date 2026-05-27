# Firestaff v2.5.3

Maintenance release for the GitHub Actions release pipeline.

## What's New

- Fixed Windows/MSYS2 builds by using a portable Nexus save-directory `mkdir` wrapper.
- Fixed Nexus V1 tick dispatch to pass the mechanics state pointer with the correct type.
- Kept DM1 V1 title intro visibility, mouse-arrow routing, and pass504 capture-route preflight fixes in the release line.

## Verification

- Local CMake build completed.
- Phase A probe passed 21/21 invariants.
- Nexus boot-profile smoke test passed.
- Nexus V1 save/load round-trip probe passed 17/17 checks.

## Platforms

| Platform | Architecture | Format |
|----------|-------------|--------|
| macOS | arm64 + x86_64 | DMG, ZIP |
| Windows | x86_64 | ZIP, Installer (EXE) |
| Linux | x86_64 | DEB, RPM |
| Linux | ARM64 | DEB, RPM |

## Requirements

- Original game data files are user-supplied and not included.
- SDL3 runtime library.
