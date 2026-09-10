# DM1 V1 Hall of Champions Eye-held original capture receipt

## Scope

This receipt records two real frames from the original DOS PC 3.4 program
after Chani was recruited in the Hall of Champions.  The first frame has the
Food/Water panel visible while an inventory item is held.  The second records
the original Eye control while it is held down.  This is evidence that the
original interaction channel and its panel redraw were reached; it is not a
scroll-reading capture and it does not establish pixel parity.

## Source identity and capture boundary

- Program: original PC 3.4 `DM.EXE`, SHA-256
  `4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4`.
- Host: DOSBox-X 2026.01.02 with focused X11 physical-style input injection.
- Capture: host-window capture, cropped to the original 320x200 canvas with
  nearest-neighbour scaling.
- The original frames remain outside Git with the legally owned game media.

## Observed frames

| Frame | Observed source state | SHA-256 |
| --- | --- | --- |
| 1 | Chani inventory; Food/Water panel visible while the selected item is in the action hand | `c9afad0e357fefe0b21f59d354cec67df501c6f410a8a890e99a4dfab05cded1` |
| 2 | Same recruited inventory session while the Eye at original coordinate `(20,54)` is held | `610ad13c55cfbe2f96f4ee2e040258bbe4ffd6be3a8f8abeaf1fb428528d7977` |

The raw images differ, and both are 320x200 source-canvas captures.  The
observed interaction was injected as a held press rather than a zero-duration
click because the original UI samples the button state asynchronously.

## Reproduction boundary

Start from the authenticated PC 3.4 Hall recruit route, open Chani's
inventory, select the occupied slot at `(54,84)`, then hold the Eye at
`(20,54)` while taking the second capture.  The reusable harness is
`scripts/dosbox_dm1_original_viewport_reference_capture.sh` with `DM -vv -sn
-pm`, global input mode, and host capture.

## Remaining work

This evidence intentionally does **not** claim an Eye-scroll result: the
captured held item is not an authenticated scroll.  A separate original route
must place a real scroll in the action hand and capture the resulting
source-owned text panel before the Eye-scroll requirement can close.
