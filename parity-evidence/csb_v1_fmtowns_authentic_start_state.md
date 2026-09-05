# CSB FM Towns authentic post-Entrance start state

This note records the retail state loaded by the native F31 English and
Japanese Game programs.  It prevents an intentionally hostile Chaos Strikes
Back start from being "fixed" into a synthetic, brighter, monster-free one.

## Source path

ReDMCSB `STARTUP1.C` enters `F0435_STARTEND_LoadGame`; `LOADSAVE.C` lines
2728--2732 restore `PartyChampionCount`, `PartyMapX`, `PartyMapY`,
`PartyDirection`, and `PartyMapIndex` from `GLOBAL_DATA`.  `PANEL.C` F0337
selects LIGHT5 when a nonzero-difficulty map has neither torch power nor
magical light.  These paths include `MEDIA529_F31E_F31J`.

Firestaff reads `CDATA/MINI.DAT` or `CJDATA/MINI.DAT` directly from the real
FM Towns ZIP.  The C5 header, all five keyed F7057 save parts, and the F7063
dungeon tail must validate before any values below are exposed.

## Authenticated retail facts

Both language programs decode the same gameplay state (their saved game time
is 82 for English and 88 for Japanese):

- party map 4, position `(22,18)`, facing 2;
- one champion, `HALK`, with both hand slots empty (`THING_NONE`);
- map difficulty 4;
- eight saved active groups;
- two of those groups are immediately adjacent at `(21,18)` and `(23,18)`.

Consequently, seeing monsters beside the party and a very dark dungeon after
the Entrance is authentic retail behavior.  Substituting a torch, brighter
palette, safer coordinate, or removing either group would reduce parity.

## Executable regression

`test_csb_v1_fmtowns_archive_launch_real` now verifies the decoded facts and
then applies the complete state transaction to `CSB_V1_RuntimeProfile`.  It
asserts that map, pose, champion count, difficulty, and all eight active-group
records survive that ownership transfer.  The existing M11 integration test
also drives the real SWITCHTW Game rectangle, C004 Entrance, C002/C003 door
transition, and checks the same live tuple plus a genuine LIGHT0--LIGHT5
palette.

The remaining visual-parity work is therefore pixel-level viewport comparison
at this authentic dark start, not changing its source state.
