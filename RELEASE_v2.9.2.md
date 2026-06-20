# Firestaff v2.9.2

Released 2026-06-20. 59 commits since v2.9.1.

This release closes **all nine Tier 2 items** in `docs/FIRESTAFF_GAP_LIST.md`,
starts **Tier 3 #10 (FTL container decoder)**, ships the new CSB Utility Disk
.CMP / Atari ST PAK / CSB hidden-code table decoders, fixes the DM1 V1
graphics LZW decoder, hardens the M12 launcher WASD handling, adds the
`asset-validate` Python tooling family (`compare_to_greatstone.py`,
`compute_md5_for_registry.py`, `compare_md5_to_sha256.py`,
`coverage_by_game.py`), brings Lefthook into CI, and completes the
19-language launcher localization cycle.

## Highlights

### CSB hidden-code skip table (Tier 2 #3)

CSB Atari ST and CSB Amiga `GRAPHICS.DAT` items 558–562 are 68k executable
code, not image/text/sound data. `src/csb/csb_hidden_code_skip_table.c` +
`include/csb_hidden_code_skip_table.h` add a per-game/per-platform skip
table that the upcoming CSB V1 graphics loader can route around. Also
covers the DM1 Amiga v2.2 "kid dungeon" easter-egg items 135–138.
`ctest -R csb_hidden_code_skip_table_unit` passes (test #596).

### DM1 V1 graphics LZW decoder fix (Tier 2 #4)

The existing `m11_gfx_lzw_decompress` in
`src/dm1/dm1_v1_graphics_loader_pc34_compat.c` had two bugs that caused
wrong pixels when Atari ST GRAPHICS.DAT was decoded: CLEAR_CODE rewound
the bitstream by also resetting `byte_pos` and chunk indices, and the
KwKwK edge case emitted `first_char` before the decoded old-code string.
Split `lzw_reset()` into `lzw_init()` (full stream + dict reset at start)
and `lzw_reset_dict()` (dict/code-width only, preserves bitstream
position). New `tests/test_dm1_lzw_round_trip.c` exercises single-byte,
literal runs, repeated pattern, KwKwK, clear-code resets, early END_CODE,
and code-width growth (8/8). Source-locked to ReDMCSB `LZW.C`
`F0495_GetNextInputCode`, `G0666 max=4096`, 12-bit codes.

### Atari ST PAK decoder (Tier 2 #5)

`src/shared/firestaff_pak_decode.c` + `include/firestaff_pak_decode.h`
parse the 4-byte PAK header (file size in words), the 28-byte Atari ST
executable header (magic `0x601A` + text/data/bss/symbol sizes + flags),
the 1920-word most-frequent-words table, and the nibble-coded compressed
stream. The nibble coding (`0xF` → two literal bytes, `0x8..0xE` → 12-bit
table index, `0x0..0x7` → 8-bit table index) is the same algorithm that
FTL HUNK_CODE uses — the building block for Tier 3 #10. `ctest -R
firestaff_pak_decode_unit` passes.

### CSB Utility Disk .CMP portrait decoder (Tier 2 #6)

`src/shared/firestaff_cmp_decode.c` + `include/firestaff_cmp_decode.h`
parse the 496-byte on-disk `.CMP` structure per ReDMCSB `DEFS.H`
(`cmp_i_C`, `cmp_i_E`, `Name[8]`, `Title[20]`, `Portrait[464]` at 32×29
4bpp). Read-only parser — exposes the portrait as a pointer into the
caller's input buffer. Amiga↔Atari ST bitplane conversion (F0515/F0516
in PORTRAIT.C) is deferred to Tier 3 rendering work. `ctest -R
firestaff_cmp_decode_unit` passes.

### MD5 / SHA256 harmonization (Tier 2 #7)

Source runtime asset matching uses MD5 (`asset_find_by_hash.c`,
`asset_status_m12.c g_requiredFiles`); `docs/VERIFIED_HASHES.md`
documents SHA256. Rather than migrate everything, `tools/asset-validate/`
gains `compute_md5_for_registry.py` (cross-table generator) and
`compare_md5_to_sha256.py` (invariant check). `docs/MD5_SHA256_HARMONIZATION.md`
documents the policy. Hashes added for Atari ST DM1 1.2 EN, CSB Atari ST
2.x, DM2 PC FR/DE/JewelCase and Mac EN so the registry now covers the
variants we physically have on disk.

### Lefthook in CI (Tier 2 #9)

`lefthook.yml` hooks: `newline_eof`, `trailing_whitespace`,
`po/validate_po_layout.sh`, hash harmonization, Python syntax checks.
`.github/workflows/verify.yml` gains an `asset-hygiene` job (ubuntu-24.04)
that installs `lefthook@latest` via `go install`, then runs hash
harmonization, PO layout validation, and `lefthook run ci`.

### Data acquisition checklist (Tier 2 #1)

`docs/DATA_ACQUISITION_CHECKLIST.md` — per-game matrix of which
platform/version variants Firestaff needs to be code-complete, which
files are present locally (SHA256-verified), which are archived but need
extraction, and which are still missing. Covers all five supported
games across all known platforms from Greatstone's 26+ extraction
matrix.

### Per-game data coverage view (Tier 2 #2)

`tools/asset-validate/coverage_by_game.py` reports a per-game,
per-variant coverage table by cross-referencing the registry, the local
data directory, and the data-extras directories. Variants are scored
**READY** / **ARCHIVED** / **MISSING**. Current totals:

| Game   | Total variants | Ready | Archived | Missing | Runtime-ready |
|--------|---------------:|------:|---------:|--------:|--------------:|
| DM1    | 14             | 13    | 1        | 0       | 92%           |
| CSB    | 9              | 8     | 1        | 0       | 88%           |
| DM2    | 9              | 9     | 0        | 0       | 100%          |
| Nexus  | 1              | 1     | 0        | 0       | 100%          |
| Theron | 2              | 2     | 0        | 0       | 100%          |

`ctest -R asset_validate` passes 3/3 (compute, compare, coverage).

### FTL container decoder header (Tier 3 #10, started)

`include/firestaff_ftl_decode.h` defines the public API for the
proprietary FTL resource container used by Atari ST / Amiga / X68000 /
MegaCD / SegaCD versions of DM1, CSB, DM2, and Nexus. The 20-byte common
header (magic `0x6160`, checksum, unknown1=2, c_6=1, c_7=0, i_8=7,
date1, date2, segment_count) and 12-byte segment headers
(HUNK_BSS `0x0010` / HUNK_DATA `0x0011` / HUNK_CODE `0x0012`) match
ReDMCSB `FTL.H` exactly. HUNK_CODE decompression will delegate to
`FirestaffPak_Decode` (same `0x5223` magic + 1920-word frequency table
+ nibble-coded stream). Implementation, tests, and CMake registration
in the next commit.

### WASD navigation fix (commit `0ad63f50`)

W/A/S/D are now an unconditional alias for the arrow keys. Previously
the keys were gated by `state->settings.wasdMovementEnabled`, which
silently disabled WASD when the M12 toggle was off. Removed the gate in
both SDL3 and SDL1/SDL2 fallback branches in `m11_poll_menu_input`;
Q/E and Home/End turn behavior unchanged, Ctrl+S save-game behavior
preserved. Settings UI no longer shows the WASD MOVEMENT row (it is now
a reserved enum slot for config/backward compatibility).

### i18n follow-up (commit `cf424cb7`)

All 7 localization domains — startup-menu, dm1, csb, dm2, firestaff,
nexus, theron — now have PO files for 19 locales with structural
validation passing. ~12,168 translation entries. Swedish remains the
only fully native layer; other locales are machine translations seeded
from the English source. DM2 is a 0-msgid structural placeholder.
`po/validate_po_layout.sh` covers all 7 domains and reports per-locale
nonblank coverage.

### Front-wall inscription bug (BUG-DNY-DM1-2026-06-16, commit `c81d85b3`)

Fixed blurry/double-exposed front-wall inscriptions in the M11 game
view. Source-locked to ReDMCSB DUNGEOG.C inscription plane handling.

## Verification

```
$ ./build/firestaff --version
Firestaff v2.9.2

$ ./build/firestaff --scan-data
dm1: READY (5 required files hash-verified)
csb: READY
dm2: READY
nexus: READY
theron: READY

$ ./build/firestaff_m11_phase_a_probe
firestaff_m11_phase_a_probe: 23/23 invariants passed

$ ctest --output-on-failure
... ~595 tests passed, 0 failed

$ python3 tools/asset-validate/compare_md5_to_sha256.py
PASS: all runtime MD5 + registry SHA256 agree

$ python3 tools/asset-validate/coverage_by_game.py
dm1: total=14  ready=13  archived=1  missing=0   (92% runtime-ready)
csb: total=9   ready=8   archived=1  missing=0   (88% runtime-ready)
dm2: total=9   ready=9   archived=0  missing=0  (100% runtime-ready)
nexus: total=1 ready=1   archived=0  missing=0  (100% runtime-ready)
theron: total=2 ready=2 archived=0  missing=0  (100% runtime-ready)
```

## Documentation

- `docs/DMWEB_REFERENCE.md` — dmweb.free.fr + greatstone.free.fr/dm/
  consolidated reference.
- `docs/PLATFORM_MATRIX.md` — canonical game/version support matrix.
- `docs/FIRESTAFF_GAP_LIST.md` — cross-game meta-analysis tracking
  Tier 1 (data blockers) through Tier 6 (launcher/accessibility).
- `docs/MD5_SHA256_HARMONIZATION.md` — MD5↔SHA256 policy.
- `docs/DATA_ACQUISITION_CHECKLIST.md` — per-game data-acquisition matrix.

## Compatibility

Pure additive release — no breaking changes to runtime config, save
games, or asset layout. Saved games from v2.9.0 and v2.9.1 load
unchanged. Hash-verified registry now covers more variants; existing
local data continues to match.

## Upgrade notes

Nothing required. Replace `Firestaff.app`, `firestaff.exe`, or the
Linux binary in your existing install location. The asset-hygiene CI
job runs on every PR going forward; contributors should
`go install github.com/evilmartians/lefthook@latest` once locally so
the local pre-commit hook matches CI.

## Source-lock citations

All Tier 2 / Tier 3 work is source-locked:

- LZW decoder: ReDMCSB `LZW.C` `F0495_GetNextInputCode`, `G0666`
  max=4096, 12-bit codes.
- PAK decoder: ReDMCSB `DECOMPCO.C` `F0913_DecompressPAK`, plus
  Greatstone PAK format spec.
- CMP decoder: ReDMCSB `DEFS.H` `CMP` struct, `PORTRAIT.C`
  `F0515_CHAMPION_ConvertPortraitsToAtariSTPlanar` and
  `F0516_CHAMPION_ConvertPortraitsFromAtariSTPlanar`.
- FTL container: ReDMCSB `FTL.H` `HEADER` + `SEGMENTHEADER`.
- IMG5 decoder: Greatstone Items format spec (4bpp planar, MSB to LSB).
- Hidden-code skip table: dmweb Encyclopaedia "Graphics.dat: Hidden
  code" page.

## Credits

Same team as v2.9.1. ReDMCSB decompilation team, Greatstone/SCK project,
CSBWin / Paul R. Stevens, skproject / Sphenx, the Dungeon Master
Encyclopaedia (dmweb.free.fr), and all the FTL Games alumni.