#ifndef FIRESTAFF_SCK_CORPUS_VERIFIER_H
#define FIRESTAFF_SCK_CORPUS_VERIFIER_H

/*
 * firestaff_sck_corpus_verifier.h
 *
 * Bounded corpus verifier for the Greatstone/SCK bundled
 * `db/map/X.map` corpus.
 *
 * Scope (this commit):
 *   - Walk every `.map` file under a corpus directory
 *     (or a single explicit path) and parse it through
 *     `firestaff_sck_mapfile`.
 *   - For SCK 2.x comma-rows, count parseable rows,
 *     preserve raw attributes verbatim, and classify
 *     each row as sized (SIZE=) / unsized / oversized
 *     against a caller-provided target file byte size.
 *   - Track per-mapfile stats + an aggregate corpus
 *     stats struct so callers can render a structured
 *     PASS evidence line.
 *   - Never panic; never exit non-zero on a parse
 *     failure.  A row parse failure or oversized slice
 *     is reported as a count, not as a CTest failure,
 *     because the corpus contains both legacy 4-token
 *     rows and SCK 2.x comma-rows and the parser must
 *     pick the right one per mapfile.
 *
 * Non-scope (kept out by design to bound this commit):
 *   - No mapfile-to-asset-loader handoff (owned by
 *     `firestaff_sck_asset_bridge`).
 *   - No per-asset-type decoder wiring.
 *   - No item extraction / decode / runtime loading.
 *   - No Greatstone download; callers pass an already-
 *     fetched corpus directory.
 *
 * Refs: greatstone `d_mapfile.html` (mapfile 2.x spec),
 * `tools/fetch_greatstone_sck_mapfiles.sh` corpus layout,
 * `_mapping.xml` index shape bundled with `sck.jar@1.5.1`,
 * the existing `firestaff_sck_mapfile` parser.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Per-mapfile verification result.  The verifier never
 * fails a corpus because a single row could not be parsed;
 * instead the row is counted as a parse failure and the
 * mapfile result tells the caller how to interpret the
 * stat counts. */
typedef enum FirestaffSckCorpusVerifierResult {
    FIRESTAFF_SCK_CORPUS_OK = 0,
    FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG,
    FIRESTAFF_SCK_CORPUS_ERR_BAD_HEADER,
    FIRESTAFF_SCK_CORPUS_ERR_OPEN_FILE,
    FIRESTAFF_SCK_CORPUS_ERR_TOO_MANY_MAPS,
    FIRESTAFF_SCK_CORPUS_ERR_NOT_A_DIRECTORY,
    FIRESTAFF_SCK_CORPUS_ERR_DIR_OPEN_FAILED
} FirestaffSckCorpusVerifierResult;

#define FIRESTAFF_SCK_CORPUS_VERIFIER_MAX_MAPS 256u
#define FIRESTAFF_SCK_CORPUS_VERIFIER_MAX_PATH 512u
#define FIRESTAFF_SCK_CORPUS_VERIFIER_FORMAT_BYTES 32u
#define FIRESTAFF_SCK_CORPUS_VERIFIER_ENDIAN_BYTES 16u
#define FIRESTAFF_SCK_CORPUS_VERIFIER_FIRST_ITEM_BYTES 16u
#define FIRESTAFF_SCK_CORPUS_VERIFIER_FIRST_TYPE_BYTES 16u
#define FIRESTAFF_SCK_CORPUS_VERIFIER_FIRST_DESC_BYTES 64u
#define FIRESTAFF_SCK_CORPUS_VERIFIER_PARSE_ERROR_BYTES 64u

/* What we know about each mapfile after parsing. */
typedef struct FirestaffSckCorpusVerifierMapStats {
    char path[FIRESTAFF_SCK_CORPUS_VERIFIER_MAX_PATH];
    char format[FIRESTAFF_SCK_CORPUS_VERIFIER_FORMAT_BYTES];
    char endian[FIRESTAFF_SCK_CORPUS_VERIFIER_ENDIAN_BYTES];
    /* SCK 2.x comma-row classification:
     *   parseableRows - rows that survived the comma split
     *   parseFailRows - rows that did not (too few fields,
     *                    field overflow, etc.)
     *   sizedRows     - rows with both numericNumber + SIZE=
     *   unsizedRows   - rows with numericNumber but no SIZE=
     *   oversizedRows - rows whose numericNumber+sizeBytes
     *                    exceed targetFileBytes (only counted
     *                    when targetFileBytes > 0)
     *   preserveRawAttributeRows - rows whose attributes
     *                    field was non-empty (raw preserved)
     *   truncatedByRowCap - 1 when the SCK 2.x parser
     *                    rejected the mapfile because it
     *                    exceeded FIRESTAFF_SCK_MAPFILE_MAX_ITEMS
     *                    (1024 rows) -- a corpus observability
     *                    signal, not a parse defect. */
    unsigned int parseableRows;
    unsigned int parseFailRows;
    unsigned int sizedRows;
    unsigned int unsizedRows;
    unsigned int oversizedRows;
    unsigned int preserveRawAttributeRows;
    unsigned int truncatedByRowCap;
    /* First parseable row summary, used for PASS evidence. */
    char firstItemNumber[FIRESTAFF_SCK_CORPUS_VERIFIER_FIRST_ITEM_BYTES];
    char firstItemType[FIRESTAFF_SCK_CORPUS_VERIFIER_FIRST_TYPE_BYTES];
    char firstItemDescription[FIRESTAFF_SCK_CORPUS_VERIFIER_FIRST_DESC_BYTES];
    /* Last parse error message captured for this mapfile, or
     * an empty string when none occurred.  Kept bounded so the
     * corpus scan can stream many mapfiles without growing. */
    char parseError[FIRESTAFF_SCK_CORPUS_VERIFIER_PARSE_ERROR_BYTES];
} FirestaffSckCorpusVerifierMapStats;

/* Aggregate corpus stats. */
typedef struct FirestaffSckCorpusVerifierCorpusStats {
    unsigned int totalMapfiles;
    unsigned int parseableMapfiles;
    unsigned int emptyMapfiles;
    unsigned int parseFailMapfiles;
    unsigned int rowCapMapfiles;
    unsigned int oversizedMapfiles;
    unsigned int totalRows;
    unsigned int parseableRows;
    unsigned int parseFailRows;
    unsigned int sizedRows;
    unsigned int unsizedRows;
    unsigned int oversizedRows;
    unsigned int preserveRawAttributeRows;
} FirestaffSckCorpusVerifierCorpusStats;

/* Optional report-row callback used by
 * FirestaffSckCorpusVerifier_VerifyCorpusDir so callers can
 * stream PASS evidence for every mapfile without building a
 * giant in-memory list.  Pass NULL to disable per-mapfile
 * callbacks. */
typedef void (*FirestaffSckCorpusVerifierMapCallback)(
    const FirestaffSckCorpusVerifierMapStats* stats,
    void* user);

/* Verify a single mapfile text blob.
 *
 * `text`          - mapfile contents (NULL-terminated)
 * `targetFileBytes` - optional bound for slice validation:
 *                     0 disables oversized-row counting and
 *                     the size check is skipped entirely.
 * `outStats`      - populated per-mapfile stats (must be non-NULL)
 *
 * Returns OK on success; BAD_HEADER when the text does not look
 * like a mapfile at all.  Row-level parse failures do NOT cause a
 * non-OK return; they are counted in `outStats->parseFailRows`
 * and the first error is captured in `outStats->parseError`. */
FirestaffSckCorpusVerifierResult FirestaffSckCorpusVerifier_VerifyMapfileText(
    const char* text,
    uint32_t targetFileBytes,
    FirestaffSckCorpusVerifierMapStats* outStats);

/* Verify a single mapfile file on disk.
 *
 * `path`          - filesystem path to the .map file
 * `targetFileBytes` - optional slice bound (0 disables)
 * `outStats`      - populated per-mapfile stats (must be non-NULL)
 *
 * Returns OK on success; OPEN_FILE when the file cannot be read;
 * BAD_HEADER when the contents do not look like a mapfile. */
FirestaffSckCorpusVerifierResult FirestaffSckCorpusVerifier_VerifyMapfilePath(
    const char* path,
    uint32_t targetFileBytes,
    FirestaffSckCorpusVerifierMapStats* outStats);

/* Walk every `.map` file under `corpusDir` (non-recursive; the
 * bundled SCK corpus uses a flat layout) and verify each one.
 *
 * `corpusDir`     - corpus directory (e.g. $HOME/.cache/.../db/map)
 * `targetFileBytes` - optional slice bound (0 disables)
 * `perMapCallback` - optional per-mapfile stream hook
 * `user`          - opaque pointer forwarded to perMapCallback
 * `outStats`      - optional aggregate stats (NULL permitted)
 *
 * Returns OK when the directory was walked cleanly, even when
 * individual mapfiles parse-fail (those are tallied in the
 * aggregate stats).  Returns DIR_OPEN_FAILED when the directory
 * cannot be opened, NOT_A_DIRECTORY when the path is not a
 * directory, and TOO_MANY_MAPS when the corpus exceeds the
 * internal cap. */
FirestaffSckCorpusVerifierResult FirestaffSckCorpusVerifier_VerifyCorpusDir(
    const char* corpusDir,
    uint32_t targetFileBytes,
    FirestaffSckCorpusVerifierMapCallback perMapCallback,
    void* user,
    FirestaffSckCorpusVerifierCorpusStats* outStats);

const char* FirestaffSckCorpusVerifier_ResultString(
    FirestaffSckCorpusVerifierResult result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SCK_CORPUS_VERIFIER_H */
