# Firestaff V2 Wave 1 UI

This directory is the production foundation for the first V2 UI asset wave.

## Families
- `viewport-frame`
- `action-area`
- `spell-area`
- `status-boxes`
- `party-hud-cells`

## Resolution policy
- 4K masters are canonical.
- 1080p assets are approved derivatives at exactly 50% scale.

## Current bounded progress
- `spell-area`, `action-area`, `status-boxes`, `party-hud-cells`, and `party-hud-four-slot` now have 2026-04-23 rebuilt 4K/1080p assets based on the trusted DM1 subset documented in `GRAPHICS_DAT_EXPORT_MAPPING_AUDIT.md`.
- `viewport-frame` remains present in `vertical-slice/`, but it is provisional and not re-approved because `0000` is still mapping-suspicious.

## Important constraint
These folders track production work, but they still do not imply finished art.
Legacy filenames may remain for pipeline stability even where the trusted semantic meaning was corrected in the rebuild note and manifests.

## Cross-class style alignment
UI work must now follow the shared all-assets V2 direction in `V2_ALL_ASSETS_STYLE_GUIDE.md`.
That means higher-resolution repainting is welcome, but the result must stay classic-DM readable: weighty silhouettes, restrained surfaces, limited accent separation, and no glossy dashboard drift.
