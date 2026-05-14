# pass514 - CSB V1 M11 runtime/capture boundary

Scope: narrow source-lock and blocker detector for the next CSB V1 launch/runtime capture boundary. This pass does not enable CSB launch, runtime, capture, gameplay, save compatibility, or pixel parity.

## Result

The current blocker persists and is now guarded by csb_v1_m11_runtime_capture_boundary.

Firestaff can catalog and hash-match CSB assets at M12, but production launch must remain blocked because the downstream M11 runtime/capture path is still DM1-shaped:

- menu_startup_m12.c keeps m12_game_supported(...) DM1-only and M12_StartupMenu_GetLaunchIntent(...) depends on that guard.
- main_loop_m11.c launches V1 original through the DM1 TITLE intro and searches only DM1 TITLE anchors before opening the game view.
- m11_game_view.c has a CSB built-in branch, but it resolves to CSB.DAT; the source-locked Atari CSB lane uses DUNGEON.DAT plus GRAPHICS.DAT, HCSB.DAT, HCSB.HTC, MINI.DAT, and CSB save-game state.
- verification-screens/capture_firestaff_ingame_series.c opens the selected menu entry as a DM1 capture fixture and reports DM1 on failure.

## Primary ReDMCSB audit

Audited source: ~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/

- DEFS.H lines 468-523: DM and CSB save headers differ; CSB uses its own save-header format and dungeon IDs C12_DUNGEON_CSB_PRISON and C13_DUNGEON_CSB_GAME.
- CEDTINC8.C lines 101-118: save routing selects M746_FILE_ID_SAVE_CSBGAME_DAT for CSB prison/game IDs, separate from DM DMSAVE.DAT.
- CEDTINCH.C lines 5-63: Make New Adventure readiness requires loaded champions and CSB-aware validation; F1996_ accepts CSB only with C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK and C13_DUNGEON_CSB_GAME.
- CEDTINCU.C lines 5-77: dungeon validation switches on DM vs CSB save-header format before accepting CSB prison/game IDs.
- HINTLOAD.C lines 11-18 and 300-386: Atari CSB runtime uses HCSB.HTC, HCSB.DAT, CSBGAME.DAT, CSBGAME.BAK, opens CSBGAME.DAT, and validates C13_DUNGEON_CSB_GAME plus C1_PLATFORM_ATARI_ST.
- FLOPPYST.C lines 7-18: Atari CSB save filenames are A:\CSBGAME.DAT and A:\CSBGAME.BAK.

## Non-claims

- No CSB runtime or capture launch is enabled.
- No CSB rendering, gameplay, save compatibility, or overlay parity is claimed.
- The detector is a blocker gate: if someone enables CSB production launch before replacing the DM1-shaped M11/capture path, the gate should fail.

Evidence JSON: parity-evidence/verification/csb_v1_m11_runtime_capture_boundary.json
