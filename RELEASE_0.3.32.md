## Firestaff v0.3.32

DM1 V1 source-lock preview release with the latest movement, viewport, touch, projectile, and original-capture verification gates after v0.3.31.

### Added
- Added DM1 V1 touch UI zone and touchscreen source-lock evidence.
- Added projectile travel blocker and projectile explosion render verification.
- Added front-cell collision source-lock evidence for movement parity.
- Added original dungeon packet boundary and original movement viewport transcript gates.
- Added viewport map resolver, draw-order map resolver, wall bucket order, and door composition occlusion gates.
- Added gated movement pending-click and movement pending-replay coverage.

### Verified
- GitHub main CI passed for the current release head before tagging.
- The previous release workflow fix was proven by the successful v0.3.31 macOS, Windows, and Linux artifact build.
- Supported release artifacts are built by GitHub Actions for macOS, Windows, and Linux.
