# DM1 V2 Phase 4 Lighting Palette Gate

Scope: narrow Phase 4 source-lock slice for V2 lighting presentation. This does not claim full Phase 4 completion.

Source evidence:

- ReDMCSB `Toolchains/Common/Source/PANEL.C:329-432` implements `F0337_INVENTORY_SetDungeonViewPalette`.
- ReDMCSB `Toolchains/Common/Source/PANEL.C:370-387` reads torch light power from the party hand slots.
- ReDMCSB `Toolchains/Common/Source/PANEL.C:405-417` sums the top torch contributions and `G0407_s_Party.MagicalLightAmount`.
- ReDMCSB `Toolchains/Common/Source/PANEL.C:418-428` selects `G0304_i_DungeonViewPaletteIndex` from the light amount thresholds.
- ReDMCSB `Toolchains/Common/Source/DATA.C:359-360` defines `G0039_ai_Graphic562_LightPowerToLightAmount[16]` and `G0040_ai_Graphic562_PaletteIndexToLightAmount[6] = { 99, 75, 50, 25, 1, 0 }`.

Firestaff gate:

- `v2_light_build_source_palette_lighting()` accepts the already source-selected DM1 V1 palette index by value, mirrors it into a V2 presentation plan, and never writes gameplay light state.
- Valid source palette indices `0..5` keep the same source palette index and threshold floor in V2 presentation.
- Invalid source palette input deterministically falls back to palette `5` (darkest) and disables V2-only local lighting effects rather than inventing brighter state.
- `test_dm1_v2_lighting_dynamic_pc34` and `dm1_v2_lighting_dynamic_source_lock` cover this slice.
