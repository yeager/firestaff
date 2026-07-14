#ifndef FIRESTAFF_REDMCSB_F1010_LOAD_X68000_BORDER_GRAPHICS_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1010_LOAD_X68000_BORDER_GRAPHICS_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IMAGE.C F1010_LoadX68000BorderGraphics is enclosed by
 * MEDIA607_X30J_X31J and writes its decoded graphics to X68000 video memory.
 * It has no PC 3.4 branch or portable host adapter.
 */
bool redmcsb_f1010_load_x68000_border_graphics_pc34_compat(void);

const char *redmcsb_f1010_load_x68000_border_graphics_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1010_LOAD_X68000_BORDER_GRAPHICS_PC34_COMPAT_H */
