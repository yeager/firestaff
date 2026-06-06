/*
 * DM1 V1 explosion-bitmap lookup helper.
 *
 * Source-locked to ReDMCSB DUNVIEW.C:
 *   F0114_DUNGEONVIEW_GetExplosionBitmap lines 4476-4530
 *   F0115 explosion call route lines 5916-6220 and lookup line 6133
 *   EXPLOSION_ASPECT Graphic558 table lines 1319-1324
 *   STARTUP2.C explosion scale/cache table lines 832-852
 *   DEFS.H M078_SCALED_DIMENSION line 2515
 */

#ifndef FIRESTAFF_DM1_V1_EXPLOSION_BITMAP_VIEWPORT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_EXPLOSION_BITMAP_VIEWPORT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_EXPLOSION_ASPECT_FIRE_PC34   0
#define DM1_V1_EXPLOSION_ASPECT_SPELL_PC34  1
#define DM1_V1_EXPLOSION_ASPECT_POISON_PC34 2
#define DM1_V1_EXPLOSION_ASPECT_SMOKE_PC34  3
#define DM1_V1_EXPLOSION_ASPECT_COUNT_PC34  4
#define DM1_V1_EXPLOSION_SCALE_D1_PC34      32
#define DM1_V1_GRAPHIC_FIRST_EXPLOSION_PC34 486

const unsigned char *
dm1_v1_explosion_bitmap_lookup_pc34(int explosion_aspect_index,
                                    int explosion_scale,
                                    int *byte_width_out,
                                    int *height_out,
                                    int flip_horizontal);

int dm1_v1_explosion_bitmap_derived_index_pc34(int explosion_aspect_index,
                                               int explosion_scale);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_EXPLOSION_BITMAP_VIEWPORT_PC34_COMPAT_H */
