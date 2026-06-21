# pass547 - CSB V1 runtime readiness backfill

Scope: source-lock/runtime readiness probe for CSB V1. pass547 is now a retired blocker gate: the old M12/M11 launch blocker is gone, but broader CSB parity remains separately gated.

## Result

The old runtime-readiness blocker is retired. Firestaff can now turn a matched CSB catalog entry into a CSB launch intent, hand it from M11 into the `FS_GAME_CSB` path, and prove a PC CSB real-asset boot/tick with `csb_v1_pc_real_asset_launch`.

pass547_csb_v1_runtime_readiness_backfill verifies three things:

- ReDMCSB treats CSB as a distinct saved-game/header/runtime lane, with CSB header format, CSB dungeon IDs, Atari platform checks, CSB support payloads and CSBGAME.DAT/CSBGAME.BAK routing.
- Firestaff now admits CSB in the M12 launch guard, returns a valid CSB launch intent for matched assets, and M11 hands CSB to `FS_GAME_CSB` instead of the DM1 dungeon-loader path.
- The positive PC real-asset gate proves CSB scan, boot, dungeon ownership and one runtime tick. Original capture parity, pixel parity, Atari/Amiga emulator parity and broad gameplay/rendering coverage remain non-claims.

## Primary ReDMCSB audit

Audited source: ~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/

- DEFS.H lines 482-523: CSB_SAVE_HEADER is separate from DM, C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK identifies CSB saves, and CSB uses dungeon IDs C12_DUNGEON_CSB_PRISON and C13_DUNGEON_CSB_GAME.
- HINTLOAD.C lines 11-18: Atari CSB support/runtime filenames are HCSB.HTC, HCSB.DAT, CSBGAME.DAT and CSBGAME.BAK.
- HINTLOAD.C lines 300-390: the Atari CSB loader opens CSBGAME.DAT, loads/deobfuscates the saved-game header and parts, accepts the CSB game only when dungeon/platform checks pass, and marks GameLoaded = C1_TRUE.
- CEDTINCH.C lines 5-63: Make New Adventure readiness requires loaded game/champions and F1996_, which accepts CSB only for C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK plus C13_DUNGEON_CSB_GAME.
- CEDTINCU.C lines 5-77: dungeon validation switches on save-header format before accepting CSB prison/game dungeon IDs.

## Firestaff positive boundary

- menu_startup_m12.c lines 2327-2337: the launcher support guard includes CSB among the five runtime-backed games.
- menu_startup_m12.c lines 7440-7512: `M12_StartupMenu_GetLaunchIntent` requires supported game ID, renderer availability, matched asset status and a matched selected version.
- m11_game_view.c lines 6673-6705: `gameId == "csb"` returns a CSB-specific handoff and emits the `CSB READY` milestone for probes.
- firestaff_game_loop.c lines 420-436: the `FS_GAME_CSB` path scans CSB assets and enters `csb_v1_boot_enter_game()`.
- firestaff_csb_v1_pc_real_asset_launch_probe.c lines 1-130: the PC CSB probe verifies hash scan, PC variant selection, dungeon ownership and one tick.

## Required next proof

The old launch blocker no longer needs a future replacement. Remaining finish work should land as narrower CSB evidence:

- CSB deterministic original/Firestaff capture fixtures before viewport/UI/render or overlay parity points count.
- CSB-specific gameplay, save, audio and rendering gates beyond boot/tick.
- Atari ST and Amiga emulator boot parity once legal TOS/Kickstart ROMs are available.

## Non-claims

- No CSB original capture parity or pixel parity is claimed.
- No CSB rendering, gameplay, save compatibility, audio/timing or overlay parity is claimed.
- Passing this gate means the historical blocker is retired, not that CSB is finished end to end.

Evidence JSON: parity-evidence/verification/pass547_csb_v1_runtime_readiness_backfill.json
