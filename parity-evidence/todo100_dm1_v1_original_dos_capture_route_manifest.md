# TODO100 DM1 V1 original DOS capture route manifest

Status: `TODO100_DM1_V1_ORIGINAL_DOS_CAPTURE_ROUTE_MANIFEST_READY`

This gate defines one deterministic original PC 3.4 DOSBox capture route for a future same-state viewport pairing. It is intentionally skip-safe: it validates route metadata and local tooling only.

## Route

- route id: `dm1_pc34_hoc_wuuf_south_return_v1`
- route events: `wait:9000 enter enter wait:1800 shot:hoc_start_south_1_3 kp8 wait:1200 shot:hoc_forward_south_1_4 kp8 wait:1200 shot:hoc_wuuf_south_return_1_5`
- expected shots: `3`
- target: WUUF / THE BIKA ordinal 13, Hall of Champions south_return pose `(map 0, x=1, y=5, SOUTH)`

## Expected Frames

| # | Label | Tuple | Crop | Pairing role |
|---:|---|---|---|---|
| 1 | `hoc_start_south_1_3` | `m0 x1 y3 SOUTH` | `01_hoc_start_south_1_3_original_viewport_224x136.ppm` | baseline Hall of Champions start viewport |
| 2 | `hoc_forward_south_1_4` | `m0 x1 y4 SOUTH` | `02_hoc_forward_south_1_4_original_viewport_224x136.ppm` | one legal forward step toward the WUUF south_return pose |
| 3 | `hoc_wuuf_south_return_1_5` | `m0 x1 y5 SOUTH` | `03_hoc_wuuf_south_return_1_5_original_viewport_224x136.ppm` | target frame for future original-vs-Firestaff viewport hash pairing |

## Anchors

- capture script: `scripts/dosbox_dm1_original_viewport_reference_capture.sh` ok=True
- Firestaff pairing probe: `probes/m11/firestaff_dm1_v1_champion_mirror_portrait_13_south_return_portrait_rect_position_runtime_probe.c` ok=True
- gap document: `docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md` ok=True

## Source Anchors

- COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC
- CLIKMENU.C:180-347 F0366_COMMAND_ProcessTypes3To6_MoveParty
- DUNGEON.C:2573,2608-2612 C127 sensorData front-wall champion ordinal
- DUNVIEW.C:3913-3928 D1C champion portrait blit
- DUNVIEW.C:525 G0109 champion portrait viewport rectangle
- DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF redraw path

## Non-Claims

- No original assets, screenshots, DOSBox captures, or user-supplied game data are committed.
- No Firestaff-vs-original pixel parity or viewport-hash match is claimed.
- The route labels are metadata for a later operator-run capture and pairing pass.
