# DM2 V1 Startup Sequence

## Comparison: DM2 vs DM1

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Startup mechanism | Watcom C startup, DOS int 21h | Watcom C startup, same pattern |
| Memory setup | Custom alloc via DOS memory blocks | Custom alloc via dm2_dballochandler |
| Graphics init | Direct GRAPHICS_STRUCTURE read | Same, via READ_GRAPHICS_STRUCTURE() |
| GDAT loading | Single phase | Three-phase: 00_0A, 00_02, 00_00 |
| Compatibility flags | None | DM1 TQ mode detection (iCompatibilityFlag) |
| Main loop entry | Immediate dungeon load | while (SHOW_MENU_SCREEN(), GAME_LOAD() != 1) |

## DM2 Startup Flow

main() [SKULLWIN/main.cpp:313]
  ProcessArgs() - parse command-line (lang, gdat version, dungeon, music)
  alg.start() - start Allegro system
  
  DM2_WATCOM_STARTUP() [SKULLWIN/main.cpp:216]
  
INIT() [SkWinCore.cpp:55593]
  _4726_02f7() - early init
  READ_GRAPHICS_STRUCTURE()
  LOAD_GDAT_INTERFACE_00_0A() - preload A-type interface data
  SET_RGB_PALETTE_FROM_DATA() - load master palette (C01/I00)
  LOAD_GDAT_INTERFACE_00_02() - load interface data, palettes
  EXTENDED_LOAD_AI_DEFINITION() - load creature AI definitions
  iCompatibilityFlag = QUERY_GDAT_ENTRY_DATA_INDEX(0,0,dtWordValue,0x10)
    if == 3 -> set bDM1Mode=1, bDM1TQMode=1
  LOAD_GDAT_INTERFACE_00_00() - load dungeon interface data
  _38c8_00c8() - configure graphics subsystem
  _3929_0e16() - text/shape renderer setup
  _2405_0009() - init text entry encoding
  _443c_0380() - init mouse/keyboard
  
  while (SHOW_MENU_SCREEN(), GAME_LOAD() != 1)
    GRAPHICS_DATA_OPEN() each iteration
  -> either NEW GAME or RESUME selected
  
  __LOAD_CREATURE_FROM_DUNGEON() - load creature definitions
  ALLOC_CPX_SETUP() - allocate creature processing buffers
  __INIT_GAME_38c8_03ad() - start game logic
  
  if (glbSpecialScreen != 0) -> teleport to marked position (RESUME path)

## Key DM2-Only Startup Features

- Three-phase GDAT loading: DM2 loads interface data in three distinct passes
  (00_0A, 00_02, 00_00) vs DM1s single pass
- DM1TQ compatibility mode: If GDAT byte 0x10 == 3, engine sets bDM1Mode and
  bDM1TQMode enabling Dungeon Master I: The Chaos Plugin compatibility
- Command-line options: DM2 supports -en/-jp/-de/-fr/-es/-it (language),
  -v1..-v5/-vx/-cartoon (gdat version), -dm1/-csb/-beta/-dm2 (dungeon),
  -hmp/-mod/-ogg/-segacd (music)
- No intro movie: DM2 goes directly from startup to menu/title screen

## Source References

- Entry point: skproject/SKULLWIN/main.cpp:313
- Startup: skproject/SKULLWIN/main.cpp:216 -> DM2_WATCOM_STARTUP()
- INIT: skproject/SKWIN/SkWinCore.cpp:55593
- Compatibility: skproject/SKWIN/SkWinCore.cpp:55620-55626
- Menu loop: skproject/SKWIN/SkWinCore.cpp:55639
