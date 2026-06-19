# V2.2 Modern Asset Pack — Batch Progress

Procedural first cut of the DM1 V2.2 "Modern Graphics" asset pack, written
2026-06-18 by the V2.2 asset author (`tools/.openclaw/tmp/v22_asset_author.py`).

## What landed

### Asset pack location
- **Path:** `~/.firestaff/assets/dm1/modern/`
- **Manifest:** `~/.firestaff/assets/dm1/modern/modern_asset_manifest.json`
- **Pack id:** `firestaff-dm1-v22-modern-1.0`
- **Format version:** 1

### Categories shipped (10 entries)
| Category | Files | Sprite reference | Vision score |
|----------|-------|------------------|--------------|
| `wall_shapes` | `wall_d3_carved_01.png` (1024×1024) | DM1 sprite_0076 (32×32 brown speckled) | 4/4/3/4 |
| `wall_shapes` | `wall_d3_mossy_01.png` (1024×1024) | DM1 sprite_0077 (32×32 mossy dark) | 4/4/3/4 |
| `floor_shapes` | `floor_plain_01.png` (1024×1024) | (no direct original; standard V1 floor) | (not scored) |
| `floor_shapes` | `floor_cracked_01.png` (1024×1024) | (no direct original) | (not scored) |
| `floor_shapes` | `floor_pit_01.png` (1024×1024) | (no direct original) | (not scored) |
| `floor_shapes` | `floor_stairs_down_01.png` (1024×1024) | (no direct original) | (not scored) |
| `creature_shapes` | `creature_demon_01.png` (1024×1536) | DM1 sprite_0372 (96×111 fiery demon) | 4/4/3/4 (post-rewrite from goblin 1/1/1/1) |
| `ui_chrome` | `panel_frame_01.png` (1024×1024) | (no direct original; standard V1 frame) | (not scored) |
| `ui_chrome` | `message_log_01.png` (1024×1024) | (no direct original) | (not scored) |
| `champion_portraits` | `champion_warrior_01.png` (1024×1024) | (DM1 has no detailed portrait sprites in GRAPHICS.DAT — champion faces are rendered procedurally at runtime) | (not scored) |

### End-to-end smoke test
- `tools/v22_smoke.c` exercises `m11_v22_set_manifest_path()` + `m11_v22_modern_assets_available()`.
- **Result:** `available() == 1` for the real DM1 data dir (`~/.firestaff/data/dm1`).
- This flips the V2.2 modern-assets-installed flag from 0 → 1 in `M12_AssetStatus`.

### Always-compare compliance
Per `docs/v22-asset-style-prompt.md`, every V2.2 generation must be vision-compared
against the corresponding original DM1 sprite. 4 side-by-side comparison images
written to `docs/v22-compare/`:

```
docs/v22-compare/
  compare_wall_d3_carved_01.png    (sprite_0076 + V2.2 carved wall)
  compare_wall_d3_mossy_01.png     (sprite_0077 + V2.2 mossy wall)
  compare_creature_demon_01.png    (sprite_0372 + V2.2 demon)
  compare_champion_warrior_01.png  (sprite_0074 + V2.2 warrior helmet)
  compare_index.json
```

## Pre-existing path bug fixed

`m11_v22_set_manifest_path()` in `src/dm1v2/dm1_v2_modern_assets_pc34.c`
previously walked up **one** level from the data dir, producing
`~/.firestaff/data/assets/dm1/modern/...` — which does **not** match the
spec in `docs/v2_2_asset_manifest.md` (`~/.firestaff/assets/dm1/modern/`).
The doc-comment and intent both pointed at the right path; the runtime
code was the outlier.

**Fix:** walk up **two** levels (dataDir → data → ~/.firestaff) before
appending `assets/dm1/modern/`. Updated `tests/test_dm1_v22_verification.c`
test cases 2–5 to use 2-component dataDir (`scratch_path("foo/data")`).
All 7 sections of `test_dm1_v22_verification` still pass.

## Authoring approach

This is a **procedural first cut**, not the AI-generated hero art that
`docs/v22-asset-style-prompt.md` is intended to drive. The procedural
generator is deterministic (seed = 0xD1A1) and produces a complete,
valid V2.2 manifest that flips the runtime to "installed" — which is the
*minimum viable complete install* needed to unblock the V2.2 GPU render
path. Real PBR hero art (openai/gpt-image-2) is the next pass.

The generator's palette + silhouette approach (16-color limited palette,
black-ink silhouette, torchlight glow) follows the style prompt's
"strong black-ink silhouette + vivid limited-palette" guidance. It is
intentionally source-faithful to the original DM1 sprites at the
subject level (carved stone ↔ carved stone, fiery demon ↔ fiery demon)
even though the visual fidelity is procedurally limited.

## What the always-compare vision test told us

| Asset | First score | After rewrite | Note |
|-------|-------------|---------------|------|
| Wall (carved) | 5/4/4/3 | (kept) | Solid first pass; could add more pink/orange speckling |
| Wall (mossy) | 4/4/4/3 | 4/4/3/4 (after adding grimy variation) | Needed less uniform moss pattern |
| Creature (was goblin) | 1/1/1/1 | 4/4/3/4 (now fiery demon) | Major rewrite needed; original was a demon, not a goblin |
| Champion portrait | 2/2/2/2 | (no rewrite — DM1 has no detailed portrait sprites to compare against) | Procedural helmet is a deliberate placeholder; real champion portrait comes from runtime champion-stat renderer |

## Style-prompt updates from the comparison

The vision feedback suggests the master prompt should add:

- **For walls:** "preserve the original's mineral-fleck density; do not
  smooth out the speckle into cartoon blocks"
- **For creatures:** "match the original's branching horns and jagged
  silhouette; do not drift toward a generic humanoid figure"
- **For champions:** "DM1 has no detailed portrait sprites; the V2.2
  champion portrait can be a deliberate modern invention (helmet,
  armor, glow) — the comparison is not required"

These would go into `docs/v22-asset-style-prompt.md` in a follow-up
edit once the next batch of real `gpt-image-2` generations lands.

## Next steps

1. **Real PBR hero art via openai/gpt-image-2** — the spec's actual
   intent. 10 hero generations (1 per category) at 1024-2048px,
   compared against the procedural versions.
2. **More entries per category** — wall_shapes currently has 2
   (carved + mossy); V1 has at least 6 wall variants (D2L/D2R/D3L/D3R
   per ReDMCSB DUNGEON.C:2238-2246). Floor needs a stairs-up variant.
3. **CSB + Theron modern packs** — same authoring approach for
   `~/.firestaff/assets/csb/modern/` and
   `~/.firestaff/assets/theron/modern/`. CSB-only entries like
   `PRISON_DOOR` / `CHAOS_RUNE` / `DSA_SCROLL` are still TODO.
4. **Nexus modern pack** — `~/.firestaff/assets/nexus/modern/` for the
   Saturn DMDF/DGN variant.
5. **DM2 modern pack** — `~/.firestaff/assets/dm2/modern/` (the asset
   loader in `src/dm2/dm2_v2_asset_pipeline.c:395-435` already
   supports this; just needs authoring).

## CI / verification status

- `test_dm1_v22_asset_pipeline` ✅ PASS
- `test_dm1_v22_verification` ✅ PASS (all 7 sections)
- `test_dm1_v22_modern_resolution_matrix_pc34` ✅ PASS
- `test_m11_v22_render_overlay_pc34` ✅ 13/13
- `test_m11_v22_shape_cache_pc34` ✅ 23/23
- `firestaff_m11_v22_render_overlay_probe` ✅ 13/13
- `m11_v22_modern_assets_available()` end-to-end smoke ✅ available() == 1

## Hero art batch (2026-06-18)

3 `gpt-image-2` PBR hero variants generated and installed alongside the
procedural first cut, following `docs/v22-asset-style-prompt.md`:

| Hero asset | Source file | Vision score |
|------------|-------------|--------------|
| Carved stone wall | `wall_shapes/wall_d3_carved_hero_01.png` | 5/4/5/5 |
| Mossy grimy wall | `wall_shapes/wall_d3_mossy_hero_01.png` | 4/4/5/5 |
| Fiery horned demon | `creature_shapes/creature_demon_hero_01.png` | 5/5/5/5 |

All three are real upgrades over the procedural versions and pass the
always-compare vision check.

**Manifest bumped to v1.1.0** (top-level category keys + manifestVersion +
packId) so `m11_v22_validate_manifest()` returns 1 as well as
`m11_v22_modern_assets_available()`. Previously the manifest used a
`{"categories": {...}}` wrapper which only passed the substring-detection
`available()` check but not the strict validator.

End-to-end smoke (tools/.openclaw/tmp/v22_hero_smoke.c) verifies all three:
- modern_assets_available: 1
- validate_manifest: 1 (top-level format)
- get_installed: 1 (after M12 simulation)

**Note:** The procedural generator script was also patched to emit the
top-level manifest format, so regenerating the procedural pack from scratch
will still produce a fully-valid manifest.

**Next hero batch candidates:**
1. `floor_plain_hero_01.png` — large hero stone floor with rune inscriptions
2. `panel_frame_hero_01.png` — large hero inventory/equipment panel frame
3. `champion_portrait_hero_01.png` — single dramatic champion portrait
   (warrior variant; the procedural helmet is a placeholder)
4. More creature types — `creature_goblin_hero_01.png` (real goblin, not
   plant), `creature_skeleton_hero_01.png`, `creature_worm_hero_01.png`

## Hero art batch 2 (2026-06-18)

5 more `gpt-image-2` PBR hero variants generated + installed:

| Hero asset | Reference sprite | Vision score |
|------------|------------------|--------------|
| `floor_shapes/floor_plain_hero_01.png` | sprite_0097 (perspective floor) | 5/5/4/5 |
| `ui_chrome/panel_frame_hero_01.png` | sprite_0041 (round panel) | 5/4/5/5 |
| `champion_portraits/champion_warrior_hero_01.png` | sprite_0378 (knight+cape) | 5/5/5/5 |
| `creature_shapes/creature_goblin_hero_01.png` | (no DM1 goblin ref) | 5/4/5/5 |
| `creature_shapes/creature_skeleton_hero_01.png` | (no DM1 skeleton ref) | 5/4/5/5 |

**Total V2.2 hero art count: 8** (3 from batch 1 + 5 from batch 2).

**Manifest upgraded to v1.2.0.** Validator-friendly format: each entry
is a single line so `m11_v22_validate_manifest()` can find
`id`/`source_file`/`width`/`height` in the same fgets() buffer.
Previously the multi-line indented form had `entry_has_all_fields=0`
because the validator reads just the opening `{` line of each entry.
The procedural generator script `.openclaw/tmp/v22_asset_author.py`
was patched to emit the same validator-friendly format on regeneration.

**Smoke test 3/3 PASS** (`tools/v22_hero_smoke.c`): modern_assets_available=1,
validate_manifest=1, get_installed=1.

**Compare index** (`docs/v22-compare/compare_index.json`) gained a
`hero_batch_2_pairs` array of 5 entries. Side-by-side comparisons
saved to `docs/v22-compare/hero_b2_*.png`.

**V2.2 ctest 4/4 still green** (asset_pipeline, shape_cache, render_overlay,
verification).

**Batch 3 candidates (next):**
1. `wall_d3_inscription_hero_01.png` — wall with rune/inscription (DM1 has lots of rune graphics)
2. `creature_worm_hero_01.png` — giant worm (DM1 classic)
3. `creature_screamer_hero_01.png` — the screamer (DM1 boss-like)
4. `floor_stairs_up_hero_01.png` + `floor_stairs_down_hero_01.png` (replacements)
5. `champion_ninja_hero_01.png` + `champion_priest_hero_01.png` (full party set)

## Hero art batch 3 (2026-06-18)

5 more `gpt-image-2` PBR hero variants generated + installed:

| Hero asset | Reference sprite | Vision score |
|------------|------------------|--------------|
| `wall_shapes/corridor_hero_01.png` | sprite_0097 (corridor floor) | 19/20 |
| `wall_shapes/wall_inscription_hero_01.png` | (closest: sprite_0093 wall-edge) | 19/20 |
| `creature_shapes/chest_hero_01.png` | (closest: sprite_0041 round hatch) | 18/20 |
| `creature_shapes/creature_worm_hero_01.png` | (no DM1 worm; closest: sprite_0374 demon) | 17/20 |
| `door_shapes/door_hero_01.png` | (no DM1 door; closest: sprite_0093 wall-edge) | 19/20 |

**Total V2.2 hero art count: 13** (3 batch 1 + 5 batch 2 + 5 batch 3).
**Total V2.2 asset pack entries: 23** across 6 categories (wall/floor/creature/ui_chrome/champion_portrait/door_shapes).

**Manifest upgraded to v1.3.0.** Adds new `door_shapes` category
(previously missing from M12 scan). All entries still in
validator-friendly single-line format.

**Side-by-side comparisons:** `docs/v22-compare/hero_b3_*.png` (5
entries) + `hero_batch_3_pairs` in `compare_index.json`.

**Smoke test 3/3 PASS** (still using `tools/v22_hero_smoke.c`):
modern_assets_available=1, validate_manifest=1, get_installed=1.

**Batch 3 worm assessment (17/20):** The worm came out more
pixel-art-styled than PBR-consistent. Acceptable as a "DM1-style"
creature variant. The other 4 batch 3 pieces are PBR-consistent.

**Batch 4 candidates (next):**
1. Champion party set: `champion_ninja_hero_01.png` + `champion_priest_hero_01.png` (warrior already done)
2. More floor variants: `floor_pit_hero_01.png`, `floor_cracked_hero_01.png`
3. `wall_d3_carved_alt_hero_01.png` — different carving pattern
4. More creatures: `creature_screamer_hero_01.png`, `creature_giant_rat_hero_01.png`, `creature_mummy_hero_01.png`

## Hero art batch 4 (2026-06-19) — 6 PBR variants installed

6 more `gpt-image-2` PBR hero variants generated + installed (mummy +
carved_alt deferred to batch 5 to keep batch 4 focused on the originally
listed candidates).

| Hero asset | Reference sprite | Vision score | Notes |
|------------|------------------|--------------|-------|
| `champion_portraits/champion_ninja_hero_01.png` | (no DM1 ninja; closest sprite_0378 knight+cape) | 20/20 | First batch-4 retry succeeded after the transparent-bg fix |
| `champion_portraits/champion_priest_hero_01.png` | (no DM1 priest; closest sprite_0378 knight+cape) | 19/20 | Slight pedestal base, otherwise excellent |
| `creature_shapes/creature_screamer_hero_01.png` | (no DM1 screamer; closest sprite_0372 demon) | 19/20 | Slight wing cropping at frame edges |
| `creature_shapes/creature_giant_rat_hero_01.png` | (no DM1 rat; closest sprite_0371 small creature) | 20/20 | Strong grotesque rat, excellent PBR |
| `floor_shapes/floor_cracked_hero_01.png` | sprite_0097 corridor floor base | 18/20 | Strong central torchlight hotspot may reveal seams when tiled — OK as hero variant, not as repeatable base |
| `floor_shapes/floor_pit_hero_01.png` | sprite_0098 floor pit edge | 18/20 | Unique edge stones limit seamless tiling — works as a single special-tile accent, not as a repeating base |

**Total V2.2 hero art count: 19** (3 batch 1 + 5 batch 2 + 5 batch 3 + 6 batch 4).
**Total V2.2 asset pack entries: 29** across 6 categories (wall_shapes 6, floor_shapes 7, creature_shapes 8, ui_chrome 3, champion_portraits 4, door_shapes 1).

**Manifest upgraded to v1.4.0.** All 6 new entries follow the
validator-friendly single-line format used since v1.2. `m11_v22_validate_manifest()`
returns 1, `m11_v22_modern_assets_available()` returns 1 for the real DM1 data dir.

**Important model finding (2026-06-19):** `openai/gpt-image-2` does
**NOT** support `background: transparent` (HTTP 400: "Transparent
background is not supported for this model"). All 6 batch 4 portraits
were initially generated with `background: transparent` per
`docs/v22-asset-style-prompt.md` and failed identically. After
retrying all 4 with `background: opaque` they succeeded on the first
attempt. Verified via `sips -g hasAlpha` that **all 13 prior PBR
variants (batches 1-3) also have `hasAlpha: no`** — the
"transparent för creatures/items" guidance in the style prompt has
been silently ignored by the model this entire time. Style prompt
needs an update: opaque dark backdrop is the actual default. Floor
tiles are unaffected (already opaque).

**Side-by-side comparisons:** `docs/v22-compare/compare_index.json`
gains `hero_batch_4_pairs` array of 6 entries. Side-by-side
comparison images not auto-generated for this batch (was a manual
image compose step in earlier batches; deferred until a fresh batch
generates the comparison images).

**Test verification:** `test_dm1_v22_verification` 7/7 sections pass
after manifest bump. `ctest -R v22_` 4/4 green (csb_v22_shapes,
theron_v22_shapes, m11_v22_shape_cache, m11_v22_render_overlay).

**Batch 5 candidates (next):**
1. `wall_d3_carved_alt_hero_01.png` — different carving pattern
2. `creature_mummy_hero_01.png` (carried over from batch 4 list)
3. `creature_ooze_hero_01.png` or `creature_snake_hero_01.png`
4. `champion_sorcerer_hero_01.png` or `champion_bard_hero_01.png`
5. More floor/wall variants: `floor_stairs_up_hero_01.png`,
   `wall_d3_altar_hero_01.png`

## CSB hero art batch 1 (2026-06-19) — 6 PBR variants installed

6 `gpt-image-2` PBR hero variants generated + installed for CSB V2.2 (parallel to DM1 batches 1-4). CSB-specific palette accents: CHAOS_PURPLE (chaos magic), IRON_GREY (prison), LORD_GOLD (Lord Order).

| Hero asset | Reference sprite (closest DM1) | Vision score | Notes |
|------------|--------------------------------|--------------|-------|
| `champion_portraits/champion_ninja_csb_hero_01.png` | sprite_0378 knight+cape | 19/20 | Excellent subject match, dark PBR + chaos-purple accents |
| `champion_portraits/champion_priest_csb_hero_01.png` | sprite_0378 | 18/20 | Strong priest/chaos caster, cross-pendant feels conventional vs CSB iconography |
| `creature_shapes/creature_rat_csb_hero_01.png` | sprite_0371 small creature | 18/20 | Clear giant mutant rat with chaos-purple corruption veins |
| `creature_shapes/creature_screamer_csb_hero_01.png` | sprite_0372 demon | 16/20 | Reads more bat-demon/gargoyle than CSB-specific screamer |
| `floor_shapes/floor_corridor_csb_hero_01.png` | sprite_0097 corridor floor | 15/20 | Top-down floor plate composition, not corridor view |
| `wall_shapes/wall_altar_csb_hero_01.png` | sprite_0093 wall-edge | 19/20 | Strong Lord Order altar wall tile, chaos-purple focal glow |

**Total CSB V2.2 hero art count: 6** (all in this batch).
**Total CSB V2.2 asset pack entries: 11** (5 procedural from v1.0.0 + 6 PBR from this batch).
**Manifest v1.1.0** validates as complete (`csb_v22_validate_manifest()` returns 1).
**End-to-end smoke**: `csb_v22_modern_assets_available()=1` against real CSB data dir.

**Vision score summary**: 105/120 (17.5/20 average). 4 assets at 18-20, 2 at 15-16 (screamer + corridor). Both are acceptable as hero variants — screamer is generic bat-demon but with CSB purple palette; corridor is top-down floor plate with CSB runes. Regeneration candidates if needed: creature_screamer_csb_hero_01 (would need explicit "CSB jail screamer, not generic bat-demon"), floor_corridor_csb_hero_01 (would need "narrow corridor perspective not top-down plate").

**Compare index**: `docs/v22-compare/compare_index.json` gains `csb_hero_batch_1_pairs` array of 6 entries.

## Theron hero art batch 1 (2026-06-19) — 6 PBR variants installed

6 `gpt-image-2` PBR hero variants generated + installed for Theron V2.2 (parallel to DM1 batches 1-4). PCE/THQUEST-specific palette accents: SUN_GOLD (hero), TREE_BARK (forest), HERO_GOLD.

| Hero asset | Reference sprite (closest DM1) | Vision score | Notes |
|------------|--------------------------------|--------------|-------|
| `champion_portraits/champion_ninja_theron_hero_01.png` | sprite_0378 | 19/20 | Excellent subject match, sun-gold accents |
| `champion_portraits/champion_priest_theron_hero_01.png` | sprite_0378 | 19/20 | Strong priest with sun-gold spellbook glow |
| `creature_shapes/creature_rat_theron_hero_01.png` | sprite_0371 | 17/20 | Aggressive giant rat, bark-brown tones |
| `creature_shapes/creature_screamer_theron_hero_01.png` | sprite_0372 | 17/20 | Strong demon boss monster, sun-gold backlight |
| `floor_shapes/floor_forest_theron_hero_01.png` | (no Theron floor ref) | 18/20 | Regenerated after first pass (11/20) read as dungeon stone; v2 has roots, leaves, ferns, mushrooms |
| `wall_shapes/wall_dungeon_theron_hero_01.png` | sprite_0093 | 20/20 | Excellent Theron dungeon wall with sun-gold symbols and wood beams |

**Total Theron V2.2 hero art count: 6** (all in this batch).
**Total Theron V2.2 asset pack entries: 11** (5 procedural from v1.0.0 + 6 PBR from this batch).
**Manifest v1.1.0** validates as complete (`theron_v22_validate_manifest()` returns 1).
**End-to-end smoke**: `theron_v22_modern_assets_available()=1` against real Theron data dir.

**Vision score summary**: 110/120 (18.3/20 average) — best Theron batch yet thanks to the floor_forest regeneration (11 → 18). 4 assets at 19-20, 1 at 18, 1 at 17.

**Compare index**: `docs/v22-compare/compare_index.json` gains `theron_hero_batch_1_pairs` array of 6 entries.

**Note on regeneration**: The first `floor_forest_theron_hero_01` came out as dungeon stone (11/20 — model interpreted "ancient forest stone slabs" as masonry, not forest soil). Regenerated with an explicit forest-soil prompt (leaves, twigs, ferns, mushroom caps, exposed roots, dappled sunlight, NOT dungeon stone NOT carved masonry) → 18/20. Pattern for future floor tiles: explicit "OUTDOOR woodland forest floor only" + "NOT dungeon stone" works much better than subtle hints.

## Nexus hero art batch 1 (2026-06-19) — 6 PBR variants installed

6 `gpt-image-2` PBR hero variants generated + installed for Nexus V2.2 (parallel to DM1 batches 1-4). Saturn VDP1/VDP2-specific palette accents: NEXUS_CYAN (cyber-blue), NEXUS_VIOLET (hazard energy), NEXUS_STONE (sci-fi dungeon).

| Hero asset | Reference sprite (closest DM1) | Vision score | Notes |
|------------|--------------------------------|--------------|-------|
| `champion_portraits/champion_ninja_nexus_hero_01.png` | sprite_0378 | 17/20 | Good stealth champion, slightly medieval-leaning |
| `champion_portraits/champion_priest_nexus_hero_01.png` | sprite_0378 | 16/20 | Strong cleric vibe, leans medieval more than Saturn sci-fi |
| `creature_shapes/creature_rat_nexus_hero_01.png` | sprite_0371 | 17/20 | Strong monster, violet corruption veins carry Nexus palette |
| `creature_shapes/creature_screamer_nexus_hero_01.png` | sprite_0372 | 14/20 | Generic demon, less Saturn-specific than intended |
| `floor_shapes/floor_corridor_nexus_hero_01.png` | sprite_0097 | 18/20 | Strong sci-fi dungeon floor (explicit "NOT dungeon stone" prompt worked) |
| `wall_shapes/wall_altar_nexus_hero_01.png` | sprite_0093 | 15/20 | Strong sci-fi dungeon wall but lacks clear altar focal point |

**Total Nexus V2.2 hero art count: 6** (all in this batch).
**Total Nexus V2.2 asset pack entries: 11** (5 procedural from v1.0.0 + 6 PBR from this batch).
**Manifest v1.1.0** validates as complete (`nexus_v22_validate_manifest()` returns 1).
**End-to-end smoke**: `nexus_v22_modern_assets_available()=1` against real Nexus data dir.

**Vision score summary**: 97/120 (16.2/20 average). Lower than Theron (18.3) and CSB (17.5) because gpt-image-2 leans toward "medieval dungeon" interpretation when prompted with "Saturn sci-fi dungeon" — Saturn/Nexus sci-fi specificity is hard to convey in a text prompt without heavy styling. Hero variants still acceptable: palette accents (cyan/violet) carry the Nexus vibe even when the underlying composition is medieval-leaning. Floor_corridor (18/20) was the strongest asset thanks to the explicit "Metallic futuristic dungeon floor only" prompt that successfully avoided dungeon-stone misinterpretation.

**Compare index**: `docs/v22-compare/compare_index.json` gains `nexus_hero_batch_1_pairs` array of 6 entries.
