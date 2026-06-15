#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0C_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D0C F0111 partly-open door transparency boundary.
 *
 * ReDMCSB anchors confirmed from the local checkout:
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor; open guard at
 *   4248, C4 closed branch at 4297-4298, C5 destroyed branch at
 *   4301-4304, C1..C3 state decrement at 4308, horizontal half-frame
 *   selection at 4312-4313, C6_UNKNOWN/C10 first-half blit at
 *   4317-4324, and 3|MASK0x4000 plus final C10 F0791 draw at 4325-4334.
 * - DUNVIEW.C:7244-7389 F0121_DUNGEONVIEW_DrawSquareD2C; its
 *   C17_ELEMENT_DOOR_FRONT branch reaches F0111 at 7313/7316 for D2C,
 *   not for D0C.
 * - DUNVIEW.C:8164-8311 F0127_DUNGEONVIEW_DrawSquareD0C; the D0C body
 *   dispatches C16_ELEMENT_DOOR_SIDE to the G0172/G2116 D0C door-frame
 *   path at 8185-8236 and has no F0111 call site.
 * - DUNVIEW.C:8498-8542 F0128_DUNGEONVIEW_Draw_CPSF order; F0121 is
 *   called for D2C at 8521, then D0L/D0R at 8536/8541, and D0C is
 *   called through F0127 at 8542.
 * - DUNVIEW.C:92 and 2654-2658 define/fill
 *   G0695_ai_DoorNativeBitmapIndex_Front_D1LCR[2]; this is a D1LCR
 *   native door-panel index pair and is not a D0C native bitmap.
 * - DUNVIEW.C:151/226/242/259 and 2162/2181/2196 define and load
 *   G2116_DoorFrameFrontD0C for the D0C native door-frame bitmap.
 * - DEFS.H:1039-1044 C0/C1/C2/C3/C4/C5 door-state ordinals;
 *   3508 C6_UNKNOWN; 3516 MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION...
 *   and 4086 C728_ZONE_DOOR_FRAME_D0C.
 *
 * Non-overlap marker: pass769-d0c-f0111-partly-open-boundary. This gate
 * is synthetic, uses a 320x200 framebuffer with a 224x136 viewport, and
 * makes no real-asset or original-DOS pixel parity claim.
 */

#define DM1_V1_D0C_F0111_PARTLY_OPEN_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D0C_F0111_PARTLY_OPEN_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D0C_F0111_PARTLY_OPEN_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D0C_F0111_PARTLY_OPEN_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D0C_F0111_PARTLY_OPEN_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0C_F0111_PARTLY_OPEN_C6_UNKNOWN_PC34 6
#define DM1_V1_D0C_F0111_PARTLY_OPEN_MASK0X4000_PC34 0x4000

typedef enum {
    DM1_V1_D0C_F0111_BRANCH_OPEN_PC34 = 0,
    DM1_V1_D0C_F0111_BRANCH_PARTLY_OPEN_PC34 = 1,
    DM1_V1_D0C_F0111_BRANCH_CLOSED_PC34 = 2,
    DM1_V1_D0C_F0111_BRANCH_DESTROYED_PC34 = 3,
    DM1_V1_D0C_F0111_BRANCH_INVALID_PC34 = -1
} DM1_V1_D0CF0111PartlyOpenBranchPc34;

typedef struct {
    int input_state;
    DM1_V1_D0CF0111PartlyOpenBranchPc34 branch;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int f0111_line_start;
    int f0111_line_end;
    int f0121_d2c_dispatch_line;
    int f0127_d0c_dispatch_line;
    int f0128_d2c_call_line;
    int f0128_d0c_call_line;
    int d0c_uses_f0127_not_f0121;
    int d0c_has_no_f0111_call_site;
    int d2c_f0121_routes_f0111;
    int g0695_is_d1lcr_not_d0c;
    int d0c_native_bitmap_is_g2116;
    int decremented_state;
    int first_half_zone;
    int first_half_clip_zone;
    int second_half_zone;
    int second_half_shift;
    int c10_transparent_color;
    int first_half_writes;
    int first_half_skips;
    int second_half_writes;
    int second_half_skips;
    uint8_t first_probe_pixel;
    uint8_t second_probe_pixel;
} DM1_V1_D0CF0111PartlyOpenDoorTracePc34;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int open_branch;
    int partly_open_branches;
    int closed_branch;
    int destroyed_branch;
    int invalid_branch;
    int c10_write_skip_checks;
    int d0c_dispatch_boundary_checks;
    int native_bitmap_boundary_checks;
    int non_overlap_checks;
} DM1_V1_D0CF0111PartlyOpenDoorSelfTestResultPc34;

int dm1_v1_viewport_d0c_f0111_partly_open_door_trace_pc34(
    int door_state,
    DM1_V1_D0CF0111PartlyOpenDoorTracePc34 *out_trace);

const char *
dm1_v1_viewport_d0c_f0111_partly_open_door_source_evidence_pc34(void);

int run_dm1_v1_viewport_d0c_f0111_partly_open_door_self_test(void);

const DM1_V1_D0CF0111PartlyOpenDoorSelfTestResultPc34 *
dm1_v1_viewport_d0c_f0111_partly_open_door_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
