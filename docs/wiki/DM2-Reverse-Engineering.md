# DM2 Reverse Engineering

Dungeon Master II: Skullkeep (DM2) is reimplemented in Firestaff at
source-level parity against **skproject**, a reconstructed C++ decompilation
of the original DOS/Atari-ST/Amiga/FM Towns codebase. This page indexes the
skproject reference material, the corresponding Firestaff module registry,
the hex-offset disassembly modules, the GDAT graphics format, DUNGEON.DAT
differences from DM1, and the current symbol-audit and lane status.

## 1. skproject Source File Map

Firestaff comments and headers cite skproject `.cpp` files as their source
of truth. The list below was generated with:

```bash
grep -roh 'c_[a-z_]*\.cpp' src/dm2/ include/dm2_* | sort -u
```

56 distinct skproject files are referenced across `src/dm2/` and
`include/dm2_*`. The reference count is the number of times the filename
appears in Firestaff source/header text (a rough proxy for how many
Firestaff modules cite that file).

| skproject file | Subsystem | References |
|---|---|---|
| `c_ai.cpp` | Creature/monster AI decision logic | 764 |
| `c_record.cpp` | Dungeon record graph (linked records, DBn types) | 271 |
| `c_creature.cpp` | Creature instance state and behavior | 271 |
| `c_tim_proc.cpp` | Timer/event process handlers | 256 |
| `c_querydb.cpp` | GDAT/record query database (`QUERY_*` accessors) | 248 |
| `c_hero.cpp` | Champion/hero state and stats | 151 |
| `c_gui_vp.cpp` | Viewport GUI rendering | 137 |
| `c_events.cpp` | Event dispatch/queueing | 121 |
| `c_weather.cpp` | Outdoor weather simulation | 86 |
| `c_gui_draw.cpp` | GUI drawing primitives | 78 |
| `c_gfx_str.cpp` | Graphics string/text rendering | 74 |
| `c_sound.cpp` | Sound engine | 65 |
| `c_moverec.cpp` | Movement record bookkeeping | 55 |
| `c_timer.cpp` | Timer subsystem | 54 |
| `c_gdatfile.cpp` | GRAPHICS.DAT file I/O | 53 |
| `c_sfx.cpp` | Sound effects playback | 52 |
| `c_item.cpp` | Item instance state | 44 |
| `c_savegame.cpp` | Save/load serialization | 40 |
| `c_map.cpp` | Map/level record management | 38 |
| `c_xrect.cpp` | Extended rectangle math | 37 |
| `c_light.cpp` | Lighting calculations | 32 |
| `c_random.cpp` | RNG | 28 |
| `c_gfx_decode.cpp` | Graphics decode (RLE/packed formats) | 26 |
| `c_gfx_main.cpp` | Graphics subsystem entry points | 25 |
| `c_gfx_blit.cpp` | Blitting routines | 24 |
| `c_combat.cpp` | Combat resolution | 24 |
| `c_loadlevel.cpp` | Level loading | 23 |
| `c_image.cpp` | Image record handling | 23 |
| `c_input.cpp` | Input handling | 18 |
| `c_dialog.cpp` | Dialog boxes | 17 |
| `c_move.cpp` | Movement mechanics | 16 |
| `c_dballoc.cpp` | Dungeon-data block allocator | 16 |
| `c_tmouse.cpp` | Mouse/text-mouse handling | 15 |
| `c_cloud.cpp` | Cloud/weather objects | 12 |
| `c_engage.cpp` | Engage command (spell/action triggers) | 10 |
| `c_bkgrnd.cpp` | Background rendering | 10 |
| `c_render.cpp` | Render pipeline | 6 |
| `c_alloc.cpp` | Memory allocator | 6 |
| `c_addon.cpp` | Add-on/expansion data | 6 |
| `c_eventqueue.cpp` | Event queue | 5 |
| `c_shop.cpp` | Shop/vendor logic | 4 |
| `c_gfx_pal.cpp` | Palette handling | 4 |
| `c_trigger.cpp` | Trigger objects | 3 |
| `c_sensor.cpp` | Sensor objects | 3 |
| `c_music_wav.cpp` | Music/WAV playback | 3 |
| `c_mcursor.cpp` | Mouse cursor | 3 |
| `c_keybd.cpp` | Keyboard input | 3 |
| `c_gfx_bmp.cpp` | Bitmap graphics | 3 |
| `c_clickrect.cpp` | Clickable rectangles | 3 |
| `c_buttons.cpp` | Button widgets | 3 |
| `c_actuator.cpp` | Actuator objects | 3 |
| `c_npc.cpp` | NPC handling | 2 |
| `c_gfx_pixel.cpp` | Pixel-level graphics ops | 2 |
| `c_allegro.cpp` | Allegro host-layer shim | 2 |
| `c_str.cpp` | String utilities | 1 |
| `c_rect.cpp` | Rectangle math | 1 |

### Central reference files

Two files are not part of the 56-file per-module list above but are the two
central skproject reference files cited pervasively across the codebase:

| File | Role | References |
|---|---|---|
| `SkWinCore.cpp` | Monolithic core: houses most of the hex-offset-addressed decompiled routines (`_0aaf_*`, `_1031_*`, `_1c9a_*`, `_2759_*`, `_2e62_*`, `_0cee_*`, etc.) that the symbol audit tracks individually | 832 |
| `SkGlobal.cpp` | Global tables and constants (direction tables, shared globals) referenced by AI, viewport, and creature logic | 24 |

## 2. Module Registry

`src/dm2/` contains 271 source files (`.c`) implementing the DM2 runtime.
They group into the following subsystems (grouping by filename prefix; a
file may touch more than one area but is listed under its primary prefix).

| Subsystem | Prefixes | Purpose |
|---|---|---|
| AI/creatures | `c_ai`, `c_creature`, `creature_ai_*`, `dm2_v1_ai_*`, `dm2_v1_creature_*` | Monster decision logic, spawn, occupancy, AI state machines |
| Combat | `c_combat`, `combat_damage`, `dm2_v1_combat_*` | Attack resolution, damage, wound/kill mechanics |
| Graphics/viewport | `c_gfx_*`, `c_gui_*`, `c_render`, `c_bkgrnd`, `dm2_v1_viewport_*` | Frame rendering, GUI drawing, background composition |
| GDAT data | `c_gdatfile`, `c_image`, `dm2_v1_gdat_*` | GRAPHICS.DAT record decode and typed-image access |
| Dungeon/records | `c_record`, `c_dballoc`, `c_querydb`, `c_map`, `c_loadlevel`, `dm2_v1_dungeon_loader`, `dm2_v1_record_pool_*` | Record graph, DUNGEON.DAT loading, query database |
| Champions/hero | `c_hero`, `champion_*`, `dm2_v1_champion_*` | Champion stats, state, and progression |
| Items/inventory | `c_item`, `c_shop`, `dm2_v1_item_*` | Item instances, inventory, shop transactions |
| Events/timers | `c_events`, `c_eventqueue`, `c_timer`, `c_tim_proc`, `dm2_v1_event_*` | Scheduled events, timer processes |
| Movement | `c_move`, `c_moverec`, `dm2_v1_move_*` | Party/creature movement and collision |
| Input/UI | `c_input`, `c_buttons`, `c_clickrect`, `c_keybd`, `c_mcursor`, `c_tmouse`, `c_dialog` | User input handling, dialogs, cursors |
| Audio | `c_sound`, `c_sfx`, `c_music_wav`, `dm2_v1_sound`, `dm2_v1_fmtowns_cdda_*` | Sound effects, music playback, CD audio |
| Sensors/actuators | `c_sensor`, `c_actuator`, `c_trigger`, `dm2_v1_sensor_*` | Dungeon sensor/actuator/trigger objects |
| Weather/outdoor | `c_weather`, `c_cloud` | Outdoor weather simulation |
| CCM scripts | `ccm_*` | Compiled Command/script (CCM) interpreter |
| Save/load | `c_savegame`, `dm2_v1_savegame_*` | Original-format DOS savegame read/write |
| Misc | `c_alloc`, `c_str`, `c_random`, `c_rect`, `c_xrect`, `c_light`, `c_npc`, `c_addon`, `c_engage` | Allocator, string, RNG, geometry, lighting, NPC, engage-command utilities |

Verify current counts with:

```bash
ls src/dm2/*.c | wc -l          # 271
ls src/dm2/*.c | grep -c '^c_'  # skproject-named modules
```

## 3. Hex-Offset Modules

Three Firestaff modules are named after raw disassembly offset labels from
the original `SkWinCore.cpp` binary rather than skproject's own filenames.
These labels come directly from the address ranges the reconstructed source
uses for otherwise-unnamed static functions:

| Firestaff file | Offset label | Notes |
|---|---|---|
| `src/dm2/dm2_v1_0aaf_pc34_compat.c` | `_0aaf_*` | GDAT 0x1a text-list builder, dialogue background routing |
| `src/dm2/dm2_v1_1031_pc34_compat.c` | `_1031_*` | Rectangle resolve and related UI-geometry helpers |
| `src/dm2/dm2_v1_1c9a_pc34_compat.c` | `_1c9a_*` | Creature AI pointer resolution and tile-lookup helpers |

These correspond to symbol-audit families (e.g. `DM2_0aaf_0067`,
`DM2_1031_01d5`, `DM2_1c9a_02c3` in
`docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`) whose
`source_file` column reads `SKULLWIN/c_0aaf.cpp`, `SKULLWIN/c_1031.cpp`, and
`SKULLWIN/c_1c9a.cpp` — skproject's own naming for functions it could only
address by original binary offset, not by a recovered symbol name.

## 4. GDAT File Format

Full internals are documented in `docs/wiki/DM2-GDAT-Internals.md`; summary:

- **Record model**: `GRAPHICS.DAT` is a typed record store, not a flat
  sprite sheet. Palette, interface, title, map, and map-chip data are
  distinct typed records.
- **Palette chain**: `dtPalIRGB -> dtPalette16 -> selected material/interface
  palette -> framebuffer`. Indexed pixels are only consumable through this
  chain; raw index writes are not accepted as a GDAT draw.
- **`dt07` records**: interface data. `dt07/2` holds bounded primary,
  secondary, and command-tail spans; `dt07/0x0A` holds Rect14 placement
  data, decoded at boot and carried to the runtime host receipt so the HUD
  does not re-parse raw GDAT.
- **Rect14 placement records**: fixed-layout placement rectangles used for
  HUD/interface element positioning, decoded once and cached.
- **Graphics set per map**: each map selects a `GRAPHICSSET`; the active
  style participates in GDAT address resolution and cache identity, so a
  wall decoded under one style cannot be reused under another.
- **Wall cache keying**: `UPDATE_GFXSET` retains the validated, decoded
  floor/ceiling pair through the M11 frame; a plan whose graphics-set index
  or command hash no longer matches the active map is a blocked no-draw
  frame rather than a fallback-plane paint.
- **Root anchors**: `INTERFACE_GENERAL/0/dt04/0` ceiling record 700 and
  floor record 701 resolve through source root anchors 11 and 14 via the
  `x=1` reference to clip record `(0,0,224,136)`.
- **Creature scene path**: root `DB4` record's `b4` creature type selects
  `CREATURES/<type>/F9` through the map-chip virtual address; corpus-gated,
  fail-closed if the F9 image is absent or rejected.

## 5. DM2 DUNGEON.DAT Differences from DM1

DM2's dungeon format (`G1`) diverges from DM1's in ways proven by corpus
analysis (DM2-001) rather than assumed from DM1 conventions:

- **`GenericRecord::w0` is game data, not a chain link.** On the real PC G1
  `DUNGEON.DAT`, `w0` holds per-record state (e.g. a creature's stored state
  byte) — it is *not* a next-record pointer, unlike patterns assumed
  elsewhere. This is proven on the real 39,437-byte PC `DUNGEON.DAT` (28
  maps, 2,859 records, 2,360 ground-stack entries), where every ground-stack
  entry resolves to a record without needing `w0` traversal.
- **`g1_w0_chains_disabled` flag** (`dm2_v1_dungeon_loader.h:1216`, set in
  `dm2_v1_dungeon_loader.c:487`) disables `w0`-as-next-link traversal
  specifically for real PC G1 data. Synthetic skproject test fixtures do not
  set this flag, so their `w0` chains still resolve as chains — the two data
  sources are handled differently on purpose.
- **No next-thing chain on real data**: `get_next_thing` returns
  `END_MARKER` for the G1 format; `get_thing_record` resolves DB3/DB4
  extension records directly instead of walking a chain.
- **DB3/DB4 extension pools**: the ground-stack table's declared capacity
  exceeds the directly typed roots, so DM2 stores overflow in extension
  pools not present in DM1's format:
  - DB3 extension: bytes `[23826, 29626)`, 8 bytes/entry, extends DB3 index
    space from 299 to 1024 (indices 299–1023).
  - DB4 extension: bytes `[29626, 31658)`, 16 bytes/entry, extends DB4 index
    space from 173 to 300 (indices 173–299).
  - A final 9-byte tail is untyped/unused.
- **`record_graph_complete` flag** reflects whether the loader proved a
  complete, resolvable record graph for the loaded dungeon; true for the
  real corpus, and the extension-pool layout must not be widened without new
  corpus-plus-source evidence.

## 6. Symbol Audit Status

Two audit files track skproject symbol coverage:

- `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` (1,752 lines) —
  per-symbol rows with `symbol`, `family`, `source_file`, `line`,
  `firestaff_mapping`, and `status`.
- `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv` (1,001 lines) — disposition
  notes for symbols that are aliases, host-ABI-only, or otherwise excluded
  from a 1:1 port.

Current status breakdown in the named-symbol audit:

| Status | Count |
|---|---|
| `IMPLEMENTED_PARITY` | 1,118 |
| `NOT_APPLICABLE_ARCH` | 560 |
| `NONAPPLICABLE` | 73 |

There are no rows currently marked `MISSING` in the audit — the backlog has
been fully drained by successive Lane A batches. That backlog was large and
tracked in `TODO.md` release notes; it shrank in stages: 899 → 891 → 883 →
867 → 851 `MISSING` rows across cycles 14–16, continuing through subsequent
Lane A batches (`c_querydb.cpp`, `c_1c9a.cpp`, `c_0aaf.cpp`, `c_1031.cpp`)
until zero `MISSING` rows remained. `SYMBOL_DISPOSITIONS.tsv` separately
excludes symbols tied to host ABIs that Firestaff intentionally does not
emulate: GEM AES trap ABI (68 rows), GEM VDI workstation ABI (23 rows),
Amiga-host device/interrupt/display boundaries (22 rows), original
exception/vector/assembly-stub entries (18 rows), and Atari ST/PRIM linker
aliases (18 rows).

## 7. Lane Status

DM2 (and cross-game) work is organized into parallel development lanes,
each tracked per-cycle in `TODO.md`/`DONE.md`:

| Lane | Focus |
|---|---|
| **Lane A** | DM2 SkWinCore symbol audit batches — closing `MISSING` rows in `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` file by file (`c_querydb.cpp`, `c_1c9a.cpp`, `c_0aaf.cpp`, `c_1031.cpp`, etc.) |
| **Lane B** | DM2 real-data audio/spell backends — e.g. DM2-008 GDAT sound backend, audible playback backend, DM2-007 spell handlers |
| **Lane C** | DM2 real-data startup/dungeon/viewport work — boot/dungeon gate repair, DRAW_ITEM and viewport renderer expansion |
| **Lane D** | Nexus (Sega Saturn) real-data gameplay — creature spawn/combat, HUD integration, altar/AI/sensors/door animation |
| **Lane E** | Theron's Quest (PC Engine) and DM2 combat/drops mechanics — real Track 02 object decode, multi-level progression, DM2 combat/drop follow-ups |

Each lane commits on its own lane branch per cycle; cycles are numbered
sequentially in `TODO.md` (recent entries around cycle 16–17). Lane
assignments are not fixed to one game forever — e.g. Lane E has covered both
Theron's Quest object decode and DM2 combat/drops mechanics in different
cycles.

## 8. FM Towns CD Audio

DM2's FM Towns release uses CD-DA (Red Book audio) tracks for music instead
of the DOS SONG.DAT/Adlib driver. Firestaff's pipeline:

- **Disc classification**: `firestaff_fmtowns_cd_classify` (shared
  include/src) parses redump-style BIN/CUE and ISO/CUE sheets in memory
  (FILE/TRACK/INDEX/PREGAP/REM/CATALOG), distinguishes MODE1/2352 vs
  MODE1/2048 vs AUDIO tracks, and scores the disc against documented DMWeb
  CD-audio track tables. For DM2 FM Towns: audio tracks 2–6 (quieter) plus a
  silent track 8, per `docs/DMWEB_REFERENCE.md` line 51.
- **CDDA pipeline modules**:
  - `src/dm2/dm2_v1_fmtowns_cdda_music.c` / `include/dm2_v1_fmtowns_cdda_music.h`
  - `src/dm2/dm2_v1_fmtowns_cd_dat.c` / `include/dm2_v1_fmtowns_cd_dat.h`
  - `src/dm2/dm2_v1_fmtowns_disc.c` / `include/dm2_v1_fmtowns_disc.h`
  - `src/dm2/dm2_v1_cdda_cd_dat.c` / `include/dm2_v1_cdda_cd_dat.h`
  - `src/dm2/dm2_v1_amiga_cd_dat.c` / `include/dm2_v1_amiga_cd_dat.h` (Amiga CD variant)
  - `include/dm2_v1_fmtowns_music_lookup.h`, `include/dm2_v1_music_map.h`
- **Boundaries kept bounded**: the classifier does not extract ISO 9660 file
  payloads, does not decode IMG2/GRAPHICS.DAT/DUNGEON.DAT, does not launch
  any FM Towns emulator, and does not vendor or hash any game data. Those
  remain `OPEN-BOUNDED` gates in `docs/FIRESTAFF_GAP_LIST.md` until real
  disc-image capture proof is available.
- **Status**: only the game-won/verified tracks are proven playable end to
  end; broader FM Towns CDDA track coverage and extra animation enumeration
  remain open follow-up work (see project memory: DM2 FM Towns status).

## References

- `docs/wiki/DM2-GDAT-Internals.md` — full GDAT format documentation
- `docs/wiki/DM2-Technical-Reference.md` — broader DM2 technical reference
- `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` — per-symbol audit
- `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv` — disposition notes for excluded symbols
- `docs/DMWEB_REFERENCE.md` — DMWeb file-format documentation index
- `docs/FIRESTAFF_GAP_LIST.md` — open/bounded gaps across all five games
- `TODO.md` / `DONE.md` — per-cycle lane work log
