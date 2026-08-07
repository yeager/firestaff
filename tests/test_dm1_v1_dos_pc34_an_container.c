#include "dm1_v1_dos_pc34_an_container.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t FIXED_HEADER[14] = {
    'A','N', 0x00,0x08, 0x00,0x00, 0x01,0x40,
    0x00,0xc8, 0x00,0x04, 0x00,0x03
};

static void test_header_ok(void) {
    dm1_v1_dos_pc34_an_header_t h;
    assert(dm1_v1_dos_pc34_an_parse_header_pc34(
        FIXED_HEADER, sizeof(FIXED_HEADER), &h) == DM1_V1_DOS_PC34_AN_OK);
    assert(h.width == 320 && h.height == 200 && h.planes == 4 && h.subtype == 3);
}

static void test_header_bad_sig(void) {
    uint8_t bad[14]; memcpy(bad, FIXED_HEADER, 14); bad[0] = 'X';
    dm1_v1_dos_pc34_an_header_t h;
    assert(dm1_v1_dos_pc34_an_parse_header_pc34(bad, 14, &h)
           == DM1_V1_DOS_PC34_AN_BAD_SIGNATURE);
}

static void test_header_bad_geom(void) {
    uint8_t bad[14]; memcpy(bad, FIXED_HEADER, 14);
    bad[6] = 0x02; /* width = 0x0240 */
    dm1_v1_dos_pc34_an_header_t h;
    assert(dm1_v1_dos_pc34_an_parse_header_pc34(bad, 14, &h)
           == DM1_V1_DOS_PC34_AN_BAD_GEOMETRY);
}

static void test_header_too_small(void) {
    dm1_v1_dos_pc34_an_header_t h;
    assert(dm1_v1_dos_pc34_an_parse_header_pc34(FIXED_HEADER, 5, &h)
           == DM1_V1_DOS_PC34_AN_TOO_SMALL);
}

static void test_find_next_tag_synthetic(void) {
    /* Build a buffer with tags at known positions. */
    uint8_t buf[64] = {0};
    memcpy(buf + 4,  "BR", 2);
    memcpy(buf + 20, "PL", 2);
    memcpy(buf + 40, "EN", 2);
    /* Also put a non-tag pair that looks vaguely close. */
    memcpy(buf + 30, "ZZ", 2);
    uint32_t off; char tag[3];
    assert(dm1_v1_dos_pc34_an_find_next_tag_pc34(buf, 64, 0, &off, tag) == 1);
    assert(off == 4 && strcmp(tag, "BR") == 0);
    assert(dm1_v1_dos_pc34_an_find_next_tag_pc34(buf, 64, 5, &off, tag) == 1);
    assert(off == 20 && strcmp(tag, "PL") == 0);
    assert(dm1_v1_dos_pc34_an_find_next_tag_pc34(buf, 64, 21, &off, tag) == 1);
    assert(off == 40 && strcmp(tag, "EN") == 0);
    assert(dm1_v1_dos_pc34_an_find_next_tag_pc34(buf, 64, 41, &off, tag) == 0);
}

static void test_real_title(void) {
    const char *p = getenv("FIRESTAFF_DM1_DOS_PC34_TITLE");
    if (!p) { puts("SKIP: no TITLE path"); return; }
    FILE *f = fopen(p, "rb"); if (!f) { puts("SKIP: open"); return; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *b = (uint8_t*)malloc((size_t)sz);
    if (fread(b, 1, sz, f) != (size_t)sz) { free(b); fclose(f); puts("SKIP"); return; }
    fclose(f);
    dm1_v1_dos_pc34_an_header_t h;
    assert(dm1_v1_dos_pc34_an_parse_header_pc34(b, sz, &h) == DM1_V1_DOS_PC34_AN_OK);
    assert(h.width == 320 && h.height == 200 && h.planes == 4);
    /* Byte-verified tag offsets in TITLE (12002 bytes): BR@14,
     * P8@20, PL@42, EN@114, PL@6324, EN@6396. Note P8 at offset 20
     * — the 0x50 0x38 pair is a real "P8" chunk marker, not a
     * coincidental overlap with BR's payload byte. */
    uint32_t off; char tag[3];
    assert(dm1_v1_dos_pc34_an_find_next_tag_pc34(b, sz, 14, &off, tag) == 1);
    assert(off == 14 && strcmp(tag, "BR") == 0);
    assert(dm1_v1_dos_pc34_an_find_next_tag_pc34(b, sz, 16, &off, tag) == 1);
    assert(off == 20 && strcmp(tag, "P8") == 0);
    assert(dm1_v1_dos_pc34_an_find_next_tag_pc34(b, sz, 22, &off, tag) == 1);
    assert(off == 42 && strcmp(tag, "PL") == 0);
    assert(dm1_v1_dos_pc34_an_find_next_tag_pc34(b, sz, 44, &off, tag) == 1);
    assert(off == 114 && strcmp(tag, "EN") == 0);
    /* Second frame. */
    assert(dm1_v1_dos_pc34_an_find_next_tag_pc34(b, sz, 116, &off, tag) == 1);
    assert(off == 6324 && strcmp(tag, "PL") == 0);
    assert(dm1_v1_dos_pc34_an_find_next_tag_pc34(b, sz, 6326, &off, tag) == 1);
    assert(off == 6396 && strcmp(tag, "EN") == 0);
    puts("PASS: TITLE header + tag offsets byte-verified");
    free(b);
}

static void test_real_end(void) {
    const char *p = getenv("FIRESTAFF_DM1_DOS_PC34_END");
    if (!p) { puts("SKIP: no END path"); return; }
    FILE *f = fopen(p, "rb"); if (!f) { puts("SKIP: open"); return; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *b = (uint8_t*)malloc((size_t)sz);
    if (fread(b, 1, sz, f) != (size_t)sz) { free(b); fclose(f); puts("SKIP"); return; }
    fclose(f);
    dm1_v1_dos_pc34_an_header_t h;
    assert(dm1_v1_dos_pc34_an_parse_header_pc34(b, sz, &h) == DM1_V1_DOS_PC34_AN_OK);
    /* END should contain multiple TD entries and at least one PL/EN. */
    int td = 0, pl = 0, en = 0;
    uint32_t off = 14; char tag[3];
    while (dm1_v1_dos_pc34_an_find_next_tag_pc34(b, sz, off, &off, tag)) {
        if      (strcmp(tag, "TD") == 0) ++td;
        else if (strcmp(tag, "PL") == 0) ++pl;
        else if (strcmp(tag, "EN") == 0) ++en;
        ++off; /* advance past the tag we just found */
    }
    printf("PASS: END scan; TD=%d PL=%d EN=%d\n", td, pl, en);
    assert(td >= 1);
    assert(pl >= 1);
    assert(en >= 1);
    free(b);
}

int main(void) {
    test_header_ok();
    test_header_bad_sig();
    test_header_bad_geom();
    test_header_too_small();
    test_find_next_tag_synthetic();
    test_real_title();
    test_real_end();
    puts("All dm1_v1_dos_pc34_an_container tests passed.");
    return 0;
}
