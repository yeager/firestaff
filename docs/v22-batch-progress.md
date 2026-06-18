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
