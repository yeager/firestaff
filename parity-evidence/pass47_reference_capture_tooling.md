# Pass 47 — ReDMCSB reference capture / rasterisation tooling

Last updated: 2026-04-24
Scope: **DM1 / PC 3.4 / English / V1 original-faithful mode** — tooling
and artifact path for true pixel overlays of viewport + side-panel
components.  Unblocks `V1_BLOCKERS.md` entry 11 as far as bounded
in-tree work can take it.

This is a tooling / capture / reference pass.  It lands **no new V1
runtime behavior**, touches no M10 semantics, and does not claim
parity for anything.  It delivers the materials future passes
(especially pass-49) need to produce honest, source-backed pixel
overlays.

---

## 1. What pass 47 delivers

### 1.1 In-tree reference compositor (no emulator required)

`tools/redmcsb_reference_compose.py` loads:

- the already-extracted DM1 PC 3.4 GRAPHICS.DAT bitmaps from
  `extracted-graphics-v1/pgm/*.pgm` (manifest-tracked, 577 entries,
  source SHA256 recorded in `extracted-graphics-v1/manifest.json`),
- the recovered VIDEODRV.C-backed 16-colour VGA palette from
  `palette-recovery/recovered_palette.json`
  (`base_palette_icon` == `G8149_ICON` == `LIGHT0`),

and produces:

- `reference-artifacts/redmcsb_reference_320x200.png` — a source-
  anchored 320×200 RGB reference canvas.  Viewport rectangle is
  placed at **(0, 33, 224, 136)** per `COORD.C` globals `G2067`/`G2068`
  and `DEFS.H` `C112_BYTE_WIDTH_VIEWPORT` / `C136_HEIGHT_VIEWPORT`.
- `reference-artifacts/anchors/*.png` — 10 palettised PNGs for the
  GRAPHICS.DAT anchor graphics used in side-panel composition (panel
  empty, champion portrait atlas, spell backdrop, spell lines, action
  area, three slot-box variants, status-box frame, viewport frame 0).
  Every anchor’s pixel dimensions exactly match its DEFS.H constant
  (10/10 match).
- `reference-artifacts/provenance.json` — full provenance including
  input SHA256s, per-anchor SHA256s, region rectangles, and an
  explicit honesty section listing what the composite does **not**
  cover.

Total tracked artifact weight is ~64 KB.

### 1.2 Overlay-diff helper

`tools/redmcsb_overlay_diff.py` takes a Firestaff-captured 320×200
frame and either the 320×200 composite or an individual anchor PNG
and produces:

- a pixel-diff mask PNG (red where different, white where identical),
- a stats JSON with input SHA256s, region rect, differing pixel
  count, percent delta, and per-channel tolerance,
- named regions matching DEFS.H-anchored geometry (`viewport`,
  `side_column`, `message_area`, `top_strip`) plus custom
  `--region-xywh` for anchor-granularity checks.

This is the diff engine pass-49 will call for each Firestaff /
ReDMCSB / DOSBox capture pair.

### 1.3 Deterministic DOSBox capture harness

`scripts/dosbox_dm1_capture.sh` is the documented DOSBox-driven
capture path for regions the reference compositor cannot synthesise
(viewport dungeon content, TEXT.C strings).  It:

- stages DM1 PC 3.4 from the already-tracked archive
  `original-games/Game,Dungeon_Master,DOS,Software.7z` (SHA256 and
  size locked via 7z) into
  `verification-screens/dm1-dosbox-capture/<tree>/`,
- writes a deterministic `dosbox.conf`:
  `machine=vgaonly`, `cpu core=normal`,
  `cycles=fixed 3000`, `scaler=none`, `aspect=false`,
  `captures=<capture_dir>` — i.e. the palette is forced to real VGA
  and timing is jitter-free enough for reproducible frames,
- supports `--multilingual` to swap to the PC 3.4 Multilingual tree
  for the i18n side,
- requires only `7zz` (already installed) and one of
  `dosbox`/`dosbox-staging`/`dosbox-x` on PATH.  If no DOSBox binary
  is present, it prints the exact install hint and stops cleanly
  (never silently downloads anything).

Both the staged tree and the capture directory are gitignored
(.gitignore updated this pass) so repeated runs never leak game
content into tracked commits.

### 1.4 Repo hygiene

`.gitignore` amended to (a) whitelist `reference-artifacts/**/*.png`
and `.ppm` so the source-anchored reference pack can be tracked,
(b) ignore `verification-screens/dm1-dosbox-capture/` so staged game
files and runtime captures never land in commits.

No unrelated runtime behavior touched.  No repo reorg.  Tracked
content remains English.

---

## 2. Exact artifact paths and how to reproduce

| Artifact | Path | How to reproduce |
|---|---|---|
| 320×200 reference canvas | `reference-artifacts/redmcsb_reference_320x200.png` | `python3 tools/redmcsb_reference_compose.py` |
| Per-anchor palettised PNGs (10) | `reference-artifacts/anchors/NNNN_<name>.png` | same script |
| Provenance JSON | `reference-artifacts/provenance.json` | same script |
| Deterministic DOSBox config | `verification-screens/dm1-dosbox-capture/dosbox.conf` | `scripts/dosbox_dm1_capture.sh` |
| Staged DM1 PC 3.4 tree | `verification-screens/dm1-dosbox-capture/DungeonMasterPC34/` | `scripts/dosbox_dm1_capture.sh` (gitignored) |
| Per-region overlay diff | `<caller chosen>.mask.png` + `.stats.json` | `python3 tools/redmcsb_overlay_diff.py --firestaff ... --reference ... --region ... --out ...` |

Reproducibility inputs (hash-locked):

- DM1 PC 3.4 GRAPHICS.DAT source SHA256:
  `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
  (recorded in `extracted-graphics-v1/manifest.json`).
- ReDMCSB source dump root:
  `../redmcsb-output/I34E_I34M/` — `DEFS.H`, `COORD.C` used for
  anchors; not shipped inside this repo.

---

## 3. Anchor table (what is MATCHED by pixel size vs DEFS.H)

From `reference-artifacts/provenance.json`:

| Graphic | Index | Size | DEFS.H / source anchor | Match |
|---|---|---|---|---|
| Viewport full frame | 0 | 224×136 | `C000_DERIVED_BITMAP_VIEWPORT` + `C112_BYTE_WIDTH_VIEWPORT`*2 / `C136_HEIGHT_VIEWPORT` | ✓ |
| Spell area backdrop | 9 | 87×25 | `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND` | ✓ |
| Action area | 10 | 87×45 | `C010_GRAPHIC_MENU_ACTION_AREA` | ✓ |
| Spell area lines | 11 | 14×39 | `C011_GRAPHIC_MENU_SPELL_AREA_LINES` | ✓ |
| Panel empty | 20 | 144×73 | `C020_GRAPHIC_PANEL_EMPTY` | ✓ |
| Champion portraits atlas | 26 | 256×87 | `C026_GRAPHIC_CHAMPION_PORTRAITS` (8-wide × 3-tall of 32×29) | ✓ |
| Slot box normal | 33 | 18×18 | probe `INV_GV_201` | ✓ |
| Slot box wounded | 34 | 18×18 | probe `INV_GV_202` | ✓ |
| Slot box acting hand | 35 | 18×18 | probe `INV_GV_203` | ✓ |
| Status box frame | 7 | 67×29 | probe `INV_GV_205` | ✓ |

Every one of these is now available as a source-faithful PNG that
Firestaff can be pixel-diffed against.  That is the first time
panel-chrome overlays can honestly be run in this tree.

---

## 4. What pass 47 does NOT deliver (honest blocker trail)

### 4.1 Viewport dungeon content (wall / floor / ceiling composition)

`DRAWVIEW.C` and `DUNVIEW.C` exist in the local ReDMCSB dump but
cannot be rasterised in-tree because:

- the dump is source-only, not a runnable program, and
- the viewport composition depends on `MEMORY.C`/`IMAGE*.C` loading
  paths and per-square zone state that require the full engine.

The strongest bounded path forward is the DOSBox harness
(`scripts/dosbox_dm1_capture.sh`) capturing DM1 PC 3.4 DM.EXE frames
for the target squares/poses.

**Remaining blocker:** scripted, deterministic keystroke injection.
DOSBox vanilla does not support command-line keystroke injection.
Two bounded options:

1. DOSBox-X `-fastlaunch` with a `[autoexec]`-driven AUTORUN.BAT that
   issues screenshots at hot-key sleeps.  DOSBox-X supports a
   `KEYB` autoexec macro path with delays (`TYPE`/`SLEEP` combos).
2. A recorded DOSBox key-mapper file plus a replay flag.

Neither is runtime-affecting; both can be bolted on top of
`scripts/dosbox_dm1_capture.sh` in a follow-up tooling pass without
touching V1 runtime code.

### 4.2 ZONES.H-driven layout (side-column panel placement)

`ZONES.H` is not in the local ReDMCSB dump, so the pixel origin of
`C101_ZONE_PANEL`, `C151..C154_ZONE_CHAMPION_*_STATUS_BOX_*`, etc.,
is still unknown at source-anchor precision.  `C020_GRAPHIC_PANEL_EMPTY`
is 144 px wide but the right column (320−224 = 96 px) is narrower, so
the panel is composited through a zone record that we do not yet
have.

The reference compositor deliberately does **not** place the panel
graphic in the 96-pixel right column: that would be a fabricated
placement and violate the honesty rules.  Instead the blocker is
recorded in `reference-artifacts/provenance.json → blocked_on_zones_h`
with a specific suggested follow-up (parse `PANEL.C` + `COORD.C`
layout-record init code once ZONES.H is recovered).

### 4.3 TEXT.C / TEXT2.C runtime text

Font glyph atlas is extractable from GRAPHICS.DAT (there is a font
index visible in the extracted set), but the placement is TEXT.C-
driven and content-dependent.  Reference canvas marks the message
area boundary only.

### 4.4 Animating / per-frame content

Creature, projectile, door-animation frames are covered by existing
GRAPHICS.DAT entries but their timing/ordering is engine-driven.  Not
a pass-47 target.

---

## 5. Does pass 47 unblock pass 49?

**Partially — enough to land the strongest honest pass 49 can land.**

Pass 49 (`V1_BLOCKERS.md` entry 14) is:

> bind at least one high-value M10 probe (movement-champions or
> dungeon-doors-sensors) to a ReDMCSB-produced expected log once
> pass-47 tooling lands.

Status per slice after pass 47:

| Pass-49 slice | Unblocked by pass 47? | Evidence path |
|---|---|---|
| Side-panel component pixel match (portraits, slot boxes, action/spell area, status-box frame, panel-empty) | **YES** | `reference-artifacts/anchors/*.png` + `redmcsb_overlay_diff.py` |
| Side-panel component *placement* (x/y in 320×200) | **NO** — `blocked_on_zones_h` | `reference-artifacts/provenance.json` |
| Viewport bounding rect (0,33,224,136) | **YES** | `tools/redmcsb_overlay_diff.py --region viewport` |
| Viewport dungeon content match | **NO** in-tree — blocked on emulator capture | `scripts/dosbox_dm1_capture.sh` + KeyMapper follow-up |
| TEXT.C string content match | **NO** | follow-up |
| M10 probe vs original log (e.g. movement-champions) | **CONDITIONAL** — probe-state check does not require graphics overlay; could bind to engine state dump once DOSBox debugger or ReDMCSB-source walk of the probed path is attempted | pass-49 design decision |

So: panel-chrome and bounding-rectangle overlays are now fully
unblocked; viewport content and string-content overlays still need the
DOSBox capture phase to actually run (prereq: user-side DOSBox install
and a keystroke recording).

---

## 6. File ledger for this pass

**Added**

- `tools/redmcsb_reference_compose.py`
- `tools/redmcsb_overlay_diff.py`
- `scripts/dosbox_dm1_capture.sh`
- `reference-artifacts/redmcsb_reference_320x200.png`
- `reference-artifacts/anchors/0000_viewport_full_frame.png`
- `reference-artifacts/anchors/0007_status_box_frame.png`
- `reference-artifacts/anchors/0009_spell_area_backdrop.png`
- `reference-artifacts/anchors/0010_action_area.png`
- `reference-artifacts/anchors/0011_spell_area_lines.png`
- `reference-artifacts/anchors/0020_panel_empty.png`
- `reference-artifacts/anchors/0026_champion_portraits.png`
- `reference-artifacts/anchors/0033_slot_box_normal.png`
- `reference-artifacts/anchors/0034_slot_box_wounded.png`
- `reference-artifacts/anchors/0035_slot_box_acting_hand.png`
- `reference-artifacts/provenance.json`
- `parity-evidence/pass47_reference_capture_tooling.md` (this file)

**Modified**

- `.gitignore` — whitelist `reference-artifacts/**/*.png|.ppm`; ignore
  `verification-screens/dm1-dosbox-capture/`.

**Untouched**

- All M10 sources / probes
- All V1 runtime (`m11_*.c`, `main_loop_m11.c`, `memory_tick_orchestrator_*`).
- All matrix / blocker docs except this file addition.

---

## 7. Pass 47 honesty note

This pass explicitly does **not**:

- claim any Firestaff element MATCHED based on the new reference
  canvas — only the *tooling* to honestly test such claims is
  delivered;
- synthesise viewport dungeon content without an engine;
- add V2 work, touch M10, or rework the repo layout;
- bypass the ZONES.H gap by fabricating a side-column placement.

If a later pass wants to demote or promote any `PARITY_MATRIX_DM1_V1.md`
row it must:

1. produce a `*.stats.json` from `redmcsb_overlay_diff.py` whose
   `differing_pixels` count is zero (within tolerance), AND
2. cite `reference-artifacts/provenance.json` input SHA256s.

Anything short of that remains `KNOWN_DIFF` / `BLOCKED_ON_REFERENCE`
per the Pass 36 honesty lock.

**This pass alone does not complete DM1/V1 parity.**
