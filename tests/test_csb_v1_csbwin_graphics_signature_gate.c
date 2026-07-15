#include "csb_v1_csbwin_graphics_signature_gate.h"

#include <stdio.h>

static int failures;

static void expect_equal(const char *name, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "%s: got %d, expected %d\n", name, actual, expected);
        ++failures;
    }
}

int main(void)
{
    const char *md5 = "000102030405060708090a0b0c0d0e0f";
    CSB_V1_CSBWinGraphicsSignatureReceipt receipt;
    int result;

    result = csb_v1_csbwin_graphics_signature_gate_validate_md5(
        md5, CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_STANDARD,
        0x03020100u, 0x07060504u, 0u, 0u, &receipt);
    expect_equal("exact extended header", result,
                 CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_OK);
    expect_equal("signature1", (int)receipt.actual_signature1,
                 (int)0x03020100u);
    expect_equal("signature2", (int)receipt.actual_signature2,
                 (int)0x07060504u);
    expect_equal("expected gate receipt", receipt.expected_signature_checked, 1);

    result = csb_v1_csbwin_graphics_signature_gate_validate_md5(
        md5, CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_STANDARD,
        1u, 0u, 0u, 0u, NULL);
    expect_equal("reject extended mismatch", result,
                 CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_ERR_EXPECTED);

    result = csb_v1_csbwin_graphics_signature_gate_validate_md5(
        md5, CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_STANDARD,
        0u, 0u, receipt.actual_folded_signature, 0u, &receipt);
    expect_equal("standard EXPOOL folded exact", result,
                 CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_OK);
    expect_equal("standard runtime receipt", receipt.runtime_signature_checked, 1);

    result = csb_v1_csbwin_graphics_signature_gate_validate_md5(
        md5, CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_STANDARD,
        0u, 0u, 0xffffffffu, 0u, &receipt);
    expect_equal("standard wildcard skips EXPOOL gate", result,
                 CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_OK);
    expect_equal("standard wildcard receipt", receipt.runtime_signature_checked, 0);

    result = csb_v1_csbwin_graphics_signature_gate_validate_md5(
        md5, CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_CUSTOM,
        0u, 0u, receipt.actual_folded_signature ^ 1u, 0u, NULL);
    expect_equal("custom EXPOOL mismatch", result,
                 CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_ERR_RUNTIME);

    result = csb_v1_csbwin_graphics_signature_gate_validate_md5(
        md5, CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_CUSTOM,
        0u, 0u, 1u, 1u, &receipt);
    expect_equal("debugging bypasses custom EXPOOL gate", result,
                 CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_OK);
    expect_equal("custom debug receipt", receipt.runtime_signature_checked, 0);

    result = csb_v1_csbwin_graphics_signature_gate_validate_md5(
        "00000000000000000000000000000000",
        CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_CUSTOM,
        1u, 0u, 1u, 0u, &receipt);
    expect_equal("CSBWin zero signature normalization", result,
                 CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_OK);
    expect_equal("custom zero folded normalization",
                 (int)receipt.actual_folded_signature, 1);

    result = csb_v1_csbwin_graphics_signature_gate_validate_md5(
        "not-an-md5", CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_STANDARD,
        0u, 0u, 0u, 0u, NULL);
    expect_equal("malformed MD5", result,
                 CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_ERR_MD5);

    if (failures != 0) return 1;
    puts("csb_v1_csbwin_graphics_signature_gate: 15/15 PASS");
    return 0;
}
