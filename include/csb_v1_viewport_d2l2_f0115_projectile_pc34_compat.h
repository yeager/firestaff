#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2L2_F0115_PROJECTILE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2L2_F0115_PROJECTILE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int view_depth;
    int view_lane;
    int field_aspect_index;
    int field_zone;
    int f0678_f0679_has_f0115_route;
    int f0113_teleporter_route;
    int door_front_rear_f0115_order;
    int door_front_f0111_order;
    int door_front_front_f0115_order;
    int projectile_g2028_row;
    int projectile_zone_base;
    int projectile_zone_cell_stride;
    int projectile_restarts_thing_list;
    int projectile_requires_type_c14;
    int projectile_requires_cell_match;
    int projectile_rejects_missing_row;
    int projectile_suppresses_depth3_front_cells;
    int projectile_suppresses_depth0_back_cells;
    int projectile_derived_bitmap_none;
    int projectile_uses_f0791_blit;
    int projectile_transparent_color;
    int creature_g2033_row;
    int creature_requires_group_marker;
    int creature_rejects_missing_row;
    int creature_zone_base;
    int creature_coordinate_set_stride;
    int creature_zone_cell_stride;
    int shift_objects_and_creatures_mask;
    int object_g2028_row;
    int object_zone_base;
    int object_zone_cell_stride;
    int object_requires_type_weapon_to_junk;
    int object_requires_cell_match;
    int object_rejects_missing_row;
    int object_pile_shift_advances;
    int explosion_g2034_row;
    int explosion_restarts_thing_list_after_cells;
    int explosion_rebirth_requires_visible_row_and_cell_match;
    int explosion_rebirth_step1_zone_base;
    int explosion_rebirth_step2_zone_base;
    int explosion_centered_zone_base;
    int explosion_side_zone_base;
    int explosion_side_zone_cell_stride;
    int explosion_uses_f0791_blit;
    int fluxcage_defers_to_field;
    int fluxcage_field_zone;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportD2L2F0115ProjectileRouteSpec;

size_t M11_GameView_D2L2F0115ProjectileRouteSpecCount(void);
const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *
M11_GameView_D2L2F0115ProjectileRouteSpecAt(size_t index);
const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *
M11_GameView_D2L2F0115ProjectileRouteSpecForSquare(int view_square);

int M11_GameView_D2L2F0115ProjectileZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec,
    unsigned char view_cell);
int M11_GameView_D2L2F0115ProjectileObjectZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec,
    unsigned char view_cell);
int M11_GameView_D2L2F0115ProjectileCreatureZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec,
    int coordinate_set,
    unsigned char view_cell);
int M11_GameView_D2L2F0115ProjectileExplosionRebirthStep1Zone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec);
int M11_GameView_D2L2F0115ProjectileExplosionRebirthStep2Zone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec);
int M11_GameView_D2L2F0115ProjectileExplosionCenteredZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec);
int M11_GameView_D2L2F0115ProjectileExplosionSideZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec,
    unsigned char view_cell);
int M11_GameView_D2L2F0115ProjectileTeleporterFieldZone(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec);
int M11_GameView_D2L2F0115ProjectileApplySyntheticC10Blit(
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

const char *M11_GameView_D2L2F0115ProjectileSourceEvidence(void);

#ifdef __cplusplus
}
#endif

#endif
