# pass273 — DM1 V1 FIRES public-symbol unblock

Date: 2026-05-06
Worktree: `firestaff`
Status: `UNBLOCKED_PUBLIC_SYMBOLS_FOUND_NO_RUNTIME_HOOK`

## ReDMCSB source audit first

Primary source root: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- `COMMAND.C:1-16` ok=True missing=[]
- `COMMAND.C:1734-1812` ok=True missing=[]
- `COMMAND.C:2045-2156` ok=True missing=[]
- `CLIKMENU.C:142-328` ok=True missing=[]
- `GAMELOOP.C:55-91` ok=True missing=[]
- `MOVESENS.C:316-556` ok=True missing=[]
- `DUNVIEW.C:8318-8611` ok=True missing=[]

## Public map inputs found on N2

- FIRES.MAP: `<firestaff-data>/redmcsb-n2-build-probe/ibm-pc-i34e-fires/HARDDISK/BUILD/I34E/FIRES.MAP` (104565 bytes, sha256 `eb85fee47611b4368bc218f0768937174ceab98e4bec2fa860059429288bd8c0`)
- DM.MAP: `<firestaff-data>/redmcsb-n2-build-probe/ibm-pc-i34e-dm/HARDDISK/BUILD/I34E/DM.MAP` (26161 bytes, sha256 `c438a75e4c29c88a744621aeacac49aca4f414a83b3f6aeb39159bf40fd80cc8`)
- Binary policy: no original/decompressed executable copied or committed; this pass records map paths, hashes, and derived addresses only.

## Global address bindings from FIRES.MAP + PC.H aliases

Load segment used from pass246 lineage: `0733`.

| source | public | link | runtime | line |
| --- | --- | --- | --- | --- |
| G0296_puc_Bitmap_Viewport | KBJ6QT | 24ED:3D2A | 2C20:3D2A | 1119 |
| G0297_B_DrawFloorAndCeilingRequested | G0297_B_DrawFloorAndCeilingRequested | 24ED:1A74 | 2C20:1A74 | 849 |
| G0305_ui_PartyChampionCount | WCUDICL7 | 24ED:3D52 | 2C20:3D52 | 1251 |
| G0308_i_PartyDirection | QEQRGB4 | 24ED:3C92 | 2C20:3C92 | 1182 |
| G0306_i_PartyMapX | W9JOPB2 | 24ED:3C94 | 2C20:3C94 | 1249 |
| G0307_i_PartyMapY | IQFBAUTV8Z | 24ED:3CE0 | 2C20:3CE0 | 1100 |
| G0309_i_PartyMapIndex | WXYKIUAQBP | 24ED:3C8A | 2C20:3C8A | 1263 |
| G0310_i_DisabledMovementTicks | ZPFIGRFT8FM | 24ED:3C9A | 2C20:3C9A | 1287 |
| G0311_i_ProjectileDisabledMovementTicks | PM1CAJ3O5 | 24ED:3D28 | 2C20:3D28 | 1174 |
| G0312_i_LastProjectileDisabledMovementDirection | ZH5OGNXB8COW | 24ED:3CE4 | 2C20:3CE4 | 1283 |
| G0321_B_StopWaitingForPlayerInput | G0321_B_StopWaitingForPlayerInput | 24ED:1A7C | 2C20:1A7C | 851 |
| G0432_as_CommandQueue | J0OV9EXG6 | 24ED:3E7A | 2C20:3E7A | 1103 |
| G0433_i_CommandQueueFirstIndex | B6SYPBHVQH3 | 24ED:3EC8 | 2C20:3EC8 | 286 |
| G0434_i_CommandQueueLastIndex | G0434_i_CommandQueueLastIndex | 24ED:1F08 | 2C20:1F08 | 863 |
| G0435_B_CommandQueueLocked | G0435_B_CommandQueueLocked | 24ED:1F0A | 2C20:1F0A | 864 |
| G0436_B_PendingClickPresent | GZPWC3PM6 | 24ED:3EC6 | 2C20:3EC6 | 1078 |

## Code seam bindings from FIRES.MAP

| source | link | runtime | meaning |
| --- | --- | --- | --- |
| F0359_COMMAND_ProcessClick_CPSC | 1BC1:030D | 22F4:030D | COMMAND.C mouse/click queue writer |
| F0380_COMMAND_ProcessQueue_CPSC | 1BC1:0699 | 22F4:0699 | COMMAND.C command dequeue/dispatch |
| F0365_COMMAND_ProcessTypes1To2_TurnParty | 1771:010D | 1EA4:010D | CLIKMENU.C turn handler |
| F0366_COMMAND_ProcessTypes3To6_MoveParty | 1771:01AA | 1EA4:01AA | CLIKMENU.C move handler |
| F0284_CHAMPION_SetPartyDirection | 0D06:000D | 1439:000D | CHAMPION.C direction write |
| F0267_MOVE_GetMoveResult_CPSCE | 1126:0516 | 1859:0516 | MOVESENS.C movement result / coordinate writes |
| F0128_DUNGEONVIEW_Draw_CPSF | 1C7A:40FE | 23AD:40FE | DUNVIEW.C viewport compose |
| F0097_DUNGEONVIEW_DrawViewport | 20D6:1E31 | 2809:1E31 | DRAWVIEW.C viewport present request/blit |

## pass270 correction

The preserved `FIRES.MAP` shows `F0267` at `1126:0516`, `F0365` at `1771:010D`, and `F0366` at `1771:01AA`. Adding load segment `0733` yields `1859:0516`, `1EA4:010D`, and `1EA4:01AA`, matching prior candidate addresses. So pass270's additional runtime `+0733` column was a double-add for these map-backed candidates.

## Blocker status / exact next step

The public-symbol route is now unblocked: use the runtime addresses above for the next stock DOS debugger run. This pass still does **not** claim a runtime hook. The required next proof is a dosbox-debug transcript showing controlled `kp5`/`kp4`/`kp6` input writing `G0432`, dequeuing via `F0380`, mutating `G0308` or `G0306/G0307`, and then consuming that tuple in `F0128`.

Manifest: `parity-evidence/verification/pass273_dm1_v1_fires_public_symbol_unblock/manifest.json`
