#include "firestaff_sck_corpus_verifier.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(int ok, const char* name) {
    if (!ok) {
        ++failures;
        printf("FAIL %s\n", name);
    } else {
        printf("PASS %s\n", name);
    }
}

static void test_verify_sck2(void) {
    const char* text =
        "MAPFORMATVERSION=2.0,MAPVERSION=1.0,FORMAT=EXE,ENDIAN=BIG,CLOCKMODE=PAL\n"
        "000000,RAW1,SIZE=12288,Unknown,Not yet decoded,\n"
        "012288,IMG1,SIZE=2560,Dungeon Graphics,Ceiling,\n"
        "014848,IMG1,SIZE=68,Dungeon Graphics,Floor Pit,Left Side 3\n"
        "999999,IMG3,NULL,No bounds,,,\n";
    FirestaffSckCorpusVerifierMapStats stats;
    FirestaffSckCorpusVerifierResult r;

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpusVerifier_VerifyMapfileText(
        text, 200000u, &stats);
    check(r == FIRESTAFF_SCK_CORPUS_OK, "SCK 2.x mapfile accepted");
    check(stats.parseableRows == 4u, "4 parseable rows counted");
    check(stats.sizedRows == 3u, "3 sized rows counted (SIZE=)");
    check(stats.unsizedRows == 1u, "1 unsized row counted (NULL/no SIZE=)");
    check(stats.oversizedRows == 0u, "no oversized rows against 200 KB target");
    check(stats.preserveRawAttributeRows == 4u,
          "all 4 rows preserve raw attributes (NULL is itself a value)");
    check(strcmp(stats.format, "EXE") == 0, "format property captured");
    check(strcmp(stats.endian, "BIG") == 0, "endian property captured");
    check(strcmp(stats.firstItemNumber, "000000") == 0, "first item number captured");
    check(strcmp(stats.firstItemType, "RAW1") == 0, "first item type captured");
    check(strcmp(stats.firstItemDescription, "Unknown") == 0,
          "first item description captured");
    check(stats.parseError[0] == '\0', "no parse error captured");
}

static void test_verify_sck2_oversized(void) {
    const char* text =
        "ENDIAN=LITTLE,FORMAT=DMCSB2\n"
        "0000,IMG3,PAL1,Dialog Box,,\n"
        "0010,IMG3,PAL1,Title,,,\n"
        "9999,IMG3,SIZE=4096,Oversized,,,\n";
    FirestaffSckCorpusVerifierMapStats stats;
    FirestaffSckCorpusVerifierResult r;

    memset(&stats, 0, sizeof(stats));
    /* Force the last row to be oversized by passing a tiny target. */
    r = FirestaffSckCorpusVerifier_VerifyMapfileText(text, 20u, &stats);
    check(r == FIRESTAFF_SCK_CORPUS_OK, "SCK 2.x mapfile accepted with tiny target");
    check(stats.parseableRows == 3u, "3 parseable rows counted");
    check(stats.sizedRows == 1u, "1 sized row counted");
    check(stats.unsizedRows == 2u, "2 unsized rows counted");
    check(stats.oversizedRows == 1u, "1 oversized row counted against 20 B target");
}

static void test_verify_sck2_preserve_raw_attributes(void) {
    const char* text =
        "ENDIAN=BIG,FORMAT=DMCSB1\n"
        "0000,IMG1,PAL=DM_PAL_TITLE,Title,,,\n"
        "0001,IMG1,,Title Without Attrs,,,\n"
        "0002,IMG1,PAL1&SIZE=128,Dungeon Graphics,Ceiling,\n";
    FirestaffSckCorpusVerifierMapStats stats;
    FirestaffSckCorpusVerifierResult r;

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpusVerifier_VerifyMapfileText(
        text, 100000u, &stats);
    check(r == FIRESTAFF_SCK_CORPUS_OK, "raw-attr SCK 2.x mapfile accepted");
    check(stats.parseableRows == 3u, "3 parseable rows counted");
    check(stats.preserveRawAttributeRows == 2u,
          "2 rows preserve raw attributes (third is empty)");
    check(stats.sizedRows == 1u, "1 sized row counted");
    check(stats.unsizedRows == 2u, "2 unsized rows counted");
}

static void test_verify_legacy(void) {
    const char* text =
        "ENDIAN=BIG,FORMAT=EXE\n"
        "IMG1 image00 0 256\n"
        "IMG5 image01 0x100 32\n";
    FirestaffSckCorpusVerifierMapStats stats;
    FirestaffSckCorpusVerifierResult r;

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpusVerifier_VerifyMapfileText(text, 512u, &stats);
    check(r == FIRESTAFF_SCK_CORPUS_OK, "legacy mapfile accepted");
    check(stats.parseableRows == 2u, "2 parseable rows counted");
    check(stats.sizedRows == 2u, "all legacy rows are sized");
    check(stats.unsizedRows == 0u, "no unsized legacy rows");
    check(stats.oversizedRows == 0u, "no oversized legacy rows against 512 B target");
    check(strcmp(stats.firstItemType, "IMG1") == 0,
          "first legacy item type captured");
    check(strcmp(stats.firstItemDescription, "image00") == 0,
          "first legacy item name captured as description");
}

static void test_verify_bad_header(void) {
    const char* text = "not a mapfile at all, just prose\n";
    FirestaffSckCorpusVerifierMapStats stats;
    FirestaffSckCorpusVerifierResult r;

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpusVerifier_VerifyMapfileText(text, 0u, &stats);
    check(r == FIRESTAFF_SCK_CORPUS_ERR_BAD_HEADER,
          "free-form prose rejected as bad header");
}

static void test_verify_null_arg(void) {
    FirestaffSckCorpusVerifierMapStats stats;
    FirestaffSckCorpusVerifierResult r;

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpusVerifier_VerifyMapfileText(NULL, 0u, &stats);
    check(r == FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG, "NULL text rejected");

    memset(&stats, 0, sizeof(stats));
    r = FirestaffSckCorpusVerifier_VerifyMapfilePath(NULL, 0u, &stats);
    check(r == FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG, "NULL path rejected");
}

static void test_result_strings(void) {
    check(strcmp(FirestaffSckCorpusVerifier_ResultString(
                     FIRESTAFF_SCK_CORPUS_OK), "OK") == 0,
          "result string OK");
    check(strcmp(FirestaffSckCorpusVerifier_ResultString(
                     FIRESTAFF_SCK_CORPUS_ERR_BAD_HEADER), "bad header") == 0,
          "result string BAD_HEADER");
    check(strcmp(FirestaffSckCorpusVerifier_ResultString(
                     FIRESTAFF_SCK_CORPUS_ERR_OPEN_FILE), "open file failed") == 0,
          "result string OPEN_FILE");
    check(strcmp(FirestaffSckCorpusVerifier_ResultString(
                     FIRESTAFF_SCK_CORPUS_ERR_DIR_OPEN_FAILED),
                 "directory open failed") == 0,
          "result string DIR_OPEN_FAILED");
}

int main(void) {
    test_verify_sck2();
    test_verify_sck2_oversized();
    test_verify_sck2_preserve_raw_attributes();
    test_verify_legacy();
    test_verify_bad_header();
    test_verify_null_arg();
    test_result_strings();
    if (failures) {
        printf("test_firestaff_sck_corpus_verifier: FAIL %d\n", failures);
        return 1;
    }
    puts("test_firestaff_sck_corpus_verifier: PASS");
    return 0;
}
