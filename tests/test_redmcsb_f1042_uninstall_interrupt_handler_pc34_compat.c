#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1042_uninstall_interrupt_handler_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1042_uninstall_interrupt_handler_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f1042_uninstall_interrupt_handler_pc34_compat());
    assert(strstr(evidence, "IO.C:1076-1085") != NULL);
    assert(strstr(evidence, "F1042_") != NULL);
    assert(strstr(evidence, "MEDIA749_A36M_A31E_A31M_A33M_A35E_A35M_X31J") != NULL);
    assert(strstr(evidence, "INT1_05_UninstallInterruptHandler") != NULL);
    assert(strstr(evidence, "G3146_s_InterruptHandler") != NULL);
    assert(strstr(evidence, "MEDIA692_X31J") != NULL);
    assert(strstr(evidence, "STARTUP2.C:1674-1680") != NULL);
    assert(strstr(evidence, "F1018_Mfree") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1042 X68000 interrupt cleanup boundary");
    return 0;
}
