# pass821 DM1 V1 Palette-Dungeon-View

- Status: PASS821_DM1_V1_PALETTE_DUNGEON_VIEW_LOCKED
- Gate: Graphics.dat item 562 init var G0021_aaui_Graphic562_Palette_DungeonView[6][16]. The 6-row × 16-col dungeon-view palette table (12-bit RGB, 4 bits per channel). Row 0 = brightest, row 5 = darkest. Current row selected by G0304_i_DungeonViewPaletteIndex based on dungeon light amount. Read sites: BASE.C:968 + DRAWVIEW.C:744/776/880/1040.
- Runtime assertion floor: 329 assertions in `tests/test_dm1_v1_palette_dungeon_view_pc34_compat.c`.
- Expected test output: `329/329 assertions passed`.

## ReDMCSB Anchors

- DATA.C:27
- DATA.C:218
- DATA.C:833
- BASE.C:968
- DRAWVIEW.C:744/776/880/1040

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-820 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_palette_dungeon_view_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass821_dm1_v1_palette_dungeon_view_pc34_compat/manifest.json`
