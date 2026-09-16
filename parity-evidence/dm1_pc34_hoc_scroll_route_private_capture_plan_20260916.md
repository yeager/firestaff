# DM1 PC 3.4 Hall of Champions scroll-route capture plan

## Scope

This is a routing receipt for the outstanding original DOS scroll-to-Eye
capture. It contains no game frame, game-data byte stream, save, or emulator
dump. It is not a pixel-parity or runtime-success claim.

## Source lock

- `DATA/DUNGEON.DAT` SHA-256:
  `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`.
- The PC 3.4 dungeon decoder read 14 maps and 35 scroll Things directly from
  that file. No alternate-platform asset or generated dungeon was used.

## Candidate

The nearest scroll from the existing Hall C127-route anchor is Thing `0` on
map `0`, level `0`, square `(4,15)`. Its source record is closed and references
text-string Thing `33`.

The existing Hall anchor is `(7,16)`, facing south. A wall-only breadth-first
route to the candidate square is:

```text
(7,16) -> (7,15) -> (6,15) -> (6,14) -> (6,13) -> (6,12)
-> (7,12) -> (7,11) -> (7,10) -> (7,9) -> (6,9) -> (5,9)
-> (4,9) -> (4,10) -> (4,11) -> (4,12) -> (4,13) -> (4,14)
-> (4,15)
```

The target is a stair square, so the live route must verify that its floor-item
cell is selectable before it is promoted. The listed coordinates are a
data-derived navigation candidate, not a claim that all transitions are
walkable, safe, or free of original events.

## Input translation for the candidate route

The PC 3.4 capture route atlas binds the original keypad controls as follows:
`KP5` forward, `KP4` turn right, `KP6` turn left, `KP1` strafe left, and
`KP3` strafe right. Starting at the stated anchor facing south, the candidate
walk translates to this *unexecuted* command sequence:

```text
KP4 KP4 KP5 KP6 KP5 KP4 KP5 KP5 KP5 KP5 KP4 KP5
KP6 KP5 KP5 KP5 KP6 KP5 KP5 KP6 KP5 KP5 KP5 KP5 KP5 KP5
```

This is a mechanical translation of the BFS edges only. The capture harness
must take state frames after each direction change and immediately before the
floor-item interaction. A blocked move, forced turn, teleporter, encounter,
or non-selectable stair cell invalidates the candidate and must be recorded as
such; it must not be repaired by inserting guessed movement.

## Required capture outcome

Use the original PC 3.4 executable through DOSBox-X only as private capture
tooling. Starting from the authenticated Hall route, record and health-check:

1. the candidate square before pickup;
2. the scroll in the action hand;
3. Eye pressed with the scroll in hand; and
4. Eye released after the original scroll panel is visible.

Keep the resulting frames and emulator recordings private. Publish only a
hash-only receipt after verifying the route state and frame provenance.
