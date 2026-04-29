# pass161 — C080 viewport click source route

This pass takes the ReDMCSB source-lock from pass160 and pins the first executable C080 route invariant in the Firestaff M11 game-view probe.

## Source chain

- `COMMAND.C` secondary movement table maps left mouse on `C007_ZONE_VIEWPORT` to `C080_COMMAND_CLICK_IN_DUNGEON_VIEW`.
- `COMMAND.C` dispatches C080 to `F0377_COMMAND_ProcessType80_ClickInDungeonView()`.
- `CLIKVIEW.C` implements the actual dungeon-view click semantics after routing: door button/wall ornament region, wall sensor touch, object grab/drop/throw, and `G4055_s_LeaderHandObject` handling.

## Firestaff gate added

`probes/m11/firestaff_m11_game_view_probe.c` now checks:

- `INV_GV_434A`: C007 top-left edge routes to C080.
- `INV_GV_434B`: C007 bottom-right edge routes to C080.
- `INV_GV_434C`: one pixel outside C007 does not route to C080 or any left-click movement command.

This prevents the current C080 bridge from being only a center-point coincidence.
