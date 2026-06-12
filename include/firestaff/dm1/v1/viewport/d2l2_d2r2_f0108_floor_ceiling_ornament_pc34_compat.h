#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D2L2/D2R2 F0108 floor+ceiling+ornament source lock.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0108:3940-4011: F0108 floor ornament ordinal, C10
 *   transparency, PC34 floor-zone math, and footprint recursion.
 * - DUNVIEW.C F0119:6987-7031: D2L route after second-depth clipping.
 * - DUNVIEW.C F0120:7180-7224: D2R route and second-depth side ornament.
 * - DUNVIEW.C F0128:8503-8517: D2L2/D2R2 footprint/dispatch recursion
 *   before D2L/D2R F0108-capable squares.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, F0172:2466-2523:
 *   cell fetch, thing-cell fetch, and kappetaal aspect construction.
 *
 * This gate is source-lock metadata and bounded-framebuffer composition
 * only; it makes no original DOS pixel parity claim.
 */

typedef struct {
    int ok;
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int d2l2_floor_calls;
    int d2r2_floor_calls;
    int footprint_recursions;
    int ceiling_calls;
    int thing_pass_calls;
    int mutation_rejections;
} DM1_V1_D2L2D2R2F0108SelfTestResultPc34;

int run_dm1_v1_viewport_d2l2_d2r2_f0108_floor_ceiling_ornament_self_test(void);

const DM1_V1_D2L2D2R2F0108SelfTestResultPc34 *
dm1_v1_viewport_d2l2_d2r2_f0108_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
