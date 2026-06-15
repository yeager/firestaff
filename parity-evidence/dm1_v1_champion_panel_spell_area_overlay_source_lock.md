# dm1_v1_champion_panel_spell_area_overlay_source_lock

- status: `DM1_V1_CHAMPION_PANEL_SPELL_AREA_OVERLAY_DRAW_CONTRACT_LOCKED_NON_DUPLICATIVE_WITH_INPUT_AND_ORCHESTRATOR_GATES`
- generatedUtc: `2026-06-12T20:11:00.000000+00:00`
- redmcsb: `/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`
- parity claim: **not made**; this is a contract-only, no-asset draw-contract
  gate for the upper-right panel HUD strip the original ReDMCSB PC 3.4 BIOS
  paints every time the acting caster identity, symbol buffer, or symbol
  step changes.

## Lane

The DM1 V1 spell-area overlay (screen box `{224, 319, 42, 74}`, 96x33 px)
is the *upper-right* panel HUD strip. It has three lines:

- Line 1 (y=42..49): 4 champion tab highlight boxes drawn by
  `F0393_MENUS_DrawSpellAreaControls` (SPELDRAW.C).
- Line 2 (y=50..61): 6 available rune symbols drawn by
  `F0397_MENUS_DrawAvailableSymbols` (MENUDRAW.C).
- Line 3 (y=62..73): up to 4 currently-typed champion symbols drawn by
  `F0398_MENUS_DrawChampionSymbols` (MENUDRAW.C).

The whole strip is brought up by `F0394_MENUS_SetMagicCasterAndDrawSpellArea`
(CASTER.C) on every caster identity change, and the lines bitmap is loaded
by `F0396_MENUS_LoadSpellAreaLinesBitmap` (MENUDRAW.C). The four ReDMCSB
callers that reissue `F0393` after a state mutation are:

- REVIVE.C:292/845/931 resurrect/reincarnate candidate routes.
- CHAMPION.C:1681 `F0284_CHAMPION_SetPartyDirection` leader rotation.
- MENU.C:1657 after `F0412_MENUS_GetChampionSpellCastResult`.
- SYMBOL.C:62-63,102-103 after a symbol step change.

This lane is distinct from the existing coverage:

- `spell_area_routes_pc34_compat` (pass602b) — covers the *input* dispatch
  (C100..C109 command ids -> F0370_CLIKMENU_ProcessChampionCommand via the
  touch-click zone matrix).
- `dm1_v1_menu_render_pc34_compat` — covers the orchestrator flag that
  F0457_START_DrawEnabledMenus_CPSF reads, but not the draw contract
  inside F0393/F0397/F0398.
- `dm1_v1_champion_panel_pressing_mouth_eye_statusbox_pc34_compat`
  (pass673) — covers the inventory mouth/eye routes that route through
  PANEL.C F0342/F0345, not the spell-area draw.
- `dm1_v1_champion_panel_food_water_status_box_pc34_compat` (pass683) —
  covers the chest-close -> 67x29 status-box redraw, not the spell area.
- `dm1_v1_champion_panel_hand_slot_priority_pc34_compat` — covers the
  status-hand -> leader-hand -> backpack -> belt priority chain, not the
  spell area.
- `dm1_v1_champion_panel_portrait_state_redraw_pc34_compat` — covers the
  CHAMDRAW.C F0292 portrait state redraw tuple, not the spell area.

The lane also avoids the F0217/F0219 wall-impact projectile sound, the
D3C F0107 wall-ornament, the door-bash feedback, the resurrect-confirm +
chest-close ordering, the scroll-wheel, mirror-candidate C028/C040/C045,
and the rest of the integrated DM1 V1 family listed in TODO.md.

## Source anchors (PC 3.4 path, MEDIA009+)

- `CASTER.C:18-21` `F0394_MENUS_SetMagicCasterAndDrawSpellArea` short-circuits
  when the requested caster equals `G0514_i_MagicCasterChampionIndex` and
  rejects a non-NONE caster with `CurrentHealth == 0`.
- `CASTER.C:23-26` `F0394` paints
  `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND` (33 rows x 48 byte width) into
  `G0000_ai_Graphic562_Box_SpellArea` only on a transition from
  `CM1_CHAMPION_NONE`.
- `CASTER.C:28-32` `F0394` `CM1_CHAMPION_NONE` path sets
  `G0514_i_MagicCasterChampionIndex = CM1_CHAMPION_NONE` and calls
  `M524_FillScreenBox(..., C00_COLOR_BLACK)`. `ENDGAME.C:1010` reuses the
  same fill for the closing scene.
- `SPELDRAW.C:36-90` `F0393_MENUS_DrawSpellAreaControls` emits the 4
  champion tab highlight boxes. Champion 0 is `x0=233..277` (44 wide to
  span the inventory champion / leader column), champions 1..3 are
  `x0=280..291/294..305/308..319` (11 wide), all with `y0=42`. Champion 0
  uses `y1=49` to extend through the leader row; champions 1..3 use
  `y1=48`. Each tab is only highlighted if `Champions[i].CurrentHealth > 0`
  and `G0305_ui_PartyChampionCount > i`.
- `MENUDRAW.C:31-45` `F0396_MENUS_LoadSpellAreaLinesBitmap` loads
  `C011_GRAPHIC_MENU_SPELL_AREA_LINES` into the 3-row stack bitmap used by
  `F0392` and `F0394`.
- `MENUDRAW.C:47-80` `F0397_MENUS_DrawAvailableSymbols` emits 6 characters
  starting at ASCII `96 + 6*SymbolStep` in `C04_COLOR_CYAN` on
  `C00_COLOR_BLACK` at screen `x = 225 + 14*i`, `y = 58`.
- `MENUDRAW.C:83-117` `F0398_MENUS_DrawChampionSymbols` emits up to 4
  characters from `Champion->Symbols[0..N-1]` (with
  `N = strlen(Symbols)` clamped to 4) with a `C20_SPACE` padding tail,
  screen `x = 232 + 9*i`, `y = 70`, cyan on black.
- `DATA.C:119` `G0000_ai_Graphic562_Box_SpellArea = {224, 319, 42, 74}`.
- `DATA.C:530-531` `G1072_ai_Box_SpellAreaLine2 = {224, 319, 50, 61}`,
  `G1073_ai_Box_SpellAreaLine3 = {224, 319, 62, 73}`.
- `DEFS.H` `C100_COMMAND_CLICK_IN_SPELL_AREA` / `C101..C106` rune symbols
  / `C107` recant / `C108` cast / `C109` set caster (input side, not
  draw side, but referenced for completeness).
- `DEFS.H` `C013_ZONE_SPELL_AREA` (alt path for MEDIA529 builds that
  draw through F0660_ and F0733_FillZoneByIndex).
- `DEFS.H` `C255..C260` `ZONE_SPELL_AREA_AVAILABLE_SYMBOL_0..5` (alt
  path for the line 2 zone index).
- `DEFS.H` `C261..C264` `ZONE_SPELL_AREA_CHAMPION_SYMBOL_0..3` (alt path
  for the line 3 zone index).
- `DEFS.H` `C04_COLOR_CYAN`, `C00_COLOR_BLACK`, `CM1_CHAMPION_NONE`.

## Draw contract

The gate pins the following invariants for the contract-only
`dm1_v1_champion_panel_spell_area_overlay_plan_pc34`:

1. F0394 same-caster short-circuit:
   `(requested == previous) && (requested != CM1_CHAMPION_NONE)` returns
   `REJECT_SAME_CASTER` and emits no draws.
2. F0394 dead-champion reject:
   `(requested != CM1_CHAMPION_NONE) && !Champions[requested].CurrentHealth`
   returns `REJECT_DEAD_CHAMPION` and emits no draws.
3. F0394 `CM1_CHAMPION_NONE` clear path:
   `(requested == CM1_CHAMPION_NONE)` sets
   `post_caster_index = CM1_CHAMPION_NONE`, `cleared_to_black = 1`, and
   emits no draws.
4. F0394 background blit:
   `(previous == CM1_CHAMPION_NONE) && (requested != CM1_CHAMPION_NONE)`
   sets `drew_background_graphic = 1`; otherwise it is 0.
5. F0394 draw contract on a normal caster transition:
   `drew_lines_bitmap = drew_spell_area_controls =
    drew_available_symbols = drew_champion_symbols = 1`.
6. F0393 champion-tab x0/x1 geometry and per-tab
   `Champions[i].CurrentHealth > 0 && i < party_champion_count` gate.
7. F0397 line 2 character math:
   `character = 96 + 6*SymbolStep + i`, `screen_x = 225 + 14*i`,
   `screen_y = 58`, cyan on black.
8. F0398 line 3 fill: `len = strlen(Champion->Symbols)` clamped to 4,
   `screen_x = 232 + 9*i`, `screen_y = 70`, cyan on black, with
   `C20_SPACE` padding on slots `[len..3]`.
9. Validation guards for `requested_caster_index` /
   `previous_caster_index` in `{-1, 0..3}`, `party_champion_count` in
   `0..4`, and `symbol_step` in `0..5`.

## Test fixture

`tests/test_dm1_v1_champion_panel_spell_area_overlay_pc34_compat.c`:

- 10 subtests (contract, same-caster reject, dead-champion reject,
  NONE clear path, first caster after NONE, caster swap with step
  window, dead champion excluded from tabs, full symbols no padding,
  empty symbols full padding, validation guards).
- 208/208 assertions pass.
- `ctest -R dm1_v1_champion_panel_spell_area_overlay_pc34_compat` passes
  1/1.
- `cmake --build build --target
  test_dm1_v1_champion_panel_spell_area_overlay_pc34_compat` builds with
  0 warnings.
- Full related `spell_area_*` and `champion_panel_*` family (32 tests)
  still passes (no regression).
- `git diff --check` is clean.
- Full M10 + test build (`cmake --build build --parallel`) is clean.
