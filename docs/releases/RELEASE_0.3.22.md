## Firestaff v0.3.22

Verified DM1 V1 parity-gate macOS DMG release from current `main`.

### Added
- Source-locked DM1 V1 Elija route gate evidence.
- DM1 V1 viewport hand overlay compatibility gate.
- DM1 V1 viewport status bar layout compatibility gate.

### Packaging
- macOS DMG + app zip.
- Linux DEB + RPM via GitHub Actions.
- Windows zip + NSIS installer via GitHub Actions.
- SHA256 checksums included per platform.

### Verified
- `git diff --check` clean.
- Conflict marker scan clean.
- Strict secret scan clean.
- Sensitive filename scan clean.
- CMake Release configure/build.
- macOS smoke probes.
- Focused DM1 V1 viewport/status CTest gates passed.
