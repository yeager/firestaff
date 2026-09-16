# CSB FM Towns F31 Entrance runtime gate

## Scope

This receipt records a native Firestaff runtime check against the original
FM Towns CSB archive.  It covers the defect where the moving Prison doors
revealed C004's red placeholder instead of the F0797/F0128 C255
micro-dungeon viewport.

It is an implementation gate, not an original-emulator capture and not a
claim of complete visual parity.

## Source media

- Original FM Towns English-capable CSB archive SHA-256:
  `54b40c1fd0b18ca2df1dbcd70c6cb07fb0333d540b99f8108b1fca84aa192b88`.
- Media is consumed directly from its ZIP archive.  No extracted game data,
  generated image, or substituted PC asset is accepted by this route.

## Reproduced check

`test_csb_v1_fmtowns_m11_game_handoff` was built and run with software
presentation and audio output disabled.  The test drives the original F31
`SWTCHTW` Game rectangle, then renders every source opening tick.

It verifies that:

1. the Game click enters the source-owned Prison opening transition;
2. a C002/C003 door frame is visible during that transition;
3. at opening steps 10 through 20, the uncovered central aperture differs
   from C004 at the same coordinates, proving that the F0797/F0128 C255
   micro-dungeon viewport replaced the red placeholder; and
4. the handoff reaches the original MINI.DAT runtime graph at level 4,
   position `(22,18)`, facing direction `2`, with its original timeline and
   group counts.

The successful result was:

```text
PASS: real FM Towns SWITCHTW -> CHTWE.EXP entrance handoff
```

The broader parity requirement remains open: authentic external captures of
the CSB HUD, viewport, doors, and audio are still required for Atari, Amiga,
and FM Towns.
