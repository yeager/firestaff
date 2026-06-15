#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2C_F0111_DOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2C_F0111_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D2C_F0111_DOOR_PC34_C10_COLOR_FLESH 10
#define DM1_V1_D2C_F0111_DOOR_PC34_MASK0X8000_FOOTPRINTS 0x8000u
#define DM1_V1_D2C_F0111_DOOR_PC34_MAX_TRACE 32

typedef enum {
    DM1_V1_D2C_F0111_OP_F0128_STACK_D1C_NEARER_PC34 = 1,
    DM1_V1_D2C_F0111_OP_F0128_STACK_D2C_PC34 = 2,
    DM1_V1_D2C_F0111_OP_F0128_STACK_D3C_FARTHER_PC34 = 3,
    DM1_V1_D2C_F0111_OP_F0121_D2C_BODY_PC34 = 4,
    DM1_V1_D2C_F0111_OP_F0108_FLOOR_ORNAMENT_PC34 = 5,
    DM1_V1_D2C_F0111_OP_F0108_MASK_0X8000_KEEP_OUT_PC34 = 6,
    DM1_V1_D2C_F0111_OP_F0115_PASS1_BACK_CELLS_PC34 = 7,
    DM1_V1_D2C_F0111_OP_F0104_LEFT_FRAME_C10_PC34 = 8,
    DM1_V1_D2C_F0111_OP_F0105_RIGHT_FRAME_C10_PC34 = 9,
    DM1_V1_D2C_F0111_OP_F0111_DOOR_CLOSED_PC34 = 10,
    DM1_V1_D2C_F0111_OP_F0111_DOOR_PARTLY_OPEN_PC34 = 11,
    DM1_V1_D2C_F0111_OP_F0111_C10_TRANSPARENT_PC34 = 12,
    DM1_V1_D2C_F0111_OP_F0115_PASS2_FRONT_CELLS_PC34 = 13,
    DM1_V1_D2C_F0111_OP_F0107_WALL_ORNAMENT_KEEP_OUT_PC34 = 14,
    DM1_V1_D2C_F0111_OP_ZONE_BAND_C705_C706_PC34 = 15,
    DM1_V1_D2C_F0111_OP_NEGATIVE_NO_DOOR_FRONT_CELL_PC34 = 16,
    DM1_V1_D2C_F0111_OP_NEGATIVE_NO_NATIVE_FRAME_BLIT_PC34 = 17
} DM1_V1_D2CF0111DoorOpcodePc34;

typedef enum {
    DM1_V1_D2C_F0111_DOOR_STATE_OPEN_PC34 = 0,
    DM1_V1_D2C_F0111_DOOR_STATE_ONE_FOURTH_PC34 = 1,
    DM1_V1_D2C_F0111_DOOR_STATE_HALF_PC34 = 2,
    DM1_V1_D2C_F0111_DOOR_STATE_THREE_FOURTHS_PC34 = 3,
    DM1_V1_D2C_F0111_DOOR_STATE_CLOSED_PC34 = 4,
    DM1_V1_D2C_F0111_DOOR_STATE_DESTROYED_PC34 = 5
} DM1_V1_D2CF0111DoorStatePc34;

typedef enum {
    DM1_V1_D2C_F0111_ELEMENT_CORRIDOR_PC34 = 1,
    DM1_V1_D2C_F0111_ELEMENT_WALL_PC34 = 0,
    DM1_V1_D2C_F0111_ELEMENT_DOOR_FRONT_PC34 = 17
} DM1_V1_D2CF0111ElementPc34;

typedef struct {
    DM1_V1_D2CF0111DoorOpcodePc34 opcode;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D2CF0111DoorOpcodeInfoPc34;

typedef struct {
    int contract_only;
    int no_game_data;
    int no_real_asset_bitmap_parity;
    int view_square_d2c;
    int relative_depth;
    int relative_lateral;
    int element_door_front;
    int door_zone_d2c;
    int door_frame_top_zone_d2c;
    int door_frame_left_zone_d2c;
    int door_frame_right_zone_d2c;
    int door_native_bitmap_index_front_d2lcr;
    int door_ornament_view_d2lcr;
    int transparent_color;
    unsigned int f0115_pass1_order;
    unsigned int f0115_pass2_order;
    int f0128_source_line_order_after_d3c;
    int f0128_source_line_order_before_d1c;
    int painter_stack_after_d1c;
    int painter_stack_before_d3c;
    int defs_zone_band_c705;
    int defs_zone_band_c706;
    const char *redmcsb_f0121_d2c_anchor;
    const char *redmcsb_f0124_required_anchor;
    const char *redmcsb_f0128_caller_anchor;
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_f0104_f0105_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D2CF0111DoorSourceLockPc34;

typedef struct {
    int view_square_index;
    int element;
    int door_state;
    unsigned int floor_ornament_ordinal;
    int wall_ornament_ordinal;
    uint8_t destination_pixel;
    uint8_t floor_pixel;
    uint8_t left_frame_pixel;
    uint8_t right_frame_pixel;
    uint8_t door_pixel;
    uint8_t pass1_pixel;
    uint8_t pass2_pixel;
} DM1_V1_D2CF0111DoorScenarioPc34;

typedef struct {
    int ok;
    int opcode_count;
    DM1_V1_D2CF0111DoorOpcodePc34 opcodes[DM1_V1_D2C_F0111_DOOR_PC34_MAX_TRACE];
    int f0111_count;
    int f0104_count;
    int f0105_count;
    int f0107_keepout_count;
    int f0108_mask_keepout_count;
    int f0115_pass_count;
    int c10_transparent_count;
    int negative_count;
    uint8_t after_floor;
    uint8_t after_pass1;
    uint8_t after_left_frame;
    uint8_t after_right_frame;
    uint8_t after_door;
    uint8_t after_pass2;
    unsigned int pass1_cells;
    unsigned int pass2_cells;
} DM1_V1_D2CF0111DoorTracePc34;

const char *dm1_v1_viewport_d2c_f0111_door_source_evidence_pc34(void);
const DM1_V1_D2CF0111DoorSourceLockPc34 *
dm1_v1_viewport_d2c_f0111_door_source_lock_pc34(void);
size_t dm1_v1_viewport_d2c_f0111_door_opcode_count_pc34(void);
const DM1_V1_D2CF0111DoorOpcodeInfoPc34 *
dm1_v1_viewport_d2c_f0111_door_opcode_at_pc34(size_t index);
int dm1_v1_viewport_d2c_f0111_door_trace_pc34(
    const DM1_V1_D2CF0111DoorScenarioPc34 *scenario,
    DM1_V1_D2CF0111DoorTracePc34 *out_trace);
uint32_t dm1_v1_viewport_d2c_f0111_door_hash_trace_pc34(
    const DM1_V1_D2CF0111DoorTracePc34 *trace);
uint8_t dm1_v1_viewport_d2c_f0111_door_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

#ifdef __cplusplus
}
#endif

#endif
