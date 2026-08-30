# Release Process

## Version Scheme

Firestaff uses semantic versioning: `MAJOR.MINOR.PATCH` (for example,
`3.0.315`). The version is set in `CMakeLists.txt`:

```cmake
project(Firestaff VERSION 3.0.315 LANGUAGES C)
```

## Release notes format

Each version block in `RELEASE_NOTES.md` contains only game and release-wide
categories that have a concrete delta. Valid game headings are `DM1`, `DM2`,
`CSB`, `Nexus`, and `Theron`; valid release-wide headings are `Added`,
`Changed`, `Removed`, and `Fixed`. A bullet names the affected function,
subsystem, command, screen, or feature in backticks. Empty categories are
omitted—`None.` is rejected by the release validator.

## Creating a release

1. **Bump version** in `CMakeLists.txt`.
2. **Write release notes** in `RELEASE_NOTES.md` (add a section headed
   `# Firestaff vX.Y.Z`). Include only the relevant game and release-wide
   categories, each with concrete bullets:

   ```markdown
   # Firestaff vX.Y.Z

   ## CSB
   ### Added
   - `csb_feature`: describe the concrete change.

   ## Firestaff
   ### Fixed
   - `release_feature`: describe the concrete release-wide fix.
   ```

   Name the affected function, subsystem, command, screen or feature in every
   bullet. Do not use aggregate wording such as “various updates” or generic
   release summaries. The release workflow rejects notes that do not meet this
   contract.
3. **Update active TODO and DONE ledgers** as needed: `TODO.md` for
   cross-game work, `TODO-<game>.md` for active game work, and the matching
   `DONE*.md` only after evidence-backed completion. Then run
   `scripts/sync_wiki.sh` in the release workflow to publish `docs/wiki/`.
4. **Commit** the version bump and documentation.
5. **Tag** with `v` prefix only when a release is explicitly requested.
6. **Push** the tag only for that release.

The GitHub Actions release workflow triggers automatically on `v*` tags.

Ordinary documentation and code work is pushed to `main` without creating a
tag or GitHub Release.

## CI/CD Pipeline

### Verify Workflow (`verify.yml`)

Runs on every push and PR. Tests on:
- Ubuntu 24.04
- macOS 14
- Windows 2022

Checks: build, test suite, compiler warnings, asset hygiene (no game data committed).

### Release Workflow (`release.yml`)

Triggers on `v*` tags or manual `workflow_dispatch`. Jobs:

1. **Preflight** — validates the version against `CMakeLists.txt` and verifies
   that the selected release-notes section names added, changed and removed
   functions/features before extracting it
2. **macOS** — builds x86_64 (Intel) and arm64 (Apple Silicon) in parallel
   - Produces: `.dmg`, `.zip`
   - Bundles: Artpack Studio, Dungeon Studio, Savegame Editor
   - Runs Phase A and Audio smoke probes
3. **Windows** — builds x86_64 via MSYS2 UCRT64
   - Produces: portable `.zip`, Inno Setup installer `.exe`
   - Bundles: Artpack Studio, Dungeon Studio, Savegame Editor
4. **Linux** — builds x86_64 and arm64 in parallel
   - Produces: `.deb`, `.rpm`
   - x86_64 also produces Steam Deck pacman `.pkg.tar.zst` and AppImage
   - SDL3 built from source (pinned to `release-3.2.14`)
5. **iOS** — builds arm64 on macOS 14
   - Cross-compiles with iOS toolchain (deployment target iOS 14.0)
   - SDL3 built as static library for iOS
   - Produces: `.ipa` (ad-hoc signed, AltStore Classic sideloadable)
6. **Android** — builds arm64 on Ubuntu 24.04
   - Cross-compiles with Android NDK 27 (API 24, arm64-v8a)
   - SDL3 built as shared library for Android
   - Produces: `.apk` (debug-signed)
7. **Publish** — creates/updates the GitHub Release
   - Downloads all platform artifacts
   - Generates release notes from `RELEASE_NOTES.md`
   - Creates combined SHA-256 checksum manifest
   - Uploads all artifacts to the GitHub Release

### SDL3 Version

All platform builds pin SDL3 to `release-3.2.14` via the `SDL3_TAG` environment variable.

## Release Artifacts

| Platform | Artifacts | Architecture |
|----------|-----------|-------------|
| macOS | `.dmg`, `.zip` | x86_64, arm64 |
| Windows | `.zip`, `.exe` (installer) | x86_64 |
| Linux | `.deb`, `.rpm` | x86_64, arm64 |
| Steam Deck | `.pkg.tar.zst`, `.AppImage` | x86_64 |
| iOS | `.ipa` | arm64 |
| Android | `.apk` | arm64 |

Each platform artifact set includes a `.sha256` checksum file. A combined `all-assets.sha256` manifest covers every artifact in the release.

## Pre-commit Hooks (Lefthook)

Three checks run on every commit:

- **newline_eof**: all committed files must end with a newline
- **trailing_whitespace**: no trailing whitespace allowed
- **no_game_data_payloads**: original game data files (DUNGEON.DAT, GRAPHICS.DAT, etc.) must never be tracked by git

## Manual Dispatch

The release workflow can also be triggered manually via `workflow_dispatch` with optional inputs:

- **version**: override release version (defaults to `CMakeLists.txt` version)
- **draft**: create as draft release
- **prerelease**: mark as prerelease
