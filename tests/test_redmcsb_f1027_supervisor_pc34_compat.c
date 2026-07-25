#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1027_supervisor_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1027_supervisor_source_evidence_pc34();

    assert(!redmcsb_f1027_supervisor_pc34_compat(0L));
    assert(!redmcsb_f1027_supervisor_pc34_compat(0x12345678L));
    assert(strstr(evidence, "IMAGE.C:30-38") != NULL);
    assert(strstr(evidence, "CEDT027.C:540-547") != NULL);
    assert(strstr(evidence, "F1027_Supervisor") != NULL);
    assert(strstr(evidence, "MEDIA577_X30J") != NULL);
    assert(strstr(evidence, "MEDIA692_X31J") != NULL);
    assert(strstr(evidence, "DOS CALL SUPER (0xFF20)") != NULL);
    assert(strstr(evidence, "DEFS.H:9636-9639") != NULL);
    assert(strstr(evidence, "IO.C:1887-1918") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1027 X68000 supervisor boundary");
    return 0;
}
