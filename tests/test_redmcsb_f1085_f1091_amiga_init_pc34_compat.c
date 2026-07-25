#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1085_intuition_vector_replacement_pc34_compat.h"
#include "redmcsb_f1086_replace_intuition_vectors_pc34_compat.h"
#include "redmcsb_f1087_restore_intuition_vectors_pc34_compat.h"
#include "redmcsb_f1088_open_amiga_stuff_pc34_compat.h"
#include "redmcsb_f1089_close_amiga_stuff_pc34_compat.h"
#include "redmcsb_f1090_get_csb_internal_error_message_pc34_compat.h"
#include "redmcsb_f1091_get_csb_system_error_message_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    uint8_t *internal = redmcsb_f1090_get_csb_internal_error_message_pc34_compat();
    (void)internal;
    uint8_t *system = redmcsb_f1091_get_csb_system_error_message_pc34_compat();
    (void)system;

    assert(redmcsb_f1085_intuition_vector_replacement_pc34_compat() == 0);
    redmcsb_f1086_replace_intuition_vectors_pc34_compat();
    redmcsb_f1087_restore_intuition_vectors_pc34_compat();
    redmcsb_f1088_open_amiga_stuff_pc34_compat();
    redmcsb_f1089_close_amiga_stuff_pc34_compat();
    assert(internal[0] == 0x00U && internal[1] == 0xb8U && internal[2] == 0x10U);
    assert(strcmp((const char *)&internal[3], "Chaos Strikes Back System Error 00") == 0);
    assert(system[0] == 0x00U && system[1] == 0x94U && system[2] == 0x10U);
    assert(strcmp((const char *)&system[3], "Chaos Strikes Back Internal Error: 00000000") == 0);
    assert(strstr(redmcsb_f1086_replace_intuition_vectors_source_evidence_pc34(), "AMIGINIT.C:283-291") != NULL);
    assert(strstr(redmcsb_f1087_restore_intuition_vectors_source_evidence_pc34(), "0x80FF0015") != NULL);
    assert(strstr(redmcsb_f1088_open_amiga_stuff_source_evidence_pc34(), "AMIGINIT.C:333-361") != NULL);
    assert(strstr(redmcsb_f1089_close_amiga_stuff_source_evidence_pc34(), "AMIGINIT.C:363-389") != NULL);
    assert(strstr(redmcsb_f1090_get_csb_internal_error_message_source_evidence_pc34(), "F1093") != NULL);
    assert(strstr(redmcsb_f1091_get_csb_system_error_message_source_evidence_pc34(), "F1094") != NULL);
    puts("ok: ReDMCSB F1085-F1091 Amiga-init boundaries and error templates");
    return 0;
}
