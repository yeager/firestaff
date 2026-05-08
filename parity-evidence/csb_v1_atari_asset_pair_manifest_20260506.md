# CSB V1 Atari asset-pair manifest — 2026-05-06

Scope: evidence-only manifest for the selected Atari ST CSB V1 lane. This pass does **not** enable CSB launch, rendering, gameplay, save compatibility, or Make New Adventure behavior.

## Selected lane

`csb_atari_st_v2x_harddisk_2009_02_22_pp`

## Source audit anchors

- CSB lineage source `<csb-source>/CSB/src`, HEAD `dda570585abb4c8113a3298d21c0b599e6cac4f9`.
  - `README:14-21` names the play-directory payload set: `dungeon.dat`, `hcsb.dat`, `hcsb.hct`, `mini.dat`, `graphics.dat`, and `config.txt`.
  - `Graphics.cpp:1814-1915` separates `openGraphicsFile()` / `graphics.dat` from `OpenCSBgraphicsFile()` / `CSBgraphics.dat` error boundaries.
  - `Chaos.cpp:507-623` ties utility/new-adventure runtime data to `CSBGAME*`, `csbgame.dat`, `csbgame.bak`, and `MINI.DAT` messages.
- ReDMCSB source `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source`.
  - `DEFS.H:519-523` separates `C12_DUNGEON_CSB_PRISON` and `C13_DUNGEON_CSB_GAME`, with `C13` noted as the value used in CSB `MINI.DAT`.
  - `CEDTINCH.C:5-63` gates Make New Adventure on loaded game/champions and CSB save-header/dungeon IDs.
- CSBWin source `<csbwin-source>/CSBWin`, HEAD `2f63d10d9b8c155e0be17888271d394255ce1bac`.
  - `Game/readme.txt:1-30` confirms the workflow boundary: enter dungeon, choose prison, then Make New Adventure.
- Firestaff boundary: `menu_startup_m12.c:1180-1222` still catalogs `csb` but `m12_game_supported()` remains DM1-only.

## Manifested assets

| asset | role | bytes | sha256 |
| --- | --- | ---: | --- |
| `GRAPHICS.DAT` | selected Atari pair | 319080 | `33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af` |
| `DUNGEON.DAT` | selected Atari pair | 2098 | `3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba` |
| `HCSB.DAT` | runtime support | 30793 | `5268b36a108f582e043a0e698052ce6fe67d33132737a3dc2271caa3031e6fcc` |
| `HCSB.HTC` | runtime support | 66172 | `1b2fbff81a11928afd153f46c117355cce1f9a93f482d14d58e35a115d9cde38` |
| `MINI.DAT` | runtime support | 42815 | `61d981061bbb7a81b9b7f4795e99c24a592ee329169eb0c195459ba4eb62e3a9` |
| `_canonical/csb/atari-GRAPHICS.DAT` | canonical pair | 319080 | `33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af` |
| `_canonical/csb/atari-DUNGEON.DAT` | canonical pair | 2098 | `3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba` |

The local Atari extraction spells the support file as `HCSB.HTC`; the CSB lineage README spells `hcsb.hct`. This manifest records the observed local 8.3 payload without claiming a source spelling fix.

## Boundary

- `asset_pair_ready=true`
- `runtime_support_payloads_ready=true`
- `launch_intent_allowed=false`

CSB remains unsupported in Firestaff launch/runtime code. This manifest is the prerequisite evidence a future explicit experimental CSB launch-intent gate must consume; it is not itself launch clearance.

## Gate

```sh
python3 -m py_compile tools/verify_csb_v1_atari_asset_pair_manifest.py
./tools/verify_csb_v1_atari_asset_pair_manifest.py
python3 -m json.tool parity-evidence/verification/csb_v1_atari_asset_pair_manifest.json >/dev/null
```

JSON output: `parity-evidence/verification/csb_v1_atari_asset_pair_manifest.json`
