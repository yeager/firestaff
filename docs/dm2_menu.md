# DM2 V1 Main Menu

## Menu Structure

DM2 main menu is driven by SHOW_MENU_SCREEN() which loads two GDAT images:
- GDAT_CATEGORY_TITLE (0x05), index 4: Main menu screen background
- GDAT_CATEGORY_TITLE (0x05), index 1: Credit/tombstone screen

```
SHOW_MENU_SCREEN() [SkWinCore.cpp:55181]
  Load menu background (GDAT 0x05, idx 4 or raw data)
  Load credit screen (GDAT 0x05, idx 1)
  GRAPHICS_DATA_CLOSE()
  
  do {
    DrawMenuBackground()
    MessageLoop(true) // main menu event polling
    MAIN_LOOP() // process UI events
    
    Exit conditions:
    - glbSpecialScreen == 0x63 -> loop (still at menu)
    - glbSpecialScreen == 0xDA (218) -> credits screen
    - glbSpecialScreen == 1 -> new game or resume chosen
  } while (loop)
```

## Menu Event Codes (from defines.h)

| Event Code | Hex | Action |
|------------|-----|--------|
| UI_EVENTCODE_START_NEW_GAME | 0xD7 | Start fresh game (glbSpecialScreen=1, _4976_5bea=1) |
| UI_EVENTCODE_0D8 | 0xD8 | Unknown (glbSpecialScreen=1, _4976_5bea=1) |
| UI_EVENTCODE_RESUME_GAME | 0xD9 | Resume from save (glbSpecialScreen=0) |
| UI_EVENTCODE_SHOW_CREDITS | 0xDA | Show credits (glbSpecialScreen=0xDA) |
| UI_EVENTCODE_0DF | 0xDF | Unknown (_4976_5bf2=1) |
| UI_EVENTCODE_QUIT_GAME | 0xE0 | Exit game (SK_PREPARE_EXIT) |

## State Variables

| Variable | Type | Purpose |
|----------|------|---------|
| glbSpecialScreen | __int16 | Screen state flag (0=game, 1=menu, 0xDA=credits) |
| _4976_5bea | Bit16u | Flag: new game path vs resume path |
| _4976_5bf2 | __int16 | Flag: controls dungeon file loading behavior |

## Menu Flow after Selection

1. User clicks NEW/RESUME at menu
2. HANDLE_UI_EVENT processes selection
3. glbSpecialScreen set (0 or 1)
4. GAME_LOAD() called - awaits user to pick save slot or create party
5. Loop continues until GAME_LOAD returns 1 (load complete)
6. __LOAD_CREATURE_FROM_DUNGEON() - load creature data
7. ALLOC_CPX_SETUP() - allocate processing buffers
8. __INIT_GAME_38c8_03ad() - begin gameplay

## Credits Screen

SHOW_CREDITS() [SkWinCore.cpp:31002]:
- Draws credit image (tombstone graphic)
- Loops for SCREEN_CREDITS_TIMER ticks (~1800)
- Responds to UI_EVENTCODE_QUIT_CREDITS (0xEF) to exit early
- Sets glbSpecialScreen = 0xDA before returning

## DM2 vs DM1 Menu Differences

| Feature | DM1 | DM2 |
|---------|-----|-----|
| Menu type | Full party creation | Party creation + save/load |
| Credits button | Yes | Yes (0xDA event) |
| Quit button | Yes | Yes (0xE0 event) |
| Save/Load | Via in-game menu | Via main menu RESUME |
| New Game path | Direct to party gen | Via _4976_5bea=1 flag |
| Resume path | glbSpecialScreen=0 | glbSpecialScreen=0 |

## Source References

- Menu screen: skproject/SKWIN/SkWinCore.cpp:55181 (SHOW_MENU_SCREEN)
- Menu events: skproject/SKWIN/SkWinCore.cpp:32000-32040
- Credits: skproject/SKWIN/SkWinCore.cpp:31002 (SHOW_CREDITS)
- Event codes: skproject/SKWIN/defines.h:1053-1070
- Init/game load: skproject/SKWIN/SkWinCore.cpp:55639
