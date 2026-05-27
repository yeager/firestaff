# Pass504 - DM1 V1 original capture route preflight

Status: `PASS504_ORIGINAL_CAPTURE_ROUTE_PREFLIGHT_READY`

This gate checks the exact N2 prerequisites for the next original DM1 V1 movement/viewport/wall capture attempt. It does not run DOSBox and does not promote stale captures.

## ReDMCSB Source Locks
- `COMMAND.C:106-114` / `G0448_as_Graphic561_SecondaryMouseInput_Movement` - N2 route clicks must use source-owned PC34 movement/viewport boxes. ok=`True`
- `COMMAND.C:252-260,272-280` / `G0459_as_Graphic561_SecondaryKeyboardInput_Movement` - keypad/arrow route tokens are readiness inputs; promotion still needs handler/state proof. ok=`True`
- `CLIKMENU.C:142-174` / `F0365_COMMAND_ProcessTypes1To2_TurnParty` - turn labels are promotable only after F0365 mutates party direction. ok=`True`
- `CLIKMENU.C:224-347` / `F0366_COMMAND_ProcessTypes3To6_MoveParty` - step labels require the source legality path, not a screenshot taken immediately after input. ok=`True`
- `MOVESENS.C:738-818` / `F0267_MOVE_GetMoveResult_CPSCE` - accepted movement must update move-result/state before viewport evidence is labeled. ok=`True`
- `DUNVIEW.C:577-593,8318-8618` / `F0128_DUNGEONVIEW_Draw_CPSF` - wall/viewport captures must bind to the F0128 tuple and G0296 composition. ok=`True`
- `DRAWVIEW.C:709-858` / `F0097_DUNGEONVIEW_DrawViewport` - the capture seam is after the PC34 viewport-present blit, not setup/menu echo. ok=`True`

## N2 Preconditions
- `dosbox`: `/opt/homebrew/bin/dosbox` ok=`True`
- `xvfb-run`: `None` ok=`True`
- `xdotool`: `None` ok=`True`
- `python3`: `/usr/bin/python3` ok=`True`
- `dosbox-debug`: `None` ok=`False`
- `python3-pillow`: `python3 import PIL.Image` ok=`True`

## Canonical DM1 Inputs
- `DUNGEON.DAT` sha256=`d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` expected=`d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` ok=`True`
- `GRAPHICS.DAT` sha256=`2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` expected=`2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` ok=`True`
- `TITLE` sha256=`adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745` expected=`adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745` ok=`True`
- `DungeonMasterPC34/DM.EXE` sha256=`4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4` expected=`4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4` ok=`True`

## Capture Contract
- Program: `DM -vv -sn -pk`
- Wrapper: `DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run`
- Six `shot`/`shot:<label>` tokens are required by the capture script before normalization.
- Click centers are source-locked from `COMMAND.C`, but labels become promotable only after F0380 -> F0365/F0366 -> F0128 -> F0097/VIDRV proof.

## Secondary References
- Greatstone atlas: `/Users/bosse/.openclaw/data/firestaff-greatstone-atlas` exists=`True`
- Original DM canonical data: `/Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1` exists=`True`
- CSBWin: `/Users/bosse/.openclaw/data/firestaff-csbwin-source/CSBWin` exists=`True`; not used for this DM1 PC34 gate.

## Gate

- `python3 tools/verify_pass504_dm1_v1_original_capture_route_preflight.py`

Manifest: `parity-evidence/verification/pass504_dm1_v1_original_capture_route_preflight/manifest.json`
