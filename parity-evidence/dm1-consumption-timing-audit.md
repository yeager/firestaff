# DM1 consumption timing audit

## Source contract

ReDMCSB `PANEL.C` F0349, lines 1928-1944, draws four alternating mouth
icons with F0022_MAIN_Delay(8) after each frame, then requests C08 swallow.
Potions and waterskins retain their object and bypass that animation loop.
This is a synchronous source command; advancing ordinary gameplay while
waiting must not be assumed equivalent to the source delay.

The visible food panel has a separate ordering boundary: F0349 changes
Food at line 1918, but requests C08 at 1944 before marking statistics/panel
dirty and calling F0292 at 1945-1949. A full host redraw during the delay
must therefore preserve the consumer's previous displayed food value,
without rolling back the committed simulation value. Pixel verification
must load the authentic font as well as graphics: otherwise F0355 inventory
admission may reject the panel and leave unrelated background/status pixels.
The pending I34E implementation now snapshots that consumer's displayed food
and preserves the previously active food panel across leader-hand removal.
The latter removal otherwise clears the panel flag. Four bounded
original-graphics/font cases (Original/Modern and mouth/UseItem) pass: all
four pending whole-panel images match the pre-command panel, another
champion does not inherit the snapshot, and completion visibly changes the
panel. This proves indexed framebuffer behavior, not a host display capture.
The independent full original DOS object corpus passes (104.21 seconds,
no skip) with an allocated original food record, unchanged pending panel
pixels, changed completion pixels and exactly one original C08 buffer enqueue.

`BASE.C` F0022_MAIN_Delay, lines 1584-1593, confirms that each delay unit
calls M526_WaitVerticalBlank. Thus the four explicit delays consume 32 source
VBlank waits, not 32 simulation ticks. This is not necessarily the total
command duration: viewport presentation may add synchronization. The
platform-specific M526 implementation remains
the authority for the actual interrupt/refresh clock.

## Current runtime gap

Local implementation status (2026-09-06): an explicit I34E food
command now separates palette-edge/frame/delay completion, gates gameplay
while pending and discards simulation debt in the main loop. Bounded tests
cover both input routes, 36 source edges, repeated input, release, shutdown
and fractional clock accumulation. The startup binding is covered by original-media tests:
it requires the selected archive's original I34E GRAPHICS.DAT, DM.EXE and
VGA hashes, including identity with the selected graphics member. It uses
25,175,000 / 359,200 Hz. Loose files and unverified editions are not bound.
The original archive startup regression passed (99.84 seconds), including
automatic clock selection. A subsequent admission tightening requires exact
top-level `DATA/GRAPHICS.DAT`, `DM.EXE` and `VGA` members so sibling packages
cannot supply the program/driver hashes. The tightened positive case passes
(99.72 seconds). The dedicated four-case package test also passes (0.91 seconds,
no skips): original bytes bind, while missing VGA, genuine EGA substituted
for VGA, and sibling-directory program/driver members remain unbound.
Each case checks that the selected archive still owns runtime GRAPHICS.DAT.
Nested/renamed packages remain unbound until package-root identity is proven.
Neither result establishes a full hardware timing gate.
Presentation now requires
a matching serial acknowledgement after successful host present; elapsed
edges before that acknowledgement cannot satisfy the next eight waits.
The original DOS corpus passes (109.10 seconds) with allocated source food
and C08 from its selected archive: no samples queued before edge 36, exactly
the source buffer's sample count queued on completion, unchanged marker
identity and no extra queueing on a subsequent edge. This is successful
source-buffer queueing, not audible output or raster-phase capture.
`playedMarkerCount` is unsuitable as a no-generated-audio assertion for this
DOS path because it also counts successful original-buffer playback; the
regression uses queue length and marker identity instead. Exact wall-clock
phase still requires further evidence beyond automatic package binding.

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

### DOS PC3.4 driver wait chain

Source inspection resolves the additional viewport wait for the PC driver:

- `DRAWVIEW.C:823-834`: F0097(C0) calls F0694 with the inventory palette
  (or LIGHT0 at the entrance), even when no palette-index change occurred.
- `IMAGE.C:140-144`: the PC branch dispatches F0694 through VIDRV_08.
- `VIDEODRV.C:3439-3444`: with G4094_CURTAIN_FLAG equal to one, palette
  application invokes F8156; `:3319` calls F8153 before writing the palette.
- `VIDEODRV.C:3163-3184`: F8153 polls port 0x3DA bit 3 until it is low,
  then until it becomes high. These are edge waits, not a millisecond sleep.

Consequently normal visible VGA food processing performs four repetitions
of palette wait, viewport blit and eight explicit delay waits: 36 driver
wait calls, not merely 32. The palette wait is conditional on the curtain
state; do not make it an unconditional property of every platform or state.
This source count does not establish an exact elapsed duration from input:
the first edge depends on phase and rendering/palette work also consumes
time. An emulator capture remains required for wall-clock/phase parity.
The inspected VIDEODRV implementation is selected by MEDIA701_I34E.
I34M shares the high-level F0097/IMAGE dispatch, but its actual driver
artifact still needs resolution before inheriting the same low-level claim.

### Revalidated integration points (2026-09-06)

### Original DOS execution probe (2026-09-06)

A bounded 20-second DOSBox-X 2026.01.02 run of the existing French DOS
original-media copy, using `DM.EXE -vv -sn -pk`, reports INT10 mode 13 and
then VGA refresh 70.086 Hz. The preceding shell mode 3 reports 70.087 Hz;
these are distinct transitions, not a shell-only refresh observation.
Local evidence: `.codex-scratch/dos-raster-oyDOTL/vga.log`, lines 80-83.
Executable SHA-256: `64014cc79af9dfb4b3d547e3744feb48f610c32636248975d8c6084a6f55ce22`.
VGA driver SHA-256: `4d9815e777e135bf69e3575fea533128b6073ae8c6b5282c24529c606f95af3b`.
The emulator used default configuration, SDL dummy video/audio and a separate
1.3 MB working copy; original media was not changed. The timeout terminated
the emulator (exit 137), not a successful game exit. No food interaction,
register dump or phase capture was obtained, so this establishes the game's
mode/rate observation only, not complete food timing or exact rational rate.

Candidate VGA cadence evidence (not a game capture): DOSBox-X upstream
revision `c3b26856993fb9446e4be7a7f8de9bebc8219b56` lists mode 13h with
100 horizontal character clocks and 449 scanlines in
[int10_modes.cpp](https://github.com/joncampbell123/dosbox-x/blob/c3b26856993fb9446e4be7a7f8de9bebc8219b56/src/ints/int10_modes.cpp#L91).
Its standard VGA clock-zero path uses 25,175,000 Hz, divides by eight for
eight-dot character clocks, then computes frame frequency from both totals
in [vga_draw.cpp](https://github.com/joncampbell123/dosbox-x/blob/c3b26856993fb9446e4be7a7f8de9bebc8219b56/src/hardware/vga_draw.cpp#L6823).
This implies the rational rate 25,175,000 / 359,200 Hz, approximately
70.0863 Hz, under those register settings and without a refresh override.
Verify the original game's actual raster settings before binding this
candidate; neither its input phase nor a live DM1 capture is established
by the emulator's default mode table.

Driver lineage is now established by direct SHA-256 comparison: `VGA` in
the original English DOS 3.4 ZIP, the French working copy above and
`reference/redmcsb-20210206/Reference/Original/I34E/VGA` are byte-identical
(the driver hash above). Thus the French run used the exact I34E driver
bytes also shipped in the English archive; this is stronger than inferring
driver equivalence from language or GRAPHICS.DAT format alone.
Offline LZEXE decompression yields SHA-256
`63d4f2fa66cb6dc2d97ca961f9f65a2dcebd016614b116d7f4381210b5a8ee6f`.
Its 0x200-byte executable header is excluded from load-module offsets:
0x1006/0x1008/0x100A selects BIOS mode 13h; 0x098D polls port 0x3DA.
The eight decoded OUT instructions in the identified driver code address
DAC ports 0x3C8/0x3C9, not CRTC/miscellaneous clock registers. This supports
the standard raster for this driver, without claiming that every executable
in an arbitrary collection archive behaves identically. The offline
decompressor and unpacked binary are not runtime dependencies or shipped
game assets. Startup's
`verifiedAssetMd5` identifies admitted game assets, not by itself this VGA
driver, so a clock binding must not treat the generic nonlegacy-DOS flag
as proof of the verified driver/normal-palette contract.

The gap remains in the current tree: `m11_tick_v1_mouth_animation` is
called from `M11_GameView_AdvanceIdleTick`. Both mouth input and alternate
UseItem need to share a pending food-command completion path; fixing only
the sound call at one entry leaves the other route divergent.

`main_loop_m11.c` already passes elapsed wall time to
`DM1_V1_VBlankTiming_Update`, but that does not establish an appropriate
food-delay clock. `dm1_v1_vblank_timing.c:79-80` consumes fixed integer
periods; `dm1_v1_vblank_timing.h` specifies 20 ms PAL and 16 ms Towns.
The latter is 62.5 Hz, not 60 Hz: subtracting 16 repeatedly cannot recover
the omitted fractional period. Do not inherit that approximation for a
new source-delay implementation or call it authenticated capture timing.

Implement a nonblocking command state with explicit completion, serviced
from an edition-appropriate source clock before ordinary simulation and
input dispatch. Preserve the original effect-before-animation ordering
shown by PANEL.C F0349:1910-1944 while deferring command completion and
C08. Confirm the additional F0097 viewport synchronization separately.

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
