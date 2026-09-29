# DM1 SONG.DAT — format specification (PC v3.4)

Pass 50 — source-backed documentation of the original Dungeon Master PC
v3.4 `SONG.DAT` file, as a prerequisite to replacing Firestaff's V1
procedural audio placeholders with original-faithful samples.

Format evidence comes from the following community references:

- dmweb — DM/CSB/DMII data-files format spec (DMCSB2 container,
  item-type map, SEQ2, SND8 decode algorithm)
  <http://dmweb.free.fr/community/documentation/file-formats/data-files/>
- Greatstone — per-item semantic labels for SONG.DAT DM PC v3.4
  <http://greatstone.free.fr/dm/db_data/dm_pc_34/song.dat/song.dat.html>

Playback behavior is checked against ReDMCSB WIP20210206:
`SELECTOR.C` F8367/F8368/F8369 (lines 810–850, 909–1051),
`IBMIO.C` F8120/F8121/F8125 (lines 1894–2128), and
`VIDEODRV.C` F8153 (lines 3163–3185). Container and sample counts
are also empirically verified against the real file
`DungeonMasterPC34/DATA/SONG.DAT` shipped in the DM1 DOS package
(`original-games/Game,Dungeon_Master,DOS,Software.7z`, dated
1992-02-26) using the parser in `song_dat_loader_v1.c`.

The file is **not** vendored into this repository.  Only its shape,
its decoding algorithm, and the public English per-item labels are.

---

## 1. File container — DMCSB2

`SONG.DAT` uses the DMCSB2 container (same as `GRAPHICS.DAT` DM PC
v3.4).  Endianness for word fields is little-endian.

```
offset  size  field
------  ----  -----------------------------------------
0       2     file signature           : 0x8001   (LE u16 on disk)
2       2     number of items N        : 10       (LE u16)
4       2*N   compressed size per item (LE u16 each, bytes)
4+2N    2*N   decompressed size per item (LE u16 each, bytes)
4+4N    4*N   item attributes — 2 consecutive LE u16 per item
4+8N    …     item data, each item stored at
              (header_end + sum of previous items' compressed sizes)
```

For N = 10 this puts the data section at offset **84** and item data
runs from 84 .. 162482 = 162398 bytes, which exactly matches the
162,482-byte file on disk.

**Note on the signature:** dmweb describes the signature as `0x8001
in big endian`.  In `SONG.DAT` DM PC v3.4 the first two bytes on disk
are `01 80`, which is `0x8001` read as LE.  Firestaff's loader accepts
the value as `0x8001` in either endian interpretation; for DM PC v3.4
LE is the one that matches.

---

## 2. Empirically verified item table (DM PC v3.4, EN)

Header parse from the real file (see
`parity-evidence/pass50_song_dat_header.txt` for byte-for-byte output):

| # | Offset | Compressed | Decompressed | Attr0  | Attr1  | Type |
|---|--------|-----------:|-------------:|--------|--------|------|
| 0 |    84  |         40 |           40 | 0x0001 | 0x0002 | SEQ2 |
| 1 |   124  |       3192 |         6374 | 0xE418 | 0x0000 | SND8 |
| 2 |  3316  |      16504 |        29256 | 0x4672 | 0x3372 | SND8 |
| 3 | 19820  |       5640 |         9787 | 0x3926 | 0xFE60 | SND8 |
| 4 | 25460  |      29141 |        37890 | 0x0094 | 0x2261 | SND8 |
| 5 | 54601  |      33315 |        43565 | 0x2BAA | 0xB080 | SND8 |
| 6 | 87916  |       3506 |         7002 | 0x581B | 0x708F | SND8 |
| 7 | 91422  |        272 |          537 | 0x1702 | 0x708F | SND8 |
| 8 | 91694  |      32740 |        38658 | 0x0097 | 0x1572 | SND8 |
| 9 | 124434 |      38048 |        46367 | 0x1DB5 | 0x57A2 | SND8 |

Sum of compressed sizes: **162398**.  Header: 84.  File size: **162482**
— exact match.

Per the dmweb item-type map for SONG.DAT DM PC v3.4:

- Item 0: **SEQ2** — music sequence (indices into items 1..9)
- Items 1..9: **SND8** — DPCM-encoded mono samples; the selector requests 11126 Hz

Greatstone's per-item label table for this exact file
(<http://greatstone.free.fr/dm/db_data/dm_pc_34/song.dat/song.dat.html>)
gives the authoritative English semantic names:

| # | Label                                             |
|---|---------------------------------------------------|
| 0 | Music Score (Sequence of Music Parts Numbers)     |
| 1 | Music Part 1                                      |
| 2 | Music Part 2                                      |
| 3 | Music Part 3                                      |
| 4 | Music Part 4                                      |
| 5 | Music Part 5                                      |
| 6 | Music Part 6                                      |
| 7 | Music Part 7                                      |
| 8 | Music Part 8                                      |
| 9 | Music Part 9                                      |

This confirms two things:

1. Every SND8 item in SONG.DAT is a **music part**, not an in-game
   SFX sample.  In-game SFX are the **SND3** items in GRAPHICS.DAT
   (see §5 below).
2. The SEQ2 word list in item 0 is the concert-piece score — a list
   of indices into the 9 music parts.

These labels are surfaced at runtime by `V1_Song_ItemLabel()` in
`song_dat_loader_v1.h` and verified by probe invariant
`INV_V1_SONG_07`.

---

## 3. SEQ2 item format — music sequence

Stored as a flat list of little-endian u16 words.  Each word is an
index into the sound-sample items (1..9 in this file).  The last word
has bit 15 set to mark a sequence jump. SELECTOR.C F8367 interprets
its low 15 bits as a zero-based sequence index, not a sample-part index.
The special word 0xFFFF instead jumps to sequence index zero.

**Empirically observed sequence in item 0 (20 words):**

```
raw words   : 0001 0002 0003 0002 0003 0002 0003 0002
              0004 0005 0006 0002 0003 0002 0004 0005
              0007 0008 0009 8001
music parts : 1 2 3 2 3 2 3 2 4 5 6 2 3 2 4 5 7 8 9 [jump to sequence index 1, whose part is 2]
```

The first part plays once. The terminal 0x8001 then repeats sequence
entries 1 through 18, beginning with part 2. It does not replay part 1.

The first pass contains **476,494 decoded samples**, approximately
**42.827 seconds** at the source-requested rate. Subsequent loops omit
the first part's 6,372 samples and last approximately **42.254 seconds**.
These are requested-rate durations, not measurements of a physical card.

---

## 4. SND8 item format — DPCM sound sample

```
offset  size  field
------  ----  -----------------------------------------
0       2     number of samples N_samples (big-endian u16)
2       …     packed DPCM nibbles, high nibble first
```

Source-requested playback rate: **11126 Hz**, mono, 8-bit signed after
decoding. This rate is supplied by SELECTOR.C F8367, not stored in SND8.
F0786 passes the count and rate to IODRV_24; IBMIO.C F8125 starts the next
part when its predecessor completes. Physical PC devices quantize the
requested rate through PIT/Tandy divisors and may downsample. Firestaff
uses the requested rate; it does not claim device-specific clock emulation.

### Decoding algorithm (verbatim from dmweb)

```
prev = 0
while nibbles remain and out < N_samples:
    n1 = next nibble (0..15)
    sign-extend: if n1 > 7: n1 -= 16           # now -8..7
    if n1 != -8:
        diff = n1
    else:
        n2 = next nibble; n3 = next nibble
        diff = (n2 << 4) | n3                   # unsigned byte
        if diff > 127: diff -= 256              # signed int8
    prev += diff
    emit sample = prev (signed int8, clamped)
```

### Verified sample durations (DM PC v3.4 EN)

| Item | Declared samples | Decoded samples | Duration (11126 Hz) |
|-----:|-----------------:|----------------:|--------------------:|
|    1 |             6372 |            6372 |              573 ms |
|    2 |            29254 |           29254 |             2629 ms |
|    3 |             9785 |            9785 |              879 ms |
|    4 |            37888 |           37888 |             3405 ms |
|    5 |            43563 |           43563 |             3915 ms |
|    6 |             7000 |            7000 |              629 ms |
|    7 |              535 |             535 |               48 ms |
|    8 |            38656 |           38656 |             3474 ms |
|    9 |            46365 |           46365 |             4167 ms |

Decoded sample count matches the declared count **exactly** for all 9
items, which validates the SND8 decoder end-to-end.

Total audible content in SONG.DAT: **≈ 19.721 seconds** of raw PCM
before sequencing/looping.

---

## 5. Where SONG.DAT is used in original DM1

SONG.DAT belongs to the PC34 entrance selector, including its Credits
page. SELECTOR.C F8368 loads it before entrance setup. F8367 waits 60
VIDRV_07 retraces before the first part, then continues the sequence while
the user stays in the selector or Credits. F8369 releases it on exit.
For VGA mode 13h, 60 retraces at the modeled 70 Hz are about 0.857 seconds.
This delay is independent of the SWSH and TITLE animation schedules.

In-game MUSIC.C CD-track requests are a separate interface. IBMIO.C
F8123 is empty for PC34; those requests must not substitute SONG.DAT.
FM Towns CDDA follows its own original platform route.

In-game sound effects come from GRAPHICS.DAT SND3 items. Firestaff decodes
those separately and maintains independent native music and effect streams.
The music transport retains decoded PCM in bounded memory, queues at most
one second at a time and refills from the sequence's loop offset. Host pause
freezes refill; stop, disable, rebind and shutdown disarm it. Physical audio
output and original sound-card behavior remain separate verification work.

---

## 6. License / copyright

The original `SONG.DAT`, `GRAPHICS.DAT`, and all byte content of
Dungeon Master PC v3.4 are copyrighted game assets and are **not**
checked into this repository.  Only (a) the format spec above, (b)
the byte-layout metadata verified from the real file (offsets/sizes
only, no audio content), and (c) the decoding algorithms are tracked.

The decoder in `song_dat_loader_v1.c` requires a user-supplied path
to a SONG.DAT file at runtime; it will refuse to run without one.
