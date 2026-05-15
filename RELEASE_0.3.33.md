## Firestaff v0.3.33

DM1 V1 source-lock preview release for the latest movement stairs semantics and viewport far-side wall evidence after v0.3.32.

### Added
- Added DM1 V1 stairs enter-deferral reporting for cross-map stairs transitions.
- Added movement probes for stepping into stairs, moving backward while already on stairs, and turning on stairs during cooldown.
- Added DM1 V1 D3R2/D2 far-side wall and field-order evidence.
- Added viewport far-side wall clip source audit updates for D3R2 and D2L2/D2R2 ranges.

### Fixed
- Made the pass512 viewport source-audit verifier portable across N2, macOS, and CI data paths.
- Made regenerated pass512 evidence paths stable with repo-relative and $OPENCLAW_DATA references.

### Verified
- Local git diff --check passed before each landing.
- Local targeted movement and viewport builds/tests passed.
- GitHub main CI passed on the release head, including warnings, CMake smoke, macOS/Linux/Windows verify, and cross-platform determinism.
- Supported release artifacts are built by GitHub Actions for macOS, Windows, and Linux.
