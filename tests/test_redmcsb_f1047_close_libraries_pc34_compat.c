#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1047_close_libraries_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1047_close_libraries_source_evidence_pc34();

    assert(!redmcsb_f1047_close_libraries_pc34_compat());
    assert(strstr(evidence, "MAINLIB.C:4-60") != NULL);
    assert(strstr(evidence, "F1047_CloseLibraries") != NULL);
    assert(strstr(evidence,
                  "MEDIA749_A36M_A31E_A31M_A33M_A35E_A35M_X31J") != NULL);
    assert(strstr(evidence, "MAINLIB.C:52-58") != NULL);
    assert(strstr(evidence, "MEDIA692_X31J") != NULL);
    assert(strstr(evidence, "F9009_ClosePRIM") != NULL);
    assert(strstr(evidence, "GAMELOOP.C:334-336") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1047 non-PC library-close boundary");
    return 0;
}
