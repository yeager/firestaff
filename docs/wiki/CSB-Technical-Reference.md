# Chaos Strikes Back (CSB) Technical Reference

> **Status reviewed 2026-08-11.** CSB has native Amiga, Atari ST and FM Towns
> media paths, broad source-locked engine coverage and ongoing end-to-end
> runtime hardening. It has no original DOS release, so PC-shaped CSB data is
> a reference/compatibility boundary rather than the default route.

## Scope

CSB uses ReDMCSB as its main reference and CSBWin for CSB-specific save,
DSA, champion, and input behavior. Launch needs hash-verified CSB graphics and
dungeon data; missing title, entrance, door, or HUD art is not replaced. See
the shared [game-data format reference](https://github.com/yeager/firestaff/blob/main/docs/GAME_DATA_FORMATS.md)
for the platform-specific IMG1, DMCSB1/LZW, IMG2, save and program-media
boundaries.

## Status: Q-CSB-01 through Q-CSB-10 covered, runtime hardening active

The ten CSB work-queue areas are covered by focused source-lock and runtime
checks. They are not a claim of end-to-end real-data playability:

| Item | Area | Evidence |
|---|---|---|
| Q-CSB-01 | DSA opcode core | 12 test files (9255 lines), 117 unique operations; CSBWin DSA runtime header (998 lines, 264 CSBWin source refs) |
| Q-CSB-02 | DSA monster/world execution | Monster generator gate, timer restart/duplicate policy, door timer handoff, death/damage/feeding/sound/cursor filters, expool recovery, dungeon world mutation, F2262 timer-A events, M11 timer queue resume (14 tests) |
| Q-CSB-03 | Startup presentation chain | C001-C005 FTL/PRESENTS/CHAOS/STRIKES/Entrance; 13/14 startup tests pass (9308 lines) |
| Q-CSB-04 | Entrance and credits handoff | 9 tests: F0128 entrance runtime, F0439/F0441/F0442 start/end boundaries, F0579 bitplanes, F0797 micro dungeon, F0806 entrance loop, F0807 animation step, entrance pointer, opening door tick receipt |
| Q-CSB-05 | HUD and champion panels | C017/C040 champion, inventory, action/spell, cursor, text and transparency rendering — model layer complete |
| Q-CSB-06 | Dungeon viewport geometry | 47 viewport tests: walls D0-D3 all sides, doors, partly-open doors, ornaments, floor/ceiling, backgrounds, center fields, footprints, projectile routing, item explosions |
| Q-CSB-07 | Thing/sensor runtime | 38 sensor/teleporter tests (37 pass, 1 game-data-dependent): F0267-F0276 sensor families, F0247 teleporter/projectile impact, Lord Chaos teleport direction, teleporter rotation |
| Q-CSB-08 | Combat and movement runtime | Grey Lord combat, projectile speed, F0247 teleporter impact/retention, F0266 group move projectile receipt, F0115 projectile viewport rendering |
| Q-CSB-09 | Original saves and Utility Disk | 32 save/utility test files (15 tests pass): save header build/read, native F0435 provenance, export/import, CSBWin save loader boundary, utility save transaction |
| Q-CSB-10 | Media, input and expansion packages | 17 test files, all passing: package identity, sound filter, expansion save identity; keyboard commands test exists (3 game-data-dependent failures) |

## Startup and Dungeon

PRESENTS, CHAOS, STRIKES BACK, entrance, door opening, and HUD handoff are
typed indexed-raster phases with independent palette/timing contracts. They
are not recolored DM1 assets. M12 passes a verified package session to M11.

The M10 F0267 route owns live loaded Thing chains, including cross-level pit
and teleporter movement. It preserves source-tail ordering and restores the
prior current-level context after a transfer.

### Archive and ADF launch ownership

CSB archive and ADF media are scanned as virtual members, but they are never
used as a flat mixed-platform data directory at launch. When a player selects
a verified CSB edition, M12 materializes only that edition's required members
into its version-private local runtime cache and passes that cache to M11.
The cache is hash-verified and invalidated when its source member changes.

This matters for shared game libraries: an Amiga 3.1 title program and its
`GRAPHICS.DAT`/`DUNGEON.DAT` must not be combined with PC, Atari ST, or FM
Towns files found beside the archive. The source archive remains untouched;
the cache is local derived data and is not game media to commit or distribute.
The opt-in CTest `csb_v1_amiga31_m12_m11_real_media_handoff` can be enabled
with `FIRESTAFF_CSB_AMIGA31_DATA_DIR` pointing at a directory containing the
verified Amiga 3.1 ADF or archive. It selects the Amiga package in M12,
verifies its private cache members, and requires M11's native title-to-runtime
handoff; without supplied original media it reports an explicit skip.

The matching CLI regression accepts either form too. It first proves the
native startup page, then runs the source-visible title sequence into the
verified campaign runtime. For A31M this includes APPB's original English-box
mouse-release after `TITL.DAT`; a boot-probe `click:x:y` now emits both the
normal press and release, exactly as the SDL route does. The probe fixes its
presentation to 320x200 so the source coordinates remain deterministic.

The corresponding Atari ST row,
`csb_v1_atari_st_m12_m11_real_media_handoff`, is enabled by
`FIRESTAFF_CSB_ATARI_ST_ROOT`. It separately verifies selection and
materialization of the original Atari package, the ANIMATE.SCR/DAT 50 Hz VBlank
sequence, and the first native FTLCODE runtime frame. The existing
`csb_v1_atari_mini_runtime_archive` test covers the original `MINI.DAT`
resume path from the archive without adding a synthetic save.

The same Atari archive also preserves the retail **Save Disk** as a 720 KiB
MSA/FAT12 medium. Its root directory is genuinely empty: it is formatted
media awaiting the game’s native save transaction, not a carrier for a hidden
`CSBGAME.DAT`. The skip-safe `csb_v1_atari_save_disk_archive` test extracts
only that member, decodes the MSA container, inventories the root directory,
and requires the zero-file result. Firestaff must report this as *no saved
session* and must never invent a replacement save merely to launch it.

FM Towns is treated the same way even though its retail archive contains a
large CD image: temporary expansion happens inside the selected edition's
Firestaff cache. On success it is atomically promoted to `FMTOWNS.IMG`; on
failure the staging file is removed. The directory containing the original
ZIP/RAR remains read-only from Firestaff's point of view.

### PC-9801 HDM preservation receipt (unsupported)

PC-9801 CSB 3.1 is preserved as its own Japanese HDM/floppy family, never as
PC, Atari ST, or FM Towns data. It is intentionally unsupported by Firestaff:
it cannot be selected or launched from the menu or CLI. The byte classifier reads the actual FAT12
root directory using the PC-98 2HD geometry: this corpus stores 1024-byte
sectors and its root begins after one reserved sector plus two two-sector FATs
(offset 5120). A verified root contains `CJDATA` and `CSBGAME`; `FIRES` may
also be present. The classifier reports these directory-slot offsets and the
`pc98-2hd-raw` media shape.

It deliberately leaves the original-versus-cracked and protection fields
unknown for this route. `CSBGAME`'s documented protection offset is relative
to the executable payload, not to its root-directory entry; adding it to a
directory offset would fabricate a result. On this corpus the offset is beyond
the compressed `CSBGAME` file length, so correct classification also needs
bounded FAT-chain extraction and LZEXE expansion. Those steps, file hashes,
the PC-98 three-light-level presentation, native input, and a runtime launch
remain separate proof gates. The receipt test accepts a supplied HDM path or
standard input, so an archive member can be checked without leaving extracted
game data on disk.

### Amiga 3.5 CAPS/IPF preservation boundary

The available Amiga 3.5 CTRaw corpus is not an ADF: each game/utility image
starts with the `CAPS` container signature and contains CAPS/IPF flux-track
records. Those records are evidence of protected original floppy media, not
flat AmigaDOS sectors from which `GRAPHICS.DAT`, `DUNGEON.DAT`, or program
files may be guessed. Firestaff therefore reports this form as unsupported
until a bounded CAPS/IPF decoder can prove its sector reconstruction against
an original 3.5 corpus. It must not relabel the files as ADF, borrow a 3.1
cache, or use a cracked/hard-disk replacement as a silent fallback.

## CSB graphics formats

Amiga CSB graphics use direct **IMG1** nibble-RLE records. Atari ST uses a
563-record **DMCSB1** catalog whose records are Atari-LZW compressed before
their big-endian IMG1 decode. FM Towns uses a distinct **IMG2** record stream
inside its own `0x8001` envelope. These routes are intentionally separate:
neither an IMG3 decoder nor a different platform's pixels may stand in for a
missing native record. C001/C004 are examples of authenticated Amiga IMG1
surfaces, not evidence that every platform shares the same encoding.

## CSBWin DSA Boundary

Extended-save actions are checksum-authenticated and retain source `(dsa,
state, ordinal)` identity. Supported pure stack execution is transactional:
LOAD/STORE, local/global banks, and bounded JUMP/GOSUB transfer chains use the
first source-file-order `(state,column)` match. Type-47 `DSAselector` resolves
through authenticated `DSALevelIndex[level][selector]` data.

DSA execution coverage includes NOOP, EQUAL, QUESTION, and the STKOP families:
Loc2AbsCoord, FetchExCellFlg, BitCount, ParamFetch/Store, GlobalFetch,
PartyDistance, TimeFetch, ThisDSAId, WhoHasTalent, CountInjury, TalentsFetch,
DisableSaves, ChPoss/MonPoss, ExamineCell, Copy, CharFetch/Store,
SwapCharacter, CausePoison, Mastery, MissileInfoFetch/Store, MonsterFetch,
PartyFetch, Override, Message, Overlay, Palette, ExperiencePlus, and
JumpGear/GosubGear. Movement filter, multilevel filter, timer bridge, text
bank, and trigger single-step are all tested. `JUMP`/`GOSUB` follow CSBWin
`Execute` selection rules: state/column lookup picks the first exact
file-order action, JUMP transfers within the bounded execution frame, GOSUB
records a one-frame nested transfer, missing targets end selection without a
synthetic action, and depth/transfer ceilings reject before publication.

Forged actions, malformed extensions, unknown IDs, transfer/depth limits, and
world-mutating opcodes reject before state publication. This is deliberately
not a claim that every CSBWin opcode, ProcessDSAFilter path, or EXPOOL class is
implemented.

## Combat Runtime

Combat integration is tested through DSA `CausePoison` and `CountInjury`
opcodes plus a damage-character filter, alongside dedicated combat modules:
Grey Lord combat, projectile speed tuning, F0247 teleporter impact/retention,
and F0266 group move projectile receipt. C38 creature missiles create
source-owned C14/C49 entries rather than degrading to invented melee.

## Save Ownership

Save interop is anchored on F0435 native provenance, export/import round
trips, the CSBWin save loader boundary, and the utility save transaction path
(32 save/utility test files). CSBWin GAMEBLOCK1/body import rejects malformed
non-empty DB11/EXPOOL tails before atomic runtime staging and records
source-file provenance only after commit.

## Reference Limits

ReDMCSB and CSBWin answer different questions. ReDMCSB is the primary source
for original CSB engine behavior, original EVENT/timeline structure, media
branches, and original save-header contracts. It is not evidence for CSBWin
extensions.

| Area | Primary evidence | What ReDMCSB cannot prove |
|---|---|---|
| Original timers and dungeon mutation | ReDMCSB `TIMELINE.C`, `DUNGEON.C`, `GROUP.C` | CSBWin TIMER queue ownership and `timerObj6/8` semantics |
| CSBWin DSA/custom dungeons | CSBWin `DSA.cpp`, `data.cpp`, `SaveGame.cpp` | EXPOOL, type-47 selector tables, opcode behavior, or DSA state |
| Save interoperability | ReDMCSB `DEFS.H`, `LOADSAVE.C` plus per-media corpus | GAMEBLOCK2, ITEM16, extended tails, EXPOOL, and CSBWin continuation bytes |
| Mouse, audio, interrupt timing | target-media capture; CSBWin host sources for CSBWin paths | vector-dispatched `USIOSTUB.C`, `MUSCSTUB.C`, and interrupt timing as portable behavior |
| Title/entrance/HUD pixels | hash-identified original assets and capture | a universal palette, cadence, or renderer across `MEDIA*` branches |

The source archive contains platform dispatch and assembly segments rather
than a universal host specification. For example, `USIOSTUB.C` forwards mouse
and queued-input calls through library vectors, `MUSCSTUB.C` forwards music
calls, `VBLANK.C` installs interrupt handlers, and `GRAPH21.C` carries
media-specific CPSE/fuzzy-sector logic. These establish that a path exists;
they do not establish equivalent SDL ordering, sampled input, audio mixing,
or copy-protection results.

For any claimed original save format, preserve the evidence tuple:

```text
(media/version hash, original save bytes, parser receipt, runtime capture)
```

For any CSBWin DSA/save claim, preserve the additional tuple:

```text
(CSBWin save hash, timer queue slot, source timer index,
 DSA/EXPOOL record identity, selected source action)
```

Absent those tuples Firestaff must fail closed or state that the route is not
yet verified. The detailed work queue is `REDMCSB-CSB-GAP-001` through
`REDMCSB-CSB-GAP-013` in `TODO.md`. The additional boundaries make clear that
the ReDMCSB rebuild is not a PC-binary oracle, its source does not define a
complete PC boot-media contract, and historical bug notes require an explicit
Firestaff policy rather than automatic reproduction.

## Save and HUD Ownership

Save imports stage GAMEBLOCK, champions, items, timers, extensions, and trace
data before committing. The CSB raster boundary owns title, entrance, door, and
HUD palettes; unresolved skin/EXPOOL data cannot become substitute pixels.

## Verification

```bash
cmake --build build --target test_csb_v1_phase7_verification \
  test_csb_v1_dsa_trigger_single_step_pc34_compat --parallel
./build/test_csb_v1_phase7_verification
./build/test_csb_v1_dsa_trigger_single_step_pc34_compat
```

For authenticated DSA, actuator, save, and raster contracts, see [CSB DSA and
Save Internals](CSB-DSA-and-Save-Internals).
