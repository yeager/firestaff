# DM1 all-graphics phase 0 baseline

Date: 2026-04-25 09:46 Europe/Stockholm
Scope: Firestaff V1 / DM1 PC 3.4 in-game graphics baseline after starting debug-text containment and asset decode work.

## Worktree snapshot

Saved under:

- `verification-m11/dm1-all-graphics/phase0-baseline-20260425-0946/git-status-short.txt`
- `verification-m11/dm1-all-graphics/phase0-baseline-20260425-0946/git-diff-stat.txt`

Current local edits at baseline time were intentionally preserved and not overwritten.

## Original data source

`GRAPHICS.DAT` SHA-256:

```text
2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e  /Users/bosse/.firestaff/data/GRAPHICS.DAT
```

This matches the expected DM1 PC 3.4 original asset bank used by the V1 path.

## Screenshot artifacts

Generated indexed PGM plus RGB PNG versions using the DM PC 3.4 VGA palette from `vga_palette_pc34_compat.c`.

Normal V1:

- `verification-m11/dm1-all-graphics/phase0-baseline-20260425-0946/normal/party_hud_statusbox_gfx.{pgm,png}`
- `verification-m11/dm1-all-graphics/phase0-baseline-20260425-0946/normal/18_dm_action_hand_icon_cells.{pgm,png}`
- `verification-m11/dm1-all-graphics/phase0-baseline-20260425-0946/normal/19_dm_action_menu_mode.{pgm,png}`
- `verification-m11/dm1-all-graphics/phase0-baseline-20260425-0946/normal/21_projectile_in_flight.{pgm,png}`
- `verification-m11/dm1-all-graphics/phase0-baseline-20260425-0946/normal/22_explosion_after_advance.{pgm,png}`
- plus inventory/map/combat/shield fixture frames in the same folder.

Debug HUD mode:

- `verification-m11/dm1-all-graphics/phase0-baseline-20260425-0946/debug/*.pgm`
- `verification-m11/dm1-all-graphics/phase0-baseline-20260425-0946/debug/*.png`

## Text audit

Normal V1 screenshot path is now intended to suppress Firestaff telemetry/debug strings. Known banned strings for normal V1 include:

- `G GRAB`
- `P DROP`
- `TICK`
- `HERE`
- `AHEAD`
- `MISSILE FADES`
- generic projectile debug such as `FADES`, `OUT OF BOUNDS`, `COLLIDES IN FLIGHT`
- `SAVE DATA`
- `MENU BUTTON`

The normal probe log contains none of those banned text strings. Visual review should continue against the RGB PNGs, not grayscale PGM alone.

Debug HUD remains available separately with `FIRESTAFF_DEBUG_HUD=1` and is allowed to show diagnostic strings.

## Verification

Baseline generation command:

```sh
PROBE_SCREENSHOT_DIR=verification-m11/dm1-all-graphics/phase0-baseline-20260425-0946/normal ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
FIRESTAFF_DEBUG_HUD=1 PROBE_SCREENSHOT_DIR=verification-m11/dm1-all-graphics/phase0-baseline-20260425-0946/debug ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
```

Both probe runs completed successfully and produced the artifact set above.

## Next phase

Proceed to Phase 1/2/3:

1. hard V1/V2 asset boundary evidence and assertions,
2. complete debug-text containment probes,
3. cleanly finish `GRAPHICS.DAT` decode guard/odd-width work, especially graphic `0010` action/PASS area.
