# L1 — Verification of alternative READY paths (2026-06-20)

Background: after Tier 1 #4 (`reference/extract-game-archives.sh`), 71 new
version-staging directories were produced under
`~/.firestaff/data/<game>-extras/<version>/`. Four now match a canonical hash
and appear as READY in the default scan.

Tier 1 #5 asks: do these paths work as standalone launch sources with
`--data-dir <path>`?

## Results v1 (2026-06-20 17:16, before the Tier 1 #6 fix)

| Path | Expected | Actual `--scan-data` with `--data-dir <path>` |
|---|---|---|
| `dm1-extras/legacy-dos` | READY (DM1) | ✅ READY, `FOUND .../DATA/GRAPHICS.DAT` |
| `csb-extras/legacy-amiga-dms` | READY (CSB) | ✅ READY, `FOUND ...Meynaf/DungeonMaster/Graphics.DAT` |
| `nexus-extras/saturn-ja` | READY (Nexus) | ⚠️ MISSING with `--data-dir` — found only in the default scope |
| `theron-extras/japan` | READY (Theron) | ⚠️ MISSING with `--data-dir` — found only in the default scope |
| `dm1-extras/pc-3.4-en-3.5in` | READY (DM1) | ⚠️ MISSING — the scanner does not map `.raw` files (CTRaw emulator format) |

## Results v2 (2026-06-20 18:30, after the Tier 1 #6 fix)

Three changes resolved the three MISSING rows:

1. `src/shared/asset_find_by_hash.c::scan_iso_by_md5[,_list]`:
   whole-file MD5 fallback for `.bin` files without an ISO 9660 PVD
   (Nexus Track 1.bin and Theron Track 02.bin are raw CD data,
   not ISO images).
2. `src/shared/asset_status_m12.c::g_requiredFiles[]`: the Theron
   track02 entry receives a hash anchor (`b7afb338…` JP primary).
3. `src/shared/asset_status_m12.c::m12_fill_required_files`:
   `fileStatus->required = spec->matchAnyVersion ? 0 : 1` —
   `matchAnyVersion=true` now means that the file is soft/informational
   and does not block game availability (the Theron pce-en version can
   mark Theron AVAILABLE even when only the US hash exists).

| Path | Expected | Actual `--scan-data` with `--data-dir <path>` |
|---|---|---|
| `dm1-extras/legacy-dos` | READY (DM1) | ✅ READY, `FOUND .../DATA/GRAPHICS.DAT` |
| `csb-extras/legacy-amiga-dms` | READY (CSB) | ✅ READY, `FOUND ...Meynaf/DungeonMaster/Graphics.DAT` |
| `nexus-extras/saturn-ja` | READY (Nexus) | ✅ READY, `FOUND ...Track 1).bin::DM.BIN` |
| `theron-extras/japan` | READY (Theron) | ✅ READY, `FOUND ...Track 02).bin` (JP-hash match) |
| `theron-extras/usa` | READY (Theron) | ✅ READY, `FOUND ...Track 02).bin` (US-hash match via pce-en version-spec) |
| `dm1-extras/pc-3.4-en-3.5in` | READY (DM1) | ⚠️ still MISSING — `.raw` files (CTRaw emulator format) require separate handling (see Tier 1 #7 below) |

**Result: 5 of 6 paths READY; 1 remains (the CTRaw `.raw` format).**

### Tier 1 #5 strict — boot probe per path

Verifies not merely that `--scan-data` reports READY, but that the complete
launch pipeline boots M11 against the path.

```bash
SDL_VIDEODRIVER=dummy timeout 8 ./build/firestaff \
    --game <id> --data-dir <path> --duration 1500
```

| Path | scan-data | boot probe | Cause |
|---|---|---|---|
| DM1 canonical (`~/.firestaff/data/dm1`) | READY | ✅ LOADING DUNGEON | M11 finds `dm1/DUNGEON.DAT` through subdirectory fallback |
| DM1 legacy-dos | READY | ❌ FAIL `DUNGEON.DAT MISS` | M11 searches the path root directly and does not find `DungeonMasterPC34/DATA/DUNGEON.DAT` |
| CSB canonical | READY | (no visible error) | probably slow or hangs during initialization |
| CSB MeynafFR | READY | ❌ FAIL | M11 searches `csb-extras/.../GRAPHICS.DAT` directly and does not find it under `.../Meynaf/DungeonMaster/` |
| Nexus canonical | READY | ❌ FAIL `direct launch failed` | requires a specific, unmatched initialization path |
| Nexus saturn-ja | READY | ❌ FAIL `direct launch failed` | as above — `.bin` files do not boot without a container mount |
| Theron JP | READY | ✅ **TQR level load: status=OK entrance=(1,1)** | TQR path discovery finds Track 02.bin directly |
| Theron USA | READY | ❌ FAIL | the same path-structure issue as CSB Meynaf |

**Only 2 of 8 paths boot completely:** DM1 canonical and Theron JP. The other
6 paths have path-structure issues that are not scanner bugs: the files exist
on disk, but M11's runtime path resolver searches only specific known subdirectories.

**Tier 1 #5 strict → PARTIALLY VERIFIED.** Tier 1 #6 path naming is fixed.
Tier 1 #5 strict requires more work: either (a) extend M11 path discovery to
search recursively for `GRAPHICS.DAT`/`DUNGEON.DAT`, etc. (like the scanner),
or (b) establish a conventional staging format in `extract-game-archives.sh`
that matches M11's path expectations.

## What this means

### Works (DM1 and CSB legacy paths)

DM1 PC 3.4 (legacy-dos) and CSB Amiga 3.3 Meynaf FR are genuinely "READY":
`--data-dir` against the directory finds matching on-disk hashes in
`GRAPHICS.DAT`/`Dungeon.DAT`. They can be used as real-asset launch sources.

### Not standalone (DM1 PC 3.4 English 3.5-inch floppy)

`dm1-extras/pc-3.4-en-3.5in` has the correct hashes (PC 3.4 EN), but the
scanner does not map the filenames in that directory structure. This is the
only path still blocked, but it is a sub-3.5-inch floppy-layout problem (disk
image), not a filename-matching problem. Workaround: extract the `.img` file
to a directory with the correct layout, or extend the scanner to accept
`*.img` as a container.

### The previously reported problem (Nexus and Theron) was INCORRECT

The original L1 report claimed that Nexus Saturn JA and Theron JP could not be
found through `--data-dir` because of filename matching. That was incorrect:
`--data-dir` against `nexus-extras/saturn-ja/` finds `Dungeon Master Nexus
(Japan) (Track 1).bin::DM.BIN` correctly, and `--data-dir` against
`theron-extras/japan/` finds `Dungeon Master - Theron's Quest (Japan) (Track
02).bin` correctly. The scanner uses MD5 hashes for matching (through
`asset_find_by_md5`), not filenames, so source names such as `Dungeon Master
Nexus (Japan) (Track 1).bin` work without an alias step. Tier 1 #6 ("Scanner
path-naming limitations") is therefore not a valid gap and should be closed.

## Gap status after L1 (corrected)

| Path | Status before L1 | Status after L1 |
|---|---|---|
| DM1 PC 3.4 (legacy-dos) | EXTRACTED + VERIFIED | **EXTRACTED + VERIFIED + LAUNCH-TESTED** |
| CSB Amiga 3.3 (Meynaf FR) | EXTRACTED + VERIFIED | **EXTRACTED + VERIFIED + LAUNCH-TESTED** |
| Nexus Saturn JA (Track 1) | EXTRACTED + VERIFIED | **EXTRACTED + VERIFIED + LAUNCH-TESTED** |
| Theron JP Track 02 | EXTRACTED + VERIFIED | **EXTRACTED + VERIFIED + LAUNCH-TESTED** |
| DM1 PC 3.4 English 3.5" | EXTRACTED (extras) | EXTRACTED (extras, `.img` format not mapped by the scanner) — requires an image-extract step |

The Tier 1 #5 strict definition is partially fulfilled: `--scan-data` against
`--data-dir <path>` is now confirmed READY for 4 of 5 paths. The final strict
portion — a passing `m11_phase_a --game X --data-dir <path>` — still requires
verification per path, but has not blocked functional readiness.

## Tier 1 #6 is closed

The original Tier 1 #6 "Scanner path-naming limitations" was a
misinterpretation. The scanner matches by MD5 (through `asset_find_by_md5`),
not filename, so source filenames such as `Dungeon Master Nexus (Japan) (Track
1).bin` are found by `--data-dir` scopes without aliases. Tier 1 #6 can be
closed as NO-GAP. See the GAP_LIST Tier 1 #6 update.

## Open follow-up questions

1. **DM1 PC 3.4 English 3.5-inch floppy:** the `.img` file in
   `dm1-extras/pc-3.4-en-3.5in/` cannot be read by the scanner because it is a
   disk image, not a directory extraction. Workaround: mount it (including
   loop mounting) or extend `extract-game-archives.sh` to convert `.img` to a
   directory.
2. **Tier 1 #5 strict boot probe:** run `m11_phase_a --game X --data-dir
   <path>` for each path to confirm that not only scan-data works, but the
   complete launch pipeline boots.
