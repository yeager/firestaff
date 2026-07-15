#include "csb_v1_csbwin_graphics_signature_gate.h"

#include <stddef.h>
#include <string.h>

static int csb_v1_csbwin_graphics_signature_hex_nibble(char value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

static int csb_v1_csbwin_graphics_signature_parse_md5(
    const char *md5_hex,
    uint8_t digest[16])
{
    size_t index;

    if (!md5_hex || !digest || strlen(md5_hex) != 32u) return -1;
    for (index = 0u; index < 16u; ++index) {
        int high = csb_v1_csbwin_graphics_signature_hex_nibble(
            md5_hex[index * 2u]);
        int low = csb_v1_csbwin_graphics_signature_hex_nibble(
            md5_hex[index * 2u + 1u]);

        if (high < 0 || low < 0) return -1;
        digest[index] = (uint8_t)((high << 4) | low);
    }
    return 0;
}

int csb_v1_csbwin_graphics_signature_gate_validate_md5(
    const char *md5_hex,
    CSB_V1_CSBWinGraphicsSignatureKind kind,
    uint32_t expected_signature1,
    uint32_t expected_signature2,
    uint32_t runtime_signature,
    uint32_t debugging_data,
    CSB_V1_CSBWinGraphicsSignatureReceipt *out_receipt)
{
    uint8_t digest[16];
    uint32_t signature1;
    uint32_t signature2;
    uint32_t folded_signature;
    int check_runtime_signature;
    CSB_V1_CSBWinGraphicsSignatureReceipt receipt;

    if (kind != CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_STANDARD &&
        kind != CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_CUSTOM) {
        return CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_ERR_ARGUMENT;
    }
    if (csb_v1_csbwin_graphics_signature_parse_md5(md5_hex, digest) != 0) {
        return CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_ERR_MD5;
    }

    signature1 = (uint32_t)digest[0] |
                 ((uint32_t)digest[1] << 8) |
                 ((uint32_t)digest[2] << 16) |
                 ((uint32_t)digest[3] << 24);
    signature2 = (uint32_t)digest[4] |
                 ((uint32_t)digest[5] << 8) |
                 ((uint32_t)digest[6] << 16) |
                 ((uint32_t)digest[7] << 24);
    /* CSBWin data.cpp::Signature reserves zero as an invalid first word. */
    if (signature1 == 0u) signature1 = 1u;
    folded_signature = signature1 | signature2;
    folded_signature = (folded_signature | (folded_signature >> 16)) & 0xffffu;
    if (kind == CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_CUSTOM &&
        folded_signature == 0u) {
        folded_signature = 1u;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.actual_signature1 = signature1;
    receipt.actual_signature2 = signature2;
    receipt.actual_folded_signature = folded_signature;
    if ((expected_signature1 | expected_signature2) != 0u) {
        receipt.expected_signature_checked = 1;
        if (signature1 != expected_signature1 ||
            signature2 != expected_signature2) {
            return CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_ERR_EXPECTED;
        }
    }

    check_runtime_signature =
        kind == CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_STANDARD
            ? runtime_signature != 0u && runtime_signature != 0xffffffffu
            : runtime_signature != 0u && debugging_data == 0u;
    if (check_runtime_signature) {
        receipt.runtime_signature_checked = 1;
        if (folded_signature != runtime_signature) {
            return CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_ERR_RUNTIME;
        }
    }
    if (out_receipt) *out_receipt = receipt;
    return CSB_V1_CSBWIN_GRAPHICS_SIGNATURE_GATE_OK;
}
