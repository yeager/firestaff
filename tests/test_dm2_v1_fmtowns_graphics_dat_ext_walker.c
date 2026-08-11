#include "dm2_v1_fmtowns_graphics_dat_ext_walker.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Build a small raw-table blob with the same layout as the real ext_v4/v5
 * files. `auxes` is retained only to keep the test call sites explicit that
 * the old four-byte stride had no second real field. */
static uint8_t *make_blob(uint16_t sig, uint16_t n,
                          const uint16_t *sizes, const uint16_t *auxes,
                          size_t *out_len) {
    (void)auxes;
    size_t payload = 0;
    for (uint16_t i = 0; i < n; ++i) payload += sizes[i];
    size_t total = 6u + 2u * (size_t)n + payload;
    uint8_t *b = (uint8_t *)calloc(1, total);
    b[0] = (uint8_t)sig; b[1] = (uint8_t)(sig >> 8);
    b[2] = (uint8_t)n;   b[3] = (uint8_t)(n >> 8);
    b[4] = (uint8_t)sizes[0]; b[5] = (uint8_t)(sizes[0] >> 8);
    b[6] = (uint8_t)(sizes[0] >> 16); b[7] = (uint8_t)(sizes[0] >> 24);
    for (uint16_t i = 1; i < n; ++i) {
        size_t rp = 8u + (size_t)(i - 1u) * 2u;
        b[rp+0] = (uint8_t)sizes[i]; b[rp+1] = (uint8_t)(sizes[i] >> 8);
    }
    size_t cursor = 6u + 2u * (size_t)n;
    for (uint16_t i = 0; i < n; ++i) {
        memset(b + cursor, (int)(uint8_t)i, sizes[i]);
        cursor += sizes[i];
    }
    *out_len = total;
    return b;
}

static void test_header(void) {
    uint16_t sizes[3] = {10, 20, 30};
    uint16_t auxes[3] = {1, 2, 3};
    size_t len;
    uint8_t *b = make_blob(0x8004, 3, sizes, auxes, &len);
    dm2_v1_fmtowns_graphics_dat_ext_header_t h;
    assert(dm2_v1_fmtowns_graphics_dat_ext_parse_header_pc34(b, len, &h)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK);
    assert(h.signature == 0x8004u);
    assert(h.asset_count == 3u);
    assert(h.header_size == 12u);
    assert(h.payload_offset == 12u);
    assert(h.payload_size == 60u);
    free(b);
}

static void test_bad_sig(void) {
    uint8_t b[16] = { 0x01, 0x80, 0x00, 0x00 };
    dm2_v1_fmtowns_graphics_dat_ext_header_t h;
    assert(dm2_v1_fmtowns_graphics_dat_ext_parse_header_pc34(b, sizeof(b), &h)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_BAD_SIGNATURE);
}

static void test_v5_sig(void) {
    uint16_t sizes[2] = {5, 7};
    uint16_t auxes[2] = {0, 1};
    size_t len;
    uint8_t *b = make_blob(0x8005, 2, sizes, auxes, &len);
    dm2_v1_fmtowns_graphics_dat_ext_header_t h;
    assert(dm2_v1_fmtowns_graphics_dat_ext_parse_header_pc34(b, len, &h)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK);
    assert(h.signature == 0x8005u);
    free(b);
}

static void test_get_record(void) {
    uint16_t sizes[4] = {8, 16, 4, 32};
    uint16_t auxes[4] = {8, 15, 4, 30};
    size_t len;
    uint8_t *b = make_blob(0x8004, 4, sizes, auxes, &len);
    dm2_v1_fmtowns_graphics_dat_ext_record_t r;
    assert(dm2_v1_fmtowns_graphics_dat_ext_get_record_pc34(b, len, 0, &r)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK);
    assert(r.index == 0 && r.stored_size == 8 && r.aux == 0);
    assert(r.payload_offset == 14u);        /* raw table header */
    assert(r.is_directory == 1);            /* raw 0 is the ENT1 directory */
    assert(dm2_v1_fmtowns_graphics_dat_ext_get_record_pc34(b, len, 2, &r)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK);
    assert(r.stored_size == 4 && r.aux == 0);
    assert(r.payload_offset == 14u + 8u + 16u);
    /* Out of range. */
    assert(dm2_v1_fmtowns_graphics_dat_ext_get_record_pc34(b, len, 4, &r)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_BAD_ARGS);
    free(b);
}

static void test_directory_flag(void) {
    uint16_t sizes[2] = {40000, 100};
    uint16_t auxes[2] = {0, 100};
    size_t len;
    uint8_t *b = make_blob(0x8004, 2, sizes, auxes, &len);
    dm2_v1_fmtowns_graphics_dat_ext_record_t r;
    assert(dm2_v1_fmtowns_graphics_dat_ext_get_record_pc34(b, len, 0, &r)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK);
    assert(r.is_directory == 1);
    assert(dm2_v1_fmtowns_graphics_dat_ext_get_record_pc34(b, len, 1, &r)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK);
    assert(r.is_directory == 0);
    free(b);
}

static void test_overrun(void) {
    /* Declare huge size but supply a truncated payload. */
    uint16_t sizes[1] = {1000};
    uint16_t auxes[1] = {1000};
    size_t len;
    uint8_t *b = make_blob(0x8004, 1, sizes, auxes, &len);
    /* Truncate. */
    len -= 500;
    dm2_v1_fmtowns_graphics_dat_ext_record_t r;
    assert(dm2_v1_fmtowns_graphics_dat_ext_get_record_pc34(b, len, 0, &r)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OVERRUN);
    free(b);
}

static int count_visitor(void *user, const dm2_v1_fmtowns_graphics_dat_ext_record_t *r) {
    (void)r; (*(int*)user)++; return 0;
}

static int early_stop_visitor(void *user, const dm2_v1_fmtowns_graphics_dat_ext_record_t *r) {
    (*(int*)user)++;
    return r->index == 1 ? 1 : 0;
}

static void test_walk(void) {
    uint16_t sizes[5] = {10, 20, 30, 40, 50};
    uint16_t auxes[5] = {10, 20, 30, 40, 50};
    size_t len;
    uint8_t *b = make_blob(0x8004, 5, sizes, auxes, &len);
    int c = 0;
    assert(dm2_v1_fmtowns_graphics_dat_ext_walk_pc34(b, len, count_visitor, &c)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK);
    assert(c == 5);
    c = 0;
    dm2_v1_fmtowns_graphics_dat_ext_walk_pc34(b, len, early_stop_visitor, &c);
    assert(c == 2);
    free(b);
}

/* Real-data test: verify DM2 FM Towns GRAPHICS.DAT parses cleanly
 * and every record's payload stays inside the file. */
static int real_visitor(void *user, const dm2_v1_fmtowns_graphics_dat_ext_record_t *r) {
    uint32_t *totals = (uint32_t *)user;
    totals[0] += r->stored_size;
    totals[1] += r->aux;
    if (r->is_directory) totals[2]++;
    return 0;
}
static void test_real_dm2_fmtowns(void) {
    const char *p = getenv("FIRESTAFF_DM2_FMTOWNS_GRAPHICS_DAT");
    if (!p) { puts("SKIP: no DM2 FM Towns GRAPHICS.DAT path"); return; }
    FILE *f = fopen(p, "rb"); if (!f) { puts("SKIP: open"); return; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *b = (uint8_t*)malloc((size_t)sz);
    if (fread(b, 1, sz, f) != (size_t)sz) { free(b); fclose(f); puts("SKIP: read"); return; }
    fclose(f);
    dm2_v1_fmtowns_graphics_dat_ext_header_t h;
    assert(dm2_v1_fmtowns_graphics_dat_ext_parse_header_pc34(b, sz, &h)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK);
    assert(h.signature == 0x8004u);
    assert(h.asset_count == 3407u);
    uint32_t totals[3] = {0, 0, 0};
    assert(dm2_v1_fmtowns_graphics_dat_ext_walk_pc34(b, sz, real_visitor, totals)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK);
    printf("PASS: DM2 FM Towns walked; sum(stored)=%u payload=%u directories=%u\n",
           totals[0], h.payload_size, totals[2]);
    /* Directory count is exactly 1 (asset 0). */
    assert(totals[2] == 1u);
    /* Sum of stored sizes must not exceed payload — it's allowed to
     * fall short (documented ~208 KB gap) but never overrun. */
    assert(totals[0] <= h.payload_size);
    free(b);
}
static void test_real_dm2_dos(void) {
    const char *p = getenv("FIRESTAFF_DM2_DOS_GRAPHICS_DAT");
    if (!p) { puts("SKIP: no DM2 DOS GRAPHICS.DAT path"); return; }
    FILE *f = fopen(p, "rb"); if (!f) { puts("SKIP: open"); return; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *b = (uint8_t*)malloc((size_t)sz);
    if (fread(b, 1, sz, f) != (size_t)sz) { free(b); fclose(f); puts("SKIP: read"); return; }
    fclose(f);
    dm2_v1_fmtowns_graphics_dat_ext_header_t h;
    assert(dm2_v1_fmtowns_graphics_dat_ext_parse_header_pc34(b, sz, &h)
           == DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK);
    assert(h.signature == 0x8005u);
    assert(h.asset_count == 5624u);
    printf("PASS: DM2 DOS header parsed; count=%u payload=%u\n",
           h.asset_count, h.payload_size);
    free(b);
}

int main(void) {
    test_header();
    test_bad_sig();
    test_v5_sig();
    test_get_record();
    test_directory_flag();
    test_overrun();
    test_walk();
    test_real_dm2_fmtowns();
    test_real_dm2_dos();
    puts("All dm2_v1_fmtowns_graphics_dat_ext_walker tests passed.");
    return 0;
}
