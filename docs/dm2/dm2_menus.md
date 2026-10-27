# DM2 V1 — Menu System

## Overview

DM2 replaces DM1's text-mode menus with a graphical, event-driven dialog/menu system. Menus are rendered as image overlays (often from GDAT assets) with programmatic button layouts. The pause system, save/load dialogs, and in-game menus are all part of a unified UI event code system.

## Event Codes

DM2 uses a central `UI_EVENTCODE_*` enum to route all menu/game flow events. Key codes:

| Event | Value | Description |
|---|---|---|
| `UI_EVENTCODE_PAUSE` | 0x93 | Invoke game pause |
| `UI_EVENTCODE_END_PAUSE` | 0x94 | Resume from pause |
| `UI_EVENTCODE_SLEEP` | 0x91 | Champion sleeps (wait state) |
| `UI_EVENTCODE_WAKE` | 0x92 | Champion wakes up |
| `UI_EVENTCODE_HAND_RUNE_QUIT` | — | Rune/hand quit action |
| `UI_EVENTCODE_DIALOG_BUTTON_1-4` | — | Dialog button responses |
| `UI_EVENTCODE_QUIT_GAME` | — | Quit to title |
| `UI_EVENTCODE_QUIT_CREDITS` | — | Exit credits screen |

## Pause Screen

When `UI_EVENTCODE_PAUSE` is triggered:
1. `_38c8_0002()` — pause audio
2. Screen filled black via `FILL_ENTIRE_PICT(_4976_4c16, COLOR_BLACK)`
3. "GAME PAUSED" text drawn in cyan via `DRAW_VP_RC_STR()` using `QUERY_GDAT_TEXT(0x01, 0x00, 0x12, ...)`
4. `CHANGE_VIEWPORT_TO_INVENTORY(0)` — switch viewport to inventory layer
5. `_1031_0675(3)` and `_1031_098e()` — additional pause rendering

Resume (`UI_EVENTCODE_END_PAUSE`):
- `_38c8_0060()` — resume audio
- `_1031_06a5()` — resume rendering
- `_1031_098e()` — finalize

## Game Save Menu

`GAME_SAVE_MENU()` (address ~0x2066:0D09):
- Accessed from `SHOW_MENU_SCREEN()` main loop
- `FIRE_HIDE_MOUSE_CURSOR()` — hide mouse during menu
- `REARRANGE_TIMERLIST()` — pause timers during save
- Uses dialog box ID system via `_0aaf_02f8_DIALOG_BOX()`:
  - 0x0b / 0x12 — save slot selection (0x12 for existing save)
  - 0x0c — confirm overwrite dialog
  - 0x0d — save name entry
  - 0x13 — success notification
- `_476d_04e8(2)` — possibly disk write operation

### Dialog Box IDs
- `0x0b` — empty save slot selection
- `0x0c` — confirmation (button 3 = proceed)
- `0x0d` — name input
- `0x12` — existing save overwrite
- `0x13` — confirmation message
- `0x14` — error/invalid

## Title/Menu Screen

`SHOW_MENU_SCREEN()`:
- `glbImageMenuScreen` — pointer to menu background from GDAT
- Loads via `QUERY_GDAT_ENTRY_DATA_PTR(GDAT_CATEGORY_TITLE, 0x00, dt07, 4)` or `QUERY_GDAT_IMAGE_ENTRY_BUFF(GDAT_CATEGORY_TITLE, 0x00, 0x4)`
- Main loop: `while (SHOW_MENU_SCREEN(), GAME_LOAD() != 1)` — keeps showing menu until game loaded

## Dialog Panel System

`__OPEN_DIALOG_PANEL(cls2, yy)`:
- Generic dialog renderer
- Uses `DRAW_DIALOGUE_PICT()` / `DRAW_DIALOGUE_PARTS_PICT()` for rendering
- Buttons rendered via `QUERY_GDAT_TEXT()` for labels
- `_0aaf_0067()` — dialog result handler

Dialog picture rendering:
- `DRAW_DIALOGUE_PICT()` — draws dialog frame image
- `DRAW_DIALOGUE_PARTS_PICT()` — draws sub-components
- `QUERY_EXPANDED_RECT(4, &bp10)` — retrieves rect data for layout

## Menu vs DM1 Differences

| Feature | DM1 | DM2 |
|---|---|---|
| Menu type | DOS text-mode | Graphical image-based |
| Pause | DOS keystroke | UI_EVENTCODE_PAUSE + viewport switch |
| Save dialog | DOS file I/O | GDAT-backed dialog boxes |
| Save/Load | File name entry | Multi-step dialog with slot selection |
| Menus driven by | Switch/case on keystroke | Event code dispatcher |
| Screen backgrounds | None (console) | GDAT images for each menu |

## Main Game Loop Menu Integration

```
while (SHOW_MENU_SCREEN(), GAME_LOAD() != 1) { }
```
- `SHOW_MENU_SCREEN()` displays title/menu
- `GAME_LOAD()` returns 1 on successful load
- Loop continues until player starts or quits

## Credits Screen

- `while (MAIN_LOOP(), glbUIEventCode != UI_EVENTCODE_QUIT_CREDITS)` — credits loop
- `DRAW_GAMELOAD_DIALOGUE_TO_SCREEN(glbImageCreditScreen, ...)` — credit screen rendering
- `glbImageCreditScreen` — loaded from GDAT title category

## Key Globals

- `glbImageMenuScreen` (`_4976_52b2`) — pointer to menu background image
- `glbUIEventCode` — current UI event being processed
- `_4976_4c02` — pause state flag (0=paused, 1=resumed)
- `_4976_49a0` — save game presence flag
- `_4976_5258` / `_4976_5246` / `_4976_5240` — save dialog state

## Source Files

- `skproject/SKWIN/SkWinCore.cpp` — menu implementations (`GAME_SAVE_MENU`, `SHOW_MENU_SCREEN`, pause handling)
- `skproject/SKWIN/DME.h` / `DME.cpp` — dialog event processing
- `skproject/SKWIN/defines.h` — UI_EVENTCODE_* enum, dialog box IDs
- `skproject/SKWIN/SkWinCore.h` — menu method declarations
- `skproject/SKWIN/skval2.h` — menu-related globals
