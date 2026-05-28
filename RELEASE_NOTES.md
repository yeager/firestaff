# Firestaff v2.5.4

Gameplay and launcher polish release on top of the v2.5.x release pipeline.

## What's New

- Fixed DM1 V1 Hall of Champions mirror routing, candidate vitals decoding, and reincarnate handling so mirror recruits no longer trigger an all-dead Game Over path.
- Added M12 touch gestures, refreshed launcher locales, and added Japanese and Simplified Chinese startup-menu resources.
- Added DM2 V2 HUD overlay and interaction feedback scaffolding with smoke coverage.
- Added DM1 V2 champion-select source-lock gates and HUD health-pulse support.

## Verification

- GitHub Actions verify workflow passed on `main` before release.
- Local CMake build completed.
- Phase A probe passed 21/21 invariants.
- DM2 V2 HUD overlay smoke test passed 76/76 checks.
- DM1 V1 resurrection and Hall mirror probes passed in the release line.

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
