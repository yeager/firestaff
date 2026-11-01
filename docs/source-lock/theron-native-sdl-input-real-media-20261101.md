# Native Theron SDL input on authentic US media (2026-11-01)

## Build and source identity

Firestaff was configured and built as a native macOS Debug application from
the current repository revision. The test used
`$HOME/.firestaff/data/theron/TQUS02.bin`, MD5
`f23601102138f87c33025877767ebf76`; it did not launch Mednafen or use emulator
state. The CTest route also checks that each start reaches a loaded dungeon
and that its runtime boot asset has this exact Track 02 identity.

## Input routes exercised

`theron_v1_sdl_input_events` ran to completion three consecutive times with
`ctest --repeat until-fail:3`. Each execution launches the native application
through its regular SDL event loop and checks:

- Arrow/key actions, Button II turn chords, keypad 1/3 turns, and the Z/Space
  wait actions.
- Left/right mouse-panel movement, turns, champion selection, and waiting.
- Pickup, reselection, and drop of the source-backed level-0 sandals.
- Touch-swipe forward and turn actions.
- Panel input at fractional scaling and popup hit testing.
- Button II without an active cancel surface produces an explicit no-op and
  does not advance the dungeon tick.

All three runs passed. Their durations were 116.17, 116.36, and 115.60 seconds
(348.18 seconds total). The exact command was:

```sh
FIRESTAFF_TEST_SCRATCH_ROOT=/external/scratch/theron-input \
  ctest --test-dir /external/build/firestaff \
  --output-on-failure --repeat until-fail:3 \
  -R '^theron_v1_sdl_input_events$'
```

The scratch path is intentionally outside the checkout. The test itself
prints a final pass line naming authentic Theron SDL routes and checks its
per-route debug receipts and action logs before passing.

## Scope

This proves the enumerated native input and source-backed item actions on the
US Akutuba startup route. It does not prove retail input equivalence beyond
the tested actions, Japanese runtime input parity, a source-driven dungeon
viewport, combat, audio, save progression, or complete Theron parity. The
separate real-media data checks for both regions are recorded in CTest; this
input capture is US-only.
