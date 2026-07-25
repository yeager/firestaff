#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1048_setjmp.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

static __attribute__((unused)) int jump_with_zero_value(void)
{
    jmp_buf environment;
    int result = F1048_setjmp(environment);

    if (result == 0) {
        longjmp(environment, 0);
    }
    return result;
}

int main(void)
{
    const char *evidence = redmcsb_f1048_setjmp_source_evidence();
    (void)evidence;

    assert(jump_with_zero_value() == 1);
    assert(strstr(evidence, "DEFS.H:3208-3215") != NULL);
    assert(strstr(evidence, "F1048_setjmp") != NULL);
    assert(strstr(evidence,
                  "MEDIA749_A36M_A31E_A31M_A33M_A35E_A35M_X31J") != NULL);
    assert(strstr(evidence, "MEDIA764_AU1E_AU2E_AU3E") != NULL);
    assert(strstr(evidence, "DEFS.H:3399-3408") != NULL);
    assert(strstr(evidence, "MEDIA551_F20E_F20J_F31E_F31J") != NULL);

    puts("ok: ReDMCSB F1048 portable setjmp alias");
    return 0;
}
