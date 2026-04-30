# Pass: CSB/DM2 source-lock boundary guard

Scope: evidence-only guard for CSB/DM2 boundaries. No runtime behavior was changed and no DM1 V1 paths were built or edited.

Primary source audited: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

Audited ReDMCSB source line ranges:

- `DEFS.H:468-498` — `DM_SAVE_HEADER` and `CSB_SAVE_HEADER`, plus `C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER` and `C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK`.
- `DEFS.H:519-523` — dungeon ID namespace: `C10_DUNGEON_DM`, `C12_DUNGEON_CSB_PRISON`, `C13_DUNGEON_CSB_GAME`.
- `CEDTINCU.C:5-77` — `F7272_IsDungeonValid(...)`; criteria 1 accepts DM/CSB/prison, criteria 2 accepts CSB game only, criteria 3 accepts DM/prison and excludes CSB game.
- `CEDTINCH.C:5-47` — `F7086_IsReadyToMakeNewAdventure(...)` uses validation criteria 3 after loaded-game/champion checks.
- `CEDTINCH.C:49-64` — `F1996_(GAME*)` accepts only CSB-format headers with `DungeonID == C13_DUNGEON_CSB_GAME` for the SU1E path.
- `CEDTINC8.C:101-118` — save routing sends `C10_DUNGEON_DM` to `M745_FILE_ID_SAVE_DMSAVE_DAT`; CSB game/prison (`C13`/`C12`) to `M746_FILE_ID_SAVE_CSBGAME_DAT`.

Boundary conclusion:

- CSB source-lock facts are available for save-header/dungeon-ID/routing validation, but they are not a CSB runtime parity claim.
- DM2 inventory exists locally, including extracted `SKULL.ASM`, but no ReDMCSB C source-lock equivalent was found/used in this pass; DM2 remains inventory-only until a DM2-specific source/runtime matrix exists.
- Firestaff exposes three config slots (`DM1, CSB, DM2`), but this pass guards against treating that as parity support.

Verifier:

- `scripts/verify_csb_dm2_source_lock_boundaries.py`

Latest output:

```text
CSB save-header / dungeon-ID / save-routing source-lock boundary guard
redmcsb_source=/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source
original_dm=/home/trv2/.openclaw/data/firestaff-original-games/DM
repo=/home/trv2/work/firestaff

[source checks]
PASS redmcsb-save-header-formats: DEFS.H:468-498 - DM and CSB save headers are structurally separate; do not infer one from the other.
PASS redmcsb-dungeon-id-namespace: DEFS.H:519-523 - CSB/prison are source IDs 13/12, distinct from DM source ID 10.
PASS redmcsb-csb-validation-criteria: CEDTINCU.C:5-77 - CSB utility validation has explicit criteria; criteria 2 is CSB-game only, criteria 3 excludes CSB-game.
PASS redmcsb-new-adventure-boundary: CEDTINCH.C:5-47 - New Adventure is source-gated through validation criteria 3, not a generic CSB acceptance path.
PASS redmcsb-su1e-csb-game-only-boundary: CEDTINCH.C:49-64 - The SU1E-specific gate accepts only CSB-format headers for the CSB game dungeon ID.
PASS redmcsb-save-file-routing-boundary: CEDTINC8.C:101-118 - Save routing source-lock keeps DM saves and CSB saves on separate file IDs.

[reference inventory]
FOUND Game,Chaos_Strikes_Back,Amiga,Software.7z bytes=3327297 sha256=77c3b9ceb3b6d7a9cf96b7cb4801e2b7e51e6de11c5982c82342da268dfddc58
FOUND Game,Chaos_Strikes_Back,Atari_ST,Software.7z bytes=1669479 sha256=ce6e638622a099bbf15e6dacd7750ce811a52373a20b2d0f92ef6332cc47d7f5
FOUND Game,Dungeon_Master_II,DOS,Source,Disassembly,Software.7z bytes=379848 sha256=beb703174fe2e263d47e80f56d90b61fad30d2ce04a39e896e5205d6d698265a
FOUND _extracted/dm2-dos-asm/SKULL.ASM bytes=7841116 sha256=a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099

[repo boundary scan]
MATRIX ## 10. CSB and DM2 readiness (W10)
MATRIX | CSB reference inventory | `SOURCE_LOCK_INVENTORIED` | Local CSB Atari ST/Amiga archives exist; source-lock boundary guard verifies ReDMCSB save-header, dungeon-ID, validation, New Adventure, and save-routing seams before any CSB parity claims. See `scripts/verify_csb_dm2_source_lock_boundaries.py` and `parity-evidence/pass-csb-dm2-source-lock-boundary-guard.md`. |
MATRIX | CSB parity matrix created | Not started | Depends on a CSB-specific source/runtime matrix; do not reuse the DM1 V1 matrix as a CSB claim. |
MATRIX | DM2 reference inventory | `SOURCE_LOCK_INVENTORIED` | Local DM2 DOS archives and extracted `dm2-dos-asm/SKULL.ASM` exist, but no ReDMCSB C source-lock exists for DM2; keep DM2 as inventory-only until a DM2-specific source matrix is written. |
MATRIX | DM2 parity matrix created | Not started | Depends on a DM2-specific disassembly/runtime matrix; do not infer from DM1 or CSB. |
MATRIX | DM1 assumptions leaking into CSB/DM2 | Guarded by lightweight source-lock probe | The guard cites source line ranges and records that Firestaff has 3 game config slots without promoting CSB/DM2 runtime parity. |
CONFIG config_m12.h:13: M12_CONFIG_GAME_COUNT = 3  /* DM1, CSB, DM2 */
CONFIG config_m12.h:30: /* Per-game options (indexed by game slot 0..M12_CONFIG_GAME_COUNT-1) */
CONFIG config_m12.h:31: int gameUsePatch[M12_CONFIG_GAME_COUNT];
CONFIG config_m12.h:32: int gameVersionIndex[M12_CONFIG_GAME_COUNT];
CONFIG config_m12.h:33: int gameLanguageIndex[M12_CONFIG_GAME_COUNT];
CONFIG config_m12.h:34: int gameCheatsEnabled[M12_CONFIG_GAME_COUNT];
CONFIG config_m12.h:35: int gameSpeed[M12_CONFIG_GAME_COUNT];
CONFIG config_m12.h:36: int gameAspectRatio[M12_CONFIG_GAME_COUNT];
CONFIG config_m12.h:37: int gameResolution[M12_CONFIG_GAME_COUNT];
```
