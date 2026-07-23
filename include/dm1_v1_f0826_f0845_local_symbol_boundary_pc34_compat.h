#ifndef FIRESTAFF_DM1_V1_F0826_F0845_LOCAL_SYMBOL_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0826_F0845_LOCAL_SYMBOL_BOUNDARY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB labels 0826-0845 are L-local storage, not F-callable symbols.
 * They have no standalone PC34 ABI, data asset, renderer, or timer route. */
typedef struct {
    unsigned int number;
    const char *label;
    const char *source_file;
    const char *parent_callable;
} DM1_V1_F0826F0845LocalSymbolBoundaryPc34;

const DM1_V1_F0826F0845LocalSymbolBoundaryPc34 *
dm1_v1_f0826_f0845_local_symbol_boundary_pc34(unsigned int number);
int dm1_v1_f0826_f0845_has_standalone_pc34_route(unsigned int number);
const char *dm1_v1_f0826_f0845_local_symbol_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
