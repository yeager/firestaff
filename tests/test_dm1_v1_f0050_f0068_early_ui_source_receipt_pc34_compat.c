#include "dm1_v1_f0050_f0068_early_ui_source_receipt_pc34_compat.h"

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
    DM1_V1_F0050F0068RuntimeInputPc34 input;
    DM1_V1_F0050F0068ReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    ok &= check(!dm1_v1_f0050_f0068_early_ui_source_receipt_pc34(
                    &input, &receipt),
                "missing original font, pointer material, owned surfaces, or IODRV fails closed");
    ok &= check(receipt.suppressSyntheticUi && !receipt.valid,
                "failure explicitly suppresses host text and cursor fallback");
    ok &= check(strstr(dm1_v1_f0050_f0068_source_evidence_pc34(),
                       "TEXT.C:1863-2027") != NULL &&
                    strstr(dm1_v1_f0050_f0068_source_evidence_pc34(),
                           "SOUND.C F0060-F0065") != NULL,
                "source audit distinguishes TEXT/IO ownership from sound");
    ok &= check(DM1_V1_F0054_M653_BYTE_COUNT_PC34 == 768 &&
                    DM1_V1_F0052_VIEWPORT_WIDTH_PC34 == 224 &&
                    DM1_V1_F0052_VIEWPORT_HEIGHT_PC34 == 136 &&
                    DM1_V1_F0053_SCREEN_WIDTH_PC34 == 320 &&
                    DM1_V1_F0053_SCREEN_HEIGHT_PC34 == 200,
                "PC34 text targets and source font geometry are pinned");
    ok &= check(dm1_v1_f0050_f0068_fnv1a_pc34(NULL, 0U) == 0U,
                "empty material never authenticates");
    if (!ok) return 1;
    puts("PASS dm1_v1_f0050_f0068_early_ui_source_receipt_pc34_compat");
    return 0;
}
