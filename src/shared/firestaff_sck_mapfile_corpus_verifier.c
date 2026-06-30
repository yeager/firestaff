/*
 * firestaff_sck_mapfile_corpus_verifier.c
 *
 * Bounded verifier over the Greatstone/SCK db/map corpus shipped
 * inside `sck.jar@1.5.1`.  Walks the corpus directory, parses
 * each `.map` with the bounded V1/V2 parsers, and reports
 * parseable rows, distinct type tags, distinct attribute
 * prefixes, per-FORMAT aggregate counts, and the unsized /
 * oversized slice distribution.
 *
 * Strategy:
 *   - Heuristic format detection per file: try SCK 2.x first
 *     (the corpus is dominated by V2 comma-separated rows); fall
 *     back to legacy V1 whitespace-separated rows when V2 fails.
 *   - When the V2 parser truncates at FIRESTAFF_SCK_MAPFILE_MAX_ITEMS,
 *     the verifier records `v2Rows == MAX_ITEMS`, classifies the
 *     failure as TOO_LARGE_FOR_PARSER (the file is well-formed
 *     but exceeds the bounded parser buffer), and emits the
 *     parser error string verbatim so operators can see what
 *     happened.
 *   - Empty files (header only, no items) are reported as
 *     PARSE_FAILED rather than OK, because header-only
 *     descriptors (`animation_sega_enda.map`,
 *     `savegame_default.map`, etc.) still surface as parse
 *     failures for the bounded parser contract.
 *   - Per-FORMAT aggregate counts let downstream probes detect
 *     regressions per Greatstone container shape (DMCSB1 /
 *     DMCSB2 / DMII / FTL / EXE / ROM / ANIMATION / CMP /
 *     SAVEGAME / DUNGEON / SINGLEITEM) without re-walking the
 *     per-file stats array.
 *
 * Non-scope: payload decoding, runtime asset-loader wiring.
 */

#include "firestaff_sck_mapfile_corpus_verifier.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#endif

static void copy_field(char* dst, size_t dstBytes, const char* src, size_t srcLen) {
    if (dstBytes == 0u) {
        return;
    }
    if (srcLen >= dstBytes) {
        srcLen = dstBytes - 1u;
    }
    if (srcLen != 0u) {
        memcpy(dst, src, srcLen);
    }
    dst[srcLen] = '\0';
}

static void set_err(char* dst, size_t cap, const char* msg) {
    size_t n;
    if (!dst || cap == 0u) {
        return;
    }
    n = strlen(msg);
    if (n >= cap) {
        n = cap - 1u;
    }
    memcpy(dst, msg, n);
    dst[n] = '\0';
}

static int is_regular_file(const char* path) {
    struct stat st;
    if (!path || stat(path, &st) != 0) {
        return 0;
    }
    return S_ISREG(st.st_mode) ? 1 : 0;
}

static int has_map_extension(const char* name) {
    /* Filter: only .map basename.  This excludes `_mapping.xml`
     * and `SOURCE.txt` so we walk only mapfile descriptors. */
    const char* dot = strrchr(name, '.');
    if (!dot) {
        return 0;
    }
    return strcmp(dot, ".map") == 0 ? 1 : 0;
}

static int corpus_list_files(const char* dir,
                             char names[][FIRESTAFF_SCK_CORPUS_NAME_BYTES],
                             unsigned int maxNames,
                             unsigned int* outCount) {
    unsigned int count = 0u;
    *outCount = 0u;
#if defined(_WIN32)
    {
        char pattern[FIRESTAFF_SCK_CORPUS_PATH_BYTES];
        HANDLE h;
        WIN32_FIND_DATAA data;
        if (!dir || !*dir) {
            return 0;
        }
        snprintf(pattern, sizeof(pattern), "%s\\*.map", dir);
        h = FindFirstFileA(pattern, &data);
        if (h == INVALID_HANDLE_VALUE) {
            return 0;
        }
        for (;;) {
            size_t n = strlen(data.cFileName);
            if (n > 0u && data.cFileName[0] != '.' && has_map_extension(data.cFileName)) {
                if (count >= maxNames) {
                    FindClose(h);
                    return 0;
                }
                copy_field(names[count], FIRESTAFF_SCK_CORPUS_NAME_BYTES,
                           data.cFileName, n);
                ++count;
            }
            if (!FindNextFileA(h, &data)) {
                break;
            }
        }
        FindClose(h);
        *outCount = count;
        return 1;
    }
#else
    {
        DIR* d;
        struct dirent* ent;
        if (!dir || !*dir) {
            return 0;
        }
        d = opendir(dir);
        if (!d) {
            return 0;
        }
        while ((ent = readdir(d)) != NULL) {
            size_t n;
            if (ent->d_name[0] == '.') {
                continue;
            }
            if (!has_map_extension(ent->d_name)) {
                continue;
            }
            if (count >= maxNames) {
                closedir(d);
                return 0;
            }
            n = strlen(ent->d_name);
            copy_field(names[count], FIRESTAFF_SCK_CORPUS_NAME_BYTES,
                       ent->d_name, n);
            ++count;
        }
        closedir(d);
        *outCount = count;
        return 1;
    }
#endif
}

/* Heuristic V2 vs V1 detection: V2 has a comma-separated header
 * row of KEY=*** pairs (and the first non-blank, non-comment
 * line starts with a non-digit letter).  V1 starts directly
 * with `type name offset size` whitespace-separated rows where
 * the type is an all-uppercase token. */
static int looks_like_v2(const char* text) {
    const char* p = text;
    /* Skip leading whitespace. */
    while (*p == ' ' || *p == '\t' || *p == '\r') {
        ++p;
    }
    /* Skip a single leading comment line. */
    if (*p == '#') {
        while (*p && *p != '\n') {
            ++p;
        }
        if (*p == '\n') {
            ++p;
        }
        while (*p == ' ' || *p == '\t' || *p == '\r') {
            ++p;
        }
    }
    if (!*p || isdigit((unsigned char)*p)) {
        /* Starts with a digit -> likely V1 (`0001,RAW1,...` would
         * still match this rule; we let the parser decide).  But
         * a V2 header always starts with a letter (KEY=...). */
        return 0;
    }
    {
        const char* eq = strchr(p, '=');
        const char* comma = strchr(p, ',');
        const char* nl = strchr(p, '\n');
        if (!eq) {
            return 0;
        }
        /* V2 header has at least one ',' before the newline
         * (KEY=***,KEY=***) OR a single property that ends at
         * the newline.  V1 rows have `=` only inside numeric
         * tokens like `0x100`.  The `eq < nl` check rules out
         * V1 lines that happen to contain a hex token. */
        if (eq > nl) {
            return 0;
        }
        if (comma && comma < nl && comma < eq) {
            return 0; /* comma before `=` looks V1-shaped */
        }
        return 1;
    }
}

static int contains_distinct(char distinct[][FIRESTAFF_SCK_CORPUS_TYPE_BYTES],
                             unsigned int max,
                             unsigned int* count,
                             const char* token,
                             size_t tokenLen) {
    unsigned int i;
    for (i = 0u; i < *count; ++i) {
        if (strlen(distinct[i]) == tokenLen &&
            strncmp(distinct[i], token, tokenLen) == 0) {
            return 0;
        }
    }
    if (*count >= max) {
        return 0;
    }
    copy_field(distinct[*count], FIRESTAFF_SCK_CORPUS_TYPE_BYTES, token, tokenLen);
    ++(*count);
    return 1;
}

/* Walk a SCK 2.x attributes blob (KEY=***) and collect distinct
 * KEY prefixes so the verifier can show "this mapfile uses
 * PAL=PAL1, ID=0001, SIZE=2560 attribute prefixes".
 *
 * The literal token `NULL` is a documented placeholder for
 * "no attributes"; we skip it.  Empty blobs are also skipped. */
static void collect_attr_prefixes(const char* attrs,
                                  char distinct[][FIRESTAFF_SCK_CORPUS_TYPE_BYTES],
                                  unsigned int max,
                                  unsigned int* count) {
    const char* p = attrs;
    if (!attrs || !*attrs) {
        return;
    }
    if (strcmp(attrs, "NULL") == 0) {
        return;
    }
    while (p && *p) {
        const char* eq;
        const char* amp;
        const char* sep;
        size_t keyLen;
        eq = strchr(p, '=');
        if (!eq) {
            break;
        }
        amp = strchr(p, '&');
        /* The KEY ends at the first '=' or '&', whichever comes
         * first.  Without this, "PAL=PAL1&SIZE=64" would record
         * "PAL=PAL1" as the key instead of "PAL". */
        sep = (amp && amp < eq) ? amp : eq;
        keyLen = (size_t)(sep - p);
        if (keyLen > 0u && keyLen < FIRESTAFF_SCK_CORPUS_TYPE_BYTES) {
            (void)contains_distinct(distinct, max, count, p, keyLen);
        }
        p = amp ? amp + 1 : NULL;
    }
}

/* Same shape as collect_attr_prefixes but the separator is the
 * SCK header `,` and the terminator is the end-of-line.  Used to
 * pull distinct KEY names out of the comma-separated header blob
 * ("ENDIAN=BIG,FORMAT=ANIMATION,SNDS.SPR=22050,SND4.SPR=7812")
 * that every V2 file carries as its first non-blank line. */
static void collect_header_attr_prefixes(const char* header,
                                         char distinct[][FIRESTAFF_SCK_CORPUS_TYPE_BYTES],
                                         unsigned int max,
                                         unsigned int* count) {
    const char* p = header;
    if (!header || !*header) {
        return;
    }
    while (p && *p) {
        const char* eq;
        const char* comma;
        const char* sep;
        size_t keyLen;
        /* Trim leading whitespace between header keys so
         * ",FORMAT=ANIMATION" still resolves to "FORMAT". */
        while (*p == ' ' || *p == '\t' || *p == '\r') {
            ++p;
        }
        if (!*p || *p == '\n' || *p == '\r') {
            break;
        }
        eq = strchr(p, '=');
        comma = strchr(p, ',');
        if (!eq) {
            break;
        }
        /* Key ends at the first '=' or ',', whichever comes
         * first.  The `,` separator keeps us on the same KEY
         * for value blobs that happen to contain `=` (none of
         * the documented SCK 2.x header values do). */
        sep = (comma && comma < eq) ? comma : eq;
        keyLen = (size_t)(sep - p);
        if (keyLen > 0u && keyLen < FIRESTAFF_SCK_CORPUS_TYPE_BYTES) {
            (void)contains_distinct(distinct, max, count, p, keyLen);
        }
        p = comma ? comma + 1 : NULL;
    }
}

static void reset_file_stats(FirestaffSckCorpusFileStats* stats) {
    memset(stats, 0, sizeof(*stats));
}

FirestaffSckCorpusFileResult FirestaffSckCorpus_VerifyText(
    const char* name,
    const char* text,
    uint32_t targetFileBytes,
    FirestaffSckCorpusFileStats* stats) {
    int isV2 = 0;
    int parseRc;
    char err[160];
    FirestaffSckMapfileV2 v2map;
    FirestaffSckMapfile v1map;
    unsigned int i;
    int truncated = 0;
    unsigned int distinctTypeCount = 0u;

    if (!stats) {
        return FIRESTAFF_SCK_CORPUS_FILE_PARSE_FAILED;
    }
    reset_file_stats(stats);
    if (name) {
        copy_field(stats->name, sizeof(stats->name), name, strlen(name));
    }
    if (!text || !*text) {
        set_err(stats->parseError, sizeof(stats->parseError), "empty corpus file");
        return FIRESTAFF_SCK_CORPUS_FILE_EMPTY;
    }

    isV2 = looks_like_v2(text);
    if (isV2) {
        memset(&v2map, 0, sizeof(v2map));
        memset(err, 0, sizeof(err));
        parseRc = FirestaffSckMapfile_ParseSck2Text(text, &v2map, err, sizeof(err));
        copy_field(stats->format, sizeof(stats->format),
                   v2map.format, strlen(v2map.format));
        copy_field(stats->endian, sizeof(stats->endian),
                   v2map.endian, strlen(v2map.endian));
        /* Capture header-level property keys regardless of
         * whether the parser accepted the file.  The underlying
         * SCK 2.x parser populates headerProperties before
         * item-level validation, so this still works for the
         * header-only descriptors (animation / dungeon / cmp /
         * savegame / singleitem entries) that the bounded
         * item-required parser rejects. */
        if (v2map.headerProperties[0] != '\0') {
            collect_header_attr_prefixes(v2map.headerProperties,
                                          stats->distinctHeaderAttrs,
                                          FIRESTAFF_SCK_CORPUS_DISTINCT_HEADER_ATTRS,
                                          &stats->distinctHeaderAttrCount);
        }
        if (parseRc == 1) {
            stats->parseOk = 1u;
            stats->v2Rows = v2map.itemCount;
            for (i = 0u; i < v2map.itemCount; ++i) {
                const FirestaffSckMapfileV2Item* item = &v2map.items[i];
                size_t typeLen = strlen(item->type);
                if (item->hasSizeBytes) {
                    ++stats->sizedRows;
                    if (targetFileBytes != 0u &&
                        (item->numericNumber > targetFileBytes ||
                         item->sizeBytes > targetFileBytes - item->numericNumber)) {
                        ++stats->oversizedRows;
                    }
                    if (item->numericNumber + item->sizeBytes > stats->largestSliceEnd) {
                        stats->largestSliceEnd = item->numericNumber + item->sizeBytes;
                    }
                } else {
                    ++stats->unsizedRows;
                }
                if (typeLen > 0u) {
                    (void)contains_distinct(stats->distinctTypes,
                                             FIRESTAFF_SCK_CORPUS_DISTINCT_TYPES,
                                             &distinctTypeCount,
                                             item->type,
                                             typeLen);
                }
                if (item->attributes[0] != '\0') {
                    collect_attr_prefixes(item->attributes,
                                          stats->distinctAttrs,
                                          FIRESTAFF_SCK_CORPUS_DISTINCT_ATTRS,
                                          &stats->distinctAttrPrefixCount);
                }
            }
            stats->distinctTypeCount = distinctTypeCount;
            return FIRESTAFF_SCK_CORPUS_FILE_OK;
        }
        /* Detect parser-buffer truncation: error string starts
         * with "too many".  Surface a distinct file-level error
         * so the corpus walk can flag oversized mapfiles
         * separately from malformed ones.  We report the parsed
         * count as a lower bound (the parser filled the bounded
         * buffer before bailing) so corpus totals stay honest. */
        if (strncmp(err, "too many", 8u) == 0) {
            set_err(stats->parseError, sizeof(stats->parseError), err);
            stats->v2Rows = v2map.itemCount;
            stats->truncated = 1u;
            truncated = 1;
        } else {
            set_err(stats->parseError, sizeof(stats->parseError), err);
        }
    }

    /* Fall back to V1 parser.  When V2 truncated the parser
     * buffer, V1 still confirms the file is V2-shape (it rejects
     * comma-separated V2 lines as malformed, which is correct). */
    memset(&v1map, 0, sizeof(v1map));
    memset(err, 0, sizeof(err));
    parseRc = FirestaffSckMapfile_ParseText(text, &v1map, err, sizeof(err));
    if (parseRc == 1) {
        unsigned int distinctTypeCountV1 = 0u;
        /* V1 parser does not understand SIZE= attributes; all
         * V1 rows are reported as unsized. */
        stats->parseOk = 1u;
        stats->v1Rows = v1map.itemCount;
        stats->unsizedRows = v1map.itemCount;
        for (i = 0u; i < v1map.itemCount; ++i) {
            size_t typeLen = strlen(v1map.items[i].type);
            if (typeLen > 0u) {
                (void)contains_distinct(stats->distinctTypes,
                                         FIRESTAFF_SCK_CORPUS_DISTINCT_TYPES,
                                         &distinctTypeCountV1,
                                         v1map.items[i].type,
                                         typeLen);
            }
        }
        stats->distinctTypeCount = distinctTypeCountV1;
        return FIRESTAFF_SCK_CORPUS_FILE_OK;
    }
    if (stats->parseError[0] == '\0') {
        set_err(stats->parseError, sizeof(stats->parseError), err);
    }
    stats->parseFailed = 1u;
    /* File failed both V2 and V1 parse. */
    return truncated ? FIRESTAFF_SCK_CORPUS_FILE_TOO_LARGE
                     : FIRESTAFF_SCK_CORPUS_FILE_PARSE_FAILED;
}

static void merge_distinct(char dst[][FIRESTAFF_SCK_CORPUS_TYPE_BYTES],
                           unsigned int max,
                           unsigned int* count,
                           const char src[][FIRESTAFF_SCK_CORPUS_TYPE_BYTES],
                           unsigned int srcCount) {
    unsigned int i;
    for (i = 0u; i < srcCount && *count < max; ++i) {
        if (src[i][0] == '\0') {
            continue;
        }
        (void)contains_distinct(dst, max, count, src[i], strlen(src[i]));
    }
}

static FirestaffSckCorpusFormatStats* find_or_create_format(
    FirestaffSckCorpusSummary* summary,
    const char* format,
    int* outCreated) {
    unsigned int i;
    *outCreated = 0;
    if (!format || !*format) {
        return NULL;
    }
    for (i = 0u; i < summary->formatCount; ++i) {
        if (strcmp(summary->formats[i].format, format) == 0) {
            return &summary->formats[i];
        }
    }
    if (summary->formatCount >= FIRESTAFF_SCK_CORPUS_PER_FORMAT) {
        return NULL;
    }
    {
        FirestaffSckCorpusFormatStats* fmt = &summary->formats[summary->formatCount];
        memset(fmt, 0, sizeof(*fmt));
        copy_field(fmt->format, sizeof(fmt->format), format, strlen(format));
        ++summary->formatCount;
        *outCreated = 1;
        return fmt;
    }
}

static void accumulate_file_stats(FirestaffSckCorpusSummary* summary,
                                  FirestaffSckCorpusFileStats* stats) {
    FirestaffSckCorpusFormatStats* fmt;
    int created = 0;
    summary->totalRows += stats->v1Rows + stats->v2Rows;
    summary->v1Rows += stats->v1Rows;
    summary->v2Rows += stats->v2Rows;
    summary->sizedRows += stats->sizedRows;
    summary->unsizedRows += stats->unsizedRows;
    summary->oversizedRows += stats->oversizedRows;
    if (stats->parseOk) {
        ++summary->parseableMapfiles;
    } else {
        ++summary->unparseableMapfiles;
    }
    if (stats->truncated) {
        ++summary->truncatedMapfiles;
    }
    fmt = find_or_create_format(summary, stats->format, &created);
    if (fmt) {
        ++fmt->fileCount;
        if (stats->parseOk) {
            ++fmt->parseableFileCount;
        }
        if (stats->truncated) {
            ++fmt->truncatedFileCount;
        }
        fmt->itemCount += stats->v1Rows + stats->v2Rows;
        fmt->sizedRows += stats->sizedRows;
        fmt->unsizedRows += stats->unsizedRows;
        fmt->oversizedRows += stats->oversizedRows;
    }
}

FirestaffSckCorpusResult FirestaffSckCorpus_VerifyDirectory(
    const char* corpusDir,
    uint32_t targetFileBytes,
    FirestaffSckCorpusSummary* summary) {
    char names[FIRESTAFF_SCK_CORPUS_MAX_FILES][FIRESTAFF_SCK_CORPUS_NAME_BYTES];
    unsigned int fileCount = 0u;
    unsigned int i;

    if (!summary) {
        return FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG;
    }
    memset(summary, 0, sizeof(*summary));

    if (!corpusDir || !*corpusDir) {
        return FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG;
    }

    if (!corpus_list_files(corpusDir, names, FIRESTAFF_SCK_CORPUS_MAX_FILES, &fileCount)) {
        /* opendir / FindFirstFileA failed.  This is a hard
         * error -- the directory was named but cannot be read. */
        return FIRESTAFF_SCK_CORPUS_ERR_DIR_OPEN;
    }
    if (fileCount == 0u) {
        /* Directory exists but has no .map entries -- treat as
         * skip-safe so CTest stays green in offline environments. */
        return FIRESTAFF_SCK_CORPUS_ERR_NO_CORPUS;
    }

    for (i = 0u; i < fileCount; ++i) {
        char path[FIRESTAFF_SCK_CORPUS_PATH_BYTES];
        FILE* f;
        long len;
        char* text = NULL;
        size_t got;
        FirestaffSckCorpusFileStats stats;
        FirestaffSckCorpusFileResult r;

        snprintf(path, sizeof(path), "%s/%s", corpusDir, names[i]);
        if (!is_regular_file(path)) {
            continue;
        }
        f = fopen(path, "rb");
        if (!f) {
            continue;
        }
        if (fseek(f, 0, SEEK_END) != 0) {
            fclose(f);
            continue;
        }
        len = ftell(f);
        if (len < 0) {
            fclose(f);
            continue;
        }
        rewind(f);
        text = (char*)malloc((size_t)len + 1u);
        if (!text) {
            fclose(f);
            continue;
        }
        got = fread(text, 1u, (size_t)len, f);
        fclose(f);
        if (got != (size_t)len) {
            free(text);
            continue;
        }
        text[len] = '\0';

        r = FirestaffSckCorpus_VerifyText(names[i], text, targetFileBytes, &stats);
        free(text);
        (void)r;

        if (summary->fileCount >= FIRESTAFF_SCK_CORPUS_MAX_FILES) {
            return FIRESTAFF_SCK_CORPUS_ERR_TOO_MANY_FILES;
        }
        summary->files[summary->fileCount++] = stats;
        ++summary->totalMapfiles;
        accumulate_file_stats(summary, &stats);
        merge_distinct(summary->distinctTypes,
                       FIRESTAFF_SCK_CORPUS_DISTINCT_TYPES,
                       &summary->distinctTypeCount,
                       stats.distinctTypes,
                       stats.distinctTypeCount);
        merge_distinct(summary->distinctAttrPrefixes,
                       FIRESTAFF_SCK_CORPUS_DISTINCT_ATTRS,
                       &summary->distinctAttrPrefixCount,
                       stats.distinctAttrs,
                       stats.distinctAttrPrefixCount);
        merge_distinct(summary->distinctHeaderAttrPrefixes,
                       FIRESTAFF_SCK_CORPUS_DISTINCT_HEADER_ATTRS,
                       &summary->distinctHeaderAttrPrefixCount,
                       stats.distinctHeaderAttrs,
                       stats.distinctHeaderAttrCount);
    }

    return FIRESTAFF_SCK_CORPUS_OK;
}

int FirestaffSckCorpus_ResolveDefaultDir(char* out, size_t outBytes) {
    const char* env;
    if (!out || outBytes == 0u) {
        return 0;
    }
    out[0] = '\0';
    env = getenv("FIRESTAFF_GREATSTONE_SCK_DIR");
    if (env && env[0] != '\0') {
        char candidate[FIRESTAFF_SCK_CORPUS_PATH_BYTES];
        struct stat st;
        snprintf(candidate, sizeof(candidate), "%s/db/map", env);
        if (stat(candidate, &st) == 0 && S_ISDIR(st.st_mode)) {
            snprintf(out, outBytes, "%s", candidate);
            return 1;
        }
        if (stat(env, &st) == 0 && S_ISDIR(st.st_mode)) {
            snprintf(out, outBytes, "%s", env);
            return 1;
        }
        /* Env was set but did not resolve; honour operator intent. */
        return 0;
    }
    {
        const char* home = getenv("HOME");
        if (home && home[0] != '\0') {
            char candidate[FIRESTAFF_SCK_CORPUS_PATH_BYTES];
            struct stat st;
            snprintf(candidate, sizeof(candidate),
                     "%s/.cache/firestaff/greatstone-sck-mapfiles/db/map", home);
            if (stat(candidate, &st) == 0 && S_ISDIR(st.st_mode)) {
                snprintf(out, outBytes, "%s", candidate);
                return 1;
            }
        }
    }
    return 0;
}

const char* FirestaffSckCorpus_ResultString(FirestaffSckCorpusResult result) {
    switch (result) {
        case FIRESTAFF_SCK_CORPUS_OK: return "OK";
        case FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG: return "NULL_ARG";
        case FIRESTAFF_SCK_CORPUS_ERR_NO_CORPUS: return "NO_CORPUS";
        case FIRESTAFF_SCK_CORPUS_ERR_DIR_OPEN: return "DIR_OPEN";
        case FIRESTAFF_SCK_CORPUS_ERR_TOO_MANY_FILES: return "TOO_MANY_FILES";
        default: return "UNKNOWN";
    }
}

const char* FirestaffSckCorpus_FileResultString(FirestaffSckCorpusFileResult result) {
    switch (result) {
        case FIRESTAFF_SCK_CORPUS_FILE_OK: return "OK";
        case FIRESTAFF_SCK_CORPUS_FILE_EMPTY: return "EMPTY";
        case FIRESTAFF_SCK_CORPUS_FILE_NOT_FOUND: return "NOT_FOUND";
        case FIRESTAFF_SCK_CORPUS_FILE_TOO_LARGE: return "TOO_LARGE";
        case FIRESTAFF_SCK_CORPUS_FILE_PARSE_FAILED: return "PARSE_FAILED";
        default: return "UNKNOWN";
    }
}
