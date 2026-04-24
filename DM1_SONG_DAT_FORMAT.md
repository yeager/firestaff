# DM1 SONG.DAT — format specification (PC v3.4)

Pass 50 — source-backed documentation of the original Dungeon Master PC
v3.4 `SONG.DAT` file, as a prerequisite to replacing Firestaff's V1
procedural audio placeholders with original-faithful samples.

All facts in this document are either (a) taken verbatim from the
community format reference at
<http://dmweb.free.fr/community/documentation/file-formats/data-files/>
or (b) empirically verified against the real file
`DungeonMasterPC34/DATA/SONG.DAT` shipped in the DM1 DOS package
(`original-games/Game,Dungeon_Master,DOS,Software.7z`, sha-1 unchanged
since 1992-02-26) using the parser in `song_dat_loader_v1.c`.

The file is **not** vendored into this repository.  Only its shape and
the decoding algorithm are.

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
- Items 1..9: **SND8** — DPCM-encoded mono 11025 Hz samples

---

## 3. SEQ2 item format — music sequence

Stored as a flat list of little-endian u16 words.  Each word is an
index into the sound-sample items (1..9 in this file).  The last word
has bit 15 set to mark the end of the sequence; at that point the
music loops back to the first word.

**Empirically observed sequence in item 0 (20 words):**

```
raw words   : 0001 0002 0003 0002 0003 0002 0003 0002
              0004 0005 0006 0002 0003 0002 0004 0005
              0007 0008 0009 8001
music parts : 1 2 3 2 3 2 3 2 4 5 6 2 3 2 4 5 7 8 9 [loop→1]
```

The pattern strongly suggests:

- Sample 1 = intro (part 1 appears only at the loop boundary)
- Samples 2, 3 = the main repeating motif (2-3-2-3-2-3-2)
- Samples 4, 5, 6 = a B-section (4-5-6 … 4-5)
- Samples 7, 8, 9 = a transition / outro block appearing once before
  looping back to the intro.

Total decoded sequence length:
3192 + 16504 + 5640×3 + 5640×2 (…) — i.e. about **45 seconds** of
music before the first loop point, given sample durations below.

---

## 4. SND8 item format — DPCM sound sample

```
offset  size  field
------  ----  -----------------------------------------
0       2     number of samples N_samples (big-endian u16)
2       …     packed DPCM nibbles, high nibble first
```

Playback rate: **11025 Hz**, mono, 8-bit signed after decoding.

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

| Item | Declared samples | Decoded samples | Duration (11025 Hz) |
|-----:|-----------------:|----------------:|--------------------:|
|    1 |             6372 |            6372 |              577 ms |
|    2 |            29254 |           29254 |             2653 ms |
|    3 |             9785 |            9785 |              887 ms |
|    4 |            37888 |           37888 |             3436 ms |
|    5 |            43563 |           43563 |             3951 ms |
|    6 |             7000 |            7000 |              634 ms |
|    7 |              535 |             535 |               48 ms |
|    8 |            38656 |           38656 |             3506 ms |
|    9 |            46365 |           46365 |             4205 ms |

Decoded sample count matches the declared count **exactly** for all 9
items, which validates the SND8 decoder end-to-end.

Total audible content in SONG.DAT: **≈ 19.9 seconds** of raw PCM
before sequencing/looping.

---

## 5. Where SONG.DAT is used in original DM1

SONG.DAT is the **title-music** source.  In-game sound-effect samples
(footsteps, doors, swings, creature voices, spells …) are not in
SONG.DAT — they are the **SND3** items in `GRAPHICS.DAT`:

- Items 671-675, 677-685, 687-693, 701-712 → 33 raw 8-bit unsigned
  PCM samples at 6000 Hz (see dmweb SND3 section).

This means full original-faithful V1 audio eventually requires both:

1. SONG.DAT SND8 + SEQ2 decoding (title music) — documented and
   decoded in Firestaff by this pass.
2. GRAPHICS.DAT SND3 decoding (in-game SFX) — **not yet landed**;
   see `V1_BLOCKERS.md` entry 15 for the remaining gap.

---

## 6. License / copyright

The original `SONG.DAT`, `GRAPHICS.DAT`, and all byte content of
Dungeon Master PC v3.4 are copyrighted game assets and are **not**
checked into this repository.  Only (a) the format spec above, (b)
the byte-layout metadata verified from the real file (offsets/sizes
only, no audio content), and (c) the decoding algorithms are tracked.

The decoder in `song_dat_loader_v1.c` requires a user-supplied path
to a SONG.DAT file at runtime; it will refuse to run without one.
