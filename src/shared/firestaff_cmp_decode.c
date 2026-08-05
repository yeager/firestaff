/*
 * firestaff_cmp_decode.c
 *
 * Implementation of the CSB Utility Disk .CMP decoder
 * declared in firestaff_cmp_decode.h.
 *
 * The decoder is a thin parser over the 508-byte on-disk
 * format. It does not perform Amiga <-> Atari ST bitplane
 * conversion; that lives in ReDMCSB PORTRAIT.C and will be
 * reimplemented in Firestaff only when CSB V1 portrait
 * rendering needs it (Tier 3 work).
 *
 * Style:
 *   - Modern C99, no COMPILE.H dependency.
 *   - Read-only: no allocation, no mutation of input.
 *   - All validation explicit; no UB on bad input.
 */

#include "firestaff_cmp_decode.h"

#include <stdio.h>
#include <string.h>

/* ── Big-endian reads (Atari ST is big-endian) ─── */

static uint16_t rd16_be(const uint8_t* p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

/* ── Validation helpers ─────────────────────────────────────── */

static int is_valid_name_char(uint8_t c) {
    /* Uppercase A-Z, space, or null terminator. */
    return c == 0 ||
           (c >= 'A' && c <= 'Z') ||
           c == ' ' ||
           (c >= '0' && c <= '9');
}

/* ── Decoder ─────────────────────────────────────────────────── */

int FirestaffCmp_Decode(const uint8_t* data, size_t data_size,
                         FirestaffCmp* out) {
    if (!data || !out) return -1;
    memset(out, 0, sizeof(*out));

    if (data_size != FIRESTAFF_CMP_FILE_SIZE) return -1;

    /* ReDMCSB DEFS.H CMP and CEDT001.C F7000 write this complete 44-byte
       header in big-endian Atari ST order. CEDT001 validates Magic, ui_6,
       and ui_8's high bit before allowing a portrait to be used. */
    out->magic = rd16_be(data + 0);
    out->dungeon_id = rd16_be(data + 2);
    out->platform = rd16_be(data + 4);
    out->cmp_ui_6 = rd16_be(data + 6);
    out->cmp_ui_8 = rd16_be(data + 8);
    out->cmp_i_A = rd16_be(data + 10);
    out->cmp_i_C = rd16_be(data + 12);
    out->cmp_i_E = rd16_be(data + 14);
    if (out->magic != 0x91a7u || out->cmp_ui_6 != 1u ||
        (out->cmp_ui_8 & 0x8000u) != 0u || out->cmp_i_A != 0u ||
        out->cmp_i_C != 0u || out->cmp_i_E != 0u) return -2;

    /* Name[8] -- copy into the struct (zero-padded) */
    memcpy(out->name, data + FIRESTAFF_CMP_NAME_OFFSET, FIRESTAFF_CMP_NAME_SIZE);
    out->name[FIRESTAFF_CMP_NAME_SIZE - 1] = '\0';
    for (size_t i = 0; i < FIRESTAFF_CMP_NAME_SIZE; ++i) {
        if (!is_valid_name_char((uint8_t)out->name[i])) return -3;
    }

    /* Title[20] -- copy into the struct (zero-padded) */
    memcpy(out->title, data + FIRESTAFF_CMP_TITLE_OFFSET,
           FIRESTAFF_CMP_TITLE_SIZE);
    out->title[FIRESTAFF_CMP_TITLE_SIZE - 1] = '\0';
    for (size_t i = 0; i < FIRESTAFF_CMP_TITLE_SIZE; ++i) {
        if (!is_valid_name_char((uint8_t)out->title[i])) return -3;
    }

    /* Portrait[464] -- pointer into caller's buffer */
    out->portrait = data + FIRESTAFF_CMP_PORTRAIT_OFFSET;
    out->portrait_size = FIRESTAFF_CMP_PORTRAIT_BYTES;

    return 0;
}

/* ── Self-tests ──────────────────────────────────────────────── */

/*
 * Build a minimal but valid CMP file in a heap buffer.
 * Fills name/title with uppercase ASCII, fills portrait
 * with a recognisable pattern, and sets the two reserved
 * 16-bit words to 0.
 */
static int build_cmp(uint8_t* buf, size_t cap,
                     const char* name, const char* title,
                     uint8_t portrait_fill) {
    if (cap < FIRESTAFF_CMP_FILE_SIZE) return -1;
    memset(buf, 0, FIRESTAFF_CMP_FILE_SIZE);

    /* ReDMCSB CEDT001.C F7000 writes this source compatibility header. */
    buf[0] = 0x91; buf[1] = 0xa7;
    buf[7] = 1;
    buf[9] = 1;

    /* Name: zero-padded uppercase ASCII, max 7 chars + NUL. */
    if (name) {
        size_t n = strlen(name);
        if (n > FIRESTAFF_CMP_NAME_SIZE) n = FIRESTAFF_CMP_NAME_SIZE;
        memcpy(buf + FIRESTAFF_CMP_NAME_OFFSET, name, n);
    }

    /* Title: zero-padded uppercase ASCII, max 19 chars + NUL. */
    if (title) {
        size_t n = strlen(title);
        if (n > FIRESTAFF_CMP_TITLE_SIZE) n = FIRESTAFF_CMP_TITLE_SIZE;
        memcpy(buf + FIRESTAFF_CMP_TITLE_OFFSET, title, n);
    }

    /* Portrait: fill with a recognisable byte. */
    memset(buf + FIRESTAFF_CMP_PORTRAIT_OFFSET,
           portrait_fill, FIRESTAFF_CMP_PORTRAIT_BYTES);

    return 0;
}

#define ST_FAIL(msg) do {                                       \
    fprintf(stderr, "test_firestaff_cmp_decode FAIL: %s\n", msg);\
    return 0;                                                    \
} while (0)

#define ST_ASSERT(cond, msg) do {                                \
    if (!(cond)) { fprintf(stderr, "%s:%d: %s (%s)\n",            \
                            __FILE__, __LINE__, msg, #cond);     \
                   return 0; }                                   \
} while (0)

static int test_valid_cmp(void) {
    uint8_t buf[FIRESTAFF_CMP_FILE_SIZE];
    int rc = build_cmp(buf, sizeof(buf), "HECTOR", "WARROR", 0x55);
    if (rc != 0) ST_FAIL("build_cmp");

    FirestaffCmp cmp;
    rc = FirestaffCmp_Decode(buf, sizeof(buf), &cmp);
    if (rc != 0) ST_FAIL("decode rc != 0");
    ST_ASSERT(cmp.magic == 0x91a7u, "magic");
    ST_ASSERT(cmp.cmp_i_C == 0, "cmp_i_C");
    ST_ASSERT(cmp.cmp_i_E == 0, "cmp_i_E");
    ST_ASSERT(strcmp(cmp.name, "HECTOR") == 0, "name match");
    ST_ASSERT(strcmp(cmp.title, "WARROR") == 0, "title match");
    ST_ASSERT(cmp.portrait_size == 464, "portrait size");
    ST_ASSERT(cmp.portrait != NULL, "portrait pointer");
    ST_ASSERT(cmp.portrait[0] == 0x55, "portrait fill[0]");
    ST_ASSERT(cmp.portrait[463] == 0x55, "portrait fill[463]");
    /* The portrait should point INTO our buf. */
    ST_ASSERT(cmp.portrait >= buf && cmp.portrait < buf + FIRESTAFF_CMP_FILE_SIZE,
              "portrait points into input buffer");
    return 1;
}

static int test_too_short(void) {
    uint8_t buf[10] = {0};
    FirestaffCmp cmp;
    int rc = FirestaffCmp_Decode(buf, sizeof(buf), &cmp);
    ST_ASSERT(rc == -1, "too short should fail");
    return 1;
}

static int test_bad_magic(void) {
    uint8_t buf[FIRESTAFF_CMP_FILE_SIZE];
    build_cmp(buf, sizeof(buf), "HECTOR", "WARROR", 0x55);
    /* Corrupt Magic: set to 0x4242 big-endian. */
    buf[0] = 0x42;
    buf[1] = 0x42;

    FirestaffCmp cmp;
    int rc = FirestaffCmp_Decode(buf, sizeof(buf), &cmp);
    ST_ASSERT(rc == -2, "bad magic should fail with -2");
    return 1;
}

static int test_bad_name(void) {
    uint8_t buf[FIRESTAFF_CMP_FILE_SIZE];
    /* Lowercase character 'a' is not in our accepted set. */
    build_cmp(buf, sizeof(buf), "heCTOR", "WARROR", 0x55);

    FirestaffCmp cmp;
    int rc = FirestaffCmp_Decode(buf, sizeof(buf), &cmp);
    ST_ASSERT(rc == -3, "lowercase name should fail with -3");
    return 1;
}

static int test_bad_title(void) {
    uint8_t buf[FIRESTAFF_CMP_FILE_SIZE];
    /* Control char in title. */
    build_cmp(buf, sizeof(buf), "HECTOR", "WAR\x01ROR", 0x55);

    FirestaffCmp cmp;
    int rc = FirestaffCmp_Decode(buf, sizeof(buf), &cmp);
    ST_ASSERT(rc == -3, "control char in title should fail with -3");
    return 1;
}

static int test_max_lengths(void) {
    /* Maximum-length name (7 chars) and title (19 chars). */
    uint8_t buf[FIRESTAFF_CMP_FILE_SIZE];
    int rc = build_cmp(buf, sizeof(buf), "ABCDEFG", "TITLENAMETITLENAME1", 0xAA);
    if (rc != 0) ST_FAIL("build_cmp");

    FirestaffCmp cmp;
    rc = FirestaffCmp_Decode(buf, sizeof(buf), &cmp);
    ST_ASSERT(rc == 0, "max-length should parse");
    ST_ASSERT(strcmp(cmp.name, "ABCDEFG") == 0, "name");
    ST_ASSERT(strcmp(cmp.title, "TITLENAMETITLENAME1") == 0, "title");
    return 1;
}

int FirestaffCmp_SelfTest(void) {
    int total = 0, passed = 0;
    #define RUN(name) do { total++; if (name()) passed++; } while (0)
    RUN(test_valid_cmp);
    RUN(test_too_short);
    RUN(test_bad_magic);
    RUN(test_bad_name);
    RUN(test_bad_title);
    RUN(test_max_lengths);
    #undef RUN
    return (passed == total) ? 0 : -1;
}
