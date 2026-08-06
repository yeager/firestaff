/*
 * csb_v1_csbwin_save_loader_boundary_pc34_compat.c
 *
 * CSB V1 CSBWin save-side loader-boundary evidence gate.
 *
 * Implementation note: this module is intentionally thin. It
 * classifies caller-provided CSBWin / DM1 save bytes. The focused contract
 * target may also feed compact CSBGAME fixtures into the historical roster
 * reader and records that result against the documented contract. Production
 * instead requires the complete original body before runtime import. No new
 * parser logic, no new decode path, no new data — just an
 * evidence-only gate that proves the loader boundary behaves as
 * documented today and surfaces which shapes remain OPEN-LARGE.
 *
 * Source-lock boundary (see header for full references):
 *   - ReDMCSB CEDTINC8.C:101-118 (DMSAVE / CSBGAME.DAT routing)
 *   - ReDMCSB LOADSAVE.C F0433/F0435 (CSBGAME namespace)
 *   - ReDMCSB SAVEHEAD.C F0429/F0430 (header read/write)
 *   - ReDMCSB DEFS.H:1289 (CSBGAME.DAT magic)
 *   - CSBWin SaveGame.cpp:927/1711/2111 (save file I/O)
 *   - CSBWin CSBCode.cpp:421-422 (csbgame.dat / csbgame.bak)
 *   - include/csb_v1_save_import_path_pc34_compat.h
 *     (CSB_V1_PartyState + CSB_SaveImportResult enum + header
 *     offsets used by the test-only fixture helper).
 */

#include "csb_v1_csbwin_save_loader_boundary_pc34_compat.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Documented contract table ───────────────────────────────────────── */

/* Build a small helper to keep the contract table readable. */
#define CONTRACT_ENTRY(SHAPE, LABEL, ACCEPT, CODE, EVIDENCE)               \
    { (SHAPE), (LABEL), (ACCEPT), (CODE), (EVIDENCE) }

static const CSB_V1_CSBWinSaveShapeContract k_contract_table[] = {
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_CSBGAME_V20,
        "CSB v2.0 (CSBGAME\\0 + 0x200)",
        1, 0,
        "ReDMCSB DEFS.H:1289 CSBGAME.DAT magic + LOADSAVE.C F0433"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_CSBGAME_V21,
        "CSB v2.1 (CSBGAME\\0 + 0x201)",
        1, 0,
        "ReDMCSB DEFS.H:1289 CSBGAME.DAT magic + LOADSAVE.C F0435"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_DM1_RAW_RDMCSB15,
        "DM1 raw RDMCSB15",
        0, CSB_SAVE_IMPORT_ERR_BAD_MAGIC,
        "ReDMCSB CEDTINC8.C:101-118 + memory_savegame_pc34_compat.h:227"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_CSBGAME_CDSA,
        "CSB+DSA CDSA marker",
        0, CSB_SAVE_IMPORT_ERR_NO_CHAMPIONS,
        "CSBWin SaveGame.cpp CDSA section marker (offset 12 collides with champ_count)"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1,
        "CSBWin 512-byte CSB\\1",
        0, CSB_SAVE_IMPORT_ERR_BAD_MAGIC,
        "CSBWin SaveGame.cpp:927 512-byte XOR header"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_CSBWIN_512_DM01,
        "CSBWin 512-byte DM\\0\\1",
        0, CSB_SAVE_IMPORT_ERR_BAD_MAGIC,
        "CSBWin SaveGame.cpp:1711 DM1 512-byte XOR header"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CEDT,
        "CSBWin 512-byte CEDT",
        0, CSB_SAVE_IMPORT_ERR_BAD_MAGIC,
        "CSBWin Data.h:590 SaveGameFilename CEDT section"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_TOO_SMALL_UNDER_8,
        "< 8-byte magic (too-small)",
        0, CSB_SAVE_IMPORT_ERR_TRUNCATED,
        "ReDMCSB SAVEHEAD.C F0429 header read + F0430 header write"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_NO_MAGIC_8_PLUS,
        "8+ bytes but no recognised magic",
        0, CSB_SAVE_IMPORT_ERR_BAD_MAGIC,
        "ReDMCSB CEDTINC8.C:101-118 routing + CSBWin CSBCode.cpp:421"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_0,
        "CSB v2.0 with champion_count = 0",
        0, CSB_SAVE_IMPORT_ERR_NO_CHAMPIONS,
        "csb_v1_import_csb_save_buffer champCount range check"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_5,
        "CSB v2.0 with champion_count = 5 (out of range)",
        0, CSB_SAVE_IMPORT_ERR_NO_CHAMPIONS,
        "csb_v1_import_csb_save_buffer champCount range check"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_TRUNCATED_RECORDS,
        "CSB v2.0 with truncated records",
        0, CSB_SAVE_IMPORT_ERR_TRUNCATED,
        "csb_v1_import_csb_save_buffer need = HEADER + count*CHAMP_SIZE"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_CSBGAME_BAD_VERSION,
        "CSB v2.0 with version 0x055 (unsupported)",
        0, CSB_SAVE_IMPORT_ERR_VERSION,
        "csb_v1_import_csb_save_buffer version range check"),
    CONTRACT_ENTRY(
        CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_BAK_PAYLOAD,
        "CSB v2.0 .bak filename payload (loader sees the same bytes)",
        1, 0,
        "CSBWin CSBCode.cpp:422 csbgame.bak literal — .bak suffix is a launcher-side flag only")
};

/* ── Small classification helpers ────────────────────────────────────── */

static int ascii_tolower_int(int c)
{
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

static int ascii_ieq(const char *a, const char *b)
{
    if (!a || !b) return 0;
    while (*a && *b) {
        if (ascii_tolower_int((unsigned char)*a) !=
            ascii_tolower_int((unsigned char)*b)) {
            return 0;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

static const char *basename_hint(const char *path)
{
    const char *base = path;
    const char *p;
    if (!path) return "";
    for (p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
    }
    return base;
}

static unsigned int read_le32_at(const uint8_t *bytes, size_t size, size_t off)
{
    if (!bytes || size < off + 4u) return 0u;
    return (unsigned int)bytes[off]
         | ((unsigned int)bytes[off + 1u] << 8)
         | ((unsigned int)bytes[off + 2u] << 16)
         | ((unsigned int)bytes[off + 3u] << 24);
}

static uint32_t csb_v1_csbwin_save_fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes || size == 0u) return 0u;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static CSB_V1_CSBWinSaveShape classify_valid_xor512_shape(
    const uint8_t *bytes,
    size_t size,
    CSB_V1_CSBWin512Report *out_report)
{
    CSB_V1_CSBWin512Report local_report;
    CSB_V1_CSBWin512Report *report = out_report ? out_report : &local_report;
    int rc;

    if (!bytes || size < CSB_V1_CSBWIN_BLOCK1_BYTES) {
        return CSB_V1_CSBWIN_SHAPE_COUNT;
    }
    rc = csb_v1_csbwin_512_xor_pad_classify(bytes, size, report);
    if (rc != CSB_V1_CSBWIN_512_OK) {
        return CSB_V1_CSBWIN_SHAPE_COUNT;
    }
    if (report->verdict == CSB_V1_CSBWIN_512_VERDICT_CSB) {
        return CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1;
    }
    if (report->verdict == CSB_V1_CSBWIN_512_VERDICT_DM) {
        return CSB_V1_CSBWIN_SHAPE_CSBWIN_512_DM01;
    }
    return CSB_V1_CSBWIN_SHAPE_COUNT;
}

static CSB_V1_CSBWinSaveShape classify_bytes_shape(
    const uint8_t *bytes,
    size_t size)
{
    static const unsigned char kCsbGameMagic[CSB_SAVE_MAGIC_LEN] = {
        'C','S','B','G','A','M','E','\0'
    };
    unsigned int version;
    unsigned int champion_count;
    size_t need;

    if (!bytes) return CSB_V1_CSBWIN_SHAPE_COUNT;
    if (size < CSB_SAVE_MAGIC_LEN) {
        return CSB_V1_CSBWIN_SHAPE_TOO_SMALL_UNDER_8;
    }
    if (memcmp(bytes, "RDMCSB15", CSB_SAVE_MAGIC_LEN) == 0) {
        return CSB_V1_CSBWIN_SHAPE_DM1_RAW_RDMCSB15;
    }
    if (size >= 4u &&
        bytes[0] == (uint8_t)'C' &&
        bytes[1] == (uint8_t)'S' &&
        bytes[2] == (uint8_t)'B' &&
        bytes[3] == 0x01u) {
        return CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1;
    }
    if (size >= 4u &&
        bytes[0] == (uint8_t)'D' &&
        bytes[1] == (uint8_t)'M' &&
        bytes[2] == 0x00u &&
        bytes[3] == 0x01u) {
        return CSB_V1_CSBWIN_SHAPE_CSBWIN_512_DM01;
    }
    if (size >= 4u &&
        bytes[0] == (uint8_t)'C' &&
        bytes[1] == (uint8_t)'E' &&
        bytes[2] == (uint8_t)'D' &&
        bytes[3] == (uint8_t)'T') {
        return CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CEDT;
    }
    {
        CSB_V1_CSBWinSaveShape xor_shape =
            classify_valid_xor512_shape(bytes, size, NULL);
        if (xor_shape != CSB_V1_CSBWIN_SHAPE_COUNT) {
            return xor_shape;
        }
    }
    if (memcmp(bytes, kCsbGameMagic, CSB_SAVE_MAGIC_LEN) != 0) {
        return CSB_V1_CSBWIN_SHAPE_NO_MAGIC_8_PLUS;
    }
    if (size < (size_t)CSB_SAVE_HEADER_SIZE) {
        return CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_TRUNCATED_RECORDS;
    }

    /* CSBWin SaveGame.cpp writes an optional CDSA marker into the
     * CSBGAME header; today Firestaff records that shape as a
     * bounded loader rejection instead of attempting DSA decode. */
    if (size >= (size_t)(CSB_SAVE_HDR_OFF_CHAMP_COUNT + 4u) &&
        bytes[CSB_SAVE_HDR_OFF_CHAMP_COUNT] == (uint8_t)'C' &&
        bytes[CSB_SAVE_HDR_OFF_CHAMP_COUNT + 1u] == (uint8_t)'D' &&
        bytes[CSB_SAVE_HDR_OFF_CHAMP_COUNT + 2u] == (uint8_t)'S' &&
        bytes[CSB_SAVE_HDR_OFF_CHAMP_COUNT + 3u] == (uint8_t)'A') {
        return CSB_V1_CSBWIN_SHAPE_CSBGAME_CDSA;
    }

    version = read_le32_at(bytes, size, CSB_SAVE_HDR_OFF_VERSION);
    if (version != CSB_SAVE_VERSION_V20 && version != CSB_SAVE_VERSION_V21) {
        return CSB_V1_CSBWIN_SHAPE_CSBGAME_BAD_VERSION;
    }
    champion_count = (size > (size_t)CSB_SAVE_HDR_OFF_CHAMP_COUNT)
        ? (unsigned int)bytes[CSB_SAVE_HDR_OFF_CHAMP_COUNT]
        : 0u;
    if (champion_count == 0u) {
        return CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_0;
    }
    if (champion_count > (unsigned int)CSB_V1_MAX_CHAMPIONS) {
        return CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_5;
    }
    need = (size_t)CSB_SAVE_HEADER_SIZE
         + (size_t)champion_count * (size_t)CSB_SAVE_CHAMP_SIZE;
    if (size < need) {
        return CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_TRUNCATED_RECORDS;
    }
    return (version == CSB_SAVE_VERSION_V21)
        ? CSB_V1_CSBWIN_SHAPE_CSBGAME_V21
        : CSB_V1_CSBWIN_SHAPE_CSBGAME_V20;
}

static int is_csbwin_512_shape(CSB_V1_CSBWinSaveShape shape)
{
    return shape == CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1 ||
           shape == CSB_V1_CSBWIN_SHAPE_CSBWIN_512_DM01 ||
           shape == CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CEDT;
}

enum {
    CSB_V1_CSBWIN_SAVE_FILE_DEFAULT_MAX_BYTES = 4 * 1024 * 1024
};

const CSB_V1_CSBWinSaveShapeContract *
csb_v1_csbwin_save_loader_boundary_contract(size_t *out_count)
{
    if (out_count) {
        *out_count = (size_t)(sizeof(k_contract_table)
                               / sizeof(k_contract_table[0]));
    }
    return k_contract_table;
}

/* ── Public API ──────────────────────────────────────────────────────── */

int csb_v1_csbwin_save_loader_boundary_check(
    const uint8_t *bytes,
    size_t         size,
    CSB_V1_CSBWinSaveShape shape,
    CSB_V1_CSBWinLoaderBoundaryResult *out)
{
    const CSB_V1_CSBWinSaveShapeContract *table;
    size_t table_count = 0u;
    size_t i;
    const CSB_V1_CSBWinSaveShapeContract *row = NULL;
    int rc;
#ifdef CSB_V1_CSBWIN_SAVE_LOADER_BOUNDARY_CONTRACT_ONLY
    CSB_V1_PartyState party;
#endif

    if (!out) {
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    /* Zero the result so an early return still leaves a
     * deterministic, contract_match=0 record. */
    memset(out, 0, sizeof(*out));
    out->shape        = shape;
    out->shape_label  = csb_v1_csbwin_save_loader_boundary_shape_name(shape);
    out->loader_code  = CSB_SAVE_IMPORT_ERR_NULL;
    out->champion_count = 0;

    table = csb_v1_csbwin_save_loader_boundary_contract(&table_count);
    for (i = 0u; i < table_count; ++i) {
        if (table[i].shape == shape) {
            row = &table[i];
            break;
        }
    }
    if (!row) {
        /* Unknown shape — fail closed with a NULL-style verdict. */
        out->loader_code    = CSB_SAVE_IMPORT_ERR_NULL;
        out->expected_code  = CSB_SAVE_IMPORT_ERR_NULL;
        out->contract_match = 0;
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    out->expected_code = row->expect_code;

    if (!bytes || size == 0u) {
        /* No bytes supplied — record the loader result for an
         * empty buffer (NULL/0) and let the test verdict handle
         * whether this is a contract match. For most shapes the
         * loader returns ERR_NULL on NULL bytes and ERR_TRUNCATED
         * on zero-length input; both are deterministic and
         * covered by the test's per-shape expectations. */
#ifdef CSB_V1_CSBWIN_SAVE_LOADER_BOUNDARY_CONTRACT_ONLY
        if (!bytes) {
            rc = csb_v1_import_csb_save_buffer(&party, NULL, 0);
        } else {
            rc = csb_v1_import_csb_save_buffer(&party, bytes, 0);
        }
#else
        rc = !bytes ? CSB_SAVE_IMPORT_ERR_NULL : CSB_SAVE_IMPORT_ERR_TRUNCATED;
#endif
        out->loader_code    = rc;
        out->champion_count = (rc > 0) ? rc : 0;
        out->contract_match =
            (!row->expect_accept && rc == row->expect_code) ? 1 : 0;
        return rc;
    }
    /* The compact reader has no complete original GAMEBLOCK body. It is a
     * focused compatibility contract only; a product classifier must never
     * mistake its roster result for a resumable original save. */
#ifdef CSB_V1_CSBWIN_SAVE_LOADER_BOUNDARY_CONTRACT_ONLY
    rc = csb_v1_import_csb_save_buffer(&party, bytes, (long)size);
#else
    rc = CSB_SAVE_IMPORT_ERR_INCOMPLETE_BODY;
#endif
    out->loader_code    = rc;
    out->champion_count = (rc > 0) ? rc : 0;

    if (row->expect_accept) {
        /* Accept contract: the loader must return a positive
         * champion count. The exact value isn't pinned — it
         * depends on the fixture's champ_count. We require
         * loader_code > 0 (i.e. accept) and surface the actual
         * count for the test to compare against the fixture. */
        out->contract_match = (rc > 0) ? 1 : 0;
        /* expected_code is unused on accept-shapes (0). */
    } else {
        out->contract_match = (rc == row->expect_code) ? 1 : 0;
    }
    return rc;
}

CSB_V1_CSBWinSaveShape csb_v1_csbwin_save_loader_boundary_match(
    const uint8_t *bytes,
    size_t         size)
{
    /* Quick route: if the bytes don't start with CSBGAME\0 they
     * cannot match any accept-shape. The accept-shapes are
     * precisely CSBGAME_V20 and CSBGAME_V21. */
    static const unsigned char kAcceptMagic[CSB_SAVE_MAGIC_LEN] = {
        'C','S','B','G','A','M','E','\0'
    };
    if (!bytes || size < CSB_SAVE_HEADER_SIZE) {
        return CSB_V1_CSBWIN_SHAPE_COUNT;
    }
    if (memcmp(bytes, kAcceptMagic, CSB_SAVE_MAGIC_LEN) != 0) {
        return CSB_V1_CSBWIN_SHAPE_COUNT;
    }
    /* Distinguish v2.0 vs v2.1 by the version word at offset 8. */
    if (size < (size_t)(CSB_SAVE_HDR_OFF_VERSION + 4u)) {
        return CSB_V1_CSBWIN_SHAPE_COUNT;
    }
    {
        unsigned int v = (unsigned int)bytes[CSB_SAVE_HDR_OFF_VERSION]
                       | ((unsigned int)bytes[CSB_SAVE_HDR_OFF_VERSION + 1] << 8)
                       | ((unsigned int)bytes[CSB_SAVE_HDR_OFF_VERSION + 2] << 16)
                       | ((unsigned int)bytes[CSB_SAVE_HDR_OFF_VERSION + 3] << 24);
        if (v == CSB_SAVE_VERSION_V20) return CSB_V1_CSBWIN_SHAPE_CSBGAME_V20;
        if (v == CSB_SAVE_VERSION_V21) return CSB_V1_CSBWIN_SHAPE_CSBGAME_V21;
    }
    return CSB_V1_CSBWIN_SHAPE_COUNT;
}

CSB_V1_CSBWinSaveFileKind
csb_v1_csbwin_save_loader_boundary_file_kind(const char *path_hint)
{
    const char *base = basename_hint(path_hint);
    if (ascii_ieq(base, "csbgame.dat")) {
        return CSB_V1_CSBWIN_SAVE_FILE_CSBGAME_DAT;
    }
    if (ascii_ieq(base, "csbgame2.dat")) {
        return CSB_V1_CSBWIN_SAVE_FILE_CSBGAME2_DAT;
    }
    if (ascii_ieq(base, "csbgame3.dat")) {
        return CSB_V1_CSBWIN_SAVE_FILE_CSBGAME3_DAT;
    }
    if (ascii_ieq(base, "csbgame4.dat")) {
        return CSB_V1_CSBWIN_SAVE_FILE_CSBGAME4_DAT;
    }
    if (ascii_ieq(base, "csbgame.bak")) {
        return CSB_V1_CSBWIN_SAVE_FILE_CSBGAME_BAK;
    }
    if (ascii_ieq(base, "dmsave.dat")) {
        return CSB_V1_CSBWIN_SAVE_FILE_DMSAVE_DAT;
    }
    if (ascii_ieq(base, "dmsave.bak")) {
        return CSB_V1_CSBWIN_SAVE_FILE_DMSAVE_BAK;
    }
    return CSB_V1_CSBWIN_SAVE_FILE_NONE;
}

int csb_v1_csbwin_save_loader_boundary_classify(
    const char *path_hint,
    const uint8_t *bytes,
    size_t size,
    CSB_V1_CSBWinSaveDiscoveryResult *out)
{
    CSB_V1_CSBWinSaveShape shape;
    CSB_V1_CSBWinSaveFileKind kind;
    int rc;

    if (!out) {
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    memset(out, 0, sizeof(*out));

    kind = csb_v1_csbwin_save_loader_boundary_file_kind(path_hint);
    shape = classify_bytes_shape(bytes, size);
    if ((kind == CSB_V1_CSBWIN_SAVE_FILE_CSBGAME_BAK ||
         kind == CSB_V1_CSBWIN_SAVE_FILE_DMSAVE_BAK) &&
        shape == CSB_V1_CSBWIN_SHAPE_CSBGAME_V20) {
        shape = CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_BAK_PAYLOAD;
    }

    out->file_kind = kind;
    out->filename_candidate = (kind != CSB_V1_CSBWIN_SAVE_FILE_NONE) ? 1 : 0;
    out->shape = shape;
    out->file_kind_label =
        csb_v1_csbwin_save_loader_boundary_file_kind_name(kind);

    if (is_csbwin_512_shape(shape)) {
        CSB_V1_CSBWinSaveShape xor_shape =
            classify_valid_xor512_shape(bytes, size, &out->xor512_report);
        out->xor512_valid =
            (xor_shape != CSB_V1_CSBWIN_SHAPE_COUNT) ? 1 : 0;
        if (out->xor512_valid &&
            csb_v1_csbwin_512_verify_save_body(
                bytes, size, 0u, &out->xor512_body_report) ==
                    CSB_V1_CSBWIN_512_OK) {
            out->xor512_body_valid = 1;
        }
    }

    if (shape == CSB_V1_CSBWIN_SHAPE_COUNT) {
        out->loader.shape = shape;
        out->loader.shape_label =
            csb_v1_csbwin_save_loader_boundary_shape_name(shape);
        out->loader.loader_code = CSB_SAVE_IMPORT_ERR_NULL;
        out->loader.expected_code = CSB_SAVE_IMPORT_ERR_NULL;
        out->loader.contract_match = 0;
        out->should_attempt_import = 0;
        out->decision_label =
            csb_v1_csbwin_save_loader_boundary_decision_name(out);
        return CSB_SAVE_IMPORT_ERR_NULL;
    }

    rc = csb_v1_csbwin_save_loader_boundary_check(bytes, size, shape,
                                                  &out->loader);
    out->should_attempt_import =
        (out->filename_candidate &&
         out->loader.contract_match &&
         out->loader.loader_code > 0) ? 1 : 0;
    out->decision_label =
        csb_v1_csbwin_save_loader_boundary_decision_name(out);
    return rc;
}

int csb_v1_csbwin_save_loader_boundary_classify_file(
    const char *path,
    size_t max_size,
    CSB_V1_CSBWinSaveDiscoveryResult *out)
{
    FILE *fp;
    long file_size_long;
    size_t file_size;
    uint8_t *bytes;
    size_t got;
    int rc;

    if (!path || path[0] == '\0' || !out) {
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    memset(out, 0, sizeof(*out));
    out->file_kind = csb_v1_csbwin_save_loader_boundary_file_kind(path);
    out->filename_candidate =
        (out->file_kind != CSB_V1_CSBWIN_SAVE_FILE_NONE) ? 1 : 0;
    out->shape = CSB_V1_CSBWIN_SHAPE_COUNT;
    out->file_kind_label =
        csb_v1_csbwin_save_loader_boundary_file_kind_name(out->file_kind);
    out->decision_label =
        csb_v1_csbwin_save_loader_boundary_decision_name(out);

    fp = fopen(path, "rb");
    if (!fp) {
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    if (max_size == 0u) {
        max_size = (size_t)CSB_V1_CSBWIN_SAVE_FILE_DEFAULT_MAX_BYTES;
    }
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    file_size_long = ftell(fp);
    if (file_size_long < 0) {
        fclose(fp);
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    file_size = (size_t)file_size_long;
    if (file_size > max_size) {
        fclose(fp);
        return CSB_SAVE_IMPORT_ERR_TRUNCATED;
    }
    if (fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    bytes = (uint8_t *)malloc(file_size > 0u ? file_size : 1u);
    if (!bytes) {
        fclose(fp);
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    got = fread(bytes, 1u, file_size, fp);
    fclose(fp);
    if (got != file_size) {
        free(bytes);
        return CSB_SAVE_IMPORT_ERR_TRUNCATED;
    }

    rc = csb_v1_csbwin_save_loader_boundary_classify(path, bytes, file_size,
                                                     out);
    free(bytes);
    return rc;
}

static int read_entire_bounded_file(const char *path,
                                    size_t max_size,
                                    uint8_t **out_bytes,
                                    size_t *out_size)
{
    FILE *fp;
    long file_size_long;
    size_t file_size;
    uint8_t *bytes;
    size_t got;

    if (!path || !out_bytes || !out_size) {
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    *out_bytes = NULL;
    *out_size = 0u;
    fp = fopen(path, "rb");
    if (!fp) {
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    if (max_size == 0u) {
        max_size = (size_t)CSB_V1_CSBWIN_SAVE_FILE_DEFAULT_MAX_BYTES;
    }
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    file_size_long = ftell(fp);
    if (file_size_long < 0) {
        fclose(fp);
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    file_size = (size_t)file_size_long;
    if (file_size > max_size) {
        fclose(fp);
        return CSB_SAVE_IMPORT_ERR_TRUNCATED;
    }
    if (fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    bytes = (uint8_t *)malloc(file_size > 0u ? file_size : 1u);
    if (!bytes) {
        fclose(fp);
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    got = fread(bytes, 1u, file_size, fp);
    fclose(fp);
    if (got != file_size) {
        free(bytes);
        return CSB_SAVE_IMPORT_ERR_TRUNCATED;
    }
    *out_bytes = bytes;
    *out_size = file_size;
    return 0;
}

int csb_v1_csbwin_save_loader_boundary_dsa_corpus_receipt(
    const char *path_hint,
    const uint8_t *bytes,
    size_t size,
    CSB_V1_CSBWinDSASaveCorpusReceipt *out)
{
    size_t gameblock_offset;
    int rc;

    if (!out) {
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    memset(out, 0, sizeof(*out));
    out->valid = 1;
    out->file_kind = csb_v1_csbwin_save_loader_boundary_file_kind(path_hint);
    out->filename_candidate =
        (out->file_kind != CSB_V1_CSBWIN_SAVE_FILE_NONE) ? 1 : 0;
    out->file_kind_label =
        csb_v1_csbwin_save_loader_boundary_file_kind_name(out->file_kind);

    if (!bytes || size == 0u) {
        out->extended_result = CSB_V1_CSBWIN_EXTENDED_ERR_ARGUMENT;
        out->decision_label =
            csb_v1_csbwin_save_loader_boundary_dsa_corpus_decision_name(out);
        return 0;
    }
    out->save_bytes_fnv1a = csb_v1_csbwin_save_fnv1a32(bytes, size);
    if (!out->filename_candidate) {
        out->extended_result = CSB_V1_CSBWIN_EXTENDED_ABSENT;
        out->decision_label =
            csb_v1_csbwin_save_loader_boundary_dsa_corpus_decision_name(out);
        return 0;
    }

    rc = csb_v1_csbwin_512_inspect_extended_tail(bytes, size, &out->tail,
                                                  &out->dsa, &out->features);
    out->extended_result = rc;
    out->extended_tail_valid = (rc == CSB_V1_CSBWIN_EXTENDED_OK &&
                                out->tail.valid) ? 1 : 0;
    out->dsa_section_valid = (out->dsa.valid && out->dsa.dsa_count > 0u) ? 1 : 0;
    out->dsa_has_runtime_actions =
        (out->dsa_section_valid && out->dsa.action_count > 0u &&
         out->dsa.program_word_count > 0u) ? 1 : 0;
    if (!out->extended_tail_valid || !out->dsa_has_runtime_actions) {
        out->decision_label =
            csb_v1_csbwin_save_loader_boundary_dsa_corpus_decision_name(out);
        return 0;
    }

    gameblock_offset = out->tail.next_payload_offset;
    out->gameblock1_offset = gameblock_offset;
    if (gameblock_offset > size ||
        size - gameblock_offset < CSB_V1_CSBWIN_BLOCK1_BYTES) {
        out->decision_label =
            csb_v1_csbwin_save_loader_boundary_dsa_corpus_decision_name(out);
        return 0;
    }
    out->gameblock1_result = csb_v1_csbwin_512_xor_pad_classify(
        bytes + gameblock_offset, size - gameblock_offset, &out->gameblock1);
    out->gameblock1_valid =
        (out->gameblock1_result == CSB_V1_CSBWIN_512_OK &&
         (out->gameblock1.verdict == CSB_V1_CSBWIN_512_VERDICT_CSB ||
         out->gameblock1.verdict == CSB_V1_CSBWIN_512_VERDICT_DM)) ? 1 : 0;

    if (!out->gameblock1_valid) {
        out->decision_label =
            csb_v1_csbwin_save_loader_boundary_dsa_corpus_decision_name(out);
        return 0;
    }
    out->gameblock1_body_result = csb_v1_csbwin_512_verify_save_body(
        bytes + gameblock_offset, size - gameblock_offset, 0u,
        &out->gameblock1_body);
    out->gameblock1_body_valid =
        (out->gameblock1_body_result == CSB_V1_CSBWIN_512_OK &&
         out->gameblock1_body.header_valid) ? 1 : 0;
    if (out->gameblock1_body_valid) {
        out->gameblock1_body_fnv1a = csb_v1_csbwin_save_fnv1a32(
            bytes + gameblock_offset, size - gameblock_offset);
    }

    out->corpus_positive = out->gameblock1_valid &&
        out->gameblock1_body_valid && out->save_bytes_fnv1a != 0u &&
        out->gameblock1_body_fnv1a != 0u;
    out->runtime_handoff_ready = out->corpus_positive;
    out->decision_label =
        csb_v1_csbwin_save_loader_boundary_dsa_corpus_decision_name(out);
    return out->corpus_positive;
}

int csb_v1_csbwin_save_loader_boundary_dsa_corpus_receipt_file(
    const char *path,
    size_t max_size,
    CSB_V1_CSBWinDSASaveCorpusReceipt *out)
{
    uint8_t *bytes = NULL;
    size_t size = 0u;
    int rc;

    if (!path || !out) {
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    rc = read_entire_bounded_file(path, max_size, &bytes, &size);
    if (rc != 0) {
        memset(out, 0, sizeof(*out));
        out->valid = 1;
        out->file_kind = csb_v1_csbwin_save_loader_boundary_file_kind(path);
        out->filename_candidate =
            (out->file_kind != CSB_V1_CSBWIN_SAVE_FILE_NONE) ? 1 : 0;
        out->file_kind_label =
            csb_v1_csbwin_save_loader_boundary_file_kind_name(out->file_kind);
        out->decision_label =
            csb_v1_csbwin_save_loader_boundary_dsa_corpus_decision_name(out);
        return rc;
    }
    rc = csb_v1_csbwin_save_loader_boundary_dsa_corpus_receipt(
        path, bytes, size, out);
    free(bytes);
    return rc;
}

const char *csb_v1_csbwin_save_loader_boundary_shape_name(
    CSB_V1_CSBWinSaveShape shape)
{
    switch (shape) {
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20:                 return "csbgame_v20";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V21:                 return "csbgame_v21";
    case CSB_V1_CSBWIN_SHAPE_DM1_RAW_RDMCSB15:            return "dm1_raw_rdmcsb15";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_CDSA:                return "csbgame_cdsa";
    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1:             return "csbwin_512_csb1";
    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_DM01:             return "csbwin_512_dm01";
    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CEDT:             return "csbwin_512_cedt";
    case CSB_V1_CSBWIN_SHAPE_TOO_SMALL_UNDER_8:           return "too_small_under_8";
    case CSB_V1_CSBWIN_SHAPE_NO_MAGIC_8_PLUS:             return "no_magic_8_plus";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_0:   return "csbgame_v20_champ_count_0";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_5:   return "csbgame_v20_champ_count_5";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_TRUNCATED_RECORDS: return "csbgame_v20_truncated_records";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_BAD_VERSION:         return "csbgame_bad_version";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_BAK_PAYLOAD:     return "csbgame_v20_bak_payload";
    case CSB_V1_CSBWIN_SHAPE_COUNT:                       return "shape_count";
    default:                                              return "unknown";
    }
}

const char *csb_v1_csbwin_save_loader_boundary_file_kind_name(
    CSB_V1_CSBWinSaveFileKind kind)
{
    switch (kind) {
    case CSB_V1_CSBWIN_SAVE_FILE_CSBGAME_DAT: return "csbgame.dat";
    case CSB_V1_CSBWIN_SAVE_FILE_CSBGAME2_DAT: return "csbgame2.dat";
    case CSB_V1_CSBWIN_SAVE_FILE_CSBGAME3_DAT: return "csbgame3.dat";
    case CSB_V1_CSBWIN_SAVE_FILE_CSBGAME4_DAT: return "csbgame4.dat";
    case CSB_V1_CSBWIN_SAVE_FILE_CSBGAME_BAK: return "csbgame.bak";
    case CSB_V1_CSBWIN_SAVE_FILE_DMSAVE_DAT:  return "dmsave.dat";
    case CSB_V1_CSBWIN_SAVE_FILE_DMSAVE_BAK:  return "dmsave.bak";
    case CSB_V1_CSBWIN_SAVE_FILE_NONE:
    default:                                  return "none";
    }
}

const char *csb_v1_csbwin_save_loader_boundary_decision_name(
    const CSB_V1_CSBWinSaveDiscoveryResult *result)
{
    if (!result) return "reject_null_result";
    if (!result->filename_candidate) {
        return "reject_non_csbwin_save_filename";
    }
    if (result->should_attempt_import) {
        return "accept_loader_ready";
    }
    if (is_csbwin_512_shape(result->shape) && result->xor512_body_valid) {
        return "accept_csbwin_512_runtime_handoff_ready";
    }
    if (is_csbwin_512_shape(result->shape) && result->xor512_valid) {
        return "reject_csbwin_512_header_valid_import_pending";
    }
    switch (result->shape) {
    case CSB_V1_CSBWIN_SHAPE_DM1_RAW_RDMCSB15:
        return "reject_dm1_raw_needs_conversion";
    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1:
    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_DM01:
    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CEDT:
        return "reject_csbwin_512_needs_decoder";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_CDSA:
        return "reject_cdsa_needs_dsa_section_decoder";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_0:
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_5:
        return "reject_champion_count";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_TRUNCATED_RECORDS:
    case CSB_V1_CSBWIN_SHAPE_TOO_SMALL_UNDER_8:
        return "reject_truncated";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_BAD_VERSION:
        return "reject_bad_version";
    case CSB_V1_CSBWIN_SHAPE_NO_MAGIC_8_PLUS:
        return "reject_bad_magic";
    case CSB_V1_CSBWIN_SHAPE_COUNT:
        return "reject_no_bytes";
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20:
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V21:
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_BAK_PAYLOAD:
    default:
        return "reject_loader_contract_mismatch";
    }
}

const char *csb_v1_csbwin_save_loader_boundary_dsa_corpus_decision_name(
    const CSB_V1_CSBWinDSASaveCorpusReceipt *receipt)
{
    if (!receipt || !receipt->valid) {
        return "reject_null_dsa_corpus_receipt";
    }
    if (!receipt->filename_candidate) {
        return "reject_dsa_corpus_non_csbwin_filename";
    }
    if (receipt->extended_result == CSB_V1_CSBWIN_EXTENDED_ABSENT) {
        return "reject_dsa_corpus_no_extended_features";
    }
    if (!receipt->extended_tail_valid) {
        return "reject_dsa_corpus_extended_tail_invalid";
    }
    if (!receipt->dsa_section_valid) {
        return "reject_dsa_corpus_no_dsa_section";
    }
    if (!receipt->dsa_has_runtime_actions) {
        return "reject_dsa_corpus_no_runtime_actions";
    }
    if (!receipt->gameblock1_valid) {
        return "reject_dsa_corpus_missing_valid_gameblock1";
    }
    if (!receipt->gameblock1_body_valid) {
        return "reject_dsa_corpus_missing_verified_gameblock_body";
    }
    if (receipt->corpus_positive && receipt->runtime_handoff_ready) {
        return "accept_dsa_save_runtime_corpus";
    }
    return "reject_dsa_corpus_unready";
}

size_t csb_v1_csbwin_save_loader_boundary_accept_count(void)
{
    size_t total = 0u;
    size_t n = 0u;
    const CSB_V1_CSBWinSaveShapeContract *t =
        csb_v1_csbwin_save_loader_boundary_contract(&n);
    size_t i;
    for (i = 0u; i < n; ++i) {
        if (t[i].expect_accept) ++total;
    }
    return total;
}

size_t csb_v1_csbwin_save_loader_boundary_reject_count(void)
{
    size_t total = 0u;
    size_t n = 0u;
    const CSB_V1_CSBWinSaveShapeContract *t =
        csb_v1_csbwin_save_loader_boundary_contract(&n);
    size_t i;
    for (i = 0u; i < n; ++i) {
        if (!t[i].expect_accept) ++total;
    }
    return total;
}

const char *csb_v1_csbwin_save_loader_boundary_source_evidence(void)
{
    return
        "ReDMCSB CEDTINC8.C:101-118 (DMSAVE / CSBGAME.DAT routing)\n"
        "ReDMCSB LOADSAVE.C F0433/F0435 (CSBGAME namespace + C29 key)\n"
        "ReDMCSB SAVEHEAD.C F0429/F0430 (header read/write)\n"
        "ReDMCSB DEFS.H:1289 (CSBGAME.DAT magic literal)\n"
        "CSBWin SaveGame.cpp:927/1711/2111 (save file I/O + 512-byte XOR header)\n"
        "CSBWin SaveGame.cpp:1768-1855 (GAMEBLOCK2/ITEM16/characters/timers load order)\n"
        "CSBWin SaveGame.cpp:211-260/298-385 (Extended Features DSA + tail corpus gate)\n"
        "CSBWin CSBCode.cpp:9061-9069 (UnscrambleStream checksum gate)\n"
        "CSBWin CSBCode.cpp:421-422 (csbgame.dat / csbgame.bak literals)\n"
        "CSBWin CSBCode.cpp:9813 (SaveGameFilename pointer)\n"
        "CSBWin Data.h:590 (SaveGameFilename field)\n"
        "include/memory_savegame_pc34_compat.h:227 (RDMCSB15 magic)\n"
        "include/csb_v1_save_import_path_pc34_compat.h (CSB_V1_PartyState + CSB_SaveImportResult)\n"
        "docs/FIRESTAFF_GAP_LIST.md row CSBWin custom resource handling (csbgraphics.dat + dmsave + csbgame)";
}
