## Firestaff v0.3.34

DM1 V1 blocker-verification release focused on making the latest source-lock gates portable across N2, macOS, and GitHub Actions after the v0.3.33 evidence batch.

### Fixed
- Made DM1 V1 canonical movement and playable-route probes resolve canonical DM1 data from the active user home instead of an N2-only path.
- Made ReDMCSB-backed verifier scripts resolve source/data roots portably on macOS and N2.
- Updated the wall-composition contract probe for the expanded door-front occlusion matrix now present in the renderer metadata.
- Made pass558/pass563 C254 boundary verification consume local copied evidence when present, with N2 worktree evidence as fallback.
- Relaxed canonical README checks to verify named canonical files instead of requiring machine-specific absolute paths.

### Verified
- \`git diff --check\` passed.
- Targeted blocker regression set passed: 17 tests, 0 failures.
- \`main\` was pushed at commit \`d21ed14\`.
- Supported release artifacts are built by GitHub Actions for macOS, Windows, and Linux.

### Known Blocker
- pass462 title/end original capture parity remains incomplete and was not included as verified in this release.
