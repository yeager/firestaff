# Pass 41 — Champion status-box stride drop (V1 original-faithful)

Last updated: 2026-04-24
Scope: **DM1 / PC 3.4 / English / V1 original-faithful mode** — champion
status-box horizontal stride.  Closes out `V1_BLOCKERS.md` §5
("Champion status-box stride +8 px per slot vs DEFS.H C69") by
binding the V1 stride to the source anchor `C69_CHAMPION_STATUS_BOX_SPACING`.

Pass 41 is a bounded visual-parity pass.  It makes **no change to V2
vertical-slice mode** (which keeps its pre-baked 302×28 four-slot
sprite aligned to the legacy 77/71 geometry) and touches **no M10
code paths**.

---

## 1. What pass 41 delivers

### 1.1 Source-anchored V1 status-box constants in `m11_game_view.c`

Added to the existing `M11_PARTY_*` enum (alongside the unchanged
legacy values), with in-line source citations:

```c
/* DM1 PC 3.4 source-anchored champion status-box geometry
 * for V1 original-faithful mode.  STEP == DEFS.H:2157
 * C69_CHAMPION_STATUS_BOX_SPACING (69).  W == graphic
 * C007_GRAPHIC_STATUS_BOX frame width (67, verified by the
 * M11 asset-loader probe invariant INV_GV_205 which asserts
 * graphic 7 is 67x29).
 * Ref: V1_BLOCKERS.md §5 / PARITY_MATRIX_DM1_V1.md §1. */
M11_V1_PARTY_SLOT_W    = 67,
M11_V1_PARTY_SLOT_STEP = 69,
```

The legacy `M11_PARTY_SLOT_STEP = 77` and `M11_PARTY_SLOT_W = 71`
enum members are **left unchanged** so V2 vertical-slice mode keeps
binary compatibility with its pre-baked four-slot HUD sprite
(`m11_v2_party_hud_four_slot_base`, 302×28) and active overlay
(`m11_v2_party_hud_four_slot_active_overlay`, 71×28).  These sprites
were authored against the 77 stride and would misalign if the enum
value changed.

### 1.2 Mode-aware helpers

Two static helpers added in `m11_game_view.c`, adjacent to the
existing `m11_v2_vertical_slice_enabled()`:

```c
static int m11_party_slot_step(void) {
    return m11_v2_vertical_slice_enabled()
        ? (int)M11_PARTY_SLOT_STEP        /* 77 */
        : (int)M11_V1_PARTY_SLOT_STEP;    /* 69 */
}

static int m11_party_slot_w(void) {
    return m11_v2_vertical_slice_enabled()
        ? (int)M11_PARTY_SLOT_W           /* 71 */
        : (int)M11_V1_PARTY_SLOT_W;       /* 67 */
}
```

V1 is the default mode; V2 is opt-in via the `FIRESTAFF_V2_VERTICAL_SLICE`
environment variable (pre-existing).

### 1.3 Call-site audit (all call sites re-bound to helpers)

Two call sites previously read the enum literals directly; both have
been re-pointed through the helpers:

| Site | File | Before | After |
|---|---|---|---|
| Hit-test in `M11_GameView_HandlePointer` | `m11_game_view.c` §4320 | `M11_PARTY_PANEL_X + slot * M11_PARTY_SLOT_STEP` + `M11_PARTY_SLOT_W` | cached `slotStep`/`slotW` from helpers |
| Draw loop in `m11_draw_party_panel` | `m11_game_view.c` §11125 | same literal references | cached `slotStep`/`slotW` from helpers |

The procedural fallback (`m11_fill_rect` + `m11_draw_rect`) and the
V1 active-champion double yellow highlight border now derive their
widths from `slotW` (67 in V1, 71 in V2), keeping the highlight
strictly inside the 67-wide status-box frame in V1:

| Element | V1 (pass 41) | V2 (unchanged) |
|---|---|---|
| Outer highlight width | `slotW - 2` = 65 px | `slotW - 2` = 69 px |
| Inner highlight width | `slotW - 4` = 63 px | `slotW - 4` = 67 px |

### 1.4 Diagnostic probe `firestaff_m11_pass41_status_box_stride_probe.c`

24 invariants, all green (`# summary: 24/24 invariants passed`):

- **INV_P41_01..03** — DEFS.H source anchors (C69 == 69, C007 ==
  67×29; literal presence of `C69_CHAMPION_STATUS_BOX_SPACING` in
  `dm7z-extract/Toolchains/Common/Source/DEFS.H`).
- **INV_P41_04..07** — enum wiring in `m11_game_view.c`
  (`M11_V1_PARTY_SLOT_STEP = 69`, `M11_V1_PARTY_SLOT_W = 67`,
  legacy `M11_PARTY_SLOT_STEP = 77` + `M11_PARTY_SLOT_W = 71`).
- **INV_P41_08..11** — helper signatures and V1/V2 branch wiring
  (`m11_party_slot_step()`, `m11_party_slot_w()`).
- **INV_P41_12..13** — V1 slot rectangles at
  `(12, 81, 150, 219)` × 67 are strictly non-overlapping
  (2-px gap between successive frames, matching `69 − 67`).
- **INV_P41_14** — V1 party-panel right edge = 286 ≤ 320
  framebuffer width.
- **INV_P41_15..17** — pass-34 drift numbers preserved:
  stride delta 8 px/slot, width delta 4 px/slot, total
  party-panel width shrinks 302 → 274 px (28 px saved).
- **INV_P41_18..19** — V1 highlight border insets
  (slotW − 2 = 65, slotW − 4 = 63) stay inside the 67-wide frame.
- **INV_P41_20..21** — both V1 code paths (hit test + draw)
  actually go through the helpers; no remaining direct read of
  `M11_PARTY_SLOT_STEP` in the V1 iteration.

Probe runs standalone via:

```
./run_firestaff_m11_pass41_status_box_stride_probe.sh
```

Artifact: `verification-m11/pass41-status-box-stride/pass41_status_box_stride_probe.log`.

### 1.5 Empirical screenshot evidence

Because `.pgm` files are gitignored repo-wide (see `.gitignore:88`),
the pixel-level evidence is committed as a JSON stats summary at
`parity-evidence/overlays/pass41/pass41_party_hud_statusbox_frame_stats.json`.
The source PGMs are reproducible in-tree:

| Source PGM (untracked) | Reproduction |
|---|---|
| `party_hud_statusbox_pre_pass41_stride77.pgm` | checkout commit 09caa98 (pass 40), run `./run_firestaff_m11_game_view_probe.sh`, copy `verification-m11/game-view/party_hud_statusbox_gfx.pgm` |
| `party_hud_statusbox_v1_stride69.pgm`          | on main after pass 41, run `./run_firestaff_m11_game_view_probe.sh`, copy `verification-m11/game-view/party_hud_statusbox_gfx.pgm` |

The JSON stats file records the nonzero-x ranges on scan row
**y = 163** (the status-box top border) for both builds.  After pass
41 the frame-border columns at y=163 are exactly:

| Slot | Frame x-range | Gap to next |
|---|---|---|
| slot 0 | 12..78  | 2 px (79, 80 black) |
| slot 1 | 81..147 | 2 px (148, 149 black) |
| slot 2 | 150..216 | 2 px (217, 218 black) |
| slot 3 | 219..285 | — |

For comparison, the **pre-pass-41** render (`parity-evidence/
overlays/pass41/party_hud_statusbox_pre_pass41_stride77.pgm`, same
probe, same build minus the pass-41 edits) has the frames at
`12..78, 89..155, 166..232, 243..309` — i.e. stride 77, which is
what pass 34 measured.

This is a real pixel-level landing of the `C69` anchor: the stride
delta is exactly +8 px/slot between the two renders, the slot frame
width changes from 71 → 67 px, and the rightmost box now ends at
x=285 instead of x=309, saving 28 px of horizontal chrome.

---

## 2. Visual / ownership gain

- Firestaff V1 now renders the four champion status boxes with the
  DEFS.H-anchored spacing `C69_CHAMPION_STATUS_BOX_SPACING` (69 px)
  instead of the previous invented 77 px stride.
- Slot width binds to the source graphic `C007_GRAPHIC_STATUS_BOX`
  (67 px wide, verified by M11 asset probe `INV_GV_205`), so
  hit-test rectangles no longer overspill the actual frame.
- Previous +8 px per-slot drift (pass-34 measurement) is resolved.
- 28 px of invented horizontal HUD footprint is reclaimed across
  the party panel (302 px → 274 px total width); that 28 px is
  exactly what pass 42's chrome reroute will eventually absorb
  into source-faithful side-column placement.
- V2 vertical-slice mode is untouched: it still reads 77/71 via
  the helpers and its pre-baked 302×28 sprite remains pixel-aligned.

---

## 3. Exact remaining gap before pass 42

Pass 41 is stride-only.  The following side-panel drifts remain and
are **explicitly out of pass-41 scope**:

- **Party-panel origin `(12, 160)` vs ZONES.H-anchored placement** —
  still `BLOCKED_ON_REFERENCE` (pass 34 §3).  Cannot land until
  ZONES.H is recovered (pass 47b).
- **Utility strip / control strip / prompt strip / status lozenge /
  inspect readout** — Firestaff-invented chrome surfaces (blocker
  §6).  Deferred to pass 42.
- **HP/stamina/mana as numeric strings vs CHAMDRAW.C bar graphs
  (C187..C190)** — blocker §7.  Deferred to pass 43.
- **Viewport rectangle** — still at `(12, 24, 196, 118)` per pass
  40's locked `KNOWN_DIFF`.  Depends on pass 42 reroute of invented
  chrome, which in turn is why pass 40 deliberately did not swap
  the runtime viewport even though the DM1 anchor is now encoded
  as `M11_DM1_VIEWPORT_*`.

None of these are regressed or paused by pass 41.

---

## 4. Verification baseline after pass 41 (all green)

| Gate | Result |
|---|---|
| `run_firestaff_m11_phase_a_probe.sh`                 | **18/18 invariants passed** |
| `run_firestaff_m11_game_view_probe.sh`               | **361/361 invariants passed** (full game-view slice; party-HUD screenshot written) |
| `run_firestaff_m11_launcher_smoke.sh`                | **PASS** |
| `run_firestaff_m10_verify.sh GRAPHICS.DAT`           | **20/20 phases PASS** (M10 semantically untouched) |
| `run_firestaff_m11_verify.sh`                        | **Phase A PASS, game-view PASS, summary written** |
| `run_firestaff_m11_pass41_status_box_stride_probe.sh`| **24/24 invariants passed** (new) |

Previously-green pass probes (pass-30 / pass-37 / pass-38 / pass-39 /
pass-40) stay green; pass 41 does not touch their sources.

---

## 5. What pass 41 does NOT do (honesty trail)

Pass 41 does **not**:

- change any M10 code path, any tick-orchestrator behavior, or any
  sensor / movement / door / projectile / champion-state semantics;
- touch the viewport rectangle (pass 40 locked its drift; pass 42
  is responsible for the actual bind);
- reroute or hide any Firestaff-invented HUD surface (pass 42);
- change the V2 vertical-slice pre-baked sprites or their offsets;
- replace the numeric HP/stamina/mana strings with bar graphs
  (pass 43);
- affect typography / font rendering (pass 45);
- touch the palette, audio, or save/load logic.

What pass 41 **does** is land the DM1 DEFS.H-anchored 69-px stride
(and the matching 67-px frame width) for V1 original-faithful mode,
leaving V2 mode untouched, with probe-verified numeric invariants
and a reproducible screenshot diff against the pre-pass-41 render.

**This pass alone does not complete DM1/V1 parity.**  It resolves
`V1_BLOCKERS.md` §5 and is an independent precondition for pass 42's
side-column layout work.
