# Pass 54 — V1 audio / SONG.DAT title-music runtime driver

Pass 54 is the bounded follow-up to Pass 53 for V1 blocker #15
("Audio samples are procedural placeholders").  Scope is only runtime loading,
resampling, concatenation, and queueing support for the already decoded DM PC
v3.4 English `SONG.DAT` title-music bank.

No original cadence/overlap claim is made.

---

## 1. What landed

| File | Role |
|------|------|
| `audio_sdl_m11.[ch]` | Asset-gated SONG.DAT loading, SND8 resampling, SEQ2 one-cycle concatenation, and `M11_Audio_PlayTitleMusic(...)` queue API |
| `CMakeLists.txt` | Adds `song_dat_loader_v1.c` to the M11 library so the runtime path links in normal builds |
| `run_firestaff_m11_audio_probe.sh` / `run_firestaff_m11_pass53_snd3_runtime_probe.sh` | Add `song_dat_loader_v1.c` to standalone audio-probe link lines after the runtime gained a SONG.DAT dependency |
| `probes/m11/firestaff_m11_pass54_song_runtime_probe.c` | Focused title-music runtime probe |
| `run_firestaff_m11_pass54_song_runtime_probe.sh` | Probe build/run wrapper |
| `parity-evidence/pass54_v1_song_runtime_probe.txt` | Probe PASS log |

The runtime now attempts to locate original `SONG.DAT` in this order:

1. `FIRESTAFF_SONG_DAT`
2. `SONG_DAT_PATH` (kept for the Pass 50 probe environment)
3. `./SONG.DAT`
4. `$HOME/.firestaff/data/SONG.DAT`
5. `/tmp/fs_pass50_extract/dm_dos/DungeonMasterPC34/DATA/SONG.DAT` (local extracted test asset path)

If the asset is absent, malformed, or disabled with
`FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG`, title music remains a safe no-op.
Procedural SFX fallback and the Pass 53 SND3 path are preserved.

---

## 2. Sample-rate and SEQ2 policy

The SDL stream remains fixed at **22050 Hz float mono** (`M11_AUDIO_SAMPLE_RATE`).

Pass 50 verified SONG.DAT SND8 source buffers as **signed 8-bit mono at
11025 Hz**.  Pass 54 does one-time linear resampling at init from 11025 Hz to
22050 Hz for each of the 9 decoded SND8 music parts.  The title-music phrase is
then built by walking SEQ2 words and concatenating the referenced resampled
parts.

The documented/probed bit-15 behavior is handled narrowly:

- normal words (`0x0001`..`0x0009`) append that music part;
- the first bit-15 word stops the one-cycle build;
- the low 15 bits of that marker are recorded as the loop target;
- for DM PC v3.4 EN the probed marker is `0x8001`, so the runtime records loop
  target part `1`.

Pass 54 deliberately does **not** claim continuous loop cadence, restart timing,
or overlap behavior.  `M11_Audio_PlayTitleMusic(...)` queues the decoded and
resampled one-cycle phrase when an SDL backend is available; callers can use
that API, but original-faithful title-screen start/stop timing still requires
an original runtime capture.

---

## 3. Probe result

Against the real DM PC v3.4 English `SONG.DAT` visible at
`/tmp/fs_pass50_extract/dm_dos/DungeonMasterPC34/DATA/SONG.DAT`:

```text
PASS P54_SONG_RUNTIME_01 fallback/no-op path does not require SONG.DAT assets
PASS P54_SONG_RUNTIME_02 PlayTitleMusic is a safe no-op when original SONG.DAT is unavailable/disabled
PASS P54_SONG_RUNTIME_03 original SONG.DAT loads all 9 SND8 music-part buffers when asset is present
PASS P54_SONG_RUNTIME_04 SEQ2 walk stops at the bit-15 loop-back marker and records loop target part 1
PASS P54_SONG_RUNTIME_05 11025 Hz signed SND8 parts are linearly resampled/concatenated for the fixed 22050 Hz stream
PASS P54_SONG_RUNTIME_06 SDL3 runtime queues the decoded/resampled one-cycle title-music phrase
# summary: 6/6 invariants passed
```

Rerun regressions:

- Pass 50 SONG.DAT probe: 7/7 PASS
- Pass 53 runtime SND3 probe: 5/5 PASS
- Existing M11 audio probe: 8/8 PASS
- CMake `firestaff_m11` build: PASS (one pre-existing unused-function warning in `m11_game_view.c`)

---

## 4. What Pass 54 does NOT claim

- No original capture of title-music start time, stop time, cadence, looping
  cadence, or overlap with SFX.
- No claim that the title frontend invokes the new API at the exact original
  moment; this pass lands the asset-backed queue driver and safe fallback.
- No audit/conversion of remaining direct `M11_Audio_EmitMarker(...)` call sites.
- No distribution of original audio assets.

Pass 54 narrows the music gap from "SONG.DAT decoded but not usable by runtime"
to "SONG.DAT title music can be asset-gated, decoded, resampled, SEQ2-walked,
concatenated, and queued as one pre-loop phrase, with loop-target metadata
recorded and no-op fallback preserved."
