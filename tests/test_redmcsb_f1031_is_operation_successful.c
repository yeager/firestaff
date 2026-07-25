#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1031_is_operation_successful.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    int16_t error_count = 0;
    (void)error_count;
    const char *evidence;
    (void)evidence;

    assert(redmcsb_f1031_is_operation_successful(&error_count));
    assert(error_count == 0);

    error_count = 1;
    assert(!redmcsb_f1031_is_operation_successful(&error_count));
    assert(error_count == 0);

    error_count = INT16_MIN;
    assert(!redmcsb_f1031_is_operation_successful(&error_count));
    assert(error_count == 0);

    evidence = redmcsb_f1031_is_operation_successful_source_evidence();
    assert(strstr(evidence, "FILE.C:754-792") != NULL);
    assert(strstr(evidence, "FILE.C:763-792") != NULL);
    assert(strstr(evidence, "MEDIA607_X30J_X31J") != NULL);
    assert(strstr(evidence, "G3091_i_ErrorCount") != NULL);
    assert(strstr(evidence, "C0_FALSE") != NULL);
    assert(strstr(evidence, "C1_TRUE") != NULL);
    assert(strstr(evidence, "TRAP 14") != NULL);

    puts("ok: ReDMCSB F1031 operation-success error-counter transition");
    return 0;
}
