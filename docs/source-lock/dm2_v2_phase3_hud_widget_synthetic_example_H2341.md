# DM2 V2 Phase 3 - HUD Widget Synthetic Manifest Example

**Status:** COMPLETE - synthetic non-placeholder example landed 2026-06-28; manifest path sync guard strengthened 2026-06-29
**CTest name:** `firestaff_dm2_v2_hud_widget_synthetic_promotion_probe`
**Example path:** `examples/dm2_hud_widget_synthetic/`

## Scope

The DM2 V2 HUD widget gate already classified operator manifests as
`NO_MANIFEST`, `PLACEHOLDER`, `PARTIAL`, or `COMPLETE`, but the
repository did not include a safe non-placeholder manifest path that
could promote the gate without relying on copyrighted or unfinished
PBR HUD art.

This example adds a data-free promotion fixture:

- `examples/dm2_hud_widget_synthetic/hud_widget_manifest.json`
- two primary widget fixtures in `hud_widgets/`
- five supporting chrome fixtures in `hud_chrome/`

Every manifest entry uses `generator: "synthetic_test"`. That value is
intentionally non-placeholder so the existing gate treats disk-resolved
entries as `REAL`, but the README and source evidence mark the pack as
test-only. The PNG files are 1x1 procedural tiles tagged with a PNG tEXt
chunk (`synthetic-test-fixture`); they are not derived from DM2 art.

## Gate Coverage

`firestaff_dm2_v2_hud_widget_synthetic_promotion_probe` installs the
example into `/tmp/scratch/dm2_hwa_synthetic_promotion_probe/`, sets
the data dir to the scratch DM2 data path, and verifies:

- missing manifest -> `NO_MANIFEST`
- one copied synthetic source file -> `PARTIAL`
- all seven copied synthetic source files -> `COMPLETE`
- `installed` mirrors `PARTIAL` and `COMPLETE`
- every complete slot reports `generator == "synthetic_test"` and an
  on-disk `resolved_path`
- every complete slot reports `width > 0 && height > 0` so a corrupt
  manifest without declared dimensions cannot pass the gate
- every complete slot's `source_file` remains the canonical
  `<slot_id>.png`
- every complete slot resolves through the expected install category
  (`hud_widgets/` for the two primary widgets, `hud_chrome/` for the
  five supporting chrome slots)
- every fixture starts with the PNG 8-byte signature
  (`89 50 4E 47 0D 0A 1A 0A`) so the fixture cannot silently rot into
  arbitrary text without the probe noticing
- a rewritten manifest (every `generator` swapped from
  `"synthetic_test"` to `"pbr_hero"`) still promotes to COMPLETE, so
  the gate's classification logic is generator-string-agnostic for any
  non-placeholder marker — an operator-installed real pack whose
  generator is `"pbr_hero"` or `"ai_upscale"` is treated identically
  to the synthetic pack
- source evidence cites the synthetic example and keeps the no-finished-
  art boundary visible

The probe never writes to `~/.firestaff/` and tears down its scratch
directory at exit.

## Manifest Contract

The manifest shape is the same as the production HUD widget manifest:

```json
{
  "manifestVersion": "1.0.0",
  "packId": "dm2-v2-hud-widget-synthetic-example",
  "hud_widgets": [
    {
      "id": "inventory_quick_view",
      "generator": "synthetic_test",
      "source_file": "inventory_quick_view.png",
      "width": 64,
      "height": 32
    }
  ]
}
```

All seven entries live in the `hud_widgets` manifest array because the
current lightweight scanner walks that array, while file resolution
still uses the internal slot table category:

```text
hud_widgets/<source_file>      primary widget slots
hud_chrome/<source_file>       supporting chrome slots
```

## Source-Lock Anchors

| Anchor | Why |
|--------|-----|
| `SKULL.ASM T560` | DM2 HUD rendering pipeline |
| `skproject/SKULLWIN/c_gui_vp.cpp` | DM2 UI chrome layout |
| ReDMCSB `PANEL.C F0354` | Champion status-box drawing |
| ReDMCSB `DUNGEON.C F0260` | Stat-bar refresh timing |
| `include/dm2_v2_hud_widget_assets.h` | Gate API under test |
| `src/dm2/dm2_v2_hud_runtime.c` | REAL path-mode anchor-stamp replacement site |
| `docs/FIRESTAFF_GAP_LIST.md` | DM2 V2 Phase 3 HUD bitmap-assets row |

## Honest Boundary

This closes the testability gap, not the art gap.

`generator == "synthetic_test"` is allowed to drive the classifier in a
scratch probe, but it must not be used as evidence that Firestaff ships
finished DM2 V2 HUD widget art. Public release notes and README copy
should continue to say that real widget art and the final bitmap blit
remain open until an operator-installed or project-authored PBR pack
exists and has visual verification.
