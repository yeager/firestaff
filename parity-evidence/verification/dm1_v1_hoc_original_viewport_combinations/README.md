# DM1 V1 Hall of Champions original viewport combination receipt

## Scope

This receipt records a four-frame original PC 3.4 DOSBox-X run after the
authenticated Hall-of-Champions recruit route. It supplies actual source
viewport states for panel close, an attempted forward step, and a right turn.
It is not a full map route, floor-pickup, Eye-scroll, or pixel-parity claim.

## Source and capture boundary

- Original PC 3.4 `DM.EXE` SHA-256:
  `4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4`.
- Emulator: DOSBox-X 2026.01.02.
- Input: focused physical-style X11 injection, original 320x200 coordinate
  system.

| Frame | Observed original state | SHA-256 |
| --- | --- | --- |
| 1 | Recruited Chani inventory panel | `3f220429f90882daab43317023901a4d3d90cb35002672c7a1a3e0e75b555990` |
| 2 | Panel closed; HoC champion portrait in viewport | `93df2601172d5f9e420df67a47d98e79e399182f2955142d465309992df1e69a` |
| 3 | Attempted forward movement; source position remains blocked | `62eef12d668b2a1ba07b52814e8fa9ffcc5bff9a01d082d87272077372e329d5` |
| 4 | Right rotation; source wall/viewport composition changes | `aaacc39fcd3aa24427ec2735e9ca974bec75dfeb78d7c6b81cd7c8ae6012a21a` |

The original frames remain outside Git with the legally-owned game media.
The route transcript records the close-panel click, keypad-forward attempt,
and keypad-right turn before their corresponding capture points.

## Right-facing forward pair

A second original transaction begins at the right-facing fourth state above.
Its first forward command produces a changed viewport; the immediate second
forward command leaves that new view unchanged. This is useful observational
movement evidence, but no coordinate or collision meaning is inferred from
the screenshots alone.

| Frame | Observed original state | SHA-256 |
| --- | --- | --- |
| 1 | Right-facing viewport before forward command | `fb675195bec0857f09332b1026f04aa680af7e44cd6898aa299fafac6ea04cbd` |
| 2 | Viewport after first forward command; composition changed | `508ac8830b0b291519433fdd4f1211a159c9be833cdb747306d5c5fef13ee194` |
| 3 | Viewport after immediate second forward command; composition retained | `b86538a80fcee110520d9d762bb9dfb0269b1e94dd156326ad816cebcc583cb5` |

## Five-frame turn and movement sweep

One further original run begins from that right-facing state, performs a
forward command, turns left, attempts a forward command, turns right, and
attempts a final forward command.  The screenshots establish the original
rendered output for this exact input sequence.  They do not, by themselves,
identify dungeon coordinates or prove collision semantics.

| Frame | Input boundary | Observed original state | SHA-256 |
| --- | --- | --- | --- |
| 1 | Right-facing forward | First changed right-facing corridor view | `508ac8830b0b291519433fdd4f1211a159c9be833cdb747306d5c5fef13ee194` |
| 2 | Left turn | Mirrored corridor composition | `cc4b0de33f709af66e97a0b99c759172ebdee7bab8aea7cc86c7e4a94c04ac67` |
| 3 | Forward attempt | Left-facing corridor remains visible with changed status state | `2b07359cc4993f9283e344e1b6effb260e6e82264dc38752c4b117c6f3920384` |
| 4 | Right turn | Returns to the right-facing corridor composition | `8b5b1dbe63503e5bd6862f9d7e8133fb1064e9da2d9b712933ce5040656cd85b` |
| 5 | Forward attempt | Repeats the corresponding right-facing corridor view | `f4592378f3d586d497150529282bbb9a8321fb242db3b972cf4f02a73472e19d` |

## Remaining work

Additional original routes must still cover successful movement, floor pickup,
an actual scroll in the action hand, Eye-scroll text, doors, and varied
far-to-near viewport compositions before this can support broad DM1 parity.
