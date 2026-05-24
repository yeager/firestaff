# DM1 V1 Platform/Build System — Source Lock

## Build System: CMake 3.20+

### CMake Configuration
- **Minimum version**: CMake 3.20
- **Language**: C11 (CMAKE_C_STANDARD = 11)
- **Compiler**: GCC/Clang on Unix, MSVC on Windows
- **Export compile commands**: enabled (CMAKE_EXPORT_COMPILE_COMMANDS ON)

### Library Architecture

#### firestaff_m10 (static)
- **Sources**: src/*_pc34_compat.c (glob, excludes test_ and dm1_v1_engine_pc34_compat)
- **Also includes**: src/shared/dungeon_decompressor_ftl.c
- **Defines**: COMPILE_H, STATICFUNCTION=static, SEPARATOR=, FINAL_SEPARATOR=), HUGE=, huge=
- **Compiler options**: -std=c99 -O2 -include stddef.h (non-MSVC)
- **Purpose**: Frozen Turbo-C compatibility sources — M10 era code

#### firestaff_csb_v2 (static)
- **Sources**: src/csb/csb_v2_*.c
- **Links**: firestaff_m10, m (math)
- **Options**: -Wall -Wextra -O2

#### firestaff_dm2 (static)
- **Sources**: src/dm2/dm2_v1_*.c
- **No m10 link** (standalone DM2)
- **Options**: -Wall -Wextra -O2

#### firestaff_nexus (static)
- **Sources**: src/nexus/nexus_v1_*.c

#### firestaff_dm2_v2 (static)
- **Sources**: src/dm2/dm2_v2_*.c
- **Links**: firestaff_m10, m

#### firestaff_v2 (static)
- **Sources**: src/dm1v2/*_pc34.c (glob)
- **Links**: firestaff_m10, m
- **Include dirs**: include, src

#### firestaff_m11 (static)
- **Sources**: explicit file list (see CMakeLists.txt)
- **Links**: firestaff_m10, SDL3::SDL3, m
- **Options**: -Wall -Wextra -O2
- **Special**: src/engine/m11_game_view.c has relaxed warnings (-Wno-format-truncation -Wno-unused-function -Wno-maybe-uninitialized)

#### firestaff_m12 (static)
- **Sources**: explicit file list (UI, config, assets)

### SDL Discovery
SDL3 preferred; SDL2 aliased to SDL3::SDL3 target. Fatal error if neither found.
Uses find_package(PkgConfig) on all platforms.

### Build Targets (summary)
| Target | Type | Key Link Dependencies |
|--------|------|----------------------|
| firestaff_m10 | static lib | — |
| firestaff_csb_v2 | static lib | m10, m |
| firestaff_dm2 | static lib | — |
| firestaff_nexus | static lib | — |
| firestaff_dm2_v2 | static lib | m10, m |
| firestaff_v2 | static lib | m10, m |
| firestaff_m11 | static lib | m10, SDL3::SDL3, m |
| firestaff_m12 | static lib | (assets/UI) |

Executables (test_* targets, dm1_v1_engine_pc34_compat) link against these libraries.

### Data Paths
- **Source tree**: /home/trv2/work/firestaff
- **Build directory**: /home/trv2/work/firestaff/build (in-tree)
- **ReDMCSB reference**: /home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/

### Build Artifacts
- Static libraries in build/ directory
- Test executables: build/test_*
- Verification binaries in verification-* directories
