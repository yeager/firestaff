# pass514 - CSB V1 M11 runtime/capture boundary

Scope: narrow source-lock detector for the CSB V1 launch/runtime capture boundary. This pass now verifies that the old M12/M11 blocker has been retired, without claiming capture, gameplay, save compatibility or pixel parity.

## Result

The old blocker is retired and now guarded by `csb_v1_m11_runtime_capture_boundary` as a positive boundary gate.

Firestaff can catalog and hash-match CSB assets at M12, return a valid CSB launch intent, and hand CSB from M11 into the `FS_GAME_CSB` game-loop path. The PC real-asset probe then proves CSB boot, dungeon ownership and one runtime tick:

- menu_startup_m12.c includes CSB in `m12_game_supported(...)`, and `M12_StartupMenu_GetLaunchIntent(...)` still requires matched assets and renderer availability.
- m11_game_view.c handles `gameId == "csb"` by returning the `CSB READY` handoff into `FS_GAME_CSB`.
- firestaff_game_loop.c initializes the CSB boot profile, scans assets and enters `csb_v1_boot_enter_game()`.
- firestaff_csb_v1_pc_real_asset_launch_probe.c verifies PC CSB scan, variant selection, dungeon ownership and one tick.

## Primary ReDMCSB audit

Audited source: ~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/

- DEFS.H lines 468-523: DM and CSB save headers differ; CSB uses its own save-header format and dungeon IDs C12_DUNGEON_CSB_PRISON and C13_DUNGEON_CSB_GAME.
- CEDTINC8.C lines 101-118: save routing selects M746_FILE_ID_SAVE_CSBGAME_DAT for CSB prison/game IDs, separate from DM DMSAVE.DAT.
- CEDTINCH.C lines 5-63: Make New Adventure readiness requires loaded champions and CSB-aware validation; F1996_ accepts CSB only with C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK and C13_DUNGEON_CSB_GAME.
- CEDTINCU.C lines 5-77: dungeon validation switches on DM vs CSB save-header format before accepting CSB prison/game IDs.
- HINTLOAD.C lines 11-18 and 300-386: Atari CSB runtime uses HCSB.HTC, HCSB.DAT, CSBGAME.DAT, CSBGAME.BAK, opens CSBGAME.DAT, and validates C13_DUNGEON_CSB_GAME plus C1_PLATFORM_ATARI_ST.
- FLOPPYST.C lines 7-18: Atari CSB save filenames are A:\CSBGAME.DAT and A:\CSBGAME.BAK.

## Non-claims

- No CSB original capture parity or pixel parity is claimed.
- No CSB rendering, gameplay, save compatibility, audio/timing or overlay parity is claimed.
- Atari ST and Amiga emulator boot parity remains separate from this PC-first gate.

Evidence JSON: parity-evidence/verification/csb_v1_m11_runtime_capture_boundary.json
