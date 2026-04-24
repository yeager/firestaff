/*
 * firestaff_v1_song_dat_probe — Pass 50 verification probe.
 *
 * Verifies that the Firestaff SONG.DAT loader parses a real original
 * DM PC v3.4 SONG.DAT file correctly:
 *   - header signature, item count, offsets/sizes
 *   - SEQ2 parse produces a well-formed music sequence with end marker
 *   - each SND8 item decodes the declared number of samples exactly
 *
 * The original SONG.DAT is NOT checked in.  The probe is strictly a
 * run-time check.  Without the path it reports the environment
 * requirement and exits 0 with a SKIP line — no asset is implied.
 *
 * Path resolution:
 *   1. SONG_DAT_PATH environment variable
 *   2. /tmp/fs_pass50_extract/dm_dos/DungeonMasterPC34/DATA/SONG.DAT
 *      (the default local extraction location this repository's
 *      parity-evidence uses for Pass 50 verification)
 *
 * Exit codes: 0 on PASS or SKIP, 1 on FAIL.
 */

#include "../../song_dat_loader_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Expected verified header for DM PC v3.4 EN SONG.DAT.
 * (See DM1_SONG_DAT_FORMAT.md §2; verified empirically against the real
 * file shipped in the DM1 DOS package.)
 */
typedef struct {
    unsigned long   fileOffset;
    unsigned int    compressedBytes;
    unsigned int    decompressedBytes;
    unsigned short  attr0;
    unsigned short  attr1;
} ExpectedItem;

static const ExpectedItem kExpected[V1_SONG_DAT_ITEM_COUNT] = {
    {     84,    40,    40, 0x0001, 0x0002 },   /* SEQ2 */
    {    124,  3192,  6374, 0xE418, 0x0000 },   /* SND8 1 */
    {   3316, 16504, 29256, 0x4672, 0x3372 },   /* SND8 2 */
    {  19820,  5640,  9787, 0x3926, 0xFE60 },   /* SND8 3 */
    {  25460, 29141, 37890, 0x0094, 0x2261 },   /* SND8 4 */
    {  54601, 33315, 43565, 0x2BAA, 0xB080 },   /* SND8 5 */
    {  87916,  3506,  7002, 0x581B, 0x708F },   /* SND8 6 */
    {  91422,   272,   537, 0x1702, 0x708F },   /* SND8 7 */
    {  91694, 32740, 38658, 0x0097, 0x1572 },   /* SND8 8 */
    { 124434, 38048, 46367, 0x1DB5, 0x57A2 },   /* SND8 9 */
};

static const unsigned int kExpectedSndSamples[V1_SONG_DAT_ITEM_COUNT] = {
    0,     /* seq */
    6372, 29254, 9785, 37888, 43563, 7000, 535, 38656, 46365
};

/* Observed verified SEQ2 sequence in DM PC v3.4 EN SONG.DAT. */
static const unsigned short kExpectedSeqWords[] = {
    0x0001, 0x0002, 0x0003, 0x0002, 0x0003, 0x0002, 0x0003, 0x0002,
    0x0004, 0x0005, 0x0006, 0x0002, 0x0003, 0x0002, 0x0004, 0x0005,
    0x0007, 0x0008, 0x0009, 0x8001
};
#define EXPECTED_SEQ_WORD_COUNT \
    (sizeof(kExpectedSeqWords) / sizeof(kExpectedSeqWords[0]))

static int g_passCount = 0;
static int g_failCount = 0;

static void report_pass(const char* name, const char* detail) {
    if (detail && detail[0])
        printf("PASS %s %s\n", name, detail);
    else
        printf("PASS %s\n", name);
    g_passCount++;
}

static void report_fail(const char* name, const char* detail) {
    if (detail && detail[0])
        printf("FAIL %s %s\n", name, detail);
    else
        printf("FAIL %s\n", name);
    g_failCount++;
}

static const char* resolve_song_path(void) {
    const char* env = getenv("SONG_DAT_PATH");
    if (env && env[0]) return env;
    return "/tmp/fs_pass50_extract/dm_dos/DungeonMasterPC34/DATA/SONG.DAT";
}

int main(void) {
    const char* path = resolve_song_path();
    FILE* probe = fopen(path, "rb");
    if (!probe) {
        printf("SKIP INV_V1_SONG_01 original SONG.DAT not present at %s\n", path);
        printf("  set SONG_DAT_PATH=<path to original SONG.DAT> to run this probe\n");
        printf("  format/decoder remain covered by DM1_SONG_DAT_FORMAT.md\n");
        printf("# summary: skipped (no original asset present) — not a failure\n");
        return 0;
    }
    fclose(probe);

    V1_SongManifest manifest;
    char err[256];
    if (!V1_Song_ParseManifest(path, &manifest, err, sizeof(err))) {
        report_fail("INV_V1_SONG_01", err);
        printf("# summary: %d/%d invariants passed\n",
               g_passCount, g_passCount + g_failCount);
        return 1;
    }
    report_pass("INV_V1_SONG_01", "manifest parsed");

    /* File layout invariant. */
    if (manifest.fileBytes == V1_SONG_DAT_EXPECTED_SIZE) {
        report_pass("INV_V1_SONG_02",
                    "file size matches DM PC v3.4 EN canonical value");
    } else {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "file size=%lu, expected=%u",
                 manifest.fileBytes, V1_SONG_DAT_EXPECTED_SIZE);
        report_fail("INV_V1_SONG_02", buf);
    }

    if (manifest.signature == V1_SONG_DAT_SIGNATURE &&
        manifest.itemCount == V1_SONG_DAT_ITEM_COUNT &&
        manifest.headerBytes == 84) {
        report_pass("INV_V1_SONG_03",
                    "DMCSB2 signature=0x8001, 10 items, 84-byte header");
    } else {
        report_fail("INV_V1_SONG_03", "header invariants not met");
    }

    /* Per-item layout. */
    int itemsOk = 1;
    for (unsigned int i = 0; i < V1_SONG_DAT_ITEM_COUNT; ++i) {
        const V1_SongItemHeader* got = &manifest.items[i];
        const ExpectedItem*      exp = &kExpected[i];
        if (got->fileOffset        != exp->fileOffset        ||
            got->compressedBytes   != exp->compressedBytes   ||
            got->decompressedBytes != exp->decompressedBytes ||
            got->attr0             != exp->attr0             ||
            got->attr1             != exp->attr1) {
            char buf[160];
            snprintf(buf, sizeof(buf),
                     "item %u mismatch: off=%lu comp=%u decomp=%u "
                     "attr=%04x/%04x vs expected off=%lu comp=%u decomp=%u "
                     "attr=%04x/%04x",
                     i, got->fileOffset, got->compressedBytes,
                     got->decompressedBytes, got->attr0, got->attr1,
                     exp->fileOffset, exp->compressedBytes,
                     exp->decompressedBytes, exp->attr0, exp->attr1);
            report_fail("INV_V1_SONG_04", buf);
            itemsOk = 0;
            break;
        }
    }
    if (itemsOk) {
        report_pass("INV_V1_SONG_04", "all 10 item headers match verified table");
    }

    /* SEQ2 decode. */
    V1_SongSequence seq;
    if (!V1_Song_DecodeSequence(path, &manifest, &seq, err, sizeof(err))) {
        report_fail("INV_V1_SONG_05", err);
    } else if (!seq.hasEndMarker) {
        report_fail("INV_V1_SONG_05", "SEQ2 missing end-marker word");
    } else if (seq.wordCount != EXPECTED_SEQ_WORD_COUNT) {
        char buf[96];
        snprintf(buf, sizeof(buf), "SEQ2 word count=%u, expected=%u",
                 seq.wordCount, (unsigned)EXPECTED_SEQ_WORD_COUNT);
        report_fail("INV_V1_SONG_05", buf);
    } else {
        int seqOk = 1;
        for (unsigned int i = 0; i < seq.wordCount; ++i) {
            if (seq.words[i] != kExpectedSeqWords[i]) {
                seqOk = 0;
                break;
            }
        }
        if (seqOk) {
            report_pass("INV_V1_SONG_05",
                        "SEQ2 music sequence matches verified 20-word pattern");
        } else {
            report_fail("INV_V1_SONG_05", "SEQ2 words do not match");
        }
    }

    /* SND8 decode for every sample item. */
    int allSndOk = 1;
    for (unsigned int i = V1_SONG_DAT_FIRST_SND8_INDEX;
         i <= V1_SONG_DAT_LAST_SND8_INDEX; ++i) {
        V1_SndBuffer b;
        if (!V1_Song_DecodeSnd8(path, &manifest, i, &b, err, sizeof(err))) {
            char buf[160];
            snprintf(buf, sizeof(buf), "SND8 item %u decode failed: %s",
                     i, err);
            report_fail("INV_V1_SONG_06", buf);
            allSndOk = 0;
            break;
        }
        if (b.sampleRateHz != V1_SONG_DAT_SAMPLE_RATE_HZ) {
            char buf[96];
            snprintf(buf, sizeof(buf),
                     "SND8 item %u rate=%u (expected %u)",
                     i, b.sampleRateHz, V1_SONG_DAT_SAMPLE_RATE_HZ);
            report_fail("INV_V1_SONG_06", buf);
            V1_Song_FreeSndBuffer(&b);
            allSndOk = 0;
            break;
        }
        if (b.declaredSampleCount != kExpectedSndSamples[i] ||
            b.decodedSampleCount  != kExpectedSndSamples[i]) {
            char buf[160];
            snprintf(buf, sizeof(buf),
                     "SND8 item %u: declared=%u decoded=%u expected=%u",
                     i, b.declaredSampleCount, b.decodedSampleCount,
                     kExpectedSndSamples[i]);
            report_fail("INV_V1_SONG_06", buf);
            V1_Song_FreeSndBuffer(&b);
            allSndOk = 0;
            break;
        }
        V1_Song_FreeSndBuffer(&b);
    }
    if (allSndOk) {
        report_pass("INV_V1_SONG_06",
                    "all 9 SND8 items decode to exactly declared sample count at 11025 Hz");
    }

    /* Summary. */
    int total = g_passCount + g_failCount;
    printf("# summary: %d/%d invariants passed\n", g_passCount, total);
    return g_failCount == 0 ? 0 : 1;
}
