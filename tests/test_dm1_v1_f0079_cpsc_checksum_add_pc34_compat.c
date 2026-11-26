#ifdef NDEBUG
#undef NDEBUG
#endif
#include "dm1_v1_f0079_cpsc_checksum_add_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static DM1_V1_F0079CpscRequestPc34 request_for(
    DM1_V1_F0079CpscVariantPc34 variant,
    const uint8_t *code, size_t code_size)
{
    DM1_V1_F0079CpscRequestPc34 request;
    memset(&request, 0, sizeof(request));
    request.variant = variant;
    request.codeBytes = code;
    request.codeByteCount = code_size;
    request.codeImageAddress = UINT32_C(0x1000);
    request.functionAddress = UINT32_C(0x1000);
    request.mediaSourceVerified = 1;
    return request;
}

int main(void)
{
    /* COPYPRO1.C:20-30: follow one JMP-absolute table entry, sum big-endian
     * words from 100, and stop before UNLK A6 / RTS. */
    const uint8_t atari_code[] = {
        0x4e, 0xf9, 0x00, 0x00, 0x10, 0x06,
        0x00, 0x01, 0x00, 0x02, 0x4e, 0x5e, 0x4e, 0x75
    };
    const uint8_t amiga_code[] = {
        0x00, 0x01, 0x00, 0x02, 0x4e, 0x5d, 0x4e, 0x75
    };
    const uint8_t skip_code[] = { 1, 2, 3, 4, 5, 6 };
    const uint8_t skip_table[] = { 0, 6, 0, 2, 0, 2, 0, 6 };
    DM1_V1_F0079CpscRequestPc34 request;
    DM1_V1_F0079CpscReceiptPc34 receipt;

    request = request_for(DM1_V1_F0079_CPSC_TERMINATED_ATARI_ST_PC34,
                          atari_code, sizeof(atari_code));
    assert(dm1_v1_f0079_cpsc_checksum_add_pc34(&request, &receipt));
    assert(receipt.accepted && receipt.checksum == 103u);
    assert(receipt.consumedByteCount == 4u);
    assert(receipt.suppressSyntheticFallback);

    request = request_for(DM1_V1_F0079_CPSC_TERMINATED_AMIGA_PC34,
                          amiga_code, sizeof(amiga_code));
    assert(dm1_v1_f0079_cpsc_checksum_add_pc34(&request, &receipt));
    assert(receipt.checksum == 103u && receipt.consumedByteCount == 4u);

    request = request_for(DM1_V1_F0079_CPSC_SKIP_TABLE_PC34,
                          skip_code, sizeof(skip_code));
    request.skipTableBytes = skip_table;
    request.skipTableByteCount = sizeof(skip_table);
    assert(dm1_v1_f0079_cpsc_checksum_add_pc34(&request, &receipt));
    assert(receipt.checksum == 114u && receipt.consumedByteCount == 6u);

    request.skipTableByteCount = 6u;
    assert(!dm1_v1_f0079_cpsc_checksum_add_pc34(&request, &receipt));
    assert(!receipt.accepted);
    request = request_for(DM1_V1_F0079_CPSC_TERMINATED_ATARI_ST_PC34,
                          atari_code, 10u);
    assert(!dm1_v1_f0079_cpsc_checksum_add_pc34(&request, &receipt));
    request.mediaSourceVerified = 0;
    assert(!dm1_v1_f0079_cpsc_checksum_add_pc34(&request, &receipt));

    assert(strstr(dm1_v1_f0079_cpsc_checksum_add_source_evidence_pc34(),
                  "COPYPRO1.C:14-73") != NULL);
    puts("PASS: bounded ReDMCSB F0079 checksum variants");
    return 0;
}
