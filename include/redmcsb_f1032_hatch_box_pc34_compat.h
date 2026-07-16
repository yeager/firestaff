#ifndef FIRESTAFF_REDMCSB_F1032_HATCH_BOX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1032_HATCH_BOX_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F1032_GRF1_12_HatchBox exists only on the X68000 and Amiga media routes.
 * PC 3.4 provides neither of its hardware graphics backends nor a
 * source-defined portable adapter.
 */
bool redmcsb_f1032_hatch_box_pc34_compat(void);

bool F1032_GRF1_12_HatchBox(void);

const char *redmcsb_f1032_hatch_box_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1032_HATCH_BOX_PC34_COMPAT_H */
