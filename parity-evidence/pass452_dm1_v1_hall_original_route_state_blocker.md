# Pass452 — DM1 V1 original Hall route-state blocker

- status: `PASS_PASS452_WRONG_FACING_BLOCKER_SUPERSEDED_BY_INITIAL_SOUTH_RERUN`
- artifact: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/dm1-hall-dosbox-20260509/manifest.json`
- DUNGEON.DAT sha256: `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- GRAPHICS.DAT sha256: `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
- TITLE sha256: `adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745`

## Diagnosis

The N2 capture did prove a stock PC34 Hall/front-mirror visible artifact for a Hall-specific route, but 2026-05-14 webchat screenshots supersede using Hall of Champions as the normal Firestaff DM1 start baseline. This gate must not be cited as proof that ordinary DM1 launch starts in Hall of Champions. The stale Hall route clicked after this manifest route:

- select VGA
- select No Sound
- select Mouse/input
- title/menu ENTER
- turn_left_east
- step_east_blocked
- turn_left_north_front_mirror
- candidate portrait source center
- resurrect confirm source center

That route is not a valid normal-start baseline. It is only a legacy Hall-specific route-state diagnosis: party map0 `(1,3)` facing **South**, facing the C127 champion portrait sensor on `(1,4)`. The capture then turned left to East, attempted an East step that was blocked, and turned left again to **North** before clicking x111/y82. Since turns do not move the party and blocked steps do not move it, the click touched the north front wall, not the south C127 wall.

Correction 2026-05-14: Daniel provided webchat screenshot evidence that Firestaff DM1 does **not** begin in Hall of Champions. Treat Hall-route evidence as a bounded resurrection/champion-route lane only, never as the expected ordinary DM1 launch frame.

That stale run is now superseded by the corrected initial-south rerun when `corrected_initial_south_transition.ok` is true. The diagnosis remains in the evidence record so the bad route is not reused.

## Source locks

- `LOADSAVE.C:1940-1944` — legacy Hall-route assumption came from DungeonHeader InitialPartyLocation decoding to map0 x=1 y=3 dir=South; this must not be reused as proof that normal Firestaff DM1 launch starts in Hall of Champions
- `CLIKMENU.C:142-173,264-328` — turns stay on the same square; blocked steps do not move the party, so the N2 route turn-left, blocked-step, turn-left ends at the initial square facing North, not South
- `CLIKVIEW.C:347-431` — PC C080 click subtracts viewport origin, then only C05/front-wall ornament hit calls F0372 to touch the wall sensor in front of the current facing direction
- `MOVESENS.C:1309-1503` — C127 wall champion portrait is the candidate transition; it is allowed with no leader, but only for the clicked wall cell/facing precondition
- `DUNVIEW.C:525,3912-3919` — visible source portrait box is viewport x=96..127 y=35..63; with PC viewport origin y=33, center is screen x=111 y=82
- `COORD.C:1693-1698,1748-1749` — PC viewport origin is x=0 y=33 and portrait is 32x29, confirming x111/y82 screen click geometry
- `REVIVE.C:63-67,272,704-789` — F0280 appends/marks the candidate; only after that can C160/C161/C162 panel commands in F0282 finalize/cancel candidate state
- `COMMAND.C:108-114,1985-1991` — viewport C080 and resurrect/reincarnate panel commands are separate command spaces; C160/C161 cannot create a candidate before C127/F0280

## Resolution

- corrected initial-south rerun ok: `True`
- corrected artifact: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509`
- images: `6` / unique `3`
- candidate index: `1`
- terminal index: `2`
