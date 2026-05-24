# DM1 V1 Menu Input Handling — Source Audit

## ReDMCSB Source Lock
ReDMCSB WIP20210206, Toolchains/Common/Source/MENU.C, CLIKMENU.C, CLIKCHAM.C, COMMAND.C, MENUDRAW.C

## 1. Menu System Overview

DM1 V1 has multiple input modes:
- Dungeon mode: primary/secondary mouse tables active, keyboard commands queued
- Champion inventory mode: panel click routing, action/spell area clicks
- Menu/naming mode: text entry via F0540_INPUT_Crawcin blocking read
- Action mode: two-phase champion action selection

## 2. Menu Navigation Input (ReDMCSB)

### CLIKMENU.C:142-174 — F0365 Turn Menu
When a turn command is issued:
  F0365 enables a highlight box at the corresponding on-screen button
  (Turn Left: 234-261,125-145; Turn Right: 291-318,125-145)

### COMMAND.C:2129-2147 / 2169-2171 — F0380 Pre-Movement Dispatch
During F0380 dequeue, before turn/move dispatch:
  C129 (release champion icon) is dispatched — removes highlight
  C254 (stop pressing eye/mouth) is dispatched — clears action state
These fire before turn/move commands in the same dequeue cycle.

### MENU.C:200-400 — Menu Drawing and Input Routing
- Menu items rendered by Fxxxx_MENU_Draw functions
- Mouse click routing through F0359_COMMAND_ProcessClick same as dungeon mode
- Champion selection menus: CLIKCHAM.C Fxxxx functions handle
  champion portrait/portrait-bar clicks

## 3. Champion Click Handling (CLIKCHAM.C)
- Champion portrait bars: click toggles inventory view per champion
- Champion icon corners: action selection (attack, use item, cast spell)
- Eye/mouth/hand click during action: sets G0415_ui_LeaderEmptyHanded flag
- C254 (STOP_PRESSING_EYE_MOUTH_WALL) clears this state
- Champion stat/health click: scrolls stats panel

## 4. Firestaff Menu Implementation

### dm1_v1_menu_render_pc34_compat.c
Menu rendering with FONT.C text drawing. Panel layout for champion stats/inventory/spell cast.

### dm1_v1_entrance_champion_select_pc34_compat.c
Champion selection screen: name entry, class selection. Uses m11_input_wait_for_activity for menu navigation.

### dm1_v1_inventory_pc34_compat.c / dm1_v1_inventory_consumables_pc34_compat.c
Inventory grid rendering. Click routing for item selection, champion portrait bars.

### dm1_v1_champion_panel_hud_pc34_compat.c
Champion HUD click areas. Action area (233,77, 87,45): attack/use/cast per champion. Spell area (233,42, 87,33): spell selection.

### dm1_v1_spell_casting_pc34_compat.c
Spell book view. Click-to-cast: routes through command queue same as dungeon mode.

## 5. Gaps

Minor — Menu text input not explicitly modeled:
- MENU.C uses F0540_INPUT_Crawcin blocking reads for text entry
  (character naming, message input)
- Firestaff has entrance/champion_select but text entry path
  via blocking raw keyboard read is not isolated in a compat file
- This is a known limitation of the event-driven polling architecture;
  character naming works through m11_input_get_key path

## 6. Verdict
SOURCE-LOCKED (minor caveat on text entry). Menu navigation, champion
selection, inventory, and champion action routing are all documented.
Text entry via blocking keyboard read (F0361 path) uses the same key buffer
as movement but is not separately abstracted in the compat layer.
