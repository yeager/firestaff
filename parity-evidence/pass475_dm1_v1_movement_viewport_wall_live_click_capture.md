# Pass475 — DM1 V1 movement/viewport/wall live click capture

Status: `BLOCKED_PASS475_CLICK_REACHED_QUEUE_NO_MOVEMENT_HANDLER`

click run reached command queue, but not turn/step handler before timeout

## Evidence summary
- Source audit ok: `True`
- Queue hit: `True`
- Movement handlers hit: `[]`
- F0128 hit: `False`
- Post-present seam hit: `False`
- Capture count: `0`

## Artifacts
- Manifest: `parity-evidence/verification/pass475_dm1_v1_movement_viewport_wall_live_click_capture/manifest.json`
- Transcript: `parity-evidence/verification/pass475_dm1_v1_movement_viewport_wall_live_click_capture/pass475_dm1_v1_movement_viewport_wall_live_click_capture_runtime.clean.txt`
- Command log: `parity-evidence/verification/pass475_dm1_v1_movement_viewport_wall_live_click_capture/pass475_dm1_v1_movement_viewport_wall_live_click_capture_command_log.json`
- Click log: `parity-evidence/verification/pass475_dm1_v1_movement_viewport_wall_live_click_capture/pass475_dm1_v1_movement_viewport_wall_live_click_capture_click_log.json`

## Promotion rule
Fresh wall/viewport frames are promotable only after the F0128 composition and F0097/VIDRV post-present seam in this live run.
