# DM1 legacy inventory verification scope

## Public CI versus original-media tests

The public `verify.yml` CTest step selects `production|source_boundary`.
It does not select the five `dm1_*_names_real` tests below. A successful
public workflow therefore does **not** prove these inventory checks ran.
Licensed game media must remain user-supplied and must not be uploaded to
GitHub to make the public matrix exercise them.

The historical `names_real` names now cover more than object names:

| CTest | Original edition |
|---|---|
| `dm1_amiga_names_real` | Amiga English 2.0 |
| `dm1_amiga_hd_names_real` | Amiga English HD archive |
| `dm1_atari_names_real` | Atari English 1.2 |
| `dm1_atari_de_names_real` | Atari German 1.2 |
| `dm1_atari_fr_names_real` | Atari French 1.3 |

Build `test_dm1_atari_names_real` and `test_dm1_amiga_names_real` in the
chosen build directory. With the original archives available at the paths
configured by CMake, select exactly these tests:

```sh
ctest --test-dir BUILD_DIR --output-on-failure --parallel 2 --no-tests=error \
  -R '^dm1_(atari|atari_de|atari_fr|amiga|amiga_hd)_names_real$'
```

Replace `BUILD_DIR` with the actual build directory. Check that all five
tests **ran and passed**, not merely that CTest exited successfully.
Return code 77 marks absent optional media as skipped; skips are not
positive parity evidence. Other archive-open failures return 1.

The five separately registered `dm1_*_floor_drop_load_real` tests select
the targeted load diagnostic. They verify conserved slot-to-hand weight
and removal of that weight after a normal-input floor drop. A second
champion carries a distinct allocated original weapon. Two-way keyboard
and source-layout name-click leader selection transfers only the held
weight; mouse press/release must not open inventory or repeat the transfer.
Zero-health selection is rejected through both input paths, using a
controlled health change in RAM rather than a complete death sequence.
Their expected weight shares F0140 with the engine; they do not prove
every weight-table entry, full cross-champion inventory interaction,
Modern composed-HUD input, or death/resurrection lifecycle parity.

## Current evidence

The shared test helper `tests/dm1_legacy_scroll_real_check.h` exercises:

- Original scroll panel and font raster in Original/V2.1.
- Original padded C033 border pixels for all 30 inventory slots.
- Independent F0141 object-info index calculation from normalized Thing
  bytes, followed by original graphic-559 G0237 mask lookup.
- Empty-slot placement, pickup and rejection across all 30 slots using
  source G0038 rules, checking hand and resident identity after press/release.
- Occupied action-hand exchanges with a distinct original weapon, in both
  directions, also checking both identities after release.

The corpus uses original records; temporary placements affect RAM only.
It does not create replacement media or write modified game archives.

## Still unproven

Archive ingestion and dungeon normalization, slot-coordinate resolution,
font decoding and scroll line counting remain shared with the engine.
The scroll oracle retains actual transparent background pixels. Occupied
equipment/backpack exchanges, complete drag gestures, load accounting,
save persistence and full same-state emulator rendering are not established
by these checks. Neither this suite nor green CI establishes full DM1,
CSB, or cross-platform gameplay parity.
