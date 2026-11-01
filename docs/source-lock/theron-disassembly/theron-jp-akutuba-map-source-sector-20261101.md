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
tile values without modifying the media.
