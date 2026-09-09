# DM1 V1 Hall of Champions inventory pickup: original capture receipt

## Scope

This receipt records a repeatable capture from the original DOS PC 3.4
program. It covers a recruited Chani's inventory panel and one item transfer
from an occupied inventory slot into the action hand, followed by placement
in a second slot. It is not evidence for a floor pickup, an Eye-scroll
interaction, or viewport pixel parity.

## Source identity

- Program: original `DM.EXE` (PC 3.4), SHA-256
  `4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4`.
- Capture host: DOSBox-X 2026.01.02.
- Input method: focused X11 physical-style injection, with original-frame
  coordinates.
- Capture mode: host window cropped to the trailing 320x200 DOS canvas with
  nearest-neighbour scaling.

## Observed frames

The private, legally-owned capture bundle contains the following raw 320x200
frames, in order:

| Frame | Meaning | SHA-256 |
| --- | --- | --- |
| 1 | Recruited Chani panel before transfer | `1fc88fecc78de75521d914fbf4217f43cafe18f71192ed8672df91fa57fe98ad` |
| 2 | Occupied slot selected; the item is visibly in the action hand | `e7b6fd200122ef9a5515fc01e107ec601a7fcbfe031f6ce4680dd4d36b96c8c2` |
| 3 | Action hand cleared after placement in the second slot | `4a75b8caa8a1870f8c2bd7319b1834b92a9ffedefda2b9c79e612f11f5e038cf` |

## Reproduction boundary

Use `scripts/dosbox_dm1_original_viewport_reference_capture.sh` with the
hash-verified PC 3.4 media, DOSBox-X, `DM -vv -sn -pm`, `global` input mode,
and host capture. The established Hall-of-Champions route must first recruit
Chani and open her inventory. From that stable C007 state, record a frame,
click original-frame slot `(54,84)`, record a second frame, click `(75,83)`,
then record the final frame.

The capture harness intentionally labels this as capture-channel evidence
only. Promotion to a broader gameplay-parity claim still requires an
independent source-owned route transcript and matching Firestaff runtime
frames.
