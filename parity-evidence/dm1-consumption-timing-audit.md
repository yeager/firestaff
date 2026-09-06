# DM1 consumption timing audit

## Source contract

ReDMCSB `PANEL.C` F0349, lines 1928-1944, draws four alternating mouth
icons with F0022_MAIN_Delay(8) after each frame, then requests C08 swallow.
Potions and waterskins retain their object and bypass that animation loop.
This is a synchronous source command; advancing ordinary gameplay while
waiting must not be assumed equivalent to the source delay.

`BASE.C` F0022_MAIN_Delay, lines 1584-1593, confirms that each delay unit
calls M526_WaitVerticalBlank. Thus the four explicit delays consume 32 source
VBlank waits, not 32 simulation ticks. This is not necessarily the total
command duration: viewport presentation may add synchronization. The
platform-specific M526 implementation remains
the authority for the actual interrupt/refresh clock.

## Current runtime gap

At commit `637318a85`, `m11_process_v1_mouth_click` requests swallow before
starting `m11_start_v1_mouth_animation`. Alternative food UseItem requests
swallow immediately without starting that animation. The animation countdown
is driven by `M11_GameView_AdvanceIdleTick` and does not establish source
VBlank timing. The explicit mouth-visual clear resets the animation state;
current callers include inventory switching/toggling. Button release alone
has not been established as a caller and must be tested separately.

`main_loop_m11.c` dispatches AdvanceIdleTick from the accumulator using
M11_GameView_IdleTickIntervalMs (around lines 7444-7450 and 7705-7713).
Its normal game-tick default is 200 ms. This establishes a clock-domain
mismatch in the animation countdown; simply moving the sound to the last
existing animation tick would retain that mismatch. Boot probes directly
call AdvanceIdleTick and therefore cannot prove wall-clock duration.

## Platform dispatch evidence

`DEFS.H:3134-3166` maps Atari ST waits to Vsync(), early Amiga to WaitTOF(),
and later/platform-specific builds to F0693. Do not select an unrelated
F0693 implementation just because its function name matches: for example,
`VBLANK.C:626` contains a 6809 implementation, whereas
`DRAWVIEW.C:700-706` uses the FM Towns EGB palette service. The per-edition
preprocessor selection is part of the reference contract.

`PANEL.C:1934-1936` invokes F0097_DrawViewport inside the frame loop for
later editions. Its platform-specific synchronization must be included in
the wall-clock proof rather than equating every edition to a flat 640 ms.

The original-media tests establish accepted sound selection and reject
generated-marker substitution. They intentionally do not establish the
ordering or elapsed time required above. Their immediate-food-sound assertion
must change together with a correct deferred-consumption implementation.

## Required implementation and verification

- Model food command completion explicitly, including pending sound and
  behavior on button release, panel closure, repeated input and shutdown.
- Drive the four delays from the appropriate source clock, not an assumed
  equivalence between simulation ticks and VBlanks.
- Preserve source command/timeline ordering during the delay; do not merely
  defer sound while allowing unrelated simulation to run unchecked.
- Verify no early C08, exactly one C08 after the fourth delay, and no sound
  on rejected food/water use. Retained-object drinks must remain immediate.
- Compare platform-appropriate emulator audio/video captures before claiming
  waveform, wall-clock timing or complete consumption parity.

No emulator timing capture was obtained in this audit. Savegames are not
required to inspect this command path and remain out of scope.
