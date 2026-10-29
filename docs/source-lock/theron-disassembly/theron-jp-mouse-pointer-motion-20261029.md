# Japanese Theron pointer motion in PCE mouse mode

## Reference setup

A cold Mednafen 1.32.1 `pce_fast` run used the authentic Japanese Rev. 1 disc
and canonical headerless System Card 3.0 body. Mednafen loaded the original
cue and selected PCE Fast. Its log reports the Japanese SBI sidecar missing;
the full headered card image was unavailable. The run used an isolated
`HOME`, `autosave 0`, and no state-load argument. Game media stayed in the
TRV2 user data directory.

The cold input route held Run for four seconds and Button I for four seconds
at the new-game, file, stage, and first story steps, followed by six separate
250 ms Button I presses. Mednafen's port 1 device selector then changed the
device from gamepad to mouse. Two screenshots after moving the host mouse to
positions 150 screen pixels apart show a 708-pixel difference within a 41 x 51
pixel box around the in-game hand cursor. The dungeon background, HUD, and
remaining screenshot pixels are unchanged. This visually ties relative host
mouse movement to movement of the original hand cursor in the authentic game.

The PCE Fast module documents a mouse device for port 1 in its
[official settings reference](https://mednafen.github.io/documentation/pce_fast.html).
The retail TurboGrafx-16 manual describes D-pad cursor movement and Button I
actions on pp. 4–6 in the
[instruction manual](https://www.tg-16.com/contributors/bernie/Dungeon_Master/Dungeon_Master.pdf),
but does not document mouse input. This capture supplies game-specific visual
evidence for pointer motion.

## Limits

The probe produced screenshots only. It has no guest-input trace, source
coordinate receipt, command-buffer write, target receipt, or state-transition
receipt. A later left click was followed by a black viewport, so its result is
inconclusive. Do not infer hit regions, button behavior, or a native input
mapping from this run. Direct Firestaff world-coordinate clicks and
state-driven rendering remain open.

Private screenshots, log, notes and hashes are at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-mouse-pointer-motion-20261029-r1/`.
The pointer-motion screenshot SHA-256 values are:

| Artifact | SHA-256 |
|---|---|
| `after-mouse-move.png` | `339b7387b29c2ca3b4324cb94242e76433db9a5eb6bd1bc262218eaa944dc6fe` |
| `after-mouse-move-right.png` | `562f2980b0f529784ce2f3cd0179eb95b556e60952a2b0b9c2c0cc48f4f6ade7` |

No game media, BIOS, save-state, or raw RAM snapshot is in the evidence bundle.
