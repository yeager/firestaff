# DM2 V1 — Current Regression and Verification Status

The 2026-04 stub inventory formerly at the top of this page is obsolete.
Firestaff now reads supported original DM2 archives directly in memory, with
no runtime emulator dependency and no game-data extraction to disk.

## Verified native startup paths

| Platform | Retail input | Verified sequence | Result |
| --- | --- | --- | --- |
| DOS English | `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | launcher card selection → title/menu → New Game → movement | Native runtime receipt reports `dm2RealAssets=1`, `dm2NoCoreFallbacks=1`, and `dm2FallbackDraws=0`. |
| FM Towns Japanese | `Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip` | launcher card selection → regional title → New Game → dungeon choice → movement | Native runtime receipt reports the same no-fallback conditions from the Towns archive. |

Run the real-media checks with:

```sh
bash tests/test_dm2_v1_dos_native_cli_boot.sh <firestaff>
bash tests/test_dm2_v1_fmtowns_native_cli_boot.sh <firestaff>
```

DOS owns its MVE/GDAT title and menu path.  FM Towns owns a distinct regional
title path; after the source New Game click it shows its regional
`Resume`/`New Game`/`Quit` menu.  A successful boot does not prove complete
animation, palette, HUD, audio, save, or campaign parity.

## Active parity work

- Verify title-animation palette timing frame by frame against authenticated
  original output for each supported revision.
- Verify menu layout, highlights, cursor and transition cadence with
  source-owned screenshot/capture comparisons.
- Extend real-media interaction coverage for HUD, viewport, saves, audio and
  platform-specific controls.
- Establish equivalent native startup/gameplay evidence for Amiga and Mac.

## Historical notes (superseded)

The remaining sections are retained only for source-history context.  Their
claims about DM2 being a stub or its menu being unable to launch are not
current status.

---

## Shared infrastructure history

DM2 V1 depends on these shared Firestaff components. If these break, DM2
development would be affected:

### 2.1 CMake Build System

**What could break**: `firestaff_dm2` static library target stops compiling

**Current state**:
```cmake
file(GLOB DM2_SOURCES CONFIGURE_DEPENDS "src/dm2/dm2_v1_*.c")
add_library(firestaff_dm2 STATIC ${DM2_SOURCES})
```

**Risk**: Low. Stub files are trivial and compile cleanly.

**Monitor**: `cmake --build build` continues to produce `libfirestaff_dm2.a`

### 2.2 asset_find_by_hash / Asset Discovery

**What could break**: DM2's hash-based dungeon/graphics lookup in
`dm2_v1_game.c` uses `asset_find_by_md5_list()`. If this function breaks,
DM2 dungeon loading breaks.

**Current state** (from `dm2_v1_game.c`):
```c
if (asset_find_by_md5_list(state->data_dir, dm2_dungeon_hashes,
                           path, sizeof(path), NULL, 4)) {
    // found dungeon at path
}
```

**Risk**: Medium. `asset_find_by_hash.c` is a shared component used by
all games. A regression there would affect DM1, CSB, and DM2 simultaneously.

**Regression scenario**: If `asset_find_by_md5_list()` changes its signature
or behavior, `dm2_v1_game.c` would fail to compile or silently find wrong files.

**Monitor**: CTest `test_asset_find_by_hash` continues to pass.

### 2.3 M10 Dependency (firestaff_m10)

**What could break**: DM2 V1 stubs currently do not use M10 symbols, but
the plan (per `docs/dm2_platform_build.md`) is for `firestaff_dm2` to link
against `firestaff_m10`. If M10 changes its API or removes symbols DM2
needs, build would break.

**Current CMake** (hypothetical, not in tree yet per docs):
```cmake
target_link_libraries(firestaff_dm2 PUBLIC firestaff_m10 m)
```

**Risk**: Medium. M10 is actively developed. The TODO shows M10 still
has in-progress items.

**Monitor**: `firestaff_m10` continues to compile, and `firestaff_dm2`
linking does not produce undefined symbol errors.

### 2.4 SDL3 Dependency

**What could break**: Outdoor renderer and any future DM2 UI will need SDL3.
If SDL3 API changes (e.g., `SDL_CreateRenderer` signature change), DM2
builds would break.

**Risk**: Low. SDL3 is stable (tag `release-3.2.14`).

---

## 3. SKWin/SKULLWIN Source Reference Drift

### What Could Happen

As DM2 V1 implementation proceeds, the team will reference `SKWIN/c_ai.cpp`,
`SKULLWIN` sources, and `SKULL.ASM` disassembly. If these references are
updated (e.g., a better disassembly is found, or the skproject is amended),
implementation decisions based on older references could be invalidated.

### Specific Drift Risks

| Reference | What changes could invalidate |
|-----------|-------------------------------|
| SKULL.ASM | Better disassembly with corrected labels/addresses |
| SKWIN c_ai.cpp | Code cleanup that changes structure but not behavior |
| SKULLWIN | Allegro version upgrade changes API calls |
| docs/dm2_bugs.md | Community finds new bug, changes known-bug list |

### Mitigation

Source-lock manifests in `docs/dm2_source_lock.md` record the exact archive
versions and hashes. Any change to DM2 source reference must update the
manifest. Implementation tests should reference the manifest SHA, not
just the file path.

---

## 4. DM1/CSB Regressions Affecting DM2 Shared Code

The `src/shared/` directory contains code used by all games. A regression
in shared code could silently break DM2:

### 4.1 Memory Allocator (src/memory/)

DM2's dungeon loader allocates `raw_data` via `malloc`:
```c
out->raw_data = (uint8_t *)malloc(size);
```

If the shared memory allocator changes (e.g., adding a wrapper that requires
special initialization), DM2's allocator calls could break.

**Monitor**: `test_memory_cache_*` suite continues to pass.

### 4.2 String/Path Utilities (src/shared/)

If `asset_find_by_hash` or path-normalization functions in `src/shared/`
change, DM2's asset discovery would be affected.

**Monitor**: `asset_find_by_hash` tests pass.

---

## 5. Known DM2-Specific Issues (Not Regressions)

These are documented issues in the existing DM2 code that will need fixing:

### 5.1 M10 Linkage Not in CMakeLists.txt

Per `docs/dm2_platform_build.md`, the documented CMake should link
`firestaff_dm2` to `firestaff_m10`, but this is NOT present in the current
`CMakeLists.txt`. This is a documentation-code gap, not a regression.

**Severity**: MEDIUM — DM2 cannot use shared M10 infrastructure until linked.

### 5.2 Native ZIP Reading

Resolved: supported original DM2 ZIP members are read by Firestaff's native
archive resolver and kept in memory. The PC-DOS SKSAVE corpus path is covered
by a real-data regression that scans four primary saves and four backups from
the retail archive, then rereads a receipt-bound payload without materialising
any member on disk. Unsupported containers remain explicit input errors rather
than extraction fallbacks.

### 5.3 V2 Files in Wrong Library

Per `docs/dm2_platform_build.md`:
- `dm2_v2_*.c` files are in `src/dm2/` but belong in `firestaff_dm2_v2`
- Current CMake uses `file(GLOB DM2_V2_SOURCES ...)` which may not correctly
  separate V1 and V2 sources if naming conventions slip

**Severity**: MEDIUM — V2 code polluting V1 library.

---

## 6. Regression Monitoring Plan

### For Phase 0 (current state — stubs only)

- Monitor `cmake --build build` continues to produce `libfirestaff_dm2.a`
- Monitor `asset_find_by_hash` tests pass
- Monitor no new compile errors in `src/dm2/dm2_v1_*.c`

### For Phase 1+ (when functional code lands)

- All new DM2 V1 tests pass before merge to main
- Dungeon loader tests pass with canonical fixture
- Save/load roundtrip tests pass
- Combat resolver tests pass against reference values
- Cross-platform determinism hash includes DM2 state

### Breakage Indicators

| Indicator | Likely Cause |
|-----------|-------------|
| `libfirestaff_dm2.a` not built | Stub file syntax error or CMake GLOB failure |
| `asset_find_by_md5_list` undefined | Shared asset code refactored |
| `DM2_V1_GameState` incomplete type | Header modified, not updated in source |
| DM2 probe segfaults on launch | Dungeon loader dereferences NULL |
