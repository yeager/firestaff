# Firestaff v2.9.1 — 2026-06-19

V2.2 Modern Graphics expanded from DM1-only to all five supported games
(DM1, CSB, Theron's Quest, Dungeon Master Nexus, Dungeon Master II:
Skullkeep). Each game now has a per-game modern-asset module
(`{dm1,csb,theron,nexus,dm2}_v22_modern_assets_pc34`), a first-cut
procedural asset pack at `~/.firestaff/assets/<game>/modern/`, and a
green ctest target exercising the full manifest-discovery + validation +
fallback chain. DM1 V2.2 also gains 6 more PBR hero variants (batch 4)
to bring the DM1 PBR count to 19. `docs/v22-asset-style-prompt.md` is
updated to reflect an actual `gpt-image-2` constraint: it does NOT
support `background: transparent`, and has been silently ignoring that
parameter for all four PBR batches (verified via `sips -g hasAlpha` on
all 19 generated PNGs).

## Headline features

### V2.2 for all five supported games

Each game now has a per-game `*_v22_modern_assets_pc34` module that
mirrors the DM1 V2.2 design (manifest path resolution from `dataDir`,
JSON manifest validation, modern-assets-available check, missing
placeholder, fallback chain V2.2 → V2.1 cold/warm → V2.0 → V1,
shape-path lookup, source-evidence citation). Each game also has a
per-game first-cut asset pack at `~/.firestaff/assets/<game>/modern/`
(5 procedurally-generated PNGs + manifest v1.0.0, deterministic seed
per game) that flips `*_v22_modern_assets_available()` from 0 → 1 for
the real data dir.

| Game    | Module header                          | Test ctest target                       | End-to-end smoke            |
|---------|----------------------------------------|----------------------------------------|------------------------------|
| DM1     | `dm1_v2_asset_pipeline_pc34.h` (existing v2.9.0) | `test_dm1_v22_verification` 7/7      | `m11_v22_modern_assets_available()=1` |
| CSB     | `csb_v22_modern_assets_pc34.h` (new)   | `test_csb_v22_modern_assets_pc34` 33/33 | `csb_v22_modern_assets_available()=1` |
| Theron  | `theron_v22_modern_assets_pc34.h` (new)| `test_theron_v22_modern_assets_pc34` 33/33 | `theron_v22_modern_assets_available()=1` |
| Nexus   | `nexus_v22_modern_assets_pc34.h` (new) | `test_nexus_v22_modern_assets_pc34` 33/33 | `nexus_v22_modern_assets_available()=1` |
| DM2     | `dm2_v22_modern_assets_pc34.h` (new)   | `test_dm2_v22_modern_assets_pc34` 33/33 | `dm2_v22_modern_assets_available()=1` |

ctest `-R v22_` is **8/8 green** after this pass.

### Per-game procedural first-cut packs

Each game's `.openclaw/tmp/<game>_v22_asset_author.py` (gitignored,
deterministic seed per game) installs 5 PNGs into the spec'd
categories:

| Game    | Seed    | Palette accents                      |
|---------|---------|---------------------------------------|
| CSB     | 0xC5B1  | CHAOS_PURPLE, IRON_GREY, LORD_GOLD     |
| Theron  | 0x7E10  | SUN_GOLD, TREE_BARK, HERO_GOLD         |
| Nexus   | 0x4E58  | NEXUS_CYAN, NEXUS_VIOLET, NEXUS_STONE  |
| DM2     | 0xD2D2  | FIRE_ORANGE, FIRE_DEEP, TEMPLE_STONE   |

Pattern follows DM1 v1.1.0 procedural first cut (10 PNGs, seed 0xD1A1).
Real PBR hero art (openai/gpt-image-2) for the four new games is the
follow-up; DM1's 19 PBR variants over 4 batches are the reference.

### DM1 batch 4 hero art (6 PBR variants)

| Hero asset                                       | Vision | Reference sprite (closest)   |
|--------------------------------------------------|--------|------------------------------|
| `champion_portraits/champion_ninja_hero_01.png`  | 20/20  | sprite_0378 knight+cape       |
| `champion_portraits/champion_priest_hero_01.png` | 19/20  | sprite_0378                  |
| `creature_shapes/creature_screamer_hero_01.png`  | 19/20  | sprite_0372 demon            |
| `creature_shapes/creature_giant_rat_hero_01.png` | 20/20  | sprite_0371                  |
| `floor_shapes/floor_cracked_hero_01.png`         | 18/20  | sprite_0097 (hotspot caveat) |
| `floor_shapes/floor_pit_hero_01.png`             | 18/20  | sprite_0098 (tiling caveat)  |

DM1 PBR variant count: **19** (3 + 5 + 5 + 6).
DM1 total asset pack entries: **29** across 6 categories.

### Style-prompt update: gpt-image-2 does NOT support transparent bg

All 4 batch generations of DM1 portraits + creatures used
`background: transparent` per the old style prompt. They all returned
HTTP 400 ("Transparent background is not supported for this model"). All
4 were retried successfully with `background: opaque`.

Verified via `sips -g hasAlpha` that **all 19 generated PBR PNGs across
batches 1–4 have `hasAlpha: no`** — the model has been silently ignoring
the transparent parameter this entire time. `docs/v22-asset-style-prompt.md`
now documents this explicitly:

- `Background: opaque` for ALL assets (no longer "transparent för
  creatures/items").
- `aspectRatio` is silently ignored by `gpt-image-2` (use `size` instead).
- Output filenames get a UUID suffix (basename preserved).

## Source-lock anchors added

Each new `*_v22_modern_assets_pc34.c` module cites game-specific source
references (parallel to the DM1 module's ReDMCSB references):

- **CSB:** ReDMCSB DUNVIEW.C F0128 (9-square viewport), LIGHT.C F0212
  (CSB torchlight), PANEL.C F0354 (CSB champion panel), COMMAND.C:108-113
  / 254-291, CSBWin/Viewport.cpp:7290 (9-square grid mapping),
  CSBWin/Chaos.cpp:60-69 (DSA dispatch).
- **Theron:** THQUEST.ASM T400/T520/T600 (dungeon viewport + champion
  panel + outdoor viewport), THQUEST.ASM torchlight routine,
  HuC6260/HuC6270 VDC + HuC6270 VCE (PCE graphics hardware).
- **Nexus:** SATURN_DMDF T400/T520/T600 (dungeon viewport + champion
  panel + outdoor viewport), SATURN_DMDF torchlight routine, Saturn
  VDP1 + VDP2 graphics.
- **DM2:** SKULL.ASM T520/T560/T600 (champion panel + indoor viewport +
  outdoor viewport), ReDMCSB DUNVIEW.C:2962-3047 (outdoor rendering).

## What's still TODO (next pass)

These are tracked in `TODO.md` under each game's Phase 2 section:

1. **Real PBR hero art** for CSB/Theron/Nexus/DM2 via `gpt-image-2`
   batches, parallel to DM1's 4 batches.
2. **DM1 V2.2 in-place drawing pipeline** — separate dedicated pass:
   `m11_draw_dm1_*` consume V22 shape cache in-place instead of
   overlay. Per-cell cache guard + Z-order preservation + pixel-diff
   regression vs overlay-result + V1-cell-unchanged invariant.
3. **Per-cell modern-art swap** in each game's V1 draw passes:
   - CSB: 9-square viewport (CSBWin/Viewport.cpp:7290)
   - Theron: T400 dungeon viewport + T600 outdoor viewport
   - Nexus: SATURN_DMDF T400/T600 viewports
   - DM2: indoor T560 + outdoor T600 viewports
4. **v2.10.0 minor release** when 1–3 land.

## ctest + verify status

- `ctest -R v22_` 8/8 green:
  - `csb_v22_shapes_pc34`, `csb_v22_modern_assets_pc34`
  - `theron_v22_shapes_pc34`, `theron_v22_modern_assets_pc34`
  - `nexus_v22_modern_assets_pc34`
  - `dm2_v22_modern_assets_pc34`
  - `m11_v22_shape_cache_pc34`, `m11_v22_render_overlay_pc34`
- `test_dm1_v22_verification` 7/7 sections pass (DM1 V2.2 end-to-end).
- All four new modules' tests are 33/33 each (path resolution, manifest
  validation missing/empty/partial, installed + epx-warm flag round-trips,
  full V1→V2.0→V2.1 cold/warm→V2.2 missing/installed fallback chain with
  state transitions, shape source name strings, 16x16 magenta placeholder,
  source-evidence citation).

## Version bump

- `CMakeLists.txt`: `project(Firestaff VERSION 2.9.1 LANGUAGES C)`
- `src/ui/menu_startup_m12.c`: `#define FIRESTAFF_VERSION_STRING "v2.9.1"`
- `src/shared/changelog_m12.c`: new v2.9.1 entry + return "2.9.1"
- `RELEASE_v2.9.1.md`: this file

## Files changed in this release

```
include/csb_v22_modern_assets_pc34.h        (new, 102 lines)
include/theron_v22_modern_assets_pc34.h     (new, 102 lines)
include/nexus_v22_modern_assets_pc34.h      (new, 102 lines)
include/dm2_v22_modern_assets_pc34.h        (new, 102 lines)

src/csb/csb_v22_modern_assets_pc34.c        (new, 658 lines)
src/theron/theron_v22_modern_assets_pc34.c  (new, 658 lines)
src/nexus/nexus_v22_modern_assets_pc34.c    (new, 658 lines)
src/dm2/dm2_v22_modern_assets_pc34.c        (new, 658 lines)

tests/test_csb_v22_modern_assets_pc34.c     (new, 227 lines)
tests/test_theron_v22_modern_assets_pc34.c  (new, 227 lines)
tests/test_nexus_v22_modern_assets_pc34.c   (new, 227 lines)
tests/test_dm2_v22_modern_assets_pc34.c     (new, 227 lines)

CMakeLists.txt                              (modified — test targets + ctest wiring + DM2 GLOB)
docs/v22-asset-style-prompt.md              (modified — gpt-image-2 model constraints + opaque-only note)
docs/v22-batch-progress.md                  (modified — batch 4 entry + gpt-image-2 finding)
docs/v22-compare/compare_index.json         (modified — hero_batch_4_pairs)
TODO.md                                     (modified — Theron/Nexus/DM2 Phase 2 entries)
DONE.md                                     (modified — CSB/Theron/Nexus/DM2 V2.2 entries)

CMakeLists.txt                              (version bump 2.9.0 → 2.9.1)
src/ui/menu_startup_m12.c                   (version bump)
src/shared/changelog_m12.c                  (version bump + new entry)
RELEASE_v2.9.1.md                           (new — this file)
```

## Credits

V2.2 expansion work landed in two stages:
- **2026-06-18 (v2.9.0):** DM1 V2.2 first end-to-end install. Procedural
  10-asset pack + 3 PBR hero variants. `m11_v22_set_manifest_path()` path
  bug fixed. CI green.
- **2026-06-19 (v2.9.1):** V2.2 expansion to CSB + Theron + Nexus + DM2
  (4 new modules + 4 first-cut packs). DM1 batch 4 (6 more PBR variants).
  Style-prompt updated for `gpt-image-2` opaque-only output.
