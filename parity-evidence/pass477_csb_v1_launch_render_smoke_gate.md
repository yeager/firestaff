# pass477 — CSB V1 launch/render smoke gate

Scope: narrow CSB V1 front-door evidence only. This pass does not enable CSB runtime launch, dungeon loading, gameplay, save compatibility, or pixel parity.

Result: positive smoke evidence for the current safe boundary:

- a matched CSB catalog entry can enter the M12 game-options view;
- the game-options view renders through the startup renderer without producing a blank frame;
- pressing Launch keeps CSB blocked with the runtime-not-ready message;
- the blocker message view also renders through the startup renderer without producing a blank frame;
- the launch intent remains invalid while preserving `gameId=csb` for diagnostics.

Primary source audit: local ReDMCSB reference payload strings show CSB is not a single-graphics-file handoff. The target path needs dungeon/graphics payload handling plus CSB save-game handling before any real runtime/render claim is valid. This keeps the Firestaff CSB launcher boundary stricter than catalog hash matching alone.

Reference locations audited on N2:

- ReDMCSB reference tree: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Reference/ReDMCSB/`
- CSB startup/runtime payload string families: Atari ST CSB `START.PAK` entries (`DUNGEON.DAT`/`DUNGEON.FTL`/`GRAPHICS.DAT`) and Amiga/utility `CHAOS.FTL`/`APPB.FTL` entries (`CSBGAME.DAT`/`CSBGAME.BAK` plus dungeon/graphics names).
- Secondary cross-check only: local CSB/CSBWin source trees remain useful for utility/save-flow vocabulary but are not used here to greenlight runtime rendering.

Gate:

- `csb_v1_launch_blocker_m12` now includes the render smoke assertions as part of the existing launch-blocker CTest.

Next blocker made concrete: replace this front-door smoke with a true CSB runtime handoff only after there is an explicit CSB media manifest and loader contract covering paired dungeon/game/save/graphics payloads.
