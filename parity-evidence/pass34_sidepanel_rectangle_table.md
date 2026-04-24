# Pass 34 — Side-panel component rectangle table (measured)

Last updated: 2026-04-24
Scope: **DM1 / PC 3.4 / English / V1 original-faithful mode** — right-column
side panel (party HUD, action area, spell area, utility strip, etc.).
Viewport is Pass 33.

This is the Pass 34 evidence file required by `PASSLIST_29_36.md` §4.34.
It produces a **measured rectangle table** for every Firestaff side-panel
component, compared against the corresponding ReDMCSB sources
(`PANEL.C`, `CHAMPION.C`, `PORTRAIT.C`, `SPELDRAW.C`, `ACTIDRAW.C`,
`DEFS.H`).  Rows promote to `MATCHED` only when an anchoring constant
is source-backed; otherwise they are `KNOWN_DIFF` with the numeric
delta or `BLOCKED_ON_REFERENCE` when neither pixel capture nor DEFS.H
anchor is available yet.

**Docs + evidence only.  No code changes.**

---

## 1. Firestaff side-panel rectangles (measured from `m11_game_view.c`)

### 1.1 Enum block near line 73

| Component | Firestaff rect `(x, y, w, h)` | Source line |
|---|---|---|
| Viewport (for cross-ref) | (12, 24, 196, 118) | §73–76 |
| Control strip | (14, 165, 88, 14) | §77–80 |
| Prompt strip | (104, 165, 202, 14) | §81–84 |
| Party panel (4 slots, horizontal step 77) | (12, 160, 77 × 4, 28) | §85–89 |
| Party slot (each) | (12 + k*77, 160, 71, 28), k=0..3 | §85–89 |
| Utility panel (Inspect/Save/Load strip) | (218, 28, 86, 42) | §90–93 |
| Utility Inspect button | (222, 56, 22, 10) | §94–97 |
| Utility Save button | (250, 56, 22, 10) | §94–100 |
| Utility Load button | (278, 56, 22, 10) | §94–100 |

### 1.2 DM action-hand icon + action-menu cells

| Component | Firestaff rect `(x, y, w, h)` | Source line |
|---|---|---|
| Action icon cell 0 | (233, 86, 20, 35) | §115–119 |
| Action icon cell step | 22 px between cells | §118 |
| Action menu area (rows 0–2) | (224, 58, 87, 9); row step 11 | §126–130 |
| Action menu area total height | 58 + 2*11 + 9 = 80 | derived |

### 1.3 Action + spell areas (from the `#define` block near line 8410)

| Component | Firestaff rect `(x, y, w, h)` | Source line |
|---|---|---|
| DM action area | (224, 45, 87, 45) | §8408 |
| DM spell area  | (224, 90, 87, 25) | §8410–8413 |

### 1.4 Portraits

| Component | Firestaff value | Source line |
|---|---|---|
| Portrait W × H | 32 × 29 | §6416–6417 |
| Portrait atlas indexing | `srcPX = (pIdx & 7) * 32`, `srcPY = (pIdx >> 3) * 29` | §11670–11671 |

---

## 2. ReDMCSB / DEFS.H anchors available (source-backed)

Extracted from `redmcsb-output/I34E_I34M/DEFS.H` unless noted otherwise:

| Anchor | Value | Reference |
|---|---|---|
| Portrait width | 32 | `G2078_C32_PortraitWidth = 32` (`COORD.C:99`) |
| Portrait height | 29 | `G2079_C29_PortraitHeight = 29` (`COORD.C:100`) |
| Portrait X in atlas (index i) | `(i & 7) * 32` | `M027_PORTRAIT_X` (`DEFS.H:709`) |
| Portrait Y in atlas (index i) | `(i >> 3) * 29` | `M028_PORTRAIT_Y` (`DEFS.H:710`) |
| Champion status box spacing | 69 px | `C69_CHAMPION_STATUS_BOX_SPACING` (`DEFS.H:1756`) |
| Screen width | 320 | `G2071_C320_ScreenPixelWidth` |
| Spell-area background graphic | 87 × 25 | `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND` (see `DEFS.H:1815`) |
| Action-area graphic | 87 × 45 | `C010_GRAPHIC_MENU_ACTION_AREA` (in-code comment §8385) |
| Spell area lines graphic | 14 × 39 | `C011_GRAPHIC_MENU_SPELL_AREA_LINES` (`DEFS.H:1816`) |

Zone ids that confirm the **existence** of a component without pinning
pixel rectangles (ZONES.H was not in the local dump, so the pixel
boundaries of these zones are not yet available at source-anchor
precision):

- `C151..C154_ZONE_CHAMPION_*_STATUS_BOX_NAME_HANDS`
- `C187..C190_ZONE_CHAMPION_*_STATUS_BOX_BAR_GRAPHS`
- `C211..C218_ZONE_SLOT_BOX_*`
- `C221..C272_ZONE_SPELL_AREA_*`
- `C700..C701_ZONE_VIEWPORT_*` (from Pass 33)

---

## 3. Measured drift table (Firestaff vs ReDMCSB)

| Component | Firestaff | ReDMCSB anchor | Delta | Status |
|---|---|---|---|---|
| Portrait W × H | 32 × 29 | 32 × 29 | 0 | `MATCHED` — evidence §2 and m11_game_view.c:6416–6417 |
| Portrait atlas addressing | `(i & 7) * 32, (i >> 3) * 29` | identical | 0 | `MATCHED` — evidence §2 and m11_game_view.c:11670–11671 |
| Champion status-box horizontal spacing | 77 px (`M11_PARTY_SLOT_STEP`) | 69 px (`C69_CHAMPION_STATUS_BOX_SPACING`) | **+8 px per slot** | `KNOWN_DIFF` — Firestaff slot stride is wider than source |
| Champion status-box width | 71 px (`M11_PARTY_SLOT_W`) | not isolated in available sources | — | `BLOCKED_ON_REFERENCE` (no DEFS.H pixel-width constant for the box itself) |
| Champion status-box height | 28 px (`M11_PARTY_SLOT_H`) | — | — | `BLOCKED_ON_REFERENCE` |
| DM action area | (224, 45, 87, 45) | Graphic 10 = 87×45 per comment in m11_game_view.c §8385; X/Y not in DEFS.H | w/h match graphic | `MATCHED` (w,h) / `BLOCKED_ON_REFERENCE` (x,y) |
| DM spell area backdrop | (224, 90, 87, 25) | Graphic 9 = 87×25 per DEFS.H:1815 | w/h match graphic | `MATCHED` (w,h) / `BLOCKED_ON_REFERENCE` (x,y) |
| Spell-area lines graphic size | — (Firestaff uses `M11_GFX_SPELL_AREA_LINES = 11`) | 14 × 39 per DEFS.H:1816 | — | `MATCHED` (asset index) / `BLOCKED_ON_REFERENCE` (placement) |
| Action icon cell W × H | 20 × 35 | not isolated | — | `BLOCKED_ON_REFERENCE` |
| Action icon cell step | 22 | not isolated | — | `BLOCKED_ON_REFERENCE` |
| Action menu row step | 11 | not isolated | — | `BLOCKED_ON_REFERENCE` |
| Utility strip (Inspect / Save / Load) | (218, 28, 86, 42) | — | — | `BLOCKED_ON_REFERENCE` — no DEFS.H anchor; this strip is a Firestaff-invented UI chrome row, already flagged in parity matrix for later audit |
| Control strip | (14, 165, 88, 14) | — | — | `BLOCKED_ON_REFERENCE` |
| Prompt strip | (104, 165, 202, 14) | — | — | `BLOCKED_ON_REFERENCE` |
| Party panel origin | (12, 160) | — | — | `BLOCKED_ON_REFERENCE` |

## 4. Asset usage map per component (GRAPHICS.DAT)

From the in-code references and comments in `m11_game_view.c`:

| Component | Firestaff graphic index | ReDMCSB reference |
|---|---|---|
| Panel empty backdrop | 20 | `C020_GRAPHIC_PANEL_EMPTY` (DEFS.H:1780) |
| Champion portraits | 26 | `C026_GRAPHIC_CHAMPION_PORTRAITS` (referenced in DUNVIEW.C) |
| Spell-area background | 9 | `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND` (DEFS.H:1815) |
| Spell-area lines | 11 | `C011_GRAPHIC_MENU_SPELL_AREA_LINES` (DEFS.H:1816) |
| Action area | 10 | `C010_GRAPHIC_MENU_ACTION_AREA` (referenced §8385 in m11_game_view.c) |
| Slot box normal | 33 | referenced in probe INV_GV_201 (18×18) |
| Slot box wounded | 34 | referenced in probe INV_GV_202 (18×18) |
| Slot box acting-hand | 35 | referenced in probe INV_GV_203 (18×18) |
| Status box frame | 7 | referenced in probe INV_GV_205 (67×29) |

All of these asset indices are used at their **source-faithful dimensions**
per the M11 game-view probe's asset-loading invariants
(`INV_GV_201..INV_GV_205`), which is one of the few component rows we
can promote to `MATCHED` for asset identity.  Placement-level parity
still requires overlay frames from ReDMCSB — blocked as above.

## 5. Text-vs-graphics flag list (feeds Pass 35)

Components with text in the Firestaff viewport panel that ReDMCSB renders
as graphics (needs pass-35 audit to finalise):

| Component | Current V1 behavior (observed from `m11_game_view.c`) | Text vs graphic? |
|---|---|---|
| Utility panel labels (INSPECT / SAVE / LOAD) | Rendered as in-engine text | **TEXT** — no ReDMCSB equivalent (Firestaff-invented UI chrome; flag for Pass 35) |
| Control strip | Rendered as in-engine text | **TEXT** — Firestaff-invented chrome; flag for Pass 35 |
| Prompt strip | Rendered as in-engine text | **MIXED** — DM1 panel shows similar prompts via text engine; needs Pass 35 TEXT.C cross-ref |
| Party HUD champion name | Rendered as text | **TEXT** — matches source (CHAMPION.C uses text engine for name) |
| Party HUD HP/stamina/mana values | Rendered as text | **TEXT** — matches source (bar-graphs zone C187–C190) |
| Status / inspect text readout | Rendered as text in `state->inspectTitle`/`inspectDetail` | **TEXT** — Firestaff-specific inspect overlay; flag for Pass 35 |

This table is the **seed input** for Pass 35 (typography / text-vs-graphics
audit), as required by `PASSLIST_29_36.md` §4.34.7.

## 6. Status promotion / demotion proposed for `PARITY_MATRIX_DM1_V1.md`

| Row | Before Pass 34 | After Pass 34 | Reason |
|---|---|---|---|
| Champion portraits | `UNPROVEN` | `MATCHED` | 32×29 and atlas addressing equal between Firestaff and ReDMCSB; evidence §2/§3 |
| Party/champion region | `UNPROVEN` | `KNOWN_DIFF` (+8 px/slot stride vs DEFS.H C69) | numeric delta §3 |
| Equipment/item icons | `UNPROVEN` | `UNPROVEN` (unchanged) — asset identity verified by M11 probe but placement overlay blocked | |
| Panel backgrounds/ornaments | `UNPROVEN` | `MATCHED` (asset identity) / `BLOCKED_ON_REFERENCE` (placement) — keep existing UNPROVEN label in matrix row but split note | |
| Spell panel | `UNPROVEN` | `MATCHED` (asset identity for graphics 9 and 11) / `BLOCKED_ON_REFERENCE` (placement) | asset size matches DEFS.H §2 |

The matrix is updated in the same commit that lands this file.

## 7. Honest scope note

Pass 34 produces measured component rectangles and asset-identity
`MATCHED` rows.  **No placement row is promoted to `MATCHED` without a
pixel anchor from DEFS.H / ReDMCSB source.**  That is the single biggest
honesty constraint for this pass and we keep it.

Components whose rectangles remain `BLOCKED_ON_REFERENCE` are flagged
here so the pass-37+ workstream has a single ledger of exactly which
pixel overlays are still missing.  Landing them does not require code
changes, only a ReDMCSB-side capture path or a ZONES.H / PANEL.C pixel
walk.

**This pass alone does not complete DM1/V1 parity.**
