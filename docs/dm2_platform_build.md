# DM2 V1 — Platform/Build Audit: Build System

## Build System

DM2 V1 is built via CMake (unlike DM1 V1 which uses a Turbo-C legacy approach
mixed with a CMake wrapper). The Firestaff CMakeLists.txt produces multiple
static libraries; DM2 V1 is built as `firestaff_dm2`.

### CMake Library Target

```cmake
file(GLOB DM2_SOURCES CONFIGURE_DEPENDS "src/dm2/dm2_v1_*.c")
add_library(firestaff_dm2 STATIC ${DM2_SOURCES})
target_include_directories(firestaff_dm2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(firestaff_dm2 PUBLIC firestaff_m10 m)   # NOTE: M10 dep NOT in current CMakeLists.txt
if(NOT MSVC)
    target_compile_options(firestaff_dm2 PRIVATE -Wall -Wextra -O2)
endif()
```

### Source Files (V1)

- `src/dm2/dm2_v1_game.c`        — core game state, init, dungeon load dispatch
- `src/dm2/dm2_v1_dungeon_loader.c` — dungeon.dat parsing (SKULL.ASM sourced)
- `src/dm2/dm2_v1_outdoor_renderer.c` — outdoor viewport, sky gradient (SKULL.ASM)
- `src/dm2/dm2_v1_save_load.c`   — SKSAVE* format (SKULL.ASM sourced)
- `src/dm2/dm2_v1_combat.c`      — combat resolver with DM2 weapon types
- `src/dm2/dm2_v1_tech_magic.c`  — tech/magic hybrid item system
- `src/dm2/dm2_v1_companion.c`   — party companions

### How DM2 Build Differs from DM1

| Aspect               | DM1 V1                              | DM2 V1                          |
|----------------------|-------------------------------------|---------------------------------|
| Build system         | CMake + M10 `_pc34_compat.c` static  | CMake + `dm2_v1_*.c` static     |
| M10 dependency       | Full M10 static lib (`firestaff_m10`) | Reused (`firestaff_m10`)         |
| Compiler flags       | `-std=c99 -O2 -include stddef.h`    | `-Wall -Wextra -O2`              |
| Graphics subsystem   | GRAPHICS.DAT + bitmap_call (shared)  | GRAPHICS.DAT + outdoor renderer  |
| Dungeon format       | DM1 dungeon.dat (33 KB)             | DM2 dungeon.dat (39 KB, outdoor) |
| Sound               | DM1 audio                           | DM2 MIDI (HMP format, Allegro)   |
| State struct         | `M10_GameState`                     | `DM2_V1_GameState`               |

### Original Build Targets (skproject)

The skproject contains three distinct DM2 builds:

1. **SKULLWIN** (Windows, Allegro 5)  
   - `skull.vcxproj` — Visual C++, Release/Win32, x86  
   - Links: `allegro-5.0.10-mt.lib`, `allegro_audio-5.0.10-mt.lib`, `allegro_acodec-5.0.10-mt.lib`  
   - Preprocessor: `WIN32`, `NDEBUG`, `_CONSOLE`  
   - Output: `SKULLWIN/SKWin*.exe`

2. **SKWIN** (Windows, SDL or MFC)  
   - `skwinmfc.vcproj` — Visual C++ MFC  
   - Also has SDL variant (`SkwinSDL.cpp`)  
   - Output: `SKWIN/SKWin*.exe` (multiple variants: Standard, Extended, SuperMode, JP)

3. **SKWINDOS** (DOS, real-mode or protected)  
   - Subdir: `SKWINDOS/src/` — C sources for DOS target  
   - Likely Turbo C / Borland C based  
   - No CMake, built with DOS-era toolchain

### SDL/Graphics Library

Firestaff prefers SDL3; falls back to SDL2. SKWIN/SKULLWIN use:
- **SKULLWIN**: Allegro 5 (standalone, not SDL)
- **SKWIN-SDL**: SDL 1.x (`SkwinSDL.cpp`, scales vram[65536] to 640×400 surface)
- SDL scales by integer factor (1x=320×200, 2x=640×400, 3x=960×600, 4x=1280×800)

### Compilation Quirks

- DM2 V2 sources (`dm2_v2_*.c`) live alongside V1 in the same `src/dm2/` directory.
  V2 files (viewport renderer, outdoor enhanced, companion UI, crafting) are
  NOT included in `firestaff_dm2` — they belong to `firestaff_dm2_v2`.
- SKULLWIN uses `#pragma warning (disable:4312)` and `/Zm` (memory model) flags.
- The `DM2_EXTENDED_MODE` preprocessor flag (1 or undefined) changes
  `CREATURE_AI_TAB_SIZE` from 42 to 64 entries.
