#ifndef NEXUS_V1_VDP1_COMMAND_SEQUENCE_H
#define NEXUS_V1_VDP1_COMMAND_SEQUENCE_H

#include <stdint.h>

#include "nexus_v1_dungeon.h"

/* Bounded hardware framing for one authenticated VDP1 VRAM snapshot.  The
 * command list is followed through CMDLINK exactly as documented by the
 * Saturn VDP1 command format; no DGN face, camera transform, or draw meaning
 * is assigned here. */
#define NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS 256

typedef struct {
    const uint8_t *vdp1_vram;
    int vdp1_vram_size;
    /* Raw VDP1 COPR value from the captured state line. */
    uint32_t copr_word;
    /* Optional live SysClip registers from the same authenticated frame. */
    int system_clip_state_present;
    uint32_t system_clip_x;
    uint32_t system_clip_y;
} Nexus_V1_Vdp1CommandSequenceInput;

typedef struct {
    int valid;
    int complete;
    int command_order_verified;
    int end_record_verified;
    int display_origin_verified;
    int semantic_admission_blocked;
    uint32_t copr_byte_offset;
    uint32_t start_byte_offset;
    uint32_t end_byte_offset;
    int command_count;
    int draw_count;
    int user_clip_count;
    int system_clip_count;
    int system_clip_state_verified;
    uint32_t system_clip_x;
    uint32_t system_clip_y;
    int local_coordinate_count;
    int display_origin_x;
    int display_origin_y;
    int user_clip_x0;
    int user_clip_y0;
    int user_clip_x1;
    int user_clip_y1;
    uint32_t command_byte_offsets[NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS];
    /* Local-coordinate state effective at each record in command order.
     * The corresponding verified flag stays clear until the first LOCAL
     * command, so a consumer cannot manufacture an origin for earlier draws. */
    int command_origin_x[NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS];
    int command_origin_y[NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS];
    uint8_t command_origin_verified[NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS];
} Nexus_V1_Vdp1CommandSequenceReceipt;

/* Returns 1 only for a bounded chain containing the captured COPR cursor,
 * at least one draw, clip state, local-coordinate state, and END record. */
int nexus_v1_vdp1_command_sequence_frame(
    const Nexus_V1_Vdp1CommandSequenceInput *input,
    Nexus_V1_Vdp1CommandSequenceReceipt *out_receipt);

#endif
