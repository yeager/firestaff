/**
 * firestaff_dm2_v1_dungeon_parser_probe.c
 *
 * Pass H2312: DM2 V1 Phase 8 — Dungeon Parser Probe
 *
 * Headless C probe that exercises dm2_v1_dungeon_load() and
 * dm2_v1_dungeon_get_square_type() against the canonical DUNGEON.DAT.
 * Verifies the DM2 multi-level dungeon format (outdoor/indoor/building),
 * level metadata parsing, and per-square tile access.
 *
 * Compile (from repo root):
 *   gcc -I include -I src/shared \
 *       probes/firestaff_dm2_v1_dungeon_parser_probe.c \
 *       src/dm2/dm2_v1_dungeon_loader.c \
 *       -o build/firestaff_dm2_v1_dungeon_parser_probe \
 *       2>&1 | head -50
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
 *     ./build/firestaff_dm2_v1_dungeon_parser_probe \
 *     ~/.firestaff/data/dm2/DUNGEON.DAT
 *
 * Source: SKULL.ASM T560 (DUNGEON_Load) and dm2_v1_dungeon_loader.c
 * Schema: firestaff.dm2_v1.dungeon_parser_probe.v1
 */

#include "dm2_v1_dungeon_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
#define ASSERT(pred, msg) do { \
    if (!(pred)) { \
        fprintf(stderr, "ASSERTION FAILED: %s (file=%s line=%d)\n", \
                (msg), __FILE__, __LINE__); \
        return 1; \
    } \
} while (0)
#else
#define ASSERT(pred, msg) do { \
    if (!(pred)) { \
        fprintf(stderr, "ASSERTION FAILED: %s\n", (msg)); \
        return 1; \
    } \
} while (0)
#endif

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        passed++; \
    } \
} while (0)

static const char *level_type_name(DM2_LevelType t) {
    switch (t) {
        case DM2_LEVEL_OUTDOOR:  return "OUTDOOR";
        case DM2_LEVEL_INDOOR:   return "INDOOR";
        case DM2_LEVEL_BUILDING: return "BUILDING";
        default:                 return "UNKNOWN";
    }
}

/* Read file into heap buffer */
static uint8_t *load_file(const char *path, int *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); return NULL; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, sz, f);
    buf[sz] = 0;
    fclose(f);
    *out_size = (int)sz;
    return buf;
}

/* Compute SHA256 of buffer (first n bytes) for state fingerprinting */
static void sha256_buf(uint8_t *out_digest, const uint8_t *data, int size) {
    /* Minimal SHA256 — inline to avoid external dependencies */
    /* Only used for fingerprinting, not crypto */
    uint32_t h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    uint8_t block[64] = {0};
    int i, bi = 0;
    uint64_t bitlen = (uint64_t)size * 8;

    for (i = 0; i < size; i++) {
        block[bi++] ^= data[i];
        if (bi == 64) {
            bi = 0; /* absorb block */
        }
    }
    block[bi++] ^= 0x80;
    if (bi > 56) {
        memset(&block[bi], 0, 64 - bi);
        bi = 0; /* absorb padding block */
    }
    memset(&block[bi], 0, 56 - bi);
    for (int j = 0; j < 8; j++) {
        block[56 + (7 - j)] = (uint8_t)((bitlen >> (j * 8)) & 0xff);
    }
    (void)h; /* unused in this stub — real probe uses openssl/sha256.c */
    memset(out_digest, 0xab, 32); /* deterministic fingerprint stub */
    /* In a real build this would call a SHA256 implementation.
     * The probe is headless and uses a deterministic stub hash so that
     * pass/fail is stable without external crypto deps. */
}

int main(int argc, char **argv) {
    const char *dungeon_path;
    int errors = 0, passed = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <dungeon.dat path>\n", argv[0]);
        fprintf(stderr, "  e.g. %s ~/.firestaff/data/dm2/DUNGEON.DAT\n", argv[0]);
        return 1;
    }
    dungeon_path = argv[1];

    fprintf(stderr, "=== DM2 V1 Dungeon Parser Probe ===\n");
    fprintf(stderr, "Source: SKULL.ASM T560 (DUNGEON_Load), dm2_v1_dungeon_loader.c\n");
    fprintf(stderr, "Input: %s\n\n", dungeon_path);

    /* ── Load file ── */
    int file_size = 0;
    uint8_t *raw = load_file(dungeon_path, &file_size);
    if (!raw) return 1;

    PROBE_ASSERT(file_size > 0, "DUNGEON.DAT file_size > 0 (got %d)", file_size);

    /* ── Parse header ── */
    /* DM2 DUNGEON.DAT header format (from dm2_v1_dungeon_loader.c):
     *   offset 0: uint16 LE level_count
     *   offset 2+: level descriptors (8 bytes each):
     *     byte 0: level_type (0=OUTDOOR, 1=INDOOR, 2=BUILDING)
     *     byte 1: level_width
     *     byte 2: level_height
     *     bytes 4-5: offset low word (LE uint16)
     *     bytes 6-7: offset high word (LE uint16)
     *   Followed by per-level tile data (uint16 per square, column-major)
     *
     * SKULL.ASM T560 source evidence:
     *   "level_count at offset 0, then 8-byte level descriptors,
     *    then per-level offset pointers to tile data"
     */

    uint16_t level_count = raw[0] | ((uint16_t)raw[1] << 8);
    fprintf(stderr, "INFO: level_count from header = %u\n", level_count);

    PROBE_ASSERT(level_count > 0, "level_count > 0 (got %u)", level_count);
    PROBE_ASSERT(level_count <= 30, "level_count <= DM2_V1_MAX_LEVELS(30), got %u", level_count);

    /* ── Load via dm2_v1_dungeon_load ── */
    DM2_V1_DungeonData dungeon;
    int load_result = dm2_v1_dungeon_load(&dungeon, raw, file_size);

    PROBE_ASSERT(load_result == 0, "dm2_v1_dungeon_load returns 0 (success)");
    PROBE_ASSERT(dungeon.level_count > 0, "dungeon.level_count > 0 (got %d)", dungeon.level_count);
    PROBE_ASSERT(dungeon.level_count <= DM2_V1_MAX_LEVELS,
                 "dungeon.level_count <= MAX_LEVELS(30), got %d", dungeon.level_count);
    PROBE_ASSERT(dungeon.raw_data != NULL, "dungeon.raw_data != NULL (data retained)");
    PROBE_ASSERT(dungeon.raw_size == file_size,
                 "dungeon.raw_size == file_size (%d == %d)", dungeon.raw_size, file_size);

    /* ── Inspect level metadata ── */
    fprintf(stderr, "\nINFO: Dungeon has %d level(s):\n", dungeon.level_count);
    for (int i = 0; i < dungeon.level_count; i++) {
        fprintf(stderr, "  level %d: type=%s, size=%dx%d, offset=0x%08x\n",
                i,
                level_type_name(dungeon.level_types[i]),
                dungeon.level_widths[i],
                dungeon.level_heights[i],
                (unsigned)dungeon.level_offsets[i]);
    }

    /* Verify level type is valid enum range */
    for (int i = 0; i < dungeon.level_count; i++) {
        PROBE_ASSERT(dungeon.level_types[i] <= DM2_LEVEL_BUILDING,
                      "level %d type is valid enum (got %d)", i, dungeon.level_types[i]);
        PROBE_ASSERT(dungeon.level_widths[i] > 0 && dungeon.level_widths[i] <= DM2_V1_MAX_MAP_SIZE,
                      "level %d width is valid 1..64 (got %d)", i, dungeon.level_widths[i]);
        PROBE_ASSERT(dungeon.level_heights[i] > 0 && dungeon.level_heights[i] <= DM2_V1_MAX_MAP_SIZE,
                      "level %d height is valid 1..64 (got %d)", i, dungeon.level_heights[i]);
    }

    /* ── Test square access on each level ── */
    fprintf(stderr, "\nINFO: Sampling square types on each level:\n");
    for (int i = 0; i < dungeon.level_count; i++) {
        int w = dungeon.level_widths[i];
        int h = dungeon.level_heights[i];

        /* Sample first square (0,0) */
        int t00 = dm2_v1_dungeon_get_square_type(&dungeon, i, 0, 0);
        PROBE_ASSERT(t00 >= 0, "level %d square(0,0) accessible (type=%d)", i, t00);

        /* Sample last square */
        int t_last = dm2_v1_dungeon_get_square_type(&dungeon, i, w - 1, h - 1);
        PROBE_ASSERT(t_last >= 0, "level %d square(%d,%d) accessible (type=%d)",
                     i, w - 1, h - 1, t_last);

        /* Sample center square */
        int cm = dm2_v1_dungeon_get_square_type(&dungeon, i, w / 2, h / 2);
        PROBE_ASSERT(cm >= 0, "level %d center square (%d,%d) accessible (type=%d)",
                     i, w / 2, h / 2, cm);

        /* Out-of-bounds should return -1 */
        int t_bad = dm2_v1_dungeon_get_square_type(&dungeon, i, -1, 0);
        PROBE_ASSERT(t_bad == -1, "level %d out-of-bounds x=-1 returns -1", i);

        t_bad = dm2_v1_dungeon_get_square_type(&dungeon, i, 0, h);
        PROBE_ASSERT(t_bad == -1, "level %d out-of-bounds y=%d returns -1", i, h);

        /* is_outdoor */
        int is_out = dm2_v1_dungeon_is_outdoor(&dungeon, i);
        PROBE_ASSERT(is_out == 0 || is_out == 1,
                     "level %d is_outdoor returns 0 or 1 (got %d)", i, is_out);

        fprintf(stderr, "  level %d: (0,0)=%d, center=%d, (%d,%d)=%d, outdoor=%d\n",
                i, t00, cm, w - 1, h - 1, t_last, is_out);
    }

    /* ── Test invalid level access ── */
    int t_bad_level = dm2_v1_dungeon_get_square_type(&dungeon, -1, 0, 0);
    PROBE_ASSERT(t_bad_level == -1, "level=-1 returns -1 (invalid level)");

    t_bad_level = dm2_v1_dungeon_get_square_type(&dungeon, dungeon.level_count, 0, 0);
    PROBE_ASSERT(t_bad_level == -1, "level=%d (out of range) returns -1", dungeon.level_count);

    int bad_outdoor = dm2_v1_dungeon_is_outdoor(&dungeon, -1);
    PROBE_ASSERT(bad_outdoor == 0, "is_outdoor(level=-1) returns 0");

    /* ── Null argument guards ── */
    int t_null = dm2_v1_dungeon_get_square_type(NULL, 0, 0, 0);
    PROBE_ASSERT(t_null == -1, "dungeon=NULL returns -1");

    int load_null = dm2_v1_dungeon_load(NULL, raw, file_size);
    PROBE_ASSERT(load_null == -1, "dungeon_load(NULL, ...) returns -1");

    load_null = dm2_v1_dungeon_load(&dungeon, NULL, file_size);
    PROBE_ASSERT(load_null == -1, "dungeon_load(..., NULL, ...) returns -1");

    load_null = dm2_v1_dungeon_load(&dungeon, raw, 0);
    PROBE_ASSERT(load_null == -1, "dungeon_load(..., size=0) returns -1");

    /* ── Source evidence string ── */
    const char *evidence = dm2_v1_dungeon_source_evidence();
    PROBE_ASSERT(evidence != NULL && strlen(evidence) > 0,
                 "dm2_v1_dungeon_source_evidence() returns non-empty string");

    /* ── State fingerprint (deterministic stub) ── */
    uint8_t digest[32];
    sha256_buf(digest, raw, file_size);
    fprintf(stderr, "\nINFO: State fingerprint (stub SHA256): ");
    for (int i = 0; i < 8; i++) fprintf(stderr, "%02x", digest[i]);
    fprintf(stderr, "...\n");

    /* ── Cleanup ── */
    dm2_v1_dungeon_free(&dungeon);
    free(raw);

    /* ── Summary ── */
    fprintf(stderr, "\n=== Results: %d passed, %d failed ===\n", passed, errors);
    if (errors > 0) {
        fprintf(stderr, "PROBE FAILED — see above for details.\n");
        return 1;
    }
    fprintf(stderr, "PROBE PASSED — all assertions succeeded.\n");
    return 0;
}