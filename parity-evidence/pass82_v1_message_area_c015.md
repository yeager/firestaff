# Pass 82 — V1 message area C015 lock

## Source evidence

- `DEFS.H:3247` defines `M532_MESSAGE_AREA_ROW_COUNT` as `4` for the PC build.
- `DEFS.H:3756` defines `C015_ZONE_MESSAGE_AREA` as zone id `15`.
- `TEXT.C:121` calls `F0638_GetZone(C015_ZONE_MESSAGE_AREA, ...)` when preparing the message area backing row.
- `TEXT.C:1383` clears the message area with `F0733_FillZoneByIndex(C015_ZONE_MESSAGE_AREA, C00_COLOR_BLACK)`.
- `TEXT.C:1461` / `TEXT.C:1481` redraw message rows by taking C015 and stepping each row by `G2088_C7_TextLineHeight`.
- `TEXT.C:1593` and `TEXT.C:1631` show the PC print path at `(row * 7) + 177`.
- `COORD.C:1758` sets `G2088_C7_TextLineHeight = 7`.
- `COORD.C:1789` sets the PC build `G2092_MessageAreaWidth = 320`; `TEXT.C:120` and `DRAWMSGA.C:107` use it as the message-area blit width.
- `zones_h_reconstruction.json` records C014 as a `320x27` bottom container and C015 as bottom-anchored under C014, resolving to the source bottom message band `(0,173,320,27)`.

## Firestaff change

The V1 renderer now gives normal chrome mode a dedicated source-style message-area pass:

- clears full-width `(0,173,320,27)` black instead of the old inset prompt strip;
- draws up to four player-facing message rows with 7-pixel stride;
- places PC rows at y `177,184,191,198`;
- keeps synthetic telemetry/debug strings filtered out of the player-facing C015 surface.

This is deliberately not V2 polish: it removes an invented Firestaff prompt-strip feel and restores the small, full-width, bottom-scrolling DM1 message surface.

## Probe coverage

`INV_GV_300AM` now locks C015 to `(0,173,320,27)`.

`INV_GV_300AM2` renders a synthetic telemetry line (`PARTY MOVED`) plus a player-facing line (`IT COMES UP HEADS.`) and asserts that only the player-facing row reaches C015.
