# DM1 V1 Ambient Dungeon Sound Source Lock

This audit covers only the TODO row `Ambient dungeon sound`. It does not cover
title music playback, title/music looping, or generic SFX playback.

## Decision

No source-backed DM1 V1 PC ambient dungeon loop was found in ReDMCSB. The
source-backed behavior in this lane is event-indexed SFX: gameplay code requests
specific sound indices at map coordinates, `SOUND.C` applies distance/priority
rules, and the main game loop flushes one pending sound per tick.

Implementation is therefore blocked unless a separate original-runtime capture
or source reference proves an always-on/looping dungeon ambience path. Adding a
procedural background loop now would be invented behavior.

## ReDMCSB Evidence

- `GAMELOOP.C:55-115` is the main loop. It updates map/music state where
  compiled, draws the dungeon, then calls `F0065_SOUND_PlayPendingSound_CPSD()`;
  it does not call `F0064_SOUND_RequestPlay_CPSD(...)` or start an ambient SFX.
- `SOUND.C:1476-1544` is `F0064_SOUND_RequestPlay_CPSD(...)`; delayed sounds are
  explicit `C20_EVENT_PLAY_SOUND` events carrying a sound index and map X/Y.
- `SOUND.C:1565-1638` applies PC/I34E distance, immediate playback, and pending
  priority logic. It plays or queues a requested sound index; it does not own a
  dungeon ambience loop.
- `SOUND.C:1756-1867` is `F0065_SOUND_PlayPendingSound_CPSD(...)`, the per-tick
  pending flush used by `GAMELOOP.C`.
- `SOUND.C:1871-1919` is `F0505_SOUND_GetVolume(...)`, the directional
  distance-to-left/right-volume helper used by platforms with stereo volume.
- `DEFS.H:100-138` defines the I34E PC sound namespace and play modes. It has
  movement/attack/buzz/door/switch/event sounds and the one-sound-per-tick
  priority mode, but no ambient dungeon sound constant.
- `DATA.C:1264-1310` maps the I34E sound event table to SND3 graphics indices,
  including the seven creature movement sounds at `DATA.C:1304-1310`.
- Example event sources are explicit triggers: `MOVESENS.C:843-854` requests a
  creature movement sound when a group moves, `GROUP.C:267-281` requests Couatl
  movement sound on an animation flip, and `TIMELINE.C:1903-1905` dispatches a
  delayed play-sound event.

The bounded text audit also finds no `ambient`/`ambience` symbols in
`GAMELOOP.C`, `SOUND.C`, `DEFS.H`, or `DATA.C`. `SOUND.C` loop/repeat hits are
sample decoder internals, not gameplay ambience scheduling.

## Firestaff State

- `TODO.md` still tracks `Ambient dungeon sound` as incomplete.
- `audio_sdl_m11.[ch]` currently exposes marker playback, source sound-index
  playback, and title-song queueing. It has no ambient-specific API, and this
  audit intentionally does not add one.
- Existing audio docs already mark cadence/overlap/continuous-loop behavior as
  unclaimed without original runtime capture.

## Verification

Run:

```sh
python3 tools/verify_dm1_v1_ambient_dungeon_sound_source_lock.py
```

The expected result is `PASS DM1_V1_AMBIENT_DUNGEON_SOUND_SOURCE_LOCK no
source-backed ambient dungeon loop found`.
