#include "redmcsb_f1063_checksum_eor_cpsx_pc34_compat.h"

void redmcsb_f1063_checksum_eor_cpsx_pc34_compat(void)
{
}

const char *redmcsb_f1063_checksum_eor_cpsx_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/READWRIT.C:73-110 defines "
           "F1063_ChecksumEor_CPSX only inside "
           "MEDIA626_A31E_A31M_A33M_A35E_A35M with NOCOPYPROTECTION "
           "disabled. READWRIT.C:90-108 accepts two function addresses and "
           "uses only 68k asm: movea.l, moveq, add.w, eor.w, addq.w, "
           "cmpa.l, and bcs.s. COMMAND.C:1537-1542 assigns its result even "
           "though READWRIT.C:90 declares void. The source supplies no "
           "portable callable behavior or PC 3.4 branch.";
}
