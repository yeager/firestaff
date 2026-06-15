# Pass764 CSB V1 viewport F0115 wall text ornament source-lock gate

Status: source-locked local fixture, branch A.

Claim: the CSB V1 D1C wall text/ornament route is pinned as a contract-only
pixel-detail gate. The synthetic framebuffer proves that the source-locked
D1C wall path can render a non-zero wall-text overlay after the F0107
wall-ornament ordinal/coordinate-set/alcove path enables the F0115
`C0x0000_CELL_ORDER_ALCOVE` thing pass. This does not claim original DOS pixel
parity and does not load game data.

## ReDMCSB Anchors

- `DUNVIEW.C F0115:4547-4581` defines the thing-pass function and documents
  that first nibble zero is the wall-alcove object path.
- `DUNVIEW.C F0107:3502-3938` locks wall-ornament ordinal decrement,
  coordinate-set selection, inscription/text drawing, C10 transparent blits,
  D1C clickable/portrait handling, and alcove return.
- `DUNVIEW.C F0124:7825-7843` locks the D1C wall route: draw D1C wall,
  call F0107 with `M552_FRONT_WALL_ORNAMENT_ORDINAL` and
  `M587_VIEW_WALL_D1C_FRONT`, then call F0115 with
  `M606_VIEW_SQUARE_D1C` and `C0x0000_CELL_ORDER_ALCOVE` only when F0107
  returns alcove.
- `DUNVIEW.C F0128:8318-8486` locks viewport frame setup and the early
  far-square F0115 ordering before D1 routes.
- `DEFS.H:2088` anchors `C10_COLOR_FLESH`.
- `DEFS.H:2527` anchors `C5_HEIGHT`.
- `DEFS.H:4139-4153` anchors the nearby zone-family constants required by the
  pass prompt.
- `DEFS.H:5576` declares `G0208_aaauc_Graphic558_DoorButtonCoordinateSets`,
  the adjacent wall-control coordinate storage named in the prompt.

## Lineage Anchors

- `CSB-lineage Viewport.cpp:1192-1209,1903-1915,1930-1944`
- `CSBWin Viewport.cpp:1903-1915`

## Firestaff Files

- `include/csb/csb_v1_viewport_f0115_wall_text_ornament_pc34_compat.h`
- `src/csb/csb_v1_viewport_f0115_wall_text_ornament_pc34_compat.c`
- `tests/test_csb_v1_viewport_f0115_wall_text_ornament_pc34_compat.c`
- `tools/verify_pass764_csb_v1_viewport_f0115_wall_text_ornament_pc34_compat.py`

## Non-Claims

- No original DOS pixel parity claim.
- No real-asset bitmap hash or game-data load claim.
- No DM1, DM2, Nexus, or Theron behavior claim.
- No change to the runtime CSB viewport renderer.
