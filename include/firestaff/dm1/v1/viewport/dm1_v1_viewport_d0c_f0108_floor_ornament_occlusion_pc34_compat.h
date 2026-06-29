#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D0C F0108 floor-ornament occlusion source-lock gate.
 *
 * The D0C body column is the DM1 V1 view square where F0108 floor-ornament
 * occlusion is achieved by *not calling F0108* from the F0127 dispatch body,
 * unlike the D1C body column which calls F0108 at DUNVIEW.C:7926 with the
 * documented BUG0_64 contract. The D0C dispatch is F0127:8184-8310 and its
 * element switch only fires F0104 (pit/stairs bitmap), F0112 (ceiling pit),
 * F0115 (thing pass), and F0113 (field, for teleporters only). The F0108
 * floor-ornament blit is *not* part of the D0C dispatch, so this D0C body
 * column path is a no-call boundary where BUG0_64 does not apply.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0127:8184-8310 is the D0C dispatch body. The element switch
 *   covers C16_ELEMENT_DOOR_SIDE, C19_ELEMENT_STAIRS_FRONT, C02_ELEMENT_PIT,
 *   C05_ELEMENT_TELEPORTER. None of the cases call F0108. C02_ELEMENT_PIT
 *   fires F0104 (M761_GRAPHIC_FLOOR_PIT_D0C or M767_GRAPHIC_FLOOR_PIT_INVISIBLE_D0C
 *   at 8265-8266) on the C860_ZONE_FLOORPIT_D0C / C862_ZONE_FLOORPIT_D0C.
 *   C19_ELEMENT_STAIRS_FRONT fires F0104+F0105 stairs-up bitmap at 8221-8231
 *   (or stairs-down at 8243-8254) on C809_ZONE_STAIRS_UP_FRONT_D0L /
 *   C810_ZONE_STAIRS_UP_FRONT_D0R / C820/C821/C824/C825 (the PC 3.4 set).
 *   C16_ELEMENT_DOOR_SIDE goes through the F0104 + F0100 door-frame path.
 *   The unconditional tail at 8293 fires F0112 ceiling-pit
 *   (C068/C069_GRAPHIC_CEILING_PIT_D0C, C869/C871_ZONE_CEILING_PIT_D0C)
 *   and F0115 thing pass with C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT
 *   on M609_VIEW_SQUARE_D0C. F0113 field is fired only for the
 *   C05_ELEMENT_TELEPORTER case at 8302-8308.
 * - DUNVIEW.C F0128:8542 dispatches D0C after D0L (F0125) and D0R (F0126)
 *   and is the last square in the F0128 view sweep. The F0098 floor+ceiling
 *   base is written by F0128:8338 / 8443 / 8615 *before* the F0127 D0C
 *   dispatch fires. An F0108 floor-ornament call would be the only way to
 *   write a floor-ornament surface on this F0127 D0C body-column path, and
 *   that blit never happens there.
 * - DUNVIEW.C F0108:3940-4011 the floor-ornament routine itself: ordinal
 *   decode at 3949-3964, footprint MASK0x8000 recursion at T0108005/4008,
 *   C10_COLOR_FLESH transparency at 3988-3993, and PC 3.4 zone math
 *   C1500 + CoordinateSet*11 + ViewFloor at 3984-4011. A D0C F0108 call
 *   site does not exist in the F0127 dispatch, so this D0C body-column
 *   path never reaches F0108.
 * - DUNVIEW.C F0098:2962-3002 floor+ceiling base, F0112 ceiling-pit, F0115
 *   thing pass: these three are the only things that touch the D0C body
 *   column surface after F0098 base.
 * - DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2596 M609_VIEW_SQUARE_D0C=0;
 *   DEFS.H:2662 C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT; DEFS.H:2745
 *   M603_VIEW_FLOOR_D0C=9; DEFS.H:4045-4046 C705/C706 wall zones;
 *   DEFS.H:4209 C862_ZONE_FLOORPIT_D0C; DEFS.H:4218 C871_ZONE_CEILING_PIT_D0C;
 *   DEFS.H:4047-4048 C715_ZONE_WALL_D0C; DEFS.H:2533-2559
 *   M550..M558 aspect slots with M558_FLOOR_ORNAMENT_ORDINAL=5.
 *
 * This is synthetic metadata coverage for a 320x200 screen and a 224x136
 * viewport. It makes no real-asset or original-DOS pixel-parity claim and
 * intentionally does not duplicate the D0C F0108 floor-ornament composition
 * gate (which is a positive F0098/F0108/F0107/F0115 contract), the
 * D0C F0108 floor/ceiling/ornament gate (which covers F0098 + F0108 +
 * F0112 + F0115), the D0C F0098 ceiling/floor gate, the D0C door-edge
 * ornament gate, the D0C stairs/pit dispatch gate, the D0C F0111 partly
 * open door gate, the D1C F0108 floor-ornament occlusion gate (which
 * pins the BUG0_64 contract on D1C only), the D0C keep-out wall-ornament
 * zone, the D2/D3 viewport draw-order gates, or any CSB/Nexus/Theron/DM2
 * surface.
 */

#define DM1_V1_D0C_F0108_FOCCL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D0C_F0108_FOCCL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D0C_F0108_FOCCL_SCREEN_WIDTH_PC34 320
#define DM1_V1_D0C_F0108_FOCCL_SCREEN_HEIGHT_PC34 200
#define DM1_V1_D0C_F0108_FOCCL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0C_F0108_FOCCL_FLOOR_ZONE_BASE_PC34 1500
#define DM1_V1_D0C_F0108_FOCCL_FLOOR_ZONE_STRIDE_PC34 11
#define DM1_V1_D0C_F0108_FOCCL_C1500_ZONE_FLOOR_ORNAMENT_PC34 1500
#define DM1_V1_D0C_F0108_FOCCL_C715_ZONE_WALL_D0C_PC34 715
#define DM1_V1_D0C_F0108_FOCCL_C862_ZONE_FLOORPIT_D0C_PC34 862
#define DM1_V1_D0C_F0108_FOCCL_C871_ZONE_CEILING_PIT_D0C_PC34 871
#define DM1_V1_D0C_F0108_FOCCL_M603_VIEW_FLOOR_D0C_PC34 9
#define DM1_V1_D0C_F0108_FOCCL_M609_VIEW_SQUARE_D0C_PC34 0
#define DM1_V1_D0C_F0108_FOCCL_C0X0021_CELL_ORDER_BACKLEFT_BACKRIGHT_PC34 0x0021

typedef enum {
    DM1_V1_D0C_F0108_FOCCL_CONTEXT_DOOR_SIDE_PC34 = 0,
    DM1_V1_D0C_F0108_FOCCL_CONTEXT_STAIRS_FRONT_PC34 = 1,
    DM1_V1_D0C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34 = 2,
    DM1_V1_D0C_F0108_FOCCL_CONTEXT_TELEPORTER_PC34 = 3,
    DM1_V1_D0C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34 = 4
} DM1_V1_D0CF0108FloorOrnamentOcclusionContextPc34;

typedef enum {
    DM1_V1_D0C_F0108_FOCCL_STEP_F0128_BASE_FLOOR_CEILING_PC34 = 0,
    DM1_V1_D0C_F0108_FOCCL_STEP_F0127_BASE_SURFACE_PC34,
    DM1_V1_D0C_F0108_FOCCL_STEP_F0127_DOOR_SIDE_F0100_F0104_PC34,
    DM1_V1_D0C_F0108_FOCCL_STEP_F0127_STAIRS_FRONT_F0104_F0105_PC34,
    DM1_V1_D0C_F0108_FOCCL_STEP_F0127_OPEN_PIT_F0104_PC34,
    DM1_V1_D0C_F0108_FOCCL_STEP_F0127_TELEPORTER_F0113_FIELD_PC34,
    DM1_V1_D0C_F0108_FOCCL_STEP_F0112_CEILING_PIT_PC34,
    DM1_V1_D0C_F0108_FOCCL_STEP_F0115_THING_PASS_PC34,
    DM1_V1_D0C_F0108_FOCCL_STEP_F0108_ABSENT_FROM_D0C_PC34
} DM1_V1_D0CF0108FloorOrnamentOcclusionStepKindPc34;

typedef struct {
    DM1_V1_D0CF0108FloorOrnamentOcclusionContextPc34 context;
    int order_index;
    int calls_f0098_ceiling_floor;
    int calls_f0104_floor_pit_stairs;
    int calls_f0105_floor_pit_stairs_flipped;
    int calls_f0100_wallset_door_frame;
    int calls_f0108_floor_ornament;
    int calls_f0112_ceiling_pit;
    int calls_f0113_field;
    int calls_f0115_thing_pass;
    int expected_zone_base;
    int expected_zone_coordinate_set;
    int expected_view_floor;
    int expected_cell_order;
    int expected_view_square;
    int expected_floor_pit_graphic;
    int expected_floor_pit_invisible_graphic;
    int expected_floor_pit_zone;
    int expected_ceiling_pit_graphic;
    int expected_ceiling_pit_zone;
    int expected_wall_zone;
    int expected_d0c_no_f0108_contract;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D0CF0108FloorOrnamentOcclusionStepPc34;

typedef struct {
    int view_square_d0c;
    int view_floor_d0c;
    int wall_zone_d0c;
    int c10_transparent_color;
    int floor_ornament_ordinal_slot;
    int first_thing_slot;
    int pit_or_teleporter_visible_slot;
    int stairs_up_slot;
    int door_state_slot;
    int door_thing_index_slot;
    int f0098_base_order;
    int f0128_dispatch_order_d0c;
    int f0128_dispatch_after_d0l;
    int f0128_dispatch_after_d0r;
    int f0128_dispatch_last_in_sweep;
    int f0127_door_side_calls_f0100;
    int f0127_door_side_calls_f0104;
    int f0127_stairs_front_calls_f0104;
    int f0127_stairs_front_calls_f0105;
    int f0127_open_pit_calls_f0104;
    int f0127_open_pit_uses_pit_visible_slot;
    int f0127_teleporter_calls_f0113_field;
    int f0127_teleporter_uses_pit_visible_slot;
    int f0127_calls_f0112_ceiling_pit;
    int f0127_calls_f0115_thing_pass;
    int f0127_d0c_cell_order;
    int f0127_d0c_dispatch_no_f0108;
    int f0098_base_writes_before_f0127;
    int f0108_zone_d0c;
    int no_f0108_call_in_d0c_dispatch;
    int bug0_64_inapplicable_to_d0c;
    int f0112_ceiling_pit_graphic;
    int f0112_ceiling_pit_zone_d0c;
    int no_graphics_dat_reads;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int f0108_d0c_call_site_absent;
    int f0107_keepout_zone_left;
    int f0107_keepout_zone_right;
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34;

bool dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
    DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34 *out_model);

uint32_t dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_hash_model_pc34(
    const DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34 *model);

uint32_t dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_deterministic_hash_pc34(void);

const DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_default_model_pc34(void);

unsigned int dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_context_count_pc34(void);

const DM1_V1_D0CF0108FloorOrnamentOcclusionStepPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_step_at_pc34(size_t index);

bool dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_context_occludes_pc34(
    DM1_V1_D0CF0108FloorOrnamentOcclusionContextPc34 context);

uint8_t dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_zone_d0c_pc34(
    int coordinate_set,
    int view_floor);

const char *dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_source_evidence_pc34(void);

const char *dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
