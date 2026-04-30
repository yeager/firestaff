# Pass: N2 CSB target platform/version curation (2026-04-30)

Scope: worker-only evidence/verifier pass on N2 (`firestaff-worker`). No emulator probing, no push, and no active DM1 HUD/inventory/pass84/runtime code touched.

## Worktree and branch

- Worktree: `/home/trv2/work/firestaff`
- Branch: `sync/n2-dm1-v1-20260428`
- Primary source: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`
- Canonical refs: `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/csb/`

## Source identity anchors first

- `DEFS.H:468-498` defines distinct `DM_SAVE_HEADER` vs `CSB_SAVE_HEADER` and `C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER` vs `C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK`.
- `DEFS.H:519-523` defines `C10_DUNGEON_DM`, `C12_DUNGEON_CSB_PRISON`, and `C13_DUNGEON_CSB_GAME`.
- `CEDTINCU.C:5-77` gates dungeon validity by criteria; criteria 2 is CSB-game only and criteria 3 accepts DM/prison but excludes CSB-game.
- `CEDTINCH.C:5-64` gates New Adventure and SU1E CSB-game-only acceptance.
- `CEDTINC8.C:101-118` routes DM saves to `M745_FILE_ID_SAVE_DMSAVE_DAT` and CSB game/prison saves to `M746_FILE_ID_SAVE_CSBGAME_DAT`.

## Canonical CSB references inspected

From `_canonical/csb/README.md` and verifier sha256s:

- Atari archive: `Game,Chaos_Strikes_Back,Atari_ST,Software.7z`, bytes `1669479`, sha256 `ce6e638622a099bbf15e6dacd7750ce811a52373a20b2d0f92ef6332cc47d7f5`.
- Amiga archive: `Game,Chaos_Strikes_Back,Amiga,Software.7z`, bytes `3327297`, sha256 `77c3b9ceb3b6d7a9cf96b7cb4801e2b7e51e6de11c5982c82342da268dfddc58`.
- Atari dungeon: `_canonical/csb/atari-DUNGEON.DAT`, bytes `2098`, sha256 `3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba`.
- Amiga dungeon: `_canonical/csb/amiga-Dungeon.DAT`, bytes `2098`, sha256 `3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba`.
- Atari graphics: `_canonical/csb/atari-GRAPHICS.DAT`, bytes `319080`, sha256 `33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af`.
- Amiga graphics: `_canonical/csb/amiga-Graphics.DAT`, bytes `435076`, sha256 `3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942`.

## Curation result

- Dungeon identity is shared between current Atari and Amiga canonical anchors: both dungeon DAT payloads have the same size/hash.
- The exact blocker is graphics lineage: Atari `GRAPHICS.DAT` and Amiga `Graphics.DAT` differ in both size and sha256, so their renderer/asset evidence is not interchangeable.
- Curated next target lane: **Atari ST English v2.x**. Reason: N2 has official English Atari ST v2.0/v2.1 disk images, including `_extracted/csb-atari/Floppy Disks STX/Chaos Strikes Back for Atari ST Game Disk v2.1 (English).stx`, while the current extracted Amiga graphics canonical anchor is from `Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/.../Graphics.DAT`. Keep Amiga v3.x as a separate graphics lineage until an official Amiga English graphics anchor is curated.

## Verifier update

`scripts/verify_csb_dm2_source_lock_boundaries.py` now prints canonical CSB dungeon/graphics sha256s and a `[csb target curation]` section that records the Atari-vs-Amiga graphics blocker and the Atari ST English v2.x recommendation.
