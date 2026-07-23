#include "dm1_v1_f0121_f0140_core_graphics_source_receipt_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int value, const char *label)
{
    if (value) return 1;
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    DM1_V1_F0121F0140RuntimeInputPc34 input;
    DM1_V1_F0121F0140ReceiptPc34 receipt;
    const char *evidence;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    ok &= check(!dm1_v1_f0121_f0140_core_graphics_source_receipt_pc34(
                    &input, &receipt),
                "missing authentic game data or surfaces fails closed");
    ok &= check(receipt.suppressSyntheticPresentation && !receipt.valid &&
                    receipt.f0137CpsfUnavailableOnPc34 &&
                    receipt.f0139DelegatedToExistingOwner &&
                    receipt.f0140DelegatedToExistingOwner,
                "no fallback is emitted and disjoint/platform ownership remains explicit");
    evidence = dm1_v1_f0121_f0140_source_evidence_pc34();
    ok &= check(strstr(evidence, "F0121-F0127") != NULL &&
                    strstr(evidence, "F0129") != NULL &&
                    strstr(evidence, "F0137") != NULL &&
                    strstr(evidence, "F0140") != NULL,
                "source audit covers the disjoint F0121-F0140 range");
    ok &= check(DM1_V1_F0121_F0140_VIEWPORT_WIDTH_PC34 == 224 &&
                    DM1_V1_F0121_F0140_VIEWPORT_HEIGHT_PC34 == 136 &&
                    DM1_V1_F0121_F0140_VIEWPORT_BYTE_WIDTH_PC34 == 112 &&
                    DM1_V1_F0121_F0140_SQUARE_COUNT_PC34 == 7,
                "PC34 viewport and square-dispatch constants are pinned");
    ok &= check(dm1_v1_f0121_f0140_fnv1a_pc34(NULL, 0U) == 0U,
                "empty source material cannot authenticate");
    if (!ok) return 1;
    puts("PASS dm1_v1_f0121_f0140_core_graphics_source_receipt_pc34_compat");
    return 0;
}
