#include "dm1_v1_fmtowns_pharlap_bridge.h"
#include "firestaff_fmtowns_disc.h"
#include "firestaff_zip_extract.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_slot_layout_valid(void) {
    assert(dm1_v1_fmtowns_pharlap_slot_layout_is_valid_pc34() == 1);
    assert(DM1_V1_FMTOWNS_PHARLAP_REALMODE_SELECTOR == 0x110U);
    assert(DM1_V1_FMTOWNS_PHARLAP_SLOT_TBIOS == 0x20U);
    assert(DM1_V1_FMTOWNS_PHARLAP_SLOT_SECONDARY == 0x40U);
    assert(DM1_V1_FMTOWNS_PHARLAP_SLOT_TIMING == 0x48U);
    assert(DM1_V1_FMTOWNS_PHARLAP_SLOT_HARDWARE_INIT == 0x80U);
}

static void test_recovered_profiles(void) {
    assert(dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.slot_tbios == 70);
    assert(dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.slot_secondary == 20);
    assert(dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.slot_timing == 1);
    assert(dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.slot_hardware_init == 2);
    assert(dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.total == 93);

    assert(dm1_v1_fmtowns_pharlap_profile_jdm_exp_pc34.total == 93);
    assert(dm1_v1_fmtowns_pharlap_profile_tmenu_exp_pc34.total == 92);
    /* JDM and EDM have identical profiles. */
    assert(dm1_v1_fmtowns_pharlap_profile_jdm_exp_pc34.slot_tbios ==
           dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.slot_tbios);
    assert(dm1_v1_fmtowns_pharlap_profile_jdm_exp_pc34.slot_secondary ==
           dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.slot_secondary);
}

static void test_scanner_null_gate(void) {
    dm1_v1_fmtowns_pharlap_call_profile_t p;
    uint8_t buf[10] = {0};
    assert(dm1_v1_fmtowns_pharlap_count_call_sites_pc34(NULL, 10, &p) == 0);
    assert(dm1_v1_fmtowns_pharlap_count_call_sites_pc34(buf, 10, NULL) == 0);
    assert(dm1_v1_fmtowns_pharlap_count_call_sites_pc34(buf, 3, &p) == 0);
}

static void test_scanner_empty_image(void) {
    dm1_v1_fmtowns_pharlap_call_profile_t p;
    uint8_t buf[100] = {0};
    assert(dm1_v1_fmtowns_pharlap_count_call_sites_pc34(buf, 100, &p) == 1);
    assert(p.total == 0);
    assert(p.slot_tbios == 0 && p.slot_secondary == 0);
    assert(p.slot_timing == 0 && p.slot_hardware_init == 0);
}

static void test_scanner_counts_canonical_slots(void) {
    dm1_v1_fmtowns_pharlap_call_profile_t p;
    /* Craft an image with 3 * fs:[0x20] and 1 each of the other three
     * slots, plus one non-canonical disp that must NOT count. */
    uint8_t img[128];
    memset(img, 0x90, sizeof(img));  /* NOPs as padding */
    unsigned int o = 0;
    #define PUT_CALL(disp) do { \
        img[o++] = 0x64; img[o++] = 0xff; img[o++] = 0x1d; \
        img[o++] = (uint8_t)((disp) & 0xff); \
        img[o++] = (uint8_t)(((disp) >> 8) & 0xff); \
        img[o++] = (uint8_t)(((disp) >> 16) & 0xff); \
        img[o++] = (uint8_t)(((disp) >> 24) & 0xff); \
    } while (0)
    PUT_CALL(0x20);
    PUT_CALL(0x40);
    PUT_CALL(0x20);
    PUT_CALL(0x48);
    PUT_CALL(0x80);
    PUT_CALL(0x20);
    PUT_CALL(0x1234);  /* non-canonical -> ignored */
    #undef PUT_CALL

    assert(dm1_v1_fmtowns_pharlap_count_call_sites_pc34(img, sizeof(img), &p) == 1);
    assert(p.slot_tbios == 3);
    assert(p.slot_secondary == 1);
    assert(p.slot_timing == 1);
    assert(p.slot_hardware_init == 1);
    assert(p.total == 6);  /* non-canonical was ignored */
}

/* Real-data round trip: scan the caller-supplied EDM.EXP and verify
 * the counts match the shipped profile constants exactly. */
static uint8_t *load_retail_edm(size_t *size_out) {
    const char *path = getenv("FIRESTAFF_DM1_FMTOWNS_EDM_EXP");
    const char *archive = getenv("FIRESTAFF_DM1_FMTOWNS_ARCHIVE");
    FILE *fp = NULL;
    long size = 0;
    uint8_t *buf = NULL, *cue = NULL, *bin = NULL;
    size_t cue_size = 0u, bin_size = 0u;
    char image_member[256];
    FmtownsDiscProbeResult probe;
    const FmtownsIsoEntry *entry;
    *size_out = 0u;
    if (path && path[0]) {
        fp = fopen(path, "rb");
        if (!fp) return NULL;
        fseek(fp, 0, SEEK_END); size = ftell(fp); fseek(fp, 0, SEEK_SET);
        if (size <= 0) { fclose(fp); return NULL; }
        buf = (uint8_t *)malloc((size_t)size);
        if (!buf || fread(buf, 1, (size_t)size, fp) != (size_t)size) {
            free(buf); fclose(fp); return NULL;
        }
        fclose(fp);
        *size_out = (size_t)size;
        return buf;
    }
    if (!archive || !archive[0] ||
        firestaff_zip_extract_by_suffix(archive, ".cue", &cue, &cue_size) != 0 ||
        !cue || !fmtowns_cue_parse_image_member((const char *)cue, cue_size,
                                                 image_member, sizeof(image_member)) ||
        firestaff_zip_extract_by_suffix(archive, image_member, &bin, &bin_size) != 0 ||
        !bin || fmtowns_disc_probe(bin, bin_size, FMTOWNS_SECTOR_2048, &probe) != 0 ||
        !probe.valid || !(entry = fmtowns_disc_find(&probe, "EDM.EXP")) ||
        fmtowns_disc_extract_alloc(bin, bin_size, FMTOWNS_SECTOR_2048, entry,
                                   &buf, size_out) != 0) {
        free(cue); free(bin); free(buf); return NULL;
    }
    free(cue); free(bin);
    return buf;
}

static void test_real_data_edm_profile(void) {
    size_t size;
    uint8_t *buf = load_retail_edm(&size);
    dm1_v1_fmtowns_pharlap_call_profile_t p;
    if (!buf) { puts("SKIP: no EDM.EXP or FM Towns archive"); return; }
    assert(dm1_v1_fmtowns_pharlap_count_call_sites_pc34(
        buf, (unsigned long)size, &p) == 1);
    /* Byte-verified: 70 + 20 + 1 + 2 = 93 total for EDM.EXP. */
    assert(p.slot_tbios == dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.slot_tbios);
    assert(p.slot_secondary == dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.slot_secondary);
    assert(p.slot_timing == dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.slot_timing);
    assert(p.slot_hardware_init == dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.slot_hardware_init);
    assert(p.total == dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34.total);
    free(buf);
    puts("PASS: retail EDM.EXP Phar Lap profile reads from its supplied media in RAM");
}

int main(void) {
    test_slot_layout_valid();
    test_recovered_profiles();
    test_scanner_null_gate();
    test_scanner_empty_image();
    test_scanner_counts_canonical_slots();
    test_real_data_edm_profile();
    puts("All dm1_v1_fmtowns_pharlap_bridge tests passed.");
    return 0;
}
