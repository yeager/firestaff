# DM1 consumption timing audit

## Source contract

ReDMCSB `PANEL.C` F0349, lines 1928-1944, draws four alternating mouth
icons with F0022_MAIN_Delay(8) after each frame, then requests C08 swallow.
Potions and waterskins retain their object and bypass that animation loop.
This is a synchronous source command; advancing ordinary gameplay while
waiting must not be assumed equivalent to the source delay.

## Current runtime gap

At commit `637318a85`, `m11_process_v1_mouth_click` requests swallow before
starting `m11_start_v1_mouth_animation`. Alternative food UseItem requests
swallow immediately without starting that animation. The animation countdown
is driven by `M11_GameView_AdvanceIdleTick` and does not establish source
VBlank timing. Releasing/clearing the mouth visual resets the animation state.

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
