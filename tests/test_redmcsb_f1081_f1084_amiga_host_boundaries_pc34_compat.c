#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1081_open_nil_pc34_compat.h"
#include "redmcsb_f1082_close_nil_pc34_compat.h"
#include "redmcsb_f1083_allocate_724_bytes_pc34_compat.h"
#include "redmcsb_f1084_free_724_bytes_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *open_nil = redmcsb_f1081_open_nil_source_evidence_pc34();
    (void)open_nil;
    const char *close_nil = redmcsb_f1082_close_nil_source_evidence_pc34();
    (void)close_nil;
    const char *allocate = redmcsb_f1083_allocate_724_bytes_source_evidence_pc34();
    (void)allocate;
    const char *free_bytes = redmcsb_f1084_free_724_bytes_source_evidence_pc34();
    (void)free_bytes;

    redmcsb_f1081_open_nil_pc34_compat();
    redmcsb_f1082_close_nil_pc34_compat();
    redmcsb_f1083_allocate_724_bytes_pc34_compat();
    redmcsb_f1084_free_724_bytes_pc34_compat();

    assert(strstr(open_nil, "AMIGINIT.C:235-246") != NULL);
    assert(strstr(open_nil, "NIL:") != NULL);
    assert(strstr(close_nil, "AMIGINIT.C:248-257") != NULL);
    assert(strstr(close_nil, "G3160_ps_NIL2") != NULL);
    assert(strstr(allocate, "AMIGINIT.C:259-266") != NULL);
    assert(strstr(allocate, "AMIGA.H:99-108") != NULL);
    assert(strstr(free_bytes, "AMIGINIT.C:267-275") != NULL);
    assert(strstr(free_bytes, "G3161_ac_Buffer724Bytes") != NULL);
    assert(strstr(open_nil, "No PC 3.4 branch") != NULL);
    assert(strstr(close_nil, "No PC 3.4 branch") != NULL);
    assert(strstr(allocate, "No PC 3.4 branch") != NULL);
    assert(strstr(free_bytes, "No PC 3.4 branch") != NULL);
    puts("ok: ReDMCSB F1081-F1084 Amiga host boundaries");
    return 0;
}
