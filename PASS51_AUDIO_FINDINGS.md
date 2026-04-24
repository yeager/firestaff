# Pass 51 — V1 audio / GRAPHICS.DAT SND3 findings

Pass 51 is the bounded follow-up to Pass 50 for V1 blocker #15
("Audio samples are procedural placeholders").  Scope is only the
DM PC v3.4 English `GRAPHICS.DAT` SND3 in-game SFX bank.

No runtime audio wiring is included in this pass.

---

## 1. Format — source-backed, empirically confirmed

External references:

- dmweb data-files documentation:
  <http://dmweb.free.fr/community/documentation/file-formats/data-files/>
- Greatstone extracted item labels:
  <http://greatstone.free.fr/dm/db_data/dm_pc_34/graphics.dat/graphics.dat.html>

For DM PC v3.4 English, dmweb identifies `GRAPHICS.DAT` as a little-endian
DMCSB2 container with 713 items.  Its SND3 items are exactly:

- `671-675`
- `677-685`
- `687-693`
- `701-712`

SND3 item format:

- 1 big-endian word: number of sound samples
- unsigned 8-bit mono PCM bytes
- 0..3 trailing bytes of unknown use
- playback rate: 6000 Hz on PC / PC-9801

The local real asset used for verification was:

    ~/.firestaff/data/GRAPHICS.DAT

which points at the local original-data store and is not tracked in this
repo.

---

## 2. Landed code / evidence

| File | Role |
|------|------|
| `graphics_dat_snd3_loader_v1.h` | V1 SND3 loader public API |
| `graphics_dat_snd3_loader_v1.c` | DMCSB2 header parse + SND3 unsigned PCM decode |
| `probes/v1/firestaff_v1_graphics_dat_snd3_probe.c` | Header/index/label/decode invariants |
| `run_firestaff_v1_snd3_probe.sh` | Probe build/run wrapper |
| `parity-evidence/pass51_v1_graphics_dat_snd3_probe.txt` | Probe PASS log against real `GRAPHICS.DAT` |
| `PASS51_AUDIO_FINDINGS.md` | This landing log |

The loader carries the 33 Greatstone labels (`Sound 00` .. `Sound 32`) and
returns decoded unsigned PCM buffers at 6000 Hz.  It intentionally has no
SDL dependency and does not map game events to sound indices.

---

## 3. Probe result

Against the real DM PC v3.4 English `GRAPHICS.DAT`:

```text
PASS INV_V1_SND3_01 manifest parsed
PASS INV_V1_SND3_02 DMCSB2 signature=0x8001, 713 items, 5708-byte header
PASS INV_V1_SND3_03 dmweb SND3 item index set matches 33-item bank
PASS INV_V1_SND3_04 all 33 SND3 headers match verified DM PC v3.4 table
PASS INV_V1_SND3_05 all 33 SND3 items decode to unsigned PCM at 6000 Hz (114157 samples)
PASS INV_V1_SND3_06 Greatstone per-sound labels present for Sound 00..32
# summary: 6/6 invariants passed
```

When the original `GRAPHICS.DAT` is absent, the probe prints a SKIP line and
exits 0, mirroring Pass 50's asset-handling discipline.

---

## 4. What Pass 51 does NOT change

- `audio_sdl_m11.c` / `audio_sdl_m11.h` still use procedural placeholder
  buffers at runtime.
- No sound-event → SND3 index mapping is claimed.
- No title-music playback or SONG.DAT runtime integration is added.
- No resampling decision is made for the current 22050 Hz SDL path vs.
  6000 Hz SND3 / 11025 Hz SND8 source rates.

---

## 5. Remaining V1 audio gaps

1. Sound-event → SND3 index mapping table.
2. Runtime audio integration for decoded SND3 buffers, gated on original
   asset availability and with a placeholder fallback.
3. SONG.DAT SEQ2/SND8 title-music playback driver.
4. Sample-rate handling/resampling policy across 6000 Hz, 11025 Hz, and the
   current runtime stream rate.
5. Original capture of cadence/overlap and any PC v3.4 playback quirks.

Pass 51 narrows the in-game SFX content gap from "procedural placeholders
only" to "original SND3 bank decoded and probe-verified; runtime mapping and
playback still pending."
