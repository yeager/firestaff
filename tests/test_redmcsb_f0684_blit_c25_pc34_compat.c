#include "redmcsb_f0684_blit_c25_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failures;
static void expect_byte(const char *name, uint8_t actual, uint8_t expected) { if (actual != expected) { fprintf(stderr, "FAIL: %s (%u != %u)\n", name, (unsigned)actual, (unsigned)expected); ++failures; } }
static void expect_true(const char *name, int actual) { if (!actual) { fprintf(stderr, "FAIL: %s\n", name); ++failures; } }
static void test_flip(int16_t flip, const uint8_t expected[6]) {
    const uint8_t source[] = {0x12U, 0x3FU, 0x45U, 0x6FU}; uint8_t bytes[18];
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {bytes, sizeof(bytes)};
    const RedmcsbF0684BoxPc34Compat box = {1, 3, 1, 2}; size_t i;
    memset(bytes, 0xEE, sizeof(bytes));
    expect_true("F0684", redmcsb_f0684_blit_c25_pc34_compat(source, sizeof(source), &aperture, &box, 0, 0, 3, 5, -1, flip, 0x10U));
    for (i = 0; i < 3U; ++i) { expect_byte("row 1", bytes[7U + i], expected[i]); expect_byte("row 2", bytes[13U + i], expected[3U + i]); }
    expect_byte("outside retained", bytes[6], 0xEEU);
}
int main(void) {
    const uint8_t none[] = {0x11U,0x12U,0x13U,0x14U,0x15U,0x16U};
    const uint8_t horizontal[] = {0x13U,0x12U,0x11U,0x16U,0x15U,0x14U};
    const uint8_t vertical[] = {0x14U,0x15U,0x16U,0x11U,0x12U,0x13U};
    const uint8_t both[] = {0x16U,0x15U,0x14U,0x13U,0x12U,0x11U};
    const uint8_t source[] = {0x10U,0x3FU}; uint8_t bytes[8];
    RedmcsbF0680C25VgaAperturePc34Compat aperture = {bytes, sizeof(bytes)};
    const RedmcsbF0684BoxPc34Compat box = {0,2,0,0};
    test_flip(0, none); test_flip(1, horizontal); test_flip(2, vertical); test_flip(3, both);
    memset(bytes, 0xA5, sizeof(bytes));
    expect_true("transparent", redmcsb_f0684_blit_c25_pc34_compat(source, sizeof(source), &aperture, &box, 0, 0, 3, 3, 0, 0, 0x10U));
    expect_byte("opaque first", bytes[0], 0x11U); expect_byte("transparent retained", bytes[1], 0xA5U); expect_byte("opaque last", bytes[2], 0x13U);
    if (strstr(redmcsb_f0684_blit_source_evidence_pc34(), "IMAGE3.C:831-928") == NULL) { ++failures; }
    if (failures != 0) { return 1; }
    puts("PASSED: ReDMCSB F0684 PC 3.4 C25 packed bitmap blit");
    return 0;
}
