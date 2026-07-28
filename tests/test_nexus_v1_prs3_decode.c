#include "nexus_v1_prs3_decode.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int g_pass, g_fail;

static void check(int cond, const char *msg) {
    if (cond) { g_pass++; }
    else { printf("  FAIL: %s\n", msg); g_fail++; }
}

static void test_header_parse(void) {
    Nexus_V1_Prs3Header hdr;
    uint8_t valid[] = {
        'P','R','S','3',
        0,0,0,1,
        0,0,0,8,
        0,0,0,4,
        0xFF, 0x32, 0x35, 0x38
    };

    check(nexus_v1_prs3_parse_header(NULL, 0, &hdr) == 0,
          "NULL data rejected");
    check(nexus_v1_prs3_parse_header(valid, 15, &hdr) == 0,
          "too-short data rejected");
    check(nexus_v1_prs3_parse_header(valid, 20, &hdr) == 1,
          "valid header accepted");
    check(hdr.valid == 1, "header.valid set");
    check(hdr.version == 1, "version == 1");
    check(hdr.uncompressed_size == 8, "uncompressed_size");
    check(hdr.compressed_size == 4, "compressed_size");
    check(hdr.stream == valid + 16, "stream pointer");
}

static void test_decompress_literal_run(void) {
    uint8_t output[8];
    Nexus_V1_Prs3DecodeResult r;
    /* Control byte 0xFF = all 8 bits set = 8 literals */
    uint8_t src[] = { 0xFF, 10, 20, 30, 40, 50, 60, 70, 80 };

    r = nexus_v1_prs3_decompress(src, sizeof(src), output, sizeof(output), 8);
    check(r.success == 1, "literal run succeeds");
    check(r.bytes_produced == 8, "8 bytes produced");
    check(output[0] == 10 && output[7] == 80, "literal values correct");
}

static void test_decompress_copy_ref(void) {
    uint8_t output[16];
    Nexus_V1_Prs3DecodeResult r;
    /* 3 literals then copy from pos 0, len 3 */
    /* offset relative=0 → raw=4078=0xFEE, b1_hi=0xF, b0=0xEE */
    /* len=3 → 3-3=0 → b1_lo=0x0, b1=0xF0 */
    uint8_t src[] = {
        0x07,           /* bits 0,1,2 = 1 (lit), bit 3 = 0 (copy), bits 4-7 = 0 */
        0xAA, 0xBB, 0xCC,
        0xEE, 0xF0,     /* copy: raw=0x0FEE, offset=0 relative, len=3 */
    };
    r = nexus_v1_prs3_decompress(src, sizeof(src), output, sizeof(output), 6);
    check(r.success == 1, "copy ref: success");
    check(r.bytes_produced == 6, "copy ref: 6 bytes");
    check(output[0] == 0xAA && output[1] == 0xBB && output[2] == 0xCC,
          "copy ref: literals correct");
    check(output[3] == 0xAA && output[4] == 0xBB && output[5] == 0xCC,
          "copy ref: copied bytes match");
}

int main(void) {
    printf("=== Nexus V1 PRS3 Decode test ===\n");
    test_header_parse();
    test_decompress_literal_run();
    test_decompress_copy_ref();
    printf("=== %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
