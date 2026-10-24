# Nexus retail hand-pointer input capture (2026-10-25)

## Setup

Ymir 0.4.1 ran on TRV2 with `--debug --fast-forward` and the Japanese retail
Nexus disc. The profile was copied from the previously verified ControlPad
profile into a new private directory. Its backup RAM had the blank/init hash
`6a0d5bcdf8c8243c4f6b8666e76aa0fc8f4eef56df7993b414f3fa7bb3a3e141` when the
profile was copied. No game save or save state was loaded.

| Input | SHA-256 |
| --- | --- |
| Retail cue | `878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699` |
| Japanese Saturn BIOS | `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac` |
| Blank backup RAM | `6a0d5bcdf8c8243c4f6b8666e76aa0fc8f4eef56df7993b414f3fa7bb3a3e141` |

The ControlPad profile maps `W/S/A/D` to the Saturn D-pad, `O` to Z, `J` to A,
and `F` to Start. Holding F at the retail title prompt opened the title menu;
J selected `NEW GAME`. After the dungeon view stabilized, O entered the retail
orange hand-pointer mode.

## Observed hand-pointer movement

The screenshots are full 1280×800 Ymir captures of the retail game. The orange
cursor bounds were measured from the original PNGs by selecting its orange
pixels. The center positions below are approximate because the cursor sprite
is animated and antialiased. Each held direction lasted about 0.2 seconds of
host time while Ymir was in fast-forward mode; these measurements establish
direction and scale, not a normal-speed repeat rate.

| Capture | Center (x, y) | Change from prior measurement |
| --- | ---: | ---: |
| Before W | (659.5, 438.7) | — |
| After W | (659.5, 389.9) | (0, -48.7) |
| After S | (659.7, 432.8) | (0, +42.9) |
| After A | (620.1, 432.6) | (-39.6, 0) |
| After D | (665.4, 432.8) | (+45.3, 0) |
| After W+A | (628.4, 393.0) | (-37.0, -39.8) |

The captures confirm that Z enters the orange cursor mode, all four D-pad
directions move it, and simultaneous directions move it diagonally. The retail
manual documents eight-way hand-pointer movement (see the control-table link
in [`docs/nexus_input.md`](../../../docs/nexus_input.md)). This trace only
measured the four cardinal directions and one diagonal.

## Mouse limit

The captures above use a ControlPad; they do not prove desktop-mouse behavior.
An Xvfb/XTest mouse-motion attempt did not move the retail cursor reliably,
even after Ymir was changed to a Shuttle Mouse profile. Do not treat that
attempt as evidence of a working mouse route or infer retail click actions
from the hit rectangles. The retail callback/action mapping is still
unresolved in
[`nexus-mouse-callback-dispatch-20261024`](../nexus-mouse-callback-dispatch-20261024/).

## Cursor texture join

A Ymir `--debug` VDP1 command capture from the same retail title identifies
the hand sprite as a 16×16, 4bpp texture. The command at VDP1 VRAM `0x12e0`
reads 128 bytes from source byte address `0x7df00`; those bytes match
hash-verified `ITEM.IBS` image 65 at offset `0x5380`. The captured VDP1 CLUT
resolves to retail `ITEM.IBS` palette 6. Firestaff composes this source image
and palette as a separate RGBA overlay so it does not replace the dungeon's
indexed palette. The verified retail file hash is recorded in
[`docs/VERIFIED_HASHES.md`](../../../docs/VERIFIED_HASHES.md).

The screenshots in this directory remain authentic retail captures, not
Firestaff screenshots or proof of full parity. Firestaff now has a host Z
toggle, arrow-key and mouse-motion pointer positioning, and the source-joined
hand sprite. Desktop mouse motion is a Firestaff host behavior; retail mouse
motion and pointer click/action routing remain unverified.
