# Pass452 — DM1 V1 original Hall route-state blocker

- status: `BLOCKED_WRONG_FACING_AFTER_CAPTURE_ROUTE_TURNS_AWAY_FROM_INITIAL_C127`
- artifact: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/dm1-hall-dosbox-20260509/manifest.json`
- DUNGEON.DAT sha256: `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- GRAPHICS.DAT sha256: `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
- TITLE sha256: `adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745`

## Diagnosis

The N2 capture did prove a stock PC34 Hall/front-mirror visible artifact, but it clicked after this manifest route:

- select VGA
- select No Sound
- select Mouse/input
- title/menu ENTER
- turn_left_east
- step_east_blocked
- turn_left_north_front_mirror
- candidate portrait source center
- resurrect confirm source center

That route is not the source-locked initial C127 click state.  From the fresh game the party starts at map0 `(1,3)` facing **South**, already facing the C127 champion portrait sensor on `(1,4)`.  The capture then turned left to East, attempted an East step that was blocked, and turned left again to **North** before clicking x111/y82.  Since turns do not move the party and blocked steps do not move it, the click touched the north front wall, not the initial south C127 wall.

So the next blocker is **state/facing**, not coordinate geometry.  Do not keep trying visual mirror centers from the north-facing frame.

## Source locks

- `LOADSAVE.C:1940-1944` — new-game initial Hall state comes from DungeonHeader InitialPartyLocation; existing decoder/gates resolve PC34 to map0 x=1 y=3 dir=South
- `CLIKMENU.C:142-173,264-328` — turns stay on the same square; blocked steps do not move the party, so the N2 route turn-left, blocked-step, turn-left ends at the initial square facing North, not South
- `CLIKVIEW.C:347-431` — PC C080 click subtracts viewport origin, then only C05/front-wall ornament hit calls F0372 to touch the wall sensor in front of the current facing direction
- `MOVESENS.C:1309-1503` — C127 wall champion portrait is the candidate transition; it is allowed with no leader, but only for the clicked wall cell/facing precondition
- `DUNVIEW.C:525,3912-3919` — visible source portrait box is viewport x=96..127 y=35..63; with PC viewport origin y=33, center is screen x=111 y=82
- `COORD.C:1693-1698,1748-1749` — PC viewport origin is x=0 y=33 and portrait is 32x29, confirming x111/y82 screen click geometry
- `REVIVE.C:63-67,272,704-789` — F0280 appends/marks the candidate; only after that can C160/C161/C162 panel commands in F0282 finalize/cancel candidate state
- `COMMAND.C:108-114,1985-1991` — viewport C080 and resurrect/reincarnate panel commands are separate command spaces; C160/C161 cannot create a candidate before C127/F0280

## Next executable transition

Use the same stock PC34 data and DOSBox-X setup, but after entering the fresh game, do **not** turn or step.  Click the source portrait center while still at initial South-facing Hall state:

1. select VGA / No Sound / Mouse as before
2. press `Return Return` to enter the fresh game
3. capture first Hall frame: expected map0 `(1,3,S)`
4. click PC screen `x=111,y=82` (N2 root mapping from this run: `x=302,y=264`)
5. wait/capture `candidate_select`; expect resurrect/reincarnate/cancel panel
6. then click C160 resurrect `x=130,y=115` (root `x=340,y=330`) or C161 reincarnate `x=186,y=115`

Route edit: remove the turn_left_east, step_east_blocked, and turn_left_north_front_mirror inputs before the portrait click.

Exact command shell:

```sh
xvfb-run -a -s "-screen 0 800x600x24" dosbox-x -conf ~/openclaw-artifacts/dm1-hall-dosbox-20260509/dosboxx.conf -fastlaunch -nogui -nomenu -time-limit 150
```
