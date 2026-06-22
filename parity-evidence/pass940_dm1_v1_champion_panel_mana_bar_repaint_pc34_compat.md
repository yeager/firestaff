# pass940 DM1 V1 Champion-Panel-Mana-Bar-Repaint

- Status: PASS940_DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_LOCKED
- Gate: A deterministic mana bar repaint contract that bridges the
  SYMBOL.C F0399 rune-mana spend path (CHAMDRAW.C F0287:141-154 bar height
  and CHAMDRAW.C F0287:307-342 PC34 bar split) to the
  CHAMDRAW.C F0292:898-935 draw-state path that the champion panel must
  take after the mutation. Synthetic champion mana pool only: no
  GRAPHICS.DAT/DUNGEON.DAT load.
- Runtime assertion floor: 80 assertions in
  `tests/test_dm1_v1_champion_panel_mana_bar_repaint_pc34_compat.c`.
- Expected test output: `>=80 assertions passed, hash=0x10B3227C`.

## ReDMCSB Anchors

- CHAMDRAW.C F0287:141-154 (mana bar height: ceil + overflow clamp at 25)
- CHAMDRAW.C F0287:307-342 (PC34 bar split: zone anchor, blank + fill band)
- CHAMDRAW.C F0292:898-935 (draw-state recompute after STATISTICS dirty)
- SYMBOL.C F0399:20-39 (rune mana cost spend before appending rune)
- SYMBOL.C F0400 (recant without refunding mana)
- DEFS.H M008_STATISTICS (0x0100) + M008_PANEL (0x0800) dirty bits
- DEFS.H M516_CHAMPIONS CurrentMana/MaximumMana field layout

## Non-Overlap

- Not pass791-799 (champion-panel/leader/mirror + auto-chest + chest-open).
- Not pass800-849 (light/animtown/chest_close_*/endgame_* gates).
- Not pass861-862 (bar-graph byte offsets / masks); this gate consumes the
  PC34 split formula but does not redefine the byte offset / mask arrays.
- Not pass805-816 (champion-panel box food/poisoned/water/panel/object-circle);
  this gate is the third-row bar (mana) only.
- Not the existing `dm1_v1_champion_panel_status_recompute` (which covers
  HP/stamina/food/water/poison/hand but NOT mana bar repaint).
- Not the existing `dm1_v1_champion_panel_clock_tick_stat_repaint` (which
  covers the F0331 party-wide dirty-flag sweep, not per-bar mana transitions).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_panel_mana_bar_repaint_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass940_dm1_v1_champion_panel_mana_bar_repaint_pc34_compat/manifest.json`
