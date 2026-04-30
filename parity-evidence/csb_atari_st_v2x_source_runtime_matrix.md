# CSB Atari ST English v2.x source/runtime matrix

Worker-only evidence note. This defines the CSB Atari ST English v2.x lane before any renderer parity work. It is source/provenance evidence only; it does not change or claim Firestaff CSB runtime parity.

## Source anchors

Primary source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- ReDMCSB scope/version anchor: `../Documentation/Readme.htm:11-22` says ReDMCSB covers reverse-engineered DM/CSB executables and explicitly lists `Chaos Strikes Back for Atari ST 2.0 English` and `2.1 English`.
- Save-header split: `DEFS.H:468-498` defines separate `DM_SAVE_HEADER` and `CSB_SAVE_HEADER`; CSB carries `Platform` and `DungeonID` in the CSB-format header.
- Format/platform/dungeon IDs: `DEFS.H:503-523` defines CSB Atari ST format/platform constants and the dungeon namespace: DM=`10`, CSB prison=`12`, CSB game=`13`.
- Validation criteria: `CEDTINCU.C:5-77` reads either DM or CSB header layout, then validates criteria 1/2/3 with CSB game accepted only where `C13_DUNGEON_CSB_GAME` is explicit.
- New Adventure boundary: `CEDTINCH.C:5-64` gates New Adventure through validation criteria 3 or, for the SU1E branch, accepts only CSB-format headers with `DungeonID == C13_DUNGEON_CSB_GAME`.
- Save routing: `CEDTINC8.C:101-118` routes DM saves to `DMSAVE.DAT` and CSB game/prison saves to `CSBGAME.DAT`; do not reuse DM save assumptions.
- Atari ST graphics lineage refs: `../Documentation/Readme.htm:199-202` says Atari ST versions uniquely bulk-initialize globals from `GRAPHICS.DAT` graphics #558-#562; `DUNGEON.C:1825` explicitly cites CSB Atari ST 2.0/2.1 dungeon sizing; `../Documentation/Engine.htm:10-18` identifies the Engine page as Atari-ST-source-only and notes a 2.1-only CSB Atari ST behavior difference.

## Canonical runtime inputs

Target lane: **Chaos Strikes Back for Atari ST English v2.x**. Do not substitute Amiga graphics for renderer parity.

| input | path/member | bytes | sha256 | status |
| --- | --- | ---: | --- | --- |
| Atari archive | `/home/trv2/.openclaw/data/firestaff-original-games/DM/Game,Chaos_Strikes_Back,Atari_ST,Software.7z` | 1669479 | `ce6e638622a099bbf15e6dacd7750ce811a52373a20b2d0f92ef6332cc47d7f5` | present |
| v2.0 game disk image | `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/csb-atari/Floppy Disks MSA/Chaos Strikes Back for Atari ST Game Disk v2.0 (English).msa` | 404254 | `e3d8dc75956ade33658b700e9ae2512dcef7a8dfa538116b8a717f4efaefe0b4` | present |
| v2.0 root `GRAPHICS.DAT` | member inside v2.0 MSA | 272069 | `cff31dbdc071af2c6de8a0b9e1110b189e067706868d42fc8b2267e18422f687` | canonical renderer input |
| v2.0 root `DUNGEON.DAT` | member inside v2.0 MSA | 2098 | `59a72978879f3a3e9de3a6767ee069266d369244b1091314ddc16c03d8d41530` | canonical dungeon input |
| v2.1 game disk image | `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/csb-atari/Floppy Disks STX/Chaos Strikes Back for Atari ST Game Disk v2.1 (English).stx` | 941828 | `aea724d663554e84393a77c45c010753c5bfb1a3a5a83d1264b5ef2af9aa5c6f` | present, not yet root-member parsed by verifier |
| Atari hard-disk candidate `GRAPHICS.DAT` | `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/csb/atari-GRAPHICS.DAT` | 319080 | `33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af` | reference only; differs from v2.0 game-disk member |
| Atari hard-disk candidate `DUNGEON.DAT` | `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/csb/atari-DUNGEON.DAT` | 2098 | `3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba` | reference only; matches Amiga canonical dungeon candidate |
| Amiga `Graphics.DAT` blocker | `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/csb/amiga-Graphics.DAT` | 435076 | `3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942` | explicit non-target lineage |

## Required runtime/capture tooling inputs before parity

- Emulator/capture target must be Atari ST v2.x, not Amiga, PC-98, X68000, or a hard-disk convenience port unless separately proven equivalent.
- Capture must preserve 320x200 ST output, palette timing, and raw/screenshot frame identity; any scaled capture needs a documented raw-frame source.
- Runtime harness must mount the selected v2.x disk/data set and record exact emulator/version, disk image hash, boot/utility/game disk sequence, save disk handling, and input script.
- First render comparison must use the v2.0 MSA root `GRAPHICS.DAT`/`DUNGEON.DAT` hashes above, or block with a documented reason to switch to a parsed v2.1 STX member set.

## Blockers before renderer parity

1. Parse or otherwise curate the v2.1 STX root members if v2.1 is the target; current verifier locks the STX image hash only.
2. Add an original Atari ST capture probe that produces verifiable 320x200 frames and records emulator/tool versions.
3. Add a CSB-specific Firestaff source/runtime gate; do not reuse DM1 HUD/inventory/pass84/runtime code as evidence.
4. Reject Amiga `Graphics.DAT` for this lane unless a later source+asset proof establishes a separate Amiga parity target.

## Verifiers

- `python3 tools/validate_csb_dm2_source_lock.py --root /home/trv2/.openclaw/data/firestaff-original-games/DM --markdown-out /tmp/csb_dm2_report.md --json-out /tmp/csb_dm2_report.json`
- `python3 scripts/verify_csb_dm2_source_lock_boundaries.py --repo /home/trv2/work/firestaff --redmcsb-source /home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source --original-dm /home/trv2/.openclaw/data/firestaff-original-games/DM`

Latest verifier result: source checks PASS; CSB Atari ST v2.0 MSA root `GRAPHICS.DAT`/`DUNGEON.DAT` members match expected hashes; Amiga graphics remains a renderer-lineage blocker.
