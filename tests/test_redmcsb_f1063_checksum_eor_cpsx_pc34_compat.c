#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1063_checksum_eor_cpsx_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1063_checksum_eor_cpsx_source_evidence_pc34();

    redmcsb_f1063_checksum_eor_cpsx_pc34_compat();

    assert(strstr(evidence, "READWRIT.C:73-110") != NULL);
    assert(strstr(evidence, "F1063_ChecksumEor_CPSX") != NULL);
    assert(strstr(evidence, "MEDIA626_A31E_A31M_A33M_A35E_A35M") != NULL);
    assert(strstr(evidence, "NOCOPYPROTECTION disabled") != NULL);
    assert(strstr(evidence, "READWRIT.C:90-108") != NULL);
    assert(strstr(evidence, "two function addresses") != NULL);
    assert(strstr(evidence, "68k asm") != NULL);
    assert(strstr(evidence, "movea.l") != NULL);
    assert(strstr(evidence, "eor.w") != NULL);
    assert(strstr(evidence, "COMMAND.C:1537-1542") != NULL);
    assert(strstr(evidence, "declares void") != NULL);
    assert(strstr(evidence, "no portable callable behavior") != NULL);
    assert(strstr(evidence, "PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1063 checksum EOR copy-protection boundary");
    return 0;
}
