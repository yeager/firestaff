/*
 * test_firestaff_ftl_decode.c
 *
 * Test driver for firestaff_ftl_decode.c. The library has a
 * FirestaffFtl_SelfTest() that runs all internal cases; this file
 * is a thin main() that just calls SelfTest and reports pass/fail.
 *
 * The internal self-tests cover:
 *   - Parse: minimal valid FTL container with BSS + DATA + CODE segments
 *   - Parse: bad magic, bad header invariants (Unknown1/c_6/c_7),
 *            truncated input, segment type+id collision, out-of-bounds
 *   - Decode HUNK_CODE: short-dict reference (nibble 0..7, 8-bit index)
 *   - Decode HUNK_CODE: long-dict reference (nibble 8..E, 12-bit index)
 *   - Decode HUNK_CODE: literal escape (nibble 0xF, 4 nibbles -> 2 bytes)
 *   - Decode HUNK_CODE: bad 0x5223 signature, truncated input, zero
 *            word_count, mixed round-trip
 *   - Decode HUNK_DATA: Greatstone Note 7 literal pairs, zero-run
 *            block, empty input, truncated run header, odd input
 *
 * This driver also runs a cross-module checksum bridge: a synthetic
 * FTL container with a compressed 0x5223 CODE hunk is parsed through
 * firestaff_ftl_container.c, checksum-verified as on-disk bytes, then
 * decoded through the public FirestaffFtl_Decode() API and checked
 * against a known decompressed byte stream.
 *
 * Build:
 *   cc -std=c99 -I include tests/test_firestaff_ftl_decode.c \
 *      src/shared/firestaff_ftl_decode.c \
 *      src/shared/firestaff_ftl_hunk_data_zero_run.c \
 *      -o test_firestaff_ftl_decode
 */

#include "firestaff_ftl_decode.h"
#include "firestaff_ftl_container.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

static void wr16_be(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xffu);
}

static void wr32_be(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xffu);
    p[2] = (uint8_t)((v >> 8) & 0xffu);
    p[3] = (uint8_t)(v & 0xffu);
}

static void set_table_word(uint8_t* table, size_t idx, uint16_t word) {
    wr16_be(table + idx * 2u, word);
}

static size_t build_checksum_bridge_ftl(uint8_t* out, size_t cap) {
    const size_t hunk_count = 3u;
    const size_t bss_off = 20u + hunk_count * 12u;
    const size_t bss_size = 40u;
    const size_t data_off = bss_off + bss_size;
    const size_t data_size = 8u;
    const size_t code_off = data_off + data_size;
    const size_t code_size = 3848u + 5u;
    const size_t total = code_off + code_size;
    uint8_t* h;
    uint8_t* bss;
    uint8_t* code;
    uint8_t* table;

    if (!out || total > cap) return 0u;
    memset(out, 0, total);

    wr16_be(out + 0u, FIRESTAFF_FTL_CONTAINER_MAGIC);
    wr16_be(out + 4u, 0x0002u);
    out[6] = 0x01u;
    out[11] = 0x01u;
    out[12] = 0x04u;
    out[13] = 0x01u;
    wr16_be(out + 18u, (uint16_t)hunk_count);

    h = out + 20u;
    wr16_be(h + 0u, FIRESTAFF_FTL_HUNK_BSS);
    wr32_be(h + 4u, (uint32_t)bss_off);
    wr32_be(h + 8u, (uint32_t)bss_size);

    h += 12u;
    wr16_be(h + 0u, FIRESTAFF_FTL_HUNK_DATA);
    wr32_be(h + 4u, (uint32_t)data_off);
    wr32_be(h + 8u, (uint32_t)data_size);

    h += 12u;
    wr16_be(h + 0u, FIRESTAFF_FTL_HUNK_CODE);
    wr32_be(h + 4u, (uint32_t)code_off);
    wr32_be(h + 8u, (uint32_t)code_size);

    memcpy(out + data_off, "ABCDEFGH", data_size);

    code = out + code_off;
    wr16_be(code + 0u, 0x5223u);
    wr16_be(code + 2u, 0x0000u);
    wr32_be(code + 4u, 3u);
    table = code + 8u;
    set_table_word(table, 0x12u, 0x1234u);
    set_table_word(table, 128u, 0xABCDu);

    /* ReDMCSB DECOMPCO.C F0913 lines 63-84:
     * short-dict 0x12, literal 0xCAFE, long-dict 0x800 -> table[128].
     * This is the FTL HUNK_CODE call shape from lines 47-50:
     * table at +8, nibble stream at +3848, word count at +4. */
    code[3848u] = 0x12u;
    code[3849u] = 0xFCu;
    code[3850u] = 0xAFu;
    code[3851u] = 0xE8u;
    code[3852u] = 0x00u;

    bss = out + bss_off;
    wr32_be(bss + 4u, (uint32_t)data_size);
    wr32_be(bss + 20u, 6u);
    wr32_be(bss + 24u, (uint32_t)data_size);
    wr16_be(bss + 36u, FirestaffFtlContainer_ByteChecksum(code, code_size));
    wr16_be(bss + 38u, FirestaffFtlContainer_ByteChecksum(out + data_off,
                                                          data_size));
    wr16_be(bss + 34u, FirestaffFtlContainer_BssChecksum(bss, bss_size));
    wr16_be(out + 2u, FirestaffFtlContainer_CommonChecksum(out, total,
                                                           hunk_count));
    return total;
}

static int checksum_bridge_test(void) {
    static const uint8_t expected_code[] = {
        0x12u, 0x34u, 0xCAu, 0xFEu, 0xABu, 0xCDu
    };
    uint8_t buf[8192];
    size_t size = build_checksum_bridge_ftl(buf, sizeof(buf));
    FirestaffFtlContainer container;
    FirestaffFtlChecksumReport report;
    FirestaffFtl decoded;
    uint16_t decoded_checksum;

    if (size == 0u) {
        fprintf(stderr, "checksum_bridge_test: synthetic build failed\n");
        return -1;
    }

    if (FirestaffFtlContainer_Parse(buf, size, &container) != 0 ||
        FirestaffFtlContainer_VerifyChecksums(buf, size, &container,
                                             &report) != 0) {
        fprintf(stderr, "checksum_bridge_test: container parse/checksum failed\n");
        return -1;
    }
    if (report.common_header.status != FIRESTAFF_FTL_CHECK_MATCH ||
        report.bss_hunk.status != FIRESTAFF_FTL_CHECK_MATCH ||
        report.data_hunk.status != FIRESTAFF_FTL_CHECK_MATCH ||
        report.code_hunk.status != FIRESTAFF_FTL_CHECK_MATCH) {
        fprintf(stderr, "checksum_bridge_test: compressed CODE checksum mismatch\n");
        return -1;
    }

    memset(&decoded, 0, sizeof(decoded));
    if (FirestaffFtl_Parse(buf, size, &decoded) != 0 ||
        FirestaffFtl_Decode(&decoded) != 0) {
        fprintf(stderr, "checksum_bridge_test: public decode failed\n");
        FirestaffFtl_Free(&decoded);
        return -1;
    }
    if (decoded.hunk_data_decoded_size != 8u ||
        memcmp(decoded.hunk_data_decoded, "ABCDEFGH", 8u) != 0 ||
        decoded.hunk_code_size != sizeof(expected_code) ||
        memcmp(decoded.hunk_code, expected_code, sizeof(expected_code)) != 0) {
        fprintf(stderr, "checksum_bridge_test: decoded bytes differ\n");
        FirestaffFtl_Free(&decoded);
        return -1;
    }
    decoded_checksum = FirestaffFtlContainer_ByteChecksum(decoded.hunk_code,
                                                         decoded.hunk_code_size);
    if (decoded_checksum != 0x0386u) {
        fprintf(stderr, "checksum_bridge_test: decoded checksum 0x%04x\n",
                decoded_checksum);
        FirestaffFtl_Free(&decoded);
        return -1;
    }
    FirestaffFtl_Free(&decoded);

    buf[size - 1u] ^= 0x01u;
    if (FirestaffFtlContainer_Parse(buf, size, &container) != 0 ||
        FirestaffFtlContainer_VerifyChecksums(buf, size, &container,
                                             &report) != 0 ||
        report.code_hunk.status != FIRESTAFF_FTL_CHECK_MISMATCH) {
        fprintf(stderr, "checksum_bridge_test: CODE corruption not detected\n");
        return -1;
    }
    return 0;
}

int main(void) {
    int rc = FirestaffFtl_SelfTest();
    int bridge_rc = checksum_bridge_test();
    if (rc == 0 && bridge_rc == 0) {
        printf("test_firestaff_ftl_decode: PASS\n");
        return 0;
    }
    printf("test_firestaff_ftl_decode: FAIL\n");
    return 1;
}
