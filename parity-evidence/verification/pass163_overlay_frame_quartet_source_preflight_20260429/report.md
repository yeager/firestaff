# Pass 163 source preflight — overlay frame quartet timing/geometry invariant

Scope: narrow source-first preflight for Lane B. This pass **does not** claim original/Firestaff pixel parity, because the original party/control route remains blocked by the known static no-party signature.

## Source anchors audited first

- `TITLE.C:F0437_STARTEND_DrawTitle` lines 201-251: title zoom is gated by `M526_WaitVerticalBlank()` per step, Amiga path has `Delay(25L)`, then BUG0_71 records that timing can be too short on fast computers before entrance replaces title.
- `ENTRANCE.C:F0438_STARTEND_OpenEntranceDoors` lines 147-239: each door step composites `G0296_puc_Bitmap_Viewport` under door graphics, advances left/right door boxes by 4 px, then waits on `M526_WaitVerticalBlank()`; BUG0_71 names the too-fast entrance door timing risk.
- `ENTRANCE.C:F0441_STARTEND_ProcessEntrance` lines 850-943: entrance waits in `C099_MODE_WAITING_ON_ENTRANCE` with `M526_WaitVerticalBlank()` + command queue processing; only after `G0298_B_NewGame` changes does it call `F0438_STARTEND_OpenEntranceDoors()`.
- `DRAWVIEW.C:F0097_DUNGEONVIEW_DrawViewport` lines 709-723: gameplay viewport display is requested via `G0324_B_DrawViewportRequested = C1_TRUE` and waits one vertical blank so `G0296_puc_Bitmap_Viewport` is on screen when the function returns.
- `VIEWPORT.C:F0564_VIEWPORT_Initialize/F0566_VIEWPORT_BlitToScreen` lines 20-96: source viewport bitplanes are 224x136 and destination starts at screen line 33; blitter copies 224/16 words by 136 lines with `(320-224)` destination modulo.
- `VIDEODRV.C:F8153_VIDRV_07_WaitVerticalBlank` lines 3163-3185: PC wait loops around VGA status bit 3, establishing vertical-sync gating.
- `VIDEODRV.C:F8161_VIDRV_09_BlitViewPort` lines 3566-3580: PC VGA viewport blit temporarily sets `G8177_c_ViewportColorIndexOffset = 0x10`, blits 224-wide viewport into the C007 box, then resets the offset.

## Verification artifact

- `source_overlay_timing_invariant.py` verifies the source anchors above plus Firestaff/capture preconditions:
  - `m11_game_view.c` has `M11_DM1_VIEWPORT_X/Y/W/H = 0/33/224/136`.
  - `scripts/dosbox_dm1_original_viewport_reference_capture.sh` crops `224x136+0+33`.
  - Existing original-route classifier evidence still records `48ed3743ab6a` and duplicate raw frames, so quartet acceptance remains blocked.
- Output: `source_overlay_timing_invariant.json`.

## Invariant for later pixel gates

Any future accepted `gameplay_viewport`, `spell_panel`, or `inventory_panel` quartet member must use the ReDMCSB source viewport rectangle `(x=0,y=33,w=224,h=136)`, and timing-sensitive title/entrance/gameplay transitions must be captured after source-equivalent vblank/timer gates rather than arbitrary emulator delay guesses.

## Gate

```sh
python3 parity-evidence/verification/pass163_overlay_frame_quartet_source_preflight_20260429/source_overlay_timing_invariant.py
```

Result: PASS (`source_overlay_timing_invariant.json` has `"pass": true`).

## Next blocker

Lane A remains the exact blocker: get original DOS DM1 into real party/control gameplay and reject static no-party hash `48ed3743ab6a`. Until then, the overlay frame quartet can only be preflighted; it cannot make final pixel parity claims.
