# DM1 V1 Light and Darkness Spell Source Lock - 2026-05-20

Scope: DM1 V1 only. This locks the spell-to-light runtime bridge for the `Light` and `Darkness` other-spell effects. It does not change spell projectiles, CSB, DM2, V2, or rendering refactors.

## ReDMCSB Audit Anchors

- Spell type constants: `DEFS.H:1760-1775` defines other-spell kind/type IDs including `C0_SPELL_TYPE_OTHER_LIGHT` and `C1_SPELL_TYPE_OTHER_DARKNESS`. It also distinguishes `C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR` for `Zo`, so this bridge only handles the non-projectile light/darkness cases.
- Light creation helper: `MENU.C:F0404_MENUS_CreateEvent70_Light`, lines `1125-1141`, creates `C70_EVENT_LIGHT`, stores `Event.B.LightPower`, schedules it at `GameTime + ticks`, and refreshes the dungeon palette.
- Weapon/action light: `MENU.C:C038_ACTION_LIGHT`, lines `1607-1611`, adds `LightPowerToLightAmount[2]` and schedules `CreateEvent70_Light(-2, 2500)`.
- Light spell branch: `MENU.C:F0412_MENUS_GetChampionSpellCastResult`, lines `1922-1938`, computes `SpellPower=(PowerSymbolOrdinal+1)<<2`, derives light duration and light power, adds magical light, and schedules a negative light fade event.
- Darkness spell branch: `MENU.C:F0412_MENUS_GetChampionSpellCastResult`, lines `1939-1943`, derives darkness light power from the same spell power, subtracts magical light, and schedules positive recovery event `98` ticks later.
- Light fade processing: `TIMELINE.C:F0257_TIMELINE_ProcessEvent70_Light`, lines `1720-1766`, turns event light power into a delta between adjacent `LightPowerToLightAmount` entries, applies the signed magical-light delta, and chains weaker events every `4` ticks.
- Light amount tables: `DATA.C:359-360` defines `G0039_ai_Graphic562_LightPowerToLightAmount` and `G0040_ai_Graphic562_PaletteIndexToLightAmount`; `DATA.C:263` defines torch charge-to-type mapping used by the existing light gate.
- Viewport palette dimming: `PANEL.C:F0337_INVENTORY_SetDungeonViewPalette`, lines `329-432`, sums torch light and `G0407_s_Party.MagicalLightAmount`, selects `G0304_i_DungeonViewPaletteIndex` from palette thresholds, and requests a dungeon palette refresh.
- RGB darkening primitive: `DARKCOLR.C:F0431_STARTEND_GetDarkenedColor`, lines `1-20`, decrements nonzero RGB444 components by one step.
- Palette level constants: `DEFS.H:3536-3541` defines `C00_LIGHT0` through `C05_LIGHT5`.

## Firestaff Binding

- `include/dm1_v1_light_pc34_compat.h` exposes `dm1_light_apply_other_spell_effect` and DM1 V1 light/darkness other-spell constants.
- `src/dm1/dm1_v1_light_pc34_compat.c` implements the `MENU.C:F0412` formulas for only `Light` and `Darkness` other-spell types, reusing the existing source-locked magical-light, darkness, event fade, and palette recalculation routines.
- `tests/test_dm1_v1_light_darkness_spell_bridge_pc34_compat.c` casts `Lo Oh Ir Ra` and `Lo Des Ir Sar` through the spell table, then applies the bridge and verifies immediate magical-light deltas, event signs, event timing, and resulting palette indices.

## Focused Verification

Commands run from build directory `/home/trv2/work/firestaff-worktrees/light-darkness-spells-source-lock-20260520-build`:

- `cmake -S /home/trv2/work/firestaff-worktrees/light-darkness-spells-source-lock-20260520 -B /home/trv2/work/firestaff-worktrees/light-darkness-spells-source-lock-20260520-build -DCMAKE_BUILD_TYPE=Debug`
- `cmake --build /home/trv2/work/firestaff-worktrees/light-darkness-spells-source-lock-20260520-build --target test_dm1_v1_light_pc34_compat test_dm1_v1_light_darkness_spell_bridge_pc34_compat -j2`
- `ctest --test-dir /home/trv2/work/firestaff-worktrees/light-darkness-spells-source-lock-20260520-build --output-on-failure -R "dm1_v1_light(_darkness_spell_bridge)?_source_lock"`
- `git diff --check HEAD~1..HEAD`
- High-signal changed diff scan over the committed diff for credential-bearing terms.

Verification results are recorded in the worker final report for the commit that contains this evidence file.
