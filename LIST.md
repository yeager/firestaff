# Firestaff DM1 Bug & Feature Tracking

**Last updated:** 2026-05-26 21:31 GMT+2
**Author:** Daniel Nylander / Bosse

---

## ✅ Completed Items

### 1. Load/Save funkar inte
**Priority:** P0
**Status:** ✅ LIKELY WORKING
**Details:** F5/F9 use DM1_SaveGame/DM1_LoadGameWithBackup directly, which properly serialize the world via F0897_WORLD_Serialize_Compat. The m11_engine_save_game stub (passes NULL) has zero callers and is not used by F5/F9.
**Note:** Verify with runtime test - play game, press F5, verify save file size > 0, restart, press F9, verify state restored.

### 2. Quit-menyn (Escape-tangent) funkar inte som den ska (båda valen avslutar)
**Priority:** P0 — UX-breaking
**Status:** ✅ FIXED — commit 98f9295f
**Details:** ESC dialog YES/NO click hit zones (hardcoded hy=67/104) didn't match drawn button positions (choiceY=109). Added dedicated hit-zone computation for returnToMenuConfirmActive + dialogChoiceCount==2 using actual drawn layout.
**ReDMCSB ref:** CLIKVIEW.C F0378/F0379/F0380
**Affected files:** `src/engine/m11_game_view.c` m11_dialog_choice_at_point

### 3. Klickar man på en Champion mirror och väljer Resurrect så blir det Game Over (alla döda)
**Priority:** P0 — UX-breaking
**Status:** ✅ FIXED — commit 1a186fdc
**Details:** candidateMirrorPartyIndex stored snapshot at recruit time became stale when party->championCount changed. Changed to party->championCount - 1 (ReDMCSB G0305 pattern).
**ReDMCSB ref:** REVIVE.C F0282, COMMAND.C C160/C161/C162
**Affected files:** `src/engine/m11_game_view.c` ConfirmMirrorCandidate/CancelMirrorCandidate

### 9. Verifiera med ReDMCSB källkoden
**Priority:** P0
**Status:** ✅ DONE — commit 483781c3
**Output:** `DM1_REDMSB_CODECOMPAR.md` (code comparison matrix)

### 11. Plan för att integrera CSB stöd i Firestaff
**Priority:** P1
**Status:** ✅ DONE — commit b604d20c
**Output:** `CSB_INTEGRATION_PLAN.md` (361 lines)

---

## 🔴 Remaining Items (P1/P2 - moved to TODO.md)

### 4. Väggar som inte visas alls (men kartan är korrekt)
**Priority:** P1 — Need runtime capture before fixing
**Details:** Walls missing from viewport, but map shows correct layout.
**ReDMCSB ref:** DUNVIEW.C, MOVESENS.C, VBLANK.C wall draw path
**Affected files:** `src/dm1/dm1_v1_viewport_3d_pc34_compat.c`, `src/dm1/dm1_v1_draw_primitives_pc34_compat.c`

### 5. Champion bilder visas mitt i luften
**Priority:** P1
**Details:** Champion images rendered at wrong Z-layer, appearing to float.
**ReDMCSB ref:** VBLANK.C draw order, DUNVIEW.C layer ordering
**Affected files:** `src/engine/m11_game_view.c` draw order section

### 6. Champion mirrors visas inte alls
**Priority:** P1
**Details:** Champion mirrors not rendered in viewport.
**ReDMCSB ref:** CLIKVIEW.C, GRAPHICS.DAT bitmap indices for mirror objects
**Affected files:** viewport rendering files in `src/dm1/`

### 7. Wall inscriptions är jättesuddiga
**Priority:** P1
**Details:** Wall inscription text is very blurry or unreadable.
**ReDMCSB ref:** PANEL.C / DUNVIEW.C inscription blit
**Affected files:** TBD

### 8. Mac app har ingen ikon
**Priority:** P2
**Details:** macOS .app bundle has no custom icon.
**Fix:** Add `firestaff.icns` to `firestaff.app/Contents/Resources/`
**Affected files:** `cmake/macosx.cmake` / CMakeLists.txt

---

## CSB V1 Status

**Started:** 2026-05-26 21:31 GMT+2
**Plan:** See `CSB_INTEGRATION_PLAN.md`
**Existing code:** 13 header + 13 source files in `src/csb/` and `include/csb*`
**Key existing modules:**
- `csb_v1_character_pc34_compat.c/h` — champion system (4 champions imported from DM1)
- `csb_v1_dungeon_loader_pc34_compat.c/h` — dungeon loading
- `csb_v1_dungeon_world_pc34_compat.c/h` — dungeon world
- `csb_v1_game.c/h` — game logic
- `csb_v1_save_load_pc34_compat.c/h` — save/load
- `csb_v1_viewport_pc34_compat.c/h` — viewport
- `csb_v1_chaos_magic_pc34_compat.c/h` — chaos magic
- Plus v2 modules: chaos_enhanced, minimap, smooth_movement, viewport_renderer

**Remaining:** CSB V1 runtime handoff (menu → game loop)

---

## Subagent Tickets

| Ticket | Scope | Priority | Status |
|---|---|---|---|
| DM1_V1_ESC_QUIT_DIALOG | ESC quit menu both exits bug | P0 | ✅ DONE |
| DM1_V1_RESURRECT_GAMEOVER | Champion resurrect → game over | P0 | ✅ DONE |
| DM1_V1_SAVE_LOAD_FIX | F5/F9 save/load | P0 | ✅ VERIFY |
| DM1_V1_VIEWPORT_WALLS | Walls not rendering | P1 | TODO |
| DM1_V1_DRAW_ORDER | Champion images floating | P1 | TODO |
| DM1_V1_MIRROR_RENDER | Champion mirrors invisible | P1 | TODO |
| DM1_V1_TEXT_INSCRIPTIONS | Wall inscriptions blurry | P1 | TODO |
| DM1_V1_FULL_CODECOMPAR | ReDMCSB full audit | P0 | ✅ DONE |
| CSB_INTEGRATION_PLAN | CSB support plan | P1 | ✅ DONE |
| CSB_V1_RUNTIME_HANDOFF | CSB V1 launch | P0 | IN PROGRESS |
