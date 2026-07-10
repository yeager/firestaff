#include "dm1_v1_original_save_classifier.h"
#include "dm1_v1_save_load.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#include <process.h>
#define test_mkdir(p) _mkdir(p)
#define test_rmdir(p) _rmdir(p)
#define test_unlink(p) remove(p)
#else
#include <unistd.h>
#define test_mkdir(p) mkdir((p), 0700)
#define test_rmdir(p) rmdir(p)
#define test_unlink(p) unlink(p)
#endif

static int g_pass = 0;
static int g_fail = 0;

static void check_int(const char *label, int got, int want) {
    if (got != want) {
        printf("FAIL %s: got %d want %d\n", label, got, want);
        g_fail++;
    } else {
        g_pass++;
    }
}

static void check_u16(const char *label, uint16_t got, uint16_t want) {
    if (got != want) {
        printf("FAIL %s: got 0x%04X want 0x%04X\n", label, got, want);
        g_fail++;
    } else {
        g_pass++;
    }
}

static void check_u32(const char *label, uint32_t got, uint32_t want) {
    if (got != want) {
        printf("FAIL %s: got 0x%08X want 0x%08X\n", label, got, want);
        g_fail++;
    } else {
        g_pass++;
    }
}

static void check_str_nonempty(const char *label, const char *value) {
    if (!value || !value[0]) {
        printf("FAIL %s: empty string\n", label);
        g_fail++;
    } else {
        g_pass++;
    }
}

static void wr16le(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static void wr16be(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)((v >> 8) & 0xffu);
    p[1] = (uint8_t)(v & 0xffu);
}

static void wr32le(uint8_t *p, uint32_t v) {
    wr16le(p, (uint16_t)(v & 0xffffu));
    wr16le(p + 2, (uint16_t)((v >> 16) & 0xffffu));
}

static void wr32be(uint8_t *p, uint32_t v) {
    wr16be(p, (uint16_t)((v >> 16) & 0xffffu));
    wr16be(p + 2, (uint16_t)(v & 0xffffu));
}

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint16_t rd16be(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static void wr16(uint8_t *p, uint16_t v, int be) {
    if (be) wr16be(p, v);
    else wr16le(p, v);
}

static void wr32(uint8_t *p, uint32_t v, int be) {
    if (be) wr32be(p, v);
    else wr32le(p, v);
}

static uint16_t rd16(const uint8_t *p, int be) {
    return be ? rd16be(p) : rd16le(p);
}

static uint16_t checksum_first_half(const uint8_t *header, int be) {
    uint16_t acc = 0;
    for (size_t i = 0; i < 32u; i++) {
        acc = (uint16_t)(acc + rd16(header + (i * 8u) + 0u, be));
        acc = (uint16_t)(acc ^ rd16(header + (i * 8u) + 2u, be));
        acc = (uint16_t)(acc - rd16(header + (i * 8u) + 4u, be));
        acc = (uint16_t)(acc ^ rd16(header + (i * 8u) + 6u, be));
    }
    return acc;
}

static uint16_t checksum_second_half_plain(const uint8_t *header, int be) {
    uint16_t sum = 0;
    for (size_t i = 128u; i < 256u; i++) {
        sum = (uint16_t)(sum + rd16(header + (i * 2u), be));
    }
    return sum;
}

static void xor_obfuscate_second_half(uint8_t *header, uint16_t key, int be) {
    uint16_t rolling_key = key;
    for (size_t i = 128u; i < 256u; i++) {
        uint8_t *word = header + (i * 2u);
        uint16_t v = rd16(word, be);
        wr16(word, (uint16_t)(v ^ rolling_key), be);
        rolling_key = (uint16_t)(rolling_key + 128u);
    }
}

static void build_original_header(uint8_t header[DM1_ORIGINAL_SAVE_HEADER_BYTES],
                                  uint8_t format_id,
                                  int big_endian,
                                  uint32_t game_id) {
    memset(header, 0, DM1_ORIGINAL_SAVE_HEADER_BYTES);

    for (size_t i = 0; i < 127u; i++) {
        wr16(header + (i * 2u),
             (uint16_t)(0x1111u + (uint16_t)(i * 37u)),
             big_endian);
    }
    wr16(header + (10u * 2u), 0x2468u, big_endian);

    header[298] = 1u;
    header[299] = format_id;
    wr32(header + 300u, 0x01020304u, big_endian);
    wr16(header + 304u, 1u, big_endian);
    wr32(header + 306u, game_id, big_endian);
    for (size_t i = 0; i < 16u; i++) {
        wr16(header + 310u + (i * 2u),
             (uint16_t)(0x2000u + i),
             big_endian);
        wr16(header + 342u + (i * 2u),
             (uint16_t)(0x3000u + (i * 3u)),
             big_endian);
    }
    wr16(header + 374u, 9u, big_endian);
    wr16(header + 376u, 10u, big_endian);
    memcpy(header + 378u, "firestaff-test-additional-data", 30u);

    {
        uint16_t second_sum = checksum_second_half_plain(header, big_endian);
        uint16_t first_before_last = checksum_first_half(header, big_endian);
        uint16_t last = (uint16_t)(rd16(header + 254u, big_endian) ^
                                   first_before_last ^
                                   second_sum);
        wr16(header + 254u, last, big_endian);
    }

    xor_obfuscate_second_half(header, rd16(header + (10u * 2u), big_endian), big_endian);
}

static void test_null_and_absent(void) {
    DM1OriginalSaveClassifyResult r;

    check_int("NULL result rejected",
              dm1_v1_original_save_classify_bytes(NULL, 0, NULL), 0);
    check_int("NULL bytes rejected",
              dm1_v1_original_save_classify_bytes(NULL, 7, &r), 0);
    check_int("empty bytes classify",
              dm1_v1_original_save_classify_bytes((const uint8_t *)"", 0, &r), 1);
    check_int("empty bytes absent shape", r.shape, DM1_ORIGINAL_SAVE_SHAPE_ABSENT);
    check_int("empty bytes absent readiness", r.readiness, DM1_ORIGINAL_SAVE_READY_ABSENT);
}

static void test_firestaff_native_rejected_as_original(void) {
    uint8_t bytes[64];
    DM1OriginalSaveClassifyResult r;
    memset(bytes, 0, sizeof(bytes));
    memcpy(bytes, DM1_SAVE_MAGIC, 8u);

    check_int("Firestaff native classify",
              dm1_v1_original_save_classify_bytes(bytes, sizeof(bytes), &r), 1);
    check_int("Firestaff native shape", r.shape, DM1_ORIGINAL_SAVE_SHAPE_FIRESTAFF_NATIVE);
    check_int("Firestaff native readiness", r.readiness, DM1_ORIGINAL_SAVE_READY_NOT_ORIGINAL);
    check_int("Firestaff native not import-blocked", r.import_blocked_until_roundtrip, 0);
}

static void test_original_dm1_little_endian(void) {
    uint8_t bytes[800];
    DM1OriginalSaveClassifyResult r;
    memset(bytes, 0x5a, sizeof(bytes));
    build_original_header(bytes, 1u, 0, 0x12345678u);

    check_int("LE original classify",
              dm1_v1_original_save_classify_bytes(bytes, sizeof(bytes), &r), 1);
    check_int("LE original shape", r.shape, DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1);
    check_int("LE original readiness", r.readiness, DM1_ORIGINAL_SAVE_READY_CLASSIFIED_HEADER_ONLY);
    check_int("LE header checksum ok", r.header_checksum_ok, 1);
    check_int("LE import blocked", r.import_blocked_until_roundtrip, 1);
    check_u16("LE format id", r.format_id, 1u);
    check_u16("LE useless byte", r.useless, 1u);
    check_u32("LE game id", r.game_id, 0x12345678u);
    check_u16("LE platform", r.platform, 9u);
    check_u16("LE dungeon", r.dungeon_id, 10u);
    check_u16("LE keys nonzero", r.save_part_key_count_nonzero, 16u);
    check_u16("LE checksums nonzero", r.save_part_checksum_count_nonzero, 16u);
}

static void test_original_dm1_big_endian(void) {
    uint8_t bytes[800];
    DM1OriginalSaveClassifyResult r;
    memset(bytes, 0xa5, sizeof(bytes));
    build_original_header(bytes, 1u, 1, 0x89abcdefu);

    check_int("BE original classify",
              dm1_v1_original_save_classify_bytes(bytes, sizeof(bytes), &r), 1);
    check_int("BE original shape", r.shape, DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1);
    check_int("BE checksum ok", r.header_checksum_ok, 1);
    check_u32("BE game id", r.game_id, 0x89abcdefu);
}

static void test_original_dm1_pc34_importer_candidate(void) {
    uint8_t bytes[800];
    DM1OriginalSaveClassifyResult r;
    memset(bytes, 0x33, sizeof(bytes));
    build_original_header(bytes, 5u, 0, 0x34345043u);

    check_int("PC34 original classify",
              dm1_v1_original_save_classify_bytes(bytes, sizeof(bytes), &r), 1);
    check_int("PC34 original shape",
              r.shape, DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34);
    check_int("PC34 readiness",
              r.readiness, DM1_ORIGINAL_SAVE_READY_CLASSIFIED_HEADER_ONLY);
    check_int("PC34 checksum ok", r.header_checksum_ok, 1);
    check_int("PC34 importer candidate", r.pc34_importer_candidate, 1);
    check_int("PC34 still blocked until real roundtrip",
              r.import_blocked_until_roundtrip, 1);
    check_u16("PC34 format id", r.format_id, 5u);
    check_u16("PC34 platform", r.platform, 9u);
    check_u16("PC34 dungeon", r.dungeon_id, 10u);
}

static void test_rejects_mutated_header(void) {
    uint8_t bytes[800];
    DM1OriginalSaveClassifyResult r;
    memset(bytes, 0, sizeof(bytes));
    build_original_header(bytes, 1u, 0, 0x11112222u);
    bytes[257] ^= 0x40u;

    check_int("mutated original classify",
              dm1_v1_original_save_classify_bytes(bytes, sizeof(bytes), &r), 1);
    check_int("mutated original rejected", r.shape, DM1_ORIGINAL_SAVE_SHAPE_REJECTED);
    check_int("mutated original checksum bad", r.header_checksum_ok, 0);
}

static void test_compat_family_and_unknown_format(void) {
    uint8_t bytes[800];
    DM1OriginalSaveClassifyResult r;
    memset(bytes, 0, sizeof(bytes));
    build_original_header(bytes, 2u, 0, 0x01010101u);
    check_int("compat family classify",
              dm1_v1_original_save_classify_bytes(bytes, sizeof(bytes), &r), 1);
    check_int("compat family shape", r.shape, DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_COMPAT_FAMILY);
    check_int("compat family still header-only",
              r.readiness, DM1_ORIGINAL_SAVE_READY_CLASSIFIED_HEADER_ONLY);
    check_int("compat family not PC34 candidate", r.pc34_importer_candidate, 0);

    build_original_header(bytes, 99u, 0, 0x02020202u);
    check_int("unknown format classify",
              dm1_v1_original_save_classify_bytes(bytes, sizeof(bytes), &r), 1);
    check_int("unknown format rejected", r.shape, DM1_ORIGINAL_SAVE_SHAPE_REJECTED);
    check_int("unknown format readiness rejected", r.readiness, DM1_ORIGINAL_SAVE_READY_REJECTED);
}

static int make_temp_root(char out[DM1_ORIGINAL_SAVE_PATH_MAX]) {
#if defined(_WIN32) || defined(_WIN64)
    int pid = _getpid();
    for (int i = 0; i < 32; i++) {
        int n = snprintf(out, DM1_ORIGINAL_SAVE_PATH_MAX,
                         "dm1_original_save_classifier_%d_%d", pid, i);
        if (n <= 0 || n >= DM1_ORIGINAL_SAVE_PATH_MAX) return 0;
        if (test_mkdir(out) == 0) return 1;
    }
    return 0;
#else
    strncpy(out, "/tmp/dm1_original_save_classifier_XXXXXX",
            DM1_ORIGINAL_SAVE_PATH_MAX - 1u);
    out[DM1_ORIGINAL_SAVE_PATH_MAX - 1u] = '\0';
    return mkdtemp(out) != NULL;
#endif
}

static int write_file(const char *path, const uint8_t *bytes, size_t size) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;
    if (size > 0u && fwrite(bytes, 1, size, fp) != size) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static void cleanup_root(const char *root) {
    char path[DM1_ORIGINAL_SAVE_PATH_MAX];
    if (!root || !root[0]) return;
    for (int i = 0; i < (int)DM1_ORIGINAL_SAVE_DEFAULT_CANDIDATE_COUNT; i++) {
        if (dm1_v1_original_save_candidate_path(root, i, path)) {
            test_unlink(path);
        }
    }
    test_rmdir(root);
}

static void test_root_manifest(void) {
    char root[DM1_ORIGINAL_SAVE_PATH_MAX];
    char path[DM1_ORIGINAL_SAVE_PATH_MAX];
    uint8_t bytes[800];
    DM1OriginalSaveManifest manifest;

    if (!make_temp_root(root)) {
        printf("FAIL temp root\n");
        g_fail++;
        return;
    }

    memset(bytes, 0, sizeof(bytes));
    build_original_header(bytes, 5u, 0, 0x0badc0deu);
    check_int("candidate path 0",
              dm1_v1_original_save_candidate_path(root, 0, path), 1);
    check_int("write DMSAVE.DAT", write_file(path, bytes, sizeof(bytes)), 1);

    check_int("candidate path 1",
              dm1_v1_original_save_candidate_path(root, 1, path), 1);
    memset(bytes, 0, sizeof(bytes));
    memcpy(bytes, DM1_SAVE_MAGIC, 8u);
    check_int("write DMSAVE.BAK", write_file(path, bytes, sizeof(bytes)), 1);

    check_int("root manifest classify",
              dm1_v1_original_save_classify_root(root, &manifest), 1);
    check_int("manifest candidate count", manifest.candidate_count,
              (int)DM1_ORIGINAL_SAVE_DEFAULT_CANDIDATE_COUNT);
    check_int("manifest present count", manifest.present_count, 2);
    check_int("manifest classified count", manifest.classified_count, 1);
    check_int("manifest original dm1 count", manifest.original_dm1_count, 1);
    check_int("manifest original dm1 pc34 count",
              manifest.original_dm1_pc34_count, 1);
    check_int("manifest pc34 importer candidate count",
              manifest.pc34_importer_candidate_count, 1);
    check_int("manifest Firestaff native count", manifest.firestaff_native_count, 1);

    cleanup_root(root);
}

static void test_helpers(void) {
    char root[DM1_ORIGINAL_SAVE_PATH_MAX];
    char path[DM1_ORIGINAL_SAVE_PATH_MAX];
    check_int("default root resolves", dm1_v1_original_save_default_root(root), 1);
    check_int("default root non-empty", root[0] ? 1 : 0, 1);
    check_int("candidate 0 path",
              dm1_v1_original_save_candidate_path("/tmp/dm1", 0, path), 1);
    check_int("candidate suffix",
              strstr(path, "DMSAVE.DAT") != NULL ? 1 : 0, 1);
    check_int("bad candidate rejected",
              dm1_v1_original_save_candidate_path("/tmp/dm1", 99, path), 0);
    check_str_nonempty("shape name",
                       dm1_v1_original_save_shape_name(DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1));
    check_str_nonempty("pc34 shape name",
                       dm1_v1_original_save_shape_name(DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34));
    check_str_nonempty("readiness name",
                       dm1_v1_original_save_readiness_name(DM1_ORIGINAL_SAVE_READY_CLASSIFIED_HEADER_ONLY));
    check_str_nonempty("source evidence",
                       dm1_v1_original_save_source_evidence());
}

int main(void) {
    test_null_and_absent();
    test_firestaff_native_rejected_as_original();
    test_original_dm1_little_endian();
    test_original_dm1_big_endian();
    test_original_dm1_pc34_importer_candidate();
    test_rejects_mutated_header();
    test_compat_family_and_unknown_format();
    test_root_manifest();
    test_helpers();

    printf("DM1 original save classifier: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
