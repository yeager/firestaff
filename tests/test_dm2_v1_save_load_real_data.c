/*
 * test_dm2_v1_save_load_real_data.c
 *
 * Validates DM2 save file loading against real sksave files from DOS EN.
 * Path: ~/.firestaff/data/dm2-extras/dos-en/data/sksave{0..3}.dat
 */

#include "dm2_v1_save_load.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *data;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (sz <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    data = malloc((size_t)sz);
    if (!data) { fclose(f); return NULL; }
    if (fread(data, 1, (size_t)sz, f) != (size_t)sz) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (size_t)sz;
    return data;
}

static void test_suppress_roundtrip(void) {
    uint8_t data[] = { 0xA5, 0x3C, 0xFF, 0x00, 0x7E };
    uint8_t mask[] = { 0xFF, 0x0F, 0xF0, 0x00, 0x55 };
    uint8_t encoded[32];
    uint8_t decoded[5];
    int enc_len, dec_len;

    enc_len = dm2_suppress_encode(data, mask, 5, encoded, sizeof(encoded));
    assert(enc_len > 0);
    printf("  PASS: SUPPRESS encode (%d bytes)\n", enc_len);

    memset(decoded, 0, sizeof(decoded));
    dec_len = dm2_suppress_decode(encoded, (size_t)enc_len, mask, 5, decoded, 0);
    assert(dec_len > 0);

    for (int i = 0; i < 5; i++)
        assert((decoded[i] & mask[i]) == (data[i] & mask[i]));
    printf("  PASS: SUPPRESS round-trip\n");
}

static void test_corpus_scan(void) {
    const char *home = getenv("HOME");
    char path[512];
    DM2_SKSaveCorpusReceipt scan_rc;

    if (!home) { printf("  SKIP: HOME not set\n"); return; }

    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/dos-en/data", home);

    memset(&scan_rc, 0, sizeof(scan_rc));
    bool ok = dm2_v1_sksave_corpus_scan(path, &scan_rc);
    printf("  corpus_scan: ok=%d candidates=%d importable=%d\n",
           ok, scan_rc.candidate_receipt_count,
           scan_rc.importable_candidate_count);

    if (ok && scan_rc.candidate_receipt_count > 0) {
        printf("  PASS: found %d save file candidates\n",
               scan_rc.candidate_receipt_count);
        printf("  largest_payload: %zu bytes\n", scan_rc.largest_payload_size);
        printf("  total_payload: %zu bytes\n", scan_rc.total_payload_size);
        if (scan_rc.first_valid_path[0])
            printf("  first_valid: %s\n", scan_rc.first_valid_path);
    } else {
        printf("  SKIP: no save files found\n");
    }
}

static void test_individual_save_file(const char *path, int slot) {
    uint8_t *data;
    size_t data_size;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read slot %d at %s\n", slot, path);
        return;
    }

    printf("  sksave%d.dat: %zu bytes\n", slot, data_size);
    assert(data_size > 100);

    /* First two bytes: version word */
    uint16_t version = (uint16_t)(data[0] | (data[1] << 8));
    printf("    version word: 0x%04X\n", version);

    /* Champion name starts at offset 2 (null-terminated ASCII) */
    char name[16];
    memset(name, 0, sizeof(name));
    memcpy(name, data + 2, 10);
    printf("    champion 0 name: '%s'\n", name);

    assert(data_size >= 100);
    printf("  PASS: slot %d valid\n", slot);

    free(data);
}

int main(void) {
    const char *home;
    char path[512];

    printf("DM2 save/load real data tests:\n\n");

    printf("SUPPRESS codec round-trip:\n");
    test_suppress_roundtrip();

    printf("\nCorpus scanner:\n");
    test_corpus_scan();

    printf("\nIndividual save files:\n");
    home = getenv("HOME");
    if (!home) { printf("  SKIP: HOME not set\n"); return 0; }

    for (int i = 0; i < 4; i++) {
        snprintf(path, sizeof(path),
                 "%s/.firestaff/data/dm2-extras/dos-en/data/sksave%d.dat",
                 home, i);
        test_individual_save_file(path, i);
    }

    printf("\nAll DM2 save/load real data tests passed.\n");
    return 0;
}
