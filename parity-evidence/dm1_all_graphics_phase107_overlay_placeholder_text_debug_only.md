# DM1 all-graphics phase 107 — placeholder overlay text debug-only

## Problem

Pass 105 classified Firestaff's current dialog/endgame overlays as placeholder `KNOWN_DIFF` surfaces. The visible overlays still included obviously invented/debug text in default V1 play:

- dialog: `TEXT PLAQUE`
- dialog: `PRESS ANY KEY TO DISMISS`
- endgame: `VICTORY AT TICK ...`
- endgame: `ESC TO RETURN TO MENU`

These labels are useful during development, but they are not source DM1 dialog/endgame visuals.

## Change

Default V1 chrome mode now suppresses those placeholder/debug labels unless `showDebugHUD=1`.

Functional behavior remains:

- dialog overlay still displays the message text and dismisses on input
- endgame overlay still displays the basic victory panel and accepts ESC/back
- debug HUD sessions still show the extra placeholder/help text

This does not claim source dialog/endgame parity; it just removes invented helper text from default V1 presentation.

## Gates

Added/updated invariants:

- `INV_GV_165B` — V1 endgame overlay keeps tick/help text debug-only
- `INV_GV_172B` — V1 dialog overlay keeps placeholder title/footer debug-only

```text
PASS INV_GV_165B V1 endgame overlay keeps tick/help text debug-only
PASS INV_GV_172B V1 dialog overlay keeps placeholder title/footer debug-only
# summary: 421/421 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
