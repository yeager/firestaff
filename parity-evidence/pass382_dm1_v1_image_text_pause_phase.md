# Pass382 — DM1 V1 IMAGE_TEXT forced-pause phase classification

Status: `BLOCKED_PASS382_IMAGE_TEXT_PAUSE_EXPECTED_RENDER_HELPER_NOT_WRONG_PHASE`

## Decision

The forced pause at 280C:14B5 decodes into the IMAGE.C/IMAGE3.C blit segment near F0683, a transparent horizontally-flipped pixel-line worker reached by F0684/F0132 blits. ReDMCSB shows this is normal render/UI machinery used by DUNVIEW viewport drawing and message-area redraws after input. It is therefore expected render-helper activity, not evidence that the runtime is in a wrong phase. Because pass379 still had no direct F0128/F0097/07FB stop, the sample does not prove the next outer-loop viewport draw; the blocker remains proving F0380/F0365/F0366 dequeue, G0321/tick wait-loop exit, then the next F0128/F0097 hit.

## Evidence

- pass379 forced pause stayed at `280C:14B5` with no F0128/F0097/07FB direct hit.
- pass380 decodes that sample into `IMAGE_TEXT`, nearest `F0683_COPYPIXELLINETOSCREENWITHT`, and explicitly not F0128/F0097.
- FIRES.MAP binds `IMAGE_TEXT` to `IMAGE.C`; ReDMCSB includes `IMAGE3.C` there.
- `F0683` is a transparent horizontally-flipped pixel-line blitter reached from `F0684_Blit` / `F0132_VIDEO_Blit`.
- DUNVIEW and DRAWMSGA both call the same blit machinery, so a forced pause there after input is normal render/UI activity, not a phase escape.

## Consequence

Do not retarget F0128/F0097 from this sample. The next useful runtime pass should arm the pass381 F0380/F0365/F0366 candidates, then prove G0321/tick wait-loop exit before expecting the next F0128/F0097 stop.

Manifest: `parity-evidence/verification/pass382_dm1_v1_image_text_pause_phase/manifest.json`
