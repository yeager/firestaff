#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0924_set_critical_error_handler_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f0924_set_critical_error_handler_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f0924_set_critical_error_handler_pc34_compat());
    assert(strstr(evidence, "CEDTINCI.C:306") != NULL);
    assert(strstr(evidence, "PRIM1.C:253-269") != NULL);
    assert(strstr(evidence, "Super(0L)") != NULL);
    assert(strstr(evidence, "etv_critic (0x0404)") != NULL);
    assert(strstr(evidence, "No PC 3.4 host adapter") != NULL);

    puts("ok: ReDMCSB F0924 Atari ST critical-error handler is host-bound");
    return 0;
}
