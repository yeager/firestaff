# DM2 V2 HUD Widget Asset - Synthetic-Test Example Manifest

This directory contains a **synthetic-test fixture** that exercises the
DM2 V2 HUD widget asset manifest gate (`dm2_v2_hud_widget_assets`).
It is NOT a real PBR HUD widget pack. It exists so the gate's
PARTIAL/COMPLETE promotion logic can be exercised without shipping
copyrighted DM2 art.

## Honest boundary

| What this example IS                          | What this example IS NOT                  |
|-----------------------------------------------|-------------------------------------------|
| Installable manifest + asset bytes for tests  | Real DM2 PBR / V2.2 HUD widget art        |
| `generator == "synthetic_test"` marker        | Finished visual asset                     |
| Drives the gate from NO_MANIFEST to PARTIAL / COMPLETE | Visible / shippable art material     |
| Tags every PNG with `synthetic-test-fixture`  | Decoded / blitted at runtime by the gate  |

The gate's `classify_slot()` only checks that `source_file` resolves on
disk via `fopen()` - it does NOT decode pixel data. The 1x1 procedural
PNG tiles here are sufficient to drive the gate state machine, and they
are obviously not real HUD art.

## Layout

```
examples/dm2_hud_widget_synthetic/
|-- README.md                          (this file)
|-- hud_widget_manifest.json           (7 slots, generator=synthetic_test)
|-- hud_widgets/                       (Phase 3 primary slots)
|   |-- inventory_quick_view.png       (1x1 RGBA fixture)
|   `-- action_prompt.png              (1x1 RGBA fixture)
`-- hud_chrome/                        (chrome supporting slots)
    |-- compass_rose.png               (1x1 RGBA fixture)
    |-- depth_indicator.png            (1x1 RGBA fixture)
    |-- gold_counter.png               (1x1 RGBA fixture)
    |-- champion_bar_frame.png         (1x1 RGBA fixture)
    `-- action_strip_frame.png         (1x1 RGBA fixture)
```

## How the CTest probe uses this example

`firestaff_dm2_v2_hud_widget_synthetic_promotion_probe` copies this
directory into `/tmp/scratch/dm2_hwa_synthetic_promotion_probe/`,
sets the manifest path to the synthetic data dir, and asserts:

1. NO_MANIFEST gate with no manifest on disk.
2. PARTIAL gate after copying the manifest plus one `generator=synthetic_test`
   source file.
3. COMPLETE gate after copying the full 7-slot synthetic source-file set.
4. installed flag mirrors PARTIAL/COMPLETE.
5. Source evidence citations mention SKULL.ASM T560, ReDMCSB PANEL.C,
   this example path, and the no-finished-art boundary.

The probe tears down its scratch dir at the end so the user's real
`~/.firestaff/` data is never touched.

## Fixture integrity guarantees

The probe also locks three small, bounded content-integrity guarantees
so the example cannot silently rot without the probe noticing:

| Guarantee                            | How it is verified                                                    |
|--------------------------------------|------------------------------------------------------------------------|
| PNG signature                         | First 8 bytes of each fixture match `89 50 4E 47 0D 0A 1A 0A`         |
| `synthetic-test-fixture` tEXt chunk   | Raw byte substring search in each fixture file                          |
| Manifest slot dimensions populated    | Per-slot `info.width > 0 && info.height > 0` after COMPLETE promotion  |
| Manifest path sync                    | Per-slot `source_file == <slot_id>.png` and resolved category path      |

A future refactor that replaces a 1x1 PNG with arbitrary text (which
would still contain the `synthetic-test-fixture` substring) would
still pass the tEXt check but fail the PNG signature check. A future
refactor that drops the manifest's `width` / `height` fields would
fail the dimension sanity check. A future manifest edit that leaves the
gate promotable but drifts away from the documented install layout would
fail the path-sync check.

## Generator-agnostic behaviour

The DM2 V2 HUD widget gate classifies a slot as REAL iff its
`generator` string is not `"placeholder"` and the `source_file`
resolves on disk. The gate is intentionally generator-string-agnostic
— `"synthetic_test"`, `"pbr_hero"`, `"ai_upscale"`, or any future
operator-installed marker must all promote the gate identically. As a
guard against a future refactor that accidentally introduces
per-generator allowlisting, the probe installs the example manifest
after rewriting every `generator` entry from `"synthetic_test"` to
`"pbr_hero"`, copies all seven fixtures, and verifies:

- the rewritten manifest validates structurally;
- the gate still promotes to COMPLETE;
- every slot's recorded `generator` matches the rewritten marker
  (not the source's `synthetic_test`);
- the rewritten manifest no longer mentions `synthetic_test`.

This is a documentation-grade regression guard, not a real pack
shipment: the rewritten manifest is installed only inside the probe's
`/tmp/scratch` directory and torn down at exit.

## How an operator would install a REAL pack

The real install path is identical except for two things:

1. `generator` should be a real-art marker (e.g. `pbr_hero`,
   `ai_upscale`) instead of `synthetic_test`.
2. `source_file` should resolve to a real PBR / V2.2 PNG, not a 1x1
   procedural fixture.

Real install path:
```
~/.firestaff/assets/dm2/hud/hud_widget_manifest.json
~/.firestaff/assets/dm2/hud/hud_widgets/<slot_id>.png
~/.firestaff/assets/dm2/hud/hud_chrome/<slot_id>.png
```

`dm2_v2_hud_widget_assets_set_manifest_path("~/.firestaff/data/dm2")`
resolves the manifest path by walking two parents up from the data
dir and appending `assets/dm2/hud/hud_widget_manifest.json`.

## Source-lock anchors

| Anchor                                    | Why                                    |
|-------------------------------------------|----------------------------------------|
| `SKULL.ASM T560`                          | DM2 HUD rendering pipeline             |
| `skproject/SKULLWIN/c_gui_vp.cpp`          | DM2 UI chrome layout                   |
| ReDMCSB `PANEL.C F0354`                   | Champion status-box drawing            |
| ReDMCSB `DUNGEON.C F0260`                 | Stat-bar refresh timing                |
| `include/dm2_v2_hud_widget_assets.h`      | Module under test                      |
| `include/dm2_v22_modern_assets_pc34.h`    | Sibling V2.2 manifest pattern          |
| `docs/FIRESTAFF_GAP_LIST.md` D2 V2 Phase 3 row | Gate closes this gap row            |
| `docs/source-lock/dm2_v2_phase3_hud_widget_synthetic_example_H2341.md` | Companion source-lock doc |

## V1 invariant

This example never affects V1 command routes, inventory, dungeon state,
or any V1 chrome. The DM2 V2 HUD widget pipeline is gated on the
Phase 3 phase gate (`DM2_V2_PHASE_DOMAIN_HUD`) and only renders when
V2 is active; V1 fallback is byte-stable.

## DO NOT install this at `~/.firestaff/assets/dm2/hud/`

Doing so would make the gate report PARTIAL/COMPLETE without any real
art to back it. The runtime's `render_with_assets()` would mark
slots as REAL via the path-mode record but only stamp the 1-pixel
anchor marker (the documented replacement site for real-bitmap blit).
