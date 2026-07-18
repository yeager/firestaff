# Pass240 — DM1 PC34 FIRES.MAP/public-symbol path

Status: `VERIFIED_NEGATIVE_FIRES_MAP_PUBLIC_SYMBOLS_MISSING`

## Concrete FIRES.MAP path

- ReDMCSB script: `/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source/MKII.BAT` line `510`
- Link order: `/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source/I34E.LNK` (36 objects)
- TLINK command: `TLINK.EXE /i /s \OBJECT\I34E\STATS.EXE\C0L.OBJ @\SOURCE\I34E.LNK \TCPP101\LIB\CL.LIB,\BUILD\I34E\FIRES.EXE,\BUILD\I34E\FIRES.MAP`
- Expected output in DOS build: `C:\BUILD\I34E\FIRES.MAP`
- Expected output from a DOSBox-mounted HARDDISK tree: `<HARDDISK_MOUNT>/BUILD/I34E/FIRES.MAP`

## Verified negative on N2

- No `*.MAP`, `*.SYM`, or `*.LST` artifact exists in focused N2-local ReDMCSB/original DM1 roots.
- Prior bounded ReDMCSB DOSBox build attempt: status `BLOCKED_NO_PROMOTED_CSIP`, return `0`, `FIRES.MAP` size `0`, `DUNVIEW.OBJ` size `0`.
- Error excerpt: `<firestaff-repo>/parity-evidence/verification/pass236_dm1_v1_fires_symbol_bridge_audit/mkii_log_error_excerpt.txt`

## Source seams audited

- `COMMAND.C` `F0380_COMMAND_ProcessQueue_CPSC` line `1497` ok `True`
- `CLIKMENU.C` `F0365_COMMAND_ProcessTypes1To2_TurnParty` line `142` ok `True`
- `CLIKMENU.C` `F0366_COMMAND_ProcessTypes3To6_MoveParty` line `180` ok `True`
- `MOVESENS.C` `F0267_MOVE_GetMoveResult_CPSCE` line `152` ok `True`
- `DUNVIEW.C` `F0098_DUNGEONVIEW_DrawFloorAndCeiling` line `2962` ok `True`
- `DUNVIEW.C` `F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap` line `3113` ok `True`
- `DUNVIEW.C` `F0108_DUNGEONVIEW_DrawFloorOrnament` line `3940` ok `True`
- `DUNVIEW.C` `F0109_DUNGEONVIEW_DrawDoorOrnament` line `4013` ok `True`
- `DUNVIEW.C` `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` line `4547` ok `True`
- `DUNVIEW.C` `F0128_DUNGEONVIEW_Draw_CPSF` line `7155` ok `True`

## Exact next blocker

Produce an authentic ReDMCSB I34E FIRES.MAP via the MKII.BAT TLINK line, or provide equivalent public symbols/debugger-observed seam hits. Current N2-local DOSBox/Turbo-C attempt reaches DUNVIEW.C but emits no DUNVIEW.OBJ and no FIRES.MAP, so the immediate blocker is fixing that ReDMCSB/TCC DUNVIEW.C compile failure or obtaining a trusted FIRES.MAP from the same I34E object/link order.
