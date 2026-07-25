#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1052_wait_for_scan_line_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1052_wait_for_scan_line_source_evidence_pc34();

    redmcsb_f1052_wait_for_scan_line_pc34_compat(INT16_MIN);
    redmcsb_f1052_wait_for_scan_line_pc34_compat(0);
    redmcsb_f1052_wait_for_scan_line_pc34_compat(INT16_MAX);

    assert(strstr(evidence, "FILLBOX.C:5-33") != NULL);
    assert(strstr(evidence, "F1052_WaitForScanLine") != NULL);
    assert(strstr(evidence,
                  "MEDIA746_A36M_A31E_A31M_A33M_A35E_A35M") != NULL);
    assert(strstr(evidence, "adds 44") != NULL);
    assert(strstr(evidence, "shifts it by eight") != NULL);
    assert(strstr(evidence, "VPOSR/VHPOSR") != NULL);
    assert(strstr(evidence, "0xDFF004") != NULL);
    assert(strstr(evidence, "0x1FF00") != NULL);
    assert(strstr(evidence, "FILLBOX.C:565, 709, 731, 746, and 759") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1052 Amiga scan-line wait boundary");
    return 0;
}
