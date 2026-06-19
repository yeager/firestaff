# pass801 DM1 V1 Light-Power-To-Light-Amount

- Status: PASS801_DM1_V1_LIGHT_POWER_TO_LIGHT_AMOUNT_LOCKED
- Gate: Graphics.dat item 562 init var G0039_ai_Graphic562_LightPowerToLightAmount[16] = {0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100}. Monotonically non-decreasing saturating curve. PANEL.C:412 sums per-torch light power with a <<multiplier>>6 scale; CHAMPION.C:529/645 reads G0039[2] for Illumulet equip delta (= 12); MENU.C:1608 (Light spell +12), MENU.C:1936 (MagicTorch +G0039[LightPower]), MENU.C:1941 (Darkness -G0039[LightPower]); TIMELINE.C:1754 diff = G0039[Strong] - G0039[Weaker] for the per-tick light-event.
- Runtime assertion floor: 126 assertions in `tests/test_dm1_v1_light_power_to_light_amount_pc34_compat.c`.
- Expected test output: `126/126 assertions passed`.

## ReDMCSB Anchors

- DATA.C:45
- DATA.C:359
- DATA.C:1088
- PANEL.C:412
- CHAMPION.C:529/645
- MENU.C:1608/1936/1941
- TIMELINE.C:1754

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + auto-chest + chest-open-stack-split).
- Not pass798 (icon-graphic-first-icon-index).
- Not pass800 (slot-boxes).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_light_power_to_light_amount_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass801_dm1_v1_light_power_to_light_amount_pc34_compat/manifest.json`
