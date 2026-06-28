/*
 * test_firestaff_sck_mapfile_corpus_verifier.c
 *
 * CTest for the bounded Greatstone/SCK db/map corpus
 * verifier.  The test is data-free: it synthesizes V1 and V2
 * mapfiles in memory and exercises the per-file and aggregate
 * paths without ever touching the real corpus directory.
 *
 * Coverage:
 *   - V2 parse path: header + comma-separated items with
 *     SIZE= attributes, attribute prefix aggregation, oversized
 *     slice classification, distinct type tracking.
 *   - V2 truncated path: a 1200-row mapfile that exceeds the
 *     bounded FIRESTAFF_SCK_MAPFILE_MAX_ITEMS=1024 cap is
 *     classified as TOO_LARGE with v2Rows reported as a lower
 *     bound.
 *   - V1 parse path: legacy `type name offset size` rows with
 *     no SIZE attributes, all reported as unsized.
 *   - Header-only / empty / garbage / single-property V2 files
 *     (the documented shape used by ANIMATION / SAVEGAME /
 *     DUNGEON container entries).
 *   - Per-FORMAT aggregate: a synthetic multi-file corpus drives
 *     `FirestaffSckCorpus_VerifyDirectory()` via a temp dir and
 *     checks the per-FORMAT slot counts.
 *   - Corpus-level rejection paths: NULL args, missing directory.
 *   - Result strings stay stable for diagnostics.
 */

#include "firestaff_sck_mapfile_corpus_verifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(_WIN32)
#include <windows.h>
#endif

static int g_failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++g_failures; \
    } \
} while (0)

#define CHECK_STR_EQ(actual, expected) do { \
    if (strcmp((actual), (expected)) != 0) { \
        fprintf(stderr, \
                "CHECK failed at %s:%d: \"%s\" != \"%s\"\n", \
                __FILE__, __LINE__, (actual), (expected)); \
        ++g_failures; \
    } \
} while (0)

/* A small SCK 2.x mapfile: 3 sized rows + 2 unsized. */
static const char* kSizedMapfile =
    "MAPFORMATVERSION=2.0,ENDIAN=BIG,FORMAT=DMCSB1\n"
    "0001,IMG1,SIZE=16,Dungeon Graphics,Tile A,\n"
    "0017,IMG3,PAL=PAL1&SIZE=64,Interface,Main,\n"
    "0081,IMG1,SIZE=128,Dungeon Graphics,Tile B,\n"
    "0200,RAW1,NULL,Code,Not yet decoded,\n"
    "9999,PAL,NULL,Palette,Unused,\n";

/* V1 legacy whitespace-separated rows. */
static const char* kLegacyMapfile =
    "# Legacy Greatstone mapfile\n"
    "IMG1 image00 0 256\n"
    "IMG5 image01 0x100 32\n"
    "PAL palette00 288 16\n";

/* Single-property V2 file (header-only descriptor shape). */
static const char* kHeaderOnlyMapfile =
    "ENDIAN=BIG,FORMAT=ANIMATION,SNDS.SPR=22050\n";

/* Truly empty file. */
static const char* kEmptyMapfile = "";

/* DUNGEON single-property shape (FORMAT=DUNGEON, no items). */
static const char* kDungeonHeaderMapfile =
    "FORMAT=DUNGEON\n";

static unsigned int distinct_contains(const char distinct[][FIRESTAFF_SCK_CORPUS_TYPE_BYTES],
                                      unsigned int count,
                                      const char* needle) {
    unsigned int i;
    for (i = 0u; i < count; ++i) {
        if (strcmp(distinct[i], needle) == 0) {
            return 1u;
        }
    }
    return 0u;
}

static const FirestaffSckCorpusFormatStats* find_format(
    const FirestaffSckCorpusSummary* summary,
    const char* fmt) {
    unsigned int i;
    for (i = 0u; i < summary->formatCount; ++i) {
        if (strcmp(summary->formats[i].format, fmt) == 0) {
            return &summary->formats[i];
        }
    }
    return NULL;
}

static void test_v2_sized_mapfile(void) {
    FirestaffSckCorpusFileStats stats;
    FirestaffSckCorpusFileResult r;

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpus_VerifyText("sized.map", kSizedMapfile, 1024u, &stats);
    CHECK(r == FIRESTAFF_SCK_CORPUS_FILE_OK);
    CHECK(stats.parseOk == 1u);
    CHECK(stats.v2Rows == 5u);
    CHECK(stats.v1Rows == 0u);
    CHECK(stats.sizedRows == 3u);
    CHECK(stats.unsizedRows == 2u);
    CHECK(stats.oversizedRows == 0u);
    CHECK(stats.truncated == 0u);
    CHECK(stats.largestSliceEnd == 81u + 128u);
    CHECK_STR_EQ(stats.format, "DMCSB1");
    CHECK_STR_EQ(stats.endian, "BIG");

    /* Distinct types include IMG1, IMG3, RAW1, PAL. */
    CHECK(distinct_contains(stats.distinctTypes, stats.distinctTypeCount, "IMG1"));
    CHECK(distinct_contains(stats.distinctTypes, stats.distinctTypeCount, "IMG3"));
    CHECK(distinct_contains(stats.distinctTypes, stats.distinctTypeCount, "RAW1"));
    CHECK(distinct_contains(stats.distinctTypes, stats.distinctTypeCount, "PAL"));
    CHECK(stats.distinctTypeCount == 4u);

    /* The IMG3 row carries PAL=SIZE= attribute prefixes. */
    CHECK(distinct_contains(stats.distinctAttrs,
                            stats.distinctAttrPrefixCount,
                            "PAL"));
    CHECK(distinct_contains(stats.distinctAttrs,
                            stats.distinctAttrPrefixCount,
                            "SIZE"));
    /* The literal `NULL` placeholder is treated as no-attributes. */
    CHECK(!distinct_contains(stats.distinctAttrs,
                             stats.distinctAttrPrefixCount,
                             "NULL"));
}

static void test_v2_oversized_slice(void) {
    FirestaffSckCorpusFileStats stats;
    FirestaffSckCorpusFileResult r;

    memset(&stats, 0, sizeof(stats));
    /* Target file 64 bytes: the IMG1 SIZE=128 row starting at
     * offset 81 ends at 209, which exceeds 64.  At least one row
     * must be flagged oversized. */
    r = FirestaffSckCorpus_VerifyText("sized.map", kSizedMapfile, 64u, &stats);
    CHECK(r == FIRESTAFF_SCK_CORPUS_FILE_OK);
    CHECK(stats.sizedRows == 3u);
    CHECK(stats.oversizedRows >= 1u);

    /* Passing targetFileBytes=0 disables the oversized check. */
    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpus_VerifyText("sized.map", kSizedMapfile, 0u, &stats);
    CHECK(r == FIRESTAFF_SCK_CORPUS_FILE_OK);
    CHECK(stats.oversizedRows == 0u);
    CHECK(stats.largestSliceEnd == 209u);
}

static void test_v2_truncated_mapfile(void) {
    /* Build a SCK 2.x mapfile with > MAX_ITEMS items so the bounded
     * parser fills its buffer and rejects the rest as TRUNCATED.
     * We assert v2Rows == MAX_ITEMS (lower bound), truncated == 1,
     * file-level outcome == TOO_LARGE, and parseOk stays 0 because
     * the truncated parser buffer is not a clean parse. */
    static char bigText[256 * 1024];
    static int built = 0;
    FirestaffSckCorpusFileStats stats;
    FirestaffSckCorpusFileResult r;
    char* p;
    unsigned int i;
    size_t left;
    int wrote;

    if (!built) {
        p = bigText;
        left = sizeof(bigText);
        wrote = snprintf(p, left,
                         "MAPFORMATVERSION=2.0,FORMAT=ROM,ENDIAN=LITTLE\n");
        if (wrote <= 0 || (size_t)wrote >= left) {
            ++g_failures;
            return;
        }
        p += wrote;
        left -= (size_t)wrote;
        for (i = 0u; i < 1200u; ++i) {
            wrote = snprintf(p, left,
                             "%u,RAW1,SIZE=4,row%u,desc,\n", i, i);
            if (wrote <= 0 || (size_t)wrote >= left) {
                break;
            }
            p += wrote;
            left -= (size_t)wrote;
        }
        built = 1;
    }

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpus_VerifyText("big.map", bigText, 1024u * 1024u, &stats);
    CHECK(r == FIRESTAFF_SCK_CORPUS_FILE_TOO_LARGE);
    CHECK(stats.parseOk == 0u);
    CHECK(stats.truncated == 1u);
    CHECK(stats.v2Rows == FIRESTAFF_SCK_MAPFILE_MAX_ITEMS);
    CHECK(stats.parseError[0] != '\0');
    CHECK(stats.parseFailed == 1u);
}

static void test_v1_legacy_mapfile(void) {
    FirestaffSckCorpusFileStats stats;
    FirestaffSckCorpusFileResult r;

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpus_VerifyText("legacy.map", kLegacyMapfile, 1024u, &stats);
    CHECK(r == FIRESTAFF_SCK_CORPUS_FILE_OK);
    CHECK(stats.v1Rows == 3u);
    CHECK(stats.v2Rows == 0u);
    CHECK(stats.sizedRows == 0u);
    CHECK(stats.unsizedRows == 3u);
    CHECK(distinct_contains(stats.distinctTypes, stats.distinctTypeCount, "IMG1"));
    CHECK(distinct_contains(stats.distinctTypes, stats.distinctTypeCount, "IMG5"));
    CHECK(distinct_contains(stats.distinctTypes, stats.distinctTypeCount, "PAL"));
}

static void test_header_only_mapfile(void) {
    FirestaffSckCorpusFileStats stats;
    FirestaffSckCorpusFileResult r;

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpus_VerifyText("hdr.map", kHeaderOnlyMapfile, 1024u, &stats);
    /* Header-only V2 files fail the V2 item requirement; the
     * verifier surfaces this as PARSE_FAILED with the V2 FORMAT
     * captured anyway. */
    CHECK(r == FIRESTAFF_SCK_CORPUS_FILE_PARSE_FAILED ||
          r == FIRESTAFF_SCK_CORPUS_FILE_EMPTY);
    CHECK(stats.parseOk == 0u);
    CHECK(stats.v1Rows == 0u);
    CHECK(stats.v2Rows == 0u);
    CHECK_STR_EQ(stats.format, "ANIMATION");
}

static void test_dungeon_single_property(void) {
    FirestaffSckCorpusFileStats stats;
    FirestaffSckCorpusFileResult r;

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpus_VerifyText("dungeon.map", kDungeonHeaderMapfile, 1024u, &stats);
    CHECK(r != FIRESTAFF_SCK_CORPUS_FILE_OK);
    CHECK_STR_EQ(stats.format, "DUNGEON");
}

static void test_empty_mapfile(void) {
    FirestaffSckCorpusFileStats stats;
    FirestaffSckCorpusFileResult r;

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpus_VerifyText("empty.map", kEmptyMapfile, 1024u, &stats);
    CHECK(r == FIRESTAFF_SCK_CORPUS_FILE_EMPTY);
    CHECK(stats.parseOk == 0u);
}

static void test_garbage_mapfile(void) {
    FirestaffSckCorpusFileStats stats;
    FirestaffSckCorpusFileResult r;

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpus_VerifyText("junk.map",
                                      "this is not a mapfile\n!!!@@@###\n",
                                      1024u,
                                      &stats);
    CHECK(r == FIRESTAFF_SCK_CORPUS_FILE_PARSE_FAILED);
    CHECK(stats.parseOk == 0u);
}

static void test_null_args(void) {
    FirestaffSckCorpusSummary summary;
    FirestaffSckCorpusFileResult fr;
    FirestaffSckCorpusResult cr;

    fr = FirestaffSckCorpus_VerifyText("foo", "ENDIAN=BIG,FORMAT=DM\n", 0u, NULL);
    CHECK(fr == FIRESTAFF_SCK_CORPUS_FILE_PARSE_FAILED);

    memset(&summary, 0, sizeof(summary));
    cr = FirestaffSckCorpus_VerifyDirectory(NULL, 1024u, &summary);
    CHECK(cr == FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG);

    memset(&summary, 0, sizeof(summary));
    cr = FirestaffSckCorpus_VerifyDirectory(
        "/nonexistent/firestaff-corpus-xyz-12345", 1024u, &summary);
    CHECK(cr == FIRESTAFF_SCK_CORPUS_ERR_DIR_OPEN);

    memset(&summary, 0, sizeof(summary));
    cr = FirestaffSckCorpus_VerifyDirectory(NULL, 1024u, NULL);
    CHECK(cr == FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG);
}

static void test_result_strings(void) {
    /* Result-string surface must be stable for diagnostics. */
    CHECK_STR_EQ(FirestaffSckCorpus_ResultString(FIRESTAFF_SCK_CORPUS_OK), "OK");
    CHECK_STR_EQ(FirestaffSckCorpus_ResultString(FIRESTAFF_SCK_CORPUS_ERR_NO_CORPUS), "NO_CORPUS");
    CHECK_STR_EQ(FirestaffSckCorpus_ResultString(FIRESTAFF_SCK_CORPUS_ERR_DIR_OPEN), "DIR_OPEN");
    CHECK_STR_EQ(FirestaffSckCorpus_FileResultString(FIRESTAFF_SCK_CORPUS_FILE_OK), "OK");
    CHECK_STR_EQ(FirestaffSckCorpus_FileResultString(FIRESTAFF_SCK_CORPUS_FILE_EMPTY), "EMPTY");
    CHECK_STR_EQ(FirestaffSckCorpus_FileResultString(FIRESTAFF_SCK_CORPUS_FILE_TOO_LARGE), "TOO_LARGE");
    CHECK_STR_EQ(FirestaffSckCorpus_FileResultString(FIRESTAFF_SCK_CORPUS_FILE_PARSE_FAILED),
                 "PARSE_FAILED");
}

/* Build a tiny temp corpus with two synthetic mapfiles and run the
 * directory walker end-to-end so we exercise the per-FORMAT slot
 * path.  Uses mkdtemp / tmpnam-free primitives only. */
static int build_temp_corpus(char* outDir, size_t outBytes) {
#if defined(_WIN32)
    char tmpPath[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, tmpPath)) {
        return 0;
    }
    snprintf(outDir, outBytes, "%s\\firestaff_corpus_XXXXXX", tmpPath);
    if (!CreateDirectoryA(outDir, NULL)) {
        return 0;
    }
    return 1;
#else
    char tmpl[256];
    snprintf(tmpl, sizeof(tmpl), "/tmp/firestaff_corpus_XXXXXX");
    if (!mkdtemp(tmpl)) {
        return 0;
    }
    snprintf(outDir, outBytes, "%s", tmpl);
    return 1;
#endif
}

static void write_temp_file(const char* dir, const char* name, const char* text) {
    char path[512];
    FILE* f;
    size_t len;
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    f = fopen(path, "wb");
    if (!f) {
        return;
    }
    len = strlen(text);
    fwrite(text, 1u, len, f);
    fclose(f);
}

static void remove_temp_file(const char* dir, const char* name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    (void)remove(path);
}

static void test_directory_walk_per_format(void) {
    char dir[256];
    FirestaffSckCorpusSummary summary;
    FirestaffSckCorpusResult cr;
    const FirestaffSckCorpusFormatStats* fmt;
    int dir_ok;

    dir_ok = build_temp_corpus(dir, sizeof(dir));
    if (!dir_ok) {
        /* Skip on hosts without tmpdir support.  This is not a
         * failure of the verifier, so we just SKIP without
         * incrementing g_failures. */
        printf("SKIP directory walk (no tmpdir)\n");
        return;
    }

    write_temp_file(dir, "a.map", kSizedMapfile);
    write_temp_file(dir, "b.map", kLegacyMapfile);
    write_temp_file(dir, "c.map", kHeaderOnlyMapfile);
    write_temp_file(dir, "d.map", kDungeonHeaderMapfile);
    write_temp_file(dir, "not_a_map.txt", "ignore me");

    memset(&summary, 0, sizeof(summary));
    cr = FirestaffSckCorpus_VerifyDirectory(dir, 1024u, &summary);
    CHECK(cr == FIRESTAFF_SCK_CORPUS_OK);
    CHECK(summary.totalMapfiles == 4u);
    CHECK(summary.parseableMapfiles == 2u);
    CHECK(summary.unparseableMapfiles == 2u);
    CHECK(summary.v2Rows == 5u);
    CHECK(summary.v1Rows == 3u);
    CHECK(summary.sizedRows == 3u);
    CHECK(summary.unsizedRows == 5u);

    /* DMCSB1 should appear with parseableFileCount=1. */
    fmt = find_format(&summary, "DMCSB1");
    CHECK(fmt != NULL);
    if (fmt) {
        CHECK(fmt->fileCount == 1u);
        CHECK(fmt->parseableFileCount == 1u);
        CHECK(fmt->itemCount == 5u);
        CHECK(fmt->sizedRows == 3u);
        CHECK(fmt->unsizedRows == 2u);
    }
    /* ANIMATION should appear (header-only) with fileCount=1. */
    fmt = find_format(&summary, "ANIMATION");
    CHECK(fmt != NULL);
    if (fmt) {
        CHECK(fmt->fileCount == 1u);
        CHECK(fmt->parseableFileCount == 0u);
    }
    /* DUNGEON should appear. */
    fmt = find_format(&summary, "DUNGEON");
    CHECK(fmt != NULL);

    /* Distinct types include both V2 and V1 tags. */
    CHECK(distinct_contains(summary.distinctTypes, summary.distinctTypeCount, "IMG1"));
    CHECK(distinct_contains(summary.distinctTypes, summary.distinctTypeCount, "IMG3"));
    CHECK(distinct_contains(summary.distinctTypes, summary.distinctTypeCount, "PAL"));
    CHECK(distinct_contains(summary.distinctTypes, summary.distinctTypeCount, "RAW1"));
    /* V1 has no `IMG5` here because the V1 fixture uses it. */
    CHECK(distinct_contains(summary.distinctTypes, summary.distinctTypeCount, "IMG5"));
    /* Attribute prefixes preserved. */
    CHECK(distinct_contains(summary.distinctAttrPrefixes,
                            summary.distinctAttrPrefixCount,
                            "PAL"));
    CHECK(distinct_contains(summary.distinctAttrPrefixes,
                            summary.distinctAttrPrefixCount,
                            "SIZE"));

    remove_temp_file(dir, "a.map");
    remove_temp_file(dir, "b.map");
    remove_temp_file(dir, "c.map");
    remove_temp_file(dir, "d.map");
    remove_temp_file(dir, "not_a_map.txt");
    (void)remove(dir);
}

static void test_directory_no_corpus(void) {
    FirestaffSckCorpusSummary summary;
    FirestaffSckCorpusResult cr;
    char emptyDir[256];
    int dir_ok;

    dir_ok = build_temp_corpus(emptyDir, sizeof(emptyDir));
    if (!dir_ok) {
        printf("SKIP no-corpus walk (no tmpdir)\n");
        return;
    }
    memset(&summary, 0, sizeof(summary));
    cr = FirestaffSckCorpus_VerifyDirectory(emptyDir, 1024u, &summary);
    CHECK(cr == FIRESTAFF_SCK_CORPUS_ERR_NO_CORPUS);
    CHECK(summary.totalMapfiles == 0u);
    (void)remove(emptyDir);
}

static void test_resolve_default_dir(void) {
    char buf[1024];
    int ok;
    /* ResolveDefaultDir must never crash on NULL. */
    ok = FirestaffSckCorpus_ResolveDefaultDir(NULL, 0u);
    CHECK(ok == 0);
    /* When env is unset and HOME may or may not resolve to the
     * corpus cache, the function returns 0 or 1 -- but never
     * writes past the buffer. */
    buf[0] = '\0';
    ok = FirestaffSckCorpus_ResolveDefaultDir(buf, sizeof(buf));
    CHECK(ok == 0 || ok == 1);
    if (ok) {
        /* When 1, the buffer must hold a non-empty path. */
        CHECK(buf[0] != '\0');
    }
    /* Tiny buffer must not overflow. */
    ok = FirestaffSckCorpus_ResolveDefaultDir(buf, 4u);
    /* Either truncated or NULL: either is acceptable as long as
     * we never crash. */
    (void)ok;
}

int main(void) {
    test_v2_sized_mapfile();
    test_v2_oversized_slice();
    test_v2_truncated_mapfile();
    test_v1_legacy_mapfile();
    test_header_only_mapfile();
    test_dungeon_single_property();
    test_empty_mapfile();
    test_garbage_mapfile();
    test_null_args();
    test_result_strings();
    test_directory_walk_per_format();
    test_directory_no_corpus();
    test_resolve_default_dir();
    if (g_failures) {
        printf("test_firestaff_sck_mapfile_corpus_verifier: FAIL %d\n", g_failures);
        return 1;
    }
    puts("test_firestaff_sck_mapfile_corpus_verifier: PASS");
    return 0;
}
