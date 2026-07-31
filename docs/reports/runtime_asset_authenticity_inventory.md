# Runtime Asset Authenticity Inventory

Updated: 2026-07-31

## Admission policy

Production game data is admitted by a known content hash. A familiar filename
is not provenance: another port, a damaged dump, or a test fixture may be
called `GRAPHICS.DAT` or `DUNGEON.DAT`.

The shared asset pipeline and M11 dungeon resolver now reject filename-only
data. The existing recursive hash scanner remains responsible for renamed
files and for archive members once they have been materialized into the
runtime cache.

## Production routes

| Area | Real material available locally | Runtime rule | Remaining work |
| --- | --- | --- | --- |
| DM1 V1 | PC 3.4 `GRAPHICS.DAT` and `DUNGEON.DAT` | Decode source records; missing material is no-draw. | Original DOS/Mac capture and save corpus. |
| CSB V1 | PC 3.4 and Atari ST packages | Decode package-owned surfaces; missing source spans are no-draw. | Wider original capture and DSA/save corpus. |
| DM2 V1 | PC `GRAPHICS.DAT`, `DUNGEON.DAT`, GDAT records | Boot-owned GDAT provider is the only production pixel owner. | Complete GDAT HUD/dungeon decode. |
| Theron V1 | Retail BIN/CUE/Track 02 corpus | Track 02 admission blocks unproven bitmap and level output. | Authentic later-level object/bitmap decode. |
| Nexus V1 | Retail `MENU.BPK`, DGN/SLEV and Saturn media | PRS3/DGN admission blocks unproven menu or dungeon output. | Authentic PRS3 pixel/palette and DGN face decode. |

## Deliberate non-production exceptions

- Unit tests may create minimal byte fixtures, but fixture assets never pass
  the runtime hash admission route.
- Debug HUD and diagnostic geometry are explicitly gated by debug mode and
  are not V1 game rendering.
- V2.x is opt-in custom presentation. A V2.2 pack without a proven source
  mapping is not promoted to V1 replacement material; the runtime retains the
  admitted V1/V2.1 pixels or no-draws the unbound region.

## Regression coverage

`test_firestaff_asset_pipeline_hash_scan` proves both properties: renamed,
hash-matched game data loads, while arbitrary files with canonical names are
rejected by the generic and multilingual DM1 pipelines.
