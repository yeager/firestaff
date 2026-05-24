# DM1 V1 - In-Game Menu (ESC Menu)

## Source-Locked
ReDMCSB WIP20210206 - Toolchains/Common/Source/

---

## ESC Menu / Pause Flow

ESC key (C0x1B) is processed by F0361_COMMAND_ProcessKeyPress which
routes to the pause/command handler in the main loop.

The main command queue is processed by:
F0380_COMMAND_ProcessQueue_CPSC (ENTRANCE.C:876)

Command codes 116-119 set the acting champion via:
F0389_MENUS_ProcessCommands116To119_SetActingChampion (MENU.C:696)

---

## Save/Load Loop (STARTUP2.C)

STARTUP2.C:1392 - F0437_STARTEND_DrawTitle called on restart
STARTUP2.C:1444 - F0441_STARTEND_ProcessEntrance called if save not found

The game load/save loop:
- F0435_STARTEND_LoadGame returns C01_LOAD_GAME_SUCCESS on success
- If loading fails, returns to entrance (STARTUP2.C:1447 loop)

---

## Menu Spell Area (MENUDRAW.C)

F0395_MENUS_DrawMovementArrows (MENUDRAW.C:7) - blits C013_GRAPHIC_MOVEMENT_ARROWS
to zone 009 (movement arrows when exploring)

F0396_MENUS_LoadSpellAreaLinesBitmap (MENUDRAW.C:30) - loads C011_GRAPHIC_MENU_SPELL_AREA_LINES

F0397_MENUS_DrawAvailableSymbols (MENUDRAW.C:43) - draws 6 symbol slots per
spell step at Y=58, X increments of 14 starting at X=225

F0398_MENUS_DrawChampionSymbols (MENUDRAW.C:76) - shows learned symbols

---

## Message After Replacements (MENU.C)

F0381_MENUS_PrintMessageAfterReplacements (MENU.C:555) formats combat
messages with champion name substitution. Token @p in message is replaced
by the acting champion name followed by a space.

---

## Action Menu / Combat Actions (MENU.C)

F0383_MENUS_SetActionList (MENU.C:635) - builds action list based on
equipped items and champion skill levels

F0388_MENUS_ClearActingChampion (MENU.C:682) - deselects acting champion

F0389_MENUS_ProcessCommands116To119_SetActingChampion (MENU.C:696) - handles
champion selection via command codes 116-119

F0390_MENUS_RefreshActionAreaAndSetChampionDirectionMaximumDamageReceived
(MENU.C:727) - updates combat UI after an action is selected

F0391_MENUS_DidClickTriggerAction (MENU.C:803) - detects clicks on action
icons to trigger combat actions

F0392_MENUS_BuildSpellAreaLine (MENU.C:844) - constructs a spellcasting
symbol line from selected symbols

---

## Sound in Combat

F0064_SOUND_RequestPlay_CPSD plays combat sound effects positioned
spatially by party map coordinates (MENU.C:1021, 1313, 1316, 1394).

---

## Summary

In-game ESC flow:
1. ESC key -> F0361_COMMAND_ProcessKeyPress -> pause state
2. Champion selection: commands 116-119 via F0389 (MENU.C:696)
3. Action list built by F0383 (MENU.C:635)
4. Spell symbols drawn by F0397 (MENUDRAW.C:43)
5. Save/Load via F0435_STARTEND_LoadGame (STARTUP2.C:1444)
