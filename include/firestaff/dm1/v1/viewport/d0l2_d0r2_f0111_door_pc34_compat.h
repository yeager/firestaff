#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0111_DOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0111_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D0L2_D0R2_F0111_DOOR_CONTRACT_ONLY_PC34 1
#define DM1_V1_D0L2_D0R2_F0111_DOOR_NO_GAME_DATA_PC34 1
#define DM1_V1_D0L2_D0R2_F0111_DOOR_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0L2_D0R2_F0111_DOOR_MASK0X4000_PC34 0x4000
#define DM1_V1_D0L2_D0R2_F0111_DOOR_MASK0X8000_PC34 0x8000

typedef enum {
    DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0L2_PC34 = 1,
    DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0R2_PC34 = 2
} DM1_V1_D0L2D0R2F0111DoorSidePc34;

typedef enum {
    DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_OPEN_PC34 = 0,
    DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_CLOSED_ONE_FOURTH_PC34 = 1,
    DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_CLOSED_HALF_PC34 = 2,
    DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_CLOSED_THREE_FOURTH_PC34 = 3,
    DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_CLOSED_PC34 = 4,
    DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_DESTROYED_PC34 = 5
} DM1_V1_D0L2D0R2F0111DoorStatePc34;

typedef enum {
    DM1_V1_D0L2_D0R2_TRACE_F0128_RELATIVE_PC34 = 0x28,
    DM1_V1_D0L2_D0R2_TRACE_F0104_F0105_NATIVE_C10_PC34 = 0xa4,
    DM1_V1_D0L2_D0R2_TRACE_F0107_WALL_KEEP_OUT_PC34 = 0xa7,
    DM1_V1_D0L2_D0R2_TRACE_F0108_MASK8000_KEEP_OUT_PC34 = 0xa8,
    DM1_V1_D0L2_D0R2_TRACE_F0115_PASS1_PC34 = 0xb1,
    DM1_V1_D0L2_D0R2_TRACE_F0111_ENTER_PC34 = 0xd1,
    DM1_V1_D0L2_D0R2_TRACE_F0111_CLOSED_PC34 = 0xd4,
    DM1_V1_D0L2_D0R2_TRACE_F0111_PARTLY_OPEN_PC34 = 0xd2,
    DM1_V1_D0L2_D0R2_TRACE_F0111_C6_HALF_BLIT_PC34 = 0xc6,
    DM1_V1_D0L2_D0R2_TRACE_F0111_FINAL_C10_PC34 = 0xda,
    DM1_V1_D0L2_D0R2_TRACE_F0115_PASS2_PC34 = 0xb2,
    DM1_V1_D0L2_D0R2_TRACE_NO_DOOR_ON_CELL_PC34 = 0xee
} DM1_V1_D0L2D0R2F0111DoorTraceOpcodePc34;

typedef struct {
    uint8_t opcode;
    const char *name;
    const char *anchor;
} DM1_V1_D0L2D0R2F0111DoorOpcodePc34;

typedef struct {
    int side;
    const char *label;
    int f0128_order;
    int relative_depth;
    int relative_lateral;
    int f0128_update_line;
    int f0128_draw_line;
    int requested_view_square;
    int source_view_square;
    int wall_zone;
    int wall_set_before_pc_fix;
    int wall_set_after_pc_fix;
    int door_zone;
    int door_frame_left_zone;
    int door_frame_right_zone;
    unsigned int f0115_pass1_cell_order;
    unsigned int f0115_pass2_cell_order;
    int f0115_first_cell_pass1;
    int f0115_first_cell_pass2;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
} DM1_V1_D0L2D0R2F0111DoorCellPc34;

typedef struct {
    int contract_only;
    int no_game_data;
    int trace_opcode_count;
    const DM1_V1_D0L2D0R2F0111DoorOpcodePc34 *trace_opcodes;
    const char *redmcsb_f0128_dispatch_anchor;
    const char *redmcsb_f0128_d0_detail_anchor;
    const char *redmcsb_f0104_f0105_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D0L2D0R2F0111DoorSourceLockPc34;

typedef struct {
    int side;
    int door_on_cell;
    int door_state;
    int door_vertical;
    int wall_ornament_keepout;
    int floor_ornament_mask0x8000;
    int pc_fix_code_size;
    uint8_t destination_pixel;
    uint8_t native_blit_pixel;
    uint8_t door_pixel;
    uint8_t pass2_pixel;
} DM1_V1_D0L2D0R2F0111DoorInputPc34;

typedef struct {
    const DM1_V1_D0L2D0R2F0111DoorCellPc34 *cell;
    uint8_t opcodes[16];
    size_t opcode_count;
    int door_called;
    int closed_or_destroyed_selected;
    int partly_open_selected;
    int c6_half_blit_called;
    int final_c10_blit_called;
    int f0104_f0105_native_c10_called;
    int f0107_keepout_called;
    int f0108_mask0x8000_keepout_called;
    int f0115_pass1_called;
    int f0115_pass2_called;
    int wall_set_selected;
    int f0111_zone;
    int f0111_c6_zone;
    int f0111_final_zone;
    uint8_t after_native_blit;
    uint8_t after_door;
    uint8_t after_pass2;
} DM1_V1_D0L2D0R2F0111DoorTracePc34;

const char *
dm1_v1_viewport_d0l2_d0r2_f0111_door_source_evidence_pc34(void);

const DM1_V1_D0L2D0R2F0111DoorSourceLockPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_door_source_lock_pc34(void);

size_t dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_count_pc34(void);

const DM1_V1_D0L2D0R2F0111DoorCellPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_at_pc34(size_t index);

const DM1_V1_D0L2D0R2F0111DoorCellPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_for_side_pc34(int side);

uint8_t dm1_v1_viewport_d0l2_d0r2_f0111_door_blend_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(
    unsigned int cell_order,
    int ordinal);

int dm1_v1_viewport_d0l2_d0r2_f0111_door_model_pc34(
    const DM1_V1_D0L2D0R2F0111DoorInputPc34 *input,
    DM1_V1_D0L2D0R2F0111DoorTracePc34 *out_trace);

uint32_t dm1_v1_viewport_d0l2_d0r2_f0111_door_fnv1a_pc34(
    const uint8_t *bytes,
    size_t count);

#ifdef __cplusplus
}
#endif

#endif
