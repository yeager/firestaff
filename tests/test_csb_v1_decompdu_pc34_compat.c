/*
 * test_csb_v1_decompdu_pc34_compat.c
 *
 * CSB V1 Compressed Dungeon Support (Dungeon GAP 4,
 * DECOMPDU.C).  Source-locked per ReDMCSB DECOMPDU.C
 * F0455_FLOPPY_DecompressDungeon (MEDIA481 portable C path).
 *
 * Covers:
 *   - detection with valid / invalid container magic
 *   - F0455 encode -> decode round-trip on patterned data
 *   - grid build -> decompress round-trip (8x8 single level
 *     and a multi-level pack), verifying width/height/levels
 *   - error paths: bad sub-format, corrupt/short stream,
 *     truncated bit stream, NULL args, dimension bounds
 */
#include "csb_v1_decompdu_pc34_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Encode `raw` then decode it back and compare. */
static int roundtrip_bytes(const unsigned char* raw, long n) {
    unsigned char* enc = (unsigned char*)malloc((size_t)(n * 2 + 64));
    unsigned char* dec = (unsigned char*)malloc((size_t)(n > 0 ? n : 1));
    long encLen = 0;
    int rc, ok = 0;
    if (!enc || !dec) { free(enc); free(dec); return 0; }
    rc = csb_v1_decompdu_f0455_encode(raw, n, enc, n * 2 + 64, &encLen);
    if (rc == CSB_DECOMPDU_ERR_OK) {
        rc = csb_v1_decompdu_f0455(enc, encLen, dec, n);
        if (rc == CSB_DECOMPDU_ERR_OK && memcmp(raw, dec, (size_t)n) == 0) {
            ok = 1;
        }
    }
    free(enc);
    free(dec);
    return ok;
}

int main(void) {
    printf("=== CSB V1 DECOMPDU (Dungeon GAP 4, F0455) ===\n");

    /* ── Detection ── */
    {
        unsigned char cdu[5] = {'C','D','U','\0', CSB_CDU_SUBFORMAT_F0455};
        CHECK(csb_v1_decompdu_detect(cdu, 5) == 1,
              "valid CDU magic + F0455 sub-format detected");
    }
    {
        unsigned char bad[5] = {'N','O','P','E', 0x01};
        CHECK(csb_v1_decompdu_detect(bad, 5) == 0,
              "invalid magic -> not detected");
        CHECK(csb_v1_decompdu_detect(NULL, 5) == 0, "NULL -> not detected");
        CHECK(csb_v1_decompdu_detect((unsigned char*)"CDU", 3) == 0,
              "len < 4 -> not detected");
    }
    CHECK(csb_v1_decompdu_implemented() == 1,
          "Dungeon GAP 4: F0455 core decompressor now implemented");

    /* ── F0455 encode/decode round-trips ── */
    {
        /* Mostly a few common bytes -> exercises most-common path. */
        unsigned char raw[64];
        int i;
        for (i = 0; i < 64; ++i) raw[i] = (unsigned char)(i & 3);
        CHECK(roundtrip_bytes(raw, 64),
              "round-trip 64 bytes of {0,1,2,3} (most-common path)");
    }
    {
        /* 20 distinct values -> exercises less-common path. */
        unsigned char raw[40];
        int i;
        for (i = 0; i < 40; ++i) raw[i] = (unsigned char)(i % 20);
        CHECK(roundtrip_bytes(raw, 40),
              "round-trip 40 bytes of 20 distinct values (less-common path)");
    }
    {
        /* Full byte spread -> exercises literal path. */
        unsigned char raw[256];
        int i;
        for (i = 0; i < 256; ++i) raw[i] = (unsigned char)i;
        CHECK(roundtrip_bytes(raw, 256),
              "round-trip all 256 byte values (literal path)");
    }
    {
        /* Single byte. */
        unsigned char raw[1] = {0x7F};
        CHECK(roundtrip_bytes(raw, 1), "round-trip single byte");
    }

    /* ── Grid build/decompress round-trip: 8x8 single level ── */
    {
        unsigned char tiles[8 * 8];
        unsigned char container[8 * 8 * 4 + 64];
        long clen = 0;
        CSB_CDUDungeon dn;
        int x, y, rc;
        for (y = 0; y < 8; ++y)
            for (x = 0; x < 8; ++x)
                tiles[y * 8 + x] = (unsigned char)((x ^ y) & 0x07);

        rc = csb_v1_decompdu_build_grid(tiles, 8, 8, 1,
                                        container, sizeof(container), &clen);
        CHECK(rc == CSB_DECOMPDU_ERR_OK, "build 8x8x1 grid container");
        CHECK(csb_v1_decompdu_detect(container, (int)clen) == 1,
              "built container is detected as CDU");

        memset(&dn, 0, sizeof(dn));
        rc = csb_v1_decompdu_decompress_grid(container, clen, &dn);
        CHECK(rc == CSB_DECOMPDU_ERR_OK, "decompress 8x8x1 grid");
        CHECK(dn.width == 8 && dn.height == 8 && dn.levelCount == 1,
              "8x8x1 dimensions round-trip");
        CHECK(dn.tiles && memcmp(dn.tiles, tiles, sizeof(tiles)) == 0,
              "8x8x1 tile data round-trips byte-exact");
        csb_v1_cdu_dungeon_free(&dn);
        CHECK(dn.tiles == NULL, "free() clears tiles pointer");
    }

    /* ── Multi-level pack (3 levels of 16x16) ── */
    {
        int W = 16, H = 16, L = 3;
        long total = (long)W * H * L;
        unsigned char* tiles = (unsigned char*)malloc((size_t)total);
        unsigned char* container = (unsigned char*)malloc((size_t)(total * 4 + 64));
        long clen = 0;
        CSB_CDUDungeon dn;
        int lvl, x, y, rc;
        for (lvl = 0; lvl < L; ++lvl)
            for (y = 0; y < H; ++y)
                for (x = 0; x < W; ++x)
                    tiles[(lvl * H + y) * W + x] =
                        (unsigned char)((lvl * 7 + x + y) & 0x0F);

        rc = csb_v1_decompdu_build_grid(tiles, W, H, L,
                                        container, total * 4 + 64, &clen);
        CHECK(rc == CSB_DECOMPDU_ERR_OK, "build 16x16x3 multi-level pack");

        memset(&dn, 0, sizeof(dn));
        rc = csb_v1_decompdu_decompress_grid(container, clen, &dn);
        CHECK(rc == CSB_DECOMPDU_ERR_OK, "decompress 16x16x3 pack");
        CHECK(dn.levelCount == 3 && dn.width == 16 && dn.height == 16,
              "16x16x3 dimensions round-trip");
        CHECK(dn.tiles && memcmp(dn.tiles, tiles, (size_t)total) == 0,
              "16x16x3 tile data round-trips byte-exact");
        csb_v1_cdu_dungeon_free(&dn);
        free(tiles);
        free(container);
    }

    /* ── Error paths ── */
    {
        unsigned char container[CSB_CDU_HEADER_SIZE + 32];
        unsigned char tiles[4] = {1,2,3,4};
        long clen = 0;
        CSB_CDUDungeon dn;
        int rc;

        rc = csb_v1_decompdu_build_grid(tiles, 2, 2, 1,
                                        container, sizeof(container), &clen);
        CHECK(rc == CSB_DECOMPDU_ERR_OK, "build 2x2x1 for error tests");

        /* Bad sub-format. */
        {
            unsigned char tmp[CSB_CDU_HEADER_SIZE + 32];
            memcpy(tmp, container, (size_t)clen);
            tmp[4] = 0x09; /* unsupported sub-format */
            memset(&dn, 0, sizeof(dn));
            CHECK(csb_v1_decompdu_decompress_grid(tmp, clen, &dn)
                      == CSB_DECOMPDU_ERR_BAD_SUBFORMAT,
                  "bad sub-format -> ERR_BAD_SUBFORMAT");
            CHECK(dn.tiles == NULL, "no allocation on bad sub-format");
        }

        /* Corrupt magic. */
        {
            unsigned char tmp[CSB_CDU_HEADER_SIZE + 32];
            memcpy(tmp, container, (size_t)clen);
            tmp[0] = 'X';
            memset(&dn, 0, sizeof(dn));
            CHECK(csb_v1_decompdu_decompress_grid(tmp, clen, &dn)
                      == CSB_DECOMPDU_ERR_CORRUPT,
                  "corrupt magic -> ERR_CORRUPT");
        }

        /* Short container (< header). */
        {
            memset(&dn, 0, sizeof(dn));
            CHECK(csb_v1_decompdu_decompress_grid(container, 4, &dn)
                      == CSB_DECOMPDU_ERR_CORRUPT,
                  "container shorter than header -> ERR_CORRUPT");
        }

        /* Wrong declared payload size. */
        {
            unsigned char tmp[CSB_CDU_HEADER_SIZE + 32];
            memcpy(tmp, container, (size_t)clen);
            tmp[8] = 0xFF; /* payload byte count no longer == w*h*l */
            memset(&dn, 0, sizeof(dn));
            CHECK(csb_v1_decompdu_decompress_grid(tmp, clen, &dn)
                      == CSB_DECOMPDU_ERR_CORRUPT,
                  "declared payload != w*h*l -> ERR_CORRUPT");
        }
    }

    /* F0455 core: corrupt (too-short table) and truncated stream. */
    {
        unsigned char tiny[10] = {0};
        unsigned char out[8];
        CHECK(csb_v1_decompdu_f0455(tiny, 10, out, 8)
                  == CSB_DECOMPDU_ERR_CORRUPT,
              "stream shorter than 20-byte table -> ERR_CORRUPT");
    }
    {
        /* Valid 20-byte table but no payload bits, asking for
         * many output bytes -> truncation. */
        unsigned char table[20];
        unsigned char out[64];
        int i;
        for (i = 0; i < 20; ++i) table[i] = (unsigned char)i;
        CHECK(csb_v1_decompdu_f0455(table, 20, out, 64)
                  == CSB_DECOMPDU_ERR_TRUNCATED,
              "no payload bits for 64 bytes -> ERR_TRUNCATED");
    }

    /* NULL / bounds. */
    {
        unsigned char out[4];
        CSB_CDUDungeon dn;
        memset(&dn, 0, sizeof(dn));
        CHECK(csb_v1_decompdu_f0455(NULL, 20, out, 4) == CSB_DECOMPDU_ERR_NULL,
              "NULL compressed -> ERR_NULL");
        CHECK(csb_v1_decompdu_decompress_grid(NULL, 32, &dn)
                  == CSB_DECOMPDU_ERR_NULL,
              "NULL container -> ERR_NULL");
    }
    {
        unsigned char tiles[4] = {1,2,3,4};
        unsigned char container[128];
        long clen = 0;
        CHECK(csb_v1_decompdu_build_grid(tiles, 0, 2, 1, container,
                                         sizeof(container), &clen)
                  == CSB_DECOMPDU_ERR_BOUNDS,
              "width 0 -> ERR_BOUNDS");
        CHECK(csb_v1_decompdu_build_grid(tiles, 2, 2, 99, container,
                                         sizeof(container), &clen)
                  == CSB_DECOMPDU_ERR_BOUNDS,
              "levelCount > 24 -> ERR_BOUNDS");
    }

    /* csb_v1_cdu_dungeon_free is NULL-safe. */
    csb_v1_cdu_dungeon_free(NULL);
    CHECK(1, "free(NULL) is a safe no-op");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
