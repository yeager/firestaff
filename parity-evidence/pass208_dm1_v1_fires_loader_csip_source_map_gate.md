# Pass208 DM1 V1 FIRES loader CS:IP/source-map gate

Classification: `blocked/decompressed-runtime-address-map-required`
Exact remaining blocker: Stock FIRES is LZEXE v0.91; this pass proves the compressed loader entry CS:IP and source seams, but no decompressed FIRES memory dump or TLINK FIRES.MAP artifact is present on N2, so source symbols cannot yet be converted to stock runtime CS:IP breakpoints.

## Loader facts
- Stock original FIRES: `$HOME/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DungeonMasterPC34/FIRES` size `94779` sha256 `ebf84045c3edbce7690b826eadbea2e278fbb4c0a3cc19a470552586f37712eb`.
- ReDMCSB bundled original I34E FIRES matches stock: `True`.
- ReDMCSB rebuilt/reference I34E FIRES is available: `$HOME/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Reference/ReDMCSB/I34E/FIRES` size `94841` sha256 `3e59c1a8d4dec64c3cac1fd06a064077754cd012bef8d627cca969cad32311d0`.
- Stock MZ/LZEXE signature at relocation table: `LZ91`.
- Stock compressed loader entry CS:IP: `1665:000e` relative to DOS load-image base (`PSP+0x10`); linear offset `0x1665e`.
- Stack at loader entry: `29d2:0080`.

## Source seams verified
- PASS `command_accepted` — `COMMAND.C:2095-2127,2150-2156,2322-2324`; symbols: `F0380_COMMAND_ProcessQueue_CPSC`, `F0365_COMMAND_ProcessTypes1To2_TurnParty`, `F0366_COMMAND_ProcessTypes3To6_MoveParty`, `F0377_COMMAND_ProcessType80_ClickInDungeonView`
- PASS `movement_applied` — `CLIKMENU.C:256-270,317-329,345-347`; symbols: `F0366_COMMAND_ProcessTypes3To6_MoveParty`, `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`, `F0267_MOVE_GetMoveResult_CPSCE`
- PASS `party_coordinates_committed` — `MOVESENS.C:438-444,493-496,573-578`; symbols: `F0267_MOVE_GetMoveResult_CPSCE`, `G0306_i_PartyMapX`, `G0307_i_PartyMapY`
- PASS `viewport_present` — `GAMELOOP.C:83-91,164-168,215-219`; symbols: `F0128_DUNGEONVIEW_Draw_CPSF`, `F0380_COMMAND_ProcessQueue_CPSC`
- PASS `viewport_present_blit` — `DRAWVIEW.C:849-858`; symbols: `F0097_DUNGEONVIEW_DrawViewport`, `VIDRV_09_BlitViewPort`
- PASS `viewport_buffer_composed` — `DUNVIEW.C:8608-8616`; symbols: `F0097_DUNGEONVIEW_DrawViewport`, `F0098_DUNGEONVIEW_DrawFloorAndCeiling`

## Link/map evidence
- I34E link order file: `$HOME/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source/I34E.LNK` (36 objects).
- First objects: `GAMELOOP.OBJ`, `PROJEXPL.OBJ`, `DIALOG.OBJ`, `DUNGEON.OBJ`, `MENUDRAW.OBJ`, `GROUP.OBJ`, `MEMORY.OBJ`, `OBJECT.OBJ`.
- Existing `*.MAP` artifacts under ReDMCSB tree: `0`.
- Expected map if the PC toolchain build is run: `\BUILD\I34E\FIRES.MAP` (from `MKII.BAT` TLINK line for `@\SOURCE\I34E.LNK`).

## Next exact unblocker
1. Produce either a decompressed stock FIRES memory dump or an I34E `FIRES.MAP` from the ReDMCSB/Turbo C toolchain.
2. Bind map/source symbols to runtime by using the MZ load base (`PSP+0x10`) and the LZEXE post-decompression transfer point, not the compressed loader entry alone.
3. Set debugger breakpoints for `command_accepted`, `movement_applied`, `party_coordinates_committed`, and `viewport_present` using `parity-evidence/verification/pass208_dm1_v1_fires_loader_csip_source_map_gate/next_debugger_breakpoints.md`.

## Non-claims
- does not claim a decompressed original FIRES runtime image base
- does not claim stock FIRES source symbols are mapped to runtime CS:IP
- does not claim debugger breakpoints were hit
- does not use <private-host> or non-N2 references
