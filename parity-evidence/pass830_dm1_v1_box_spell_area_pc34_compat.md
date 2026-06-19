# pass830 DM1 V1 Box-Spell-Area

- Status: PASS830_DM1_V1_BOX_SPELL_AREA_LOCKED
- Gate: Graphics.dat item 562 init var G0000_ai_Graphic562_Box_SpellArea[4] = {224, 319, 42, 74}. The {X, Y, W, H} byte-coordinate sub-rectangle for the spell-area background blit on the champion panel. Read sites: CASTER.C:24/31 + STARTUP2.C:376 (spell-area blit + clear + hatch).
- Runtime assertion floor: 54 assertions in `tests/test_dm1_v1_box_spell_area_pc34_compat.c`.
- Expected test output: `54/54 assertions passed`.

## ReDMCSB Anchors

- DATA.C:6
- DATA.C:119
- DATA.C:539
- CASTER.C:24
- CASTER.C:31
- STARTUP2.C:376

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-821 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_box_spell_area_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass830_dm1_v1_box_spell_area_pc34_compat/manifest.json`
