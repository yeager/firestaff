# dm1_v1_champion_panel_spell_area_clear_on_inventory_source_lock

- status: `DM1_V1_CHAMPION_PANEL_SPELL_AREA_CLEAR_ON_INVENTORY_CYCLE_LOCKED_NON_DUPLICATIVE_WITH_OVERLAY_AND_INPUT_GATES`
- generatedUtc: `2026-06-13T09:30:00.000000+00:00`
- redmcsb: `/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`
- parity claim: **not made**; this is a contract-only, no-asset
  state-machine gate for the PANEL.C F0355_INVENTORY_Toggle_CPSE
  open/close cycle that the original ReDMCSB PC 3.4 BIOS keeps
  decoupled from the upper-right spell-area state machine.

## Lane

The DM1 V1 spell-area state machine is the *upper-right* panel HUD
strip (screen box `{224, 319, 42, 74}`, 96x33 px) plus the in-memory
state `G0514_i_MagicCasterChampionIndex` + the per-champion
`Champion.Symbols[0..3]` 4-byte rune buffer + `Champion.SymbolStep`.
The lane pins the *open/close cycle* of the inventory panel
(`PANEL.C F0355_INVENTORY_Toggle_CPSE`) and the byte-stability of the
spell-area state machine across that cycle.

Concretely, the contract is:

- `F0355` is the only function that toggles the inventory panel. The
  open path (lines 2370-2440) fires
  `F0488_MEMORY_ExpandGraphicToBitmap(C017_GRAPHIC_INVENTORY,
  G0296_puc_Bitmap_Viewport)`, which visually covers the spell-area
  box `{224, 319, 42, 74}` on screen, but it does NOT call
  `F0394_MENUS_SetMagicCasterAndDrawSpellArea`.
- The close path (lines 2310-2335) fires
  `F0334_INVENTORY_CloseChest`, `F0395_MENUS_DrawMovementArrows`,
  `F0357_COMMAND_DiscardAllInput`, and
  `F0098_DUNGEONVIEW_DrawFloorAndCeiling`, but it does NOT call
  `F0394` either.
- Across the full open/close cycle, `G0514_i_MagicCasterChampionIndex`,
  the caster's `SymbolStep`, and the caster's `Symbols[0..3]` rune
  buffer are byte-stable. The C017 GRAPHIC_INVENTORY blit covers the
  spell-area box on screen while the inventory is open, but the
  in-memory state machine (G0514 + SymbolStep + Symbols[0..3])
  survives.
- The `F0077` / `F0078` mouse screen-update bracketing is balanced
  across a single F0355 entry: each open path fires F0077 at the top
  and F0078 at the bottom; the close path (C04) fires F0077 at the
  top and F0078 at the bottom. An open + close round trip leaves the
  net balance at 0.
- Negative inputs are rejected with no state change: dead inventory
  champion (PANEL.C:2280-2285), pressing mouth/eye
  (PANEL.C:2290-2295), no inventory session (G0423 == 0), C05
  special-inventory on a no-session state.

## Source anchors (PC 3.4 path, MEDIA009+ / MEDIA720)

- `PANEL.C:2244-2440` `F0355_INVENTORY_Toggle_CPSE` is the only
  function that toggles the inventory panel.
- `PANEL.C:2280-2285` `F0355` dead-inventory-champion early return
  for `P0719 < C04 && !CurrentHealth`.
- `PANEL.C:2290-2295` `F0355` `G0333_B_PressingMouth ||
  G0331_B_PressingEye` reject.
- `PANEL.C:2302` `F0077_MOUSE_EnableScreenUpdate_CPSE` at the top of
  every F0355 entry.
- `PANEL.C:2310-2329` `F0355` close-the-previous-inventory branch:
  fires `F0334_INVENTORY_CloseChest` (the only function that mutates
  `G0425`/`G0426`), sets `G0423_i_InventoryChampionOrdinal =
  M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)`, fires
  `F0098_DUNGEONVIEW_DrawFloorAndCeiling`, and (on the C04 close
  branch) `F0395_MENUS_DrawMovementArrows` + `F0357_COMMAND_DiscardAllInput`.
- `PANEL.C:2340` `F0078_MOUSE_DisableScreenUpdate` at the end of the
  C04 close branch.
- `PANEL.C:2357-2440` `F0355` open-inventory branch: collapses
  same-champion request to a close (`M000_INDEX_TO_ORDINAL(champion_index)
  == G0423_i_InventoryChampionOrdinal` test at line 2314), or computes
  the new `G0423` ordinal + champion index, fires
  `F0136_VIDEO_HatchScreenBox(C009_ZONE_MOVEMENT_ARROWS,
  C00_COLOR_BLACK)` on a fresh open (line 2366), expands
  `C017_GRAPHIC_INVENTORY` into `G0296_puc_Bitmap_Viewport` (line
  2376), hides the floppy icon (line 2379-2388), and emits the
  health/stamina/mana labels (lines 2390-2440).
- `PANEL.C:2440` `F0078_MOUSE_DisableScreenUpdate` at the end of the
  open path.
- `DEFS.H:504` `G0514_i_MagicCasterChampionIndex` (the live caster).
- `DEFS.H:5876` `G0423_i_InventoryChampionOrdinal` (the open-panel
  champion, distinct from the caster).
- `DEFS.H:712-716` `C04_CHAMPION_CLOSE_INVENTORY` sentinel.
- `DEFS.H:8200` `M000_INDEX_TO_ORDINAL` / `M001_ORDINAL_TO_INDEX`
  ordinal helpers.
- `DEFS.H:780-820` `C04..C05` ordinal sentinels and the
  `CM1_CHAMPION_NONE` sentinel.
- `DEFS.H:8194` `extern void F0355_INVENTORY_Toggle_CPSE` prototype.
- `DEFS.H:8358` `extern void F0393_MENUS_DrawSpellAreaControls`
  prototype (referenced for completeness; F0355 never calls it).
- `DEFS.H:8363` `extern void F0394_MENUS_SetMagicCasterAndDrawSpellArea`
  prototype (referenced for completeness; F0355 never calls it).
- `MENU.C:504/513` `int16_t G0514_i_MagicCasterChampionIndex =
  CM1_CHAMPION_NONE;` initial value.
- `MENU.C:855/1652` `L1203_ps_Champion =
  &M516_CHAMPIONS[G0514_i_MagicCasterChampionIndex];` (F0355 never
  dereferences this).
- `CHAMPION.C:1681` `F0284_CHAMPION_SetPartyDirection` leader
  rotation refreshes the spell area via F0393 (not F0355).
- `REVIVE.C:282/292/845/931` resurrect / reincarnate candidate routes
  refresh the spell area via F0393 (not F0355).
- `MENU.C:1657` post `F0412_MENUS_GetChampionSpellCastResult` refreshes
  F0397 + F0398 (not F0355).
- `SYMBOL.C:62-63/102-103` post symbol step change refresh (not F0355).

## State machine contract

The gate pins the following invariants for the contract-only
`dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34` and
`dm1_v1_champion_panel_spell_area_clear_on_inventory_close_pc34`
functions, and the round-trip
`dm1_v1_champion_panel_spell_area_clear_on_inventory_round_trip_pc34`:

1. F0355 open path with a live caster (no prior inventory):
   - `accepted = 1`, `reject_reason = NONE`.
   - `fired_f0394_set_magic_caster = 0` (F0355 never calls F0394).
   - `loaded_c017_graphic_inventory_into_g0296 = 1` (C017 blit covers
     the spell-area box visually).
   - `new_inventory_champion_ordinal = requested_champion_index + 1`
     (M000_INDEX_TO_ORDINAL).
   - `new_magic_caster_champion_index` byte-stable.
   - `caster_symbols_byte_stable = 1`,
     `caster_symbol_step_byte_stable = 1`,
     `symbols_buffer_byte_stable = 1` (all 4 champion symbol buffers
     byte-stable).
   - `fired_f0077_enable_screen_update = 1` and
     `fired_f0078_disable_screen_update = 1` (F0077 + F0078 balance
     for a single F0355 entry).
   - `mouse_update_balance = 0` (net balance for a single F0355 entry).
   - `fired_f0136_hatch_movement_arrows_box = 1` (fresh open only).
2. F0355 open path with a prior inventory (different champion):
   - `fired_f0334_close_chest = 1`,
     `fired_f0098_draw_floor_and_ceiling = 1` (close the prior
     inventory first).
   - `new_inventory_champion_ordinal` updated to the new champion.
   - `new_magic_caster_champion_index` byte-stable.
3. F0355 same-champion open collapses to close:
   - `accepted = 1`, `new_inventory_champion_ordinal = 0`.
   - `fired_f0334_close_chest = 1`,
     `loaded_c017_graphic_inventory_into_g0296 = 0` (no fresh open).
4. F0355 close path with a live caster:
   - `accepted = 1`, `reject_reason = NONE`.
   - `fired_f0394_set_magic_caster = 0` (F0355 close never calls F0394).
   - `fired_f0334_close_chest = 1`, `fired_f0395_draw_movement_arrows = 1`,
     `fired_f0357_discard_all_input = 1`,
     `fired_f0098_draw_floor_and_ceiling = 1`.
   - `fired_f0077_enable_screen_update = 1` and
     `fired_f0078_disable_screen_update = 1` (F0077 + F0078 balance
     for a single F0355 entry).
   - `mouse_update_balance = 0`.
5. F0355 close path with no prior session:
   - `accepted = 0`, `reject_reason = NO_INVENTORY_SESSION`.
   - `fired_f0334_close_chest = 0`, `fired_f0395_draw_movement_arrows = 0`,
     `fired_f0357_discard_all_input = 0`,
     `fired_f0098_draw_floor_and_ceiling = 0`.
6. F0355 open with a dead inventory champion:
   - `accepted = 0`, `reject_reason = DEAD_INVENTORY_CHAMPION`.
   - `fired_f0077_enable_screen_update = 0`,
     `fired_f0078_disable_screen_update = 0`,
     `fired_f0334_close_chest = 0`,
     `loaded_c017_graphic_inventory_into_g0296 = 0`.
7. F0355 open with `pressing_mouth || pressing_eye`:
   - `accepted = 0`, `reject_reason = PRESSING_MOUTH_OR_EYE`.
   - `loaded_c017_graphic_inventory_into_g0296 = 0`.
8. F0355 open with `C05_CHAMPION_SPECIAL_INVENTORY` and no prior
   session: `accepted = 0`,
   `reject_reason = NO_INVENTORY_SESSION`.
9. Round trip: `f0394_call_count = 0` (F0394 never called across
   open + close), `mouse_update_balanced = 1` (open + close net 0),
   `symbols_byte_stable = 1` (all 4 champion symbol buffers survive),
   `symbol_step_byte_stable = 1` (caster SymbolStep survives).

## Test fixture

`tests/test_dm1_v1_champion_panel_spell_area_clear_on_inventory_pc34_compat.c`:

- 19 subtests (contract, open with live caster, open with dead
  inventory champion, open with pressing mouth, open with pressing
  eye, open with C05 special-inventory on no-session, open with no
  caster, open while inventory active, open with empty symbol
  buffer, open collapses to close, close with live caster, close
  with no session, close twice rejects, round trip basic, round
  trip with other caster, round trip with no caster, round trip
  with prior inventory, round trip with short runes, round trip
  with diverse runes).
- 157/157 assertions pass (deterministic across multiple runs).
- `ctest -R dm1_v1_champion_panel_spell_area_clear_on_inventory_pc34_compat`
  passes 1/1.
- The related
  `dm1_v1_champion_panel_spell_area_overlay_pc34_compat` test still
  passes (no regression).
- `cmake --build build --target
  test_dm1_v1_champion_panel_spell_area_clear_on_inventory_pc34_compat`
  builds with 0 warnings.
- The strict `cc -Wall -Wextra -Werror -O2` build of the new test
  file is also clean.
- `SDL_VIDEODRIVER=dummy ./firestaff_m11_phase_a_probe` passes 23/23.
- `git diff --check` is clean.

## Lane non-duplication

The lane is intentionally disjoint from the existing champion-panel
spell-area family:

- `dm1_v1_champion_panel_spell_area_overlay_pc34_compat` (pass797)
  pins the F0394 / F0393 / F0397 / F0398 draw contract (lines 1..3
  of the spell area); it does NOT pin the F0355 inventory open/close
  cycle or the byte-stability of the caster state machine.
- `dm1_v1_champion_panel_portrait_state_redraw_pc34_compat` pins the
  F0292_CHAMPION_DrawState portrait state redraw tuple.
- `dm1_v1_champion_panel_hud_food_water_recompute_pc34_compat` pins
  the F0349 panel recompute slice.
- `spell_area_routes_pc34_compat_integration` (pass602b) covers the
  *input* dispatch (C100..C109 command ids -> F0370_CLIKMENU_Process
  ChampionCommand); it is input-side, not state-side.
- `dm1_v1_menu_render_pc34_compat` covers the orchestrator flag that
  F0457_START_DrawEnabledMenus_CPSF reads; it is also not state-side.
- `dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_pc34_compat`
  (pass783) pins the C040 candidate panel redraw after F0355(C04);
  it does NOT pin the G0514 / SymbolStep / Symbols[0..3] state
  machine.
- `dm1_v1_champion_panel_food_water_status_box_pc34_compat`,
  `dm1_v1_champion_panel_pressing_mouth_eye_statusbox_pc34_compat`,
  `dm1_v1_champion_panel_hand_slot_priority_pc34_compat`,
  `dm1_v1_champion_panel_f0354_box_variants_pc34_compat`,
  `dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat`,
  `dm1_v1_champion_panel_portrait_box_redraw_states_pc34_compat`,
  and the rest of the integrated chest / mirror-candidate / F0107 /
  F0108 / F0111 / F0115 family in TODO.md are all disjoint from
  this lane.
