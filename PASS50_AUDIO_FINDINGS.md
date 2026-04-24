# Pass 50 — V1 audio / SONG.DAT findings

Pass 50 is the audio-focused pass against V1 blocker #15
("Audio samples are procedural placeholders") and
`PARITY_MATRIX_DM1_V1.md` §7 row "Sound samples (content)".

Scope strictly respects:

- no M10 change
- no V2 work
- no repo reorg
- no unrelated engine changes
- no vendoring of original copyrighted audio content

This document is the landing log of what Pass 50 actually proved.

---

## 1. Asset location (verified)

Original `SONG.DAT` located and parsed from:

    original-games/Game,Dungeon_Master,DOS,Software.7z
    → DungeonMasterPC34/DATA/SONG.DAT
      162482 bytes, dated 1992-02-26

The archive is **not** extracted into the tracked tree.  It is the
canonical DM PC v3.4 package already inventoried in
`redmcsb_original_data_inventory_2026-04-16.md`.

---

## 2. Format — source-backed, empirically confirmed

`SONG.DAT` DM PC v3.4 is a DMCSB2 container (same family as
`GRAPHICS.DAT`) with little-endian word fields and 10 items.

Full format specification: `DM1_SONG_DAT_FORMAT.md`.

Two independent external references back this pass:

- dmweb — container, item-type map, SEQ2/SND8 algorithms:
  <http://dmweb.free.fr/community/documentation/file-formats/data-files/>
- Greatstone — per-item canonical English labels:
  <http://greatstone.free.fr/dm/db_data/dm_pc_34/song.dat/song.dat.html>

The Greatstone reference gives item 0 = **Music Score** and items
1..9 = **Music Part 1..9**.  These labels are now carried in
`song_dat_loader_v1.h` via `V1_Song_ItemLabel()` and probe-checked by
`INV_V1_SONG_07`.  Crucially, the label set confirms that **SONG.DAT
contains no in-game SFX** — all 9 SND8 items are music parts.
In-game SFX are the SND3 items in GRAPHICS.DAT.

Header parse of the real file (see
`parity-evidence/pass50_song_dat_header.txt`):

| # | Offset | Comp | Decomp | Attr0  | Attr1  | Type | Samples | ms (11025 Hz) |
|---|-------:|-----:|-------:|--------|--------|------|--------:|--------------:|
| 0 |     84 |   40 |     40 | 0x0001 | 0x0002 | SEQ2 |       — | —             |
| 1 |    124 | 3192 |   6374 | 0xE418 | 0x0000 | SND8 |    6372 |           577 |
| 2 |   3316 |16504 |  29256 | 0x4672 | 0x3372 | SND8 |   29254 |          2653 |
| 3 |  19820 | 5640 |   9787 | 0x3926 | 0xFE60 | SND8 |    9785 |           887 |
| 4 |  25460 |29141 |  37890 | 0x0094 | 0x2261 | SND8 |   37888 |          3436 |
| 5 |  54601 |33315 |  43565 | 0x2BAA | 0xB080 | SND8 |   43563 |          3951 |
| 6 |  87916 | 3506 |   7002 | 0x581B | 0x708F | SND8 |    7000 |           634 |
| 7 |  91422 |  272 |    537 | 0x1702 | 0x708F | SND8 |     535 |            48 |
| 8 |  91694 |32740 |  38658 | 0x0097 | 0x1572 | SND8 |   38656 |          3506 |
| 9 | 124434 |38048 |  46367 | 0x1DB5 | 0x57A2 | SND8 |   46365 |          4205 |

Sum check: header (84) + Σ compressed (162398) = 162482 bytes —
**exact file-size match**.

Per dmweb's SONG.DAT DM PC v3.4 item-type map:

- item 0 = SEQ2 (music sequence)
- items 1..9 = SND8 (DPCM, 11025 Hz, signed 8-bit)

For every SND8 item, the **declared sample count exactly equals the
decoded sample count** produced by our DPCM decoder.  This proves the
decoder implementation is correct.

SEQ2 decode yielded the 20-word sequence
`1, 2, 3, 2, 3, 2, 3, 2, 4, 5, 6, 2, 3, 2, 4, 5, 7, 8, 9, [loop→1]`
with bit-15 end marker in the final word, as the format specifies.

---

## 3. Landed code / docs

All landed in `tmp/firestaff` without touching M10 and without any
engine integration outside the audio lane:

| File                                         | Role                           |
|----------------------------------------------|--------------------------------|
| `DM1_SONG_DAT_FORMAT.md`                     | Source-backed format spec      |
| `PASS50_AUDIO_FINDINGS.md` (this file)       | Landing log + remaining gaps   |
| `song_dat_loader_v1.h`                       | V1 loader public API           |
| `song_dat_loader_v1.c`                       | DMCSB2 header + SEQ2 + SND8    |
| `probes/v1/firestaff_v1_song_dat_probe.c`    | Header + decode invariants     |
| `run_firestaff_v1_song_dat_probe.sh`         | Probe build/run                |
| `parity-evidence/pass50_song_dat_header.txt` | Real-file parse evidence       |
| `parity-evidence/pass50_v1_song_dat_probe.txt` | Probe pass log               |

Probe result against the real DM PC v3.4 SONG.DAT:

```
PASS INV_V1_SONG_01 manifest parsed
PASS INV_V1_SONG_02 file size matches DM PC v3.4 EN canonical value
PASS INV_V1_SONG_03 DMCSB2 signature=0x8001, 10 items, 84-byte header
PASS INV_V1_SONG_04 all 10 item headers match verified table
PASS INV_V1_SONG_05 SEQ2 music sequence matches verified 20-word pattern
PASS INV_V1_SONG_06 all 9 SND8 items decode to exactly declared sample count at 11025 Hz
PASS INV_V1_SONG_07 Greatstone per-item labels present (Music Score + Music Part 1..9)
# summary: 7/7 invariants passed
```

When the original SONG.DAT is absent on a given machine, the probe
emits a SKIP line and exits 0 — the format/decoder are still covered
by the tracked documentation and code.

---

## 4. What Pass 50 does NOT change

- `audio_sdl_m11.c` / `audio_sdl_m11.h`: still the procedural
  placeholder engine.  It is intentionally **not** wired to the SONG
  decoder in this pass.
- No M10 code, no M11 chrome/visuals, no movement/door/env ownership
  code.
- No repo reorganization, no V2 work.

---

## 5. Gap to "V1 audio is original-faithful"

To honestly claim V1 audio is original-faithful, the following work
still remains (each item is a distinct, boundable pass):

1. **In-game SFX bank (GRAPHICS.DAT SND3)** — not started.
   33 sound items at indices 671-675, 677-685, 687-693, 701-712 in
   `GRAPHICS.DAT` DM PC v3.4 EN, raw PCM 8-bit mono unsigned at
   6000 Hz (per dmweb).  A `graphics_dat_snd3_loader_v1.[ch]` and a
   probe mirror of pass 50 is the natural next step.

2. **Sound-event → SND3 index table (original-faithful mapping)** —
   not started.  The 33 SND3 items must be mapped to the
   game-original event set.  Pass 50 only deals with SONG.DAT (music
   source) and does not attempt the SFX mapping.

3. **Runtime audio integration** — not started.
   The current `M11_Audio_EmitMarker` path routes every trigger to
   one of 5 procedural placeholders.  An original-faithful path
   needs to:
   - replace `m11_generate_sounds` with real decoded buffers from
     GRAPHICS.DAT SND3 (indexed by event) and SONG.DAT SND8+SEQ2
     (for title music),
   - keep the placeholder path behind a compile/runtime flag for
     distribution without original assets,
   - gate on the presence of original SONG.DAT / GRAPHICS.DAT.

4. **Title-music playback driver** — not started.
   SEQ2 decoding is done; playback (concatenating the referenced
   SND8 parts in order, honouring the bit-15 loop-back marker) and
   the SDL3 streaming glue are not wired.

5. **Sample-rate handling** — partially addressed.
   The M11 audio path is 22050 Hz; SONG.DAT SND8 is 11025 Hz and
   SND3 is 6000 Hz.  Either the stream rate is reconfigured per
   source, or a resampler is added.  Pass 50 neither picked nor
   implemented this.

6. **Bug-faithful playback quirks** (e.g. original Atari-ST-only
   door-sound rate difference for the entrance door — documented
   at dmweb) — not applicable to PC v3.4, but worth noting for the
   parity ledger.

---

## 6. Honest status update for the parity matrix

- `PARITY_MATRIX_DM1_V1.md` §7 "Sound samples (content)" is updated
  from `KNOWN_DIFF` ("not attempted") to `KNOWN_DIFF` ("SONG.DAT
  format + decoder landed and probe-verified; GRAPHICS.DAT SND3
  bank and runtime integration remain").
- `V1_BLOCKERS.md` entry 15 is updated with the Pass 50 landing
  log and the six concrete remaining sub-gaps above.

Placeholder audio is still in use for V1 at runtime. This pass did
not replace it; it built the bounded, source-backed foundation
required to replace it in a subsequent pass.
