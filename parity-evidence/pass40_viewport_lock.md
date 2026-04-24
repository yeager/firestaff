# Pass 40 — Viewport dimensional drift honesty lock

Last updated: 2026-04-24
Scope: **DM1 / PC 3.4 / English / V1 original-faithful mode** — viewport
rectangle only.  Closes out V1_BLOCKERS.md §4 ("Viewport region
−28×−18 px vs DM1 original") by locking the drift honestly with a
source-anchored anchor plus a reproducible structural-overlap proof
that a simple coordinate swap cannot land in isolation at this pass.

Pass 40 makes **no runtime visual change** and takes no position on
"is Firestaff right".  It converts the pass-33 measurement into a
permanent, source-anchored artifact so pass 42+ can pick it up.

---

## 1. What pass 40 delivers

### 1.1 Source-anchored DM1 viewport constants in `m11_game_view.c`

Added alongside the existing `M11_VIEWPORT_*` enum:

```c
enum {
    M11_DM1_VIEWPORT_X = 0,    /* COORD.C G2067_i_ViewportScreenX        */
    M11_DM1_VIEWPORT_Y = 33,   /* COORD.C G2068_i_ViewportScreenY        */
    M11_DM1_VIEWPORT_W = 224,  /* DEFS.H C112_BYTE_WIDTH_VIEWPORT * 2    */
    M11_DM1_VIEWPORT_H = 136   /* DEFS.H C136_HEIGHT_VIEWPORT            */
};
```

The existing runtime viewport enum `M11_VIEWPORT_* = (12, 24, 196,
118)` is **left unchanged**.  Pass 40 does not touch the renderer,
the tick orchestrator, any V1 runtime behavior, any probe other than
the new pass-40 probe, or any M10 semantics.

Why not replace the runtime values right now?  See §3 — every
Firestaff-invented HUD surface overlaps the DM1 rectangle by a
measurable amount, and the reroute of those surfaces to the DM1
side-column is the explicit scope of pass 42 (`V1_BLOCKERS.md` §6).
The new constants exist so pass 42 can bind to them without having
to re-derive the anchor.

### 1.2 Diagnostic probe `firestaff_m11_pass40_viewport_lock_probe.c`

22 invariants, all green (`# summary: 22/22 invariants passed`):

- **INV_P40_01..04** — source anchors equal ReDMCSB DEFS.H / COORD.C.
- **INV_P40_05..06** — Firestaff runtime viewport equals the pass-33
  measurement (12, 24, 196, 118).
- **INV_P40_07..09** — numeric drift equals the recorded delta:
  width `−28 px`, height `−18 px`, pixel-area `−7 336 px²` (−24.1 %).
- **INV_P40_10..12** — DM1 rectangle overlaps every Firestaff chrome
  surface except party slot 3; total overlap area **2 914 px²**.
- **INV_P40_13** — DM1 rectangle does **not** overlap the DM1-source
  side-panel surfaces (action area 87×45 at x=224, icon-cell strip
  at x=233), i.e. the DM1 viewport + DM1 side-panel fit together
  correctly on the 320×200 framebuffer.
- **INV_P40_14..15** — DM1 rectangle fits inside the 320×200 VGA
  framebuffer (right edge 224 ≤ 320; bottom edge 169 ≤ 200).
- **INV_P40_SRC** — the new `M11_DM1_VIEWPORT_*` enum is literally
  present in `m11_game_view.c` with the expected values.  If a later
  edit removes or mutates the anchor, this invariant fires.

Probe runs standalone via:

```
./run_firestaff_m11_pass40_viewport_lock_probe.sh
```

Artifact: `verification-m11/pass40-viewport-lock/pass40_viewport_lock_probe.log`.

### 1.3 Pass-40-specific overlay diffs

Reproducible using the pass-47 tooling with the two region rectangles
under question:

| Region | File | Differing pixels |
|---|---|---|
| DM1 anchor (0, 33, 224, 136) | `parity-evidence/overlays/pass40/pass40_viewport_dm1_anchor.stats.json` | 25 990 / 30 464 (85.31 %) |
| Firestaff current (12, 24, 196, 118) | `parity-evidence/overlays/pass40/pass40_viewport_firestaff_current.stats.json` | 21 886 / 23 128 (94.63 %) |

Both deltas remain large because the `reference-artifacts/redmcsb_reference_320x200.png`
reference canvas carries no dungeon content (V1_BLOCKERS.md §11.1 —
still blocked on DOSBox keystroke automation).  Pass 40 records these
deltas as baselines; they are not used to claim parity either way.

---

## 2. Source anchors (exact citation)

From the local ReDMCSB dump at `../redmcsb-output/I34E_I34M/`:

```
DEFS.H:1997     #define C112_BYTE_WIDTH_VIEWPORT 112
DEFS.H:2003     #define C136_HEIGHT_VIEWPORT     136
COORD.C:81      int16_t G2067_i_ViewportScreenX = 0;
COORD.C:82      int16_t G2068_i_ViewportScreenY = 33;
```

Derived:

- Viewport pixel width   = `C112_BYTE_WIDTH_VIEWPORT * 2` = **224 px**.
  (DM1 VGA Mode 0x0D is 4-bpp planar, so 112 bytes × 2 pixels/byte.
  Confirmed by the dimensions of graphic `C000_DERIVED_BITMAP_VIEWPORT`
  which is 224×136 — see `reference-artifacts/anchors/0000_viewport_full_frame.png`.)
- Viewport pixel height  = `C136_HEIGHT_VIEWPORT`             = **136 px**.
- Viewport screen origin = `(G2067, G2068)`                   = **(0, 33)**.

These four values are the definitive DM1 PC 3.4 English viewport
rectangle on the 320×200 framebuffer.  Pass 40 encodes them in
`m11_game_view.c`.

---

## 3. Structural overlap — why a naive coordinate swap cannot land at pass 40

If the runtime viewport were simply retargeted to `(0, 33, 224, 136)`
without any other change, the DM1 rectangle would overlap the
following Firestaff-invented HUD surfaces (all values from
`m11_game_view.c` near the `M11_*` enum block, verified by
`firestaff_m11_pass40_viewport_lock_probe.c`):

| Firestaff surface              | Rect (x, y, w, h)    | Overlap vs DM1 viewport      |
|---|---|---|
| utility panel (header)         | (218, 28, 86, 42)    | 6 × 37 px  = 222 px² at (218, 33) |
| utility button row             | (218, 56, 86, 10)    | 6 × 10 px  = 60 px² at (218, 56)  |
| party slot 0                   | (12, 160, 71, 28)    | 71 × 9 px  = 639 px² at (12, 160) |
| party slot 1 (step = 77)       | (89, 160, 71, 28)    | 71 × 9 px  = 639 px² at (89, 160) |
| party slot 2 (step = 77)       | (166, 160, 71, 28)   | 58 × 9 px  = 522 px² at (166,160) |
| party slot 3 (step = 77)       | (243, 160, 71, 28)   | **none** (fully outside DM1 rect) |
| control strip                  | (14, 165, 88, 14)    | 88 × 4 px  = 352 px² at (14, 165) |
| prompt strip                   | (104, 165, 202, 14)  | 120 × 4 px = 480 px² at (104,165) |

Total overlap area with Firestaff-invented chrome: **2 914 px²**,
spread over 7 surfaces (every Firestaff HUD surface except party slot 3).

Crucially, the DM1 viewport rectangle does **not** overlap the DM1
side-panel surfaces that V1 mode already uses source-faithfully
(action area 87×45 at x=224, icon-cell strip at y=86..120 at x=233):
both of those start at x ≥ 224 = `DM1 right edge`.  That is the
source-intended composition and is preserved.

So the dimensional correction has two logical halves:

1. **Swap viewport constants** to the DM1-anchored values.  By
   itself, this is a one-line change, but it destroys 2 914 px²
   worth of Firestaff-invented HUD output.
2. **Reroute the invented HUD** (utility panel + utility button row
   + party slots + control/prompt strips) out of that rectangle.
   This is the exact scope of `V1_BLOCKERS.md` §6 (pass 42).

Pass 40 cannot, by its own scope rules, do half (2).  Landing half
(1) alone would introduce a regression (invisible/clipped invented
HUD where users expect to click), not a parity gain.  Therefore
pass 40 instead records the source anchor, measures the required
overlap fix as a number pass 42 can plan against, and leaves the
runtime viewport unchanged.

---

## 4. Dependency chain for the full dimensional correction

| Step | Pass | Blocker entry | Scope |
|---|---|---|---|
| Anchor DM1 viewport constants in source | 40 (this pass) | §4 | documentation + probe lock |
| Bar-graph HP/stamina/mana                | 43 | §7 | replaces invented numeric slot strip |
| Bind the runtime viewport to M11_DM1_VIEWPORT_* | part of pass 42 | §6 | requires the invented-chrome reroute below |
| Reroute Firestaff-invented HUD (utility / party / strips) | 42 | §6 | chrome-mode switch; surfaces move into message log + DM1 side-column |
| Champion status-box stride drop from +8 px to C69 | 41 | §5 | independent but adjacent; should land before pass 42 rewires the side-column |
| ZONES.H parse (`PANEL.C` + `COORD.C` layout records) | 47b | §11 | required for source-faithful placement of the rerouted side-column surfaces |

Until pass 42 lands (and ideally pass 41 + pass 47b with it), the
Firestaff runtime viewport must remain at `(12, 24, 196, 118)` to
keep the invented HUD visible and clickable.  This is why
`PARITY_MATRIX_DM1_V1.md` §1 row "Viewport region bounds" must stay
`KNOWN_DIFF` after pass 40; pass 40 only updates the rationale column.

---

## 5. Proposed status update for `PARITY_MATRIX_DM1_V1.md` §1

| Row | Before pass 40 | After pass 40 |
|---|---|---|
| Viewport region bounds | `KNOWN_DIFF` — drift measured in pass 33 | `KNOWN_DIFF` **locked** — DM1 anchor `(0, 33, 224, 136)` now encoded as `M11_DM1_VIEWPORT_*`; retirement blocked on pass 42 chrome reroute per `parity-evidence/pass40_viewport_lock.md` §4 |

No other row changes in §1.

---

## 6. Verification baseline after pass 40 (all green)

| Gate | Result |
|---|---|
| `run_firestaff_m11_phase_a_probe.sh`              | **18/18 invariants passed** |
| `run_firestaff_m11_game_view_probe.sh`            | **361/361 invariants passed** (full game-view slice) |
| `run_firestaff_m11_launcher_smoke.sh`             | **PASS** |
| `run_firestaff_m10_verify.sh GRAPHICS.DAT`        | **20/20 phases PASS** (M10 semantically untouched) |
| `run_firestaff_m11_verify.sh`                     | **Phase A PASS, game-view PASS, summary written** |
| `run_firestaff_m11_pass40_viewport_lock_probe.sh` | **22/22 invariants passed** (new) |

Previously-green pass probes (pass-30 / pass-37 / pass-38 / pass-39)
stay green; pass 40 does not touch their sources.

---

## 7. What pass 40 does NOT do (honesty trail)

Pass 40 does **not**:

- change the runtime viewport rectangle;
- change any HUD layout or hit-testing geometry;
- touch M10 semantics, any V1 runtime behavior outside the new enum
  + the new standalone probe, or any rendering path;
- reroute the invented chrome (pass 42);
- fix the portrait stride (pass 41);
- recover ZONES.H (pass 47b);
- attempt viewport-content pixel overlay against DM1 dungeon frames
  (still blocked on DOSBox keystroke automation, V1_BLOCKERS.md §11.1);
- promote the §1 matrix row to `MATCHED`.

What pass 40 **does** is record the source anchor as a real enum in
the runtime translation unit, lock the drift numerically via a probe,
quantify the exact overlap that pass 42 must clear, and update the
blocker ledger to reflect the new "locked" status.

**This pass alone does not complete DM1/V1 parity.**  It converts
V1_BLOCKERS.md §4 from "measured drift waiting for a plan" to
"locked drift with a source anchor and a quantified dependency on
pass 42", which is the deliverable the pass-40 slot in this program
was scoped to produce.
