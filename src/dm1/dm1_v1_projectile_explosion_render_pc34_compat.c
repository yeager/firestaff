/*
 * DM1 V1 Projectile & Explosion Viewport Rendering — pc34 compat layer.
 *
 * All data tables and logic source-locked to ReDMCSB:
 *   DUNVIEW.C F0115 (projectile draw: 5645-5897, explosion draw: 5916-6220)
 *   DUNGEON.C F0142 (GetProjectileAspect)
 *   DEFS.H (PROJECTIL_ASPECT, EXPLOSION_ASPECT, type constants)
 */

#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#include <string.h>

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

int dm1_v1_projectile_aspect_first_native(int aspectIndex) {
    if (aspectIndex < 0 || aspectIndex >= DM1_PROJECTILE_ASPECT_COUNT) return -1;
    return (int)DM1_ProjectileAspects[aspectIndex].firstNativeBitmapRelativeIndex;
}

unsigned int dm1_v1_projectile_aspect_graphic_info(int aspectIndex) {
    if (aspectIndex < 0 || aspectIndex >= DM1_PROJECTILE_ASPECT_COUNT) return 0u;
    return (unsigned int)DM1_ProjectileAspects[aspectIndex].graphicInfo;
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

int dm1_v1_projectile_subtype_graphic_index(int subtype) {
    int aspectIndex = dm1_v1_projectile_subtype_to_aspect(subtype);
    int first = dm1_v1_projectile_aspect_first_native(aspectIndex);
    if (first < 0) return -1;
    return DM1_GFX_FIRST_PROJECTILE + first;
}

static int dm1_v1_f0142_object_subtype_is_valid_pc34(
    int associatedThingType,
    int associatedThingSubtype)
{
    if (associatedThingSubtype < 0) {
        return 0;
    }
    switch (associatedThingType) {
        case THING_TYPE_WEAPON: return associatedThingSubtype <= 45;
        case THING_TYPE_ARMOUR: return associatedThingSubtype <= 57;
        case THING_TYPE_SCROLL: return associatedThingSubtype == 0;
        case THING_TYPE_POTION: return associatedThingSubtype <= 20;
        case THING_TYPE_CONTAINER: return associatedThingSubtype == 0;
        case THING_TYPE_JUNK: return associatedThingSubtype <= 52;
        default: return 0;
    }
}

int dm1_v1_f0142_get_projectile_aspect_pc34(
    int projectileSubtype,
    int associatedThingType,
    int associatedThingSubtype,
    int weaponProjectileAspectOrdinal)
{
    int aspect;

    if (associatedThingType >= THING_TYPE_WEAPON &&
        associatedThingType <= THING_TYPE_JUNK) {
        /* F0142 reads the authenticated C05..C0B record type directly.
         * The generic inventory aspect helper intentionally normalizes UI
         * slot values; a malformed projectile Slot must instead fail closed
         * before it can borrow subtype-zero object material. */
        if (!dm1_v1_f0142_object_subtype_is_valid_pc34(
                associatedThingType, associatedThingSubtype)) {
            return DM1_F0142_INVALID_PROJECTILE_ASPECT_PC34;
        }
        if (associatedThingType == THING_TYPE_WEAPON &&
            weaponProjectileAspectOrdinal > 0) {
            if (weaponProjectileAspectOrdinal > DM1_PROJECTILE_ASPECT_COUNT) {
                return DM1_F0142_INVALID_PROJECTILE_ASPECT_PC34;
            }
            return -weaponProjectileAspectOrdinal;
        }

        aspect = dm1_item_aspect_index(associatedThingType,
                                       associatedThingSubtype);
        if (aspect < 0) {
            return DM1_F0142_INVALID_PROJECTILE_ASPECT_PC34;
        }
        return aspect;
    }

    aspect = dm1_v1_projectile_subtype_to_aspect(projectileSubtype);
    if (aspect < 0 || aspect >= DM1_PROJECTILE_ASPECT_COUNT) {
        return DM1_F0142_INVALID_PROJECTILE_ASPECT_PC34;
    }
    return -(aspect + 1);
}

int dm1_v1_c100_rebirth_lightning_graphic_index_pc34(void)
{
    /* ReDMCSB DUNVIEW.C F0115:5965-5969,5977-5979 resolves the original
     * lightning projectile aspect, then requests its following native bitmap:
     * M613 (454) + G0210[C03].FirstNativeBitmapRelativeIndex (9) + 1.
     * C100 remains distinct from C101's M636 fire-pattern route. */
    return DM1_GFX_FIRST_PROJECTILE +
           (int)DM1_ProjectileAspects[DM1_PROJ_ASPECT_LIGHTNING_BOLT]
               .firstNativeBitmapRelativeIndex + 1;
}

int dm1_v1_projectile_material_resolve_pc34(
    int projectileSubtype,
    int associatedThingType,
    int associatedThingSubtype,
    int weaponProjectileAspectOrdinal,
    DM1_ProjectileMaterialResolutionPc34 *outResolution)
{
    DM1_ProjectileMaterialResolutionPc34 resolution;
    int aspect;

    if (!outResolution) {
        return 0;
    }
    memset(&resolution, 0, sizeof(resolution));
    resolution.graphic_index = -1;
    resolution.aspect_index = -1;
    resolution.transparent_color = 10;

    /* ReDMCSB DUNGEON.C F0142: weapons with M066's nonzero ordinal use
     * G0210 projectile art; every other carried object uses G0237/G0209.
     * DUNVIEW.C F0115:5896-5900 then enters the object draw path. */
    aspect = dm1_v1_f0142_get_projectile_aspect_pc34(
        projectileSubtype, associatedThingType, associatedThingSubtype,
        weaponProjectileAspectOrdinal);
    if (aspect == DM1_F0142_INVALID_PROJECTILE_ASPECT_PC34) {
        return 0;
    }

    if (aspect < 0) {
        aspect = -aspect - 1;
        resolution.graphic_index = dm1_v1_projectile_graphic_index(aspect, 0);
        resolution.aspect_index = aspect;
    } else {
        resolution.uses_object_aspect = 1;
        resolution.graphic_index = (int)dm1_object_aspect_graphic_index(aspect);
        resolution.aspect_index = aspect;
        if (resolution.graphic_index <= 0) {
            return 0;
        }
    }

    resolution.valid = 1;
    *outResolution = resolution;
    return 1;
}

int dm1_v1_thrown_object_projectile_blit_plan_pc34(
    DM1_ThrownObjectProjectileBlitPlanPc34 *outPlan,
    int graphicIndex,
    int objectAspectIndex,
    int depthIndex,
    int relativeCell,
    int viewLane,
    int sourceZoneRow,
    int viewportX,
    int viewportY,
    int viewportW,
    int viewportH,
    int spriteW,
    int spriteH)
{
    DM1_ThrownObjectProjectileBlitPlanPc34 plan;
    int zoneX;
    int zoneY;

    if (!outPlan || graphicIndex < DM1_GRAPHIC_FIRST_OBJECT ||
        objectAspectIndex < 0 || objectAspectIndex >= 85 ||
        relativeCell < 0 || relativeCell > 3 || sourceZoneRow < 0 ||
        viewLane < -1 || viewLane > 1 ||
        viewportW <= 0 || viewportH <= 0 || spriteW <= 0 || spriteH <= 0 ||
        !dm1_viewport_3d_c2900_projectile_raw_zone_point(
            sourceZoneRow, relativeCell, &zoneX, &zoneY)) {
        return 0;
    }

    memset(&plan, 0, sizeof(plan));
    plan.graphic_index = graphicIndex;
    plan.transparent_color = 10;
    plan.source_scale_index = dm1_viewport_3d_object_source_scale_index(
        depthIndex, relativeCell);
    plan.scale_units = dm1_viewport_3d_object_source_scale_units(
        plan.source_scale_index);
    plan.draw_w = spriteW * plan.scale_units / 32;
    plan.draw_h = spriteH * plan.scale_units / 32;
    if (plan.draw_w < 3) plan.draw_w = 3;
    if (plan.draw_h < 3) plan.draw_h = 3;
    plan.draw_x = viewportX + zoneX - plan.draw_w / 2;
    plan.draw_y = viewportY + zoneY - plan.draw_h + 1;
    /* ReDMCSB DUNVIEW.C F0115:4862-4865 applies G0209's
     * FLIP_ON_RIGHT flag to every right-side lane.  In the center lane it
     * applies only to FRONT_RIGHT (C01) and BACK_RIGHT (C02), never
     * BACK_LEFT (C03).  A thrown object re-enters this same object path at
     * :5891-5900, so its C2900 material must use the identical predicate. */
    plan.use_mirror =
        (dm1_object_aspect_graphic_info(objectAspectIndex) & 0x0001u) &&
        (viewLane > 0 ||
         (viewLane == 0 && (relativeCell == 1 || relativeCell == 2)));
    plan.uses_source_row = 1;

    if (plan.draw_x < viewportX - plan.draw_w + 1) {
        plan.draw_x = viewportX - plan.draw_w + 1;
    }
    if (plan.draw_y < viewportY - plan.draw_h + 1) {
        plan.draw_y = viewportY - plan.draw_h + 1;
    }
    if (plan.draw_x >= viewportX + viewportW ||
        plan.draw_y >= viewportY + viewportH) {
        return 0;
    }

    /* ReDMCSB DUNVIEW.C F0115:5896-5900 sets the projectile C2900
     * coordinate then jumps to T0115015_DrawProjectileAsObject.  The
     * object keeps G0209/M612 material and C10 transparency, but skips the
     * normal floor-pile shift. */
    *outPlan = plan;
    return 1;
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
int F0142_DUNGEON_GetProjectileAspect(int subtype) {
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

int dm1_v1_projectile_subtype_to_aspect(int subtype) {
    return F0142_DUNGEON_GetProjectileAspect(subtype);
}

int dm1_v1_projectile_renderable_pc34(int projectileCount,
                                      int graphicIndex)
{
    return projectileCount > 0 && graphicIndex >= 0;
}

int dm1_v1_projectile_effect_particle_pc34(int subtype,
                                           uint32_t *outColor,
                                           float *outSize)
{
    uint32_t color = 0xffaa00ffu;
    float size = 2.0f;
    if (subtype == PROJECTILE_SUBTYPE_LIGHTNING_BOLT) {
        color = 0x00ffffffu;
        size = 1.0f;
    } else if (subtype == PROJECTILE_SUBTYPE_POISON_BOLT ||
               subtype == PROJECTILE_SUBTYPE_POISON_CLOUD) {
        color = 0x00ff00ffu;
        size = 2.0f;
    }
    if (outColor) {
        *outColor = color;
    }
    if (outSize) {
        *outSize = size;
    }
    return 1;
}

int dm1_v1_projectile_d4_far_box(int relSide,
                                 int *outX,
                                 int *outY,
                                 int *outW,
                                 int *outH) {
    int x;
    if (relSide < -1 || relSide > 1) {
        return 0;
    }

    /* ReDMCSB DUNVIEW.C F0128 lines 8466-8477 calls F0115 for D4L,
     * D4R, then D4C before D3 walls overpaint it. PC34 does not expose
     * a C2900 source-zone row for D4, so Firestaff binds a tiny far box
     * behind D3 while preserving that draw order and occlusion model. */
    x = (relSide < 0) ? 78 : ((relSide > 0) ? 138 : 108);
    if (outX) *outX = x;
    if (outY) *outY = 42;
    if (outW) *outW = 10;
    if (outH) *outH = 8;
    return 1;
}

int dm1_v1_projectile_sprite_blit_plan(DM1_ProjectileSpriteBlitPlan *out_plan,
                                       int graphicIndex,
                                       int depthIndex,
                                       int relativeCell,
                                       int flipFlags,
                                       int sourceZoneRow,
                                       int viewportX,
                                       int viewportY,
                                       int viewportW,
                                       int viewportH,
                                       int paneX,
                                       int paneY,
                                       int paneW,
                                       int paneH,
                                       int spriteW,
                                       int spriteH)
{
    DM1_ProjectileSpriteBlitPlan plan;
    int zoneX = 0;
    int zoneY = 0;

    if (!out_plan || graphicIndex < DM1_GFX_FIRST_PROJECTILE ||
        graphicIndex >= DM1_GFX_FIRST_EXPLOSION ||
        paneW <= 0 || paneH <= 0 || spriteW <= 0 || spriteH <= 0 ||
        viewportW <= 0 || viewportH <= 0) {
        return 0;
    }

    memset(&plan, 0, sizeof(plan));
    plan.graphic_index = graphicIndex;
    plan.transparent_color = 10;
    plan.flip_flags = flipFlags & 0x03;

    /* ReDMCSB DUNVIEW.C F0115 lines 5635-5897 restores the projectile
     * view cell, resolves C2900 through the current view square/cell,
     * applies G0215 projectile scaling, and blits with transparent C10. */
    plan.scale_units = dm1_v1_projectile_scale_units(depthIndex, relativeCell);
    plan.draw_w = spriteW * plan.scale_units / 32;
    plan.draw_h = spriteH * plan.scale_units / 32;
    if (plan.draw_w < 3) plan.draw_w = 3;
    if (plan.draw_h < 3) plan.draw_h = 3;
    if (plan.draw_w > paneW) plan.draw_w = paneW;
    if (plan.draw_h > paneH) plan.draw_h = paneH;

    if (relativeCell >= 0 && relativeCell <= 3) {
        plan.source_scale_index =
            dm1_viewport_3d_object_source_scale_index(depthIndex,
                                                      relativeCell);
        if (paneX >= viewportX && paneY >= viewportY &&
            ((sourceZoneRow >= 0 &&
              dm1_viewport_3d_c2900_projectile_raw_zone_point(sourceZoneRow,
                                                              relativeCell,
                                                              &zoneX,
                                                              &zoneY)) ||
             (sourceZoneRow < 0 &&
              dm1_viewport_3d_c2900_projectile_zone_point(plan.source_scale_index,
                                                          relativeCell,
                                                          &zoneX,
                                                          &zoneY)))) {
            plan.uses_source_row = sourceZoneRow >= 0 ? 1 : 0;
            plan.draw_x = viewportX + zoneX - plan.draw_w / 2;
            plan.draw_y = viewportY + zoneY - plan.draw_h / 2;
        } else {
            int qx = paneW / 4;
            int qy = paneH / 4;
            if (depthIndex >= 1) {
                qx = qx * 2 / 3;
                qy = qy * 2 / 3;
            }
            if (depthIndex >= 2) {
                qx /= 2;
                qy /= 2;
            }
            switch (relativeCell) {
                case 0:
                    plan.draw_x = paneX + (paneW / 2 - qx) - plan.draw_w / 2;
                    plan.draw_y = paneY + (paneH / 2 - qy) - plan.draw_h / 2;
                    break;
                case 1:
                    plan.draw_x = paneX + (paneW / 2 + qx) - plan.draw_w / 2;
                    plan.draw_y = paneY + (paneH / 2 - qy) - plan.draw_h / 2;
                    break;
                case 2:
                    plan.draw_x = paneX + (paneW / 2 - qx) - plan.draw_w / 2;
                    plan.draw_y = paneY + (paneH / 2 + qy) - plan.draw_h / 2;
                    break;
                default:
                    plan.draw_x = paneX + (paneW / 2 + qx) - plan.draw_w / 2;
                    plan.draw_y = paneY + (paneH / 2 + qy) - plan.draw_h / 2;
                    break;
            }
        }

        if (sourceZoneRow >= 0) {
            int minX = viewportX - plan.draw_w + 1;
            int minY = viewportY - plan.draw_h + 1;
            int maxX = viewportX + viewportW - 1;
            int maxY = viewportY + viewportH - 1;
            if (plan.draw_x < minX) plan.draw_x = minX;
            if (plan.draw_y < minY) plan.draw_y = minY;
            if (plan.draw_x > maxX) plan.draw_x = maxX;
            if (plan.draw_y > maxY) plan.draw_y = maxY;
        } else {
            if (plan.draw_x < paneX) plan.draw_x = paneX;
            if (plan.draw_y < paneY) plan.draw_y = paneY;
            if (plan.draw_x + plan.draw_w > paneX + paneW) {
                plan.draw_x = paneX + paneW - plan.draw_w;
            }
            if (plan.draw_y + plan.draw_h > paneY + paneH) {
                plan.draw_y = paneY + paneH - plan.draw_h;
            }
        }
    } else {
        plan.source_scale_index = -1;
        plan.draw_x = paneX + (paneW - plan.draw_w) / 2;
        plan.draw_y = paneY + (paneH - plan.draw_h) / 2;
    }

    *out_plan = plan;
    return 1;
}


/* ── Explosion rendering queries ─────────────────────────────────── */

/* DUNVIEW.C:5958-5994 type->aspect branching. */
int dm1_v1_explosion_type_to_aspect(int explosionType) {
    if (explosionType < 0) return -1;
    if (explosionType == DM1_EXPLOSION_FLUXCAGE) return -1;
    if (explosionType == DM1_EXPLOSION_TYPE_REBIRTH_STEP1) return -1;
    if (explosionType == DM1_EXPLOSION_FIREBALL ||
        explosionType == DM1_EXPLOSION_LIGHTNING_BOLT ||
        explosionType == DM1_EXPLOSION_TYPE_REBIRTH_STEP2) {
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

int dm1_v1_explosion_sprite_blit_plan(DM1_ExplosionSpriteBlitPlan *out_plan,
                                      int aspect,
                                      int graphicIndex,
                                      int isSmoke,
                                      int frame,
                                      int maxFrames,
                                      int attack,
                                      int depthIndex,
                                      int paneX,
                                      int paneY,
                                      int paneW,
                                      int paneH,
                                      int spriteW,
                                      int spriteH)
{
    DM1_ExplosionSpriteBlitPlan plan;

    if (!out_plan || aspect < 0 || graphicIndex < 0 ||
        paneW <= 0 || paneH <= 0 || spriteW <= 0 || spriteH <= 0) {
        return 0;
    }

    memset(&plan, 0, sizeof(plan));
    plan.aspect_index = aspect;
    plan.graphic_index = graphicIndex;
    plan.is_smoke = isSmoke ? 1 : 0;
    plan.transparent_color = 0;
    plan.replace_src_a = DM1_SMOKE_RECOLOR_SRC_A;
    plan.replace_dst_a = DM1_SMOKE_RECOLOR_DST_A;
    plan.replace_src_b = DM1_SMOKE_RECOLOR_SRC_B;
    plan.replace_dst_b = DM1_SMOKE_RECOLOR_DST_B;

    /* This preserves Firestaff's current bounded explosion bitmap sizing
     * while moving the decision out of M11.  ReDMCSB source anchors are
     * DUNVIEW.C F0115/F0136/F0675 and G0212 smoke palette remapping. */
    plan.base_scale_percent = (depthIndex == 0) ? 100
                            : (depthIndex == 1) ? 70
                            : 45;
    plan.scale_percent = plan.base_scale_percent;
    if (aspect == DM1_EXPLOSION_ASPECT_FIRE ||
        aspect == DM1_EXPLOSION_ASPECT_SPELL) {
        if (frame >= 0 && maxFrames > 0) {
            if (frame == 0) {
                plan.scale_percent = (plan.base_scale_percent * 110) / 100;
            } else if (frame == 1) {
                plan.scale_percent = plan.base_scale_percent;
            } else {
                plan.scale_percent = (plan.base_scale_percent * 65) / 100;
            }
        }
    } else if (aspect == DM1_EXPLOSION_ASPECT_POISON ||
               aspect == DM1_EXPLOSION_ASPECT_SMOKE) {
        if (attack >= 0) {
            int a = attack > 200 ? 200 : attack;
            int pct = 60 + (a * 55) / 200;
            if (pct < 60) pct = 60;
            if (pct > 115) pct = 115;
            plan.scale_percent = (plan.base_scale_percent * pct) / 100;
        }
    }

    plan.draw_w = spriteW * plan.scale_percent / 100;
    plan.draw_h = spriteH * plan.scale_percent / 100;
    if (plan.draw_w < 4) plan.draw_w = 4;
    if (plan.draw_h < 4) plan.draw_h = 4;
    if (plan.draw_w > paneW) plan.draw_w = paneW;
    if (plan.draw_h > paneH) plan.draw_h = paneH;
    plan.draw_x = paneX + (paneW - plan.draw_w) / 2;
    plan.draw_y = paneY + (paneH - plan.draw_h) / 2;

    *out_plan = plan;
    return 1;
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

int dm1_v1_f0115_thing_layer_receipt_pc34(
    const unsigned short* thingRefs,
    int thingCount,
    int viewCell,
    int allowStaticEffectThings,
    DM1_F0115ThingLayerReceiptPc34* outReceipt) {
    DM1_F0115ThingRouteInputPc34 routeThings[32];
    int i;

    if (!thingRefs || thingCount < 0) {
        if (outReceipt) {
            memset(outReceipt, 0, sizeof(*outReceipt));
        }
        return 0;
    }
    if (thingCount > (int)(sizeof(routeThings) / sizeof(routeThings[0]))) {
        thingCount = (int)(sizeof(routeThings) / sizeof(routeThings[0]));
    }
    for (i = 0; i < thingCount; ++i) {
        routeThings[i].thing = thingRefs[i];
        routeThings[i].mirrorTextStringOrdinal = -1;
    }
    return dm1_v1_f0115_thing_route_receipt_pc34(
        routeThings, thingCount, viewCell, allowStaticEffectThings,
        -1, 0, outReceipt);
}

int dm1_v1_f0115_runtime_summary_pc34(
    const unsigned short* thingRefs,
    int thingCount,
    int liveProjectileCount,
    int liveExplosionCount,
    DM1_F0115RuntimeSummaryPc34* outSummary)
{
    DM1_F0115ThingLayerReceiptPc34 staticReceipt;
    DM1_F0115RuntimeSummaryPc34 summary;

    if (!outSummary) return 0;
    memset(outSummary, 0, sizeof(*outSummary));
    if (thingCount < 0 || liveProjectileCount < 0 ||
        liveExplosionCount < 0 ||
        (thingCount > 0 && !thingRefs)) {
        return 0;
    }

    memset(&staticReceipt, 0, sizeof(staticReceipt));
    if (!dm1_v1_f0115_thing_layer_receipt_pc34(
            thingRefs, thingCount, -1, 0, &staticReceipt) ||
        !staticReceipt.valid) {
        return 0;
    }

    /* ReDMCSB: DUNVIEW.C F0115 first classifies the square chain at
     * 4547-4581, then consumes live F0219/F0220 projectile records at
     * 5668-5683 and live explosion records at 5916-5933. Static C14/C15
     * links are intentionally excluded by the runtime receipt. */
    memset(&summary, 0, sizeof(summary));
    summary.groups = staticReceipt.groups;
    summary.items = staticReceipt.items;
    summary.sensors = staticReceipt.sensors;
    summary.textStrings = staticReceipt.textStrings;
    summary.teleporters = staticReceipt.teleporters;
    summary.doors = staticReceipt.doors;
    summary.projectiles = liveProjectileCount;
    summary.explosions = liveExplosionCount;
    summary.total = staticReceipt.drawableTotal + liveProjectileCount +
                    liveExplosionCount;
    summary.valid = 1;
    *outSummary = summary;
    return 1;
}

int dm1_v1_f0115_runtime_instance_summary_pc34(
    const DM1_F0115RuntimeInstanceInputPc34* input,
    DM1_F0115RuntimeSummaryPc34* outSummary)
{
    int projectileCount = 0;
    int explosionCount = 0;
    int i;

    if (!outSummary) return 0;
    memset(outSummary, 0, sizeof(*outSummary));
    if (!input || input->thingCount < 0 ||
        (input->thingCount > 0 && !input->thingRefs)) {
        return 0;
    }

    /* ReDMCSB DUNVIEW.C F0115:5668-5683 consumes only a live F0219
     * projectile on the current map square. An allocated cache slot is not
     * enough: reserved3 is the M10 active marker. */
    if (input->projectiles) {
        int count = input->projectiles->count;
        if (count < 0) count = 0;
        if (count > PROJECTILE_LIST_CAPACITY) count = PROJECTILE_LIST_CAPACITY;
        for (i = 0; i < count; ++i) {
            const struct ProjectileInstance_Compat* projectile =
                &input->projectiles->entries[i];
            if (projectile->slotIndex >= 0 && projectile->reserved3 != 0 &&
                projectile->mapIndex == input->mapIndex &&
                projectile->mapX == input->mapX &&
                projectile->mapY == input->mapY) {
                ++projectileCount;
            }
        }
    }

    /* ReDMCSB DUNVIEW.C F0115:5916-5933 similarly restarts for live F0220
     * explosion records. Keep invalid/empty slots out of the render receipt. */
    if (input->explosions) {
        int count = input->explosions->count;
        if (count < 0) count = 0;
        if (count > EXPLOSION_LIST_CAPACITY) count = EXPLOSION_LIST_CAPACITY;
        for (i = 0; i < count; ++i) {
            const struct ExplosionInstance_Compat* explosion =
                &input->explosions->entries[i];
            if (explosion->slotIndex >= 0 && explosion->reserved0 != 0 &&
                explosion->explosionType >= 0 &&
                explosion->mapIndex == input->mapIndex &&
                explosion->mapX == input->mapX &&
                explosion->mapY == input->mapY) {
                ++explosionCount;
            }
        }
    }

    return dm1_v1_f0115_runtime_summary_pc34(
        input->thingRefs, input->thingCount, projectileCount, explosionCount,
        outSummary);
}

int dm1_v1_f0115_runtime_summary_from_world_pc34(
    const struct GameWorld_Compat* world, int mapIndex, int mapX, int mapY,
    DM1_F0115RuntimeSummaryPc34* outSummary)
{
    DM1_F0115RuntimeInstanceInputPc34 input;
    unsigned short refs[32];
    unsigned short thing;
    int count = 0;
    if (!outSummary) return 0;
    memset(outSummary, 0, sizeof(*outSummary));
    if (!world || !world->dungeon) return 0;

    /* ReDMCSB DUNVIEW.C F0115:4547-4581 walks the current square's SFT
     * chain before its live F0219/F0220 passes.  DUNGEON.C F0160/F0161
     * owns the compact SFT lookup and record-next decode, so this bridge
     * deliberately consumes the M10 APIs rather than M11 raw records. */
    if (world->things) {
        thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
            world->dungeon, world->things, mapIndex, mapX, mapY);
        while (thing != THING_NONE && thing != THING_ENDOFLIST && count < 32) {
            refs[count++] = thing;
            thing = F0512_DUNGEON_GetThingNext_Compat(world->things, thing);
        }
    }

    memset(&input, 0, sizeof(input));
    input.thingRefs = refs;
    input.thingCount = count;
    input.projectiles = &world->projectiles;
    input.explosions = &world->explosions;
    input.mapIndex = mapIndex;
    input.mapX = mapX;
    input.mapY = mapY;
    return dm1_v1_f0115_runtime_instance_summary_pc34(&input, outSummary);
}

static int dm1_f0115_item_subtype_from_thing_pc34(
    const struct DungeonThings_Compat* things, unsigned short thing)
{
    /* F0115 receives the source F0141/F0156 raw-record result.  Invalid
     * data rejects the candidate; it does not borrow subtype-zero art. */
    return dm1_v1_dungeon_get_object_subtype_pc34(things, thing);
}

int dm1_v1_f0115_world_candidates_pc34(
    const struct GameWorld_Compat* world, int mapIndex, int mapX, int mapY,
    DM1_F0115MirrorOrdinalLookupPc34 mirrorOrdinalLookup, void* mirrorUser,
    DM1_F0115WorldCandidatesPc34* outCandidates)
{
    DM1_F0115ThingRouteInputPc34 routeThings[32];
    const struct DungeonMapDesc_Compat* map;
    unsigned short thing;
    int elementType;
    int routeThingCount = 0;
    int i;

    if (!outCandidates) return 0;
    memset(outCandidates, 0, sizeof(*outCandidates));
    if (!world || !world->dungeon || !world->things ||
        !world->dungeon->tilesLoaded || mapIndex < 0 ||
        mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width ||
        mapY >= (int)map->height || !world->dungeon->tiles ||
        !world->dungeon->tiles[mapIndex].squareData) {
        return 0;
    }
    elementType = (world->dungeon->tiles[mapIndex].squareData[
        mapX * (int)map->height + mapY] & DUNGEON_SQUARE_MASK_TYPE) >> 5;

    /* ReDMCSB DUNVIEW.C F0115:4547-4581, DUNGEON.C F0160/F0161:
     * enumerate only the compact SFT chain belonging to this square. */
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        world->dungeon, world->things, mapIndex, mapX, mapY);
    while (thing != THING_NONE && thing != THING_ENDOFLIST &&
           routeThingCount < (int)(sizeof(routeThings) / sizeof(routeThings[0]))) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        int mirrorOrdinal = -1;

        if (type == THING_TYPE_TEXTSTRING && mirrorOrdinalLookup &&
            world->things->textStrings && index >= 0 &&
            index < world->things->textStringCount) {
            mirrorOrdinal = mirrorOrdinalLookup(mirrorUser, index);
        }
        routeThings[routeThingCount].thing = thing;
        routeThings[routeThingCount].mirrorTextStringOrdinal = mirrorOrdinal;
        ++routeThingCount;
        thing = F0512_DUNGEON_GetThingNext_Compat(world->things, thing);
    }
    outCandidates->chainCount = routeThingCount;
    outCandidates->overflow = thing != THING_NONE && thing != THING_ENDOFLIST;

    if (!dm1_v1_f0115_thing_route_receipt_pc34(
            routeThings, routeThingCount, -1, 0, mapIndex,
            /* Map 0 is the Hall of Champions, not a blanket no-item zone.
             * REVIVE.C candidate payload is identified by its C127 mirror
             * control and filtered below by the receipt itself.  Suppressing
             * every open Hall square here made real floor/alcove objects
             * disappear before F0115 could select their source graphics. */
            0,
            &outCandidates->staticReceipt) ||
        !outCandidates->staticReceipt.valid) {
        return 0;
    }

    /* F0115 defers C04 groups until after floor objects.  Wall squares
     * do not take the floor-creature route.  The shipped PC34 map 0
     * (Hall of Champions) has no GROUP things in its compact chains, so
     * chain content alone keeps the real HoC group-free — suppressing
     * by map index would only break non-PC34 maps and synthetic
     * fixtures that legitimately carry groups on map 0. */
    if (elementType != DUNGEON_ELEMENT_WALL) {
        for (i = 0; i < routeThingCount &&
                    outCandidates->groupCount < DM1_F0115_MAX_WORLD_GROUPS; ++i) {
            int index;
            if (THING_GET_TYPE(routeThings[i].thing) != THING_TYPE_GROUP) continue;
            index = THING_GET_INDEX(routeThings[i].thing);
            if (world->things->groups && index >= 0 &&
                index < world->things->groupCount) {
                DM1_F0115WorldGroupCandidatePc34* group =
                    &outCandidates->groups[outCandidates->groupCount++];
                group->thing = routeThings[i].thing;
                group->creatureType = (int)world->things->groups[index].creatureType;
                group->creatureCount = (int)world->things->groups[index].count + 1;
                group->direction = (int)world->things->groups[index].direction;
            }
        }
    }

    for (i = 0; i < outCandidates->staticReceipt.visibleFloorItemCount &&
                outCandidates->itemCount < DM1_F0115_MAX_RECEIPT_ITEMS; ++i) {
        unsigned short itemThing =
            outCandidates->staticReceipt.visibleFloorItemThings[i];
        int subtype = dm1_f0115_item_subtype_from_thing_pc34(
            world->things, itemThing);
        if (subtype >= 0) {
            DM1_F0115WorldItemCandidatePc34* item =
                &outCandidates->items[outCandidates->itemCount++];
            item->thing = itemThing;
            item->thingType = THING_GET_TYPE(itemThing);
            item->subtype = subtype;
            item->cell = THING_GET_CELL(itemThing);
        }
    }
    outCandidates->valid = 1;
    return 1;
}

int dm1_v1_f0115_thing_route_receipt_pc34(
    const DM1_F0115ThingRouteInputPc34* things,
    int thingCount,
    int viewCell,
    int allowStaticEffectThings,
    int mapIndex,
    int suppressHallFloorItems,
    DM1_F0115ThingLayerReceiptPc34* outReceipt) {
    int i;
    int seenHallPayloadControl = 0;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->firstItemThing = THING_NONE;
    outReceipt->firstProjectileThing = THING_NONE;
    outReceipt->firstExplosionThing = THING_NONE;
    for (i = 0; i < DM1_F0115_MAX_RECEIPT_ITEMS; ++i) {
        outReceipt->visibleFloorItemThings[i] = THING_NONE;
    }

    if (!things || thingCount < 0) {
        return 0;
    }

    outReceipt->valid = 1;
    for (i = 0; i < thingCount; ++i) {
        unsigned short thing = things[i].thing;
        int type;

        if (thing == THING_NONE || thing == THING_ENDOFLIST) {
            break;
        }
        if (viewCell >= 0 && viewCell <= 3 &&
            (int)THING_GET_CELL(thing) != viewCell) {
            continue;
        }

        ++outReceipt->total;
        type = (int)THING_GET_TYPE(thing);
        if (dm1_v1_hall_candidate_payload_control_thing_pc34(
                mapIndex, type, things[i].mirrorTextStringOrdinal)) {
            seenHallPayloadControl = 1;
        }
        switch (type) {
            case THING_TYPE_DOOR:
                ++outReceipt->doors;
                ++outReceipt->ignoredControls;
                break;
            case THING_TYPE_TELEPORTER:
                ++outReceipt->teleporters;
                ++outReceipt->ignoredControls;
                break;
            case THING_TYPE_TEXTSTRING:
                ++outReceipt->textStrings;
                ++outReceipt->ignoredControls;
                break;
            case THING_TYPE_SENSOR:
                ++outReceipt->sensors;
                ++outReceipt->ignoredControls;
                break;
            case THING_TYPE_GROUP:
                ++outReceipt->groups;
                ++outReceipt->drawableTotal;
                break;
            case THING_TYPE_PROJECTILE:
                if (allowStaticEffectThings) {
                    ++outReceipt->projectiles;
                    ++outReceipt->drawableTotal;
                    if (outReceipt->firstProjectileThing == THING_NONE) {
                        outReceipt->firstProjectileThing = thing;
                    }
                } else {
                    ++outReceipt->ignoredStaticEffects;
                }
                break;
            case THING_TYPE_EXPLOSION:
                if (allowStaticEffectThings) {
                    ++outReceipt->explosions;
                    ++outReceipt->drawableTotal;
                    if (outReceipt->firstExplosionThing == THING_NONE) {
                        outReceipt->firstExplosionThing = thing;
                    }
                } else {
                    ++outReceipt->ignoredStaticEffects;
                }
                break;
            default:
                if (dm1_v1_thing_type_is_floor_item_pc34(type)) {
                    if ((mapIndex == 0 && suppressHallFloorItems) ||
                        seenHallPayloadControl) {
                        ++outReceipt->ignoredHallPayloadItems;
                        break;
                    }
                    ++outReceipt->items;
                    ++outReceipt->drawableTotal;
                    if (outReceipt->firstItemThing == THING_NONE) {
                        outReceipt->firstItemThing = thing;
                    }
                    if (outReceipt->visibleFloorItemCount <
                        DM1_F0115_MAX_RECEIPT_ITEMS) {
                        outReceipt->visibleFloorItemThings
                            [outReceipt->visibleFloorItemCount++] = thing;
                    } else {
                        outReceipt->overflow = 1;
                    }
                } else {
                    ++outReceipt->ignoredControls;
                }
                break;
        }
    }
    if (i >= thingCount && thingCount > 0 &&
        things[thingCount - 1].thing != THING_NONE &&
        things[thingCount - 1].thing != THING_ENDOFLIST) {
        outReceipt->overflow = 1;
    }

    /* ReDMCSB: DUNVIEW.C F0115 lines 4547-4581 walks the thing list by
     * layer, then restarts for projectile and explosion passes at lines
     * 5668-5683 and 5916-5933. Firestaff passes allowStaticEffectThings=0
     * for runtime DM1 rendering because F0219/F0220 effects are represented
     * by live runtime lists; stale dungeon C14/C15 refs must not drive HoC
     * floor/effect sprites. drawableTotal carries only F0115 drawable
     * layers, so callers do not keep false content counts after suppression.
     * REVIVE.C F0280 lines 297-349 consumes Hall mirror candidate payloads
     * from the same square chain; those C02/C03 controls mark following
     * objects as champion data, not DUNVIEW floor items. */
    return 1;
}
