# Firestaff vs ReDMCSB startup/menu visual comparison

Date: 2026-04-23
Scope: bounded visual comparison of Firestaff `verification-screens/01_start_menu.png` against local ReDMCSB-derived startup/title/menu artifacts already present in this repo.

## Conclusion

Firestaff's current startup screen is **not visually aligned** with the closest trustworthy local ReDMCSB startup/title/menu references.

The current Firestaff screen reads as a **modern custom launcher UI** with large framed panels, bright category/status labels, and a bespoke "FIRESTAFF" title banner. The local ReDMCSB references instead read as **sparse original-style title/menu fragments on mostly black screen space**, with tiny centered/title-range pieces and much less on-screen chrome.

Because the nearest local ReDMCSB artifacts are still fragmentary rather than a single perfectly legible full startup screenshot, this comparison is **grounded but approximate**. ReDMCSB still remains the visual truth when it disagrees with Firestaff.

## Exact files used

### Firestaff current screen
- `verification-screens/01_start_menu.png`

### Primary ReDMCSB truth references
- `exports_2026-04-17_title_menu_composed/m6_boot_attempt_a_step_0001.png`
- `exports_2026-04-17_title_menu_composed/m6_boot_attempt_a_step_0032.png`
- `exports_2026-04-17_title_menu_composed/m6_boot_attempt_a_step_0056.png`
- `analysis_m4/menu_candidate_b_wide_sheet.png`

### Secondary context references
- `exports_2026-04-17_title_menu_composed/m6_boot_attempt_a_composed_contact.png`
- `exports_2026-04-17_title_menu_best/m6_boot_attempt_a_contact.png`
- `analysis_m4/menu_candidate_b_sheet.png`
- `analysis_m4/menu_candidate_b_left_sheet.png`
- `analysis_m4/menu_candidate_b_right_sheet.png`

## Why these ReDMCSB files were chosen

### 1. `m6_boot_attempt_a_step_0001.png`
This is the closest trustworthy local reference for the **early startup/title state** because it is an actual composed 320x200 ReDMCSB-derived frame rather than a Firestaff reinterpretation. It is still sparse, but it reflects the original-style screen state more honestly than the current Firestaff launcher.

### 2. `m6_boot_attempt_a_step_0032.png`
This is useful as a mid-sequence check because it still shows the original path as **small centered title/menu-related content on a black field**, not a full boxed frontend.

### 3. `m6_boot_attempt_a_step_0056.png`
This is useful as the later stable title-range frame already called out elsewhere in the repo as a likely patch/overlay-related match candidate. It is still tiny and fragmentary, but that is exactly why it is valuable here: it shows how far Firestaff's current menu language drifted away from the original-style source path.

### 4. `analysis_m4/menu_candidate_b_wide_sheet.png`
This is the strongest local menu-fragment sheet because it gathers the widest set of startup/menu candidate fragments into one reference artifact. It is not a full final screen, but it is the best local truth for the **shape language, fragment scale, and low-density composition** that the ReDMCSB path is actually exposing.

## What Firestaff currently shows

`verification-screens/01_start_menu.png` currently shows:
- a large stylized `FIRESTAFF` banner across the top
- a small `FRONTEND PREVIEW` label above that banner
- a left boxed status area with `READY`
- a right boxed launcher/menu list with multiple labeled destinations
- explicit bright status words like `READY`, `SCHEMA`, and `OPEN`
- bottom instruction text for controls
- strong red, blue, green, and gray panel framing across most of the screen

This reads as a bespoke frontend/launcher rather than an original-faithful startup/title/menu composition.

## What the local ReDMCSB/original path shows

The selected ReDMCSB-derived references consistently show:
- mostly black negative space
- tiny centered or title-range fragments rather than a large full-screen launcher frame
- much smaller-scale graphic components
- no evidence in these local references of the big Firestaff title banner
- no evidence in these local references of the large left/right panel boxes
- no evidence in these local references of bright modern status labels or bottom help text bars

The candidate sheets also suggest the startup/title/menu path is built from **small original asset fragments** rather than a large custom UI composition.

## Main visual mismatches

### 1. Composition mismatch
Firestaff uses a full-screen boxed launcher layout with multiple large panels. The local ReDMCSB path is mostly black space with small title/menu-related fragments. Firestaff is visually dense where ReDMCSB is visually restrained.

### 2. Palette mismatch
Firestaff leans on bright red borders, blue menu bars, green status text, and a broad multi-color frontend palette. The chosen ReDMCSB references read as much darker and sparser, with the visible title/menu content occupying far less colored area.

### 3. Placement mismatch
Firestaff places major elements across the whole screen: top banner, left status box, right options list, bottom controls. The ReDMCSB references instead keep the visible content tiny and near the center/title region. Firestaff's element placement is therefore structurally wrong even before fine art details are considered.

### 4. Text and graphics usage mismatch
Firestaff currently depends on many readable UI labels and status tags to define the screen. The ReDMCSB-local references do not show this kind of text-heavy launcher presentation. That suggests Firestaff is using the wrong screen language, not just the wrong pixel art.

### 5. Scale mismatch
The Firestaff title and menu boxes are huge relative to the frame. The ReDMCSB fragments are tiny. Even if some asset identity is eventually shared, Firestaff is currently scaling and presenting the startup/menu state like a modern menu shell instead of an original-style title/menu composition.

### 6. State-sequencing mismatch
The local ReDMCSB sequence files (`step_0001`, `step_0032`, `step_0056`) imply a startup/title path that evolves through small centered/title-range content. Firestaff instead presents a fully formed launcher immediately. That likely means the current screen is also wrong at the **state-sequencing** level, not only at the visual-asset level.

## Likely causes by category

### Wrong assets
- The large `FIRESTAFF` wordmark reads as a custom asset, not an original ReDMCSB/title-path asset.
- The bright boxed menu styling and status-chip language (`READY`, `SCHEMA`, `OPEN`) do not appear grounded in the chosen ReDMCSB references.

### Wrong layout
- The left-box/right-box/footer-instruction layout does not match the structure implied by the ReDMCSB-derived frames.
- The screen is over-filled and over-framed versus the original-style sparse composition.

### Wrong palette
- The current startup screen uses an expanded modern frontend palette instead of the darker, more limited, more original-looking presentation suggested by the ReDMCSB references.

### Wrong state sequencing
- Firestaff appears to jump straight to a finished launcher state.
- The ReDMCSB sequence implies startup/title/menu content should emerge as a narrower title-range state, not as an immediate two-panel shell.

## Exact future visual truth for startup/menu work

Use these as the canonical comparison set for future startup/menu corrections:

1. Firestaff current capture:
   - `verification-screens/01_start_menu.png`
2. ReDMCSB early composed title/menu state:
   - `exports_2026-04-17_title_menu_composed/m6_boot_attempt_a_step_0001.png`
3. ReDMCSB mid-sequence check:
   - `exports_2026-04-17_title_menu_composed/m6_boot_attempt_a_step_0032.png`
4. ReDMCSB later stable title-range check:
   - `exports_2026-04-17_title_menu_composed/m6_boot_attempt_a_step_0056.png`
5. ReDMCSB menu-fragment sheet:
   - `analysis_m4/menu_candidate_b_wide_sheet.png`

If one single ReDMCSB artifact must be used first, use:
- `exports_2026-04-17_title_menu_composed/m6_boot_attempt_a_step_0001.png`
for startup-state structure, and
- `analysis_m4/menu_candidate_b_wide_sheet.png`
for local title/menu fragment truth.

## Bounded next fixes

1. **Remove or drastically reduce the custom launcher shell**
   - Stop treating startup as a full left-panel/right-panel desktop-like menu.
   - Replace it with a composition that preserves much more black negative space.

2. **Drop the current large custom `FIRESTAFF` banner from the original-faithful startup path**
   - Keep branding out of the V1/original-faithful startup path unless it is explicitly separated from original mode.

3. **Eliminate the current bright status-chip language from the startup screen**
   - `READY`, `SCHEMA`, `OPEN`, and similar labels should not drive the visual composition of the original-faithful startup/menu state.

4. **Rebuild startup/menu layout around the chosen ReDMCSB reference set**
   - Use `m6_boot_attempt_a_step_0001.png`, `step_0032.png`, and `step_0056.png` as geometry/negative-space checks.
   - Use `menu_candidate_b_wide_sheet.png` as the asset-shape/style check.

5. **Treat startup/title/menu as a sequenced state, not a static launcher mockup**
   - Verify that the first visible Firestaff startup/menu frames progress through a narrow title-range composition closer to the ReDMCSB steps before any later interaction state is shown.

6. **Keep future comparisons bounded and visual**
   - After each startup/menu change, regenerate or reuse `verification-screens/01_start_menu.png` and compare it directly against the exact ReDMCSB truth files listed above instead of judging by feel.

## Confidence and limits

Confidence is high that Firestaff's current startup/menu presentation is visually wrong relative to the chosen local ReDMCSB references.

Confidence is lower on the exact final original full-screen composition because the best local ReDMCSB artifacts available here are still partial/fragmentary rather than one perfectly legible full startup screenshot. That is why this document explicitly treats the chosen ReDMCSB references as the **closest trustworthy local truth**, not as a claim that we already have the fully solved final original screen.
