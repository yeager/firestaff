# V1 Blocker Ledger — Firestaff DM1/V1 original-faithful mode

Last updated: 2026-04-24
Owner of this file: Pass 36 "honesty lock" (see `PASSLIST_29_36.md` §4.36)
Primary consumers: pass-37+ planning, `STATUS.md`

This is the **single source of truth** for what is still outstanding
between Firestaff and DM1 PC 3.4 English in V1 original-faithful mode
after Passes 29–35.  Every entry contains:

- short title
- area tag (`OWNERSHIP`, `VISUAL`, `BEHAVIOR`, `ASSET`, `TEXT`)
- evidence path or reason it is a blocker
- suggested next bounded pass label (pass-37, pass-38, …)

Entries are numbered in priority order (lower number = higher priority
for V1 convergence).  Priority follows the 1:1 analysis plan: ownership
first, then visual parity, then typography / honesty.

---

## 1. Compat-runtime integration of sensor enter/leave events
- **Area:** `OWNERSHIP`
- **Evidence:**
  - Pass 32 landed the compat-side owner
    `F0718_SENSOR_ProcessPartyEnterLeave_Compat` and probe-verified it
    at 12/12 invariants, but
    `memory_tick_orchestrator_pc34_compat.c:F0888_ORCH_ApplyPlayerInput_Compat`
    does NOT yet invoke it on every party square change.
  - `M11_OWNERSHIP_AUDIT_MOVEMENT_DOORS_ENV.md` §3 item "sensor
    ownership migration" — the owner exists but is not the runtime
    owner yet.
- **Suggested pass:** pass-37 — wire `F0718` into the orchestrator's
  enter/leave emission around the F0702/F0704 movement result, keeping
  the effect list application bounded to teleport + text in v1 so
  regressions are small.

## 2. Animating door states (1..3) still snapped to 0/4
- **Area:** `OWNERSHIP`
- **Evidence:**
  - Pass 31 `F0715_DOOR_ResolveToggleAction_Compat` treats animating
    door states 1..3 as synonymous with "closed" and always toggles
    to 0 or 4.  `memory_door_action_pc34_compat.c` header comment
    explicitly flags this as deferred.
  - `PASSLIST_29_36.md` §4.31.5 "Explicitly out of scope" — animating
    intermediate states.
- **Suggested pass:** pass-38 — add a timeline-driven door-animation
  compat owner modeled on `TIMELINE_EVENT_DOOR_ANIMATE`.

## 3. Creature walkability not unified with party F0706
- **Area:** `OWNERSHIP`
- **Evidence:**
  - Pass 30 kept `m11_square_walkable_for_creature` on legacy logic
    because delegation to `F0706_MOVEMENT_IsSquarePassable_Compat` triggered
    a game-view probe heap regression (changed creature walking
    behavior for STAIRS squares).
  - `memory_movement_pc34_compat.h` F0706 doc comment flags the
    unification as deferred.
- **Suggested pass:** pass-39 — add a `creature-context` variant of
  F0706 (or a flag) that rejects STAIRS for creatures, then delegate.

## 4. Viewport region −28×−18 px vs DM1 original
- **Area:** `VISUAL`
- **Evidence:**
  - Pass 33 `parity-evidence/pass33_viewport_coordinate_overlay.md` §3
    — Firestaff viewport is 196×118; DM1 is 224×136 (DEFS.H
    `C112_BYTE_WIDTH_VIEWPORT` / `C136_HEIGHT_VIEWPORT`).
  - `PARITY_MATRIX_DM1_V1.md` §1 "Viewport region bounds" row now
    `KNOWN_DIFF`.
- **Suggested pass:** pass-40 — adjust the Firestaff viewport renderer
  to target 224×136 or explicitly freeze the dimensional drift as a
  V1 known-diff with a documented reason.

## 5. Champion status-box stride +8 px per slot vs DEFS.H C69
- **Area:** `VISUAL`
- **Evidence:**
  - Pass 34 `parity-evidence/pass34_sidepanel_rectangle_table.md` §3
    — `M11_PARTY_SLOT_STEP=77` vs `C69_CHAMPION_STATUS_BOX_SPACING=69`
    (DEFS.H:1756).
  - `PARITY_MATRIX_DM1_V1.md` §1 "Party/champion region" now
    `KNOWN_DIFF` (+8 px stride).
- **Suggested pass:** pass-41 — set `M11_PARTY_SLOT_STEP` to 69 in V1
  mode, measure downstream impact on slot contents.

## 6. Firestaff-invented UI chrome (utility strip, control strip, prompt strip, status lozenge, inspect readout)
- **Area:** `TEXT` (`VISUAL` adjacent)
- **Evidence:**
  - `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.2, §2.4, §2.5,
    §2.6 — 82 `m11_set_status` sites + 68 inspect-readout sites +
    Inspect/Save/Load captions + control/prompt strips at y=165.
  - `PARITY_MATRIX_DM1_V1.md` §4 "Over-labeling" now `KNOWN_DIFF`
    with the enumeration.
- **Suggested pass:** pass-42 — add a V1 chrome-mode switch that hides
  these Firestaff-invented surfaces and reroutes their notifications
  into the source-faithful message-log surface.

## 7. Champion HP/stamina/mana shown as numeric strings instead of bar graphs
- **Area:** `VISUAL`
- **Evidence:**
  - `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.7 — DM1 renders
    these via `CHAMDRAW.C` bar graphs (ZONES C187..C190), Firestaff
    renders numeric glyph strings.
- **Suggested pass:** pass-43 — replace the numeric strip with the
  bar-graph renderer.  Asset is already extracted.

## 8. Spell-panel rune labels still text, not `C011` graphic blit
- **Area:** `VISUAL`
- **Evidence:**
  - `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.8 — source uses
    `C011_GRAPHIC_MENU_SPELL_AREA_LINES` (14×39) for rune glyphs.
  - Pass 34 asset-usage map confirms index 11 is already wired as
    `M11_GFX_SPELL_AREA_LINES` but currently as backdrop, not as
    the per-rune label.
- **Suggested pass:** pass-44 — route each entered-rune readout
  through a sub-sprite of graphic 11.

## 9. Custom 7-pixel font used in place of original GRAPHICS.DAT font atlas
- **Area:** `TEXT`
- **Evidence:**
  - `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.1 — `m11_draw_text`
    has a delegation path through `g_activeOriginalFont` but it is
    not wired to the extracted font bank.
- **Suggested pass:** pass-45 — wire the extracted GRAPHICS.DAT font
  atlas into `g_activeOriginalFont` and make the delegate the default
  path.

## 10. VGA palette compat layer still uses EGA-style colors
- **Area:** `VISUAL` (`BEHAVIOR` adjacent for brightness)
- **Evidence:**
  - `PARITY_MATRIX_DM1_V1.md` §3 "Base palette" / "Brightness levels"
    / "Cyan invariant" / "Special palettes" rows — all `KNOWN_DIFF`
    with concrete deltas in `parity-evidence/dm1_v1_pass1_palette_and_assets.md`.
  - Predates Passes 29–36; left intentionally untouched so this work
    does not bundle palette changes in.
- **Suggested pass:** pass-46 — land the `recovered_palette.json`
  values in `vga_palette_pc34_compat.c`.  This is a single bounded
  swap; defer brightness / creature palettes to a follow-up.

## 11. ReDMCSB pixel overlays missing for viewport and side panel
- **Area:** `VISUAL`
- **Evidence:**
  - Pass 33 §4 and Pass 34 §3 both have `BLOCKED_ON_REFERENCE` rows
    for pixel-level placements because no ReDMCSB headless rasteriser
    is in-tree and no reference emulator capture has been produced.
- **Suggested pass:** pass-47 (blocked on tooling) — land a ReDMCSB
  headless rasteriser OR capture original DM1 PC 3.4 VGA frames via
  DOSBox with deterministic state.

## 12. Tick-prefix (`Tn:`) on message log lines
- **Area:** `TEXT`
- **Evidence:**
  - `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.3 — the tick
    prefix is a Firestaff convention, not source-faithful.
- **Suggested pass:** pass-48 — drop the `Tn:` prefix in V1 mode
  (single-line change in `m11_log_event`).

## 13. Animating door intermediate states handled upstream of F0715
- **Area:** `OWNERSHIP` (already noted in blocker #2 but tracked
  separately for timeline ownership)
- **Evidence:** same as #2.
- **Suggested pass:** pass-38 (same as #2).

## 14. M9/M10 behavioral probes verify internal consistency only, not original-match
- **Area:** `BEHAVIOR`
- **Evidence:** `PARITY_MATRIX_DM1_V1.md` §5 rows all `UNPROVEN`.
- **Suggested pass:** pass-49 — bind at least one high-value M10 probe
  (movement-champions or dungeon-doors-sensors) to a ReDMCSB-produced
  expected log once pass-47 tooling lands.

## 15. Audio samples are procedural placeholders
- **Area:** `ASSET`
- **Evidence:** `PARITY_MATRIX_DM1_V1.md` §7 "Sound samples (content)"
  already `KNOWN_DIFF`.
- **Suggested pass:** pass-50 — investigate SONG.DAT extraction.

---

## Priority groupings

- **High (pass-37 … pass-41):** ownership wiring (#1), stairs/door
  edge cases (#2, #3), measurable visual drifts with DEFS.H anchors
  (#4, #5).
- **Medium (pass-42 … pass-46):** invented chrome + text-as-graphic
  rework (#6, #7, #8), font bank (#9), palette (#10).
- **Blocked / tooling (pass-47):** overlays need capture infra (#11).
- **Lower (pass-48 … pass-50):** cosmetic tick prefix (#12, dup #13),
  behavioral probe binding (#14), audio (#15).

---

## Scope that is NOT a V1 blocker

These are listed to keep the ledger honest about what pass-37+ should
NOT spend time on:

- **M10 semantics:** passes 29–36 preserved M10.  M10 parity is not
  on this ledger; it is its own workstream.
- **V2 / V3:** out of scope per `STATUS.md` product direction.
- **M12 launcher / modern menu:** not on the V1 parity path.
- **Repo reorg:** postponed until DM1/V1 lock; not a parity item.

---

## Status after Passes 29–35 (as of 2026-04-24)

- 4 ownership passes landed (29, 30, 31, 32) with probe-verified
  compat owners for post-move env, movement legality, door actuator,
  and multi-sensor enumeration.  Runtime *wiring* for sensor
  enter/leave is still #1 on this ledger.
- 2 visual-lock measurement passes landed (33, 34) with concrete
  numeric drifts recorded in `parity-evidence/pass33_*.md` and
  `pass34_*.md`.
- 1 typography audit landed (35) with full enumeration of V1
  text-emitting surfaces in `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md`.
- Pass 36 itself (this file + the matrix honesty pass) does not land
  new code or new probes.

**Passes 29–36 together have not completed DM1/V1 parity.**  The
blocker ledger records what remains.
