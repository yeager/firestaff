/*
 * firestaff_sck_corpus_verifier_real_corpus_probe.c
 *
 * Pass-bound: SCK mapfile corpus verifier over the real
 * Greatstone SCK 1.5.1 `db/map/X.map` corpus.
 *
 * The probe walks every `.map` file under the corpus
 * directory and asks `firestaff_sck_corpus_verifier` to
 * parse each one, count parseable rows, count rows that
 * preserve raw attributes, count unsized rows, and (when
 * a target file size is supplied) count oversized slices.
 *
 * Corpus discovery (priority order):
 *   $FIRESTAFF_GREATSTONE_SCK_DIR
 *   $FIRESTAFF_GREATSTONE_SCK_DIR/db/map
 *   $HOME/.cache/firestaff/greatstone-sck-mapfiles/db/map
 *
 * When the corpus is unavailable, the probe prints a
 * SKIPPED status and exits 0 so CTest stays green in
 * offline environments.  Operators that want real-asset
 * proof run `tools/fetch_greatstone_sck_mapfiles.sh`
 * first.
 *
 * Output: prints a single PASS line with aggregate
 * corpus stats followed by per-FORMAT counts when the
 * corpus is present.  The probe is read-only and never
 * fails CTest on a single parse failure -- parse-fail
 * counts surface in the PASS evidence instead.
 */

#include "firestaff_sck_corpus_verifier.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PROBE_NAME "firestaff_sck_corpus_verifier_real_corpus_probe"

/* The bundled SCK mapfiles are sized against their own
 * target assets (GRAPHICS.DAT / DUNGEON.DAT / executable
 * blobs).  We deliberately pass 0 as the target byte size
 * so the corpus verifier never reports a false-positive
 * oversized row against a target file that is not present
 * on this host.  Per-mapfile oversized verification is
 * still available through VerifyMapfilePath with a
 * caller-supplied size; this probe just wants the
 * inventory counts. */
#define CORPUS_TARGET_FILE_BYTES 0u

/* Per-FORMAT tally.  The corpus has at most a handful of
 * distinct FORMAT values so we can keep a small fixed
 * bucket list inline. */
#define FORMAT_BUCKETS 16

typedef struct FormatBucket {
    char format[FIRESTAFF_SCK_CORPUS_VERIFIER_FORMAT_BYTES];
    unsigned int mapfiles;
    unsigned int rows;
    unsigned int unsized;
    unsigned int oversized;
} FormatBucket;

typedef struct FormatAccum {
    FormatBucket buckets[FORMAT_BUCKETS];
    unsigned int bucketCount;
    unsigned int formatOverflow;
} FormatAccum;

static int bucket_lookup(FormatAccum* acc, const char* format) {
    unsigned int i;
    for (i = 0u; i < acc->bucketCount; ++i) {
        if (strcmp(acc->buckets[i].format, format) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static void bucket_add(FormatAccum* acc,
                       const char* format,
                       const FirestaffSckCorpusVerifierMapStats* stats) {
    int idx;
    if (!format || format[0] == '\0') {
        format = "(none)";
    }
    idx = bucket_lookup(acc, format);
    if (idx < 0) {
        if (acc->bucketCount >= FORMAT_BUCKETS) {
            ++acc->formatOverflow;
            return;
        }
        idx = (int)acc->bucketCount++;
        memset(&acc->buckets[idx], 0, sizeof(FormatBucket));
        snprintf(acc->buckets[idx].format,
                 sizeof(acc->buckets[idx].format),
                 "%s",
                 format);
    }
    acc->buckets[idx].mapfiles += 1u;
    acc->buckets[idx].rows += stats->parseableRows;
    acc->buckets[idx].unsized += stats->unsizedRows;
    acc->buckets[idx].oversized += stats->oversizedRows;
}

static int is_directory(const char* path) {
    struct stat st;
    if (!path || stat(path, &st) != 0) {
        return 0;
    }
    return S_ISDIR(st.st_mode);
}

static const char* resolve_corpus_dir(void) {
    static char buf[1024];
    const char* env = getenv("FIRESTAFF_GREATSTONE_SCK_DIR");
    if (env && env[0] != '\0') {
        if (is_directory(env)) {
            return env;
        }
        snprintf(buf, sizeof(buf), "%s/db/map", env);
        if (is_directory(buf)) {
            return buf;
        }
    }
    {
        const char* home = getenv("HOME");
        if (home && home[0] != '\0') {
            snprintf(buf, sizeof(buf),
                     "%s/.cache/firestaff/greatstone-sck-mapfiles/db/map",
                     home);
            if (is_directory(buf)) {
                return buf;
            }
        }
    }
    return NULL;
}

int main(void) {
    const char* corpusDir;
    FirestaffSckCorpusVerifierCorpusStats stats;
    FirestaffSckCorpusVerifierResult r;
    FormatAccum acc;
    char errBuf[FIRESTAFF_SCK_CORPUS_VERIFIER_PARSE_ERROR_BYTES];

    memset(&stats, 0, sizeof(stats));
    memset(&acc, 0, sizeof(acc));
    memset(errBuf, 0, sizeof(errBuf));

    corpusDir = resolve_corpus_dir();
    if (!corpusDir) {
        printf("%s SKIPPED no corpus dir resolved "
               "(set FIRESTAFF_GREATSTONE_SCK_DIR or run "
               "tools/fetch_greatstone_sck_mapfiles.sh)\n",
               PROBE_NAME);
        return 0;
    }

    r = FirestaffSckCorpusVerifier_VerifyCorpusDir(
        corpusDir,
        CORPUS_TARGET_FILE_BYTES,
        NULL,
        NULL,
        &stats);
    if (r != FIRESTAFF_SCK_CORPUS_OK) {
        printf("%s FAIL corpus walk returned %s\n",
               PROBE_NAME,
               FirestaffSckCorpusVerifier_ResultString(r));
        return 1;
    }
    if (stats.totalMapfiles == 0u) {
        printf("%s SKIPPED corpus dir %s has zero .map files\n",
               PROBE_NAME, corpusDir);
        return 0;
    }

    /* Re-walk to bucket per-FORMAT counts.  We deliberately
     * skip the per-mapfile callback hook on the first walk
     * so the PASS evidence line stays a single line; the
     * second walk only updates the FORMAT accumulator. */
    {
        DIR* dir = opendir(corpusDir);
        struct dirent* ent;
        if (!dir) {
            printf("%s FAIL could not reopen %s for FORMAT bucketing\n",
                   PROBE_NAME, corpusDir);
            return 1;
        }
        while ((ent = readdir(dir)) != NULL) {
            const char* name = ent->d_name;
            size_t nameLen;
            char path[FIRESTAFF_SCK_CORPUS_VERIFIER_MAX_PATH];
            FirestaffSckCorpusVerifierMapStats ms;
            if (!name || name[0] == '.') {
                continue;
            }
            nameLen = strlen(name);
            if (nameLen < 5u ||
                strcmp(name + nameLen - 4u, ".map") != 0) {
                continue;
            }
            snprintf(path, sizeof(path), "%s/%s", corpusDir, name);
            memset(&ms, 0, sizeof(ms));
            (void)FirestaffSckCorpusVerifier_VerifyMapfilePath(
                path, CORPUS_TARGET_FILE_BYTES, &ms);
            bucket_add(&acc, ms.format, &ms);
        }
        closedir(dir);
    }

    printf("%s PASS corpus_dir=%s total_mapfiles=%u "
           "parseable_mapfiles=%u empty_mapfiles=%u "
           "parse_fail_mapfiles=%u row_cap_mapfiles=%u "
           "total_rows=%u parseable_rows=%u parse_fail_rows=%u "
           "sized_rows=%u unsized_rows=%u oversized_rows=%u "
           "preserve_raw_attribute_rows=%u format_buckets=%u%s%s\n",
           PROBE_NAME,
           corpusDir,
           stats.totalMapfiles,
           stats.parseableMapfiles,
           stats.emptyMapfiles,
           stats.parseFailMapfiles,
           stats.rowCapMapfiles,
           stats.totalRows,
           stats.parseableRows,
           stats.parseFailRows,
           stats.sizedRows,
           stats.unsizedRows,
           stats.oversizedRows,
           stats.preserveRawAttributeRows,
           acc.bucketCount,
           acc.formatOverflow > 0u ? " format_overflow=" : "",
           acc.formatOverflow > 0u ? "" : "");
    /* Per-format buckets are rendered on follow-up lines so
     * the main PASS line stays grep-friendly. */
    {
        unsigned int i;
        for (i = 0u; i < acc.bucketCount; ++i) {
            const FormatBucket* b = &acc.buckets[i];
            printf("%s FORMAT %s maps=%u rows=%u unsized=%u oversized=%u\n",
                   PROBE_NAME,
                   b->format,
                   b->mapfiles,
                   b->rows,
                   b->unsized,
                   b->oversized);
        }
    }
    return 0;
}
