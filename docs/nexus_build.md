# Nexus V1 Build System Audit — Source-Locked

## Sources
- `CMakeLists.txt` (lines 1-50, 100-180, 200-280)
- `src/nexus/nexus_v1_*.c` (all 19 source files)
- `docs/platform/platform_build.md`
- `docs/nexus_overview.md`

---

## 1. Build System: CMake 3.20+

### CMake Configuration
```
cmake_minimum_required(VERSION 3.20)
project(Firestaff C)
set(CMAKE_C_STANDARD 11)
```

All Firestaff games (DM1, CSB, DM2, Nexus) use the **same CMake build system**.

### Nexus-Specific Library: firestaff_nexus

From `CMakeLists.txt`:
```cmake
add_library(firestaff_nexus STATIC
    src/nexus/nexus_v1_champions.c
    src/nexus/nexus_v1_combat.c
    src/nexus/nexus_v1_creatures.c
    src/nexus/nexus_v1_dmdf_model.c
    src/nexus/nexus_v1_dungeon.c
    src/nexus/nexus_v1_engine.c
    src/nexus/nexus_v1_game.c
    src/nexus/nexus_v1_iso_reader.c
    src/nexus/nexus_v1_magic.c
    src/nexus/nexus_v1_math3d.c
    src/nexus/nexus_v1_rasterizer.c
    src/nexus/nexus_v1_saturn_font.c
    src/nexus/nexus_v1_text.c
    src/nexus/nexus_v1_viewport.c
    # Nexus V2 modules:
    src/nexus/nexus_v2_atmosphere.c
    src/nexus/nexus_v2_config.c
    src/nexus/nexus_v2_lighting.c
    src/nexus/nexus_v2_particles.c
    src/nexus/nexus_v2_render_pipeline.c
    src/nexus/nexus_v2_upscaler.c
)
target_include_directories(firestaff_nexus PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nexus
)
```

**Note:** `firestaff_nexus` has NO external library dependencies.
Unlike `firestaff_m11` (requires SDL3), the Nexus library is pure C
with no SDL, no `m` (math library), and no other Firestaff libraries linked.

### Compiler Support
| Compiler | Platform | Flags |
|----------|----------|-------|
| GCC | Linux/macOS/Windows | `-Wall -Wextra -O2` |
| Clang | macOS/Linux | `-Wall -Wextra -O2` |
| MSVC | Windows | `/W4 /O2` |

### Build Artifacts
- **Static library**: `build/libfirestaff_nexus.a`
- **Verification binaries**: `build/verification-nexus-*` (when wired in)
- **No standalone Nexus executable yet** — library exists but not linked into a binary

---

## 2. Nexus Source Files and Their Roles

### V1 Files (original Nexus implementation)
| File | Purpose |
|------|---------|
| `nexus_v1_engine.c` | Engine init, data source (ISO/extracted), main loop, font loading |
| `nexus_v1_game.c` | Game state (party, champions, inventory, spells) |
| `nexus_v1_dungeon.c` | DGN file parser (grid, 3D geometry blob) |
| `nexus_v1_iso_reader.c` | ISO 9660 / CUE+BIN disc image reader |
| `nexus_v1_dmdf_model.c` | DMDF/MNS 3D model loader (big-endian) |
| `nexus_v1_rasterizer.c` | Software triangle rasterizer (320×200 framebuffer) |
| `nexus_v1_viewport.c` | First-person viewport renderer |
| `nexus_v1_math3d.c` | Vec3/Mat4 3D math, perspective projection |
| `nexus_v1_champions.c` | Champion stats, leveling, inventory management |
| `nexus_v1_combat.c` | Combat system (DM1 mechanics) |
| `nexus_v1_creatures.c` | Creature AI, spawning, death/drops |
| `nexus_v1_magic.c` | Spell system (DM1 magic + alignment) |
| `nexus_v1_text.c` | Text rendering (font loading, string drawing) |
| `nexus_v1_saturn_font.c` | 256-char Saturn font (including Japanese Shift-JIS) |

### V2 Files (Firestaff enhancements)
| File | Purpose |
|------|---------|
| `nexus_v2_render_pipeline.c` | Enhanced rendering pipeline |
| `nexus_v2_lighting.c` | Lighting model for 3D scene |
| `nexus_v2_atmosphere.c` | Atmospheric effects (fog, particles) |
| `nexus_v2_particles.c` | Particle system |
| `nexus_v2_config.c` | Configuration management |
| `nexus_v2_upscaler.c` | EPX/others upscaling (2×, 3×) |

---

## 3. How Nexus V1 Is Compiled

### Build Commands
```bash
# Configure
cmake -B build -DFIRESTAFF_NEXUS=ON

# Build static library only
cmake --build build --target firestaff_nexus

# Or build everything
cmake --build build
```

### Compilation Units
19 V1/V2 C files compiled into `libfirestaff_nexus.a`:
- No generated sources (no code generation step)
- No external dependencies beyond standard C library
- Big-endian byte order handled via explicit byte-swapping functions

### Linking
Currently `firestaff_nexus` is NOT linked into any test executable.
The library is fully compiled but "orphaned" — no `add_executable` references it.

Status from CMakeLists.txt:
```
set_tests_properties(dm1_v2_completion_matrix_gate PROPERTIES WILL_FAIL TRUE)
# Many DM1 V2 tests WILL_FAIL
# No nexus tests exist
```

---

## 4. How Nexus Differs from DM1/CSB Build

| Aspect | DM1/CSB (firestaff_m10) | Nexus V1 (firestaff_nexus) |
|--------|-------------------------|---------------------------|
| Dependencies | SDL3 (M11), math lib | None (pure C) |
| Language standard | C99 | C11 |
| Compiler flags | -std=c99 -O2 | -std=c11 -O2 |
| External libs | m (math), SDL3 | none |
| Generated code | None | None |
| Test coverage | 304 tests | 0 tests |
| Executable link | Yes (dm1_v1_engine) | No (library only) |

---

## 5. Platform-Specific Build Notes

### Windows (MSVC)
```bash
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```
Platform detection: `_WIN32` macro used throughout `nexus_v1_engine.c`

### macOS (Clang)
```bash
cmake -B build
cmake --build build
```
Platform detection: `__APPLE__` macro

### Linux (GCC/Clang)
```bash
cmake -B build
cmake --build build
```
Standard POSIX, no special flags

---

## 6. Build Verification

### Build Integrity Check
```bash
# Should produce libfirestaff_nexus.a without errors
cmake -B build && cmake --build build --target firestaff_nexus
```

### Current Status
- ✅ `libfirestaff_nexus.a` compiles successfully
- ✅ 19 source files, no compilation errors
- ✅ No link dependencies on SDL/external libs
- ❌ No executable links against `firestaff_nexus`
- ❌ No CTest entries for Nexus
- ❌ No parity evidence directory for Nexus

---

## 7. Original Saturn Build (Hypothetical)

**Original development toolchain (not in Firestaff):**
- Sega C compiler for SH-2 (proprietary)
- Custom linker scripts (Saturn memory map)
- No CMake — proprietary makefile system
- ROM generation: `makecomb ROM NEXUS.BIN`

Firestaff is a **clean-room reimplementation**, not a binary port.
It does not use the original Saturn toolchain.
