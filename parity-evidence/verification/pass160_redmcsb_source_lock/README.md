# pass160 — ReDMCSB source-lock gate for DM1/V1

Daniel explicitly said to use and analyze the ReDMCSB source because the answers are there. This pass records the concrete source anchors that must govern the next DM1/V1 parity work before emulator/capture guessing.

## Source facts locked

- `TITLE.C` uses `M526_WaitVerticalBlank()` around title rendering and keeps `Delay(25L)`; its `BUG0_71` comment says fast computers otherwise replace the title too quickly.
- `ENTRANCE.C` `F0438_STARTEND_OpenEntranceDoors()` animates entrance doors and uses repeated `M526_WaitVerticalBlank()`; its `BUG0_71` comment says the doors open too quickly without the timing limit.
- `COMMAND.C` maps left-click in `C007_ZONE_VIEWPORT` to `C080_COMMAND_CLICK_IN_DUNGEON_VIEW` and dispatches that to `F0377_COMMAND_ProcessType80_ClickInDungeonView()`.
- `CLIKVIEW.C` owns the viewport click behavior: wall sensor touch, object grab/drop/throw, front-wall knocks, door button/wall ornament region checks, and `G4055_s_LeaderHandObject.Thing` use.
- `MOVESENS.C` owns `G4055_s_LeaderHandObject`, `F0267_MOVE_GetMoveResult_CPSCE()`, and `F0275_SENSOR_IsTriggeredByClickOnWall()`.
- `DRAWVIEW.C` `F0097_DUNGEONVIEW_DrawViewport()` requests/blits the viewport and waits for vertical blank; it blits to `C007_ZONE_VIEWPORT`.

## Firestaff implications

- Treat source as the oracle. Captures verify; they do not invent behavior.
- Keep Firestaff's V1 viewport zone id as C007 (`7`).
- Keep leader-hand state as a dedicated `G4055`-equivalent transient mouse-hand object, not synthesized from champion equipment.
- Next implement/probe target should be C080 viewport click semantics: click route → door button/wall ornament/front-wall sensor → `F0275_SENSOR_IsTriggeredByClickOnWall()`/object movement flows.
