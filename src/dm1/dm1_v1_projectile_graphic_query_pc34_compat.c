/*
 * DM1 projectile graphic query — PC 3.4 compatibility layer.
 *
 * ReDMCSB: DUNVIEW.C F0115:5746-5786 and Graphic558 tables.  This small
 * source-locked query unit is deliberately separate from live world rendering
 * so spell/light verification does not pull an unowned dungeon renderer.
 */

#include "dm1_v1_projectile_explosion_render_pc34_compat.h"

const unsigned char DM1_ProjectileScales[7] = {32, 27, 21, 18, 14, 12, 9};

const DM1_ProjectileAspect DM1_ProjectileAspects[DM1_PROJECTILE_ASPECT_COUNT] = {
    { 0, 0, 0x0011}, { 3, 0, 0x0011}, { 6, 0, 0x0010},
    { 9, 0, 0x0112}, {11, 0, 0x0011}, {14, 0, 0x0010},
    {17, 0, 0x0010}, {20, 0, 0x0011}, {23, 0, 0x0011},
    {26, 0, 0x0012}, {28, 0, 0x0103}, {29, 0, 0x0103},
    {30, 0, 0x0103}, {31, 0, 0x0103}
};

int dm1_v1_projectile_aspect_type(int aspectIndex) {
    if (aspectIndex < 0 || aspectIndex >= DM1_PROJECTILE_ASPECT_COUNT) return -1;
    return (int)(DM1_ProjectileAspects[aspectIndex].graphicInfo & 0x0003u);
}

int dm1_v1_projectile_aspect_first_native(int aspectIndex) {
    if (aspectIndex < 0 || aspectIndex >= DM1_PROJECTILE_ASPECT_COUNT) return -1;
    return (int)DM1_ProjectileAspects[aspectIndex].firstNativeBitmapRelativeIndex;
}

unsigned int dm1_v1_projectile_aspect_graphic_info(int aspectIndex) {
    if (aspectIndex < 0 || aspectIndex >= DM1_PROJECTILE_ASPECT_COUNT) return 0u;
    return (unsigned int)DM1_ProjectileAspects[aspectIndex].graphicInfo;
}

int dm1_v1_projectile_bitmap_delta(int aspectIndex, int relativeDir) {
    int aspectType;
    if (aspectIndex < 0 || aspectIndex >= DM1_PROJECTILE_ASPECT_COUNT) return 0;
    aspectType = (int)(DM1_ProjectileAspects[aspectIndex].graphicInfo & 0x0003u);
    if (relativeDir < 0) relativeDir = 0;
    relativeDir &= 3;
    if (aspectType == 3) return 0;
    if (relativeDir == 1 || relativeDir == 3) return aspectType == 2 ? 1 : 2;
    if (aspectType >= 2 || (aspectType == 1 && relativeDir != 0)) return 0;
    return 1;
}

int dm1_v1_projectile_graphic_index(int aspectIndex, int relativeDir) {
    int first;
    if (aspectIndex < 0 || aspectIndex >= DM1_PROJECTILE_ASPECT_COUNT) return -1;
    first = (int)DM1_ProjectileAspects[aspectIndex].firstNativeBitmapRelativeIndex;
    return DM1_GFX_FIRST_PROJECTILE + first +
           dm1_v1_projectile_bitmap_delta(aspectIndex, relativeDir);
}
