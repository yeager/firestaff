#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1080_close_input_device_pc34_compat.h"

int main(void)
{
    const char *evidence = redmcsb_f1080_close_input_device_source_evidence_pc34();
    (void)evidence;
    redmcsb_f1080_close_input_device_pc34_compat();
    assert(strstr(evidence, "AMIGINIT.C:217-232") != NULL);
    assert(strstr(evidence, "no PC 3.4 branch") != NULL);
    puts("ok: ReDMCSB F1080 input-device teardown boundary");
    return 0;
}
