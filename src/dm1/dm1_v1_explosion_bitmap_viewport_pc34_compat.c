#include "dm1_v1_explosion_bitmap_viewport_pc34_compat.h"

#include <stddef.h>

typedef struct DM1V1ExplosionAspectPc34 {
    unsigned char byte_width;
    unsigned char height;
} DM1V1ExplosionAspectPc34;

/* ReDMCSB DUNVIEW.C:1319-1324, G0211_as_Graphic558_ExplosionAspects. */
static const DM1V1ExplosionAspectPc34 s_explosion_aspects[DM1_V1_EXPLOSION_ASPECT_COUNT_PC34] = {
    {80, 111},
    {64,  97},
    {80,  91},
    {80,  91}
};

/* Stable marker storage for the pure lookup helper.  F0114:4517-4526 returns
 * either a native GRAPHICS.DAT bitmap or a cached derived bitmap; this helper
 * mirrors that selection without owning asset memory. */
static const unsigned char s_native_bitmap_markers[3][2] = {
    {0xF0u, 0xF1u},
    {0xF2u, 0xF3u},
    {0xF4u, 0xF5u}
};
static const unsigned char s_derived_bitmap_markers[DM1_V1_EXPLOSION_ASPECT_COUNT_PC34][14][2] = {{{0}}};

static int scaled_dimension(int dimension, int scale)
{
    return ((dimension * scale) + (scale >> 1)) >> 5;
}

static int bounded_scale(int explosion_scale)
{
    if (explosion_scale > DM1_V1_EXPLOSION_SCALE_D1_PC34) {
        return DM1_V1_EXPLOSION_SCALE_D1_PC34;
    }
    return explosion_scale;
}

int dm1_v1_explosion_bitmap_derived_index_pc34(int explosion_aspect_index,
                                               int explosion_scale)
{
    int scale = bounded_scale(explosion_scale);
    if (explosion_aspect_index < 0 ||
        explosion_aspect_index >= DM1_V1_EXPLOSION_ASPECT_COUNT_PC34 ||
        scale < 4) {
        return -1;
    }

    /* ReDMCSB F0114 DUNVIEW.C:4518:
     * (aspect * 14) + (scale >> 1) + (M538_DERIVED_BITMAP_FIRST_EXPLOSION - 2).
     * This normalized index omits M538 so tests can gate the source table
     * offset independent of the global cache base. */
    return (explosion_aspect_index * 14) + (scale >> 1) - 2;
}

const unsigned char *
dm1_v1_explosion_bitmap_lookup_pc34(int explosion_aspect_index,
                                    int explosion_scale,
                                    int *byte_width_out,
                                    int *height_out,
                                    int flip_horizontal)
{
    int scale = bounded_scale(explosion_scale);
    const DM1V1ExplosionAspectPc34 *aspect;
    int native_index;
    int derived_index;

    if (byte_width_out) {
        *byte_width_out = 0;
    }
    if (height_out) {
        *height_out = 0;
    }

    if (explosion_aspect_index < 0 ||
        explosion_aspect_index >= DM1_V1_EXPLOSION_ASPECT_COUNT_PC34 ||
        scale < 4) {
        return NULL;
    }

    aspect = &s_explosion_aspects[explosion_aspect_index];
    if (byte_width_out) {
        *byte_width_out = scaled_dimension((int)aspect->byte_width, scale);
    }
    if (height_out) {
        *height_out = scaled_dimension((int)aspect->height, scale);
    }

    /* ReDMCSB F0114 DUNVIEW.C:4514-4516: native bitmap only at full scale,
     * except smoke, which always derives from poison with palette changes. */
    if (scale == DM1_V1_EXPLOSION_SCALE_D1_PC34 &&
        explosion_aspect_index != DM1_V1_EXPLOSION_ASPECT_SMOKE_PC34) {
        native_index = explosion_aspect_index;
        if (native_index > DM1_V1_EXPLOSION_ASPECT_POISON_PC34) {
            native_index = DM1_V1_EXPLOSION_ASPECT_POISON_PC34;
        }
        return &s_native_bitmap_markers[native_index][flip_horizontal ? 1 : 0];
    }

    derived_index = dm1_v1_explosion_bitmap_derived_index_pc34(explosion_aspect_index, scale);
    if (derived_index < 0 || derived_index >= 56) {
        return NULL;
    }
    return &s_derived_bitmap_markers[explosion_aspect_index][scale >> 1][flip_horizontal ? 1 : 0];
}
