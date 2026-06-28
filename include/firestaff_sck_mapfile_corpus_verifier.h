#ifndef FIRESTAFF_SCK_MAPFILE_CORPUS_VERIFIER_H
#define FIRESTAFF_SCK_MAPFILE_CORPUS_VERIFIER_H

/*
 * firestaff_sck_mapfile_corpus_verifier.h
 *
 * Bounded verifier for the Greatstone/SCK db/map corpus shipped
 * inside `sck.jar@1.5.1`.  Walks a corpus directory, parses each
 * `.map` file with the bounded V1/V2 parsers, preserves raw
 * attribute prefixes, and reports per-file and per-FORMAT
 * parseable/unsized/oversized slice counts against a caller-
 * supplied synthetic targetFileBytes.
 *
 * Scope:
 *   - Walk a corpus directory and pair each `*.map` entry with
 *     the parser that fits its shape (V2 SCK 2.x comma rows vs
 *     V1 legacy `type name offset size` whitespace rows).
 *   - For V2: report per-file item count, sized vs unsized split,
 *     distinct type tags, distinct attribute prefixes, oversized
 *     slice count vs a configurable synthetic target file size,
 *     and overall parse success.
 *   - For V1: same shape where applicable; V1 rows do not carry
 *     SIZE= attributes and are reported as unsized.
 *   - Aggregate per-FORMAT counts (FORMAT=... header value) so
 *     callers can flag regressions per Greatstone container shape.
 *   - Parser-buffer truncation ("too many SCK mapfile items" at
 *     the bounded FIRESTAFF_SCK_MAPFILE_MAX_ITEMS=1024 cap) is
 *     classified as TOO_LARGE rather than PARSE_FAILED and the
 *     parsed count is recorded as a lower bound so corpus totals
 *     stay honest.
 *
 * Non-scope (kept out by design to bound this commit):
 *   - No actual payload decode (IMG1/IMG3/IMG5/RAW/FTL/PAL/SND
 *     items stay parser-only).
 *   - No runtime asset-loader wiring.
 *   - No MAP-3 parser bump to lift the 1024-row cap; the
 *     truncation surface is the documented signal.
 *   - No network/Greatstone download; the corpus must already be
 *     on disk under `corpusDir`.
 *
 * Refs: greatstone `d_mapfile.html` (mapfile 2.x spec),
 * `_mapping.xml` index bundled with `sck.jar@1.5.1`, the DMWeb
 * encyclopaedia cross-link for mapfile layout, and the existing
 * `firestaff_sck_mapfile` V1+V2 parsers.
 */

#include <stddef.h>
#include <stdint.h>

#include "firestaff_sck_mapfile.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FIRESTAFF_SCK_CORPUS_MAX_FILES 256u
#define FIRESTAFF_SCK_CORPUS_PATH_BYTES 1024u
#define FIRESTAFF_SCK_CORPUS_TYPE_BYTES 16u
#define FIRESTAFF_SCK_CORPUS_NAME_BYTES 64u
#define FIRESTAFF_SCK_CORPUS_FORMAT_BYTES 32u
#define FIRESTAFF_SCK_CORPUS_DISTINCT_TYPES 32u
#define FIRESTAFF_SCK_CORPUS_DISTINCT_ATTRS 32u
#define FIRESTAFF_SCK_CORPUS_PER_FORMAT 16u

typedef struct FirestaffSckCorpusFileStats {
    char name[FIRESTAFF_SCK_CORPUS_NAME_BYTES];
    char format[FIRESTAFF_SCK_CORPUS_FORMAT_BYTES];
    char endian[16];
    unsigned int v1Rows;
    unsigned int v2Rows;
    unsigned int sizedRows;
    unsigned int unsizedRows;
    unsigned int oversizedRows;
    unsigned int parseOk;
    unsigned int parseFailed;
    unsigned int truncated;
    char parseError[128];
    uint32_t largestSliceEnd;
    unsigned int distinctTypeCount;
    char distinctTypes[FIRESTAFF_SCK_CORPUS_DISTINCT_TYPES]
                      [FIRESTAFF_SCK_CORPUS_TYPE_BYTES];
    unsigned int distinctAttrPrefixCount;
    char distinctAttrs[FIRESTAFF_SCK_CORPUS_DISTINCT_ATTRS]
                      [FIRESTAFF_SCK_CORPUS_TYPE_BYTES];
} FirestaffSckCorpusFileStats;

typedef struct FirestaffSckCorpusFormatStats {
    char format[FIRESTAFF_SCK_CORPUS_FORMAT_BYTES];
    unsigned int fileCount;
    unsigned int parseableFileCount;
    unsigned int truncatedFileCount;
    unsigned int itemCount;
    unsigned int sizedRows;
    unsigned int unsizedRows;
    unsigned int oversizedRows;
} FirestaffSckCorpusFormatStats;

typedef struct FirestaffSckCorpusSummary {
    unsigned int totalMapfiles;
    unsigned int parseableMapfiles;
    unsigned int unparseableMapfiles;
    unsigned int truncatedMapfiles;
    unsigned int totalRows;
    unsigned int v1Rows;
    unsigned int v2Rows;
    unsigned int sizedRows;
    unsigned int unsizedRows;
    unsigned int oversizedRows;
    unsigned int distinctTypeCount;
    char distinctTypes[FIRESTAFF_SCK_CORPUS_DISTINCT_TYPES]
                      [FIRESTAFF_SCK_CORPUS_TYPE_BYTES];
    unsigned int distinctAttrPrefixCount;
    char distinctAttrPrefixes[FIRESTAFF_SCK_CORPUS_DISTINCT_ATTRS]
                             [FIRESTAFF_SCK_CORPUS_TYPE_BYTES];
    unsigned int formatCount;
    FirestaffSckCorpusFormatStats formats[FIRESTAFF_SCK_CORPUS_PER_FORMAT];
    unsigned int fileCount;
    FirestaffSckCorpusFileStats files[FIRESTAFF_SCK_CORPUS_MAX_FILES];
} FirestaffSckCorpusSummary;

/* Outcome of a single file parse inside the verifier.  Unknown
 * shapes are reported as parse failures (no items), never as
 * silent drops, so callers can flag the corpus for upgrade when
 * a new mapfile variant appears.  TOO_LARGE means the parser
 * filled its bounded buffer before reaching the end of the
 * mapfile; the parsed-count field is a documented lower bound. */
typedef enum FirestaffSckCorpusFileResult {
    FIRESTAFF_SCK_CORPUS_FILE_OK = 0,
    FIRESTAFF_SCK_CORPUS_FILE_EMPTY,
    FIRESTAFF_SCK_CORPUS_FILE_NOT_FOUND,
    FIRESTAFF_SCK_CORPUS_FILE_TOO_LARGE,
    FIRESTAFF_SCK_CORPUS_FILE_PARSE_FAILED
} FirestaffSckCorpusFileResult;

/* Result codes for the public API.  Non-zero values indicate a
 * caller-visible failure; 0 means OK (SKIP / no corpus still
 * returns OK with `summary->totalMapfiles == 0`). */
typedef enum FirestaffSckCorpusResult {
    FIRESTAFF_SCK_CORPUS_OK = 0,
    FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG,
    FIRESTAFF_SCK_CORPUS_ERR_NO_CORPUS,
    FIRESTAFF_SCK_CORPUS_ERR_DIR_OPEN,
    FIRESTAFF_SCK_CORPUS_ERR_TOO_MANY_FILES
} FirestaffSckCorpusResult;

/* Verify a single mapfile in-memory.  `text` is the raw text
 * (read until the first NUL).  `name` is used purely for the
 * per-file reporting slot.  Always populates `stats`; returns
 * the file-level outcome. */
FirestaffSckCorpusFileResult FirestaffSckCorpus_VerifyText(
    const char* name,
    const char* text,
    uint32_t targetFileBytes,
    FirestaffSckCorpusFileStats* stats);

/* Build a summary over every `*.map` file found under `corpusDir`.
 *
 * `targetFileBytes` is the synthetic target file size used to
 * flag oversized slices: any V2 item whose SIZE-backed numeric
 * number + size exceeds `targetFileBytes` is counted as
 * `oversizedRows`.  Pass 0 to skip the oversized check.
 *
 * The summary is fully populated when the call returns OK and
 * `summary->totalMapfiles > 0`.  When the directory is missing
 * or empty, `summary->totalMapfiles == 0` and the result is
 * `FIRESTAFF_SCK_CORPUS_OK` so callers can treat it as `SKIPPED`. */
FirestaffSckCorpusResult FirestaffSckCorpus_VerifyDirectory(
    const char* corpusDir,
    uint32_t targetFileBytes,
    FirestaffSckCorpusSummary* summary);

/* Resolve the default Greatstone/SCK db/map corpus directory.
 * Honors, in priority order:
 *   1. $FIRESTAFF_GREATSTONE_SCK_DIR/db/map (when env points
 *      at the Greatstone sck root and the db/map child exists)
 *   2. $FIRESTAFF_GREATSTONE_SCK_DIR (when env points at the
 *      db/map directory already)
 *   3. $HOME/.cache/firestaff/greatstone-sck-mapfiles/db/map
 * Writes into `out` (caller-owned) and returns 1 on success.
 * Returns 0 when no directory resolved. */
int FirestaffSckCorpus_ResolveDefaultDir(char* out, size_t outBytes);

const char* FirestaffSckCorpus_ResultString(FirestaffSckCorpusResult result);

const char* FirestaffSckCorpus_FileResultString(FirestaffSckCorpusFileResult result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SCK_MAPFILE_CORPUS_VERIFIER_H */
