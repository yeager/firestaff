#ifndef FIRESTAFF_REDMCSB_F1076_CLOSE_LAYERS_LIBRARY_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1076_CLOSE_LAYERS_LIBRARY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F1076_CloseLayersLibrary releases an Amiga Layers library handle. The
 * source supplies no PC 3.4 branch or portable host behavior.
 */
void redmcsb_f1076_close_layers_library_pc34_compat(void);

const char *redmcsb_f1076_close_layers_library_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1076_CLOSE_LAYERS_LIBRARY_PC34_COMPAT_H */
