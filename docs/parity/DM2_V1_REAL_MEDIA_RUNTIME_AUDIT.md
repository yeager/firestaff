# DM2 V1 Real-Media Runtime Audit

Audit date: 2026-08-31
Scope: native DM2 V1 routes using the supplied archive data in memory.  This
audit excludes the explicitly unsupported Macintosh First Chapter demo.

## Verified real-media routes

The following CTest selection passed serially on the supplied media.  The
tests consume archive members directly; they do not require an emulator, BIOS,
or extracted game directory.

The selection was rebuilt and rerun on 2026-08-31: all 44 selected routes
passed. This includes the native CLI/start-menu entries, the archive `SKSAVE`
resume matrix, Mac resource/movie/audio/input paths, Amiga LZX/New Game,
FM Towns M12/M11/title paths, and focused DOS pit/WIELD/spell routes.

The same build audit found and corrected a host-memory boundary defect in the
world-state weather initializer: the destination has the source model's 30
world-state slots, while the previous loop used the unrelated 64-entry dungeon
loader limit. Initialization now uses `DM2_WORLD_STATE_MAX_LEVELS`; the native
minimap/level-transition regression passes with the corrected bound.

The complete labelled DM2 regression suite was then rerun on the same build:
all 87 tests passed. That broader result includes focused source-contract and
fixture isolation tests in addition to the real-media rows catalogued below;
it is not a claim of complete campaign or capture parity.

| Platform / route | Verified boundary | Test |
|---|---|---|
| DOS English | archive identity, native menu, MVE → SKULL → New Game, and observed directional/action input matrix | `dm2_v1_real_media_hash_lock`, `dm2_v1_dos_native_cli_boot` |
| DOS French | native CLI/start-menu admission | `dm2_v1_dos_fr_native_cli_boot` |
| Amiga | native CLI/start-menu, LZX admission, boot, and New Game | `dm2_v1_amiga_native_cli_boot`, `test_dm2_v1_amiga_lzx_real_media`, `test_dm2_v1_amiga_boot_real_media`, `test_dm2_v1_amiga_new_game_real_media` |
| FM Towns | native CLI/start-menu and runtime admission | `dm2_v1_fmtowns_native_cli_boot` |
| Cross-platform | admitted real-media catalogue and G1 world-model handoff | `dm2_v1_four_platform_catalog_real_media`, `dm2_v1_world_model_g1_handoff_real_data` |
| DOS runtime | authentic pit transition, all available archive `SKSAVE` resumes, restored `GAME_LOAD` clock/RNG/serialized weather state, source WIELD miss fail-closed behaviour, and M11 spell cast after resume | `test_dm2_v1_dos_pit_runtime_real_media`, `test_dm2_v1_dos_sksave_archive_resume_real_media`, `dm2_v1_save_load_real_data`, `test_dm2_v1_dos_sksave1_wield_drop_real_media`, `test_dm2_v1_dos_m11_spell_cast_real_media` |

## Interpretation boundary

These results prove the listed boot, handoff, save, and focused runtime paths.
They do not prove a complete campaign, full timing/audio parity, or original
pixel-overlay parity.  DM2 behaviour must continue to cite DM2-specific
SKULL.ASM, SKWin, or skproject evidence; ReDMCSB is authoritative for DM1 and
CSB only.

`dm2_v1_m11_launcher_handoff_boundary` remains a source-contract regression,
not a real-media result in this run: it correctly skipped its corpus-specific
assertion because no explicit DM2 corpus was selected for that standalone test.

## Remaining evidence work

### Original DOS capture intake (2026-08-31)

The external DOSBox capture harness now accepts the supplied
`Dungeon-Master-II-Skullkeep_DOS_EN.zip` directly, uses its retail
`DM2.BAT -> EREGCARD -> IBMIOP -> SKULL.EXE` start chain, and records the
archive stage hash before capture.  The Linux path also handles DOSBox 0.74's
program-derived screenshot names (`splash_*.png`, `ftl_*.png`, and
`intro_*.png`) instead of assuming a host-generated `image*.png` prefix.
The locally inspected output is a genuine original FTL/intro sequence.

The original startup route is now validated through the real menu: after the
intro, `Down` then `Enter` selects the source NEW option and reaches the
original 320x200 dungeon viewport. The locally retained raw frame has
SHA-256 `ec27c77d2a8171b0f30a7067887bb3c883dfd261ccf46d462848b1484949d1d6`.
The capture remains outside version control because it contains copyrighted
game imagery.

Firestaff was separately launched from the same untouched ZIP through M12,
the native PC title/startup route, and source NEW GAME. Its boot receipt was
`phase=dm2-runtime`, `levelLoaded=1`, `party=1,8,0`,
`dm2RealAssets=1`, `dm2NoCoreFallbacks=1`, and `dm2FallbackDraws=0`; the
indexed 320x200 capture SHA-256 was
`57c1195ae270110856e356b673b758e3e04b8deca7d4b0743b21bc180bfb3e1a`.
The corresponding native scene receipt is pinned as `map=0`,
`party=(1,8,0)`, `runtimeTick=15000`, `GRAPHICSSET=2`,
`sceneHash=2792835211`, `sceneColorKey=9`, `sceneFlags=11`, and
`interfacePaletteHash=3200579677`. The boot-probe now prints these fields so
future capture rows can bind the exact source material transaction rather than
only a human menu sequence.

Visual inspection found that this native frame's material composition is not
visually equivalent to the retained original dungeon frame: the Firestaff
frame contains repeated bright/green bands where the original frame contains
the brown dungeon corridor. This is an actionable renderer-parity gap, not a
missing-media or generic palette fallback: the native receipt records real
GDAT assets and zero core fallbacks, while the renderer's local palette
translation is source-owned. It must be resolved by binding a same-state
capture (tuple, tick, map graphics set, local material palettes, and scene
rect plan) and comparing that pair; changing a global presenter palette alone
would not address this material/scenography discrepancy.

Source review subsequently repaired two concrete presentation primitives in
the c_gfx_main compatibility layer. `DM2_DRAWINGS_COMPLETED` now resolves
expanded screen rectangle 7 before it presents the 224x136 backbuffer, rather
than reusing the backbuffer-local rectangle at `(0,0)`. `_specialblit` now
matches the original bit-15 transition flag: the normal `0x0008` completion
call is a direct blit and only `0x8008` requests the progressive stretch.
`dm2_v1_gfx_main_pc34_compat` covers both rules. These repairs remove known
source divergences, but they do not by themselves establish the missing
same-state frame pair or a full visual-parity claim.

### RAW4 viewport-aperture decoding (corrected 2026-08-31)

The live boot profile's compressed-rectangle decoder contained a second,
independent presentation defect.  The retail `GRAPHICS.DAT` RAW4 record for
rectangle 7 is the simple source root-to-marker form: its root anchors the
screen destination at `(0,40)` and marker 3 supplies the `224x136`
backbuffer dimensions.  The prior decoder applied that root offset again when
it encountered the marker and reported `(0,80,224,136)`.

`dm2_v1_boot_expand_hud_rect` now follows the original
`DM2_QUERY_BLIT_RECT` ownership for that form and reports `(0,40,224,136)`.
`test_dm2_v1_boot_profile_smoke` queries the real archive-backed profile and
asserts the rectangle plus its RAW4 provenance receipt (current archive proof:
`raw4=67fa6c8c`, receipt `4140d3d5`).  Archive virtual members remain in
memory during that test; a renamed-loose-file regression is explicitly skipped
for an archive member rather than copying or extracting it to disk.

This is still not a same-state pair. The original tuple/tick and its exact
viewport/palette state have not yet been captured and bound to the Firestaff
tuple. The frames are therefore not promoted to an overlay baseline or a
pixel-parity claim.

### Runtime viewport compositor boundary (confirmed 2026-08-31)

The source implementation makes the remaining renderer work concrete. In
`SKULLWIN/c_gui_vp.cpp::DM2_DISPLAY_VIEWPORT`, `DM2_INIT_BACKBUFF()` creates a
separate `224x136` dungeon surface before the source calls
`DM2_DRAW_DUNGEON_GRAPHIC` for ceiling (`GRAPHICSSET/1`, rectangle `0x2bc`) and
floor (`GRAPHICSSET/0`, rectangle `0x2bd`), then executes environment, tiles,
and player passes. `SKULLWIN/c_gfx_main.cpp::DM2_DRAWINGS_COMPLETED` presents
that surface through expanded `RECT_7`; the normal source call is
`_specialblit(..., 0x0008)`, while bit 15 selects the transition stretch.

Firestaff now matches the `RECT_7` decode and the `0x0008`/`0x8008` decision in
its isolated `c_gfx_main` compatibility layer, but its active M11 DM2 runtime
still composes a fixed `320x200` surface directly. The retained pre-fix
authentic DOS new-game capture recorded the resulting repeated banding when
real GDAT material was used. That was a compositor/geometry gap, not
permission to replace the frame with generated art or to relabel it as
palette-only.

The authenticated `GRAPHICSSET` plane route now submits each decoded
`RECT_700`/`RECT_701` image once to its source-owned destination, instead of
repeating IMG3 rows as a host texture. The archive-backed plan test compares
every resulting plane pixel (including its source local palette and initial
source flip) to the selected retail GDAT bytes. This removes the known striped
plane failure, but it does not claim that the remaining dungeon, HUD, or
`RECT_7` presentation passes have reached frame parity.

The next implementation must introduce a source-sized dungeon backbuffer,
bind the actual `RECT_7` destination at `(0,40,224,136)`, and port source
`DM2_DISPLAY_VIEWPORT` pass ordering into that surface. It must retain the
current in-memory GDAT ownership checks and verify the same new-game tuple
against an original capture before promoting a visual-parity result.

- Real long-route captures for combat resolution, creature drops, weather,
  audio and level transitions on each admitted platform.
- Representative original-frame overlays for dungeon, HUD, doors and
  outdoor rendering.
- Long-route capture evidence for the non-serialized weather globals
  (`v1e146e`, cloud timer and transient command gates). Resume now restores
  every weather field serialized by `s_savegamebuffer`, derives `v1e1472`
  and the fixed `v1e1438` day offset from their source GDAT queries, and
  keeps the remaining fields unavailable rather than inventing them.
- Source-bound consumption of the restored delayed-movement fields. Resume
  now retains serialized `v1e026e`, `v1e025e`, and `v1e0274`, and applies
  the source tick's documented decrements and direction-specific
  `c_input.cpp` admission formula. A committed step now calculates the
  maximum `DM2_CALC_PLAYER_WALK_DELAY` across the loaded party, includes the
  event hero's source-record/GDAT-resolved carried item weight, consumes the
  matching strength-enchantment `c_random` draw, clears `v1e025e`, and adds
  the source half-delay to `v1e026e`. The unbound `v1e025c` delayed-pose term
  remains explicitly false: Firestaff does not synthesize a pose or viewport
  interpolation without the original owner.

No synthetic fixture may be promoted as evidence for these remaining items.
