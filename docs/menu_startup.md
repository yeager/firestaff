# DM1 V1 - Startup Menu (Entrance Screen)

## Source-Locked
ReDMCSB WIP20210206 - Toolchains/Common/Source/

---

## Entrance Flow (ENTRANCE.C)

The startup/entrance sequence is driven by F0441_STARTEND_ProcessEntrance
(ENTRANCE.C:620), called from F0448_STARTUP1_InitializeMemoryManager_CPSADEF
(STARTUP1.C:162).

The outer loop calls F0439_STARTEND_DrawEntrance then waits for user input
in mode C099_MODE_WAITING_ON_ENTRANCE. Two outcomes are possible:

| Constant                   | Value | Meaning                      | Location      |
|----------------------------|-------|------------------------------|---------------|
| C001_MODE_LOAD_DUNGEON     | 1     | New Game - enter dungeon     | DEFS.H:2992   |
| C000_MODE_LOAD_SAVED_GAME  | 0     | Continue / Load Saved Game   | DEFS.H:2991   |
| C099_MODE_WAITING_ON_ENTRANCE | 99 | Waiting on entrance          | DEFS.H:2993   |

Mouse clicks on entrance buttons route to modes above. On Atari ST/early DOS,
carriage-return also commits new game (ENTRANCE.C:858-872).

After the loop exits, F0438_STARTEND_OpenEntranceDoors animates the doors
if G0298_B_NewGame != C000_MODE_LOAD_SAVED_GAME (ENTRANCE.C:942).

---

## Entrance Graphics

F0441 loads two graphics:
- C004_GRAPHIC_ENTRANCE -> G0563_puc_Graphic4_InterfaceEntranceScreen (full UI)
- C005_GRAPHIC_CREDITS -> G0564_puc_Graphic5_InterfaceCredits

Sound C03_SOUND_DOOR_RATTLE_ENTRANCE plays on door open.
Music C0_MUSIC_ENTRANCE plays during the screen (ENTRANCE.C:836).

---

## Buttons / Click Zones (CEDTDATA.C)

File picker dialog: G2256_ai_Box_FilePickerDialog = {62,255,48,149} (CEDTDATA.C:11)

Mouse input records for dialogs:
- G2260_as_MouseInput_Dialog3ChoicesA[4] - 3-choice dialog
- G2259_as_MouseInput_Dialog2Choices[3] - 2-choice dialog
- G2258_as_MouseInput_Dialog1Choice[2] - 1-choice dialog
- G2285_as_FilePickerDialogButtons[5] - file picker (list/new disk/cancel/up)

---

## Title Screen (TITLE.C)

F0437_STARTEND_DrawTitle (TITLE.C:12) renders the animated title logo
with zoom-in Dungeon/Chaos word effect. Requires ~133-146 KB RAM.
Returns early if insufficient memory.

---

## Complete Startup Chain

1. F0448_STARTUP1_InitializeMemoryManager (STARTUP1.C:70)
2. F0437_STARTEND_DrawTitle (TITLE.C:12) - animated logo
3. F0441_STARTEND_ProcessEntrance (ENTRANCE.C:620)
   - loops on C099_MODE_WAITING_ON_ENTRANCE
   - New Game -> C001_MODE_LOAD_DUNGEON | Continue -> C000_MODE_LOAD_SAVED_GAME
4. F0438_STARTEND_OpenEntranceDoors (if new game) (ENTRANCE.C:942)
5. F0435_STARTEND_LoadGame (load save file) (STARTUP1.C:163)
6. F0462_START_StartGame_CPSEF (begin dungeon) (STARTUP1.C:172)
