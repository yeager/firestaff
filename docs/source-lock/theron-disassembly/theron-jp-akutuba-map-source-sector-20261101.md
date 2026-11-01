# JP Akutuba map source-sector trace target (2026-11-01)

## Source identity and address conversion

This source-only trace plan uses the authenticated Japanese Rev. 1 Track 02
BIN (MD5 `b7afb338ad31be1025b53f9aff12d73a`, SHA-256
`d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb`). The
file contains 3,445 MODE1/2352 sectors. The current map decoder addresses a
user-data projection with a 225-sector prefix
(`src/theron/theron_v1_track02_dungeon_map.c`); the authenticated JP CUE places
INDEX 01 at raw file sector 224 and Track 02 begins at disc LBA 3590.

For a projection user-data offset `u`, the raw track-file sector is
`floor(u / 2048)`, the byte within its user-data payload is `u % 2048`, and
the raw BIN byte is `sector * 2352 + 16 + payload_byte`. Convert that file
sector to a Track 02 relative LBA by subtracting 224, then to disc LBA by
adding 3590. All spans below were read from this verified BIN; no fixture or
generated map bytes were used.

## Akutuba source spans

The authenticated Japanese offset table in
`theron_v1_track02_dungeon_map.c` gives descriptor offset `$2991D` and tile
array offset `$29B03`. With the decoder's 225-sector user-data prefix, both
the 68-byte descriptor and all four map arrays are in the same raw MODE1
sector: track-file sector 308, Track 02 relative LBA 84, disc LBA 3674.

| Source bytes | Projection user range | Raw BIN range | Disc LBA |
|---|---|---|---:|
| Descriptor and map dimensions | `[$9A11D,$9A161)` | `[$B0EED,$B0F31)` | 3674 |
| Map 0 tile array (6 × 8) | `[$9A303,$9A333)` | `[$B10D3,$B1103)` | 3674 |
| Map 1 tile array (19 × 13) | `[$9A333,$9A42A)` | `[$B1103,$B11FA)` | 3674 |
| Map 2 tile array (19 × 17) | `[$9A42A,$9A56D)` | `[$B11FA,$B133D)` | 3674 |
| Map 3 tile array (13 × 12) | `[$9A56D,$9A609)` | `[$B133D,$B13D9)` | 3674 |

The column-major tile bytes at `(2,3)` are map 0 `$20` at user offset
`$9A316` / raw offset `$B10E6`, and map 2 `$00` at user offset `$9A44F` /
raw offset `$B121F`. Their difference can discriminate these two source maps
only after the game-owned active selector, party pose and tile consumer have
been observed in the same run.

## Capture use and limits

Use disc LBA 3674 (or the equivalent Track 02 file-sector/user-byte range) as
the narrow source-read filter for a cold JP capture. Preserve a shared event
sequence from the read through its System Card or game-owned RAM destination,
the active-map selection, and a tile read at the captured pose; pair it with
atomic VDC/VCE/VRAM and an accepted-input/no-input control. A static media
location does not show that the original reads, selects, copies, or consumes
these bytes. It does not identify a runtime map, player coordinate, visual
meaning, or renderer behavior. Current cold JP RUN captures stop in the System
Card and report `transition=missing`, so no such game-owned join is claimed.

Recompute the authentic-media target with:

```sh
python3 tools/plan_theron_jp_akutuba_map_trace.py \
  "$HOME/.firestaff/data/theron/TQJP02.bin"
```

The tool checks the complete retail identity, parses dimensions from the
source descriptor, derives raw-sector and disc-LBA ranges, and prints source
tile values without modifying the media. Its optional `--dump-grids` mode
prints every Akutuba map byte with rows indexed by `y` and columns by `x`, while
reading the authentic column-major arrays. It also prints descriptor bytes
through the parser's field labels, retaining `unknown_1` and `unknown_2` as
uninterpreted values. These static IDs and metadata do not identify the
runtime-selected map, decode tile meaning, or authorize rendering.

## Offline save-state source match (2026-11-01)

An offline byte scan of an existing JP Mednafen `.mc0` save state found the
complete 1,260-byte source span from the descriptor through the end of all four
Akutuba maps exactly once in the gzip-inflated state payload at offset
`0x6633b`. The state file SHA-256 is
`b4f2a06788e4723110b19892f93e8ac4aef1b5c18c6358a9fe36f1baf747b068`; the
matched source span SHA-256 is
`a8f780a39eee82ef9a36df53808de4e60bef7f05205c660e45f7d7c50d53e615`. Each
grid also occurs once in source order in that payload. This confirms that all
four authentic map arrays are present in the state. It does not identify the
active array: all four are resident.

The nearby screenshot is timestamped 49 seconds after the state file, but no
separate screenshot receipt binds it to this exact state. The `.mc0` itself
contains an embedded 341×232 RGB preview and named VCE/VDC0/VRAM/SAT state
variables, so those components form one atomic saved-state snapshot. A
runtime consumer/selector join, pose binding, cold CD-to-RAM provenance, and
rendered-response evidence remain open. This static save-state match does not
authorize runtime map selection or rendering.

## Named Mednafen state mapping (2026-11-01)

[`inspect_theron_mednafen_pce_fast_state.py`](../../../tools/inspect_theron_mednafen_pce_fast_state.py) reads the gzip-compressed
state only when its header reports Mednafen 1.32.1 (`0x00103201`), validates
the complete section and variable boundaries, and matches the full source span
against the authentic JP Rev. 1 Track 02 BIN. Its bounded structural regression
uses a minimal format-only record; the result below comes from the authentic
state and media files named above.

The unique match is in `HuC.ROMSpace + 0x68 * 8192`, a 262,144-byte variable
declared by Mednafen 1.32.1 `src/pce_fast/huc.cpp` in `HuC_StateAction`. The
source span begins at variable offset `0x1891d`, which corresponds to ROMSpace
page `$74`, page offset `$091d`, and physical address `$0E891D`. The four map
arrays begin at variable offsets `$18B03`, `$18B33`, `$18C2A`, and `$18D6D`.
Mednafen's saved `CPU.MPR` bytes are `FF F8 68 6B 78 70 69 00`; none selects
page `$74`. `MAIN.BaseRAM` contains no contiguous copy of any of the four
source arrays. These facts confirm where the source bytes are resident in
this snapshot, but do not show whether another MPR mapping or decoded copy is
used during gameplay.

The same state includes the preview RGB SHA-256
`f6341ff7e8f4bc52c7f6c1f8bd2747b283b27f727f59cefe8a348a8cda33eca4`, VCE
color-table SHA-256
`ba02dbd8f03665cce75dae462f53c0120757adc5864d751693d0744e2d0b2875`, VDC0
VRAM SHA-256 `56b6e2b7b502be2cc0c2ad33ad943dc30aa8dbb1e3af2dcaaf3acaa80c92baa6`,
and SAT SHA-256
`46f243e653e59d5558b831051ed9ff4dd9cf70498115b1a5ab02eaa1847b1fb4`. The
snapshot is suitable for later state-bound capture analysis, but it does not
identify the active map, party pose, tile read, or Firestaff's production
renderer input. The next reference trace must observe the game after state
load, including any remapping or copy, active selector, and pose-bound tile
consumer, before changing native rendering.
