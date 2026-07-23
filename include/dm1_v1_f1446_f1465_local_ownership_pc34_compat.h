#ifndef FIRESTAFF_DM1_V1_F1446_F1465_LOCAL_OWNERSHIP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F1446_F1465_LOCAL_OWNERSHIP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int number;
    const char *symbol;
    const char *parent_owner;
    const char *source_anchor;
} DM1_V1_F1446F1465LocalOwnershipPc34;

const DM1_V1_F1446F1465LocalOwnershipPc34 *
dm1_v1_f1446_f1465_local_ownership_pc34(unsigned int number);
int dm1_v1_f1446_f1465_admits_standalone_route_pc34(unsigned int number);
int dm1_v1_f1446_f1465_has_synthetic_route_pc34(unsigned int number);
const char *dm1_v1_f1446_f1465_local_ownership_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
