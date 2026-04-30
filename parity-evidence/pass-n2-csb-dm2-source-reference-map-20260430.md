# Pass: N2 CSB/DM2 source/reference map (2026-04-30)

Scope: worker-only evidence pass on N2 (`firestaff-worker`). No emulator probing, no HUD/viewport/inventory/touch/overlay edits, no push.

## Worker and worktree

- Worktree: `/home/trv2/work/firestaff`
- Branch: `sync/n2-dm1-v1-20260428`
- Primary ReDMCSB source: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`
- Original/reference root: `/home/trv2/.openclaw/data/firestaff-original-games/DM/`
- DANNESBURK was not used.

## Source/reference availability map

### CSB / ReDMCSB C source anchors

These are source-lock anchors only; they do not claim CSB runtime/render parity.

- `DEFS.H:468-498` defines distinct `DM_SAVE_HEADER` and `CSB_SAVE_HEADER`, then `C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER` and `C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK`.
- `DEFS.H:519-523` defines dungeon IDs: `C10_DUNGEON_DM`, `C12_DUNGEON_CSB_PRISON`, `C13_DUNGEON_CSB_GAME`.
- `CEDTINCU.C:5-77`, `F7272_IsDungeonValid(...)`, source-gates validation criteria: criteria 1 accepts DM/CSB/prison; criteria 2 is CSB-game only; criteria 3 accepts DM/prison and excludes CSB-game.
- `CEDTINCH.C:5-47`, `F7086_IsReadyToMakeNewAdventure(...)`, requires loaded source game/champions and calls `F7272_IsDungeonValid(..., 3)` for supported media.
- `CEDTINCH.C:49-64`, `F1996_(GAME*)`, accepts only `C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK` with `DungeonID == C13_DUNGEON_CSB_GAME` for that SU1E path.
- `CEDTINC8.C:101-118` routes save output by dungeon ID: `C10_DUNGEON_DM` to `M745_FILE_ID_SAVE_DMSAVE_DAT`, `C13_DUNGEON_CSB_GAME`/`C12_DUNGEON_CSB_PRISON` to `M746_FILE_ID_SAVE_CSBGAME_DAT`, otherwise error path.

### CSB original/reference anchors on N2

From `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/csb/README.md`:

- `Game,Chaos_Strikes_Back,Amiga,Software.7z` bytes `3327297`, sha256 `77c3b9ceb3b6d7a9cf96b7cb4801e2b7e51e6de11c5982c82342da268dfddc58`.
- `Game,Chaos_Strikes_Back,Atari_ST,Software.7z` bytes `1669479`, sha256 `ce6e638622a099bbf15e6dacd7750ce811a52373a20b2d0f92ef6332cc47d7f5`.
- Canonical CSB dungeon payload is shared by current Amiga/Atari anchors: `sha256 3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba`.
- CSB graphics anchors differ by platform (`amiga-Graphics.DAT` sha256 `3af5396f...`, `atari-GRAPHICS.DAT` sha256 `33f672bf...`), so renderer parity must wait for a curated platform target.

### DM2 source/reference anchors on N2

From `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm2/README.md`:

- `Dungeon-Master-II-Skullkeep_DOS_EN.zip` bytes `13203537`, sha256 `d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929`.
- `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` bytes `46596215`, sha256 `a32818cd1e691b3771e091d668bf3e236ce95fde7ef75943cb7a191ed1fc7228`.
- Extracted disassembly: `_extracted/dm2-dos-asm/SKULL.ASM`, bytes `7841116`, sha256 `a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099`, `522128` lines.
- No ReDMCSB-style C source-lock equivalent was found for DM2 in this pass; DM2 remains disassembly/reference-inventory only.

## First safe non-colliding lane

Recommended first lane: **CSB save-header / dungeon-ID / save-routing source-lock guard**.

Why this lane is safe now:

- It is evidence/verifier-only and does not touch active DM1 HUD, viewport, inventory, touch, or overlay code paths.
- It starts from ReDMCSB source line ranges above, not emulator guesses.
- It can extend `scripts/verify_csb_dm2_source_lock_boundaries.py` and docs only, keeping CSB/DM2 as locked references until a platform/runtime target is chosen.

Do **not** start with CSB graphics/viewport, CSB inventory/HUD, CSB mouse/touch, or DM2 runtime/rendering. CSB graphics platform anchors diverge, and DM2 has only a DOS disassembly anchor here, so those would collide with active DM1 parity lanes or require curation first.

## Validation run

```text
$ python3 scripts/verify_csb_dm2_source_lock_boundaries.py
PASS redmcsb-save-header-formats: DEFS.H:470-498
PASS redmcsb-dungeon-id-namespace: DEFS.H:519-523
PASS redmcsb-csb-validation-criteria: CEDTINCU.C:5-77
PASS redmcsb-new-adventure-boundary: CEDTINCH.C:19-44
PASS redmcsb-save-file-routing-boundary: CEDTINC8.C:101-118
FOUND Game,Chaos_Strikes_Back,Amiga,Software.7z
FOUND Game,Chaos_Strikes_Back,Atari_ST,Software.7z
FOUND Game,Dungeon_Master_II,DOS,Source,Disassembly,Software.7z
FOUND _extracted/dm2-dos-asm/SKULL.ASM
```

Additional validation for this doc-only pass:

```text
git diff --check
```

## Missing tools/libs/errors

- No build tools or emulator were required.
- No missing source files for the CSB guard anchors above.
- Missing curation before implementation lanes: CSB target platform/version, CSB source/runtime matrix, DM2 disassembly/runtime matrix.
