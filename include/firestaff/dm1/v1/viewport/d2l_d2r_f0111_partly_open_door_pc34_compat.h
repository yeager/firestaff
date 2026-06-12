/*
 * DM1 V1 PC 3.4 contract-only source-lock gate for F0111 partly-open
 * corridor-side D2L/D2R door fronts.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor; line 4248 skips
 *   fully-open doors, line 4308 decrements partly-open states, lines
 *   4312-4313 select LeftHorizontal/RightHorizontal, lines 4317-4324
 *   apply the state-adjusted zone plus C6_UNKNOWN and C10_COLOR_FLESH
 *   first-half blit, and lines 4325-4334 apply 3|MASK0x4000 before
 *   F0791_DUNGEONVIEW_DrawBitmapXX with C10_COLOR_FLESH.
 * - DUNVIEW.C:8504-8508 F0128 MEDIA720 D2L2/D2R2 side-wall guard,
 *   8513-8517 D2L/D2R dispatch, and 8521 F0121 D2C dispatch only as
 *   a center-square order bound.
 * - DUNVIEW.C:6987-7004 F0119 D2L and 7180-7197 F0120 D2R are the
 *   D2L/D2R C17_ELEMENT_DOOR_FRONT F0111 callers.
 * - DUNVIEW.C:7244-7389 F0121_DUNGEONVIEW_DrawSquareD2C is cited only
 *   as a D2C-center anchor, not as the D2L/D2R corridor-side route.
 * - DUNVIEW.C:6837-6865 F0678_DrawD2L2 and 6868-6896 F0679_DrawD2R2
 *   are side-wall anchors which return wall cases before any F0111
 *   door-front route.
 * - DEFS.H:1039-1043 C0..C4 door states, 2088 C10_COLOR_FLESH,
 *   2603-2604 M604_VIEW_SQUARE_D2L/M605_VIEW_SQUARE_D2R, 2669/2672
 *   C0x0218/C0x0349, 2790 C1_VIEW_DOOR_ORNAMENT_D2LCR, 3508
 *   C6_UNKNOWN, 3516 MASK0x4000, 4254-4258 M627/M629 door zones,
 *   and 5457/5539/5541 G0694/G0182/G0184 symbols.
 * - CSB counterpart: csb_v1_viewport_d2c_f0111_partly_open_door_pc34_compat
 *   covers D2C partly-open horizontal halves; sibling
 *   test_dm1_v1_viewport_d0l_d0r_f0111_door_pc34_compat covers the
 *   non-overlapping DM1 D0L/D0R closed-door contract.
 *
 * This gate is synthetic and asset-free. It makes no real-asset or
 * original-DOS pixel-parity claim.
 */
#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int d2l_partly_one;
    int d2l_partly_two;
    int d2l_partly_three;
    int d2r_partly_one;
    int d2r_partly_two;
    int d2r_partly_three;
    int d2l_partly;
    int d2r_partly;
    int closed_rejections;
    int open_rejections;
    int unknown_rejections;
} DM1_V1_D2LD2RF0111PartlyOpenDoorSelfTestResultPc34;

int run_dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_self_test(void);

const DM1_V1_D2LD2RF0111PartlyOpenDoorSelfTestResultPc34 *
dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_last_self_test_result_pc34(void);

const char *
dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
