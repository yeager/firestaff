# Firestaff Dungeon (`.fsdung`) format

`FSDG` is Firestaff's portable, editable dungeon interchange format.  It is
not a replacement for original game data: it describes a new dungeon target
and its optional author-supplied PNG graphics.  Firestaff Dungeon Studio 0.2
can create, open, and save it for DM1, Chaos Strikes Back, DM2, Theron's Quest,
and Nexus.

All multibyte values are little-endian.  Version 1 begins with a 16-byte
header:

| Offset | Size | Meaning |
| --- | ---: | --- |
| 0 | 4 | ASCII `FSDG` |
| 4 | 2 | Format version (`1`) |
| 6 | 1 | Map count (1–32) |
| 7 | 1 | Target game: DM1=1, CSB=2, DM2=3, Theron=4, Nexus=5; zero means legacy DM1 |
| 8 | 2 | Party X |
| 10 | 2 | Party Y |
| 12 | 2 | Party direction |
| 14 | 2 | Reserved, zero |

The header is followed by a 32-byte descriptor and UTF-8 name for each map,
then each map's tile array and 8-byte thing records.  An optional graphics
section follows the maps: a `u16` count and repeated UTF-8 key / `u32` byte
length / PNG-byte payloads.  Limits, validation, and the corresponding C
loader are defined in `include/firestaff_fsdung_loader.h`.

The target-game byte is deliberately in a previously reserved byte so existing
version-1 readers retain their historical DM1 interpretation.  It identifies
the intended ruleset; it does not convert or overwrite original game files.
