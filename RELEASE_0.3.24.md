## Firestaff v0.3.24

DM1 V1 focused macOS preview release with the corrected alcove item fix and the latest movement/viewport evidence from N2 worktrees.

### Fixed
- Corrected the v0.3.23 WALL-square item filter so real alcove items remain available while phantom floor sprites stay blocked.

### Added
- Refreshed movement route blocker manifests.
- Refreshed DM1 V1 movement/viewport parity gap evidence.
- Tightened DM1 V1 movement group-collision source-lock evidence.
- Added pass503 viewport wall draw-order evidence and verifier.
- Narrowed the pass435 original movement/viewport route blocker with fresh artifact evidence.

### Verified
- `git diff --check` clean.
- High-confidence secret scan clean.
- CMake Release configure/build on macOS.
- macOS DMG + app zip packaging.
- Focused movement, viewport, pass435, and alcove gates passed.
