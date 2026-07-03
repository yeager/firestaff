# DM1 V2.2 Finished-Art Material Gate

**Status:** ✅ COMPLETE — initial seed landed 2026-06-28; PNG IHDR
provenance hardening landed 2026-06-29
**CTest names:**
- `dm1_v22_finished_art_material_gate_pc34` (CI-runnable synthetic
  gate — every branch exercised via synthetic manifest fixtures)
- `firestaff_dm1_v22_finished_art_material_gate_probe` (headless CI
  probe mirroring the CTest scenarios)

**Companion (SKIP-only real-asset) gate:**
`test_dm1_v22_real_asset_material_gate_pc34` — the existing gate that
SKIPs unless an operator has dropped a full hero manifest on disk. The
new gate covers the same manifest surface (the same seven `hero_01` ids
in `wall_shapes` / `floor_shapes` / `creature_shapes` /
`champion_portraits` / `door_shapes` / `field_shapes`) but exercises
the placeholder-vs-real state machine with synthetic manifests so CI can verify the
gate without requiring real PBR art.

**Module files:**
- `include/dm1_v22_finished_art_material_gate_pc34.h`
- `src/dm1v2/dm1_v22_finished_art_material_gate_pc34.c`
- `tests/test_dm1_v22_finished_art_material_gate_pc34.c`
- `probes/firestaff_dm1_v22_finished_art_material_gate_probe.c`

**Sibling gates (same placeholder-vs-real pattern):**
- `include/dm2_v2_hud_widget_assets.h` — DM2 V2 HUD widget gate
  (Phase 3, 7 slots). The original pattern that this gate mirrors.
- `include/theron_v2_hud_widget_assets_pc34.h` — Theron V2 Phase 3
  HUD widget gate (7 slots).
- `include/csbb_v22_modern_assets_pc34.h` (sibling modern-asset
  manifest validation across games).

---

## Scope

The `docs/FIRESTAFF_GAP_LIST.md` row 170 ("Per-mode pixel/material
verification gates", `OPEN-BOUNDED`) tracks DM1 V2.2 finished-art /
real-asset screenshot material verification. This gate is the
CI-runnable distinction between:

- **placeholder / synthetic art** — the honest current runtime
  default. The DM1 V2.2 modern-asset pack ships procedural bitmaps;
  the manifest declares every slot with `generator == "placeholder"`.
- **reviewed finished-art** — the operator-installed state. An
  operator has dropped a `modern_asset_manifest.json` with every
  required slot declaring `generator != "placeholder"` and a
  `source_file` that resolves on disk as a PNG whose IHDR dimensions
  match the manifest.

The sibling SKIP-only gate (`dm1_v22_real_asset_material_gate_pc34`)
runs only when the real-art pack is present and rejects the build
otherwise. This new gate is the data-free counterpart that exercises
the state machine on every CI run.

This document specifies:

1. The seven material slots the gate tracks.
2. The manifest JSON schema operators can populate at
   `~/.firestaff/assets/dm1/modern/modern_asset_manifest.json`.
3. The gate state machine: `NOT_PROBED` / `NO_MANIFEST` /
   `SYNTHETIC_PLACEHOLDER` / `PARTIAL` / `FINISHED_REAL`.
4. The optional runtime screenshot/material receipt state machine.
5. How M12 launcher status and the Phase 7 verification suite read
   the gate.
6. The honest boundary: this gate tracks manifest classification
   only. It does **NOT** claim finished PBR art has been reviewed
   or shipped.

---

## 1. Material Slots

Slots mirror the material surface already covered by the in-place
render probe (`probes/firestaff_dm1_v22_inplace_render_probe.c`) and
the existing SKIP-only sibling gate. The mapping is:

| Slot                  | Runtime location                  | Manifest id                    | Category             | Synthetic asset_id (cache fallback) |
|-----------------------|-----------------------------------|--------------------------------|----------------------|-------------------------------------|
| `WALL_D3_CARVED`      | D1 center cell                    | `wall_d3_carved_hero_01`       | `wall_shapes`        | `wall_d3_carved_01`                 |
| `FLOOR_PLAIN`         | D1 mid cell                       | `floor_plain_hero_01`          | `floor_shapes`       | `floor_plain_01`                    |
| `FLOOR_PIT`           | D1 right cell                     | `floor_pit_hero_01`            | `floor_shapes`       | `floor_pit_01`                      |
| `CREATURE_DEMON`      | Creature fallback                 | `creature_demon_hero_01`       | `creature_shapes`    | `creature_demon_01`                 |
| `CHAMPION_WARRIOR`    | Champion portrait slot 0          | `champion_warrior_hero_01`     | `champion_portraits` | (none — falls back to runtime champion-stat renderer) |
| `DOOR_FRONT`          | Door shape                        | `door_hero_01`                 | `door_shapes`        | (none — drawn by `m11_draw_dm1_door_pc34`) |
| `TELEPORTER_FIELD`    | F0113 teleporter field surface    | `field_teleporter_hero_01`     | `field_shapes`       | `field_teleporter_01`               |

Slot ordinals are stable: `DM1_V22_FAMG_WALL_D3_CARVED == 0`,
`DM1_V22_FAMG_FLOOR_PLAIN == 1`, ..., `DM1_V22_FAMG_DOOR_FRONT == 5`,
`DM1_V22_FAMG_TELEPORTER_FIELD == 6`,
`DM1_V22_FAMG_MATERIAL_COUNT == 7`. The slot table is the canonical
mapping between gate slots and the manifest surface — reordering or
inserting slots must keep the count in sync and update the
`k_slot_table` array in the .c file.

---

## 2. Manifest Schema

Reuses the existing `modern_asset_manifest.json` format defined for
`include/dm1_v2_asset_pipeline_pc34.h`. Each slot entry carries:

```json
{
  "id": "wall_d3_carved_hero_01",
  "generator": "placeholder" | "pbr_hero" | "ai_upscale" | "reviewed",
  "source_file": "wall_d3_carved_hero_01.png",
  "width": 64,
  "height": 64
}
```

- `id` — the canonical slot id. Must match one of the seven ids in the
  slot table.
- `generator` — required. `"placeholder"` is the procedural fallback
  marker; any other value (`"pbr_hero"`, `"ai_upscale"`,
  `"reviewed"`) is a non-placeholder marker.
- `source_file` — required. The on-disk filename relative to
  `<manifest_dir>/<category>/`.
- `width`, `height` — required, must be `> 0` and must match the
  on-disk PNG IHDR dimensions for non-placeholder entries.

Path resolution: the manifest is read from
`~/.firestaff/assets/dm1/modern/modern_asset_manifest.json`. Each
entry's `source_file` is resolved against
`<manifest_dir>/<category>/<source_file>`. If the file exists on
disk, starts with the PNG 8-byte signature, has an `IHDR` chunk, and
the IHDR width/height matches the manifest dimensions, the slot is
`REAL`; otherwise it is `PARTIAL` (real metadata, missing file or bad
PNG provenance).

The gate walks two parents up from the `dataDir` argument to find
`~/.firestaff`, then descends into `assets/dm1/modern/`. This
mirrors `m11_v22_set_manifest_path` in
`src/dm1v2/dm1_v2_modern_assets_pc34.c`.

---

## 3. Gate State Machine

```
NOT_PROBED
   │
   ▼  (first gate() call, path unset)
NO_MANIFEST ─────────────────────────────────────────────┐
   │                                                    │
   ▼  manifest file present, no slot entries declared   │
SYNTHETIC_PLACEHOLDER ◀───────────────────────────────┐ │
   │                                                  │ │
   ▼  declared slots with generator="placeholder"     │ │
SYNTHETIC_PLACEHOLDER ◀───────────────────────────────┤ │
   │                                                  │ │
   ▼  declared slots with generator="placeholder" +   │ │
      some required fields missing                    │ │
SYNTHETIC_PLACEHOLDER ◀───────────────────────────────┤ │
   │                                                  │ │
   ▼  declared slots with non-placeholder generator   │ │
      and source_file missing / bad PNG provenance    │ │
SYNTHETIC_PLACEHOLDER ◀───────────────────────────────┤ │
   │                                                  │ │
   ▼  at least one slot REAL, others not REAL         │ │
PARTIAL                                                │ │
   │                                                  │ │
   ▼  every required slot REAL                        │ │
FINISHED_REAL                                          │ │
   │                                                  │ │
   ▼  operator removes a real PNG / edits manifest    ─┘
      (gate recomputes on next gate() call)
```

| Gate state              | Trigger                                                  | Operator action to move forward |
|-------------------------|----------------------------------------------------------|---------------------------------|
| `NOT_PROBED`            | Gate has never been evaluated                            | Call `dm1_v22_famg_set_manifest_path()` + `dm1_v22_famg_gate()` |
| `NO_MANIFEST`           | Path unset OR file unreadable                            | Drop a valid `modern_asset_manifest.json` under `~/.firestaff/assets/dm1/modern/` |
| `SYNTHETIC_PLACEHOLDER` | Manifest valid, every declared slot uses `placeholder` generator (the honest CI default) | Add a non-placeholder slot to promote to `PARTIAL`; complete all seven for `FINISHED_REAL` |
| `PARTIAL`               | At least one REAL, at least one non-REAL                  | Replace remaining placeholders with disk-resolved real PNGs to move toward `FINISHED_REAL` |
| `FINISHED_REAL`         | Every required slot REAL with non-placeholder generator + on-disk PNG whose IHDR dimensions match the manifest | Promotion requires an explicit sibling gap-list update |

The gate is recomputed on every call. There is no caching; the
manifest file is re-read on each `gate()` call so an operator can
edit the manifest on disk and see the new state on the next gate
query.

---

## 4. Runtime Screenshot/Material Receipt

The material gate can now read one optional receipt entry from the
same `modern_asset_manifest.json`:

```json
{
  "id": "dm1_v22_real_screenshot_material_receipt_01",
  "generator": "synthetic_test",
  "source_file": "synthetic_frame.bmp",
  "width": 320,
  "height": 200,
  "frame_hash": "sha256:...",
  "material_gate": "FINISHED_REAL"
}
```

The receipt source file resolves under
`~/.firestaff/assets/dm1/modern/receipts/<source_file>`.
This keeps proprietary or operator-generated runtime evidence out of
the repository while giving operators a stable place to drop local
proof metadata. The CI tests create temporary receipt files only under
`/tmp/scratch`; they are not final runtime evidence.

Receipt gate states:

| Receipt state | Trigger |
|---------------|---------|
| `NO_RECEIPT` | Manifest missing, unreadable, or no receipt entry is present |
| `SYNTHETIC_PLACEHOLDER` | Receipt entry exists but declares `generator` as `placeholder`, `synthetic`, or `synthetic_test` |
| `PARTIAL` | Receipt metadata is incomplete, the receipt file is missing, `material_gate` is not `FINISHED_REAL`, or the seven-slot material gate is not currently `FINISHED_REAL` |
| `FINISHED_REAL` | The seven-slot material gate is `FINISHED_REAL`, receipt `generator` is non-synthetic, `source_file` resolves under `receipts/`, and `width`/`height`/`frame_hash`/`material_gate` are present |

This intentionally separates two claims:

- `dm1_v22_famg_is_finished_real()` means the seven tracked material
  slots are real operator-installed files.
- `dm1_v22_famg_has_finished_real_receipt()` means there is also an
  operator-reviewed runtime screenshot/material receipt. This is the
  state needed before public docs can talk about a real DM1 V2.2
  screenshot proof instead of a synthetic CI fixture.

---

## 5. M12 / Phase 7 Integration Points

The gate is read by:

- `M12_AssetStatus_Scan` — at startup, after resolving the
  Firestaff data directory. The installed flag mirrors
  `PARTIAL` / `FINISHED_REAL` → `1`, everything else → `0`,
  matching the sibling `m11_v22_modern_assets_available` contract.
- The Phase 7 verification suite — `dm1_v2_per_mode_material_signatures_pc34`
  confirms the cross-mode material hashes; this gate confirms the
  asset provenance / finished-art status.

The convenience helpers:

- `dm1_v22_famg_is_finished_real()` — `1` only when gate is
  `FINISHED_REAL`. Use this as the gate-level "real reviewed
  finished-art pack installed" predicate.
- `dm1_v22_famg_is_synthetic_or_partial()` — `1` when gate is
  `SYNTHETIC_PLACEHOLDER` or `PARTIAL`. Use this as the
  "placeholder/synthetic art" predicate.

---

## 6. Honest Boundary

This gate **does NOT** claim finished PBR art has been reviewed or
shipped. `FINISHED_REAL` is reachable only when:

1. An operator has installed a `modern_asset_manifest.json` at
   `~/.firestaff/assets/dm1/modern/` with all seven required slots
   declaring `generator != "placeholder"`.
2. Each slot's `source_file` resolves on disk under
   `<manifest_dir>/<category>/<source_file>`.
3. Each resolved file starts with the PNG 8-byte signature and has an
   `IHDR` width/height matching the manifest dimensions.
4. Width and height are `> 0`.

Until then, the gate reports `SYNTHETIC_PLACEHOLDER` (the honest
current default matching the procedural bitmaps the V2.2 modern-asset
author generates today) or `NO_MANIFEST` (when the operator has not
yet dropped any manifest).

The runtime does **NOT** silently switch to a "review-pending"
state. `PARTIAL` is a transient state during the operator's
incremental install — it tells the M12 launcher that *some* hero
slots are real while others still fall back to procedural.

The screenshot/material receipt path is also metadata-only. It does
not commit, generate, or promote any proprietary screenshot. A
synthetic receipt is useful for CI, but public README/release evidence
still requires an operator-reviewed receipt file outside the repo and
an explicit docs/status promotion.

---

## 7. V1 / V2 Invariants

- V1 command routes, dungeon state, save/restore, sensor processing
  are NEVER bypassed by this gate.
- The gate's classification is **advisory**. The runtime continues
  to honour `m11_v22_set_installed()` (set by `M12_AssetStatus_Scan`
  via `m11_v22_modern_assets_available`) and the
  `dm1_v2_presentation_mode` selector when deciding whether to
  consume the modern asset pack.
- This gate is read-only with respect to the manifest. It does
  not edit `modern_asset_manifest.json`.

---

## Source Citations

```
ReDMCSB DUNVIEW.C:6697-6816       — DM1 viewport composition order
ReDMCSB DUNGEON.C:2238-2246       — square-type decode feeding m11_v22_shape_for_cell
ReDMCSB PANEL.C F0354             — champion status-box drawing
include/dm1_v2_asset_pipeline_pc34.h  — modern asset manifest path resolution
include/m11_v22_inplace_draw_pc34.h   — cell -> variant -> asset_id
include/m11_v22_shape_cache_pc34.h    — per-frame V22 shape cache
src/dm1v2/dm1_v22_shapes.c           — DM1 V2.2 shape classification
src/dm1v2/dm1_v2_modern_assets_pc34.c — missing-asset placeholder 16x16 magenta
sibling dm2_v2_hud_widget_assets.c    — placeholder-vs-real pattern
docs/FIRESTAFF_GAP_LIST.md B3 row     — V2 per-mode material verification gate
docs/source-lock/theron_v2_phase3_hud_widget_assets_H2340.md — sibling gate doc
```

## Manifest Path

`~/.firestaff/assets/dm1/modern/modern_asset_manifest.json`

## Schema (per slot entry)

`{ id, generator, source_file, width, height }`

`generator == "placeholder"` is the procedural fallback marker.
Any other value is a non-placeholder marker.

`width` and `height` must both be positive integers.

For non-placeholder entries, existence alone is not enough: the file
must begin with the PNG signature `89 50 4E 47 0D 0A 1A 0A`, contain
an `IHDR` chunk at the standard offset, and report the same width and
height as the manifest. This keeps a placeholder text file renamed to
`.png`, or a stale PNG with mismatched dimensions, in `PARTIAL`
instead of promoting the finished-art gate.

## Receipt Schema

`{ id, generator, source_file, width, height, frame_hash, material_gate }`

The receipt id is
`dm1_v22_real_screenshot_material_receipt_01`. `source_file`
resolves under `receipts/`. Synthetic generators (`placeholder`,
`synthetic`, `synthetic_test`) never promote final evidence; a
finished receipt requires `material_gate == "FINISHED_REAL"` and the
seven-slot material gate to be `FINISHED_REAL` at query time.

## Companion (SKIP-only) gate

`test_dm1_v22_real_asset_material_gate_pc34` runs only when the
real-art pack is fully installed. This new gate is the data-free
counterpart that exercises the state machine on every CI run.
