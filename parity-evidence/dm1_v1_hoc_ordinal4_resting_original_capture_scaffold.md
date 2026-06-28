# DM1 V1 HoC Ordinal 4 RESTING Original-Capture Scaffold

Status: `SCAFFOLD_READY_NO_PIXEL_PARITY_CLAIM`

This is a capture-readiness scaffold for the Hall of Champions LEIF
(`ordinal=4`) RESTING row. It does not promote original DOS pixel evidence yet.

## Source Locks

- ReDMCSB `COORD.C:1693-1698`: PC 3.4 viewport origin `(0,33)`.
- ReDMCSB `DUNVIEW.C:525`: D1C champion portrait box `{96,127,35,63}`.
- ReDMCSB `DUNVIEW.C:3913-3928`: C026 portrait blit with 32x29 cells.
- ReDMCSB `DEFS.H:821-826`: `M027/M028` portrait atlas math.
- ReDMCSB `COMMAND.C:414`: C145 rest icon viewport zone.
- ReDMCSB `COMMAND.C:453-455`: C146 wake input while party is resting.
- ReDMCSB `COMMAND.C:2336-2363` and `CHAMPION.C:1382-1401`: rest/wake state transition.
- Firestaff `src/engine/m11_game_view.c`: current RESTING overlay at framebuffer `(100,70,120,30)`.

## Locked Scaffold

- Screen: `320x200`.
- Viewport crop: `(0,33,224,136)`.
- D1C portrait rect: viewport `(96,35,32,29)`, framebuffer `(96,68,32,29)`.
- LEIF atlas cell: ordinal `4`, C026 source rect `(128,0,32,29)`.
- Firestaff RESTING overlay: framebuffer `(100,70,120,30)`.
- D1C/RESTING overlap: `(100,70,28,27)` = `756` pixels.
- Future artifact root:
  `verification-screens/dm1-v1-hoc-ordinal4-resting-original/`.
- Future shot labels:
  `hoc_ordinal4_pre_rest`,
  `hoc_ordinal4_resting_overlay`,
  `hoc_ordinal4_wake_repaint`,
  `hoc_ordinal4_post_wake_candidate_return`.

## Verification

CTest target:
`dm1_v1_hoc_ordinal4_resting_original_capture_scaffold_probe`

Binary:
`firestaff_dm1_v1_hoc_ordinal4_resting_original_capture_scaffold_probe`

## Non-Claims

- No original DOSBox capture is committed here.
- No Firestaff-vs-original pixel diff is claimed.
- The existing Firestaff runtime gates still own sleep/wake repaint behavior;
  this scaffold only makes the next original-capture attempt reproducible.
