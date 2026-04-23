# Firestaff Post-14:00 Handoff Plan (GPT-5.4 + Codex only)

**Written:** 2026-04-23, before 14:00 local time, while Opus 4.6 is still available.
**Scope:** plan how to keep Firestaff work moving after Opus 4.6 is no longer available today, using only **GPT-5.4** and **Codex**.

This document is planning only. It does not modify V1/M10 code or V2 assets. It does not schedule any repo reorganization work — that is explicitly postponed until DM1/V1 is complete.

It has two clearly separated execution tracks:

1. **DM1/V1 fidelity in M11** — continues after in-flight pass 27.
2. **V2 implementation track** — continues after Phase 6.

Plus:

3. **Routing matrix** — which task types go to GPT-5.4 vs Codex.
4. **Risks + mitigations** — for parity-sensitive work under the weaker model mix.

---

## 0. Model/tool capability assumptions after 14:00

We assume, after 14:00:

- **GPT-5.4** is available as the main reasoning/authoring model for:
  - research, spec writing, structured plans, checklists
  - code review of small/medium changes
  - small, well-scoped code changes with strong reference material
  - CI triage and verification framing
  - asset-production coordination and JSON/table authoring
  - writing tightly bounded pass briefs
- **Codex** is available as an implementation agent for:
  - bounded, spec-driven code changes inside a single module/file cluster
  - converting a well-written pass brief into a working commit
  - mechanical refactors that are already justified
  - test/probe extensions against an existing shape
- Neither model is assumed strong enough to match Opus 4.6 on:
  - long-range parity reasoning across DM1 disassembly + ReDMCSB + Firestaff source at once
  - deciding *what the next V1 slice should be* purely from screenshots
  - arbitrating subtle fidelity disagreements between ReDMCSB and CSBwin
  - large, cross-cutting V2 integration passes where presentation and engine meet

So the strategy is:

- **Lean on bounded passes with pre-written briefs.**
- **Keep parity-sensitive judgement cheap to verify.**
- **Defer judgement-heavy parity arbitration until a stronger model is available again.**

---

## 1. DM1/V1 post-14 execution plan

### 1.1 Where we are

- M11 game-view parity track has landed verified passes 23–26.
- Pass 27 is in flight separately (do not assume it landed; do not claim it did).
- Baseline invariants (175/175 game-view + 18/18 Phase A + 4/4 audio) remain the floor.
- Primary truth sources remain: `GRAPHICS.DAT`, `DUNGEON.DAT`, ReDMCSB, CSBwin.
- Primary fidelity yardstick remains: **does a screenshot read like Dungeon Master?** (see `M11_V1_SCREENSHOT_GAP_SPEC.md`).

### 1.2 Working rules under GPT-5.4 + Codex

1. **Only execute passes that already have a written brief.** Do not let GPT-5.4 decide “what looks most like DM” from a blank slate. That kind of open-ended parity judgement is what Opus 4.6 was doing; post-14 we do not have that depth.
2. **Every pass must cite a specific source reference** (file + function + ReDMCSB component or F-number) before any code changes. If it cannot cite one, defer the pass.
3. **Do not rewrite or restructure `M11` game-view composition in one pass.** Keep changes inside one DM1 function family or one UI region per pass.
4. **M10 stays semantically untouched.** Enforced by existing verify gates — do not relax them.
5. **Always run the minimum verification set** on any touched surface:
   - `./run_firestaff_m11_phase_a_probe.sh`
   - `./run_firestaff_m11_game_view_probe.sh`
   - `./run_firestaff_m11_launcher_smoke.sh`
   - `./run_firestaff_m10_verify.sh "$HOME/.firestaff/data/GRAPHICS.DAT"`
6. **No "close enough" claims.** If the probe is green but the screenshot is still visibly off DM1, call that honestly in the pass report.

### 1.3 Next 3–6 bounded DM1/V1 passes after pass 27

Listed in priority order. Each is scoped so GPT-5.4 can produce a brief and Codex can land the change. None depend on model-level DM1 arbitration.

1. **Pass 28 — Action-menu row rendering fidelity second slice**
   - Scope: after pass 27 lands the first half of action-menu row visuals, align remaining row cells (icons, text area, highlight) to DM1 `F0386_MENUS_DrawActionIcon` family.
   - Source: DM1 `F0386` / `F0391` family, ReDMCSB action-menu component.
   - Probe: extend existing action-menu invariants in `firestaff_m11_game_view_probe.c`.
   - GPT-5.4 role: write the brief. Codex role: land the commit.

2. **Pass 29 — Spell-area frame + rune panel geometry alignment**
   - Scope: make the spell-area panel (frame, rune cells, symbol readout) match DM1 geometry; do not touch casting logic.
   - Source: ReDMCSB spell-area component, DM1 F-functions already referenced in prior commits.
   - Probe: bounded geometry invariants, no logic change.
   - GPT-5.4 role: brief + reference table. Codex role: geometry-only change.

3. **Pass 30 — Status-box content fidelity (left + right pair)**
   - Scope: left/right status boxes (name band, health/stamina/mana bars, load, injury strip) rendered with DM1 ordering and widths. No data-model changes.
   - Source: ReDMCSB status-box component + DM1 status draw F-functions.
   - Probe: per-field rect invariants.
   - Defer if pass 27 alters the status-box render path; otherwise go.

4. **Pass 31 — Message-log panel DM1 geometry + glyph alignment**
   - Scope: position, clipping, scroll mechanic, and glyph baseline of message log to match DM1.
   - Source: ReDMCSB message-log component + DM1 message F-functions.
   - Probe: add/expand message-log rect + line-count invariants; keep existing INV_GV_21, 24 green.

5. **Pass 32 — Inventory-overlay DM1 frame and slot geometry (V1-only)**
   - Scope: when the inventory overlay opens, its frame, slot layout, and backdrop read as DM1, not as a modern overlay. Drag/drop logic untouched.
   - Source: ReDMCSB inventory component.
   - Risk: this is closer to the parity-sensitive band (many screen regions interact). Keep strictly cosmetic / geometric.

6. **Pass 33 — DM1 minimap/map-overlay geometric fidelity (V1-only)**
   - Scope: map-overlay framing and minimap inset proportions to DM1. No new features.
   - Source: ReDMCSB map component + original screenshots.
   - This pass is the last acceptable *purely geometric* slice before we hit the band where Opus-class parity judgement helps most.

### 1.4 DM1/V1 work to defer until stronger parity reasoning is available again

Do **not** attempt these with GPT-5.4 + Codex only:

- Open-ended “make the whole in-game screen read more like DM1” passes that are not pre-scoped to one region.
- Arbitration passes where ReDMCSB, CSBwin, and DM1 disassembly disagree.
- Creature render fidelity decisions that require cross-referencing multiple pose tables + animation timing.
- Palette/color correction passes that are not already resolved into a numeric palette table.
- Any pass whose main decision is “what slice is highest-value next?” without an existing brief.

These are queued as “Opus-class” passes and should be labeled as such in future plan docs.

### 1.5 DM1/V1 pass shape (use this template)

Every post-14 V1 pass must be expressible as:

```
Label: firestaff-m11-dm1-v1-pass-NN-<short-name>
One-region scope: <UI region or F-function family>
Source citation: <DM1 F-number(s) / ReDMCSB component(s)>
Verification set: <list of probes/smokes to run>
Hard rules: M10 untouched; tracked content English; no speculative refactor
Success = screenshot + probe diff both consistent with DM1
Failure = report honestly; do not claim "close enough"
```

If the brief cannot be filled in fully, the pass is not ready for GPT-5.4/Codex.

---

## 2. V2 post-14 execution plan

### 2.1 Where we are

- V2 foundation landed: roadmap, matrix, upscale plan, asset classification, vertical slice pack, wave-1 production pack.
- First real assets landed (first-pass V2 vertical slice asset pack — commit `1448716`).
- First engine integration landed (V2 vertical-slice game view skinning — commit `8286bb3`).
- Phase 6 started with GPT-5.4 already. Post-14 it continues with GPT-5.4 + Codex only.

### 2.2 Working rules under GPT-5.4 + Codex

1. **V2 is the safer track for GPT-5.4** because it is presentation-over-logic and does not require DM1/ReDMCSB arbitration at the same depth as V1.
2. **V2 must not change V1 rules.** If a change touches shared code, the default is to branch via a V2 path, not to modify V1 behavior.
3. **Asset production continues to be spec-first.** GPT-5.4 authors the asset contract (IDs, sizes, families, layering). Codex can land build-glue and manifest updates, but does not invent art decisions.
4. **Each V2 pass must be bounded to one family** (viewport frame, action area, status box, party HUD cell, spell area, etc.) or to one subsystem (scaling, AA, tone, preset plumbing).
5. **Keep V1 probes green.** Any V2 pass must run the V1 verification set as a regression gate.

### 2.3 Next 3–5 bounded V2 passes after Phase 6

1. **Phase 7 — Party HUD cell family expansion**
   - Scope: extend the party HUD cell family beyond the vertical-slice single-cell case to the full 4-slot strip, with matching highlight/active overlays.
   - Assets: already specified in `V2_VERTICAL_SLICE_PACK.md`.
   - GPT-5.4 role: asset manifest + integration brief. Codex role: wire 4 slots, reuse existing cell base.

2. **Phase 8 — Spell-area V2 family (slice-level)**
   - Scope: introduce the V2 spell-area family at vertical-slice scope (frame, rune cells bed, symbol readout shell). No rune/logic changes.
   - This is the first family that was explicitly **not** in the vertical-slice pack; it is the natural next family.
   - GPT-5.4 role: extend the asset contract table. Codex role: add skins + layer mapping.

3. **Phase 9 — V2 preset plumbing (V2.1 baseline)**
   - Scope: data + loading path for V2.1 baseline presets (scale mode, AA off/FXAA, brightness/contrast/gamma). No UI yet beyond a minimum settings hook.
   - Source: `V2_ROADMAP_MATRIX.md` V2.1 column.
   - This is a Codex-friendly pass once GPT-5.4 writes the preset schema.

4. **Phase 10 — V2 scaling family first integration**
   - Scope: wire integer + linear + one enhanced upscale path into the V2 render pipeline with a runtime switch. No filter-family explosion.
   - Keep edge-aware/HQ/xBR/AI upscale out — those are V2.2+.
   - Risk: this touches the V2 render path itself; keep V1 path untouched.

5. **Phase 11 — V2 tone controls first integration (brightness/contrast/gamma)**
   - Scope: brightness, contrast, gamma controls applied to V2 presentation only. No V1 touch.
   - Straightforward shader/LUT pass; Codex-friendly once GPT-5.4 writes the numeric ranges and defaults.

### 2.4 V2 work to defer until stronger parity/fidelity reasoning is available again

- **Anything that reinterprets DM1 look-and-feel into V2** beyond what is already in the slice pack. Deciding how DM1 aesthetics should translate into enhanced 2D is judgement-heavy.
- **Upscale families beyond V2.1** (edge-aware, HQ/xBR, AI/hybrid). These need careful quality arbitration.
- **CRT / scanline / phosphor families.** These are late V2.3 and need taste calls.
- **Cross-track decisions** where a V2 change would imply a V1 parity decision. Route those to Opus-class when available.

### 2.5 V2 pass shape (use this template)

```
Label: firestaff-v2-phase-NN-<short-name>
One-family scope: <viewport_frame | action_area | status_box_family | party_hud_cell_family | spell_area | scaling | AA | tone | preset>
Asset contract: <IDs + sizes + layering> or N/A
Source citation: V2_ROADMAP.md / V2_ROADMAP_MATRIX.md / V2_VERTICAL_SLICE_PACK.md / V2_WAVE1_PRODUCTION_PACK.md
V1 regression gate: game-view + phase-a + launcher-smoke + m10 verify must stay green
Success = V2 family renders as specified AND V1 regression gate stays green
```

---

## 3. Routing matrix — GPT-5.4 vs Codex after 14:00

Compact decision table. "Primary" is where the task *starts*. The other model may assist.

| Task type | Primary | Secondary | Notes |
|---|---|---|---|
| Research / source reading (DM1, ReDMCSB, CSBwin) | GPT-5.4 | — | Produces a citation list, not a change. |
| Spec / plan / pass brief authoring | GPT-5.4 | — | Must end in a filled-in pass template. |
| Small code change (≤ ~150 lines, one file cluster) | Codex | GPT-5.4 reviews | Codex lands; GPT-5.4 reviews diff before push. |
| Large code change (multi-file, multi-region) | **Defer** | — | Split into bounded passes first. No direct large change after 14:00. |
| Verification / probe extension | Codex | GPT-5.4 writes invariants | GPT-5.4 defines the invariants; Codex wires them. |
| Asset production coordination (manifests, contracts, tables) | GPT-5.4 | Codex applies JSON/build glue | No art decisions delegated to Codex. |
| Asset pipeline build glue (manifest loaders, CMake entries) | Codex | GPT-5.4 reviews | Must not regress V1. |
| V2 integration pass (one-family scope) | Codex | GPT-5.4 briefs | Family-bounded only. |
| V2 integration pass (cross-family) | **Defer** | — | Needs Opus-class judgement. |
| DM1/V1 parity slice (bounded, geometry/render only) | Codex | GPT-5.4 briefs | Must cite DM1 F-number(s). |
| DM1/V1 parity arbitration (ReDMCSB vs CSBwin vs DM1 disagree) | **Defer** | — | Queue as Opus-class. |
| Palette / color correction not already numerically resolved | **Defer** | — | Queue as Opus-class. |
| CI triage (build break, flaky probe, platform difference) | GPT-5.4 | Codex patches | GPT-5.4 diagnoses; Codex lands the fix. |
| Release packaging / DMG/AppImage/NSIS work | Codex | GPT-5.4 reviews | Mechanical enough for Codex. |
| Docs (planning, status, release notes, READMEs) | GPT-5.4 | — | English, tracked, no inflated claims. |
| Repo reorganization | **Deferred entirely** | — | Not before DM1/V1 complete. |

Rule of thumb:

- If the hardest part of the task is **deciding what to do**, route to GPT-5.4 or defer.
- If the hardest part is **typing a bounded, cited change**, route to Codex.
- If the hardest part is **arbitrating parity across conflicting sources**, defer to Opus-class.

---

## 4. Risks + mitigations for parity-sensitive work

| Risk | Why it matters | Mitigation |
|---|---|---|
| GPT-5.4 over-claims DM1 fidelity from screenshots | Screenshots can look “close” while being wrong in ways Opus caught. | Require F-number + ReDMCSB component citation in every V1 pass. No citation → defer. |
| Codex writes plausible-but-wrong parity code | Codex is fast but does not deeply arbitrate parity sources. | GPT-5.4 must review the diff before push; keep scope to one region per pass. |
| Drift in M10 semantics through unrelated changes | M10 is the verify baseline. | Keep `run_firestaff_m10_verify.sh` mandatory on every touching pass. |
| V2 changes leak into V1 render path | Breaks parity silently. | V2 must branch; V1 regression gate mandatory. |
| Passes get too big because GPT-5.4 generalizes | Larger passes hide bad judgement. | Enforce the pass-shape templates; reject briefs without a one-region scope. |
| Deferred work accumulates | Backlog balloons before next Opus window. | Keep an explicit “Opus-class queue” section in future plan updates; do not let it hide. |
| Silent fidelity regressions between passes | Screenshot-level regressions slip past probes. | Continue screenshot capture artifacts per pass; compare against previous pass screenshots, not just probes. |
| Asset/engine contract drift in V2 | Asset IDs and sizes must stay stable. | Any asset change routed through `V2_VERTICAL_SLICE_PACK.md` / `V2_WAVE1_PRODUCTION_PACK.md`; GPT-5.4 must update the contract table in the same pass. |

---

## 5. Opus-class queue (park here, do not execute after 14:00)

Use this section as the honest landing place for work we are **not** doing post-14. Do not quietly do them with GPT-5.4/Codex.

- Open-ended "make it look more like DM" passes without pre-scoped region.
- ReDMCSB vs CSBwin vs DM1 disassembly arbitration.
- Palette/color correction not already numerically captured.
- Creature animation/pose timing fidelity across full pose table.
- Cross-family V2 integration passes.
- V2 taste calls on CRT / scanline / advanced upscale families.
- Repo reorganization (already explicitly postponed until DM1/V1 complete).

---

## 6. Summary recommendations

1. **Continue DM1/V1 with a strictly bounded, citation-required pass pipeline.** After pass 27, the next 3–6 passes (28–33) are all geometric/render fidelity in one UI region each, all expressible as a filled-in pass template.
2. **Continue V2 via one-family passes (Phases 7–11).** V2 is the safer post-14 track because it is presentation-over-logic and decouples cleanly from V1 arbitration.
3. **Route by hardest part of the task.** Research/spec/review → GPT-5.4. Bounded implementation → Codex. Parity arbitration → defer.
4. **Hold M10 verify and V1 regression gates as non-negotiable** on every touched surface, V1 and V2 alike.
5. **Do not start repo reorganization.** Explicitly postponed until DM1/V1 is complete.
6. **Maintain the Opus-class queue honestly.** Parity arbitration, open-ended DM1 look passes, and cross-family V2 integration wait for a stronger model; do not quietly let GPT-5.4 do them.
