# DM1 V1 queued PLAY_SOUND source-index evidence

Date: 2026-05-20
Worker worktree: `/home/trv2/work/firestaff-worktrees/audio-event-source-index-20260520`

## Primary source audit

ReDMCSB primary source root:
`/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

- `SOUND.C:1536-1544` (`F0064_SOUND_RequestPlay_CPSD`) schedules delayed sound requests with `C20_EVENT_PLAY_SOUND`, copies the source `P0088_SoundIndex` into `L0045_s_Event.C.SoundIndex`, preserves `MapX`/`MapY`, and returns instead of playing immediately.
- `TIMELINE.C:1903-1905` (`F0261_TIMELINE_Process_CPSEF`) handles `C20_EVENT_PLAY_SOUND` by calling `F0064_SOUND_RequestPlay_CPSD(L0682_s_Event.C.SoundIndex, L0682_s_Event.B.Location.MapX, L0682_s_Event.B.Location.MapY, C01_MODE_PLAY_IF_PRIORITIZED)`.
- `DEFS.H:136-138` defines the play modes: immediate, prioritized next-loop, and one-tick-later. The C20 event path re-enters the prioritized sound request path.
- `DEFS.H:944` defines `C20_EVENT_PLAY_SOUND` as event type 20.

This source evidence supports queued event audio as event-indexed SFX routing. It does not add ambient loops or party footsteps.

## Firestaff seam locked by this slice

- `src/memory/memory_tick_orchestrator_pc34_compat.c` already maps `TIMELINE_EVENT_PLAY_SOUND` to `EMIT_SOUND_REQUEST` with payload `[soundIndex, mapX, mapY, mapIndex]`.
- `src/engine/m11_game_view.c` and `src/shared/audio_sdl_m11.c` already route `EMIT_SOUND_REQUEST` through `M11_Audio_EmitSourceSoundIndex`, preserving `lastSoundIndex` even when playback falls back to marker/no-audio mode.
- This pass adds probe coverage in `probes/m11/firestaff_m11_audio_probe.c` so a queued PLAY_SOUND event with source sound index 2 (`C02_SOUND_DOOR_RATTLE`) must preserve index 2 and location through the orchestrator emission and M11 audio seam.

## Verification target

Targeted CTest:

```sh
ctest --test-dir build -R "^m11_audio$" --output-on-failure
```

The test is intentionally headless/fallback-safe. It proves source-index identity and route payload preservation, not waveform cadence parity.
