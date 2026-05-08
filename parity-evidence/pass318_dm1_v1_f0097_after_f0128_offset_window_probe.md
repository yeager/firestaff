# Pass318 — DM1 V1 F0097 / VIDRV offset binding

Status: `BLOCKED_F0097_RUNTIME_HIT_STATIC_VIDRV_CANDIDATE_2809_1EFF`

## Source audit

- DRAWVIEW.C `F0097_DUNGEONVIEW_DrawViewport`: lines 709, 850, 857 verified.
- DUNVIEW.C F0128 calls F0097 at line 8610 verified.

## Binding decision

- `2809:1E31` is the F0097 function entry, from FIRES.MAP `20D6:1E31` + loader segment `0733`.
- Better static candidate for `DRAWVIEW.C:857 VIDRV_09_BlitViewPort` is `2809:1EFF` (`26 ff 5f 24`, indirect far call through the video driver table).
- The after-F0128 runtime window probe did not capture a F0097-window stop, so no viewport-present runtime seam is promoted.

Manifest: `parity-evidence/verification/pass318_dm1_v1_f0097_after_f0128_offset_window_probe.json`
