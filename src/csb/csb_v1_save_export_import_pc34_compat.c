/*
 * csb_v1_save_export_import_pc34_compat.c
 *
 * CSB V1 per-game save-byte export/import compatibility proof.
 *
 * This module is intentionally thin. It does not introduce a new
 * CSB parser. The whole point of the gate is to prove that a CSB
 * save-byte buffer produced by the existing
 * csb_v1_build_csb_save_buffer() can be:
 *
 *   1. Wrapped in a Firestaff-versioned envelope (FSSB).
 *   2. Round-tripped through the EXISTING
 *      csb_v1_import_csb_save_buffer() loader (so the export and
 *      import both go through the production code path).
 *   3. Distinguished from raw CSBGAME v2.0 / v2.1, DM1 RDMCSB15,
 *      and CSBWin 512-byte buffers (so the launcher does not
 *      double-wrap a raw CSB save).
 *
 * Source-lock boundary (see header for the full chain):
 *   - ReDMCSB CEDTINC8.C:101-118 (CSBGAME routing)
 *   - ReDMCSB LOADSAVE.C F0433/F0435 (CSBGAME namespace)
 *   - ReDMCSB SAVEHEAD.C F0429/F0430 (header read/write)
 *   - ReDMCSB DEFS.H:1289 (CSBGAME.DAT magic literal)
 *   - CSBWin SaveGame.cpp:927/1711/2111 (save file I/O)
 *   - CSBWin CSBCode.cpp:421-422 (csbgame.dat / csbgame.bak)
 *   - include/csb_v1_save_import_path_pc34_compat.h
 *     (CSB_V1_PartyState + CSB_SaveImportResult + builder)
 */

#include "csb_v1_save_export_import_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Internal: little-endian byte writers/parsers ──────────────────── */

static void wr_u16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

static void wr_u32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static uint16_t rd_u16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t rd_u32(const uint8_t *p) {
    return  (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

/* ── CRC-32 / ISO 3309 (poly 0xEDB88320, reflected) ───────────────── */

/* Standard reflected CRC-32 used by PNG / gzip / zlib. We
 * compute it table-free (bit-by-bit) because the gate runs on
 * ≤ 4 KB payloads and we don't want to depend on zlib being
 * available at build time. */
uint32_t csb_v1_save_export_crc32(const uint8_t *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFFu;
    size_t i;
    if (!data) return 0u;
    for (i = 0u; i < len; ++i) {
        crc ^= (uint32_t)data[i];
        {
            int b;
            for (b = 0; b < 8; ++b) {
                uint32_t mask = (uint32_t)-(int32_t)(crc & 1u);
                crc = (crc >> 1) ^ (0xEDB88320u & mask);
            }
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

/* ── Classification API ────────────────────────────────────────────── */

CSB_V1_SaveExportKind csb_v1_save_export_classify(const uint8_t *bytes,
                                                   size_t size)
{
    /* FSSB envelope: 4-byte magic "FSSB" at offset 0. */
    if (bytes && size >= CSB_V1_SAVE_EXPORT_HEADER_LEN) {
        if (memcmp(bytes, CSB_V1_SAVE_EXPORT_MAGIC,
                   CSB_V1_SAVE_EXPORT_MAGIC_LEN) == 0) {
            return CSB_V1_SAVE_EXPORT_KIND_FSSB_ENVELOPE;
        }
    }
    /* Raw CSBGAME v2.0 / v2.1 — 8-byte magic "CSBGAME\0" + LE u32
     * version. We do NOT accept CSBWin 512-byte shapes here
     * because their first 8 bytes do not match "CSBGAME\0". */
    if (bytes && size >= 12u) {
        if (memcmp(bytes, "CSBGAME\0", 8u) == 0) {
            uint32_t v = rd_u32(bytes + 8u);
            if (v == CSB_SAVE_VERSION_V20) {
                return CSB_V1_SAVE_EXPORT_KIND_RAW_CSBGAME_V20;
            }
            if (v == CSB_SAVE_VERSION_V21) {
                return CSB_V1_SAVE_EXPORT_KIND_RAW_CSBGAME_V21;
            }
        }
    }
    /* DM1 RDMCSB15 — magic "RDMCSB15" at offset 0. */
    if (bytes && size >= 8u) {
        if (memcmp(bytes, "RDMCSB15", 8u) == 0) {
            return CSB_V1_SAVE_EXPORT_KIND_RAW_DM1_RDMCSB;
        }
    }
    /* CSBWin 512-byte CSB\1 / DM\0\1 / CEDT — different magic
     * words at offset 0. We deliberately keep this group as a
     * single kind; the existing csb_v1_csbwin_save_loader_
     * boundary gate owns the per-shape verdict. */
    if (bytes && size >= 4u) {
        if (memcmp(bytes, "CSB\x01", 4u) == 0 ||
            memcmp(bytes, "DM\x00\x01", 4u) == 0 ||
            memcmp(bytes, "CEDT", 4u) == 0) {
            return CSB_V1_SAVE_EXPORT_KIND_RAW_CSBWIN_512;
        }
    }
    return CSB_V1_SAVE_EXPORT_KIND_UNKNOWN;
}

const char *csb_v1_save_export_kind_name(CSB_V1_SaveExportKind kind)
{
    switch (kind) {
    case CSB_V1_SAVE_EXPORT_KIND_FSSB_ENVELOPE:   return "fssb_envelope";
    case CSB_V1_SAVE_EXPORT_KIND_RAW_CSBGAME_V20: return "raw_csbgame_v20";
    case CSB_V1_SAVE_EXPORT_KIND_RAW_CSBGAME_V21: return "raw_csbgame_v21";
    case CSB_V1_SAVE_EXPORT_KIND_RAW_DM1_RDMCSB:  return "raw_dm1_rdmcsb";
    case CSB_V1_SAVE_EXPORT_KIND_RAW_CSBWIN_512:  return "raw_csbwin_512";
    case CSB_V1_SAVE_EXPORT_KIND_UNKNOWN:         return "unknown";
    default:                                      return "unknown";
    }
}

/* ── Build / size / parse ─────────────────────────────────────────── */

size_t csb_v1_save_export_envelope_size(size_t payload_len)
{
    if (payload_len == 0u) return 0u;
    if (payload_len > CSB_V1_SAVE_EXPORT_MAX_PAYLOAD) return 0u;
    return CSB_V1_SAVE_EXPORT_HEADER_LEN + payload_len;
}

long csb_v1_save_export_build_envelope(const uint8_t *payload,
                                        size_t payload_len,
                                        uint16_t payload_kind,
                                        const char *source_path,
                                        uint8_t *out,
                                        size_t out_capacity)
{
    size_t total;

    if (!payload || !out) return CSB_V1_SAVE_EXPORT_ERR_NULL;
    if (payload_len == 0u) return CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL;
    if (payload_len > CSB_V1_SAVE_EXPORT_MAX_PAYLOAD)
        return CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL;
    if (payload_kind != 0u && payload_kind != 1u)
        return CSB_V1_SAVE_EXPORT_ERR_BAD_PAYLOAD_KIND;

    total = CSB_V1_SAVE_EXPORT_HEADER_LEN + payload_len;
    if (out_capacity < total) return CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL;

    /* Header is zeroed first so reserved bytes are deterministic. */
    memset(out, 0, CSB_V1_SAVE_EXPORT_HEADER_LEN);

    /* Magic. */
    memcpy(out, CSB_V1_SAVE_EXPORT_MAGIC, CSB_V1_SAVE_EXPORT_MAGIC_LEN);
    /* [4..7]   manifest_version */
    wr_u32(out + 4u,  CSB_V1_SAVE_EXPORT_MANIFEST_VERSION);
    /* [8..11]  game_id */
    wr_u32(out + 8u,  CSB_V1_SAVE_EXPORT_GAME_ID);
    /* [12..13] payload_kind */
    wr_u16(out + 12u, payload_kind);
    /* [14..15] reserved — kept zero. */
    /* [16..19] payload_len */
    wr_u32(out + 16u, (uint32_t)payload_len);
    /* [20..23] payload_crc */
    wr_u32(out + 20u, csb_v1_save_export_crc32(payload, payload_len));

    /* [24..87] source_path, NUL-padded (no terminator when full). */
    {
        const char *src = source_path ? source_path : "(synthetic)";
        size_t copy_len = strlen(src);
        if (copy_len >= CSB_V1_SAVE_EXPORT_SOURCE_PATH_LEN) {
            copy_len = CSB_V1_SAVE_EXPORT_SOURCE_PATH_LEN - 1u;
        }
        memcpy(out + 24u, src, copy_len);
        /* Last byte of source_path is left zero for NUL termination
         * when truncated, so the field reads as a C string for any
         * copy_len < CSB_V1_SAVE_EXPORT_SOURCE_PATH_LEN. */
        out[24u + copy_len] = '\0';
    }

    /* Payload follows the header. */
    memcpy(out + CSB_V1_SAVE_EXPORT_HEADER_LEN, payload, payload_len);
    return (long)total;
}

int csb_v1_save_export_parse_header(const uint8_t *raw,
                                     size_t raw_size,
                                     CSB_V1_SaveExportHeader *out)
{
    if (!raw || !out) return CSB_V1_SAVE_EXPORT_ERR_NULL;
    if (raw_size < CSB_V1_SAVE_EXPORT_HEADER_LEN)
        return CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL;

    if (memcmp(raw, CSB_V1_SAVE_EXPORT_MAGIC,
               CSB_V1_SAVE_EXPORT_MAGIC_LEN) != 0) {
        return CSB_V1_SAVE_EXPORT_ERR_BAD_VERSION;
    }
    memset(out, 0, sizeof(*out));
    out->manifest_version = rd_u32(raw + 4u);
    out->game_id          = rd_u32(raw + 8u);
    out->payload_kind     = rd_u16(raw + 12u);
    out->reserved         = rd_u16(raw + 14u);
    out->payload_len      = rd_u32(raw + 16u);
    out->payload_crc      = rd_u32(raw + 20u);
    memcpy(out->source_path,
           raw + 24u,
           CSB_V1_SAVE_EXPORT_SOURCE_PATH_LEN);
    out->source_path[CSB_V1_SAVE_EXPORT_SOURCE_PATH_LEN - 1u] = '\0';
    return CSB_V1_SAVE_EXPORT_OK;
}

int csb_v1_save_export_validate_envelope(const uint8_t *raw,
                                           size_t raw_size)
{
    CSB_V1_SaveExportHeader hdr;
    int rc;

    rc = csb_v1_save_export_parse_header(raw, raw_size, &hdr);
    if (rc != CSB_V1_SAVE_EXPORT_OK) return rc;

    if (hdr.manifest_version != CSB_V1_SAVE_EXPORT_MANIFEST_VERSION)
        return CSB_V1_SAVE_EXPORT_ERR_BAD_VERSION;
    if (hdr.game_id != CSB_V1_SAVE_EXPORT_GAME_ID)
        return CSB_V1_SAVE_EXPORT_ERR_BAD_VERSION;
    if (hdr.reserved != 0u)
        return CSB_V1_SAVE_EXPORT_ERR_BAD_VERSION;
    if (hdr.payload_kind != 0u && hdr.payload_kind != 1u)
        return CSB_V1_SAVE_EXPORT_ERR_BAD_PAYLOAD_KIND;
    if (hdr.payload_len == 0u)
        return CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL;
    if (hdr.payload_len > CSB_V1_SAVE_EXPORT_MAX_PAYLOAD)
        return CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL;

    {
        size_t expected = CSB_V1_SAVE_EXPORT_HEADER_LEN + hdr.payload_len;
        if (raw_size < expected) return CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL;

        {
            uint32_t crc = csb_v1_save_export_crc32(
                raw + CSB_V1_SAVE_EXPORT_HEADER_LEN, hdr.payload_len);
            if (crc != hdr.payload_crc) return CSB_V1_SAVE_EXPORT_ERR_BAD_CRC;
        }
    }
    return CSB_V1_SAVE_EXPORT_OK;
}

/* ── Round-trip a CSB_V1_PartyState ────────────────────────────────── */

long csb_v1_save_export_roundtrip(const CSB_V1_PartyState *party,
                                   unsigned int csb_version,
                                   const char *source_path,
                                   uint8_t *out,
                                   size_t out_capacity)
{
    long payload_len;
    long envelope_len;
    uint16_t payload_kind;
    /* Local scratch for the CSB v2.x payload. We must NOT
     * reuse `out` for both the payload and the envelope —
     * the envelope header overwrites the first
     * CSB_V1_SAVE_EXPORT_HEADER_LEN bytes, which would corrupt
     * the payload the envelope is supposed to wrap. The
     * scratch buffer is small (≤ 896 bytes for a 4-champion
     * CSB v2.x container) so it lives on the stack. */
    uint8_t payload_scratch[CSB_SAVE_HEADER_SIZE
                            + CSB_V1_MAX_CHAMPIONS * CSB_SAVE_CHAMP_SIZE];

    if (!party) return CSB_V1_SAVE_EXPORT_ERR_NULL;
    if (party->ChampionCount < 1 || party->ChampionCount > CSB_V1_MAX_CHAMPIONS)
        return CSB_V1_SAVE_EXPORT_ERR_BAD_PARTY;
    if (csb_version != CSB_SAVE_VERSION_V20 && csb_version != CSB_SAVE_VERSION_V21)
        return CSB_V1_SAVE_EXPORT_ERR_BAD_VERSION;
    payload_kind = (csb_version == CSB_SAVE_VERSION_V20) ? 0u : 1u;

    /* Build the raw CSB v2.x payload first into local scratch. */
    payload_len = csb_v1_build_csb_save_buffer(
        party, csb_version, payload_scratch,
        (long)sizeof(payload_scratch));
    if (payload_len < 0) {
        /* Map CSB_SaveImportResult errors onto our envelope
         * errors where possible. The mapping is intentionally
         * coarse — both modules fail closed and the test
         * driver asserts the exact negative code. */
        switch (payload_len) {
        case CSB_SAVE_IMPORT_ERR_NULL:        return CSB_V1_SAVE_EXPORT_ERR_NULL;
        case CSB_SAVE_IMPORT_ERR_BAD_MAGIC:   return CSB_V1_SAVE_EXPORT_ERR_BAD_PARTY;
        case CSB_SAVE_IMPORT_ERR_VERSION:     return CSB_V1_SAVE_EXPORT_ERR_BAD_VERSION;
        case CSB_SAVE_IMPORT_ERR_NO_CHAMPIONS:return CSB_V1_SAVE_EXPORT_ERR_BAD_PARTY;
        case CSB_SAVE_IMPORT_ERR_TRUNCATED:   return CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL;
        default:                              return CSB_V1_SAVE_EXPORT_ERR_BAD_PARTY;
        }
    }
    envelope_len = csb_v1_save_export_build_envelope(
        payload_scratch, (size_t)payload_len, payload_kind,
        source_path, out, out_capacity);
    if (envelope_len < 0) return envelope_len;
    return envelope_len;
}

/* ── Import through the production loader ─────────────────────────── */

int csb_v1_save_export_import_envelope(CSB_V1_PartyState *party,
                                        const uint8_t *raw,
                                        size_t raw_size)
{
    CSB_V1_SaveExportHeader hdr;
    int rc;
    int loader_rc;

    if (!party || !raw) return CSB_V1_SAVE_EXPORT_ERR_NULL;

    rc = csb_v1_save_export_validate_envelope(raw, raw_size);
    if (rc != CSB_V1_SAVE_EXPORT_OK) return rc;

    rc = csb_v1_save_export_parse_header(raw, raw_size, &hdr);
    if (rc != CSB_V1_SAVE_EXPORT_OK) return rc;

    /* Hand the inner payload to the production loader. This is
     * the load-bearing call — if the round-trip ever regressed,
     * this is where it would surface. */
    loader_rc = csb_v1_import_csb_save_buffer(
        party,
        raw + CSB_V1_SAVE_EXPORT_HEADER_LEN,
        (long)hdr.payload_len);
    return loader_rc;  /* positive count on success, negative on failure */
}

/* ── File I/O ──────────────────────────────────────────────────────── */

int csb_v1_save_export_write_envelope(const char *path,
                                        const uint8_t *envelope,
                                        size_t envelope_len)
{
    FILE *f;
    size_t got;

    if (!path || !envelope) return CSB_V1_SAVE_EXPORT_ERR_NULL;
    if (envelope_len < CSB_V1_SAVE_EXPORT_HEADER_LEN)
        return CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL;
    if (envelope_len > CSB_V1_SAVE_EXPORT_MAX_ENVELOPE)
        return CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL;

    f = fopen(path, "wb");
    if (!f) return CSB_V1_SAVE_EXPORT_ERR_IO;
    got = fwrite(envelope, 1u, envelope_len, f);
    fclose(f);
    if (got != envelope_len) return CSB_V1_SAVE_EXPORT_ERR_IO;
    return CSB_V1_SAVE_EXPORT_OK;
}

int csb_v1_save_export_read_envelope(const char *path,
                                       uint8_t *out,
                                       size_t out_capacity)
{
    FILE *f;
    long sz;
    size_t got;
    int rc;

    if (!path || !out) return CSB_V1_SAVE_EXPORT_ERR_NULL;
    if (out_capacity < CSB_V1_SAVE_EXPORT_HEADER_LEN)
        return CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL;

    f = fopen(path, "rb");
    if (!f) return CSB_V1_SAVE_EXPORT_ERR_IO;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return CSB_V1_SAVE_EXPORT_ERR_IO; }
    sz = ftell(f);
    if (sz < 0) { fclose(f); return CSB_V1_SAVE_EXPORT_ERR_IO; }
    if ((size_t)sz > out_capacity) {
        /* Truncate the read so the caller still gets a header it
         * can use to size a larger buffer. */
        if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return CSB_V1_SAVE_EXPORT_ERR_IO; }
        got = fread(out, 1u, out_capacity, f);
        fclose(f);
        return (int)got;  /* tell caller how many bytes landed in `out` */
    }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return CSB_V1_SAVE_EXPORT_ERR_IO; }
    got = fread(out, 1u, (size_t)sz, f);
    fclose(f);
    if (got != (size_t)sz) return CSB_V1_SAVE_EXPORT_ERR_IO;

    /* Validate before reporting success so callers don't treat a
     * truncated envelope as a well-formed import. */
    rc = csb_v1_save_export_validate_envelope(out, got);
    if (rc != CSB_V1_SAVE_EXPORT_OK) return rc;
    return (int)got;
}

int csb_v1_save_export_scan(const char *data_dir,
                              int max_depth,
                              CSB_V1_SaveExportScanResult *out)
{
    /* Skip-safe scan: walk data_dir looking for *.csbsave files
     * (the suggested extension for FSSB envelopes). For every
     * candidate we try to validate_envelope; the count fields
     * stay deterministic on missing data because we never call
     * exit / error out. */
    char cmd[2048];
    FILE *p;
    char line[1024];

    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    if (!data_dir || data_dir[0] == '\0' || max_depth < 0) return 0;

    snprintf(cmd, sizeof(cmd),
             "find '%s' -maxdepth %d -type f -name '*.csbsave' "
             "2>/dev/null", data_dir, max_depth);
    p = popen(cmd, "r");
    if (!p) return 0;
    while (fgets(line, sizeof(line), p)) {
        size_t n = strlen(line);
        uint8_t scratch[CSB_V1_SAVE_EXPORT_MAX_ENVELOPE];
        size_t got;

        while (n > 0u && (line[n - 1u] == '\n' || line[n - 1u] == '\r')) {
            line[--n] = '\0';
        }
        if (n == 0u) continue;
        ++out->present_count;

        {
            FILE *f = fopen(line, "rb");
            if (!f) continue;
            got = fread(scratch, 1u, sizeof(scratch), f);
            fclose(f);
            if (got < CSB_V1_SAVE_EXPORT_HEADER_LEN) continue;
        }
        if (csb_v1_save_export_validate_envelope(scratch, got)
            == CSB_V1_SAVE_EXPORT_OK) {
            ++out->well_formed_count;
            if (out->first_path[0] == '\0') {
                size_t copy_len = n;
                if (copy_len >= sizeof(out->first_path)) {
                    copy_len = sizeof(out->first_path) - 1u;
                }
                memcpy(out->first_path, line, copy_len);
                out->first_path[copy_len] = '\0';
            }
        }
    }
    pclose(p);
    return 1;  /* always 1 on a successful scan call; missing data is 0 */
}

/* ── Source-evidence citation ──────────────────────────────────────── */

const char *csb_v1_save_export_source_evidence(void)
{
    return
        "ReDMCSB CEDTINC8.C:101-118 (CSBGAME routing via GameIndex + DungeonID)\n"
        "ReDMCSB LOADSAVE.C F0433/F0435 (CSBGAME namespace + C29 key)\n"
        "ReDMCSB SAVEHEAD.C F0429/F0430 (header read/write)\n"
        "ReDMCSB DEFS.H:1289 (CSBGAME.DAT magic literal)\n"
        "ReDMCSB CEDTDATA.C:392/406 (file id dispatch for M742 / M746)\n"
        "CSBWin SaveGame.cpp:927/1711/2111 (save file I/O + 512-byte XOR header)\n"
        "CSBWin CSBCode.cpp:421-422 (csbgame.dat / csbgame.bak literals)\n"
        "include/csb_v1_save_import_path_pc34_compat.h (CSB_V1_PartyState + CSB_SaveImportResult + build_csb_save_buffer)\n"
        "include/csb_v1_csbwin_save_loader_boundary_pc34_compat.h (sibling shape gate for CSBWin / DM1 save bytes)\n"
        "ISO 3309 / ITU-T V.42 CRC-32 (poly 0xEDB88320 reflected, used by PNG / gzip / zlib)\n"
        "docs/FIRESTAFF_GAP_LIST.md row CSB save / import compatibility (real CSB save-byte round-trip beyond the launcher quick-resume manifest)";
}
