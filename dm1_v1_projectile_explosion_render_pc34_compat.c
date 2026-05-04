/*
 * DM1 V1 Projectile & Explosion Viewport Rendering — pc34 compat layer.
 *
 * All data tables and logic source-locked to ReDMCSB:
 *   DUNVIEW.C F0115 (projectile draw: 5645-5897, explosion draw: 5916-6220)
 *   DUNGEON.C F0142 (GetProjectileAspect)
 *   DEFS.H (PROJECTIL_ASPECT, EXPLOSION_ASPECT, type constants)
 */

#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

/* ── G0215_auc_Graphic558_ProjectileScales ───────────────────────────
 * 7 scale units out of 32.
 * Ref: DUNVIEW.C:5712 (ST source), scaleIndex computation at :5718.
 * [0] = D1 native/back (32), [1] = D2 front (27), [2] = D2 back (21),
 * [3] = D3 front (18), [4] = D3 back (14), [5] = D4 front (12),
 * [6] = D4 back (9). */
const unsigned char DM1_ProjectileScales[7] = {32, 27, 21, 18, 14, 12, 9};

/* ── G0210_as_Graphic558_ProjectileAspects (14 entries) ──────────────
 * Ref: DUNVIEW.C line 78-91 (Graphic558 data section), DEFS.H:2037-2044.
 * Format: {FirstNativeBitmapRelativeIndex, FirstDerivedBitmapRelativeIndex,
 *           GraphicInfo}.
 * GraphicInfo bits:
 *   [1:0] = aspect type (0-3)
 *   [4]   = SIDE flag (MASK0x0010)
 *   [8]   = SCALE_WITH_KINETIC_ENERGY (MASK0x0100)
 *
 * Cross-checked against m11_game_view.c kFirstNative[14] and kGraphicInfo[14]. */
const DM1_ProjectileAspect DM1_ProjectileAspects[DM1_PROJECTILE_ASPECT_COUNT] = {
    { 0, 0, 0x0011}, /*  0: Arrow/dart/shuriken — type0, SIDE */
    { 3, 0, 0x0011}, /*  1: Sword/axe/club      — type0, SIDE */
    { 6, 0, 0x0010}, /*  2: Dagger              — type0, no-SIDE? (SIDE=0x0010 set) */
    { 9, 0, 0x0112}, /*  3: Lightning bolt      — type2, SCALE_KE */
    {11, 0, 0x0011}, /*  4: weapon #1           — type0, SIDE */
    {14, 0, 0x0010}, /*  5: weapon #2           — type0, SIDE=0x0010 */
    {17, 0, 0x0010}, /*  6: weapon #3           — type0 */
    {20, 0, 0x0011}, /*  7: weapon #4           — type0, SIDE */
    {23, 0, 0x0011}, /*  8: weapon #5           — type0, SIDE */
    {26, 0, 0x0012}, /*  9: weapon #6           — type2 */
    {28, 0, 0x0103}, /* 10: Fireball            — type3, SCALE_KE */
    {29, 0, 0x0103}, /* 11: Default spell       — type3, SCALE_KE */
    {30, 0, 0x0103}, /* 12: Slime               — type3, SCALE_KE */
    {31, 0, 0x0103}, /* 13: Poison bolt/cloud   — type3, SCALE_KE */
};

/* ── G0216_auc_Graphic558_ExplosionBaseScales ────────────────────────
 * 4 values indexed by view depth (0=D0 closest, 3=D3 farthest).
 * Ref: DUNVIEW.C:6109 (the actual table values from ReDMCSB data section).
 * Used at DUNVIEW.C:6109: L0194_i_ExplosionScale =
 *   max(4, (max(48, attack+1) * G0216[depth]) >> 8) & 0xFFFE */
const unsigned char DM1_ExplosionBaseScales[4] = {32, 21, 14, 9};


/* ── Projectile rendering queries ────────────────────────────────── */

int dm1_v1_projectile_aspect_type(int aspectIndex) {
    if (aspectIndex < 0 || aspectIndex >= DM1_PROJECTILE_ASPECT_COUNT) return -1;
    return (int)(DM1_ProjectileAspects[aspectIndex].graphicInfo & 0x0003u);
}

/* DUNVIEW.C:5746-5786, L0183_i_ProjectileBitmapIndexDelta. */
int dm1_v1_projectile_bitmap_delta(int aspectIndex, int relativeDir) {
    int aspectType;
    if (aspectIndex < 0 || aspectIndex >= DM1_PROJECTILE_ASPECT_COUNT) return 0;
    aspectType = (int)(DM1_ProjectileAspects[aspectIndex].graphicInfo & 0x0003u);
    if (relativeDir < 0) relativeDir = 0;
    relativeDir &= 3;

    /* Type 3 (NO_BACK_NO_ROTATION): always delta=0 */
    if (aspectType == 3) return 0;

    /* Perpendicular to party facing (right or left) */
    if (relativeDir == 1 || relativeDir == 3) {
        /* Type 2 (NO_BACK_AND_ROTATION): delta=1 */
        if (aspectType == 2) return 1;
        /* Type 0/1 (HAS_BACK): delta=2 */
        return 2;
    }

    /* Parallel (forward=0 or backward=2) */
    if (aspectType >= 2) return 0;
    /* Type 1 (HAS_BACK_NO_ROTATION): delta=0 unless facing backward */
    if (aspectType == 1 && relativeDir != 0) return 0;
    /* Type 0/1 facing away: delta=1 (back graphic) if applicable */
    return 1;
}

int dm1_v1_projectile_graphic_index(int aspectIndex, int relativeDir) {
    int first;
    if (aspectIndex < 0 || aspectIndex >= DM1_PROJECTILE_ASPECT_COUNT) return -1;
    first = (int)DM1_ProjectileAspects[aspectIndex].firstNativeBitmapRelativeIndex;
    return DM1_GFX_FIRST_PROJECTILE + first +
           dm1_v1_projectile_bitmap_delta(aspectIndex, relativeDir);
}

/* DUNVIEW.C:5745-5806 flip flags.
 * bit0 = horizontal, bit1 = vertical. */
int dm1_v1_projectile_flip_flags(int aspectIndex, int relativeDir,
                                 int relativeCell, int mapX, int mapY) {
    unsigned int info;
    int aspectType;
    int flags = 0;
    if (aspectIndex < 0 || aspectIndex >= DM1_PROJECTILE_ASPECT_COUNT) return 0;
    info = (unsigned int)DM1_ProjectileAspects[aspectIndex].graphicInfo;
    aspectType = (int)(info & 0x0003u);
    if (relativeDir < 0) relativeDir = 0;
    relativeDir &= 3;

    /* Type 3 (no back, no rotation): no flipping ever. */
    if (aspectType == 3) return 0;

    /* Type 0 (has back + rotation) */
    if (aspectType == 0) {
        int parityVertical = ((mapX + mapY) & 1) ? 1 : 0;
        if (relativeDir == 1 || relativeDir == 3) {
            /* Perpendicular: flip based on sub-cell and parity. */
            if (relativeCell == 0 || relativeCell == 2) flags |= 0x01;
            if (parityVertical) flags |= 0x02;
            else flags ^= 0x01;
        } else {
            /* Parallel: vertical flip based on parity + back row. */
            if (parityVertical && relativeCell < 2) flags |= 0x02;
        }
    } else if (relativeDir == 1) {
        /* Type 1/2: horizontal flip when flying right. */
        flags |= 0x01;
    }

    /* SIDE flag: horizontal flip when flying left. */
    if ((info & 0x0010u) && relativeDir == 3) {
        flags |= 0x01;
    }

    return flags;
}

int dm1_v1_projectile_scale_units(int depthIndex, int relativeCell) {
    int frontRow = (relativeCell < 0) ? 1 : (relativeCell >= 2);
    int idx;
    if (depthIndex <= 0) return DM1_ProjectileScales[0];
    idx = depthIndex * 2 - (frontRow ? 1 : 0);
    if (idx < 1) idx = 1;
    if (idx > 6) idx = 6;
    return DM1_ProjectileScales[idx];
}

/* F0142_DUNGEON_GetProjectileAspect — subtype to aspect index mapping. */
int dm1_v1_projectile_subtype_to_aspect(int subtype) {
    switch (subtype) {
        case PROJECTILE_SUBTYPE_FIREBALL:           return DM1_PROJ_ASPECT_FIREBALL;
        case PROJECTILE_SUBTYPE_SLIME:              return DM1_PROJ_ASPECT_SLIME;
        case PROJECTILE_SUBTYPE_LIGHTNING_BOLT:     return DM1_PROJ_ASPECT_LIGHTNING_BOLT;
        case PROJECTILE_SUBTYPE_POISON_BOLT:
        case PROJECTILE_SUBTYPE_POISON_CLOUD:       return DM1_PROJ_ASPECT_POISON;
        case PROJECTILE_SUBTYPE_HARM_NON_MATERIAL:
        case PROJECTILE_SUBTYPE_OPEN_DOOR:          return DM1_PROJ_ASPECT_DEFAULT;
        case PROJECTILE_SUBTYPE_KINETIC_ARROW:
        default:                                    return 0;
    }
}


/* ── Explosion rendering queries ─────────────────────────────────── */

/* DUNVIEW.C:5958-5994 type->aspect branching. */
int dm1_v1_explosion_type_to_aspect(int explosionType) {
    if (explosionType < 0) return -1;
    if (explosionType == DM1_EXPLOSION_FLUXCAGE) return -1;
    if (explosionType == DM1_EXPLOSION_REBIRTH_STEP1) return -1;
    if (explosionType == DM1_EXPLOSION_FIREBALL ||
        explosionType == DM1_EXPLOSION_LIGHTNING_BOLT ||
        explosionType == DM1_EXPLOSION_REBIRTH_STEP2) {
        return DM1_EXPLOSION_ASPECT_FIRE;
    }
    if (explosionType == DM1_EXPLOSION_POISON_BOLT ||
        explosionType == DM1_EXPLOSION_POISON_CLOUD) {
        return DM1_EXPLOSION_ASPECT_POISON;
    }
    if (explosionType == DM1_EXPLOSION_SMOKE) {
        return DM1_EXPLOSION_ASPECT_SMOKE;
    }
    return DM1_EXPLOSION_ASPECT_SPELL;
}

/* M614_GRAPHIC_FIRST_EXPLOSION + min(aspect, C2_POISON). */
int dm1_v1_explosion_aspect_to_graphic(int aspect) {
    if (aspect < 0) return -1;
    if (aspect >= 2) return DM1_GFX_FIRST_EXPLOSION + 2;
    return DM1_GFX_FIRST_EXPLOSION + aspect;
}

/* DUNVIEW.C:6040-6044: 3 graphics per pattern. */
int dm1_v1_explosion_size_class(int attack) {
    int shifted = attack >> 5;
    if (shifted == 0) return 0;  /* small */
    if (shifted <= 3) return 1;  /* medium */
    return 2;                    /* large */
}

int dm1_v1_explosion_pattern_graphic_index(int explosionType, int attack) {
    int aspect = dm1_v1_explosion_type_to_aspect(explosionType);
    int sizeClass;
    if (aspect < 0) return -1;
    /* Smoke uses poison graphics (aspect 2) with palette remap. */
    if (aspect == DM1_EXPLOSION_ASPECT_SMOKE) aspect = DM1_EXPLOSION_ASPECT_POISON;
    sizeClass = dm1_v1_explosion_size_class(attack);
    return DM1_GFX_FIRST_EXPLOSION_PATTERN + aspect * 3 + sizeClass;
}

int dm1_v1_explosion_base_scale(int viewDepth) {
    if (viewDepth < 0) viewDepth = 0;
    if (viewDepth > 3) viewDepth = 3;
    return (int)DM1_ExplosionBaseScales[viewDepth];
}

int dm1_v1_explosion_is_smoke(int explosionType) {
    return explosionType == DM1_EXPLOSION_SMOKE ? 1 : 0;
}


/* ── Draw order verification ─────────────────────────────────────── */

int dm1_v1_verify_f0115_draw_order(const int* order, int count) {
    int i;
    if (!order || count < 2) return 1;
    for (i = 1; i < count; ++i) {
        if (order[i] <= order[i - 1]) return 0;
    }
    return 1;
}
