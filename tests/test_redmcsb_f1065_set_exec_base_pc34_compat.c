#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1065_set_exec_base_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1065_set_exec_base_source_evidence_pc34();
    (void)evidence;

    redmcsb_f1065_set_exec_base_pc34_compat();

    assert(strstr(evidence, "AMIGALIB.C:837-844") != NULL);
    assert(strstr(evidence, "F1065_SetExecBase") != NULL);
    assert(strstr(evidence, "AMIGALIB.C:841-843") != NULL);
    assert(strstr(evidence, "move.l (4).w,ExecBase(a5)") != NULL);
    assert(strstr(evidence, "68k global-base register") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    assert(strstr(evidence, "portable host behavior") != NULL);

    puts("ok: ReDMCSB F1065 Amiga Exec-base boundary");
    return 0;
}
