#include "dm1_v1_input_poll_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void test_ascii_extraction_uses_only_explicit_low_byte(void)
{
    assert(F1690_GetASCIICode(0x0041u) == 0x0041u);
    assert(F1690_GetASCIICode(0x0141u) == 0x0041u);
    assert(F1690_GetASCIICode(DM1_KEY_ESCAPE) == DM1_KEY_ESCAPE);
    assert(F1690_GetASCIICode(DM1_KEY_BACKSPACE) == DM1_KEY_BACKSPACE);
    assert(F1690_GetASCIICode(DM1_KEY_FORWARD) == 0);
    assert(F1690_GetASCIICode(0x00E9u) == 0);
}

static void test_cconis_and_crawcin_read_caller_owned_key_buffer(void)
{
    DM1_V1_InputStatePc34 state;
    DM1_V1_Input_InitPc34Compat(&state);

    assert(F1691_Cconis(0) == 0);
    assert(F1692_Crawcin(0) == 0);
    assert(F1691_Cconis(&state) == 0);
    assert(F1692_Crawcin(&state) == 0);

    assert(DM1_V1_Input_StoreKeyPc34Compat(&state, 0x0041u) == 1);
    assert(F1691_Cconis(&state) == 1);
    assert(F1692_Crawcin(&state) == 0x0041u);
    assert(F1691_Cconis(&state) == 0);

    assert(DM1_V1_Input_StoreKeyPc34Compat(&state, DM1_KEY_FORWARD) == 1);
    assert(F1691_Cconis(&state) == 1);
    assert(F1692_Crawcin(&state) == DM1_KEY_FORWARD);
    assert(F1691_Cconis(&state) == 0);
}

static void test_source_evidence_names_exact_boundaries(void)
{
    const char *evidence = F1690_GetASCIICode_SourceEvidence();
    (void)evidence;
    assert(strstr(evidence, "USIO2.C:287") != 0);
    assert(strstr(evidence, "low-byte ASCII") != 0);
    assert(strstr(evidence, "no scan-code table") != 0);
    assert(strstr(evidence, "host polling") != 0);

    evidence = F1691_Cconis_SourceEvidence();
    assert(strstr(evidence, "USIO2.C:282") != 0);
    assert(strstr(evidence, "key-buffer availability") != 0);
    assert(strstr(evidence, "does not poll host") != 0);

    evidence = F1692_Crawcin_SourceEvidence();
    assert(strstr(evidence, "USIO2.C:15") != 0);
    assert(strstr(evidence, "buffered key") != 0);
    assert(strstr(evidence, "fabricating host input") != 0);
}

int main(void)
{
    test_ascii_extraction_uses_only_explicit_low_byte();
    test_cconis_and_crawcin_read_caller_owned_key_buffer();
    test_source_evidence_names_exact_boundaries();
    return 0;
}
