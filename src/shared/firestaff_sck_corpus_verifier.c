#include "firestaff_sck_corpus_verifier.h"

#include "firestaff_sck_mapfile.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* POSIX directory walk.  Including <dirent.h> directly keeps the
 * verifier scoped to macOS / Linux hosts (the existing probes in
 * this tree all follow the same pattern). */
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

/* The bundled SCK corpus has both flavors:
 *   - legacy 4-token rows (`type name offset size`) optionally
 *     preceded by a `ENDIAN=...,FORMAT=...` header
 *   - SCK 2.x comma rows (`number,type,attributes,description,
 *     long,comment`) preceded by a header
 *
 * Routing rule (see FirestaffSckCorpusVerifier_VerifyMapfileText):
 * try SCK 2.x first (the stricter parser) and fall back to legacy
 * on failure.  This handles every corpus shape we have seen in
 * the wild without sniffing the first line. */

static void copy_bounded(char* dst, size_t dstBytes, const char* src) {
    size_t n;
    if (!dst || dstBytes == 0u) {
        return;
    }
    if (!src) {
        dst[0] = '\0';
        return;
    }
    n = strlen(src);
    if (n >= dstBytes) {
        n = dstBytes - 1u;
    }
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static void copy_bounded_len(char* dst, size_t dstBytes, const char* src, size_t len) {
    size_t n;
    if (!dst || dstBytes == 0u) {
        return;
    }
    if (!src) {
        dst[0] = '\0';
        return;
    }
    while (len > 0u && (*src == ' ' || *src == '\t' || *src == '\r')) {
        ++src;
        --len;
    }
    while (len > 0u && (src[len - 1u] == ' ' ||
                        src[len - 1u] == '\t' ||
                        src[len - 1u] == '\r')) {
        --len;
    }
    n = len;
    if (n >= dstBytes) {
        n = dstBytes - 1u;
    }
    if (n != 0u) {
        memcpy(dst, src, n);
    }
    dst[n] = '\0';
}

static int read_text_file(const char* path, char** outText, size_t* outLen) {
    FILE* f;
    long len;
    char* buf;
    size_t got;
    if (!path || !outText || !outLen) {
        return 0;
    }
    f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    len = ftell(f);
    if (len < 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    buf = (char*)malloc((size_t)len + 1u);
    if (!buf) {
        fclose(f);
        return 0;
    }
    got = fread(buf, 1, (size_t)len, f);
    fclose(f);
    if (got != (size_t)len) {
        free(buf);
        return 0;
    }
    buf[len] = '\0';
    *outText = buf;
    *outLen = (size_t)len;
    return 1;
}

static void init_map_stats(FirestaffSckCorpusVerifierMapStats* outStats,
                           const char* path) {
    memset(outStats, 0, sizeof(*outStats));
    if (path) {
        copy_bounded(outStats->path, sizeof(outStats->path), path);
    }
}

/* Forward declaration: extract_header_pair_legacy is defined
 * later in this file but verify_sck2() below uses it to populate
 * the per-mapfile FORMAT/ENDIAN stats BEFORE the SCK 2.x parser
 * runs (so row-cap and empty-mapfile outcomes still report the
 * header metadata). */
static int extract_header_pair_legacy(const char* text,
                                      const char* key,
                                      char* dst,
                                      size_t dstBytes);

static void extract_header_pair(const char* props,
                                const char* key,
                                char* dst,
                                size_t dstBytes) {
    size_t keyLen = strlen(key);
    const char* p = props;
    if (!props || !dst || dstBytes == 0u) {
        if (dst && dstBytes > 0u) {
            dst[0] = '\0';
        }
        return;
    }
    dst[0] = '\0';
    while (p && *p) {
        const char* next = strchr(p, ',');
        size_t len = next ? (size_t)(next - p) : strlen(p);
        if (len > keyLen && strncmp(p, key, keyLen) == 0 && p[keyLen] == '=') {
            copy_bounded_len(dst, dstBytes, p + keyLen + 1u, len - keyLen - 1u);
            return;
        }
        p = next ? next + 1 : NULL;
    }
}

static FirestaffSckCorpusVerifierResult verify_sck2(
    const char* text,
    uint32_t targetFileBytes,
    FirestaffSckCorpusVerifierMapStats* outStats) {
    FirestaffSckMapfileV2 map;
    char err[128];
    unsigned int i;
    unsigned int totalRows;

    memset(&map, 0, sizeof(map));
    memset(err, 0, sizeof(err));
    /* Always try to extract FORMAT/ENDIAN from the raw text first,
     * so a row-cap or empty-mapfile failure still surfaces its
     * header metadata in the per-mapfile stats. */
    (void)extract_header_pair_legacy(text, "FORMAT", outStats->format, sizeof(outStats->format));
    (void)extract_header_pair_legacy(text, "ENDIAN", outStats->endian, sizeof(outStats->endian));
    if (FirestaffSckMapfile_ParseSck2Text(text, &map, err, sizeof(err)) != 1) {
        copy_bounded(outStats->parseError, sizeof(outStats->parseError), err);
        /* The SCK 2.x parser is bounded at FIRESTAFF_SCK_MAPFILE_MAX_ITEMS
         * rows.  When a real-world mapfile exceeds that cap (DM2
         * GD/PC9801 etc. push past 1024 rows), the parser reports
         * 'too many SCK mapfile items'.  We surface that as a
         * truncated-by-row-cap observation rather than a parse
         * defect so the corpus report can distinguish "the file
         * is broken" from "the parser is intentionally bounded". */
        if (strstr(err, "too many SCK mapfile items") != NULL) {
            outStats->truncatedByRowCap = 1u;
        }
        return FIRESTAFF_SCK_CORPUS_ERR_BAD_HEADER;
    }

    extract_header_pair(map.headerProperties, "FORMAT", outStats->format, sizeof(outStats->format));
    extract_header_pair(map.headerProperties, "ENDIAN", outStats->endian, sizeof(outStats->endian));

    totalRows = map.itemCount;
    for (i = 0u; i < totalRows; ++i) {
        const FirestaffSckMapfileV2Item* item = &map.items[i];
        if (item->number[0] == '\0' && item->type[0] == '\0' &&
            item->description[0] == '\0' && item->longDescription[0] == '\0' &&
            item->comment[0] == '\0') {
            /* Truly empty row (whitespace-only or full NULL row). */
            ++outStats->parseFailRows;
            continue;
        }
        ++outStats->parseableRows;
        if (item->attributes[0] != '\0') {
            ++outStats->preserveRawAttributeRows;
        }
        if (item->hasNumericNumber) {
            if (item->hasSizeBytes) {
                ++outStats->sizedRows;
                if (targetFileBytes > 0u) {
                    if (item->numericNumber > targetFileBytes ||
                        item->sizeBytes > targetFileBytes - item->numericNumber) {
                        ++outStats->oversizedRows;
                    }
                }
            } else {
                ++outStats->unsizedRows;
            }
        } else if (item->hasSizeBytes) {
            /* SIZE= without a numeric offset is still partially
             * usable; count it as unsized so the report shows
             * the missing-offset case explicitly. */
            ++outStats->unsizedRows;
        }
        if (outStats->firstItemNumber[0] == '\0' && item->number[0] != '\0') {
            copy_bounded(outStats->firstItemNumber, sizeof(outStats->firstItemNumber), item->number);
        }
        if (outStats->firstItemType[0] == '\0' && item->type[0] != '\0') {
            copy_bounded(outStats->firstItemType, sizeof(outStats->firstItemType), item->type);
        }
        if (outStats->firstItemDescription[0] == '\0' && item->description[0] != '\0') {
            copy_bounded(outStats->firstItemDescription,
                         sizeof(outStats->firstItemDescription),
                         item->description);
        }
    }
    outStats->parseError[0] = '\0';
    return FIRESTAFF_SCK_CORPUS_OK;
}

static int extract_header_pair_legacy(const char* text,
                                      const char* key,
                                      char* dst,
                                      size_t dstBytes) {
    /* Legacy header is OPTIONAL.  When present it is one
     * `KEY=VALUE,KEY=VALUE` line; when absent we just leave
     * dst empty and return 0. */
    const char* p = text;
    size_t keyLen;
    if (!text || !dst || dstBytes == 0u) {
        return 0;
    }
    dst[0] = '\0';
    keyLen = strlen(key);
    /* Only sniff the first non-comment, non-blank line. */
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
        ++p;
    }
    if (*p == '#' || *p == '\0') {
        return 0;
    }
    while (*p && *p != '\n') {
        const char* next;
        size_t len;
        while (*p == ' ' || *p == '\t' || *p == '\r') {
            ++p;
        }
        next = strchr(p, ',');
        len = next ? (size_t)(next - p) : strlen(p);
        if (len > keyLen && strncmp(p, key, keyLen) == 0 && p[keyLen] == '=') {
            copy_bounded_len(dst, dstBytes, p + keyLen + 1u, len - keyLen - 1u);
            return 1;
        }
        p = next ? next + 1 : p + len;
    }
    return 0;
}

static FirestaffSckCorpusVerifierResult verify_legacy(
    const char* text,
    uint32_t targetFileBytes,
    FirestaffSckCorpusVerifierMapStats* outStats) {
    FirestaffSckMapfile map;
    char err[128];
    unsigned int i;
    const char* legacyText = text;

    memset(&map, 0, sizeof(map));
    memset(err, 0, sizeof(err));
    /* Some legacy mapfiles in the corpus carry a single
     * `ENDIAN=...,FORMAT=...` header line.  When the very
     * first non-comment, non-blank line has the shape
     * `KEY=VALUE(,KEY=VALUE)*` with no whitespace tokens,
     * strip it before handing the body to the legacy
     * parser.  Otherwise we leave the text untouched. */
    {
        const char* p = text;
        const char* lineEnd;
        int looksLikeHeader = 1;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
            ++p;
        }
        if (*p == '#' || *p == '\0') {
            looksLikeHeader = 0;
        }
        lineEnd = p;
        while (*lineEnd && *lineEnd != '\n') {
            int c = (unsigned char)*lineEnd;
            if (c == ' ' || c == '\t') {
                looksLikeHeader = 0;
                break;
            }
            ++lineEnd;
        }
        if (looksLikeHeader) {
            legacyText = (*lineEnd == '\n') ? (lineEnd + 1) : lineEnd;
        }
    }
    if (FirestaffSckMapfile_ParseText(legacyText, &map, err, sizeof(err)) != 1) {
        copy_bounded(outStats->parseError, sizeof(outStats->parseError), err);
        return FIRESTAFF_SCK_CORPUS_ERR_BAD_HEADER;
    }
    (void)extract_header_pair_legacy(text, "FORMAT", outStats->format, sizeof(outStats->format));
    (void)extract_header_pair_legacy(text, "ENDIAN", outStats->endian, sizeof(outStats->endian));

    for (i = 0u; i < map.itemCount; ++i) {
        const FirestaffSckMapfileItem* item = &map.items[i];
        ++outStats->parseableRows;
        /* Legacy rows always have a sized type/name/offset/size
         * tuple, so SIZE= counting collapses to "all rows are
         * sized".  preserveRawAttributeRows is not applicable to
         * legacy rows because they have no attributes field. */
        ++outStats->sizedRows;
        if (targetFileBytes > 0u) {
            if (item->offset > targetFileBytes ||
                item->size > targetFileBytes - item->offset) {
                ++outStats->oversizedRows;
            }
        }
        if (outStats->firstItemType[0] == '\0' && item->type[0] != '\0') {
            copy_bounded(outStats->firstItemType, sizeof(outStats->firstItemType), item->type);
        }
        if (outStats->firstItemDescription[0] == '\0' && item->name[0] != '\0') {
            copy_bounded(outStats->firstItemDescription,
                         sizeof(outStats->firstItemDescription),
                         item->name);
        }
    }
    outStats->parseError[0] = '\0';
    return FIRESTAFF_SCK_CORPUS_OK;
}

FirestaffSckCorpusVerifierResult FirestaffSckCorpusVerifier_VerifyMapfileText(
    const char* text,
    uint32_t targetFileBytes,
    FirestaffSckCorpusVerifierMapStats* outStats) {
    if (!text || !outStats) {
        return FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG;
    }
    init_map_stats(outStats, NULL);
    /* Try SCK 2.x first; fall back to legacy on parser failure.
     * The SCK 2.x parser is the stricter of the two and correctly
     * rejects mapfiles that do not have a header + 4+-field CSV
     * rows.  The legacy parser accepts the simpler `type name
     * offset size` shape used by older mapfiles in the corpus.
     *
     * Important: when the SCK 2.x parser reports "too many items"
     * we know the file IS SCK 2.x (header + CSV rows) but exceeds
     * the bounded parser cap.  In that case we keep the SCK 2.x
     * partial evidence (truncatedByRowCap flag) rather than calling
     * the legacy parser, which would misinterpret the header line
     * and report a worse error. */
    {
        FirestaffSckCorpusVerifierMapStats probe;
        FirestaffSckCorpusVerifierResult r;
        memset(&probe, 0, sizeof(probe));
        r = verify_sck2(text, targetFileBytes, &probe);
        if (r == FIRESTAFF_SCK_CORPUS_OK) {
            *outStats = probe;
            return FIRESTAFF_SCK_CORPUS_OK;
        }
        if (probe.truncatedByRowCap) {
            *outStats = probe;
            return r;
        }
    }
    return verify_legacy(text, targetFileBytes, outStats);
}

FirestaffSckCorpusVerifierResult FirestaffSckCorpusVerifier_VerifyMapfilePath(
    const char* path,
    uint32_t targetFileBytes,
    FirestaffSckCorpusVerifierMapStats* outStats) {
    char* text = NULL;
    size_t textLen = 0u;
    FirestaffSckCorpusVerifierResult r;

    if (!path || !outStats) {
        return FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG;
    }
    init_map_stats(outStats, path);
    if (!read_text_file(path, &text, &textLen)) {
        copy_bounded(outStats->parseError, sizeof(outStats->parseError), "open failed");
        return FIRESTAFF_SCK_CORPUS_ERR_OPEN_FILE;
    }
    r = FirestaffSckCorpusVerifier_VerifyMapfileText(text, targetFileBytes, outStats);
    /* Preserve the path on the per-mapfile stats even when the
     * verifier succeeded. */
    copy_bounded(outStats->path, sizeof(outStats->path), path);
    free(text);
    return r;
}

FirestaffSckCorpusVerifierResult FirestaffSckCorpusVerifier_VerifyCorpusDir(
    const char* corpusDir,
    uint32_t targetFileBytes,
    FirestaffSckCorpusVerifierMapCallback perMapCallback,
    void* user,
    FirestaffSckCorpusVerifierCorpusStats* outStats) {
    DIR* dir;
    struct dirent* ent;
    FirestaffSckCorpusVerifierCorpusStats local;
    unsigned int mapCount = 0u;

    if (!corpusDir) {
        return FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG;
    }
    memset(&local, 0, sizeof(local));
    dir = opendir(corpusDir);
    if (!dir) {
        return FIRESTAFF_SCK_CORPUS_ERR_DIR_OPEN_FAILED;
    }
    while ((ent = readdir(dir)) != NULL) {
        const char* name = ent->d_name;
        size_t nameLen;
        char path[FIRESTAFF_SCK_CORPUS_VERIFIER_MAX_PATH];
        FirestaffSckCorpusVerifierMapStats stats;
        FirestaffSckCorpusVerifierResult r;

        /* Skip dot entries and non-.map files.  The bundled SCK
         * corpus uses a flat layout with `_mapping.xml` next to
         * the `.map` files; the verifier only consumes the maps. */
        if (!name || name[0] == '.') {
            continue;
        }
        nameLen = strlen(name);
        if (nameLen < 5u) {
            continue;
        }
        if (strcmp(name + nameLen - 4u, ".map") != 0) {
            continue;
        }
        if (mapCount >= FIRESTAFF_SCK_CORPUS_VERIFIER_MAX_MAPS) {
            closedir(dir);
            return FIRESTAFF_SCK_CORPUS_ERR_TOO_MANY_MAPS;
        }
        snprintf(path, sizeof(path), "%s/%s", corpusDir, name);

        r = FirestaffSckCorpusVerifier_VerifyMapfilePath(
            path, targetFileBytes, &stats);
        ++mapCount;
        ++local.totalMapfiles;
        local.totalRows += stats.parseableRows + stats.parseFailRows;
        local.parseableRows += stats.parseableRows;
        local.parseFailRows += stats.parseFailRows;
        local.sizedRows += stats.sizedRows;
        local.unsizedRows += stats.unsizedRows;
        local.oversizedRows += stats.oversizedRows;
        local.preserveRawAttributeRows += stats.preserveRawAttributeRows;

        switch (r) {
        case FIRESTAFF_SCK_CORPUS_OK:
            ++local.parseableMapfiles;
            if (stats.parseableRows == 0u) {
                ++local.emptyMapfiles;
            }
            if (stats.oversizedRows > 0u) {
                ++local.oversizedMapfiles;
            }
            break;
        case FIRESTAFF_SCK_CORPUS_ERR_BAD_HEADER:
            if (stats.truncatedByRowCap) {
                ++local.rowCapMapfiles;
            } else {
                ++local.parseFailMapfiles;
            }
            break;
        default:
            /* OPEN_FILE / NULL_ARG / etc.: count the failure but
             * keep walking so the corpus report is complete. */
            ++local.parseFailMapfiles;
            break;
        }
        if (perMapCallback) {
            perMapCallback(&stats, user);
        }
    }
    closedir(dir);
    if (outStats) {
        *outStats = local;
    }
    return FIRESTAFF_SCK_CORPUS_OK;
}

const char* FirestaffSckCorpusVerifier_ResultString(
    FirestaffSckCorpusVerifierResult result) {
    switch (result) {
    case FIRESTAFF_SCK_CORPUS_OK:
        return "OK";
    case FIRESTAFF_SCK_CORPUS_ERR_NULL_ARG:
        return "null argument";
    case FIRESTAFF_SCK_CORPUS_ERR_BAD_HEADER:
        return "bad header";
    case FIRESTAFF_SCK_CORPUS_ERR_OPEN_FILE:
        return "open file failed";
    case FIRESTAFF_SCK_CORPUS_ERR_TOO_MANY_MAPS:
        return "too many mapfiles in corpus";
    case FIRESTAFF_SCK_CORPUS_ERR_NOT_A_DIRECTORY:
        return "not a directory";
    case FIRESTAFF_SCK_CORPUS_ERR_DIR_OPEN_FAILED:
        return "directory open failed";
    }
    return "unknown";
}
