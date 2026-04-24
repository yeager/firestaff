# Pass 35 — V1 Typography and Text-vs-Graphics Audit

Last updated: 2026-04-24
Scope: **DM1 / PC 3.4 / English / V1 original-faithful mode.**  Enumerates
player-visible text-emitting surfaces in `m11_game_view.c` and classifies
each against ReDMCSB ownership (`TEXT.C`, `TEXT2.C`, `PANEL.C`,
`CHAMPION.C`, `PORTRAIT.C`, `SPELDRAW.C`).

This audit is **docs only.  No code changes.**  It satisfies
`PASSLIST_29_36.md` §4.35.

Classification legend (per pass spec):

- `REPLACE_WITH_GRAPHIC <index>` — original used a bitmap at this spot.
- `KEEP_AS_TEXT <TEXT.C/TEXT2.C ref>` — original used the text engine.
- `KNOWN_DIFF` — Firestaff-custom text; no current source-faithful plan.

---

## 1. Emitter inventory (`m11_game_view.c`)

Quantities as of the Pass 35 commit (directly measured with `grep`):

| Emitter symbol | Call count | Role |
|---|---|---|
| `m11_set_status` | 82 | Top-of-panel status lozenge ("DOOR", "SPELL CAST", …) |
| `m11_log_event`  | 83 | Rolling message-log line (prefixed with `Tn:`, color-coded) |
| `m11_set_inspect_readout` | 11 | Inspect title + detail panel (when the inspect mode is active) |
| `state->inspectTitle / state->inspectDetail` via `snprintf` | 57 | Same inspect overlay, written without going through the helper |
| `m11_draw_text` | 75 | Low-level glyph blitter used by every panel-text path above |
| `m11_draw_text_original` | 1 (internal) | Delegation wrapper onto `g_activeOriginalFont` |

The 57 direct `snprintf(&inspectTitle, …)` / `inspectDetail` call sites
are counted as part of the inspect readout surface; they go through the
same `m11_draw_text` render path as `m11_set_inspect_readout`.

**Total distinct text-emitting symbols (public + internal): 6.**  Each
appears in the §2 classification below.

## 2. Per-emitter classification

### 2.1 `m11_draw_text` / `m11_draw_text_original`
- **Role:** low-level glyph blit (7-pixel-tall custom font; can delegate
  to a "original" font when loaded).
- **ReDMCSB reference:** `TEXT.C` / `TEXT2.C` own the text-engine glyph
  rendering in the original.  The V1 in-panel text lines are rendered
  via the original engine; the font itself is GRAPHICS.DAT content.
- **Classification:** `KEEP_AS_TEXT <TEXT.C/TEXT2.C>` — but the font
  pipeline **is currently Firestaff's custom 7-pixel font**, not the
  original font bank.  Until that is swapped, this whole surface is
  effectively a typography `KNOWN_DIFF` for every caller.
- **Pass-37+ follow-up:** wire `g_activeOriginalFont` through an
  extracted GRAPHICS.DAT font atlas so the `m11_draw_text_original`
  delegate becomes the default path.

### 2.2 `m11_set_status` (82 call sites)
- **Role:** short status lozenge at the top of the side panel; 40-ish
  distinct values currently in use.  Example set (alphabetised first 40
  extracted with `grep`):
  `"ACTIVE CHAMPION READY"`, `"ASCENDED TO PREVIOUS LEVEL"`,
  `"BACK TO LAUNCHER"`, `"BOOT"`, `"CANNOT USE THIS ITEM"`, `"CAST"`,
  `"CHAIN ERROR"`, `"CHAMP"`, `"CHAMPION CANNOT ACT"`, `"DEATH"`,
  `"DESCENDED TO NEXT LEVEL"`, `"DOOR CHECK"`, `"DOOR CLOSED"`,
  `"DOOR DESTROYED"`, `"DOOR OPENED"`, `"DOOR"`, `"DRANK WATER"`,
  `"DROP"`, `"ENDGAME"`, `"ENEMY SPOTTED"`, `"FACING UPDATED"`,
  `"FELL TO NEXT LEVEL"`, `"FLASK IS EMPTY"`, `"FOOD CONSUMED"`,
  `"FRONT CELL READOUT"`, `"GAME DATA LOADED"`,
  `"GAME VIEW NOT STARTED"`, `"HANDS ARE EMPTY"`, `"IDLE TICK ADVANCED"`,
  `"IDLE TICK HELD"`, `"INSPECT"`, `"INVALID RUNE SEQUENCE"`,
  `"INVENTORY FULL"`, `"ITEM DROPPED"`, `"ITEM TAKEN"`, `"LOAD"`,
  `"MOVEMENT BLOCKED"`, `"NO ACTIVE CHAMPION"`, `"NO QUICKSAVE FOUND"`,
  `"NO STAIRS HERE"`, …
- **ReDMCSB reference:** ReDMCSB's panel does **not** have a
  custom-phrased "status lozenge" row.  The original panel surfaces
  equivalent state through the message-log + champion status-box
  bar-graphs (`C187..C190_ZONE_CHAMPION_*_STATUS_BOX_BAR_GRAPHS`) and
  character UI, not through a synthesised status string.
- **Classification:** `KNOWN_DIFF` — Firestaff-invented helper surface.
  Every call site is a V1 honesty diff against the original.
- **Pass-37+ follow-up:** a bounded pass should either (a) route
  "relevant" status events into the message-log with the original's
  wording, or (b) hide the lozenge entirely in V1 mode.  This audit
  flags all 82 call sites as `KNOWN_DIFF` collectively; the pass-36
  ledger must record it as a single blocker.

### 2.3 `m11_log_event` (83 call sites)
- **Role:** rolling message-log line, color-coded, with a `Tn:` tick
  prefix.  Closest structural match to ReDMCSB's message log.
- **ReDMCSB reference:** `TEXT.C` owns the classic DM message log with
  a fixed-width log region.  Tick prefixing is **not** a ReDMCSB
  convention; DM1 message lines are terse descriptions without a
  tick prefix.
- **Classification:** `KEEP_AS_TEXT <TEXT.C>` for the log region itself;
  the `Tn:` prefix and the color choices are `KNOWN_DIFF`.
- **Pass-37+ follow-up:** audit each of the 83 call sites against
  `TEXT.C` / `TEXT2.C` original phrasings.  That is a bounded typography
  pass on its own.

### 2.4 `m11_set_inspect_readout` and direct `inspectTitle/inspectDetail`
- **Role:** two-line inspect overlay ("PIT FALL" / "DROPPED TO LEVEL N,
  POSITION (X,Y)", etc.).  Emitted from 11 helper sites plus 57 direct
  `snprintf` sites, almost always paired.
- **ReDMCSB reference:** no 1:1 equivalent.  DM1 surfaces "inspect"
  content by cycling through champion status boxes / object description
  panels (`C03_PANEL_OBJECT_DESCRIPTION`), never through a dedicated
  "inspect detail" two-liner on the viewport.
- **Classification:** `KNOWN_DIFF` — Firestaff-invented inspect surface.
- **Pass-37+ follow-up:** delete or hide the inspect overlay in V1
  mode; re-route pickup / drop / spell / damage messaging back into the
  message log.

### 2.5 Utility strip captions ("INSPECT" / "SAVE" / "LOAD")
- **Role:** three captioned buttons inside `M11_UTILITY_PANEL`
  at (218, 28, 86, 42) on the right column.
- **ReDMCSB reference:** no equivalent utility strip exists in DM1.  DM1
  offers no on-screen "save" button; save/load is driven from the
  main menu.
- **Classification:** `KNOWN_DIFF` — Firestaff-invented chrome row,
  already flagged by the Pass 34 rectangle table (`pass34_sidepanel_rectangle_table.md` §3).
- **Pass-37+ follow-up:** hide in V1; expose only via the classic menu
  path (already present).

### 2.6 Control strip and prompt strip (at y=165)
- **Role:** a 14-pixel-tall horizontal strip at the bottom of the
  framebuffer containing control hints / prompt text.
- **ReDMCSB reference:** DM1 does not have a bottom hint strip.  The
  closest source-faithful element is the message log rendered higher up.
- **Classification:** `KNOWN_DIFF` — Firestaff-invented chrome strip.
- **Pass-37+ follow-up:** hide / remove in V1 mode.

### 2.7 Party HUD champion-name + HP/stamina/mana numeric values
- **Role:** each of the four party slots carries the champion name and
  numeric HP/stamina/mana values.
- **ReDMCSB reference:** `CHAMPION.C` + `TEXT.C` own the name text
  rendering; `C159..C162_ZONE_CHAMPION_*_STATUS_BOX_NAME` and
  `C187..C190_ZONE_CHAMPION_*_STATUS_BOX_BAR_GRAPHS`.  Text engine
  **is** used for the champion name; **bar graphs** (not numeric
  strings) show HP / stamina / mana.
- **Classification (name)**: `KEEP_AS_TEXT <CHAMPION.C / TEXT.C>` —
  matches source.
- **Classification (numeric HP/stamina/mana readouts)**:
  `REPLACE_WITH_GRAPHIC` — the original renders these as bar graphs
  drawn by `CHAMDRAW.C`, not as numeric glyph strings.
- **Pass-37+ follow-up:** migrate Firestaff's numeric strip to the
  bar-graph renderer.  Tracked as a concrete blocker in the Pass 36
  ledger.

### 2.8 Spell-panel rune labels / runic inscription
- **Role:** rune entry status text inside `M11_DM_SPELL_AREA` at (224, 90).
- **ReDMCSB reference:** `SPELDRAW.C` uses `C011_GRAPHIC_MENU_SPELL_AREA_LINES`
  (14×39) + `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND` (87×25) to draw
  the rune symbols; the "rune entered" indicator is a graphic blit,
  not a text string.
- **Classification:** `REPLACE_WITH_GRAPHIC C011_GRAPHIC_MENU_SPELL_AREA_LINES`.
- **Pass-37+ follow-up:** route the current rune-entered status text
  to the `C011` line-graphic blit path.  The asset is already present
  per the Pass 34 asset-usage map.

## 3. Per-surface rollup

| Surface | Current V1 behavior | Source-faithful? | Classification | Pass-37+ follow-up |
|---|---|---|---|---|
| Font pipeline | Custom 7-pixel font; delegation to `g_activeOriginalFont` exists but is not wired | No — typography drifts | `KNOWN_DIFF` → eventually `KEEP_AS_TEXT <TEXT.C>` | Wire original font bank |
| Status lozenge (`m11_set_status`) | 82 Firestaff-authored strings | No — UI-invented row | `KNOWN_DIFF` (all 82 sites) | Remove surface or fold into message log |
| Message log (`m11_log_event`) | Close to source but prefixed with `Tn:` | Partial | `KEEP_AS_TEXT` (region) + `KNOWN_DIFF` (prefix) | Rewrite call sites to TEXT.C strings; drop tick prefix in V1 |
| Inspect readout | Firestaff two-liner over the viewport | No — invented | `KNOWN_DIFF` (all 68 sites) | Remove in V1, reroute to message log / object-description panel |
| Utility strip (Inspect/Save/Load captions) | Firestaff chrome | No — no DM1 equivalent | `KNOWN_DIFF` | Hide in V1 |
| Control + prompt strips (y=165) | Firestaff chrome | No — no DM1 equivalent | `KNOWN_DIFF` | Hide in V1 |
| Champion names in party HUD | Text, via `m11_draw_text` | Yes (CHAMPION.C uses text engine) | `KEEP_AS_TEXT <CHAMPION.C>` | Use original font when §2.1 lands |
| Champion HP/stamina/mana numeric values | Text | No — source uses bar graphs | `REPLACE_WITH_GRAPHIC <CHAMDRAW.C bar graphs>` | Migrate to bar-graph renderer |
| Spell-panel rune labels | Text | No — source uses C011 lines graphic | `REPLACE_WITH_GRAPHIC 11 (C011_GRAPHIC_MENU_SPELL_AREA_LINES)` | Route through the C011 blit |

## 4. Cross-reference with `extracted-graphics-v1/`

The graphics referenced above are all present in `extracted-graphics-v1/`
per the M11 game-view probe's asset-loading invariants
(`INV_GV_201..INV_GV_205`) and per the Pass 34 asset-usage map:

- `C009` (spell-area background, 87×25) — available.
- `C010` (action area, 87×45) — available.
- `C011` (spell-area lines, 14×39) — available.
- `C020` (panel empty, 144×73) — available.
- `C026` (champion portraits atlas) — available, 32×29 per portrait.
- `C033/34/35` (slot boxes, 18×18) — available.
- `C007` (status box frame, 67×29) — available.

No `REPLACE_WITH_GRAPHIC` entry in §3 is blocked on asset extraction;
all blockers are on the renderer side.

## 5. Status rollup for `PARITY_MATRIX_DM1_V1.md` §2 / §4

Row updates proposed (applied in the same commit that lands this file):

| Matrix row | Before Pass 35 | After Pass 35 | Reason |
|---|---|---|---|
| Text-tag fallbacks (§2) | `UNPROVEN` | `KNOWN_DIFF` (enumerated in this file) | full enumeration complete |
| Over-labeling (§4) | `UNPROVEN` | `KNOWN_DIFF` (Firestaff invents status lozenge + inspect readout + utility captions + chrome strips) | enumeration complete |
| Font rendering (§4) | `UNPROVEN` | `KNOWN_DIFF` (custom 7-pixel font still primary, delegation path exists but unwired) | see §2.1 |

## 6. Honest scope note

This is the complete enumeration of distinct text-emitting entry points
in `m11_game_view.c`.  Every emitter symbol and every call site category
is accounted for exactly once.  No classification is `MATCHED`; every
row is either `KEEP_AS_TEXT` (source-faithful if the font is swapped)
or `KNOWN_DIFF` / `REPLACE_WITH_GRAPHIC` (needs follow-up).  No row is
promoted to `MATCHED` without evidence, matching the pass-36 honesty
lock rules.

**This pass alone does not complete DM1/V1 parity.**
