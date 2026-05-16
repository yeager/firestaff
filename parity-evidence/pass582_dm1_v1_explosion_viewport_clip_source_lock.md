# Pass582 DM1 V1 Explosion Viewport Clip Source Lock

Status: PASS582_DM1_V1_EXPLOSION_VIEWPORT_CLIP_SOURCE_LOCKED

## Claim

D0C explosions use source zone C004; non-D0C explosion bitmaps are viewport-clipped before blit.

This pass is narrower than D4 row-order work and does not claim pixel parity.

## Primary ReDMCSB Evidence
- DUNVIEW.C:6028-6071 - D0C explosions use C004 explosion-pattern path into G0296_puc_Bitmap_Viewport.
  - if (AP0145_i_ViewSquareExplosionIndex == C12_VIEW_SQUARE_D0C_EXPLOSION)
  - AL0128_puc_Bitmap = F0489_MEMORY_GetNativeBitmapOrGraphic(AL0127_i_ExplosionAspectIndex + M636_GRAPHIC_FIRST_EXPLOSION_PATTERN)
  - F0133_VIDEO_BlitBoxFilledWithMaskedBitmap(AL0128_puc_Bitmap, G0296_puc_Bitmap_Viewport
  - F0638_GetZone(C004_ZONE_EXPLOSION_PATTERN_D0C, L2481_ai_XYZ)
  - G2073_C224_ViewportPixelWidth
  - F0493_CACHE_AddDerivedBitmap(C000_DERIVED_BITMAP_VIEWPORT)
- DUNVIEW.C:6079-6137 - Non-D0C explosions select coordinates and scale before fetching the scaled explosion bitmap.
  - if (L0196_B_RebirthExplosion)
  - G0227_aai_Graphic558_RebirthStep2ExplosionCoordinates
  - if (L0191_ps_Explosion->Centered)
  - G0225_aai_Graphic558_CenteredExplosionCoordinates
  - G0226_aaai_Graphic558_ExplosionCoordinates
  - L0194_i_ExplosionScale
  - F0114_DUNGEONVIEW_GetExplosionBitmap(AL0127_i_ExplosionAspectIndex, L0194_i_ExplosionScale
- DUNVIEW.C:6147-6174 - Explosion bitmap boxes are clipped to the 224x136 viewport and empty clips are skipped.
  - M771_BOX_BOTTOM(L0145_auc_Box) = F0024_MAIN_GetMinimumValue(135
  - AL0127_i_Y = F0025_MAIN_GetMaximumValue(0
  - if (AL0127_i_Y >= 136)
  - M769_BOX_RIGHT(L0145_auc_Box) = AL0127_i_X
  - M768_BOX_LEFT(L0145_auc_Box) = F0026_MAIN_GetBoundedValue(0
  - if (M769_BOX_RIGHT(L0145_auc_Box) <= M768_BOX_LEFT(L0145_auc_Box))
  - L0136_i_ByteWidth = M077_NORMALIZED_BYTE_WIDTH(L0136_i_ByteWidth)
- DUNVIEW.C:6192-6194 - PC34/I34E final explosion draw uses computed zone index and viewport destination.
  - F0791_DUNGEONVIEW_DrawBitmapXX(AL0128_puc_Bitmap, G0296_puc_Bitmap_Viewport, L2474_i_ZoneIndex, L0143_B_FlipHorizontal, C10_COLOR_FLESH
- DEFS.H:3747-3753 - C004 is the source zone id for the D0C explosion pattern.
  - #define C004_ZONE_EXPLOSION_PATTERN_D0C
  - #define C007_ZONE_VIEWPORT
- COORD.C:2389-2410 - MEDIA720 F0635 clips zones and rejects empty clips before callers blit.
  - L2302_i_ = M704_ZONE_LEFT(L2307_ai_XYZ) - M704_ZONE_LEFT(P2130_pi_XYZ)
  - M708_ZONE_WIDTH(P2130_pi_XYZ) = F0024_MAIN_GetMinimumValue
  - M709_ZONE_HEIGHT(P2130_pi_XYZ) = F0024_MAIN_GetMinimumValue
  - if ((M708_ZONE_WIDTH(P2130_pi_XYZ) <= 0) || (M709_ZONE_HEIGHT(P2130_pi_XYZ) <= 0))
  - return NULL

## Firestaff Evidence
- m11_game_view.c D0C explosion pattern helper
  - int M11_GameView_GetV1ExplosionPatternD0CZoneId(void)
  - return 4;
  - int M11_GameView_GetV1ExplosionPatternD0CZone(int* outX
  - if (outX) *outX = 0;
  - if (outY) *outY = 0;
  - if (outW) *outW = 32;
  - if (outH) *outH = 29;
- m11_game_view.c deferred explosion pass
  - static void m11_draw_dm1_deferred_explosion_pass
  - DUNVIEW.C:5915 exits the packed-cell
  - m11_draw_dm1_deferred_center_explosion
  - m11_draw_dm1_deferred_side_explosion
- m11 game-view probe covers C004 helper
  - M11_GameView_GetV1ExplosionPatternD0CZoneId() == 4
  - M11_GameView_GetV1ExplosionPatternD0CZone(&expX, &expY, &expW, &expH)
  - expX == 0 && expY == 0 && expW == 32 && expH == 29
  - "explosion pattern and viewport-centered text zones expose layout-696 C004/C006 geometry"
- CMake registration
  - NAME pass582_dm1_v1_explosion_viewport_clip_source_lock
  - verify_pass582_dm1_v1_explosion_viewport_clip_source_lock.py

## Verification

- /usr/bin/python3 /home/trv2/work/firestaff-worktrees/projectile-explosion-row-clip-pass582-b27520b8/tools/verify_pass582_dm1_v1_explosion_viewport_clip_source_lock.py --check-only -> 0
~~~
status=PASS582_DM1_V1_EXPLOSION_VIEWPORT_CLIP_SOURCE_LOCKED
anchors=DUNVIEW.C:6028-6071,DUNVIEW.C:6079-6137,DUNVIEW.C:6147-6174,DUNVIEW.C:6192-6194,DEFS.H:3747-3753,COORD.C:2389-2410
~~~

## Non-Claims

- Does not duplicate pass579/pass557 D4-before-nearer-wall ordering.
- Does not claim original DOS pixel parity.
- Does not use Greatstone as primary evidence.
- No renderer behavior change in this pass.
