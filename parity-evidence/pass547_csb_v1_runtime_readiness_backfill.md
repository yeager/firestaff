# pass547 - CSB V1 runtime readiness backfill

Scope: source-lock/runtime readiness probe for CSB V1. This is a blocker-present gate, not a launch enabler.

## Result

The runtime blocker is not just missing code. The source chain says CSB V1 needs its own runtime and capture path before Firestaff can turn a matched CSB catalog entry into a launchable game.

pass547_csb_v1_runtime_readiness_backfill verifies three things:

- ReDMCSB treats CSB as a distinct saved-game/header/runtime lane, with CSB header format, CSB dungeon IDs, Atari platform checks, CSB support payloads and CSBGAME.DAT/CSBGAME.BAK routing.
- Firestaff still deliberately blocks CSB at M12 and the downstream M11/capture path remains DM1-shaped.
- CSB V1 completion stays at 20/100. Runtime, viewport/render, gameplay, audio/timing and original overlay criteria remain at zero until CSB-specific runtime/capture evidence exists.

## Primary ReDMCSB audit

Audited source: ~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/

- DEFS.H lines 482-523: CSB_SAVE_HEADER is separate from DM, C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK identifies CSB saves, and CSB uses dungeon IDs C12_DUNGEON_CSB_PRISON and C13_DUNGEON_CSB_GAME.
- HINTLOAD.C lines 11-18: Atari CSB support/runtime filenames are HCSB.HTC, HCSB.DAT, CSBGAME.DAT and CSBGAME.BAK.
- HINTLOAD.C lines 300-390: the Atari CSB loader opens CSBGAME.DAT, loads/deobfuscates the saved-game header and parts, accepts the CSB game only when dungeon/platform checks pass, and marks GameLoaded = C1_TRUE.
- CEDTINCH.C lines 5-63: Make New Adventure readiness requires loaded game/champions and F1996_, which accepts CSB only for C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK plus C13_DUNGEON_CSB_GAME.
- CEDTINCU.C lines 5-77: dungeon validation switches on save-header format before accepting CSB prison/game dungeon IDs.

## Firestaff blocker boundary

- menu_startup_m12.c lines 1408-1414: production launch support remains DM1-only.
- main_loop_m11.c lines 196-270: V1 original title discovery searches DM1 TITLE anchors.
- main_loop_m11.c lines 698-728: launch handoff plays the ReDMCSB title intro and entrance transition before opening the game view.
- m11_game_view.c lines 1003-1015: the built-in CSB branch still resolves CSB.DAT, while the source-locked Atari CSB manifest is based on DUNGEON.DAT, GRAPHICS.DAT, HCSB.DAT, HCSB.HTC, MINI.DAT and CSB save state.
- verification-screens/capture_firestaff_ingame_series.c lines 213-218: the deterministic ingame capture fixture is still DM1-specific.

## Required next proof

Before CSB V1 can move beyond 20 %, a future pass must replace the guarded boundary with CSB-specific runtime/capture handling:

- CSB launch handoff after M12, not the DM1 TITLE/entrance path.
- Atari CSB manifest-aware runtime open/load path, not the current CSB.DAT built-in branch.
- CSB deterministic capture fixture before viewport/UI/render or overlay points count.

## Non-claims

- No CSB runtime or capture launch is enabled.
- No CSB rendering, gameplay, save compatibility, audio/timing or overlay parity is claimed.
- Passing this gate means the blocker is documented and guarded, not solved.

Evidence JSON: parity-evidence/verification/pass547_csb_v1_runtime_readiness_backfill.json
