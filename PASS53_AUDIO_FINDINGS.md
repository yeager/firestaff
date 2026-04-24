# Pass 53 — V1 audio / runtime GRAPHICS.DAT SND3 playback wiring

Pass 53 is the bounded follow-up to Pass 52 for V1 blocker #15
("Audio samples are procedural placeholders").  Scope is only runtime playback
wiring for already-decoded/mapped DM PC v3.4 English `GRAPHICS.DAT` SND3
in-game SFX.

No `SONG.DAT` title-music playback is included in this pass.

---

## 1. What landed

| File | Role |
|------|------|
| `audio_sdl_m11.[ch]` | Runtime asset-gated SND3 loading, resampling, and event-index playback API |
| `m11_game_view.c` | Routes `EMIT_SOUND_REQUEST` payloads through `M11_Audio_EmitSoundIndex(...)` before procedural fallback |
| `probes/m11/firestaff_m11_pass53_snd3_runtime_probe.c` | Focused runtime SND3/fallback probe |
| `run_firestaff_m11_pass53_snd3_runtime_probe.sh` | Probe build/run wrapper |
| `parity-evidence/pass53_v1_snd3_runtime_probe.txt` | Probe PASS log |

The runtime now attempts to locate original `GRAPHICS.DAT` in this order:

1. `FIRESTAFF_GRAPHICS_DAT`
2. `./GRAPHICS.DAT`
3. `$HOME/.firestaff/data/GRAPHICS.DAT`

If the asset is absent, malformed, or disabled with
`FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SND3`, the existing procedural marker path is
preserved.

---

## 2. Sample-rate policy

The SDL stream remains fixed at **22050 Hz float mono** (`M11_AUDIO_SAMPLE_RATE`).

Pass 51 verified SND3 source buffers as **unsigned 8-bit mono at 6000 Hz**.
Pass 53 does **one-time linear resampling at init** from 6000 Hz to 22050 Hz for
each of the 35 mapped DM sound event indices.  Runtime emission then queues the
already-resampled float buffer directly to SDL.

This is intentionally small and conservative:

- no stream-rate switching
- no callback mixer refactor
- no cadence/overlap claim beyond SDL queueing the requested buffer
- aliases remain duplicated as event-index buffers, matching the pass-52
  event-index API boundary

---

## 3. Probe result

Against the real DM PC v3.4 English `GRAPHICS.DAT` visible at
`~/.firestaff/data/GRAPHICS.DAT`:

```text
PASS P53_SND3_RUNTIME_01 fallback path does not require GRAPHICS.DAT/SND3 assets
PASS P53_SND3_RUNTIME_02 EmitSoundIndex falls back to procedural marker playback when original SND3 is disabled/unavailable
PASS P53_SND3_RUNTIME_03 original GRAPHICS.DAT SND3 bank loads all 35 event-index buffers when asset is present
PASS P53_SND3_RUNTIME_04 6000 Hz SND3 source is linearly resampled once to the fixed 22050 Hz SDL stream rate
PASS P53_SND3_RUNTIME_05 SDL3 runtime queues the decoded/resampled SND3 buffer for a mapped sound event
# summary: 5/5 invariants passed
```

Rerun regressions:

- Pass 51 SND3 bank probe: 6/6 PASS
- Pass 52 event-map probe: 5/5 PASS
- Existing M11 audio probe: 8/8 PASS
- CMake build: PASS

---

## 4. What Pass 53 does NOT claim

- No `SONG.DAT` / SEQ2 title music playback.
- No original capture of sound cadence, prioritization, or overlap.
- No proof that every non-`EMIT_SOUND_REQUEST` M11 direct marker call has been
  converted to a source sound index; those calls retain procedural fallback.
- No distribution of original audio assets.

Pass 53 narrows the in-game SFX runtime gap from "decoded/mapped but not used"
to "mapped `EMIT_SOUND_REQUEST` events queue decoded/resampled SND3 buffers when
original assets are present, with procedural fallback preserved."
