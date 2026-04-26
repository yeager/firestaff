# Pass 125 — V1 audio event-index fallback ordering

Pass 125 is a bounded audio/timing candidate pass.  It does **not** claim
waveform, live playback, overlap, or original cadence parity.

## What changed

| File | Purpose |
| --- | --- |
| `audio_sdl_m11.c` | Preserve `lastSoundIndex` for valid `M11_Audio_EmitSoundIndex(...)` calls even when playback falls back to marker/no-audio. Direct marker calls still clear `lastSoundIndex`. |
| `probes/m11/firestaff_m11_pass125_audio_event_order_probe.c` | Headless event-order/identity probe for the source sound-event seam. |
| `run_firestaff_m11_pass125_audio_event_order_probe.sh` | Standalone build/run wrapper for the probe. |
| `parity-evidence/pass125_audio_event_order_probe.txt` | PASS log from the local probe run. |

## Source-backed honesty boundary

The existing pass-52 map already binds DM PC v3.4 sound event indices to
GRAPHICS.DAT SND3 items.  This pass only tightens the runtime seam so a headless
or fallback run can still distinguish:

1. a source sound-event request that could not play decoded SND3, from
2. a direct/procedural marker request.

That is useful for future cadence/queue probes, but it is still metadata only.
The remaining original-parity blockers are unchanged: original SFX/music capture,
overlap/priority behaviour, continuous title-music loop cadence, and the four
remaining direct-marker TODO buckets from pass 55.

## Probe result

```text
PASS P125_AUDIO_EVENT_ORDER_01 headless fallback backend initializes without opening SDL audio
PASS P125_AUDIO_EVENT_ORDER_02 mapped combat event preserves source sound index while falling back to marker/no-audio path
PASS P125_AUDIO_EVENT_ORDER_03 direct marker emission remains distinguishable from source sound-event emission
PASS P125_AUDIO_EVENT_ORDER_04 out-of-range sound index uses marker fallback without recording a false source event
# summary: 4/4 invariants passed
```
