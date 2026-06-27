# Theron V2 Phase 3 — HUD Widget Asset Manifest + Gate

**Status:** ✅ COMPLETE — initial seed landed 2026-06-27
**CTest names:** `theron_v2_hud_widget_assets_pc34` (105/105 PASS),
`firestaff_theron_v2_hud_widget_assets_probe` (65/65 PASS)
**Module files:**
- `include/theron_v2_hud_widget_assets_pc34.h`
- `src/theron/theron_v2_hud_widget_assets_pc34.c`
- `tests/test_theron_v2_hud_widget_assets_pc34.c`
- `probes/firestaff_theron_v2_hud_widget_assets_probe.c`

**Sibling gate:** `include/dm2_v2_hud_widget_assets.h` — DM2 V2
Hud Widget Asset Manifest + Gate (the original Phase 3
placeholder-vs-real pattern that this Theron module mirrors).

---

## Scope

Phase 3 (initial seed) of the Theron V2 enhanced UI overlay
plan needs a machine-checkable way to decide whether the runtime
should draw its overlay elements procedurally (rectangle / 5x8
digit / 3x5 letter fallback — the current default) or use a
finished PBR/PNG sprite pack when one is on disk.

This document specifies:

1. The seven widget slots the gate tracks.
2. The manifest JSON schema operators can populate at
   `~/.firestaff/assets/theron/hud/hud_widget_manifest.json`.
3. The gate state machine: `NOT_PROBED` / `NO_MANIFEST` /
   `PLACEHOLDER` / `PARTIAL` / `COMPLETE`.
4. How M12 launcher status and the Phase 7 verification suite
   read the gate.

This module does NOT claim finished PBR widget art is shipped.
It only catalogues slots, classifies the manifest, and exposes
a promotion knob (set `generator != "placeholder"` + resolve a
real `source_file` on disk → gate moves toward `COMPLETE`).

---

## 1. Widget Slots

Slots mirror the Theron V2 HUD overlay surface defined in
`src/theron/theron_v2_hud_overlay_pc34.c`:

| Slot                  | Runtime location                  | Phase 3 group       | Category        |
|-----------------------|-----------------------------------|---------------------|-----------------|
| `compass_rose`        | Top-bar, top-left (16,12)         | Phase 3 primary     | `hud_widgets`   |
| `quest_items`         | Top-bar, "Qx/Qy" counter (64,4)   | Phase 3 primary     | `hud_widgets`   |
| `dungeon_progress`    | Top-bar, "Dn/7" indicator (160,4) | Phase 3 primary     | `hud_widgets`   |
| `relic_counter`       | Top-bar, "Rr/7" indicator (220,4) | Phase 3 primary     | `hud_widgets`   |
| `rune_indicator`      | Top-bar, 4-slot spell-rune        | Phase 3 primary     | `hud_widgets`   |
| `champion_bars`       | Bottom panel, 4-champion HP/Stam  | chrome supporting   | `hud_chrome`    |
| `action_strip`        | Bottom strip, ATK/CST/USE/DRP/MOV | chrome supporting   | `hud_chrome`    |

`hud_widgets` slots are Phase 3 additions not present in the V1
UI chrome (`src/theron/theron_v1_ui_chrome.c`).
`hud_chrome` slots are slots the V1 chrome already draws;
classification parity keeps the gap-list honest when finished
sprites replace the procedural fallback.

The order is stable; ordinals double as table indices. Adding a
slot requires a corresponding row in the slot table in
`src/theron/theron_v2_hud_widget_assets_pc34.c` and an update to
`THERON_V2_HUD_WIDGET_COUNT`.

---

## 2. Manifest Schema

Operator-installed path:
```
~/.firestaff/assets/theron/hud/hud_widget_manifest.json
```

The path is resolved by walking two parents up from the Theron
data dir (`~/.firestaff/data/theron`) and appending
`assets/theron/hud/hud_widget_manifest.json`. Mirrors the
sibling `theron_v22_modern_assets_pc34` resolver.

```json
{
  "manifestVersion": "1.0.0",
  "packId": "theron-hud-widget-pack-1",
  "hud_widgets": [
    {
      "id": "compass_rose",
      "generator": "pbr_hero",
      "source_file": "compass_rose.png",
      "width": 32,
      "height": 32
    },
    {
      "id": "quest_items",
      "generator": "pbr_hero",
      "source_file": "quest_items.png",
      "width": 48,
      "height": 16
    },
    {
      "id": "dungeon_progress",
      "generator": "pbr_hero",
      "source_file": "dungeon_progress.png",
      "width": 48,
      "height": 16
    },
    {
      "id": "relic_counter",
      "generator": "pbr_hero",
      "source_file": "relic_counter.png",
      "width": 48,
      "height": 16
    },
    {
      "id": "rune_indicator",
      "generator": "pbr_hero",
      "source_file": "rune_indicator.png",
      "width": 40,
      "height": 16
    },
    {
      "id": "champion_bars",
      "generator": "pbr_hero",
      "source_file": "champion_bars.png",
      "width": 256,
      "height": 24
    },
    {
      "id": "action_strip",
      "generator": "pbr_hero",
      "source_file": "action_strip.png",
      "width": 200,
      "height": 28
    }
  ]
}
```

### Required fields per slot

| Field         | Type   | Notes                                    |
|---------------|--------|------------------------------------------|
| `id`          | string | must match one of the seven slot ids     |
| `generator`   | string | `"placeholder"` = procedural fallback; anything else (e.g. `"pbr_hero"`, `"ai_upscale"`) is treated as a real-asset declaration |
| `source_file` | string | relative filename resolved against `<manifest-dir>/<category>/` |
| `width`       | int > 0| sprite width in pixels                   |
| `height`      | int > 0| sprite height in pixels                  |

### File resolution

A `source_file` is resolved at runtime by walking from the
manifest's parent directory into the slot's category:

```
~/.firestaff/assets/theron/hud/hud_widgets/<source_file>      (Phase 3 primary)
~/.firestaff/assets/theron/hud/hud_chrome/<source_file>       (chrome supporting)
```

If the resolved path does not exist on disk, the slot classifies
as `PARTIAL` (the manifest declared real metadata but the file is
absent). Operators can promote by dropping the file on disk.

---

## 3. Gate State Machine

```
NOT_PROBED          (initial; no scan yet)
    │
    ▼
NO_MANIFEST         (path unset, file missing, or unreadable)
    │                 every slot → MISSING
    │                 installed = 0
    │
    ▼
PLACEHOLDER         (manifest valid + every declared slot has
    │                 generator == "placeholder")
    │                 installed = 0
    │
    ▼
PARTIAL             (some slots REAL, some PLACEHOLDER or MISSING)
    │                 installed = 1
    │
    ▼
COMPLETE            (all seven slots declared + generator !=
                      "placeholder" + source_file resolves)
                      installed = 1
```

Public API in `include/theron_v2_hud_widget_assets_pc34.h`:

- `theron_v2_hud_widget_assets_set_manifest_path(const char* dataDir)`
- `theron_v2_hud_widget_assets_gate(void)` — returns the gate state
- `theron_v2_hud_widget_assets_classify_slot(slot)` — per-slot
- `theron_v2_hud_widget_assets_uses_placeholder(slot)` — runtime
  helper, returns 1 unless slot is `REAL`
- `theron_v2_hud_widget_assets_get_installed()` — mirror flag

---

## 4. M12 / Phase 7 Integration

- M12 launcher status can read `theron_v2_hud_widget_assets_get_installed()`
  to surface "Phase 3 HUD widget pack" as INSTALLED / PLACEHOLDER
  / NOT INSTALLED in the asset status card.
- The Phase 7 verification suite reads the gate state per
  scenario and asserts: `NO_MANIFEST` for default CI runs,
  `PLACEHOLDER` after a procedural manifest is dropped, and
  `COMPLETE` only when an operator-installed pack with
  `generator != "placeholder"` and disk-resolvable
  `source_file`s is present.

---

## 5. Source-Lock Anchors

| Anchor                                 | Why                                                |
|----------------------------------------|----------------------------------------------------|
| `THQUEST.ASM T520`                     | party placement / start position                   |
| `THQUEST.ASM T560`                     | dungeon loading (header parsing, dungeon_seed)     |
| `THQUEST.ASM T600`                     | UI overlay zones (top-bar / right / bottom)        |
| `THQUEST.ASM T700`                     | timers / world tick                                |
| `THQUEST.ASM T800`                     | champion persistence + inventory reset             |
| `THQUEST.ASM T900`                     | object database / rune magic                       |
| `HuC6260 / HuC6270` datasheet          | PC Engine VDC + VCE (256×224 indexed framebuffer)  |
| ReDMCSB `PANEL.C F0354`                | champion status-box drawing (sibling pattern)      |
| ReDMCSB `DUNGEON.C F0260`              | stat-bar refresh timing (sibling pattern)          |
| dmweb Theron overview                  | 7 dungeons + 7 relic goals + rune magic            |
| `docs/source-lock/tqr_v1_phase2_data_formats_H2339.md` | Theron V1 data formats                |
| `src/theron/theron_v2_hud_overlay_pc34.c` | Procedural fallback that this gate shadows     |
| `include/theron_v22_modern_assets_pc34.h` | Sibling V2.2 manifest pattern                    |
| `include/dm2_v2_hud_widget_assets.h`   | Original Phase 3 placeholder-vs-real gate pattern  |
| `docs/FIRESTAFF_GAP_LIST.md`           | Theron V2 Phase 3 row this gate closes             |

---

## 6. Honest Boundary

This module tracks **asset classification**, not finished art.
Promotion to `COMPLETE` requires:

1. operator writes the manifest at the documented path,
2. every declared slot has `generator != "placeholder"`,
3. every declared `source_file` resolves on disk against the
   slot's category directory,
4. sibling gap-list row is updated to reflect the new state.

No M11 wire-up of `theron_v2_hud_render()` is required for this
gate to be useful — the gate can run headless today and only
becomes runtime-visible once the wire-up lands. No real-art
visual verification is claimed by this initial seed.
