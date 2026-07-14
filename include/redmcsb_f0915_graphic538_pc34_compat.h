#ifndef FIRESTAFF_REDMCSB_F0915_GRAPHIC538_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0915_GRAPHIC538_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * GRAPH538.C has no PC 3.4 implementation. Its available routes directly
 * program Atari ST FDC/PSG registers or Amiga disk.resource, CIA, and custom
 * DMA registers. A portable host must provide a platform-specific adapter
 * before this operation can be performed.
 */
bool redmcsb_f0915_graphic538_pc34_compat(void);

const char *redmcsb_f0915_graphic538_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0915_GRAPHIC538_PC34_COMPAT_H */
