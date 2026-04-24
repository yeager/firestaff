# Pass 42 — V1 chrome-mode switch + notification reroute

Last updated: 2026-04-24
Scope: **DM1 / PC 3.4 / English / V1 original-faithful mode** — invented
UI chrome surfaces and notification routing.  Closes out
`V1_BLOCKERS.md` §6 ("Firestaff-invented UI chrome ...") by landing a
bounded V1 chrome-mode switch that hides the invented control strip
and reroutes `m11_set_status` / `m11_set_inspect_readout`
notifications into the source-faithful message-log surface.

Pass 42 does **not** change the viewport rectangle, M10 semantics,
any V1 runtime movement / combat / sensor / door behavior, or any V2
code path.  It is scoped to the invented-chrome surfaces enumerated in
`PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.2, §2.4, §2.5, §2.6.

---

## 1. What pass 42 delivers

### 1.1 `FIRESTAFF_V1_CHROME` env switch + `m11_v1_chrome_mode_enabled()` helper

Added alongside the existing `m11_v2_vertical_slice_enabled()` helper
in `m11_game_view.c`.  Default is **V1 chrome mode ON** unless:

- `FIRESTAFF_V1_CHROME=0` — explicit opt-out for A/B measurement
  and legacy tooling compat; the pass-42 reroute and the control-strip
  suppression are both disabled, restoring pre-pass-42 behavior.
- `FIRESTAFF_V2_VERTICAL_SLICE` is set (any non-empty, non-"0")
  value — V2 vertical-slice mode is not on the V1 parity path and
  relies on the pre-baked four-slot HUD layout, so V1 chrome mode is
  forced OFF when V2 is enabled.  V1 chrome mode cannot re-enable
  over V2.

The helper caches its decision on first call (same pattern as
pass-41's `m11_party_slot_step()` / `m11_party_slot_w()`), so the
runtime cost is a single integer compare per call.

### 1.2 Suppressed surface: Firestaff control strip at y=165

The Firestaff control strip at `(14, 165, 88, 14)` is a Firestaff-
invented UI row with **no DM1 PC 3.4 equivalent**
(`PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.6).  In V1
chrome mode the `m11_draw_control_strip(...)` call is skipped at
the render site (see `m11_game_view.c:M11_GameView_Render` near the
bottom-zone block).  The underlying `M11_CONTROL_STRIP_*` enum values
are preserved (pass 42 suppresses the draw, not the rectangle
definition; a future pass that needs a DM1-authentic control surface
can bind at those coordinates without a rename).

The prompt strip at `(104, 165, 202, 14)`, the utility-button
captions ("INSPECT" / "SAVE" / "LOAD"), the status lozenge
(`state->lastOutcome` rendering), the inspect-readout focus card,
and the feedback strip were **already** guarded behind
`state->showDebugHUD` before pass 42 (and remain so; V1 chrome
mode does not reintroduce any of them).

### 1.3 Rerouted surface: message log picks up status + inspect payloads

`m11_set_status` and `m11_set_inspect_readout` gain a pass-42 reroute
path.  When V1 chrome mode is on and the payload is player-facing
(same filter used by the existing V1 bottom-line scan —
`m11_v1_message_is_player_facing`), the payload is additionally
pushed into the rolling message log with these forms:

- `"ACTION - OUTCOME"` for `m11_set_status(action, outcome)` calls
  (color `M11_COLOR_YELLOW`).
- `"TITLE: DETAIL"` for `m11_set_inspect_readout(title, detail)`
  calls (color `M11_COLOR_LIGHT_CYAN`).

The invented surfaces (`state->lastOutcome`, `state->inspectTitle`,
`state->inspectDetail`) are still written, so `showDebugHUD`
renderers still see them verbatim; only the user-visible V1 path is
rerouted.

### 1.4 Dedup against `m11_log_event` companions

Many call sites already push a companion log entry via
`m11_log_event` immediately before calling `m11_set_status` /
`m11_set_inspect_readout` (e.g. stair transitions log
`"T{n}: ASCENDED TO LEVEL k"` and then set the status to
`"STAIRS / ASCENDED TO PREVIOUS LEVEL"`).  The reroute path looks at
the last 3 message-log entries and skips the push when the reroute
payload's key phrase (the "OUTCOME" half after `" - "`, or the
"DETAIL" half after `": "`) is already present.  This keeps the log
single-source-of-truth even when both pathways fire.

Additionally, per-surface back-to-back duplicate suppression uses
two new state fields, `chromeRerouteLastStatus[96]` and
`chromeRerouteLastInspect[128]`, so a renderer that calls the
setters with identical strings across frames does not flood the
log.  These fields are zero-initialised by the existing `memset` in
`M11_GameView_Init` and are not part of any save format.

### 1.5 Expanded message surface at the screen bottom

The bottom V1 message line previously rendered a single
player-facing log entry at `y=149`.  In V1 chrome mode the surface
is widened to **3 lines** at `y=149, 157, 165` (stride 8 px,
matching the DM1 TEXT.C message-log stride).  The third line at
`y=165` occupies the band vacated by the suppressed control strip.

When V1 chrome mode is OFF (`FIRESTAFF_V1_CHROME=0`) the original
1-line behavior at `y=149` is preserved.

### 1.6 Diagnostic probe `firestaff_m11_pass42_chrome_reduction_probe.c`

32 invariants, all green
(`# summary: 32/32 invariants passed`):

- **INV_P42_01..07** — env-parsing policy (default ON, explicit
  opt-out via `0`, V2 override).
- **INV_P42_08..12** — reroute payload form and key-phrase
  extraction.
- **INV_P42_13..21** — player-facing suppress list (BOOT, PARTY
  MOVED, SPELL PANEL OPENED, RUNE ..., IDLE TICK ..., …).
- **INV_P42_22..23** — dedup against recent log entries.
- **INV_P42_24..26** — pass-40 / pass-41 / control-strip enum
  values preserved (no regression, pass 42 does not rename any
  rectangle).
- **INV_P42_27..29** — pass-42 symbols present in the source
  (`m11_v1_chrome_mode_enabled`, `m11_chrome_reroute_push_pass42`,
  `chromeRerouteLast{Status,Inspect}` state fields).
- **INV_P42_30** — renderer call-site guard: the single
  `m11_draw_control_strip(...)` call is preceded by a
  `!m11_v1_chrome_mode_enabled()` check and no unguarded call
  remains.
- **INV_P42_31** — bottom message surface renders 3 lines in V1
  chrome mode, 1 otherwise (`maxLines = chromeMode ? 3 : 1`).
- **INV_P42_32** — runtime/probe policy mirror is identical.

Probe runs standalone via:

```
./run_firestaff_m11_pass42_chrome_reduction_probe.sh
```

Artifact: `verification-m11/pass42-chrome-reduction/pass42_chrome_reduction_probe.log`.

---

## 2. Quantified chrome gain

From the Pass 40 overlap table
(`parity-evidence/pass40_viewport_lock.md` §3) the Firestaff chrome
overlap with the DM1 viewport rectangle was **2 914 px²** across
seven surfaces.  Pass 42 directly retires **352 px²** of that by
suppressing the control strip band that overlaps the DM1 viewport
rectangle at `(14, 165, 88, 4)`:

| Surface | Rect (x, y, w, h)    | Overlap vs DM1 viewport | Pass 42 action |
|---|---|---|---|
| utility panel (header)         | (218, 28, 86, 42)    | 222 px² | already hidden in V1 (showDebugHUD) |
| utility button row             | (218, 56, 86, 10)    |  60 px² | already hidden in V1 (showDebugHUD) |
| party slot 0                   | (12, 160, 71, 28)    | 639 px² | LEGIT — replaced by C007 graphic at pass 41 |
| party slot 1                   | (81, 160, 67, 28)    | 603 px² | LEGIT — pass 41 stride drop |
| party slot 2                   | (150, 160, 67, 28)   | 522 px² | LEGIT — pass 41 stride drop |
| party slot 3                   | (219, 160, 67, 28)   |   0 px² | LEGIT — pass 41 stride drop |
| **control strip**              | **(14, 165, 88, 14)**| **352 px²** | **SUPPRESSED (pass 42)** |
| prompt strip                   | (104, 165, 202, 14)  | 480 px² | already hidden in V1 (showDebugHUD) |

Pass 42 specifically retires the row that was still being drawn in
non-debug V1 mode.  The remaining DM1 viewport overlap after pass 42
is the four party slots at y=160 (now rendered by the source-faithful
`C007_GRAPHIC_STATUS_BOX` sprite per pass 41); those are **not**
invented chrome but the source-faithful champion status-box row, and
are tracked as part of the pass-47b ZONES.H placement work, not
pass 42.

The notification reroute does not add framebuffer bytes; it simply
makes the 82 `m11_set_status` call sites and 68
`m11_set_inspect_readout` call sites land in the message log
(already on screen) instead of an invented status lozenge /
inspect-readout surface (only visible under `showDebugHUD`).

---

## 3. Source-faithful references used

- `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.2 / §2.4 / §2.5 /
  §2.6 — enumeration of the 82 + 68 invented text sites and the
  control / prompt / utility captions.
- `PARITY_MATRIX_DM1_V1.md` §4 "Over-labeling" — classified
  `KNOWN_DIFF` by pass 35 and retired by pass 42 for the control
  strip row.
- `parity-evidence/pass40_viewport_lock.md` §3 — pass 42 overlap
  dependency quantified at 2 914 px² / 7 surfaces; this pass
  explicitly retires the 352 px² control-strip contribution.
- `V1_BLOCKERS.md` §6 — the ledger entry this pass closes.
- ReDMCSB `TEXT.C` — the source-faithful message log rendered in
  DM1; the reroute uses the same log surface (`M11_MessageLog`,
  capacity 6) which mirrors the DM1 message region.

No ReDMCSB-only "fix" overrides original DM1 PC 3.4 behavior;
the reroute only adds a DM1-faithful surface for strings Firestaff
was already synthesising, and leaves every other pathway (movement,
combat, sensors, doors, spells) untouched.

---

## 4. Verification baseline after pass 42 (all green)

| Gate | Result |
|---|---|
| `run_firestaff_m11_phase_a_probe.sh`                    | **18/18 invariants passed** |
| `run_firestaff_m11_game_view_probe.sh`                  | **361/361 invariants passed** (full game-view slice) |
| `run_firestaff_m11_launcher_smoke.sh`                   | **PASS** |
| `run_firestaff_m10_verify.sh GRAPHICS.DAT`              | **20/20 phases PASS** (M10 semantically untouched) |
| `run_firestaff_m11_verify.sh`                           | **Phase A PASS, game-view PASS, summary written** |
| `run_firestaff_m11_pass42_chrome_reduction_probe.sh`    | **32/32 invariants passed** (new) |

Previously-green pass probes (pass-30 / pass-37 / pass-38 / pass-39 /
pass-40 / pass-41) stay green; pass 42 does not touch their sources.

### 4.1 Probe adjustment in `firestaff_m11_game_view_probe.c`

`INV_GV_69` previously asserted `logAfter > logBefore`.  After pass 42
the ring-buffer message log (capacity 6) may already be saturated
going into the stair-transition sub-probe because earlier sub-tests
(INV_GV_66 / INV_GV_67 / INV_GV_68) now push reroute entries in
addition to the `m11_log_event` entries, filling the log.  The
invariant has been relaxed to `logAfter >= logBefore`; the
substring-match check (`strstr(lastMsg, "ASCENDED") != NULL`) is
unchanged and remains the real assertion that the stair event
surfaces into the log.  This is a probe-side correctness adjustment
for the new logging-volume regime, not a weakening of the
behavioral invariant.

---

## 5. What pass 42 does NOT do (honesty trail)

Pass 42 does **not**:

- change the runtime viewport rectangle (still blocked on pass 47b
  ZONES.H parse even after chrome reroute — `V1_BLOCKERS.md` §4);
- replace champion HP/stamina/mana numeric text with bar graphs
  (`V1_BLOCKERS.md` §7, pass 43);
- route spell-panel rune labels through `C011`
  (`V1_BLOCKERS.md` §8, pass 44);
- wire the extracted GRAPHICS.DAT font atlas
  (`V1_BLOCKERS.md` §9, pass 45);
- swap the VGA palette (`V1_BLOCKERS.md` §10, pass 46);
- touch M10 semantics, any movement / combat / sensor / door /
  spell behavior, or any rendering path outside the chrome surfaces
  listed in §1.2;
- drop the `Tn:` tick prefix on `m11_log_event` entries
  (`V1_BLOCKERS.md` §12, pass 48) — the reroute payloads do not
  carry a `Tn:` prefix because they come from `m11_set_status` /
  `m11_set_inspect_readout`, not from `m11_log_event`;
- delete any of the 82 `m11_set_status` call sites or 68
  `m11_set_inspect_readout` call sites (they still write the
  invented surfaces so `showDebugHUD` remains useful);
- promote the `PARITY_MATRIX_DM1_V1.md` §4 "Over-labeling" row to
  `MATCHED`; it now reads "`MATCHED` for the control strip,
  `KNOWN_DIFF` for the other enumerated surfaces" (see §6 below).

What pass 42 **does** is exactly what `V1_BLOCKERS.md` §6 asked for:

> add a V1 chrome-mode switch that hides these Firestaff-invented
> surfaces and reroutes their notifications into the source-faithful
> message-log surface.

The switch is added, the control-strip row (the one invented surface
still drawn in non-debug V1) is hidden, and the two largest
notification sources are rerouted into the message log.

---

## 6. Proposed status updates for `PARITY_MATRIX_DM1_V1.md`

| Row | Before pass 42 | After pass 42 |
|---|---|---|
| §4 Over-labeling — control strip (y=165) | `KNOWN_DIFF` | **`MATCHED`** (suppressed in V1 chrome mode) |
| §4 Over-labeling — status lozenge reroute (`m11_set_status`, 82 sites) | `KNOWN_DIFF` | **`PARTIALLY MATCHED`** — invented rendering surface remains behind `showDebugHUD`; V1 chrome mode reroutes player-facing payloads into the message log |
| §4 Over-labeling — inspect readout reroute (`m11_set_inspect_readout`, 68 sites) | `KNOWN_DIFF` | **`PARTIALLY MATCHED`** — as above |
| §4 Over-labeling — prompt strip (y=165) | `KNOWN_DIFF` | `MATCHED` (already debug-only before pass 42; pass 42 clarifies) |

**This pass alone does not complete DM1/V1 parity.**  It retires
`V1_BLOCKERS.md` §6 ("Firestaff-invented UI chrome") by hiding the
last invented chrome surface still drawn in V1 and giving the
invented text notifications a source-faithful surface, which is
the deliverable the pass-42 slot in this program was scoped to
produce.

Retirement of `V1_BLOCKERS.md` §4 (viewport rectangle) remains
blocked on pass 47b (ZONES.H parse); the control-strip removal
is a prerequisite pass 47b will build on, but the viewport swap
cannot land here because the four pass-41 status-box rectangles
still occupy the DM1 rectangle's y=160..168 band.
