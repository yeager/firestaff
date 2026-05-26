# Firestaff DM1 Bug & Feature Tracking

**Last updated:** 2026-05-26
**Author:** Daniel Nylander / Bosse

---

## 🔴 Critical Bugs (P0 - blocking)

### 1. Load/Save funkar inte
**Priority:** P0
**Status:** UNVERIFIED
**Details:** F5/F9 save/load wired in main_loop_m11.c, but m11_engine_save_game sends NULL data — no dungeon/champion/event serialization.
**ReDMCSB ref:** F0897_WORLD_Serialize_Compat, LOADSAVE.C
**Affected files:** `src/dm1/dm1_v1_engine_pc34_compat.c`, `src/dm1/dm1_v1_save_load.c`

### 2. Quit-menyn (Escape-tangent) funkar inte som den ska (båda valen avslutar)
**Priority:** P0 — UX-breaking
**Status:** UNDER INVESTIGATION
**Details:** ESC dialog shows YES/NO but both choices dismiss and return to launcher.
**ReDMCSB ref:** REVIVE.C:F0282, COMMAND.C:C160/C161/C162
**Affected files:** `src/engine/m11_game_view.c:6706-6745`

### 3. Klickar man på en Champion mirror och väljer Resurrect så blir det Game Over (alla döda)
**Priority:** P0 — UX-breaking
**Status:** UNDER INVESTIGATION
**Details:** Clicking champion mirror → recruit → panel → RESURRECT → all champions dead → game over.
**ReDMCSB ref:** REVIVE.C:F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel
**Affected files:** `src/engine/m11_game_view.c:6635-6692`
**Root causes to verify:**
  - Cancel vs Confirm path confusion in candidateMirrorPanel handling
  - candidateMirrorPartyIndex index mismatch after recruit
  - F0643_PARTY_ClearChampionSlot clearing wrong slot

### 4. Väggar som inte visas alls (men kartan är korrekt)
**Priority:** P1
**Status:** UNVERIFIED
**Details:** Walls missing from viewport, but map shows correct layout.
**ReDMCSB ref:** DUNVIEW.C, MOVESENS.C, VBLANK.C wall draw path
**Affected files:** `src/dm1/dm1_v1_viewport_3d_pc34_compat.c`, `src/dm1/dm1_v1_draw_primitives_pc34_compat.c`

---

## 🟡 Major Bugs (P1)

### 5. Champion bilder visas mitt i luften
**Priority:** P1
**Status:** UNVERIFIED
**Details:** Champion images rendered at wrong Z-layer, appearing to float.
**ReDMCSB ref:** VBLANK.C draw order, DUNVIEW.C layer ordering
**Affected files:** `src/engine/m11_game_view.c` draw order section

### 6. Champion mirrors visas inte alls
**Priority:** P1
**Status:** UNVERIFIED
**Details:** Champion mirrors not rendered in viewport.
**ReDMCSB ref:** CLIKVIEW.C, GRAPHICS.DAT bitmap indices for mirror objects
**Affected files:** viewport rendering files in `src/dm1/`

### 7. Wall inscriptions är jättesuddiga
**Priority:** P1
**Status:** UNVERIFIED
**Details:** Wall inscription text is very blurry or unreadable.
**ReDMCSB ref:** PANEL.C / DUNVIEW.C inscription blit
**Affected files:** TBD

---

## 🟢 Features (P2)

### 8. Mac app har ingen ikon
**Priority:** P2
**Status:** UNVERIFIED
**Details:** macOS .app bundle has no custom icon.
**Fix:** Add `firestaff.icns` to `firestaff.app/Contents/Resources/`
**Affected files:** `cmake/macosx.cmake` / CMakeLists.txt

---

## 🔍 Investigation Tasks

### 9. Verifiera med ReDMCSB källkoden
**Priority:** P0
**Scope:** For every fix, cite ReDMCSB source as primary evidence before implementation.
**ReDMCSB path:** `/home/trv2/.openclaw/data/firestaff-redmcsb-source/`
**Key files:** COMMAND.C, CLIKVIEW.C, REVIVE.C, DUNVIEW.C, MOVESENS.C, LOADSAVE.C, VBLANK.C, PANEL.C, ENDGAME.C, CHAMPION.C, DEFS.H, GRAPHICS.DAT, DUNGEON.DAT, SOUNDS.DAT, MUSIC.DAT

### 10. Komplett kodgranskning: ReDMCSB vs Firestaff, C-fil per C-fil
**Priority:** P0
**Status:** PLANNED
**Output:** `FIRESTAFF_REDMCSB_AUDIT.md`
**ReDMCSB archive:** `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

### 11. Plan för att integrera CSB stöd i Firestaff
**Priority:** P1
**Status:** PLANNED
**Output:** `CSB_INTEGRATION_PLAN.md`
**Requires:**
  - CSB DAT/BDY file format analysis
  - CSB graphics atlas (16-color planar EGALO, vs DM1 16-color chain)
  - CSB champion system differences
  - CSB map format differences
  - CSB load sequence differences

---

## Subagent Tickets

| Ticket | Scope | Priority |
|---|---|---|
| DM1_V1_ESC_QUIT_DIALOG | ESC quit menu both exits bug | P0 |
| DM1_V1_RESURRECT_GAMEOVER | Champion resurrect → game over | P0 |
| DM1_V1_SAVE_LOAD_FIX | F5/F9 broken save/load | P0 |
| DM1_V1_VIEWPORT_WALLS | Walls not rendering | P1 |
| DM1_V1_DRAW_ORDER | Champion images floating | P1 |
| DM1_V1_MIRROR_RENDER | Champion mirrors invisible | P1 |
| DM1_V1_TEXT_INSCRIPTIONS | Wall inscriptions blurry | P1 |
| DM1_V1_FULL_CODECOMPAR | ReDMCSB full audit | P0 |
| CSB_INTEGRATION_RESEARCH | CSB support plan | P1 |
