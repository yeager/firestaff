/*
 * csb_v1_save_export_import_pc34_compat.h
 *
 * CSB V1 per-game save-byte export/import compatibility proof.
 *
 * The launcher-side quick-resume manifest
 * (M12_Config_ExportSaveManifestJSON / ImportSaveManifestJSON) only
 * records portable launcher state (path, settings, lastSavePath).
 * It does NOT export or import the actual per-game save bytes,
 * which is the level of portability a CSB player would expect
 * from "Save Manifest" wording.
 *
 * This module adds a CSB-specific per-game save-byte
 * export/import gate that:
 *   1. Wraps the production CSB V1 save container (CSBGAME\0 + v2.0
 *      / v2.1 version word + 1..4 champion records, exactly the
 *      same bytes csb_v1_import_csb_save_buffer() reads) inside a
 *      Firestaff-versioned manifest envelope:
 *        magic     "FSSB"        4 bytes   (Firestaff Save Bytes)
 *        version   uint32 LE     4 bytes   (CSB_SAVE_EXPORT_MANIFEST_VERSION)
 *        game_id   uint32 LE     4 bytes   (CSB_V1_SAVE_GAME_ID = 0x43534201 'CSB\1')
 *        payload_kind uint16 LE  2 bytes   (0 = CSB v2.0, 1 = v2.1)
 *        reserved  uint16 LE     2 bytes   (must be 0)
 *        payload_len uint32 LE   4 bytes   (length of payload bytes)
 *        payload_crc uint32 LE   4 bytes   (CRC-32 / ISO 3309 over payload)
 *        source_path char[64]   64 bytes   (origin artifact path or "(synthetic)")
 *        payload   <payload_len> bytes     (the raw CSB save container)
 *      Total fixed header = 88 bytes; total envelope =
 *      88 + payload_len bytes.
 *
 *   2. Provides a versioned manifest manifest-classification API
 *      that says "this envelope looks like a Firestaff save-byte
 *      export" or "this is a raw CSB save that should NOT be
 *      re-wrapped" — distinguishing FSSB envelopes from raw
 *      CSBGAME\0 / DM1 / CSBWin-512 / CEDT payloads without
 *      double-wrapping.
 *
 *   3. Provides an import path that validates the envelope (magic
 *      + version + reserved + length + CRC) and then feeds the
 *      inner payload through the EXISTING
 *      csb_v1_import_csb_save_buffer() — i.e. a CSB save-byte
 *      round-trip exercises the same production loader the M11
 *      runtime would use. This is the proof that exporting a CSB
 *      save-byte manifest actually yields a buffer the runtime
 *      can re-import.
 *
 * Source-lock boundary:
 *   - ReDMCSB CEDTINC8.C:101-118 (CSBGAME routing)
 *   - ReDMCSB LOADSAVE.C F0433/F0435 (CSBGAME namespace)
 *   - ReDMCSB SAVEHEAD.C F0429/F0430 (header read/write)
 *   - ReDMCSB DEFS.H:1289 (CSBGAME.DAT magic)
 *   - ReDMCSB CEDTDATA.C:392/406 (file id dispatch)
 *   - CSBWin SaveGame.cpp:927/1711/2111 (save file I/O + 512-byte XOR header)
 *   - CSBWin CSBCode.cpp:421-422 (csbgame.dat / csbgame.bak literals)
 *   - include/csb_v1_save_import_path_pc34_compat.h
 *     (CSB_V1_PartyState + CSB_SaveImportResult + build_csb_save_buffer)
 *
 * Non-claims (explicitly out of scope here):
 *   - No real CSBWin 512-byte obfuscation-key decoder.
 *   - No CSB↔DM1 / CSB↔Theron cross-game save migration.
 *   - No M11/M12 wiring of an "Import CSB save bytes" button —
 *     this is the gate behind which such wiring lives. The M12
 *     quick-resume manifest still records the launcher-side
 *     metadata; this module records the per-game save bytes.
 */

#ifndef FIRESTAFF_CSB_V1_SAVE_EXPORT_IMPORT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_SAVE_EXPORT_IMPORT_PC34_COMPAT_H

#include "csb_v1_save_import_path_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Envelope constants ─────────────────────────────────────────────── */

/* "FSSB" — Firestaff Save Bytes envelope magic. */
#define CSB_V1_SAVE_EXPORT_MAGIC       "FSSB"
#define CSB_V1_SAVE_EXPORT_MAGIC_LEN    4u

/* Bumped only when the envelope layout changes in a way that
 * breaks round-trip compatibility. v1 = original 88-byte header
 * + payload_len bytes payload. */
#define CSB_V1_SAVE_EXPORT_MANIFEST_VERSION  1u

/* "CSB\1" — same as CSB_V1_SAVE_MAGIC_CSB but written as four
 * printable ASCII bytes for human-readable manifest inspection. */
#define CSB_V1_SAVE_EXPORT_GAME_ID      0x43534201u  /* 'C','S','B',0x01 */

/* Source-path slot length inside the envelope header (NUL-padded,
 * not NUL-terminated when full). */
#define CSB_V1_SAVE_EXPORT_SOURCE_PATH_LEN  64u

/* Fixed-size envelope header length (everything up to payload). */
#define CSB_V1_SAVE_EXPORT_HEADER_LEN \
    ((size_t)(CSB_V1_SAVE_EXPORT_MAGIC_LEN + 4u + 4u + 2u + 2u + 4u + 4u \
              + CSB_V1_SAVE_EXPORT_SOURCE_PATH_LEN))

/* Maximum supported payload length (4-champion party, CSB v2.x
 * container: 256 + 4*160 = 896 bytes; we allow headroom for
 * future format extensions). */
#define CSB_V1_SAVE_EXPORT_MAX_PAYLOAD  4096u

/* Maximum supported total envelope length. */
#define CSB_V1_SAVE_EXPORT_MAX_ENVELOPE \
    (CSB_V1_SAVE_EXPORT_HEADER_LEN + CSB_V1_SAVE_EXPORT_MAX_PAYLOAD)

/* ── Manifest kind ──────────────────────────────────────────────────── */
/* Result of classifying a buffer as one of the CSB V1 save-byte
 * manifest shapes. Keeps the launcher from double-wrapping a
 * payload that is already an envelope, and from claiming a
 * raw CSBGAME\0 buffer is a Firestaff export. */
typedef enum {
    CSB_V1_SAVE_EXPORT_KIND_UNKNOWN         = 0,
    CSB_V1_SAVE_EXPORT_KIND_FSSB_ENVELOPE   = 1,
    CSB_V1_SAVE_EXPORT_KIND_RAW_CSBGAME_V20 = 2,
    CSB_V1_SAVE_EXPORT_KIND_RAW_CSBGAME_V21 = 3,
    CSB_V1_SAVE_EXPORT_KIND_RAW_DM1_RDMCSB  = 4,
    CSB_V1_SAVE_EXPORT_KIND_RAW_CSBWIN_512  = 5
} CSB_V1_SaveExportKind;

/* ── Result / error codes ───────────────────────────────────────────── */
typedef enum {
    CSB_V1_SAVE_EXPORT_OK                   =  0,
    CSB_V1_SAVE_EXPORT_ERR_NULL             = -1, /* required arg NULL */
    CSB_V1_SAVE_EXPORT_ERR_BUF_TOO_SMALL    = -2, /* output buffer < need */
    CSB_V1_SAVE_EXPORT_ERR_BAD_PARTY        = -3, /* party has no champions */
    CSB_V1_SAVE_EXPORT_ERR_BAD_VERSION      = -4, /* unknown manifest ver */
    CSB_V1_SAVE_EXPORT_ERR_BAD_PAYLOAD_KIND = -5, /* unknown payload_kind */
    CSB_V1_SAVE_EXPORT_ERR_BAD_CRC          = -6, /* CRC-32 mismatch */
    CSB_V1_SAVE_EXPORT_ERR_IO               = -7  /* fopen / fread / fwrite */
} CSB_V1_SaveExportResult;

/* ── Classification API ─────────────────────────────────────────────── */

/* Inspect `bytes` (length `size`) and report which save-byte
 * shape it matches. Trims raw CSBGAME v2.0 / v2.1, FSSB envelope,
 * DM1 RDMCSB15, and CSBWin 512-byte headers. Returns
 * CSB_V1_SAVE_EXPORT_KIND_UNKNOWN when no recognised magic. */
CSB_V1_SaveExportKind csb_v1_save_export_classify(const uint8_t *bytes,
                                                  size_t size);

/* Short ASCII label for the kind (for log messages and the
 * launcher quick-resume status popups). Never NULL. */
const char *csb_v1_save_export_kind_name(CSB_V1_SaveExportKind kind);

/* ── Build an FSSB envelope in memory ──────────────────────────────── */

/* Build an FSSB envelope around `payload` (a CSB v2.0 / v2.1
 * container produced by csb_v1_build_csb_save_buffer). Writes
 * the 88-byte header + payload_len bytes to `out`. Returns total
 * bytes written or a negative CSB_V1_SaveExportResult.
 *
 * `payload_kind` must be 0 (v2.0) or 1 (v2.1). `source_path`
 * may be NULL (recorded as "(synthetic)") or a relative path
 * recorded into the header for diagnostics; longer than 63 chars
 * is truncated. */
long csb_v1_save_export_build_envelope(const uint8_t *payload,
                                       size_t payload_len,
                                       uint16_t payload_kind,
                                       const char *source_path,
                                       uint8_t *out,
                                       size_t out_capacity);

/* Compute the total envelope length given a payload length.
 * Returns 0 on overflow / invalid sizes. */
size_t csb_v1_save_export_envelope_size(size_t payload_len);

/* ── Read an FSSB envelope ──────────────────────────────────────────── */

/* Parsed view of the 88-byte envelope header (NOT a copy of the
 * payload — the caller still owns `raw`). */
typedef struct {
    uint32_t manifest_version;          /* must equal CSB_V1_SAVE_EXPORT_MANIFEST_VERSION */
    uint32_t game_id;                   /* must equal CSB_V1_SAVE_EXPORT_GAME_ID */
    uint16_t payload_kind;              /* 0 = v2.0, 1 = v2.1 */
    uint16_t reserved;                  /* must be 0 */
    uint32_t payload_len;               /* bytes of payload that follow the header */
    uint32_t payload_crc;               /* CRC-32 / ISO 3309 of the payload */
    char     source_path[CSB_V1_SAVE_EXPORT_SOURCE_PATH_LEN]; /* NUL-padded */
} CSB_V1_SaveExportHeader;

/* Parse the 88-byte envelope header at `raw`. Validates magic
 * and reserved field. Returns 0 on success, negative
 * CSB_V1_SaveExportResult on failure. The returned header is
 * a copy; the caller still owns `raw`. */
int csb_v1_save_export_parse_header(const uint8_t *raw,
                                    size_t raw_size,
                                    CSB_V1_SaveExportHeader *out);

/* Validate the entire envelope: header + payload length + CRC.
 * Does NOT touch the CSB V1 party loader — use this when you
 * only want to confirm "this is a well-formed FSSB envelope". */
int csb_v1_save_export_validate_envelope(const uint8_t *raw,
                                          size_t raw_size);

/* ── Round-trip: export a CSB_V1_PartyState as a full envelope ────── */

/* Build a CSB v2.x container from `party`, then wrap it in an
 * FSSB envelope. Writes the full envelope to `out`. The full
 * envelope length is at most
 *   CSB_V1_SAVE_EXPORT_HEADER_LEN +
 *   CSB_SAVE_HEADER_SIZE + 4 * CSB_SAVE_CHAMP_SIZE
 * = 88 + 256 + 640 = 984 bytes (well under
 * CSB_V1_SAVE_EXPORT_MAX_ENVELOPE). Returns total bytes written
 * or a negative CSB_V1_SaveExportResult.
 *
 * `csb_version` must be CSB_SAVE_VERSION_V20 or _V21. */
long csb_v1_save_export_roundtrip(const CSB_V1_PartyState *party,
                                  unsigned int csb_version,
                                  const char *source_path,
                                  uint8_t *out,
                                  size_t out_capacity);

/* ── Import: feed envelope payload through csb_v1_import_csb_save_*
 * (the production loader). ─────────────────────────────────────── */

/* Validate the FSSB envelope at `raw`/`raw_size`, then run the
 * inner payload through csb_v1_import_csb_save_buffer() into
 * `party`. Returns champion count on success, or a negative
 * CSB_V1_SaveExportResult / CSB_SaveImportResult on failure.
 *
 * The point: this is the only public entry point that proves
 * the export/import contract — an exported envelope must round-
 * trip through the production loader, not a parallel parser. */
int csb_v1_save_export_import_envelope(CSB_V1_PartyState *party,
                                        const uint8_t *raw,
                                        size_t raw_size);

/* ── File I/O helpers ──────────────────────────────────────────────── */

/* Write an FSSB envelope to `path`. The buffer is `envelope`
 * (length `envelope_len`, must be ≥ CSB_V1_SAVE_EXPORT_HEADER_LEN).
 * Returns 0 on success, negative CSB_V1_SaveExportResult. */
int csb_v1_save_export_write_envelope(const char *path,
                                       const uint8_t *envelope,
                                       size_t envelope_len);

/* Read an FSSB envelope from `path`. Caller-owned `out`; on
 * success writes the envelope bytes to `out` (up to
 * out_capacity) and returns bytes read, or negative
 * CSB_V1_SaveExportResult. */
int csb_v1_save_export_read_envelope(const char *path,
                                      uint8_t *out,
                                      size_t out_capacity);

/* Skip-safe scan: walk `data_dir` (recursive, depth-limited)
 * looking for an FSSB envelope. Reports the first path found.
 * Returns 1 on first match (path copied to `out_path`, with
 * `present_count` set to the number of matches inspected), 0
 * otherwise. Never errors on missing data. */
typedef struct {
    size_t present_count;       /* how many .csbsave files were inspected */
    size_t well_formed_count;   /* how many passed validate_envelope */
    char   first_path[1024];    /* first match path, or "" if none */
} CSB_V1_SaveExportScanResult;

int csb_v1_save_export_scan(const char *data_dir,
                             int max_depth,
                             CSB_V1_SaveExportScanResult *out);

/* ── CRC-32 / ISO 3309 (single-shot helper used by both the
 *    envelope writer and validator). Initial value 0xFFFFFFFF,
 *    final XOR 0xFFFFFFFF. ───────────────────────────────────────── */
uint32_t csb_v1_save_export_crc32(const uint8_t *data, size_t len);

/* ── Source-evidence citation ──────────────────────────────────────── */
const char *csb_v1_save_export_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_SAVE_EXPORT_IMPORT_PC34_COMPAT_H */
