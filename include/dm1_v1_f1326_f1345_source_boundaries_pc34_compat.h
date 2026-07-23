#ifndef FIRESTAFF_DM1_V1_F1326_F1345_SOURCE_BOUNDARIES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F1326_F1345_SOURCE_BOUNDARIES_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int number;
    const char *symbol;
    const char *source_anchor;
    const char *rationale;
} DM1_V1_F1326F1345BoundaryPc34;

const DM1_V1_F1326F1345BoundaryPc34 *
dm1_v1_f1326_f1345_source_boundary_pc34(unsigned int number);
int dm1_v1_f1326_f1345_admits_authentic_route_pc34(unsigned int number);
int dm1_v1_f1326_f1345_has_synthetic_route_pc34(unsigned int number);
const char *dm1_v1_f1326_f1345_source_boundary_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
