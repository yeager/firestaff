#ifndef FIRESTAFF_REDMCSB_F1072_CLOSE_GRAPHICS_LIBRARY_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1072_CLOSE_GRAPHICS_LIBRARY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F1072_CloseGraphicsLibrary releases an Amiga graphics library handle. The
 * source supplies no PC 3.4 branch or portable host behavior.
 */
void redmcsb_f1072_close_graphics_library_pc34_compat(void);

const char *redmcsb_f1072_close_graphics_library_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1072_CLOSE_GRAPHICS_LIBRARY_PC34_COMPAT_H */
