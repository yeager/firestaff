#ifndef FIRESTAFF_DM1_V1_MAIN_LOWLEVEL_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MAIN_LOWLEVEL_BOUNDARY_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_MainLowLevelKindPc34Compat {
    DM1_V1_MAIN_LOWLEVEL_COMPOSED_RUNTIME = 1,
    DM1_V1_MAIN_LOWLEVEL_HOST_ENTRY_BOUNDARY = 2,
    DM1_V1_MAIN_LOWLEVEL_PLANAR_SURFACE_BOUNDARY = 3,
    DM1_V1_MAIN_LOWLEVEL_PORTABLE_PRIMITIVE = 4,
    DM1_V1_MAIN_LOWLEVEL_EXCEPTION_VECTOR_BOUNDARY = 5
} DM1_V1_MainLowLevelKindPc34Compat;

typedef struct DM1_V1_MainLowLevelAuditPc34Compat {
    uint16_t functionNumber;
    DM1_V1_MainLowLevelKindPc34Compat kind;
    int hasFirestaffOwner;
    int hostMutationForbidden;
    const char *sourceAnchor;
} DM1_V1_MainLowLevelAuditPc34Compat;

typedef struct DM1_V1_F0018ExceptionVectorReceiptPc34Compat {
    int accepted;
    int hostVectorMutationSuppressed;
    int syntheticInterruptSuppressed;
    uint32_t sourceFingerprint;
} DM1_V1_F0018ExceptionVectorReceiptPc34Compat;

/* Source inventory for F0003-F0010 and F0018. It describes ownership only;
 * it never dispatches process entry points, emulates Atari ST planar output,
 * or installs host exception vectors. */
const DM1_V1_MainLowLevelAuditPc34Compat *
dm1_v1_main_lowlevel_audit_pc34(uint16_t function_number);

/* ReDMCSB BASE.C F0018 is a 68000 supervisor-vector installation routine.
 * Firestaff deliberately refuses this host boundary on every platform. */
int dm1_v1_f0018_set_exception_vectors_host_boundary_pc34(
    DM1_V1_F0018ExceptionVectorReceiptPc34Compat *out_receipt);

const char *dm1_v1_main_lowlevel_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
