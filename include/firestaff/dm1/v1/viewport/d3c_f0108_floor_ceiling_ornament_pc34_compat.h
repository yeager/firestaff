#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3C_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3C_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D3C F0108 floor+ceiling+ornament source-lock gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0108:3940-4011: floor-ornament ordinal decoding,
 *   MASK0x8000_FOOTPRINTS recursion, C10 transparency, D3C center
 *   footprint flipping branch, and C1500 + CoordinateSet * 11 + ViewFloor.
 * - DUNVIEW.C F0119:6987-7031 and F0120:7180-7224: neighboring
 *   third-depth D2L/D2R floor/ceiling/thing-pass clipping surfaces that use
 *   the same F0108/F0112/F0115 source-lock contract as this D3C gate.
 * - DUNVIEW.C F0128:8503-8517: footprint/order recursion reaches D3C
 *   before the near D2L/D2R floor+ceiling follow-up.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523:
 *   thing-list mutation boundaries and square-aspect construction.
 *
 * This gate exercises a synthetic 320x200 framebuffer with a 224x136
 * viewport bound. It proves source-lock metadata and draw-order contracts
 * only; it makes no original DOS pixel-parity or real-asset bitmap claim.
 */

typedef struct {
    int ok;
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int d3c_floor_calls;
    int ceiling_calls;
    int footprint_recursions;
    int thing_pass_calls;
    int mutation_rejections;
} DM1_V1_D3CF0108SelfTestResultPc34;

int run_dm1_v1_viewport_d3c_f0108_floor_ceiling_ornament_self_test(void);

const DM1_V1_D3CF0108SelfTestResultPc34 *
dm1_v1_viewport_d3c_f0108_last_self_test_result_pc34(void);

const char *dm1_v1_viewport_d3c_f0108_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
