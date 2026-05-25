# DM1 V1 — Touchscreen Menu Input

## Source Lock
ReDMCSB WIP20210206: MENUDRAW.C F0395/F0396/F0397/F0398/F0457; MENU.C F0392/F0802; COMMAND.C G0448/G0452/G0453/G0454; CLIKMENU.C F0363/F0365/F0366; GAMELOOP.C.

## Menu Overlay (ESC / C011)
Pressing ESC or right-clicking the screen (C011) opens the in-game menu overlay. The menu renders over the dungeon view and blocks all game input until dismissed.

Menu areas drawn by F0457_START_DrawEnabledMenus_CPSF when:
```
!G0300_B_PartyIsResting
!G0299_ui_CandidateChampionOrdinal
```

## Movement Arrow Area (F0395)
Six arrows rendered at bottom of screen (y≥124). Tap detection:
```
Forward:   x=263, y=125, 27×21 → C003 → F0366 step forward
Backward:  x=263, y=147, 27×21 → C005 → F0366 step backward
Turn Left: x=234, y=125, 28×21 → C001 → F0365 turn left 90°
Turn Right: x=291, y=125, 28×21 → C002 → F0365 turn right 90°
Strafe L:  x=234, y=147, 28×21 → C006 → F0366 strafe left
Strafe R:  x=291, y=147, 28×21 → C004 → F0366 strafe right
```
F0395 blits graphic C013 (movement arrows) to zone 009.

## Action Area (F0398)
Action area rendered at right side (233,72 85×50):
- ATTACK (leader hand empty)
- CAST (leader can cast and symbol selected)
- USE (leader hand holds usable item)
- THROW (leader hand holds any item)

Tap → C111 (COMMAND.C:393 maps to C011_ZONE_ACTION_AREA) → action submenu dispatch.

### Action Submenus (CLIKMENU.C:519-585)
After C111 dispatch, click position determines child action:

| Sub-zone | Position | Command | Action |
|----------|----------|---------|--------|
| Pass | 285,77 35×7 | C112 | End turn without action |
| Row 0 | 234,86 85×11 | C113 | Action row 0 |
| Row 1 | 234,98 85×11 | C114 | Action row 1 |
| Row 2 | 234,110 85×11 | C115 | Action row 2 |
| Icon 0-3 | 233-319,86 20×35 | C116-C119 | Champion action icon |

## Spell Area (F0396/F0397)
Spell area at right side (233,42 87×33):
- F0396 loads C011 graphic for spell area frame
- F0397 draws 6 available spell symbols per casting step

Tap spell parent (C100) → C013_ZONE_SPELL_AREA → spell submenu.

### Spell Submenus (COMMAND.C G0454)

| Sub-zone | Position | Command | Action |
|----------|----------|---------|--------|
| Set Caster | 233,42 87×8 | C109 | Assign magic caster champion |
| Symbol 1 | 235,51 13×11 | C101 | Select rune symbol 1 |
| Symbol 2 | 249,51 13×11 | C102 | Select rune symbol 2 |
| Symbol 3 | 263,51 13×11 | C103 | Select rune symbol 3 |
| Symbol 4 | 277,51 13×11 | C104 | Select rune symbol 4 |
| Symbol 5 | 291,51 13×11 | C105 | Select rune symbol 5 |
| Symbol 6 | 305,51 13×11 | C106 | Select rune symbol 6 |
| Cast | 234,63 70×11 | C108 | Execute spell cast |
| Recant | 305,63 14×11 | C107 | Cancel spell selection |

## Highlight Box Feedback (F0363)
When a button/menu item is tapped, F0363_COMMAND_HighlightBoxDisable clears the command highlight box. The highlight is the visual feedback showing the button press — it is drawn during the menu render phase and cleared on the next mouse-down event.

## Text Input in Menus
Text input (e.g., champion naming, text event responses) uses keyboard input routed through:
- F0539_INPUT_Cconis: check key available
- F0540_INPUT_Crawcin: blocking key read
- F1097/F1098 key buffer (64-entry ring)

Touch does not provide text input — keyboard is required for character entry. The mouse cursor position is still tracked during text input for button presses (e.g., confirmation dialogs).

## ESC / Menu Dismiss
ESC key → F0363_COMMAND_HighlightBoxDisable → closes menu overlay if open.
Right-click on screen → C011 → same dismissal behavior.
During text input: ESC cancels text entry.

## Source Evidence
- MENUDRAW.C F0395: blit C013 movement arrows to zone 009
- MENUDRAW.C F0398: action area buttons based on leader hand state
- MENUDRAW.C F0457: orchestrator — called from GAMELOOP.C when !G0300 and !G0299
- MENU.C F0392/F0802: menu state management
- CLIKMENU.C F0363: clear command highlight box
- CLIKMENU.C F0365: turn dispatch from arrow taps
- CLIKMENU.C F0366: step/move dispatch from arrow taps
- COMMAND.C G0452/G0453: action submenu command routing
- COMMAND.C G0454: spell submenu command routing
- GAMELOOP.C: menu render call site conditions
