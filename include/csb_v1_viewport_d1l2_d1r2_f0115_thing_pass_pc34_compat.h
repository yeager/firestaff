#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1L2_D1R2_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1L2_D1R2_F0115_THING_PASS_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked contract-only gate, not real-asset bitmap parity.
 * ReDMCSB anchors: DUNVIEW.C F0122_DUNGEONVIEW_DrawSquareD1L lines
 * 7391-7557 and F0123_DUNGEONVIEW_DrawSquareD1R lines 7559-7725; view-depth
 * dispatch at 6789-6793; F0115 lines 4547-4581, 4806-4811, 4923, 5075,
 * 5201-5214, 5615-5617, 5668-5683, 5916-5923, 5998-5999.
 * DEFS.H anchors: 2088, 2596-2601, 4228-4230, and 4250-4260.
 * Cross-source: CSBWin Viewport.cpp and CSB-lineage Viewport.cpp bind
 * F1L1/F1R1 open room objects at 1167-1188; CSB-lineage
 * CustomBackgrounds/ApplyDecoration stays separate at 6503-6551.
 */

typedef struct {
    const char *contract_scope;
    const char *f0122_d1l_lines;
    const char *f0123_d1r_lines;
    const char *f0115_lines;
    const char *defs_lines;
    const char *csbwin_room_object_lines;
    const char *csb_lineage_room_object_lines;
    const char *csb_lineage_custom_backgrounds_lines;
} CSB_V1_ViewportD1L2D1R2F0115ThingPassEvidencePc34;

typedef struct {
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int side;
    int route_count;
    int f0115_call_count;
    int c10_transparency_flag;
    int transparent_color;
    int wall_route;
    int no_wall_route;
    int zone_binding_tag_count;
    int item_zone_base;
    int projectile_zone_base;
    int creature_zone_base;
    int explosion_zone_base;
    const char *zone_binding_tag;
    int view_square_index;
    int view_depth;
    int view_lane;
    int object_g2028_row;
    int creature_g2033_row;
    int explosion_g2034_row;
    int field_aspect_index;
    unsigned int f0115_cell_order;
    int no_f0107_contract;
    int no_f0111_contract;
    int no_custom_backgrounds_contract;
    int floor_ornament_precedes_f0115;
    int ceiling_pit_precedes_f0115;
    int teleporter_field_follows_f0115;
    int door_front_route_excluded;
    int f0115_draw_order_objects_first;
    int f0115_draw_order_creatures_second;
    int f0115_draw_order_projectiles_third;
    int f0115_draw_order_explosions_last;
    int csbwin_relative_cell;
    int csbwin_contents_opcode;
    int csbwin_draw_order_opcode;
    int csbwin_std_draw_room_objects_opcode;
    const char *route_name;
    const char *source_lines;
} CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34;

int csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_init_pc34(void);

size_t csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_count_pc34(void);

const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *
csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_at_pc34(size_t index);

const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *
csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(int side);

const CSB_V1_ViewportD1L2D1R2F0115ThingPassEvidencePc34 *
csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_evidence_pc34(void);

const char *csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
