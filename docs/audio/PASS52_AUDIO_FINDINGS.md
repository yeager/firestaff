# Pass 52 — V1 audio / sound-event → GRAPHICS.DAT SND3 mapping

Pass 52 is the bounded follow-up to Pass 51 for V1 blocker #15
("Audio samples are procedural placeholders").  Scope is only the source-backed
mapping from DM PC v3.4 English sound event indices to `GRAPHICS.DAT` SND3
items.

No runtime audio playback wiring is included in this pass.

---

## 1. Source evidence

The mapping is anchored in the local ReDMCSB source dump for the current V1
PC target (`I34E`):

- `Toolchains/Common/Source/DEFS.H`
  - `MEDIA485_P20JB_I34E_I34M_...` sound-index macros
  - `M513_SOUND_COUNT = 35`
  - base/attack/movement ranges: `C00_SOUND_FIRST_BASE`,
    `C19_SOUND_FIRST_ATTACK`, `C28_SOUND_FIRST_MOVEMENT`
- `Toolchains/Common/Source/DATA.C`
  - `MEDIA719_I34E_I34M_... G0060_as_Graphic562_Sounds[M513_SOUND_COUNT]`
  - each sound event row contains the original `GRAPHICS.DAT` item index

Pass 51 already verified that every referenced item is a real SND3 item in DM
PC v3.4 English `GRAPHICS.DAT` and carries the Greatstone `Sound 00` ..
`Sound 32` labels.

---

## 2. Landed code / evidence

| File | Role |
|------|------|
| `sound_event_snd3_map_v1.h` | V1 sound-event → SND3 mapping public API |
| `sound_event_snd3_map_v1.c` | Source-anchored 35-entry table from ReDMCSB `DEFS.H`/`DATA.C` |
| `probes/v1/firestaff_v1_snd3_event_map_probe.c` | Mapping invariants + optional real-bank metadata validation |
| `run_firestaff_v1_snd3_event_map_probe.sh` | Probe build/run wrapper |
| `parity-evidence/pass52_v1_snd3_event_map_probe.txt` | Probe PASS log |
| `PASS52_AUDIO_FINDINGS.md` | This landing log |

The table maps 35 DM PC v3.4 sound event indices to 33 unique SND3 items.
Two aliases are source-backed:

- `C02_SOUND_DOOR_RATTLE` and `C03_SOUND_DOOR_RATTLE_ENTRANCE` both map to
  SND3 item `673` / Greatstone `Sound 02 (Door)`.
- `C05_SOUND_STRONG_EXPLOSION` and `M541_SOUND_WEAK_EXPLOSION` both map to
  SND3 item `675` / Greatstone `Sound 04 (Exploding Fireball)`.

---

## 3. Probe result

Against the real DM PC v3.4 English `GRAPHICS.DAT` visible at
`~/.firestaff/data/GRAPHICS.DAT`:

```text
PASS INV_V1_SND3_EVENT_MAP_01 35 DM PC v3.4 sound-event entries exposed
PASS INV_V1_SND3_EVENT_MAP_02 source table order maps expected sound indices to SND3 items/labels
PASS INV_V1_SND3_EVENT_MAP_03 lookup API accepts 0..34 and rejects out-of-range/none values
PASS INV_V1_SND3_EVENT_MAP_04 35 events cover all 33 SND3 items; only door-rattle and explosion aliases duplicate items
PASS INV_V1_SND3_EVENT_MAP_05 mapping resolves to populated SND3 manifest entries with valid sample metadata
# summary: 5/5 invariants passed
```

When the original `GRAPHICS.DAT` is absent, invariants 1–4 still run and the
manifest-metadata check prints `SKIP` while exiting successfully.  The mapping
table itself does not vendor or require original assets.

---

## 4. What Pass 52 does NOT change

- `audio_sdl_m11.c` / `audio_sdl_m11.h` still use procedural placeholder
  buffers at runtime.
- No decoded SND3 buffers are loaded into the SDL runtime.
- No resampling decision is made for 6000 Hz SND3 vs. the current 22050 Hz
  SDL path.
- No SONG.DAT title-music playback is wired.
- Cadence/overlap and prioritization remain source-known but not captured
  against original runtime audio.

---

## 5. Remaining V1 audio gaps

1. Runtime integration for decoded SND3 buffers, gated on original asset
   availability and with a placeholder fallback.
2. Sample-rate handling/resampling policy for 6000 Hz SND3 in the current SDL
   path.
3. SONG.DAT SEQ2/SND8 title-music playback driver.
4. Original capture of cadence/overlap and any PC v3.4 playback quirks.

Pass 52 narrows the in-game SFX gap from "decoded bank exists but events are
unmapped" to "decoded bank and sound-event mapping are probe-verified; runtime
playback still pending."
