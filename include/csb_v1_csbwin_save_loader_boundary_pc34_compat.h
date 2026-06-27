/*
 * csb_v1_csbwin_save_loader_boundary_pc34_compat.h
 *
 * CSB V1 CSBWin save-side loader-boundary evidence gate.
 *
 * Closes a slice of the "Custom dungeon import (DM1 dungeon.dat,
 * CSBWin dmsave/csbgame)" gap (FIRESTAFF_GAP_LIST.md row C3 / A3).
 *
 * This module is the *loader-side* counterpart to the format
 * classifier in include/csb_v1_save_import_path_pc34_compat.h and
 * the on-disk shape detector that the sibling
 * gapbug_20260627_custom_dungeon_csbwin_import_evidence lane
 * ships as csb_v1_csbwin_save_classify_pc34_compat. Where the
 * classifier asks "what shape is this on disk?", this module asks
 * "what does the existing csb_v1_import_csb_save_buffer() loader
 * do when handed that shape?".
 *
 * The split is intentional and complementary:
 *
 *   - csb_v1_save_import_path_pc34_compat owns the bytes →
 *     CSB_V1_PartyState import contract for the CSB v2.0/v2.1
 *     container ("CSBGAME\0" + uint32 version 0x200/0x201).
 *   - csb_v1_csbwin_save_loader_boundary_pc34_compat owns the
 *     bytes → CSB_SaveImportResult evidence contract for the
 *     broader CSBWin / DM1 save-shape surface. It exercises the
 *     existing importer against each documented CSBWin save
 *     shape (DM1 raw, CSB+DSA, CSBWin 512-byte obfuscated, .bak,
 *     too-small, no-magic, version-mismatch, champion-count-out-of-
 *     range, truncated records) and records for each shape
 *     whether the loader's actual behaviour matches the documented
 *     accept/reject contract. The result is a deterministic,
 *     evidence-only verdict the launcher / M11 can quote when a
 *     user drops a CSBWin export into ~/.firestaff/data/.
 *
 * The module does not import a save, does not decode an
 * obfuscated CSBWin 512-byte header, and does not bind into
 * M11/M12. It only feeds synthetic / real byte buffers into
 * csb_v1_import_csb_save_buffer() and reports the contract match.
 *
 * Source references:
 *   - ReDMCSB CEDTINC8.C:101-118 (DMSAVE / CSBGAME.DAT routing)
 *   - ReDMCSB LOADSAVE.C F0433/F0435 (CSBGAME namespace)
 *   - ReDMCSB SAVEHEAD.C F0429/F0430 (header read/write)
 *   - ReDMCSB DEFS.H:1289 (CSBGAME.DAT magic)
 *   - CSBWin SaveGame.cpp:927/1711/2111 (save file I/O)
 *   - CSBWin CSBCode.cpp:421-422 ("csbgame.dat" / "csbgame.bak")
 *   - CSBWin Data.h:590 (SaveGameFilename field)
 *   - include/memory_savegame_pc34_compat.h:227 (RDMCSB15 magic)
 *   - docs/FIRESTAFF_GAP_LIST.md row "CSBWin custom resource
 *     handling (csbgraphics.dat + dmsave + csbgame)".
 *
 * Scope (non-claims):
 *   - No file I/O. Callers feed bytes; the module reports.
 *   - No LZW / RLE / XOR obfuscation decoding. The CSBWin
 *     512-byte shape is documented as loader-reject here; a
 *     separate CSBWin obfuscation-key decoder remains tracked
 *     under docs/FIRESTAFF_GAP_LIST.md as OPEN-LARGE.
 *   - No runtime binding. Callers (a future launcher import
 *     button) decide what to do with the verdict.
 *   - No real CSBWin save bytes are vendored; fixtures are
 *     synthetic and live in test_csb_v1_csbwin_save_loader_
 *     boundary_pc34_compat.c.
 */

#ifndef FIRESTAFF_CSB_V1_CSBWIN_SAVE_LOADER_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_CSBWIN_SAVE_LOADER_BOUNDARY_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_save_import_path_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Documented CSBWin / DM1 save shapes ──────────────────────────────
 *
 * Each shape names an on-disk byte pattern a launcher or M11
 * might receive when a user stages a CSBWin or DM1 save. The
 * loader-boundary contract below is the documented accept/reject
 * behaviour for csb_v1_import_csb_save_buffer() against each
 * shape; the test in tests/test_csb_v1_csbwin_save_loader_
 * boundary_pc34_compat.c pins the actual behaviour against the
 * documented contract for every shape.
 *
 * The list is intentionally narrow: it covers the shapes the
 * existing CSB V1 importer + ReDMCSB / CSBWin source files
 * actually recognise today. Future work (CSBWin 512-byte XOR
 * decode, DM1 raw→CSB conversion) will extend it.
 */
typedef enum {
    /* "CSBGAME\0" + uint32 LE 0x200 — accept (today's importer
     * fully decodes this). */
    CSB_V1_CSBWIN_SHAPE_CSBGAME_V20 = 0,
    /* "CSBGAME\0" + uint32 LE 0x201 — accept (today's importer
     * fully decodes this). */
    CSB_V1_CSBWIN_SHAPE_CSBGAME_V21 = 1,
    /* "RDMCSB15" (8-byte ASCII) — reject (BAD_MAGIC). The DM1
     * raw save needs a separate DM1→CSB conversion path tracked
     * under Champions GAP 3. */
    CSB_V1_CSBWIN_SHAPE_DM1_RAW_RDMCSB15 = 2,
    /* "CSBGAME\0" + uint32 + CDSA marker at offset 12 — reject
     * (VERSION). Today's importer does not understand the CDSA
     * marker; routed shapes require a separate DSA section
     * extractor. */
    CSB_V1_CSBWIN_SHAPE_CSBGAME_CDSA = 3,
    /* "CSB\1" / "DM\0\1" / "CEDT" (CSBWin 512-byte obfuscated
     * header) — reject (BAD_MAGIC). The per-disk XOR pad is not
     * yet reproduced; OPEN-LARGE. */
    CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CSB1 = 4,
    CSB_V1_CSBWIN_SHAPE_CSBWIN_512_DM01 = 5,
    CSB_V1_CSBWIN_SHAPE_CSBWIN_512_CEDT = 6,
    /* < 8-byte magic — reject (TRUNCATED). */
    CSB_V1_CSBWIN_SHAPE_TOO_SMALL_UNDER_8 = 7,
    /* 8+ bytes but no recognised magic — reject (BAD_MAGIC). */
    CSB_V1_CSBWIN_SHAPE_NO_MAGIC_8_PLUS = 8,
    /* CSB v2.0 header with champion_count = 0 — reject
     * (NO_CHAMPIONS). */
    CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_0 = 9,
    /* CSB v2.0 header with champion_count = 5 (out of 1..4
     * range) — reject (NO_CHAMPIONS). */
    CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_CHAMP_COUNT_5 = 10,
    /* CSB v2.0 header with champion_count = 4 but the buffer
     * is too short to hold four 160-byte records — reject
     * (TRUNCATED). */
    CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_TRUNCATED_RECORDS = 11,
    /* CSB v2.0 header with version 0x055 — reject (VERSION). */
    CSB_V1_CSBWIN_SHAPE_CSBGAME_BAD_VERSION = 12,
    /* CSB V2.0 .bak filename backed buffer — filename-based
     * detection: the launcher sees a `.bak` suffix and treats it
     * as a backup copy of the underlying recognised shape. The
     * loader itself only sees the bytes; the verdict below is
     * for the underlying CSBGAME_V20 buffer. (The .bak filename
     * detection itself lives in the on-disk shape classifier
     * sibling module; this module just verifies the loader
     * ignores the suffix and accepts the underlying bytes.) */
    CSB_V1_CSBWIN_SHAPE_CSBGAME_V20_BAK_PAYLOAD = 13,

    CSB_V1_CSBWIN_SHAPE_COUNT
} CSB_V1_CSBWinSaveShape;

/* Documented contract: what result code is the loader expected
 * to return for this shape? Negative = one of the CSB_SaveImport
 * *Result enum values documented in
 * include/csb_v1_save_import_path_pc34_compat.h; a positive value
 * = the documented champion-count return on accept. */
typedef struct {
    CSB_V1_CSBWinSaveShape shape;
    const char           *label;        /* short human label      */
    int                   expect_accept;/* 1 = expect champion count return; 0 = expect error */
    int                   expect_code;   /* the CSB_SaveImportResult value the loader must return on reject, or 0 when expect_accept */
    const char           *source_evidence; /* one-line ReDMCSB / CSBWin source pointer */
} CSB_V1_CSBWinSaveShapeContract;

/* The full contract table — one row per CSB_V1_CSBWinSaveShape.
 * NULL-terminated so callers can iterate. */
const CSB_V1_CSBWinSaveShapeContract *
csb_v1_csbwin_save_loader_boundary_contract(size_t *out_count);

/* ── Result struct ───────────────────────────────────────────────────── */

/* The full evidence record for one shape evaluation. */
typedef struct {
    CSB_V1_CSBWinSaveShape shape;          /* which shape we tested */
    int                   loader_code;     /* the actual code csb_v1_import_csb_save_buffer returned */
    int                   champion_count;  /* the actual champion count the loader reported (only meaningful when loader_code > 0) */
    int                   contract_match;  /* 1 if loader_code matches the documented contract for this shape; 0 otherwise */
    int                   expected_code;   /* what the documented contract said the loader should return */
    const char           *shape_label;     /* short human label of the tested shape */
} CSB_V1_CSBWinLoaderBoundaryResult;

/* ── Public API ──────────────────────────────────────────────────────── */

/* Build a synthetic byte buffer for the given shape into `out_buf`
 * (capacity `buf_capacity` bytes). Returns the number of bytes
 * written, or 0 if the shape is unknown or the buffer is too
 * small. The returned bytes are deterministic — no randomness —
 * and form the exact fixture the per-shape evidence check feeds
 * into csb_v1_import_csb_save_buffer(). Exposed publicly so the
 * data-free unit test can reuse the builder without depending on
 * internal layout choices. */
size_t csb_v1_csbwin_save_loader_boundary_build_fixture(
    CSB_V1_CSBWinSaveShape shape,
    uint8_t *out_buf,
    size_t buf_capacity);

/* Pure loader-boundary check. Caller hands `bytes`/`size` (the
 * first N bytes of a CSBWin / DM1 save file) and the shape under
 * test; the helper runs csb_v1_import_csb_save_buffer() against
 * the bytes (initialising a stack party), records the actual
 * loader result, and reports whether it matches the documented
 * contract for that shape.
 *
 * The function owns no memory. The party used for the import
 * attempt is a stack CSB_V1_PartyState and is destroyed on
 * return; callers that want the imported party should call
 * csb_v1_import_csb_save_buffer() directly.
 *
 * Returns:
 *   - the value of `out->loader_code` (>= CSB_SAVE_IMPORT_ERR_NULL
 *     on error or > 0 on accept),
 *   - or CSB_SAVE_IMPORT_ERR_NULL on argument failure.
 */
int csb_v1_csbwin_save_loader_boundary_check(
    const uint8_t *bytes,
    size_t         size,
    CSB_V1_CSBWinSaveShape shape,
    CSB_V1_CSBWinLoaderBoundaryResult *out);

/* Convenience: build the synthetic fixture for `shape` and run
 * the loader-boundary check on it. Returns the loader result
 * code (see csb_v1_csbwin_save_loader_boundary_check). */
int csb_v1_csbwin_save_loader_boundary_check_shape(
    CSB_V1_CSBWinSaveShape shape,
    CSB_V1_CSBWinLoaderBoundaryResult *out);

/* Convenience: which shape does the loader recognise? Returns
 * the first CSB_V1_CSBWinSaveShape whose contract specifies
 * expect_accept=1 and whose magic the bytes match; returns
 * CSB_V1_CSBWIN_SHAPE_COUNT when no accept-shape is matched.
 *
 * This is a thin wrapper over csb_v1_csbwin_save_loader_boundary_
 * check() that callers (the launcher, M11) can use to decide
 * whether to attempt an import at all, before paying for the
 * full csb_v1_import_csb_save_buffer() parse.
 */
CSB_V1_CSBWinSaveShape csb_v1_csbwin_save_loader_boundary_match(
    const uint8_t *bytes,
    size_t         size);

/* ── Lookup helpers (used by tests + probe + docs) ───────────────────── */

const char *csb_v1_csbwin_save_loader_boundary_shape_name(
    CSB_V1_CSBWinSaveShape shape);

/* Source-evidence string for tests + docs. */
const char *csb_v1_csbwin_save_loader_boundary_source_evidence(void);

/* The number of accept-shapes (rows where expect_accept=1). */
size_t csb_v1_csbwin_save_loader_boundary_accept_count(void);

/* The number of reject-shapes (rows where expect_accept=0). */
size_t csb_v1_csbwin_save_loader_boundary_reject_count(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_CSBWIN_SAVE_LOADER_BOUNDARY_PC34_COMPAT_H */
