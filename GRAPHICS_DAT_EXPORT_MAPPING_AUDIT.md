# DM1 PC 3.4 GRAPHICS.DAT export and mapping audit

Date: 2026-04-23

## Scope and goal

This is a bounded audit of the current Firestaff DM1 PC 3.4 `GRAPHICS.DAT` export/mapping workflow, focused on whether the exported original-reference assets and their semantic index bindings are trustworthy enough for comparison work.

Priority audit targets:

- `0000`
- `0007`, `0008`
- `0009`, `0010`
- `0020`
- `0033`, `0034`, `0035`
- `0078`, `0079`
- one bounded ornate-lock family pair: `0303`, `0304`

Hard rule used in this audit: when Firestaff disagrees with Greatstone/DMExtract/ReDMCSB without strong contrary evidence, Firestaff is treated as wrong.

## Current local export/mapping pipeline summary

### Export path

Current original exports come from `extraction-tools/extract_all_graphics.py`.

That script:

1. reads the real DM1 PC 3.4 `GRAPHICS.DAT`
   - source: `/Users/bosse/.openclaw/data/redmcsb-original/GRAPHICS.DAT`
   - SHA-256: `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
2. parses the format-1 header directly
3. classifies entries by metadata (`bitmap`, `placeholder`, `special`, etc.)
4. exports bitmap entries through the ReDMCSB-based visible-dispatch/original-falsecolor probes
5. writes:
   - `extracted-graphics-v1/pgm/*.pgm`
   - `extracted-graphics-v1/ppm-falsecolor/*.ppm`
   - `extracted-graphics-v1/manifest.json`
6. adds only very light category grouping (`title-ui`, `walls-ornate`, `unclassified`)

For the current extracted set, the manifest reports:

- 713 total indices
- 577 bitmap exports
- 4 non-bitmap
- 131 placeholders
- 1 special
- 0 decode failures

### Mapping path

The export step itself is mostly index-preserving and low-risk. The bigger risk is the **downstream semantic mapping layer**: docs, code comments, production briefs, and especially the comparison-PDF generator.

Current PDF generation is driven by `tools/build_original_vs_4k_asset_pdf.py`, which hard-codes pairings such as:

- `0000` -> "Viewport frame base"
- `0007` -> "Status box left frame"
- `0008` -> "Status box right frame"
- `0034` -> "Party HUD cell highlight"

Those labels are not all supported by ReDMCSB/DMExtract/Greatstone evidence.

## Sources used for this audit

Primary sources:

1. local extracted originals in `extracted-graphics-v1/`
2. `extracted-graphics-v1/manifest.json`
3. ReDMCSB/DMExtract-derived constants and usage in:
   - `dm7z-extract/Toolchains/Common/Source/DEFS.H`
   - `firestaff_extracted_frontends_probe.c`
   - `m11_game_view.c`
   - `probes/m11/firestaff_m11_game_view_probe.c`
4. existing PC identity lock notes that capture the DMExtract/Greatstone agreement for the ornate lock family:
   - `redmcsb_pc34_identity_locked_final_note_2026-04-18.md`
   - `redmcsb_pc34_lock_tail_summary_2026-04-18.md`

## Bounded audit by index/family

### `0000`

- local export: bitmap, `224x136`
- local visual read: large viewport-sized frame/dialog shell
- ReDMCSB/DEFS name: `C000_GRAPHIC_DIALOG_BOX`
- Firestaff probe comment currently says: `viewport background graphic 0 loads as 224x136`

Assessment:

- The **exported bitmap itself looks plausible and stable**.
- The **current semantic label is not trustworthy as written**. ReDMCSB names this as dialog-box graphics, not a generic viewport-frame base.
- The current PDF uses `0000` as the original source for a "Viewport frame base" page. That is too strong a claim for the evidence available.

Verdict: **suspicious mapping**.

### `0007` / `0008`

- local exports: both bitmap, `67x29`
- local visual read:
  - `0007` = normal/alive champion status box shell
  - `0008` = dead champion status box shell
- ReDMCSB/DEFS names:
  - `C007_GRAPHIC_STATUS_BOX`
  - `C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION`
- Firestaff PDF labels them as left/right frames

Assessment:

- The export sizes and visuals match the alive/dead interpretation.
- The PDF left/right interpretation is wrong.
- ReDMCSB also comments that graphic `0007` is "never used" in original code, but that does **not** change its identity; it only affects strict usage claims.

Verdict:

- `0007`: **confirmed-correct bitmap export, confirmed-wrong left/right mapping**
- `0008`: **confirmed-correct bitmap export, confirmed-wrong left/right mapping**

### `0009`

- local export: bitmap, `87x25`
- local visual read: spell-area background strip
- ReDMCSB/DEFS name: `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND`
- Firestaff usage comments also align with spell-area background

Assessment:

- Local export, dimensions, and ReDMCSB usage all agree.
- This is one of the safer source references in the current workflow.

Verdict: **confirmed-correct mapping**.

### `0010`

- local export: bitmap, `87x45`
- local visual read: action-area menu strip
- ReDMCSB/DEFS name: `C010_GRAPHIC_MENU_ACTION_AREA`
- Firestaff usage comments align

Assessment:

- Local export, dimensions, and ReDMCSB usage agree.
- This is also safe for future comparison work.

Verdict: **confirmed-correct mapping**.

### `0020`

- local export: bitmap, `144x73`
- ReDMCSB/DEFS name: `C020_GRAPHIC_PANEL_EMPTY`
- Firestaff extracted frontend probe uses `C020_GRAPHIC_PANEL_EMPTY` for the panel area repeatedly
- Firestaff game-view comments also describe this as the empty panel/inventory panel background

Assessment:

- The identity is supported by both symbolic naming and usage.
- No conflicting evidence found in the bounded audit.

Verdict: **confirmed-correct mapping**.

### `0033` / `0034` / `0035`

- local exports: all bitmap, `18x18`
- ReDMCSB/DEFS names:
  - `C033_GRAPHIC_SLOT_BOX_NORMAL`
  - `C034_GRAPHIC_SLOT_BOX_WOUNDED`
  - `C035_GRAPHIC_SLOT_BOX_ACTING_HAND`
- Firestaff extracted frontend logic selects exactly these meanings when drawing inventory slot boxes
- current PDF uses:
  - `0033` as standard cell base
  - `0034` as generic highlight overlay
  - `0035` is not represented

Assessment:

- The family identity is well supported.
- `0034` is not a generic highlight overlay; it is specifically the wounded slot-box variant.
- `0035` is the acting-hand variant and should be kept distinct from both normal and wounded.

Verdict:

- `0033`: **confirmed-correct mapping**
- `0034`: **confirmed-wrong mapping in current PDF/script**
- `0035`: **confirmed-correct mapping**

### `0078` / `0079`

- local exports:
  - `0078` bitmap, `224x97`
  - `0079` bitmap, `224x39`
- current Firestaff probe comments say:
  - `0078` = ceiling panel
  - `0079` = floor panel
- bounded visual read of the exported originals strongly indicates the reverse:
  - `0078` reads as the large receding **floor** field
  - `0079` reads as the narrower upper **ceiling** band

Assessment:

- The local bitmap exports themselves look valid.
- The current Firestaff floor/ceiling naming appears reversed.
- Because this audit is bounded and I did not complete a deeper ReDMCSB draw-path trace for the exact pair, I am not marking the pair as fully settled beyond the reversal suspicion.
- However, based on the hard rule for disagreements and the visual evidence, the current Firestaff naming should **not** be trusted.

Verdict:

- `0078`: **suspicious in Firestaff; likely mislabeled as ceiling**
- `0079`: **suspicious in Firestaff; likely mislabeled as floor**

### `0303` / `0304` ornate-lock family sample

- local exports:
  - `0303` bitmap, `16x19`
  - `0304` bitmap, `32x28`
- local visual read:
  - `0303` = left-side/edge lock element
  - `0304` = front-facing ornate lock plate
- Greatstone/DMExtract agreement already recorded locally:
  - `303/304` = `Wall Ornate 22`, `Stone Lock`, `Left Side / Front`
- ReDMCSB lock-family notes also align with this interpretation

Assessment:

- This family sample is strongly locked.
- The export appears correct and the left-side/front semantic split is trustworthy.

Verdict:

- `0303`: **confirmed-correct mapping**
- `0304`: **confirmed-correct mapping**

## Bucketed results

### Confirmed-correct mappings

- `0009` spell area background
- `0010` action area
- `0020` empty panel / panel empty
- `0033` slot box normal
- `0035` slot box acting hand
- `0303` stone lock left side
- `0304` stone lock front

### Suspicious mappings

- `0000` currently treated as a viewport-frame base; evidence supports only a broader dialog-box/frame identity
- `0078` currently treated in Firestaff as ceiling; likely floor
- `0079` currently treated in Firestaff as floor; likely ceiling

### Confirmed-wrong mappings

- `0007` labeled in the comparison PDF/script as a left status-box frame; actual identity is alive/normal status box
- `0008` labeled in the comparison PDF/script as a right status-box frame; actual identity is dead-champion status box
- `0034` labeled in the comparison PDF/script as a generic highlight overlay; actual identity is wounded slot-box variant

## Verdict on the current original-vs-4K comparison PDF

Current file: `docs/reports/original-vs-v2-4k-asset-comparison.pdf`

Verdict: **invalid**.

Why:

1. multiple pages rely on **confirmed-wrong source mappings** (`0007`, `0008`, `0034`)
2. `0000` is used with a stronger semantic claim than the audit supports
3. the PDF generator hard-codes semantic pairings without a locked reference table
4. once source bindings are wrong, the PDF stops being a trustworthy original-vs-4K reference even if the rendered images themselves are real

Most clearly affected PDF pages:

- "Viewport frame base" / "Viewport aperture / inner mask" because they depend on the unsettled `0000` semantic claim
- "Status box left frame" because `0007` is not a left-frame asset
- "Status box right frame" because `0008` is not a right-frame asset
- "Party HUD cell highlight" because `0034` is a wounded-slot variant, not a generic highlight overlay

The PDF may still contain some visually useful pages, but it should **not** be used as an authoritative mapping/reference artifact.

## Compact trusted reference list for future comparison work

Use this list as the safe starting set for future original-reference comparisons:

| Index | Trusted label | Evidence floor |
| --- | --- | --- |
| `0009` | spell area background | ReDMCSB `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND`, local export size/visual, Firestaff usage comments |
| `0010` | action area background | ReDMCSB `C010_GRAPHIC_MENU_ACTION_AREA`, local export size/visual, Firestaff usage comments |
| `0020` | panel empty / inventory-panel background | ReDMCSB `C020_GRAPHIC_PANEL_EMPTY`, repeated frontend usage, matching local export |
| `0033` | slot box normal | ReDMCSB `C033_GRAPHIC_SLOT_BOX_NORMAL`, frontend slot-box selection logic |
| `0035` | slot box acting hand | ReDMCSB `C035_GRAPHIC_SLOT_BOX_ACTING_HAND`, frontend slot-box selection logic |
| `0303` | stone lock, left side | DMExtract/Greatstone agreement captured in local PC lock notes, matching local visual |
| `0304` | stone lock, front | DMExtract/Greatstone agreement captured in local PC lock notes, matching local visual |

Use with caution / re-verify first:

- `0000` — dialog-box/frame family; do not call it a viewport-frame base without a tighter usage lock
- `0078` / `0079` — re-lock floor/ceiling naming before using them in reference docs

Do not reuse the current mapping claims for:

- `0007` as left frame
- `0008` as right frame
- `0034` as generic highlight overlay

## Short checklist for rebuilding a trustworthy replacement PDF

1. build a small locked source table first
   - index
   - exported file
   - trusted label
   - evidence source
2. only include pages whose index identity is either confirmed-correct or explicitly marked provisional
3. for provisional entries, print `PROVISIONAL` on-page
4. do not infer semantics from Firestaff asset naming alone
5. prefer ReDMCSB/DMExtract/Greatstone agreement over local convenience labels
6. keep alive/dead, normal/wounded/acting-hand, and left-side/front families separate
7. re-audit `0078`/`0079` before any floor/ceiling comparison pages
8. regenerate the PDF only from that locked table, not from ad hoc hard-coded pairings

## Bottom line

The bounded audit does **not** show a broad failure of the raw export pipeline for the audited bitmap entries. The more serious problem is the **semantic mapping layer after export**.

So the working conclusion is:

- audited original bitmap exports are mostly usable
- several current semantic bindings are not
- the existing comparison PDF should be treated as **invalid** until rebuilt from a locked reference table
