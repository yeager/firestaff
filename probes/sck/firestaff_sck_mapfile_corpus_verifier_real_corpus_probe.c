/*
 * firestaff_sck_mapfile_corpus_verifier_real_corpus_probe.c
 *
 * Pass-bound corpus verifier for the Greatstone/SCK db/map corpus
 * fetched by `tools/fetch_greatstone_sck_mapfiles.sh`.
 *
 * Walks every `*.map` file under the resolved corpus directory,
 * parses it with the bounded corpus verifier, prints a one-line
 * aggregate summary plus per-FORMAT breakdown, and exits 0.
 *
 * Corpus discovery:
 *   The probe honors, in priority order:
 *     1. $FIRESTAFF_GREATSTONE_SCK_DIR/db/map (env points at sck root)
 *     2. $FIRESTAFF_GREATSTONE_SCK_DIR               (env points at db/map)
 *     3. $HOME/.cache/firestaff/greatstone-sck-mapfiles/db/map
 *
 * When the corpus is unavailable, the probe prints a SKIPPED
 * status and exits 0 so CTest stays green in offline environments.
 * Operators that want real-asset proof run
 * `tools/fetch_greatstone_sck_mapfiles.sh` first.
 *
 * Skip-safe contract:
 *   - empty/missing corpus dir -> SKIPPED, exit 0
 *   - parse errors are reported via the bounded file-level result
 *     strings and do not flip the exit code (the corpus is
 *     intentionally heterogeneous: ANIMATION / DUNGEON / CMP /
 *     SAVEGAME entries are header-only)
 *
 * Source-of-truth references:
 *   - greatstone d_mapfile.html (mapfile 2.x shape, FORMAT/ENDIAN
 *     header properties, six-field item rows, SIZE= attribute
 *     shape used by the slice selector).
 *   - greatstone d_items.html (item-type inventory: RAW1/IMG1/
 *     IMG3/IMG5/IMG14, FTL/PAL/SND/CMP/SEQ1/ROMIMG1/etc.).
 *   - tools/fetch_greatstone_sck_mapfiles.sh (corpus fetch
 *     contract: 90 .map files + _mapping.xml).
 *   - src/shared/firestaff_sck_mapfile.c (the underlying
 *     parser the verifier consumes; intentionally kept thin and
 *     slice-bounded).
 *   - src/shared/firestaff_sck_mapfile_corpus_verifier.c (the
 *     bounded per-file / per-FORMAT aggregate module).
 *
 * Output:
 *   PROBE_NAME <aggregate line>
 *   <per-FORMAT lines, sorted by file count>
 *   VERDICT <verdict line>
 */

#include "firestaff_sck_mapfile_corpus_verifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROBE_NAME "firestaff_sck_mapfile_corpus_verifier_real_corpus_probe"
#define SYNTHETIC_TARGET_FILE_BYTES (16u * 1024u * 1024u)

static int g_failures = 0;

static void pass_invariant(const char* name, const char* detail) {
    printf("PASS %s %s\n", name, detail ? detail : "");
}

static void fail_invariant(const char* name, const char* detail) {
    ++g_failures;
    printf("FAIL %s %s\n", name, detail ? detail : "");
}

static int fmt_lex_less(const void* a, const void* b) {
    const FirestaffSckCorpusFormatStats* fa =
        (const FirestaffSckCorpusFormatStats*)a;
    const FirestaffSckCorpusFormatStats* fb =
        (const FirestaffSckCorpusFormatStats*)b;
    if (fa->fileCount != fb->fileCount) {
        return (fa->fileCount > fb->fileCount) ? -1 : 1;
    }
    return strcmp(fa->format, fb->format);
}

static int distinct_contains(const char distinct[][FIRESTAFF_SCK_CORPUS_TYPE_BYTES],
                             unsigned int count,
                             const char* needle) {
    unsigned int i;
    for (i = 0u; i < count; ++i) {
        if (strcmp(distinct[i], needle) == 0) {
            return 1;
        }
    }
    return 0;
}

int main(void) {
    char corpusDir[1024];
    FirestaffSckCorpusSummary summary;
    FirestaffSckCorpusResult cr;
    unsigned int i;
    unsigned int totalParseableFiles;

    if (!FirestaffSckCorpus_ResolveDefaultDir(corpusDir, sizeof(corpusDir))) {
        printf("%s SKIPPED no corpus dir resolved "
               "(run tools/fetch_greatstone_sck_mapfiles.sh)\n",
               PROBE_NAME);
        return 0;
    }

    memset(&summary, 0, sizeof(summary));
    cr = FirestaffSckCorpus_VerifyDirectory(corpusDir,
                                            SYNTHETIC_TARGET_FILE_BYTES,
                                            &summary);
    if (cr == FIRESTAFF_SCK_CORPUS_ERR_NO_CORPUS) {
        printf("%s SKIPPED corpus dir %s has no .map entries\n",
               PROBE_NAME, corpusDir);
        return 0;
    }
    if (cr == FIRESTAFF_SCK_CORPUS_ERR_DIR_OPEN) {
        printf("%s SKIPPED corpus dir %s could not be opened\n",
               PROBE_NAME, corpusDir);
        return 0;
    }
    if (cr != FIRESTAFF_SCK_CORPUS_OK) {
        printf("%s FAIL verify directory: %s\n", PROBE_NAME,
               FirestaffSckCorpus_ResultString(cr));
        return 1;
    }

    /* Sort per-FORMAT entries by file count (descending) for
     * stable, human-friendly output. */
    if (summary.formatCount > 1u) {
        qsort(summary.formats,
              summary.formatCount,
              sizeof(FirestaffSckCorpusFormatStats),
              fmt_lex_less);
    }

    totalParseableFiles = summary.parseableMapfiles;

    printf("%s corpus=%s total=%u parseable=%u unparseable=%u "
           "truncated=%u v2_rows=%u v1_rows=%u sized=%u unsized=%u "
           "oversized=%u distinct_types=%u distinct_attrs=%u "
           "formats=%u\n",
           PROBE_NAME,
           corpusDir,
           summary.totalMapfiles,
           summary.parseableMapfiles,
           summary.unparseableMapfiles,
           summary.truncatedMapfiles,
           summary.v2Rows,
           summary.v1Rows,
           summary.sizedRows,
           summary.unsizedRows,
           summary.oversizedRows,
           summary.distinctTypeCount,
           summary.distinctAttrPrefixCount,
           summary.formatCount);

    for (i = 0u; i < summary.formatCount; ++i) {
        const FirestaffSckCorpusFormatStats* f = &summary.formats[i];
        printf("%s FORMAT=%-12s files=%u parseable=%u truncated=%u "
               "items=%u sized=%u unsized=%u oversized=%u\n",
               PROBE_NAME,
               f->format,
               f->fileCount,
               f->parseableFileCount,
               f->truncatedFileCount,
               f->itemCount,
               f->sizedRows,
               f->unsizedRows,
               f->oversizedRows);
    }

    /* Invariants (the probe does not fail CTest just because the
     * corpus shape changed; we surface regressions as FAIL lines
     * so operators can see them and let the existing CTest gate
     * keep working in offline environments). */
    if (summary.totalMapfiles >= 1u) {
        pass_invariant("corpus_present", corpusDir);
    } else {
        fail_invariant("corpus_present", corpusDir);
    }
    if (summary.sizedRows >= 1u) {
        pass_invariant("sized_rows_present", NULL);
    } else {
        fail_invariant("sized_rows_present", NULL);
    }
    if (distinct_contains(summary.distinctTypes, summary.distinctTypeCount, "IMG1")) {
        pass_invariant("IMG1_type_present", NULL);
    } else {
        fail_invariant("IMG1_type_present", NULL);
    }
    if (distinct_contains(summary.distinctTypes, summary.distinctTypeCount, "RAW1")) {
        pass_invariant("RAW1_type_present", NULL);
    } else {
        fail_invariant("RAW1_type_present", NULL);
    }
    if (distinct_contains(summary.distinctAttrPrefixes,
                          summary.distinctAttrPrefixCount,
                          "SIZE")) {
        pass_invariant("SIZE_attribute_prefix_present", NULL);
    } else {
        fail_invariant("SIZE_attribute_prefix_present", NULL);
    }
    if (totalParseableFiles + summary.unparseableMapfiles ==
        summary.totalMapfiles) {
        pass_invariant("parseable_unparseable_partition_complete", NULL);
    } else {
        fail_invariant("parseable_unparseable_partition_complete", NULL);
    }
    if (summary.truncatedMapfiles <= summary.unparseableMapfiles) {
        pass_invariant("truncated_subset_of_unparseable", NULL);
    } else {
        fail_invariant("truncated_subset_of_unparseable", NULL);
    }

    if (g_failures) {
        printf("%s FAIL %d invariant(s)\n", PROBE_NAME, g_failures);
        return 1;
    }
    printf("%s PASS corpus aggregate line above\n", PROBE_NAME);
    return 0;
}
