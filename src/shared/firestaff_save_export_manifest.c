/*
 * firestaff_save_export_manifest.c
 *
 * Per-game save-byte export / import manifest layer.
 * See include/firestaff_save_export_manifest.h for scope,
 * what this is NOT, and source of truth.
 *
 * Implementation notes:
 *   - Detection reads ONLY the documented magic + version
 *     prefix of each kind. It does not parse the full save
 *     header (the per-game save modules own that contract).
 *   - Sidecar JSON is written/read with a minimal
 *     hand-rolled tokenizer that mirrors the
 *     config_m12.c m12_json_next_token pattern. It only
 *     accepts flat key/value objects with strings,
 *     integers, and the documented version/type/kind/
 *     magic/format_version/body_crc32/file_size/
 *     source_path/exported_bytes/exported_at_unix fields.
 *   - Body CRC32 mirrors DM1_CRC32 (IEEE 802.3 reflected,
 *     poly 0xEDB88320, init 0xFFFFFFFF, xorout 0xFFFFFFFF).
 *     The same polynomial the rest of the codebase uses,
 *     so a manifest's body_crc32 matches
 *     DM1_CRC32(saveBytes) byte-for-byte.
 *   - File I/O uses C stdio. Path handling delegates to
 *     fs_portable_compat.h for cross-platform join /
 *     parent-dir / recursive-mkdir / exists checks.
 *   - No dynamic allocation beyond FILE buffers; the JSON
 *     parser is bounded by token / line length caps.
 */

#include "firestaff_save_export_manifest.h"
#include "fs_portable_compat.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/* ── Constants ──────────────────────────────────────────── */

#define MAGIC_DM1_V1   "FSDM1SV1"   /* include/dm1_v1_save_load.h */
#define MAGIC_CSB_V1   "RDMCSB20"   /* include/memory_savegame_pc34_compat.h SaveGameHeader_Compat */
#define MAGIC_NEXUS_V1 "FNXS"       /* include/nexus_v1_save.h */
#define MAGIC_THERON_V1 "TQR "      /* include/theron_v1_save_load.h */

#define MAGIC_LEN_DM1_V1    8
#define MAGIC_LEN_CSB_V1    8
#define MAGIC_LEN_NEXUS_V1  4
#define MAGIC_LEN_THERON_V1 4

#define VERSION_DM1_V1    1   /* DM1_SAVE_FORMAT_VERSION */
#define VERSION_CSB_V1    1   /* SaveGameHeader_Compat.formatVersion */
#define VERSION_NEXUS_V1  2   /* NEXUS_SAVE_VERSION */
#define VERSION_THERON_V1 1   /* theron .tqsv format version */

/* DM2 slot-magic tokens. The DM2 slot layout uses 0xBEEF /
 * 0xDEAD sentinels per docs/dm2_save_slots.md. We do not
 * attempt full layout detection here — the launcher can
 * still pass DM2_V1 explicitly with a synthetic DM2 slot
 * already verified by the DM2 save-load unit tests. */
#define DM2_SLOT_MAGIC_BEEF 0xBEEFu
#define DM2_SLOT_MAGIC_DEAD 0xDEADu

/* Minimal header bytes the detector reads for each kind.
 * Keeps detection bounded so a truncated / corrupt save
 * does not get misclassified as a known kind. */
#define HEADER_PROBE_DM1_V1   64
#define HEADER_PROBE_CSB_V1   64
#define HEADER_PROBE_DM2_V1   16
#define HEADER_PROBE_NEXUS_V1 64
#define HEADER_PROBE_THERON_V1 32

/* Sidecar JSON tokenizer caps. Mirrors the M12 config
 * JSON tokenizer in config_m12.c. */
#define FSM_TOKEN_MAX 384
#define FSM_KEY_MAX   96
#define FSM_STR_MAX   256

/* ── Per-kind metadata table ────────────────────────────── */

typedef struct KindInfo {
    FirestaffSaveExportKind kind;
    const char* token;       /* lowercase JSON token */
    const char* magic;       /* NUL-padded magic string (zero-pad below 8 bytes) */
    int magicLen;
    uint32_t formatVersion;
    int headerProbe;         /* bytes the detector reads */
} KindInfo;

static const KindInfo k_kind_table[] = {
    {
        FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,
        "dm1_v1",
        MAGIC_DM1_V1, MAGIC_LEN_DM1_V1,
        VERSION_DM1_V1,
        HEADER_PROBE_DM1_V1
    },
    {
        FIRESTAFF_SAVE_EXPORT_KIND_CSB_V1,
        "csb_v1",
        MAGIC_CSB_V1, MAGIC_LEN_CSB_V1,
        VERSION_CSB_V1,
        HEADER_PROBE_CSB_V1
    },
    {
        FIRESTAFF_SAVE_EXPORT_KIND_DM2_V1,
        "dm2_v1",
        "BEEF\0DEAD", 4,                  /* see DM2_SLOT_MAGIC_* below */
        1,
        HEADER_PROBE_DM2_V1
    },
    {
        FIRESTAFF_SAVE_EXPORT_KIND_NEXUS_V1,
        "nexus_v1",
        MAGIC_NEXUS_V1, MAGIC_LEN_NEXUS_V1,
        VERSION_NEXUS_V1,
        HEADER_PROBE_NEXUS_V1
    },
    {
        FIRESTAFF_SAVE_EXPORT_KIND_THERON_V1,
        "theron_v1",
        MAGIC_THERON_V1, MAGIC_LEN_THERON_V1,
        VERSION_THERON_V1,
        HEADER_PROBE_THERON_V1
    }
};
#define KIND_TABLE_LEN (int)(sizeof(k_kind_table)/sizeof(k_kind_table[0]))

static const KindInfo* kind_info(FirestaffSaveExportKind kind) {
    int i;
    for (i = 0; i < KIND_TABLE_LEN; ++i) {
        if (k_kind_table[i].kind == kind) return &k_kind_table[i];
    }
    return NULL;
}

/* ── Result / kind strings ──────────────────────────────── */

const char* FirestaffSaveExportResult_String(FirestaffSaveExportResult rc) {
    switch (rc) {
        case FIRESTAFF_SAVE_EXPORT_OK:               return "OK";
        case FIRESTAFF_SAVE_EXPORT_NULL_ARG:         return "NULL_ARG";
        case FIRESTAFF_SAVE_EXPORT_BAD_PATH:         return "BAD_PATH";
        case FIRESTAFF_SAVE_EXPORT_KIND_NOT_DETECTED: return "KIND_NOT_DETECTED";
        case FIRESTAFF_SAVE_EXPORT_KIND_MISMATCH:    return "KIND_MISMATCH";
        case FIRESTAFF_SAVE_EXPORT_BAD_MAGIC:        return "BAD_MAGIC";
        case FIRESTAFF_SAVE_EXPORT_BAD_VERSION:      return "BAD_VERSION";
        case FIRESTAFF_SAVE_EXPORT_BAD_CRC:          return "BAD_CRC";
        case FIRESTAFF_SAVE_EXPORT_BAD_SIZE:         return "BAD_SIZE";
        case FIRESTAFF_SAVE_EXPORT_FILE_OPEN:        return "FILE_OPEN";
        case FIRESTAFF_SAVE_EXPORT_FILE_READ:        return "FILE_READ";
        case FIRESTAFF_SAVE_EXPORT_FILE_WRITE:       return "FILE_WRITE";
        case FIRESTAFF_SAVE_EXPORT_TARGET_EXISTS:    return "TARGET_EXISTS";
        case FIRESTAFF_SAVE_EXPORT_PARSE_FAILED:     return "PARSE_FAILED";
        case FIRESTAFF_SAVE_EXPORT_IO_ERROR:         return "IO_ERROR";
    }
    return "UNKNOWN";
}

const char* FirestaffSaveExportKind_Token(FirestaffSaveExportKind kind) {
    const KindInfo* info = kind_info(kind);
    return info ? info->token : NULL;
}

FirestaffSaveExportKind FirestaffSaveExportKind_Parse(const char* token) {
    int i;
    if (!token || !token[0]) return FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN;
    for (i = 0; i < KIND_TABLE_LEN; ++i) {
        if (strcmp(token, k_kind_table[i].token) == 0) {
            return k_kind_table[i].kind;
        }
    }
    return FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN;
}

/* ── CRC32 (IEEE 802.3 reflected) ───────────────────────── */

/* Same polynomial / init / xorout as DM1_CRC32 and the
 * firestaff_m10_savegame probe. Matches the documented
 * "123456789" → 0xCBF43926 vector. */
uint32_t FirestaffSaveExport_CRC32(const unsigned char* data, size_t in_size) {
    uint32_t crc = 0xFFFFFFFFu;
    size_t i;
    int j;
    if (!data || in_size == 0) return 0u;
    for (i = 0; i < in_size; ++i) {
        crc ^= data[i];
        for (j = 0; j < 8; ++j) {
            if (crc & 1u) crc = (crc >> 1) ^ 0xEDB88320u;
            else crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

/* ── Detection ──────────────────────────────────────────── */

static int bytes_eq(const unsigned char* a, const char* b, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        if ((char)a[i] != b[i]) return 0;
    }
    return 1;
}

/* Read a little-endian uint32 from offset in bytes. Caller
 * is responsible for bounds. */
static uint32_t read_u32_le(const unsigned char* bytes, size_t off) {
    return ((uint32_t)bytes[off]) |
           ((uint32_t)bytes[off + 1] << 8) |
           ((uint32_t)bytes[off + 2] << 16) |
           ((uint32_t)bytes[off + 3] << 24);
}

static int probe_kind(const unsigned char* bytes, size_t in_size,
                      FirestaffSaveExportKind* outKind,
                      char* outMagic, size_t outMagicSize,
                      uint32_t* outFormatVersion) {
    int i;
    int best = 0;
    FirestaffSaveExportKind bestKind = FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN;

    if (!bytes || in_size < 4) return 0;

    /* Try each known kind and accept the first whose magic
     * matches and whose headerProbe bytes are available
     * for the version check. DM2 slot-magic detection is
     * handled separately because the magic can sit anywhere
     * inside the slot. */
    for (i = 0; i < KIND_TABLE_LEN; ++i) {
        const KindInfo* info = &k_kind_table[i];
        if (info->kind == FIRESTAFF_SAVE_EXPORT_KIND_DM2_V1) continue;
        if (in_size < (size_t)info->headerProbe) continue;
        if (in_size < (size_t)info->magicLen) continue;
        if (!bytes_eq(bytes, info->magic, info->magicLen)) continue;

        /* Read format version. For 8-byte magic (DM1, CSB)
         * the formatVersion field sits right after magic at
         * offset 8; for 4-byte magic (Nexus, Theron) it
         * sits at offset 4. */
        if (info->magicLen == 8) {
            if (in_size < 12) continue;
        } else if (info->magicLen == 4) {
            if (in_size < 8) continue;
        }
        {
            uint32_t v = read_u32_le(bytes, (size_t)info->magicLen);
            if (v != info->formatVersion) {
                /* Magic matches but version is unexpected.
                 * Accept only if v == 0 (legacy unset) or if
                 * it matches the documented version. For
                 * now, only the documented version is
                 * accepted so the detector stays strict. */
                continue;
            }
        }
        best = 1;
        bestKind = info->kind;
        break;
    }

    /* DM2 slot-magic: the documented layout has a 0xBEEF
     * sentinel at the slot header. We accept either 0xBEEF
     * or 0xDEAD at offset 0 with a plausible next byte. */
    if (!best && in_size >= 4) {
        uint32_t slot0 = read_u32_le(bytes, 0);
        if (slot0 == DM2_SLOT_MAGIC_BEEF || slot0 == DM2_SLOT_MAGIC_DEAD) {
            best = 1;
            bestKind = FIRESTAFF_SAVE_EXPORT_KIND_DM2_V1;
        }
    }

    if (!best) return 0;

    if (outKind) *outKind = bestKind;
    if (outFormatVersion) {
        const KindInfo* info = kind_info(bestKind);
        *outFormatVersion = info ? info->formatVersion : 0u;
    }
    if (outMagic && outMagicSize > 0) {
        const KindInfo* info = kind_info(bestKind);
        outMagic[0] = '\0';
        if (info) {
            size_t copyLen = (size_t)info->magicLen;
            if (copyLen >= outMagicSize) copyLen = outMagicSize - 1u;
            memcpy(outMagic, info->magic, copyLen);
            outMagic[copyLen] = '\0';
        }
    }
    return 1;
}

FirestaffSaveExportKind FirestaffSaveExport_DetectKind(
        const unsigned char* bytes, size_t in_size,
        char* outMagic, size_t outMagicSize,
        uint32_t* outFormatVersion) {
    FirestaffSaveExportKind kind = FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN;
    if (outMagic && outMagicSize > 0) outMagic[0] = '\0';
    if (outFormatVersion) *outFormatVersion = 0u;
    if (!probe_kind(bytes, in_size, &kind, outMagic, outMagicSize, outFormatVersion)) {
        return FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN;
    }
    return kind;
}

FirestaffSaveExportKind FirestaffSaveExport_DetectKindFromFile(
        const char* path,
        char* outMagic, size_t outMagicSize,
        uint32_t* outFormatVersion,
        char* outError, size_t outErrorSize) {
    FILE* fp;
    unsigned char prefix[FIRESTAFF_SAVE_EXPORT_PROBE_MAX];
    size_t n;
    FirestaffSaveExportKind kind = FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN;

    if (outMagic && outMagicSize > 0) outMagic[0] = '\0';
    if (outFormatVersion) *outFormatVersion = 0u;
    if (outError && outErrorSize > 0) outError[0] = '\0';

    if (!path) {
        if (outError && outErrorSize > 0) {
            snprintf(outError, outErrorSize, "null path");
        }
        return FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN;
    }

    fp = fopen(path, "rb");
    if (!fp) {
        if (outError && outErrorSize > 0) {
            snprintf(outError, outErrorSize, "open failed: %s", strerror(errno));
        }
        return FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN;
    }
    n = fread(prefix, 1, sizeof(prefix), fp);
    fclose(fp);
    if (n == 0) {
        if (outError && outErrorSize > 0) {
            snprintf(outError, outErrorSize, "empty or unreadable file");
        }
        return FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN;
    }

    if (!probe_kind(prefix, n, &kind, outMagic, outMagicSize, outFormatVersion)) {
        if (outError && outErrorSize > 0) {
            snprintf(outError, outErrorSize,
                     "no recognised save magic in first %zu bytes", n);
        }
        return FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN;
    }
    return kind;
}

/* ── Path helpers ───────────────────────────────────────── */

int FirestaffSaveExport_BuildPaths(const char* dir,
                                   const char* sourceBasename,
                                   char* outBin, size_t outBinSize,
                                   char* outManifest, size_t outManifestSize) {
    char binName[512];
    char jsonName[512];
    size_t baseLen;
    int rc;

    if (!dir || !sourceBasename || !outBin || !outManifest) return 0;

    baseLen = strlen(sourceBasename);
    /* Strip any trailing .sav / .tqsv to keep the bundle
     * basename stable across per-game extensions. */
    if (baseLen > 4 && strcmp(sourceBasename + baseLen - 4, ".sav") == 0) {
        baseLen -= 4;
    } else if (baseLen > 5 && strcmp(sourceBasename + baseLen - 5, ".tqsv") == 0) {
        baseLen -= 5;
    }
    if (baseLen == 0 || baseLen >= sizeof(binName) - 32u) return 0;

    rc = snprintf(binName, sizeof(binName), "%.*s%s",
                  (int)baseLen, sourceBasename,
                  FIRESTAFF_SAVE_EXPORT_BIN_SUFFIX);
    if (rc <= 0 || (size_t)rc >= sizeof(binName)) return 0;

    rc = snprintf(jsonName, sizeof(jsonName), "%.*s%s",
                  (int)baseLen, sourceBasename,
                  FIRESTAFF_SAVE_EXPORT_JSON_SUFFIX);
    if (rc <= 0 || (size_t)rc >= sizeof(jsonName)) return 0;

    if (!FSP_JoinPath(outBin, outBinSize, dir, binName)) return 0;
    if (!FSP_JoinPath(outManifest, outManifestSize, dir, jsonName)) return 0;
    return 1;
}

/* ── Tiny JSON writer (only what the sidecar needs) ─────── */

static void json_write_escaped_string(FILE* fp, const char* s) {
    fputc('"', fp);
    if (!s) {
        fputc('"', fp);
        return;
    }
    while (*s) {
        unsigned char c = (unsigned char)*s++;
        switch (c) {
            case '"':  fputs("\\\"", fp); break;
            case '\\': fputs("\\\\", fp); break;
            case '\b': fputs("\\b", fp); break;
            case '\f': fputs("\\f", fp); break;
            case '\n': fputs("\\n", fp); break;
            case '\r': fputs("\\r", fp); break;
            case '\t': fputs("\\t", fp); break;
            default:
                if (c < 0x20) {
                    fprintf(fp, "\\u%04x", c);
                } else {
                    fputc((int)c, fp);
                }
        }
    }
    fputc('"', fp);
}

/* ── Tiny JSON reader (flat key/value strings + integers) ──
 *
 * Mirrors the M12 config JSON tokenizer. Accepts:
 *   { "key" : "string" , "key2" : 12345 }
 * Anything else returns PARSE_FAILED.
 */

typedef struct JsonReader {
    FILE* fp;
    int line;
    int hasError;
} JsonReader;

static void jr_error(JsonReader* jr, const char* msg) {
    if (jr->hasError) return;
    jr->hasError = 1;
    fprintf(stderr, "firestaff_save_export_manifest: parse error line %d: %s\n",
            jr->line, msg ? msg : "?");
}

static int jr_read_char(JsonReader* jr, int* outCh) {
    int ch = fgetc(jr->fp);
    if (ch == '\n') ++jr->line;
    *outCh = ch;
    return ch != EOF;
}

/* Skip whitespace + C++ / C comments. Returns 1 if a
 * non-whitespace character is available, 0 on EOF, -1 on
 * error. */
static int jr_skip_ws(JsonReader* jr, int* outCh) {
    while (1) {
        int ch;
        if (!jr_read_char(jr, &ch)) return 0;
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') continue;
        *outCh = ch;
        return 1;
    }
}

/* Read a JSON token into buf. Returns 1 on success, 0 on
 * EOF, -1 on error. */
static int jr_next_token(JsonReader* jr, char* buf, size_t bufSize) {
    int ch;
    size_t i = 0u;
    if (jr_skip_ws(jr, &ch) <= 0) return 0;

    /* Single-char tokens: { } [ ] , : */
    if (ch == '{' || ch == '}' || ch == '[' || ch == ']' ||
        ch == ',' || ch == ':') {
        if (bufSize < 2u) return -1;
        buf[0] = (char)ch;
        buf[1] = '\0';
        return 1;
    }

    /* String literal */
    if (ch == '"') {
        if (bufSize < 2u) return -1;
        buf[i++] = (char)ch;
        while (1) {
            int c;
            if (!jr_read_char(jr, &c)) { jr_error(jr, "unterminated string"); return -1; }
            if (c == '\\') {
                if (i + 2u >= bufSize) return -1;
                buf[i++] = (char)c;
                if (!jr_read_char(jr, &c)) { jr_error(jr, "unterminated escape"); return -1; }
                buf[i++] = (char)c;
                continue;
            }
            if (i + 1u >= bufSize) { jr_error(jr, "token too long"); return -1; }
            buf[i++] = (char)c;
            if (c == '"') break;
        }
        buf[i] = '\0';
        return 1;
    }

    /* Number or identifier (we use it for integers) */
    {
        int started = 0;
        while (ch && ch != ' ' && ch != '\t' && ch != '\r' &&
               ch != '\n' && ch != ',' && ch != '}' &&
               ch != ']' && ch != ':') {
            if (i + 1u >= bufSize) { jr_error(jr, "token too long"); return -1; }
            buf[i++] = (char)ch;
            started = 1;
            if (!jr_read_char(jr, &ch)) break;
        }
        if (!started) {
            jr_error(jr, "unexpected character");
            return -1;
        }
        buf[i] = '\0';
        return 1;
    }
}

/* Read a JSON-quoted string's content into out (NUL
 * terminated, unescaped for the simple \", \\, \n, \r, \t
 * cases). Returns 1 on success, 0 on missing / EOF, -1 on
 * error. */
static int jr_read_string(const char* quoted,
                          char* out, size_t outSize) {
    size_t i = 0u;
    size_t src;
    size_t len;

    if (!quoted || quoted[0] != '"') return 0;
    if (outSize == 0u) return 0;
    len = strlen(quoted);
    if (len < 2u || quoted[len - 1u] != '"') return 0;

    src = 1u;
    while (src < len - 1u) {
        char c = quoted[src++];
        if (c == '\\' && src < len - 1u) {
            char esc = quoted[src++];
            switch (esc) {
                case '"':  c = '"'; break;
                case '\\': c = '\\'; break;
                case '/':  c = '/'; break;
                case 'n':  c = '\n'; break;
                case 'r':  c = '\r'; break;
                case 't':  c = '\t'; break;
                case 'b':  c = '\b'; break;
                case 'f':  c = '\f'; break;
                default:   c = esc; break;
            }
        }
        if (i + 1u >= outSize) break;
        out[i++] = c;
    }
    out[i] = '\0';
    return 1;
}

static int jr_unquote_key(const char* quoted, char* out, size_t outSize) {
    return jr_read_string(quoted, out, outSize);
}

/* Parse a JSON int token (decimal, optional minus). Returns
 * 1 on success, 0 on missing. */
static int jr_parse_int(const char* token, long long* out) {
    char* end = NULL;
    long long v;
    if (!token || !*token) return 0;
    v = strtoll(token, &end, 10);
    if (end == token) return 0;
    *out = v;
    return 1;
}

/* ── File copy (bounded buffer) ─────────────────────────── */

/* Compute CRC32 of an entire file. Returns 0 and sets
 * outError on failure. */
static int crc32_of_file(const char* path, uint32_t* outCrc,
                         char* outError, size_t outErrorSize) {
    FILE* rfp = fopen(path, "rb");
    unsigned char buf[8192];
    size_t n;
    uint32_t crc;

    if (!rfp) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "open read failed: %s", strerror(errno));
        return 0;
    }
    crc = 0xFFFFFFFFu;
    while ((n = fread(buf, 1, sizeof(buf), rfp)) > 0) {
        size_t i;
        int j;
        for (i = 0; i < n; ++i) {
            crc ^= buf[i];
            for (j = 0; j < 8; ++j) {
                if (crc & 1u) crc = (crc >> 1) ^ 0xEDB88320u;
                else crc >>= 1;
            }
        }
    }
    fclose(rfp);
    *outCrc = crc ^ 0xFFFFFFFFu;
    return 1;
}

static int copy_bytes(const char* src, const char* dst) {
    FILE* in = fopen(src, "rb");
    FILE* out;
    unsigned char buf[8192];
    size_t n;
    int ok = 1;

    if (!in) return 0;
    out = fopen(dst, "wb");
    if (!out) { fclose(in); return 0; }
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) { ok = 0; break; }
    }
    if (ferror(in)) ok = 0;
    if (fclose(out) != 0) ok = 0;
    fclose(in);
    return ok;
}

/* ── Export ─────────────────────────────────────────────── */

FirestaffSaveExportResult FirestaffSaveExport_ExportFileWithKind(
        const char* sourcePath,
        const char* exportDir,
        FirestaffSaveExportKind kind,
        char* outBinPath, size_t outBinPathSize,
        char* outManifestPath, size_t outManifestPathSize,
        char* outError, size_t outErrorSize) {
    const KindInfo* info;
    char magic[FSM_STR_MAX] = {0};
    char binPath[FSP_PATH_MAX] = {0};
    char manifestPath[FSP_PATH_MAX] = {0};
    char manifestTmp[FSP_PATH_MAX + 16] = {0};
    char baseName[256] = {0};
    const char* slash;
    uint32_t detectedVersion = 0u;
    struct stat st;
    FILE* fp;
    uint32_t crc = 0u;
    long fileSize = 0L;
    time_t now = 0;

    if (outError && outErrorSize > 0) outError[0] = '\0';
    if (outBinPath && outBinPathSize > 0) outBinPath[0] = '\0';
    if (outManifestPath && outManifestPathSize > 0) outManifestPath[0] = '\0';

    if (!sourcePath || !exportDir) {
        if (outError && outErrorSize > 0) snprintf(outError, outErrorSize, "null arg");
        return FIRESTAFF_SAVE_EXPORT_NULL_ARG;
    }
    info = kind_info(kind);
    if (!info) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "unsupported kind");
        return FIRESTAFF_SAVE_EXPORT_KIND_NOT_DETECTED;
    }

    if (!FSP_FileExists(sourcePath)) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "source not found: %s", sourcePath);
        return FIRESTAFF_SAVE_EXPORT_BAD_PATH;
    }

    if (stat(sourcePath, &st) != 0) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "stat failed: %s", strerror(errno));
        return FIRESTAFF_SAVE_EXPORT_IO_ERROR;
    }
    fileSize = (long)st.st_size;

    /* Detect on-disk magic + version. */
    {
        FirestaffSaveExportKind detected = FirestaffSaveExport_DetectKindFromFile(
                sourcePath, magic, sizeof(magic), &detectedVersion,
                outError, outErrorSize);
        if (detected == FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN) {
            return FIRESTAFF_SAVE_EXPORT_KIND_NOT_DETECTED;
        }
        if (detected != kind) {
            if (outError && outErrorSize > 0)
                snprintf(outError, outErrorSize,
                         "detected kind != requested kind (detected=%s, requested=%s)",
                         FirestaffSaveExportKind_Token(detected), info->token);
            return FIRESTAFF_SAVE_EXPORT_KIND_MISMATCH;
        }
        if (detectedVersion != info->formatVersion) {
            if (outError && outErrorSize > 0)
                snprintf(outError, outErrorSize,
                         "version mismatch (detected=%u, expected=%u)",
                         detectedVersion, info->formatVersion);
            return FIRESTAFF_SAVE_EXPORT_BAD_VERSION;
        }
    }

    /* Compute body CRC32 (the same CRC32 the runtime would
     * write into the save's own header for DM1 / CSB /
     * Nexus; Theron .tqsv uses a different integrity check
     * and we still record the CRC32 of the bytes as a
     * sidecar receipt). */
    if (!crc32_of_file(sourcePath, &crc, outError, outErrorSize)) {
        return FIRESTAFF_SAVE_EXPORT_FILE_OPEN;
    }

    /* Derive basename from sourcePath. */
    slash = strrchr(sourcePath, '/');
#ifdef _WIN32
    { const char* bs = strrchr(sourcePath, '\\');
      if (bs && (!slash || bs > slash)) slash = bs; }
#endif
    {
        size_t baseLen;
        if (!slash) slash = sourcePath;
        else slash++;
        baseLen = strlen(slash);
        if (baseLen >= sizeof(baseName)) baseLen = sizeof(baseName) - 1u;
        memcpy(baseName, slash, baseLen);
        baseName[baseLen] = '\0';
    }
    if (baseName[0] == '\0') {
        if (outError && outErrorSize > 0) snprintf(outError, outErrorSize, "empty basename");
        return FIRESTAFF_SAVE_EXPORT_BAD_PATH;
    }

    if (!FSP_CreateDirectoryRecursive(exportDir)) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "mkdir export dir failed: %s", exportDir);
        return FIRESTAFF_SAVE_EXPORT_IO_ERROR;
    }

    if (!FirestaffSaveExport_BuildPaths(exportDir, baseName,
                                        binPath, sizeof(binPath),
                                        manifestPath, sizeof(manifestPath))) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "build paths failed");
        return FIRESTAFF_SAVE_EXPORT_BAD_PATH;
    }

    if (!copy_bytes(sourcePath, binPath)) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "copy bytes to %s failed", binPath);
        return FIRESTAFF_SAVE_EXPORT_FILE_WRITE;
    }

    /* Write manifest atomically via a .tmp rename. */
    snprintf(manifestTmp, sizeof(manifestTmp), "%s.tmp", manifestPath);
    fp = fopen(manifestTmp, "wb");
    if (!fp) {
        remove(binPath);
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "open manifest tmp failed: %s", strerror(errno));
        return FIRESTAFF_SAVE_EXPORT_FILE_WRITE;
    }

    now = time(NULL);

    fprintf(fp, "{\n");
    fprintf(fp, "  \"version\": ");
    json_write_escaped_string(fp, FIRESTAFF_SAVE_EXPORT_MANIFEST_VERSION);
    fprintf(fp, ",\n  \"type\": ");
    json_write_escaped_string(fp, FIRESTAFF_SAVE_EXPORT_MANIFEST_TYPE);
    fprintf(fp, ",\n  \"kind\": ");
    json_write_escaped_string(fp, info->token);
    fprintf(fp, ",\n  \"magic\": ");
    json_write_escaped_string(fp, magic);
    fprintf(fp, ",\n  \"format_version\": %u,\n", info->formatVersion);
    fprintf(fp, "  \"body_crc32\": %lu,\n", (unsigned long)crc);
    fprintf(fp, "  \"file_size\": %ld,\n", fileSize);
    fprintf(fp, "  \"source_path\": ");
    json_write_escaped_string(fp, sourcePath);
    {
        char binBase[256];
        const char* bp = strrchr(binPath, '/');
#ifdef _WIN32
        const char* bbs = strrchr(binPath, '\\');
        if (bbs && (!bp || bbs > bp)) bp = bbs;
#endif
        if (!bp) bp = binPath;
        else bp++;
        snprintf(binBase, sizeof(binBase), "%s", bp);
        fprintf(fp, ",\n  \"exported_bytes\": ");
        json_write_escaped_string(fp, binBase);
    }
    fprintf(fp, ",\n  \"exported_at_unix\": %lld\n", (long long)now);
    fprintf(fp, "}\n");
    if (fclose(fp) != 0) {
        remove(manifestTmp);
        remove(binPath);
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "manifest close failed: %s", strerror(errno));
        return FIRESTAFF_SAVE_EXPORT_FILE_WRITE;
    }
    if (rename(manifestTmp, manifestPath) != 0) {
        remove(manifestTmp);
        remove(binPath);
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "manifest rename failed: %s", strerror(errno));
        return FIRESTAFF_SAVE_EXPORT_FILE_WRITE;
    }

    if (outBinPath && outBinPathSize > 0) {
        snprintf(outBinPath, outBinPathSize, "%s", binPath);
    }
    if (outManifestPath && outManifestPathSize > 0) {
        snprintf(outManifestPath, outManifestPathSize, "%s", manifestPath);
    }
    return FIRESTAFF_SAVE_EXPORT_OK;
}

FirestaffSaveExportResult FirestaffSaveExport_ExportFile(
        const char* sourcePath,
        const char* exportDir,
        char* outBinPath, size_t outBinPathSize,
        char* outManifestPath, size_t outManifestPathSize,
        char* outError, size_t outErrorSize) {
    FirestaffSaveExportKind detected;
    char magic[FSM_STR_MAX] = {0};
    uint32_t version = 0u;

    if (outError && outErrorSize > 0) outError[0] = '\0';

    detected = FirestaffSaveExport_DetectKindFromFile(
            sourcePath, magic, sizeof(magic), &version,
            outError, outErrorSize);
    if (detected == FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN) {
        return FIRESTAFF_SAVE_EXPORT_KIND_NOT_DETECTED;
    }
    return FirestaffSaveExport_ExportFileWithKind(
            sourcePath, exportDir, detected,
            outBinPath, outBinPathSize,
            outManifestPath, outManifestPathSize,
            outError, outErrorSize);
}

/* ── Import ─────────────────────────────────────────────── */

FirestaffSaveExportResult FirestaffSaveExport_ImportFile(
        const char* importDir,
        const char* exportBasename,
        FirestaffSaveExportKind expectedKind,
        const char* expectedMagic,
        uint32_t expectedFormatVersion,
        const char* targetPath,
        char* outBinPath, size_t outBinPathSize,
        char* outManifestPath, size_t outManifestPathSize,
        char* outError, size_t outErrorSize) {
    char manifestPath[FSP_PATH_MAX] = {0};
    char binPath[FSP_PATH_MAX] = {0};
    char kindStr[FSM_STR_MAX] = {0};
    char magicStr[FSM_STR_MAX] = {0};
    char sourceStr[FSM_STR_MAX] = {0};
    char binName[FSM_STR_MAX] = {0};
    long long fileSize = -1;
    long long bodyCrc = -1;
    long long formatVersion = -1;
    long long exportedAt = -1;
    int sawType = 0, sawKind = 0, sawMagic = 0;
    int sawFormatVersion = 0, sawBodyCrc = 0, sawFileSize = 0;
    int sawSource = 0, sawBin = 0;
    JsonReader jr;
    char token[FSM_TOKEN_MAX];
    char key[FSM_KEY_MAX];
    FILE* fp;
    struct stat st;
    uint32_t computedCrc;
    FirestaffSaveExportKind manifestKind;

    if (outError && outErrorSize > 0) outError[0] = '\0';
    if (outBinPath && outBinPathSize > 0) outBinPath[0] = '\0';
    if (outManifestPath && outManifestPathSize > 0) outManifestPath[0] = '\0';

    if (!importDir || !targetPath) {
        if (outError && outErrorSize > 0) snprintf(outError, outErrorSize, "null arg");
        return FIRESTAFF_SAVE_EXPORT_NULL_ARG;
    }

    /* If exportBasename was given, append the .savebin / .savebin.json
     * suffixes and use it directly. Otherwise require importDir to
     * contain exactly one <basename>.savebin + matching JSON pair.
     * The launcher can pre-discover the basename via the save browser
     * before calling this function. */
    if (!exportBasename || !exportBasename[0]) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "exportBasename is required");
        return FIRESTAFF_SAVE_EXPORT_BAD_PATH;
    }
    {
        char basePlain[512];
        size_t baseLen = strlen(exportBasename);
        if (baseLen >= sizeof(basePlain)) {
            if (outError && outErrorSize > 0)
                snprintf(outError, outErrorSize, "exportBasename too long");
            return FIRESTAFF_SAVE_EXPORT_BAD_PATH;
        }
        /* Strip any of the suffixes the caller might have added. */
        if (baseLen > strlen(FIRESTAFF_SAVE_EXPORT_JSON_SUFFIX) &&
            strcmp(exportBasename + baseLen - strlen(FIRESTAFF_SAVE_EXPORT_JSON_SUFFIX),
                   FIRESTAFF_SAVE_EXPORT_JSON_SUFFIX) == 0) {
            baseLen -= strlen(FIRESTAFF_SAVE_EXPORT_JSON_SUFFIX);
        } else if (baseLen > strlen(FIRESTAFF_SAVE_EXPORT_BIN_SUFFIX) &&
                   strcmp(exportBasename + baseLen - strlen(FIRESTAFF_SAVE_EXPORT_BIN_SUFFIX),
                          FIRESTAFF_SAVE_EXPORT_BIN_SUFFIX) == 0) {
            baseLen -= strlen(FIRESTAFF_SAVE_EXPORT_BIN_SUFFIX);
        }
        memcpy(basePlain, exportBasename, baseLen);
        basePlain[baseLen] = '\0';
        if (!FirestaffSaveExport_BuildPaths(importDir, basePlain,
                                            binPath, sizeof(binPath),
                                            manifestPath, sizeof(manifestPath))) {
            if (outError && outErrorSize > 0)
                snprintf(outError, outErrorSize, "build paths failed");
            return FIRESTAFF_SAVE_EXPORT_BAD_PATH;
        }
    }

    fp = fopen(manifestPath, "rb");
    if (!fp) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "open manifest %s: %s",
                     manifestPath, strerror(errno));
        return FIRESTAFF_SAVE_EXPORT_FILE_OPEN;
    }
    memset(&jr, 0, sizeof(jr));
    jr.fp = fp;
    jr.line = 1;

    if (jr_next_token(&jr, token, sizeof(token)) <= 0 || strcmp(token, "{") != 0) {
        fclose(fp);
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "manifest missing '{'");
        return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
    }

    while (jr_next_token(&jr, token, sizeof(token)) > 0) {
        if (strcmp(token, "}") == 0) break;
        if (strcmp(token, ",") == 0) continue;
        if (token[0] != '"') {
            fclose(fp);
            if (outError && outErrorSize > 0)
                snprintf(outError, outErrorSize, "expected key string, got %s", token);
            return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
        }
        if (!jr_unquote_key(token, key, sizeof(key))) {
            fclose(fp);
            if (outError && outErrorSize > 0)
                snprintf(outError, outErrorSize, "key parse failed");
            return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
        }
        if (jr_next_token(&jr, token, sizeof(token)) <= 0 || strcmp(token, ":") != 0) {
            fclose(fp);
            if (outError && outErrorSize > 0)
                snprintf(outError, outErrorSize, "expected ':' after key %s", key);
            return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
        }
        if (jr_next_token(&jr, token, sizeof(token)) <= 0) {
            fclose(fp);
            if (outError && outErrorSize > 0)
                snprintf(outError, outErrorSize, "expected value after key %s", key);
            return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
        }
        if (strcmp(key, "type") == 0) {
            char t[FSM_STR_MAX];
            if (!jr_read_string(token, t, sizeof(t))) {
                fclose(fp);
                if (outError && outErrorSize > 0)
                    snprintf(outError, outErrorSize, "type parse failed");
                return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
            }
            if (strcmp(t, FIRESTAFF_SAVE_EXPORT_MANIFEST_TYPE) != 0) {
                fclose(fp);
                if (outError && outErrorSize > 0)
                    snprintf(outError, outErrorSize,
                             "wrong manifest type: %s", t);
                return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
            }
            sawType = 1;
        } else if (strcmp(key, "kind") == 0) {
            if (!jr_read_string(token, kindStr, sizeof(kindStr))) {
                fclose(fp);
                return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
            }
            sawKind = 1;
        } else if (strcmp(key, "magic") == 0) {
            if (!jr_read_string(token, magicStr, sizeof(magicStr))) {
                fclose(fp);
                return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
            }
            sawMagic = 1;
        } else if (strcmp(key, "format_version") == 0) {
            long long v;
            if (!jr_parse_int(token, &v)) { fclose(fp); return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED; }
            formatVersion = v;
            sawFormatVersion = 1;
        } else if (strcmp(key, "body_crc32") == 0) {
            long long v;
            if (!jr_parse_int(token, &v)) { fclose(fp); return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED; }
            bodyCrc = v;
            sawBodyCrc = 1;
        } else if (strcmp(key, "file_size") == 0) {
            long long v;
            if (!jr_parse_int(token, &v)) { fclose(fp); return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED; }
            fileSize = v;
            sawFileSize = 1;
        } else if (strcmp(key, "source_path") == 0) {
            if (!jr_read_string(token, sourceStr, sizeof(sourceStr))) {
                fclose(fp);
                return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
            }
            sawSource = 1;
        } else if (strcmp(key, "exported_bytes") == 0) {
            if (!jr_read_string(token, binName, sizeof(binName))) {
                fclose(fp);
                return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
            }
            sawBin = 1;
        } else if (strcmp(key, "version") == 0 || strcmp(key, "exported_at_unix") == 0) {
            /* Informational only. */
            if (strcmp(key, "exported_at_unix") == 0) {
                long long v;
                if (!jr_parse_int(token, &v)) { fclose(fp); return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED; }
                exportedAt = v;
            }
        } else {
            /* Unknown key: skip the value. Numbers / strings
             * are single tokens so this is fine for the
             * sidecar shape; anything else would already have
             * been read above. */
        }
    }
    fclose(fp);

    if (!sawType || !sawKind || !sawMagic || !sawFormatVersion ||
        !sawBodyCrc || !sawFileSize || !sawBin) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize,
                     "manifest missing required field(s) "
                     "(type=%d kind=%d magic=%d fmt=%d crc=%d size=%d bin=%d)",
                     sawType, sawKind, sawMagic, sawFormatVersion,
                     sawBodyCrc, sawFileSize, sawBin);
        return FIRESTAFF_SAVE_EXPORT_PARSE_FAILED;
    }

    manifestKind = FirestaffSaveExportKind_Parse(kindStr);
    if (manifestKind == FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "unknown kind token: %s", kindStr);
        return FIRESTAFF_SAVE_EXPORT_KIND_NOT_DETECTED;
    }

    if (expectedKind != FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN &&
        manifestKind != expectedKind) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize,
                     "kind mismatch (manifest=%s, expected=%s)",
                     kindStr,
                     FirestaffSaveExportKind_Token(expectedKind));
        return FIRESTAFF_SAVE_EXPORT_KIND_MISMATCH;
    }

    if (expectedMagic && expectedMagic[0]) {
        if (strcmp(expectedMagic, magicStr) != 0) {
            if (outError && outErrorSize > 0)
                snprintf(outError, outErrorSize,
                         "magic mismatch (manifest=%s, expected=%s)",
                         magicStr, expectedMagic);
            return FIRESTAFF_SAVE_EXPORT_BAD_MAGIC;
        }
    }

    if (expectedFormatVersion != 0u) {
        if ((uint32_t)formatVersion != expectedFormatVersion) {
            if (outError && outErrorSize > 0)
                snprintf(outError, outErrorSize,
                         "version mismatch (manifest=%lld, expected=%u)",
                         formatVersion, expectedFormatVersion);
            return FIRESTAFF_SAVE_EXPORT_BAD_VERSION;
        }
    }

    /* The bin file must exist with the expected size and CRC. */
    if (stat(binPath, &st) != 0) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "bin stat failed: %s", strerror(errno));
        return FIRESTAFF_SAVE_EXPORT_FILE_OPEN;
    }
    if (fileSize >= 0 && (long long)st.st_size != fileSize) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize,
                     "file_size mismatch (manifest=%lld, actual=%lld)",
                     fileSize, (long long)st.st_size);
        return FIRESTAFF_SAVE_EXPORT_BAD_SIZE;
    }

    if (!crc32_of_file(binPath, &computedCrc, outError, outErrorSize)) {
        return FIRESTAFF_SAVE_EXPORT_FILE_OPEN;
    }

    if (bodyCrc >= 0 && (unsigned long long)computedCrc != (unsigned long long)bodyCrc) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize,
                     "body_crc32 mismatch (manifest=%lld, actual=%lu)",
                     bodyCrc, (unsigned long)computedCrc);
        return FIRESTAFF_SAVE_EXPORT_BAD_CRC;
    }

    /* No-overwrite on the target. */
    if (FSP_FileExists(targetPath)) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "target exists: %s", targetPath);
        return FIRESTAFF_SAVE_EXPORT_TARGET_EXISTS;
    }

    {
        char parentDir[FSP_PATH_MAX];
        if (FSP_ParentDir(parentDir, sizeof(parentDir), targetPath)) {
            if (!FSP_CreateDirectoryRecursive(parentDir)) {
                if (outError && outErrorSize > 0)
                    snprintf(outError, outErrorSize,
                             "mkdir target parent failed: %s", parentDir);
                return FIRESTAFF_SAVE_EXPORT_IO_ERROR;
            }
        }
    }

    if (!copy_bytes(binPath, targetPath)) {
        if (outError && outErrorSize > 0)
            snprintf(outError, outErrorSize, "copy bytes to target failed: %s", targetPath);
        return FIRESTAFF_SAVE_EXPORT_FILE_WRITE;
    }

    if (outBinPath && outBinPathSize > 0) snprintf(outBinPath, outBinPathSize, "%s", binPath);
    if (outManifestPath && outManifestPathSize > 0) snprintf(outManifestPath, outManifestPathSize, "%s", manifestPath);
    if (outError && outErrorSize > 0) {
        /* Use sawSource to keep the compiler from warning. */
        if (sawSource && sourceStr[0]) {
            snprintf(outError, outErrorSize, "OK (exported from %s)", sourceStr);
        } else {
            snprintf(outError, outErrorSize, "OK (exported_at_unix=%lld)", exportedAt);
        }
    }
    (void)manifestKind;
    return FIRESTAFF_SAVE_EXPORT_OK;
}
