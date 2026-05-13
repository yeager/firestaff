# Pass505 - DM1 V1 same-viewport capture blocker

Status: `PASS505_SAME_VIEWPORT_CAPTURE_BLOCKER_LOCKED`

## Decision

Runtime capture cannot be promoted yet. The fresh N2 original route produced two viewport states, so the capture path is not totally static, but the six post-entry labels still collapse to duplicate 4/2 hashes and no same-run source stops bind those frames to F0380/F0365/F0366 followed by F0128/F0097. No paired Firestaff frame for the same direction/X/Y/wall state is present.

## Fresh N2 original attempt

- capture count: `6` at `320x200`
- classes: `{'dungeon_gameplay': 4, 'wall_closeup': 2}`
- raw hash counts: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 4, 'fbeb1b82cd096c15c2346f254d9b2b2e8c1a8d0b8d100ba1751c4230c51e3dde': 2}`
- viewport crop hash counts: `{'701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c': 4, '1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81': 2}`
- route labels: `readiness_preflight, turn_left_after_vblank, turn_right_after_vblank, forward_after_vblank, turn_left_2_after_vblank, post_redraw_after_vblank`
- interpretation: The attempt reaches original gameplay and produces two visible viewport states, but six semantic labels collapse to 4/2 duplicate raw and viewport-crop hashes. The filenames are generic crop slots that drift from the route labels, and the attempt does not include source-bound F0380/F0365/F0366/F0128/F0097 stops.

## ReDMCSB source audit
- `GAMELOOP.C:90,164,215-219` `game_loop_next_f0128_after_input_wait` ok=`True` - A post-command screenshot is promotable only after the command wait boundary permits the next draw.
- `COMMAND.C:2045-2156` `f0380_dequeues_to_f0365_f0366` ok=`True` - Host labels are insufficient; the original run must prove a real dequeued turn/move command.
- `CLIKMENU.C:142-174` `f0365_turn_state_delta` ok=`True` - Accepted turns mutate party direction before the next viewport draw.
- `CLIKMENU.C:180-347` `f0366_move_state_delta` ok=`True` - Accepted steps must be tied to destination/move-result state, not only a captured wall closeup.
- `DUNVIEW.C:8318-8610` `f0128_uses_party_tuple_then_presents` ok=`True` - The original frame must bind to the same direction/X/Y tuple used for viewport composition.
- `DRAWVIEW.C:709-858` `f0097_viewport_present_boundary` ok=`True` - The compared screenshot must be at or after the G0296 viewport present boundary.

## Same-viewport promotion predicate

Capture one original frame and one Firestaff frame for the same map/X/Y/direction and wall/door state, with the original run proving F0380 pop/load -> F0365/F0366 state mutation -> later F0128 tuple composition -> F0097/VIDRV present for that shot. Until then, pass502 stays source/probe evidence only.

## Non-claims
- no original-vs-Firestaff pixel parity
- no promotion of pass505 images as reference artifacts
- no claim that F0365/F0366 are broken
- no DANNESBURK input or paths used

## Gate

- `python3 tools/verify_pass505_dm1_v1_same_viewport_capture_blocker.py`

Manifest: `parity-evidence/verification/pass505_dm1_v1_same_viewport_capture_blocker/manifest.json`
