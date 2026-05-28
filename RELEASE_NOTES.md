# Firestaff v2.5.5

V2 expansion and DM1 launcher/gameplay polish release on top of the v2.5.x release pipeline.

## What's New

- Restored DM1 Hall of Champions mirror portraits and tightened the startup/title path so the intro remains visible at launch.
- Added CSB V2 enhanced lighting, touch/controller affordance coverage, smooth-movement bridge work, and a V2 verification suite gate.
- Added DM2 V1 utility/import flow coverage for new game, session save/load, and starter party behavior.
- Added DM2 V2 enhanced lighting/outdoor effects and Nexus V2 HUD overlay sources with smoke coverage.
- Fixed DM1 viewport/blocker polish, including viewport border walls, inscription rendering, save-and-quit dialog clicks, and hidden Hall mirror payload items.
- Fixed the Windows CI save/load test process-id helper used by the release verification matrix.

## Verification

- GitHub Actions verify workflow passed on `main` before release.
- Local CMake configure/build completed.
- Phase A probe passed.
- Audio probe passed.
- DM2 V2, CSB V2, and Nexus V2 smoke/source tests are included in the release line.

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
