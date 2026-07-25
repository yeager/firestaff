#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0944_exec_vector_patch_check_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f0944_exec_vector_patch_check_source_evidence_pc34();

    assert(!redmcsb_f0944_exec_vector_patch_check_pc34_compat());
    assert(strstr(evidence, "EXEC.C:4-16") != NULL);
    assert(strstr(evidence, "EXEC.C:408-431") != NULL);
    assert(strstr(evidence, "MEDIA442_A20E_A21E") != NULL);
    assert(strstr(evidence, "DoIO (-0x1C6)") != NULL);
    assert(strstr(evidence, "WaitIO (-0x1D8)") != NULL);
    assert(strstr(evidence, "OpenDevice (-0x1BA)") != NULL);
    assert(strstr(evidence, "G0727_B_") != NULL);
    assert(strstr(evidence, "0x80000") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    puts("ok: ReDMCSB F0944 Amiga Exec-vector host boundary");
    return 0;
}
