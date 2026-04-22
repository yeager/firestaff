# Firestaff 0.1.5, macOS Preview

Firestaff 0.1.5 is an early macOS preview release.

This release includes a real launcher, a modern high-resolution true-color startup menu, bounded original-version detection, a playable DM1 engine slice, and a self-contained macOS app bundle with SDL3 packaged inside the app.

This is still a preview release. DM1 original-presentation parity is not finished yet, and CSB / DM2 are not complete runtime targets yet.

## Highlights

- macOS preview DMG and app bundle
- modern 1280x720 true-color startup menu
- version selection with checksum-based ready / missing state
- bounded original-data detection for DM1 / CSB / DM2
- real DM1 dungeon loading and live game-view flow
- movement, combat, items, spells, stairs, pits, teleporters, XP, save/load, and survival slices
- improved source-backed creature rendering behavior using `GraphicInfo` pose fallback rules
- ten-creature gallery now shown in the README as the current launcher art/reference set

## Original data

Firestaff does not ship original assets. You must provide your own legal original game files.

Original-data search order:
1. explicit path, if provided
2. `~/.firestaff/originals/` on macOS/Linux, or `<installation-directory>\\originals` on Windows
3. legacy Firestaff data-dir fallback such as `~/.firestaff/data/`

## Not finished yet

- full DM1/V1 parity
- full CSB support
- full DM2 support
- final product polish
- Linux `.deb` / `.rpm` packaging
- Windows installer packaging
