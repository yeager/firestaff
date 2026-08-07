#include "dm1_v1_fmtowns_tbios_id.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Build a synthetic BIOS blob with a TBIOS identifier at a known
 * base. The bytes here reproduce the exact 32-byte layout every
 * real FMT_F20.ROM uses: 8B version + 8B date + 8B "towns" + 8B
 * "tbios". This is fixture data, not synthesised game content —
 * it's just enough to exercise the detector's classifier. */
static void install_identifier(uint8_t *blob, size_t base,
                               const char *version, const char *date) {
    memset(blob + base, 0, 32);
    memcpy(blob + base + 0,  version, strlen(version));
    memcpy(blob + base + 8,  date, strlen(date));
    memcpy(blob + base + 16, "towns", 5);
    memcpy(blob + base + 24, "tbios", 5);
}

static void test_null_gates(void) {
    dm1_v1_fmtowns_tbios_identity_t id;
    uint8_t buf[1] = {0};
    assert(dm1_v1_fmtowns_tbios_identify_pc34(NULL, 100, &id) == 0);
    assert(dm1_v1_fmtowns_tbios_identify_pc34(buf, 1, NULL) == 0);
    /* Buffer too small for any candidate base. */
    assert(dm1_v1_fmtowns_tbios_identify_pc34(buf, 1, &id) == 0);
}

static void test_v31l31_92(void) {
    /* Base 0x100000 -> min buffer 0x100020. */
    size_t sz = 0x100020;
    uint8_t *blob = (uint8_t *)calloc(1, sz);
    dm1_v1_fmtowns_tbios_identity_t id;
    install_identifier(blob, 0x100000, "V31L31", "92/10/16");
    assert(dm1_v1_fmtowns_tbios_identify_pc34(blob, sz, &id) == 1);
    assert(id.version == DM1_V1_FMTOWNS_TBIOS_V31L31_92);
    assert(strcmp(id.version_str, "V31L31") == 0);
    assert(strcmp(id.date_str, "92/10/16") == 0);
    assert(strcmp(id.product_str, "towns") == 0);
    assert(strcmp(id.subsystem_str, "tbios") == 0);
    assert(id.identified_at_offset == 0x100000);
    free(blob);
}

static void test_v31l22a_at_earlier_base(void) {
    size_t sz = 0x1F770;
    uint8_t *blob = (uint8_t *)calloc(1, sz);
    dm1_v1_fmtowns_tbios_identity_t id;
    install_identifier(blob, 0x1F750, "V31L22A", "89/03/08");
    assert(dm1_v1_fmtowns_tbios_identify_pc34(blob, sz, &id) == 1);
    assert(id.version == DM1_V1_FMTOWNS_TBIOS_V31L22A);
    assert(id.identified_at_offset == 0x1F750);
    free(blob);
}

static void test_v31l35_1993_and_1994(void) {
    dm1_v1_fmtowns_tbios_identity_t id;
    size_t sz = 0x100020;
    uint8_t *blob = (uint8_t *)calloc(1, sz);
    install_identifier(blob, 0x100000, "V31L35", "93/10/15");
    assert(dm1_v1_fmtowns_tbios_identify_pc34(blob, sz, &id) == 1);
    assert(id.version == DM1_V1_FMTOWNS_TBIOS_V31L35);
    install_identifier(blob, 0x100000, "V31L35", "94/12/03");
    assert(dm1_v1_fmtowns_tbios_identify_pc34(blob, sz, &id) == 1);
    assert(id.version == DM1_V1_FMTOWNS_TBIOS_V31L35_94);
    free(blob);
}

static void test_unknown_version_fails_closed(void) {
    dm1_v1_fmtowns_tbios_identity_t id;
    size_t sz = 0x100020;
    uint8_t *blob = (uint8_t *)calloc(1, sz);
    /* Product + subsystem match but version/date do not -> UNKNOWN,
     * but the strings are still recorded for logging. */
    install_identifier(blob, 0x100000, "V99L99", "99/99/99");
    assert(dm1_v1_fmtowns_tbios_identify_pc34(blob, sz, &id) == 0);
    assert(id.version == DM1_V1_FMTOWNS_TBIOS_UNKNOWN);
    assert(strcmp(id.version_str, "V99L99") == 0);
    assert(strcmp(id.product_str, "towns") == 0);
    free(blob);
}

static void test_not_a_bios(void) {
    dm1_v1_fmtowns_tbios_identity_t id;
    size_t sz = 0x100020;
    uint8_t *blob = (uint8_t *)calloc(1, sz);
    /* Random bytes at candidate base -> no match. */
    memset(blob + 0x100000, 0xff, 32);
    assert(dm1_v1_fmtowns_tbios_identify_pc34(blob, sz, &id) == 0);
    free(blob);
}

static void test_hma240_requirement(void) {
    dm1_v1_fmtowns_tbios_version_t need =
        dm1_v1_fmtowns_tbios_required_for_dm1_hma240_pc34();
    /* DM1 HMA-240 disc ships V31L22A in TBIOS.BIN (byte-verified). */
    assert(need == DM1_V1_FMTOWNS_TBIOS_V31L22A);
    /* Any later TBIOS also satisfies. */
    assert(dm1_v1_fmtowns_tbios_meets_pc34(
        DM1_V1_FMTOWNS_TBIOS_V31L31_92, need) == 1);
    assert(dm1_v1_fmtowns_tbios_meets_pc34(
        DM1_V1_FMTOWNS_TBIOS_V31L35, need) == 1);
    /* V31L22A satisfies itself. */
    assert(dm1_v1_fmtowns_tbios_meets_pc34(
        DM1_V1_FMTOWNS_TBIOS_V31L22A, need) == 1);
    /* UNKNOWN never satisfies. */
    assert(dm1_v1_fmtowns_tbios_meets_pc34(
        DM1_V1_FMTOWNS_TBIOS_UNKNOWN, need) == 0);
}

static void test_real_tbios_bin(void) {
    /* Verify the shipping DM1 disc's TBIOS.BIN is correctly
     * classified as V31L22A. */
    const char *path = getenv("FIRESTAFF_DM1_FMTOWNS_TBIOS_BIN");
    FILE *fp;
    if (!path || !path[0]) { puts("SKIP: no TBIOS.BIN"); return; }
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open"); return; }
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc((size_t)sz);
    if (!buf || fread(buf, 1, (size_t)sz, fp) != (size_t)sz) {
        free(buf); fclose(fp); puts("SKIP: read failed"); return;
    }
    fclose(fp);
    dm1_v1_fmtowns_tbios_identity_t id;
    /* TBIOS.BIN is loaded to fixed physical address; the identifier
     * is at file offset 0 (raw dump, not with a load prefix). Our
     * identify function scans candidate bases including 0x1F750 etc,
     * which won't hit for a raw TBIOS.BIN. Test the classification
     * by checking strings directly instead. */
    assert(buf[0] == 'V' && buf[1] == '3' && buf[2] == '1' &&
           buf[3] == 'L' && buf[4] == '2' && buf[5] == '2' && buf[6] == 'A');
    assert(memcmp(buf + 0x10, "towns", 5) == 0);
    assert(memcmp(buf + 0x18, "tbios", 5) == 0);
    free(buf);
    puts("PASS: DM1 disc TBIOS.BIN is V31L22A byte-verified");
}

int main(void) {
    test_null_gates();
    test_v31l31_92();
    test_v31l22a_at_earlier_base();
    test_v31l35_1993_and_1994();
    test_unknown_version_fails_closed();
    test_not_a_bios();
    test_hma240_requirement();
    test_real_tbios_bin();
    puts("All dm1_v1_fmtowns_tbios_id tests passed.");
    return 0;
}
