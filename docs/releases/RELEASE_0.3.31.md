## Firestaff v0.3.31

Release packaging unblocker for the DM1 V1 source-lock preview line.

### Fixed
- Fixed the preview release workflow so tag builds no longer fall back to a removed release-notes file.
- Added current release notes for the v0.3.30 content so macOS, Windows, and Linux packages can include a real notes file.
- Made the package scripts default to `README.md` when no explicit release-notes path is provided.

### Verified
- `git diff --check` clean.
- Release workflow YAML parses successfully.
- Dead `RELEASE_0_2_9_MAC_PREVIEW.md` references removed from workflow and packaging scripts.

Supported release artifacts are built by GitHub Actions for macOS, Windows, and Linux.
