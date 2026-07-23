#include "dm1_v1_f0413_cpsc_checksum_eor_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

int main(void)
{
    const uint8_t pairSpan[] = { 1, 2, 3, 4 };
    const uint8_t functionSpan[] = {
        0, 1, 0, 2, 0x4e, 0x5e, 0x4e, 0x75
    };
    const uint8_t skipSpan[] = { 10, 20, 30, 40, 50 };
    const uint8_t skipTable[] = {
        0, 5, 0, 2, 0, 1, 0xff, 0xff, 0, 0
    };
    const uint8_t truncatedTable[] = { 0, 5, 0, 2 };
    DM1_V1_F0413CpscRequestPc34 request;
    DM1_V1_F0413CpscReceiptPc34 receipt;

    memset(&request, 0, sizeof(request));
    request.mediaSourceVerified = 1;
    CHECK(strstr(dm1_v1_f0413_cpsc_checksum_eor_source_evidence_pc34(),
                 "COPYPRO8.C F0413") != NULL);

    request.variant = DM1_V1_F0413_CPSC_PAIR_SPAN_PC34;
    request.functionBytes = pairSpan;
    request.functionByteCount = sizeof(pairSpan);
    CHECK(dm1_v1_f0413_cpsc_checksum_eor_pc34(&request, &receipt));
    CHECK(receipt.accepted && receipt.checksum == 0x0406u &&
          receipt.consumedByteCount == sizeof(pairSpan) &&
          receipt.suppressSyntheticFallback);

    request.variant = DM1_V1_F0413_CPSC_TERMINATED_FUNCTION_PC34;
    request.functionBytes = functionSpan;
    request.functionByteCount = sizeof(functionSpan);
    CHECK(dm1_v1_f0413_cpsc_checksum_eor_pc34(&request, &receipt));
    CHECK(receipt.checksum == 6u && receipt.consumedByteCount == 4u);

    request.variant = DM1_V1_F0413_CPSC_SKIP_TABLE_PC34;
    request.functionBytes = skipSpan;
    request.functionByteCount = sizeof(skipSpan);
    request.skipTableBytes = skipTable;
    request.skipTableByteCount = sizeof(skipTable);
    CHECK(dm1_v1_f0413_cpsc_checksum_eor_pc34(&request, &receipt));
    CHECK(receipt.checksum == 118u && receipt.consumedByteCount == 5u);

    request.mediaSourceVerified = 0;
    CHECK(!dm1_v1_f0413_cpsc_checksum_eor_pc34(&request, &receipt));
    CHECK(!receipt.accepted && !receipt.suppressSyntheticFallback);

    request.mediaSourceVerified = 1;
    request.variant = DM1_V1_F0413_CPSC_PAIR_SPAN_PC34;
    request.functionByteCount = 3;
    CHECK(!dm1_v1_f0413_cpsc_checksum_eor_pc34(&request, &receipt));
    CHECK(!receipt.accepted);

    request.variant = DM1_V1_F0413_CPSC_SKIP_TABLE_PC34;
    request.functionBytes = skipSpan;
    request.functionByteCount = sizeof(skipSpan);
    request.skipTableBytes = truncatedTable;
    request.skipTableByteCount = sizeof(truncatedTable);
    CHECK(!dm1_v1_f0413_cpsc_checksum_eor_pc34(&request, &receipt));
    CHECK(!receipt.accepted);

    printf("test_dm1_v1_f0413_cpsc_checksum_eor_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
