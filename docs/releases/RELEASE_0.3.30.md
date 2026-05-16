## Firestaff v0.3.30

DM1 V1 source-lock preview release focused on movement, keyboard routing, viewport order, and blocker evidence that landed after v0.3.29.

### Added
- Added DM1 V1 keyboard buffer and runtime route evidence, including the I34E route transcript contract and narrowed queue blocker checks.
- Added viewport source-lock gates for side occlusion, wall order, door-front ordering, draw-order metadata, and F0128 draw-order coverage.
- Added movement command evidence for blocked movement lifecycle, reserved-command discard, cooldown queue looping, passable doors, projectile direction movement, PC34 C254 capture boundaries, and stairs stamina ordering.
- Added CSB V1 runtime readiness and capture-boundary blocker gates.
- Added original overlay and route transcript blocker evidence so unresolved capture issues stay explicit.

### Fixed
- Fixed the DM1 V1 door-front metadata blocker.
- Kept quick-resume state out of normal DM1 launch.
- Made DM1 viewport source-lock gates main-build aware.

### Verified
- `git diff --check` clean on the landed release head.
- GitHub main CI passed on `main` before the release tag.
- Release workflow build and smoke tests reached packaging before this notes-file blocker was fixed.

Supported release artifacts are built by GitHub Actions for macOS, Windows, and Linux.
