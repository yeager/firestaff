/*
 * csb_v1_csbwin_save_loader_boundary_pc34_compat.c
 *
 * CSB V1 CSBWin save-side loader-boundary evidence gate.
 *
 * Implementation note: this module is intentionally thin. It
 * builds synthetic byte buffers for the documented CSBWin / DM1
 * save shapes, feeds them into the existing
 * csb_v1_import_csb_save_buffer() entry point, and records the
 * actual loader result against the documented contract. No new
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
 *     offsets used to build the synthetic fixtures below).
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

/* ── Synthetic fixture builders ──────────────────────────────────────── */

/* Build a CSB v2.0 or v2.1 save buffer with `champ_count`
 * 160-byte champion records following the documented header.
 * `buf` must be at least CSB_SAVE_HEADER_SIZE +
 * champ_count*CSB_SAVE_CHAMP_SIZE bytes. The returned size is
 * the total bytes written.
 *
 * Notes:
 *   - version_word is 0x200 or 0x201 (little-endian at offset 8).
 *   - each champion record is zero-filled except the name at
 *     offset 0, which we set to the literal "CHAMP_<i>" so an
 *     import test can verify the round-trip.
 *   - the CDSA-marker shape overrides bytes [12..15] after
 *     building; we keep that mutation in a separate helper.
 */
static size_t build_csbgame_fixture(unsigned char *buf,
                                    size_t buf_capacity,
                                    unsigned int version_word,
                                    unsigned int champ_count,
                                    int write_dsa_marker)
{
    const size_t header = CSB_SAVE_HEADER_SIZE;
    const size_t record = CSB_SAVE_CHAMP_SIZE;
    const size_t total  = header + (size_t)champ_count * record;
    size_t i;
    if (total > buf_capacity) return 0u;
    if (champ_count > CSB_V1_MAX_CHAMPIONS) return 0u;

    memset(buf, 0, total);
    memcpy(buf + CSB_SAVE_HDR_OFF_MAGIC, "CSBGAME\0", CSB_SAVE_MAGIC_LEN);
    buf[CSB_SAVE_HDR_OFF_VERSION]     = (unsigned char)(version_word & 0xFFu);
    buf[CSB_SAVE_HDR_OFF_VERSION + 1] = (unsigned char)((version_word >> 8) & 0xFFu);
    buf[CSB_SAVE_HDR_OFF_VERSION + 2] = (unsigned char)((version_word >> 16) & 0xFFu);
    buf[CSB_SAVE_HDR_OFF_VERSION + 3] = (unsigned char)((version_word >> 24) & 0xFFu);
    buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = (unsigned char)champ_count;
    buf[CSB_SAVE_HDR_OFF_GAME_ID]     = 0u;
    buf[CSB_SAVE_HDR_OFF_GAME_ID + 1] = 0u;
    buf[CSB_SAVE_HDR_OFF_GAME_ID + 2] = 0u;
    buf[CSB_SAVE_HDR_OFF_GAME_ID + 3] = 0u;

    for (i = 0u; i < champ_count; ++i) {
        unsigned char *rec = buf + header + (size_t)i * record;
        const char *name;
        size_t name_len;
        size_t j;

        /* Write a recognisable name into the 16-byte name field
         * at offset 0. "CHAMP_<i>" — up to 9 chars. */
        name = "CHAMP_";
        name_len = 6u;
        memcpy(rec + CSB_SAVE_CH_OFF_NAME, name, name_len);
        rec[CSB_SAVE_CH_OFF_NAME + name_len] = (unsigned char)('0' + (int)i);
        /* remaining 9 bytes are already zero from the memset. */

        /* Set HP = 100 so a successful import surfaces a real
         * MaximumHealth > 0 in the imported party. */
        rec[CSB_SAVE_CH_OFF_CUR_HP]     = 0x64;
        rec[CSB_SAVE_CH_OFF_CUR_HP + 1] = 0x00;
        rec[CSB_SAVE_CH_OFF_MAX_HP]     = 0x64;
        rec[CSB_SAVE_CH_OFF_MAX_HP + 1] = 0x00;

        /* Per-stat minimum is 30 per the importer convention; set
         * every stat's cur/max row to 60 so a stat parity check
         * finds non-zero, non-trivial values. */
        for (j = 0u; j < (size_t)CSB_V1_STAT_COUNT; ++j) {
            int16_t v = (int16_t)60;
            unsigned char *cur = rec + CSB_SAVE_CH_OFF_STAT_CUR + j * 2u;
            unsigned char *max = rec + CSB_SAVE_CH_OFF_STAT_MAX + j * 2u;
            cur[0] = (unsigned char)(v & 0xFFu);
            cur[1] = (unsigned char)((v >> 8) & 0xFFu);
            max[0] = (unsigned char)(v & 0xFFu);
            max[1] = (unsigned char)((v >> 8) & 0xFFu);
        }
    }

    if (write_dsa_marker) {
        /* CSBWin emits a "CDSA" marker at offset 12 inside the
         * 256-byte header when a save has an attached DSA
         * section. The marker overwrites whatever was at bytes
         * 12..15 of the header — in our fixture the byte at
         * offset 12 is currently CSB_SAVE_HDR_OFF_CHAMP_COUNT.
         * We keep champ_count = 1 here so the resulting fixture
         * still has a parseable version word; the loader's
         * version check fires before the champ-count check, so
         * the CDSA marker shape is rejected as VERSION. */
        buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT    ] = (unsigned char)'C';
        buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT + 1] = (unsigned char)'D';
        buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT + 2] = (unsigned char)'S';
        buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT + 3] = (unsigned char)'A';
    }

    return total;
}

/* Build a synthetic byte buffer for the given shape. Writes into
 * `buf` (which must have `buf_capacity` bytes available) and
 * returns the actual fixture size, or 0 if the shape requires
 * more than buf_capacity bytes. */
size_t csb_v1_csbwin_save_loader_boundary_build_fixture(
    CSB_V1_CSBWinSaveShape shape,
    uint8_t *buf,
    size_t buf_capacity)
{
    switch (shape) {
    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20:
        return build_csbgame_fixture(buf, buf_capacity,
                                     CSB_SAVE_VERSION_V20, 1u, 0);

    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V21:
        return build_csbgame_fixture(buf, buf_capacity,
                                     CSB_SAVE_VERSION_V21, 1u, 0);

    case CSB_V1_CSBWIN_SHAPE_DM1_RAW_RDMCSB15: {
        size_t need = (size_t)CSB_SAVE_HEADER_SIZE;
        if (need > buf_capacity) return 0u;
        memset(buf, 0, need);
        memcpy(buf, "RDMCSB15", 8);
        return need;
    }

    case CSB_V1_CSBWIN_SHAPE_CSBGAME_CDSA:
        return build_csbgame_fixture(buf, buf_capacity,
                                     CSB_SAVE_VERSION_V20, 1u, 1);

    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1: {
        /* "CSB\1" at offset 0 as a little-endian uint32. */
        size_t need = 512u;
        if (need > buf_capacity) return 0u;
        memset(buf, 0, need);
        buf[0] = (unsigned char)'C';
        buf[1] = (unsigned char)'S';
        buf[2] = (unsigned char)'B';
        buf[3] = 0x01u;
        return need;
    }

    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_DM01: {
        size_t need = 512u;
        if (need > buf_capacity) return 0u;
        memset(buf, 0, need);
        buf[0] = (unsigned char)'D';
        buf[1] = (unsigned char)'M';
        buf[2] = 0x00u;
        buf[3] = 0x01u;
        return need;
    }

    case CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CEDT: {
        size_t need = 512u;
        if (need > buf_capacity) return 0u;
        memset(buf, 0, need);
        buf[0] = (unsigned char)'C';
        buf[1] = (unsigned char)'E';
        buf[2] = (unsigned char)'D';
        buf[3] = (unsigned char)'T';
        return need;
    }

    case CSB_V1_CSBWIN_SHAPE_TOO_SMALL_UNDER_8: {
        size_t need = 4u;  /* 4 bytes < 8-byte magic, < 256-byte header */
        if (need > buf_capacity) return 0u;
        memset(buf, 0xA5u, need);
        return need;
    }

    case CSB_V1_CSBWIN_SHAPE_NO_MAGIC_8_PLUS: {
        size_t need = (size_t)CSB_SAVE_HEADER_SIZE;
        if (need > buf_capacity) return 0u;
        memset(buf, 0x7Eu, need);  /* 0x7E7E... is not CSBGAME/RDMCSB */
        return need;
    }

    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_0:
        return build_csbgame_fixture(buf, buf_capacity,
                                     CSB_SAVE_VERSION_V20, 0u, 0);

    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_5: {
        /* Champion count > CSB_V1_MAX_CHAMPIONS (4). The loader
         * bounds-checks before reading the record area, so we
         * only need to write a 256-byte header with champ_count
         * set to 5 — the loader's count check fires first and
         * returns ERR_NO_CHAMPIONS. */
        if (CSB_SAVE_HEADER_SIZE > buf_capacity) return 0u;
        memset(buf, 0, CSB_SAVE_HEADER_SIZE);
        memcpy(buf + CSB_SAVE_HDR_OFF_MAGIC, "CSBGAME\0", 8);
        buf[CSB_SAVE_HDR_OFF_VERSION]     = 0x00u;
        buf[CSB_SAVE_HDR_OFF_VERSION + 1] = 0x02u;
        buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = 5u;
        return CSB_SAVE_HEADER_SIZE;
    }

    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_TRUNCATED_RECORDS: {
        /* Header claims 4 champions but the buffer only carries
         * the 256-byte header (no records). The loader reads
         * champ_count = 4, then computes need = HEADER + 4*160 =
         * 896, then sees that len (256) < need (896) and returns
         * ERR_TRUNCATED. */
        if (CSB_SAVE_HEADER_SIZE > buf_capacity) return 0u;
        memset(buf, 0, CSB_SAVE_HEADER_SIZE);
        memcpy(buf + CSB_SAVE_HDR_OFF_MAGIC, "CSBGAME\0", 8);
        buf[CSB_SAVE_HDR_OFF_VERSION]     = 0x00u;
        buf[CSB_SAVE_HDR_OFF_VERSION + 1] = 0x02u;
        buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = 4u;
        return CSB_SAVE_HEADER_SIZE;
    }

    case CSB_V1_CSBWIN_SHAPE_CSBGAME_BAD_VERSION:
        /* 0x055 is not 0x200/0x201 — set the version word
         * directly so we exercise the version-range rejection
         * path without bumping any other field. */
        if (CSB_SAVE_HEADER_SIZE > buf_capacity) return 0u;
        memset(buf, 0, CSB_SAVE_HEADER_SIZE);
        memcpy(buf + CSB_SAVE_HDR_OFF_MAGIC, "CSBGAME\0", CSB_SAVE_MAGIC_LEN);
        buf[CSB_SAVE_HDR_OFF_VERSION]     = 0x55u;
        buf[CSB_SAVE_HDR_OFF_VERSION + 1] = 0x00u;
        buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = 1u;
        return CSB_SAVE_HEADER_SIZE;

    case CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_BAK_PAYLOAD:
        /* Same bytes as CSB_V1_CSBWIN_SHAPE_CSBGAME_V20 — the
         * launcher-side .bak detection lives in the on-disk
         * shape classifier sibling module; the loader itself
         * ignores the filename and reads the same bytes. */
        return build_csbgame_fixture(buf, buf_capacity,
                                     CSB_SAVE_VERSION_V20, 1u, 0);

    case CSB_V1_CSBWIN_SHAPE_COUNT:
    default:
        return 0u;
    }
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
    CSB_V1_PartyState party;
    int rc;

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
        if (!bytes) {
            rc = csb_v1_import_csb_save_buffer(&party, NULL, 0);
        } else {
            rc = csb_v1_import_csb_save_buffer(&party, bytes, 0);
        }
        out->loader_code    = rc;
        out->champion_count = (rc > 0) ? rc : 0;
        out->contract_match =
            (!row->expect_accept && rc == row->expect_code) ? 1 : 0;
        return rc;
    }

    /* Normal path: feed the bytes through the existing loader. */
    rc = csb_v1_import_csb_save_buffer(&party, bytes, (long)size);
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

int csb_v1_csbwin_save_loader_boundary_check_shape(
    CSB_V1_CSBWinSaveShape shape,
    CSB_V1_CSBWinLoaderBoundaryResult *out)
{
    /* Local scratch large enough for every fixture the contract
     * table can request (the CSBWin 512-byte shapes are the
     * largest). */
    uint8_t scratch[CSB_SAVE_HEADER_SIZE + 4u * CSB_SAVE_CHAMP_SIZE + 16u];
    size_t fixture_size;

    if (!out) {
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    fixture_size = csb_v1_csbwin_save_loader_boundary_build_fixture(
        shape, scratch, sizeof(scratch));
    if (fixture_size == 0u) {
        memset(out, 0, sizeof(*out));
        out->shape        = shape;
        out->shape_label  = csb_v1_csbwin_save_loader_boundary_shape_name(shape);
        out->loader_code  = CSB_SAVE_IMPORT_ERR_NULL;
        out->expected_code = CSB_SAVE_IMPORT_ERR_NULL;
        out->contract_match = 0;
        return CSB_SAVE_IMPORT_ERR_NULL;
    }
    return csb_v1_csbwin_save_loader_boundary_check(
        scratch, fixture_size, shape, out);
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
        "CSBWin CSBCode.cpp:9061-9069 (UnscrambleStream checksum gate)\n"
        "CSBWin CSBCode.cpp:421-422 (csbgame.dat / csbgame.bak literals)\n"
        "CSBWin CSBCode.cpp:9813 (SaveGameFilename pointer)\n"
        "CSBWin Data.h:590 (SaveGameFilename field)\n"
        "include/memory_savegame_pc34_compat.h:227 (RDMCSB15 magic)\n"
        "include/csb_v1_save_import_path_pc34_compat.h (CSB_V1_PartyState + CSB_SaveImportResult)\n"
        "docs/FIRESTAFF_GAP_LIST.md row CSBWin custom resource handling (csbgraphics.dat + dmsave + csbgame)";
}
