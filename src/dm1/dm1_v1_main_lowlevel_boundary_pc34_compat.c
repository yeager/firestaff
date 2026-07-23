#include "dm1_v1_main_lowlevel_boundary_pc34_compat.h"

#include <stddef.h>
#include <string.h>

static const char s_source_evidence[] =
    "ReDMCSB NEWMAP.C F0003; DEFS.H F004/F005 declarations; "
    "BASE.C F0006-F0010 and F0018:1057-1081; CPCLRBYT.C F0007/F0008. "
    "F0018 writes Atari ST exception vectors 0x70/0x90/0x114/0x118/0x120 "
    "and is deliberately a fail-closed host boundary.";

static const DM1_V1_MainLowLevelAuditPc34Compat s_audit[] = {
    {3u, DM1_V1_MAIN_LOWLEVEL_COMPOSED_RUNTIME, 1, 0,
     "NEWMAP.C F0003 -> F0174/F0096/F0194/F0195/F0337"},
    {4u, DM1_V1_MAIN_LOWLEVEL_HOST_ENTRY_BOUNDARY, 0, 1,
     "DEFS.H:6535 main() program entry"},
    {5u, DM1_V1_MAIN_LOWLEVEL_HOST_ENTRY_BOUNDARY, 0, 1,
     "DEFS.H:6536 argc/argv initializer"},
    {6u, DM1_V1_MAIN_LOWLEVEL_PLANAR_SURFACE_BOUNDARY, 0, 1,
     "BASE.C F0006 Atari ST planar screen inversion"},
    {7u, DM1_V1_MAIN_LOWLEVEL_PORTABLE_PRIMITIVE, 1, 0,
     "CPCLRBYT.C F0007 overlap-safe byte copy"},
    {8u, DM1_V1_MAIN_LOWLEVEL_PORTABLE_PRIMITIVE, 1, 0,
     "CPCLRBYT.C F0008 bounded byte clear"},
    {9u, DM1_V1_MAIN_LOWLEVEL_PORTABLE_PRIMITIVE, 1, 0,
     "BASE.C F0009 spaced byte fill"},
    {10u, DM1_V1_MAIN_LOWLEVEL_PORTABLE_PRIMITIVE, 1, 0,
     "BASE.C F0010 spaced word fill"},
    {18u, DM1_V1_MAIN_LOWLEVEL_EXCEPTION_VECTOR_BOUNDARY, 1, 1,
     "BASE.C F0018 Atari ST exception-vector installation"}
};

const DM1_V1_MainLowLevelAuditPc34Compat *
dm1_v1_main_lowlevel_audit_pc34(uint16_t function_number)
{
    size_t index;

    for (index = 0u; index < sizeof(s_audit) / sizeof(s_audit[0]); ++index) {
        if (s_audit[index].functionNumber == function_number) {
            return &s_audit[index];
        }
    }
    return NULL;
}

int dm1_v1_f0018_set_exception_vectors_host_boundary_pc34(
    DM1_V1_F0018ExceptionVectorReceiptPc34Compat *out_receipt)
{
    DM1_V1_F0018ExceptionVectorReceiptPc34Compat receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.hostVectorMutationSuppressed = 1;
    receipt.syntheticInterruptSuppressed = 1;
    *out_receipt = receipt;
    return 0;
}

const char *dm1_v1_main_lowlevel_source_evidence_pc34(void)
{
    return s_source_evidence;
}
