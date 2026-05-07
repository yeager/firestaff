# CSB V1 source-lock inventory — 2026-05-06

Scope: concise evidence inventory for the first CSB V1 parity lane after V2. This uses only N2-local CSB/CSBWin/ReDMCSB/original-data references and does **not** enable CSB launch, rendering, gameplay, or save compatibility.

## Source/reference anchors now gated

- ReDMCSB source identity:
  - `DEFS.H:468-498` separates `DM_SAVE_HEADER` and `CSB_SAVE_HEADER` plus the DM/CSB save-header format IDs.
  - `DEFS.H:519-523` separates `C10_DUNGEON_DM`, `C12_DUNGEON_CSB_PRISON`, and `C13_DUNGEON_CSB_GAME`.
  - `CEDTINCH.C:5-64` source-locks New Adventure readiness and the CSB-game-only SU1E boundary.
  - `CEDTINC8.C:101-118` routes DM saves to `DMSAVE.DAT` and CSB game/prison saves to `CSBGAME.DAT`.
- Local CSB source tree: `<csb-source>/CSB/src`, HEAD `dda570585abb4c8113a3298d21c0b599e6cac4f9`.
  - `README:1-30` lists required play payloads: `dungeon.dat`, `hcsb.dat`, `hcsb.hct`, `mini.dat`, `graphics.dat`, `config.txt`.
  - `Graphics.cpp:1740-1915` anchors `graphics.dat` plus `CSBgraphics.dat` lookup/error boundaries.
  - `Chaos.cpp:1-40` and `Chaos.cpp:500-625` anchor the Make New Adventure/prison-save flow and `CSBGAME*` slots.
- Local CSBWin tree: `<csbwin-source>/CSBWin`, HEAD `2f63d10d9b8c155e0be17888271d394255ce1bac`.
  - `Game/readme.txt:1-30` confirms the CSBWin play workflow goes through prison entry and Make New Adventure; this is workflow evidence only, not a Firestaff parity denominator.
- Firestaff boundary: `menu_startup_m12.c` still catalogs CSB but `m12_game_supported()` remains DM1-only, so this inventory does not change launch support.

## Canonical N2 original-data anchors

| anchor | bytes | sha256 |
| --- | ---: | --- |
| Atari CSB archive | 1669479 | `ce6e638622a099bbf15e6dacd7750ce811a52373a20b2d0f92ef6332cc47d7f5` |
| Atari ST v2.0 English game MSA | 404254 | `e3d8dc75956ade33658b700e9ae2512dcef7a8dfa538116b8a717f4efaefe0b4` |
| Atari ST v2.1 English game STX | 941828 | `aea724d663554e84393a77c45c010753c5bfb1a3a5a83d1264b5ef2af9aa5c6f` |
| `_canonical/csb/atari-GRAPHICS.DAT` | 319080 | `33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af` |
| `_canonical/csb/atari-DUNGEON.DAT` | 2098 | `3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba` |
| `_canonical/csb/amiga-Graphics.DAT` | 435076 | `3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942` |
| `_canonical/csb/amiga-Dungeon.DAT` | 2098 | `3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba` |

Conclusion: dungeon payload identity currently matches across the canonical Atari/Amiga anchors, but graphics payload identity does not. Keep the curated Atari ST English v2.x lane separate; do not substitute Amiga graphics evidence.

## First landable next gaps

1. Add a CSB asset-pair manifest/verifier that requires the selected Atari ST graphics plus dungeon/runtime data before any launch intent is accepted.
2. Add an explicit experimental CSB launch-intent probe that proves menu/config routing only, with render/gameplay still disabled.
3. Resolve the curated CSB sample-save/new-adventure fixture gap, or formalize it as blocked with exact approved search roots and missing filenames.
4. Define the first CSB V1 parity surface matrix: startup/load, prison/champion route, viewport/HUD, input, save/new-adventure, with each row tied to a source anchor or `BLOCKED`.

## Non-claims

- No CSB Firestaff runtime/render parity claim.
- No CSB launch enablement claim; `menu_startup_m12.c` remains launch-gated to DM1.
- No platform-substitution claim; Atari ST and Amiga graphics anchors differ and stay separate.
- No CSB sample-save/new-adventure runtime proof; the sample-save blocker remains open.

## Gate

- Verifier: `tools/verify_csb_v1_source_lock_inventory.py`
- JSON output: `parity-evidence/verification/csb_v1_source_lock_inventory.json`
