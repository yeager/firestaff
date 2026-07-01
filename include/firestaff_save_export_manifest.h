#ifndef FIRESTAFF_SAVE_EXPORT_MANIFEST_H
#define FIRESTAFF_SAVE_EXPORT_MANIFEST_H

/*
 * firestaff_save_export_manifest.h
 *
 * Per-game save-byte export / import manifest layer.
 *
 * Scope:
 *   This is the per-game counterpart of M12's
 *   M12_Config_ExportSaveManifestJSON / ImportSaveManifestJSON
 *   helpers. The launcher-level manifest records only the
 *   quick-resume path/context (see config_m12.c, scope =
 *   "launcher-known-save-paths", runtime_save_bytes_included =
 *   0). This layer carries the actual save bytes plus a
 *   portable per-slot sidecar manifest so a launcher or
 *   sidecar tool can move a Firestaff-format save between
 *   machines / directories without invoking the runtime save
 *   modules.
 *
 * What this is:
 *   - A bounded, data-free compatibility proof that an
 *     existing on-disk Firestaff-format save file can be
 *     exported through a portable bundle directory
 *     (<basename>.savebin + <basename>.savebin.json sidecar)
 *     and re-imported byte-exactly into another data
 *     directory.
 *   - Magic/version/CRC preservation is verified on import
 *     so a future format drift surfaces as a FAIL rather
 *     than a silent round-trip.
 *   - First-launcher coverage for DM1 (FSDM1SV1) because that
 *     is the strongest playable target. CSB (RDMCSB20
 *     SaveGameHeader_Compat), DM2 (0xBEEF / 0xDEAD slot
 *     magic), Nexus (FNXS native), and Theron (TQR /
 *     .tqsv) all have per-game kinds so a sidecar tool can
 *     route them, but the proof is centered on the DM1
 *     Firestaff-native format.
 *
 * What this is NOT:
 *   - Not a runtime save/load implementation. The per-game
 *     save modules under src/<game>/ remain the source of
 *     truth for in-game save / load commands. This layer
 *     only moves existing bytes between paths.
 *   - Not a cross-version save migration. The manifest
 *     records the magic + format version + body CRC of the
 *     source save; importing a manifest whose game-kind /
 *     magic / format-version does not match the target
 *     runtime returns FIRESTAFF_SAVE_EXPORT_KIND_MISMATCH /
 *     BAD_MAGIC / BAD_VERSION so a cross-version migration
 *     is a visible boundary instead of a silent corruption.
 *   - Not a copy of original (DMWeb / CSBWin / skproject)
 *     save formats. The launcher has its own
 *     SaveGameHeader_Compat / DM1SaveHeader / FNXS / TQR
 *     native shapes and the manifest only proves round-trips
 *     inside that native shape. A real per-game import
 *     remains a separate, per-game concern.
 *
 * Source of truth:
 *   - src/engine/config_m12.c M12_Config_ExportSaveManifestJSON
 *     and ImportSaveManifestJSON: the launcher-level
 *     companion and the JSON shape this sidecar mirrors.
 *   - include/dm1_v1_save_load.h (DM1_SAVE_MAGIC = "FSDM1SV1",
 *     DM1_SAVE_FORMAT_VERSION = 1): the magic + version
 *     the sidecar validates against for kind = DM1_V1.
 *   - include/memory_savegame_pc34_compat.h
 *     SaveGameHeader_Compat ("RDMCSB20" magic + formatVersion
 *     + bodyCRC32): the canonical Firestaff-native save
 *     header shared with CSB (kind = CSB_V1 / DM2_V1).
 *   - include/nexus_v1_save.h (NEXUS_SAVE_MAGIC = "FNXS"):
 *     the kind = NEXUS_V1 magic.
 *   - include/theron_v1_save_load.h (THERON_SAVE_MAGIC =
 *     "TQR "): the kind = THERON_V1 magic.
 *   - docs/FIRESTAFF_GAP_LIST.md "Save export/import" row:
 *     the launcher-level "FIXED" row that this per-game
 *     layer complements (the launcher row is
 *     runtime_save_bytes_included = 0; this layer is the
 *     per-game bytes-included follow-up).
 *
 * Build (mirrors firestaff_save_browser_m12 / x68k classifier
 * pattern):
 *   cmake --build build --target firestaff_save_export_manifest --parallel
 *   ctest --test-dir build -R '^firestaff_save_export_manifest(_real_corpus)?$' \
 *         --output-on-failure
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Per-game save kind the manifest records. Strings below are
 * the canonical lowercase token written to the sidecar JSON
 * "kind" field. */
typedef enum FirestaffSaveExportKind {
    FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN = 0,
    FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1  = 1,  /* "dm1_v1" — FSDM1SV1 magic */
    FIRESTAFF_SAVE_EXPORT_KIND_CSB_V1  = 2,  /* "csb_v1" — SaveGameHeader_Compat RDMCSB20 */
    FIRESTAFF_SAVE_EXPORT_KIND_DM2_V1  = 3,  /* "dm2_v1" — 0xBEEF/0xDEAD slot magic */
    FIRESTAFF_SAVE_EXPORT_KIND_NEXUS_V1 = 4, /* "nexus_v1" — FNXS magic */
    FIRESTAFF_SAVE_EXPORT_KIND_THERON_V1 = 5 /* "theron_v1" — TQR / .tqsv magic */
} FirestaffSaveExportKind;

/* Return the canonical kind token (lowercase ASCII, NUL
 * terminated) or NULL for KIND_UNKNOWN. */
const char* FirestaffSaveExportKind_Token(FirestaffSaveExportKind kind);

/* Parse a kind token back to the enum. Returns KIND_UNKNOWN
 * for unknown / NULL / empty strings. */
FirestaffSaveExportKind FirestaffSaveExportKind_Parse(const char* token);

/* ── Detection helpers ──────────────────────────────────────
 *
 * These mirror the header structures of the per-game save
 * modules so a launcher / sidecar tool can pick the right
 * kind from the on-disk bytes without linking the per-game
 * save modules. They read ONLY the magic + version prefix
 * that is at a known offset; they do not parse the full
 * save.
 */

/* Bounds for the magic + version probe. The probe never
 * reads past the smaller of in_size and the documented
 * header size for that kind. */
#define FIRESTAFF_SAVE_EXPORT_PROBE_MAX 256

/* Auto-detect the kind of an in-memory save prefix. Returns
 * KIND_UNKNOWN if the bytes are too small, have no recognised
 * magic, or the version field is implausible for that kind.
 *
 * If outMagic is non-NULL and the function returns a known
 * kind, outMagic is filled with the kind's documented magic
 * string ("FSDM1SV1", "RDMCSB20", "FNXS", "TQR ", or the
 * DM2 slot-magic token) NUL terminated; otherwise outMagic
 * is set to an empty string. */
FirestaffSaveExportKind FirestaffSaveExport_DetectKind(
    const unsigned char* bytes, size_t in_size,
    char* outMagic, size_t outMagicSize,
    uint32_t* outFormatVersion);

/* Auto-detect from an on-disk path. Returns KIND_UNKNOWN and
 * sets the error reason when the file cannot be opened or is
 * too small. */
FirestaffSaveExportKind FirestaffSaveExport_DetectKindFromFile(
    const char* path,
    char* outMagic, size_t outMagicSize,
    uint32_t* outFormatVersion,
    char* outError, size_t outErrorSize);

/* ── Manifest document shape (sidecar JSON) ───────────────
 *
 * Written into <basename>.savebin.json next to <basename>.savebin
 * under the export directory:
 *
 *   {
 *     "version": "1.0",
 *     "type":   "firestaff-per-game-save-export-manifest",
 *     "kind":   "dm1_v1",
 *     "magic":  "FSDM1SV1",
 *     "format_version": 1,
 *     "body_crc32":     0xCBF43926,
 *     "file_size":      96,
 *     "source_path":    "/abs/orig/path/firestaff-dm1-slot.sav",
 *     "exported_bytes": "firestaff-dm1-slot.savebin",
 *     "exported_at_unix": 1735689600
 *   }
 *
 * "kind" matches FirestaffSaveExportKind_Token().
 * "magic" matches the bytes copied verbatim from
 * outMagic above.
 * "body_crc32" is the CRC32 (IEEE 802.3 reflected) of the
 * bytes copied to <basename>.savebin.
 * "file_size" is the byte length of <basename>.savebin
 * (== source_path size at export time).
 * "source_path" is the absolute path the bytes were copied
 * from.
 * "exported_bytes" is the basename written into exportDir.
 * "exported_at_unix" is the wall-clock time of export, or 0
 * if the platform clock could not be read.
 *
 * On import the reader validates:
 *   - type == "firestaff-per-game-save-export-manifest"
 *   - kind matches the requested kind
 *   - magic matches the requested kind's documented magic
 *   - format_version matches the kind's documented version
 *     (the requester passes the version it expects to load
 *     against)
 *   - body_crc32 matches the CRC32 of <basename>.savebin
 *   - file_size matches <basename>.savebin file size
 * Then it copies <basename>.savebin to targetPath with
 * no-overwrite semantics (returns
 * FIRESTAFF_SAVE_EXPORT_TARGET_EXISTS if the target already
 * exists).
 */
#define FIRESTAFF_SAVE_EXPORT_MANIFEST_TYPE  \
    "firestaff-per-game-save-export-manifest"
#define FIRESTAFF_SAVE_EXPORT_MANIFEST_VERSION "1.0"
#define FIRESTAFF_SAVE_EXPORT_BIN_SUFFIX ".savebin"
#define FIRESTAFF_SAVE_EXPORT_JSON_SUFFIX ".savebin.json"

/* ── Result codes (negative = error) ─────────────────────── */
typedef enum FirestaffSaveExportResult {
    FIRESTAFF_SAVE_EXPORT_OK                  =  0,
    FIRESTAFF_SAVE_EXPORT_NULL_ARG            = -1,
    FIRESTAFF_SAVE_EXPORT_BAD_PATH            = -2,
    /* NOTE: result codes deliberately do not share names with
     * FirestaffSaveExportKind so that callers do not mistake one
     * enum for the other. The KIND_* family lives above. */
    FIRESTAFF_SAVE_EXPORT_KIND_NOT_DETECTED   = -3,
    FIRESTAFF_SAVE_EXPORT_KIND_MISMATCH       = -4,
    FIRESTAFF_SAVE_EXPORT_BAD_MAGIC           = -5,
    FIRESTAFF_SAVE_EXPORT_BAD_VERSION         = -6,
    FIRESTAFF_SAVE_EXPORT_BAD_CRC             = -7,
    FIRESTAFF_SAVE_EXPORT_BAD_SIZE            = -8,
    FIRESTAFF_SAVE_EXPORT_FILE_OPEN           = -9,
    FIRESTAFF_SAVE_EXPORT_FILE_READ           = -10,
    FIRESTAFF_SAVE_EXPORT_FILE_WRITE          = -11,
    FIRESTAFF_SAVE_EXPORT_TARGET_EXISTS       = -12,
    FIRESTAFF_SAVE_EXPORT_PARSE_FAILED        = -13,
    FIRESTAFF_SAVE_EXPORT_IO_ERROR            = -14
} FirestaffSaveExportResult;

/* English error string for a result code. */
const char* FirestaffSaveExportResult_String(FirestaffSaveExportResult rc);

/* ── Export ─────────────────────────────────────────────── */

/* Export the save file at sourcePath under exportDir.
 *
 * On success returns FIRESTAFF_SAVE_EXPORT_OK and writes:
 *   <exportDir>/<basename(sourcePath)>.savebin
 *   <exportDir>/<basename(sourcePath)>.savebin.json
 *
 * outBinPath / outManifestPath (optional) receive the
 * absolute paths written. Pass NULL to skip the
 * corresponding output.
 *
 * If the file's kind cannot be auto-detected, the function
 * returns FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN and writes the
 * detection reason into outError. The caller can then retry
 * with FirestaffSaveExport_ExportWithKind() and pass an
 * explicit kind for kinds the detector does not yet know
 * (e.g. DM2 slot-magic layouts that vary by save tool).
 */
FirestaffSaveExportResult FirestaffSaveExport_ExportFile(
    const char* sourcePath,
    const char* exportDir,
    char* outBinPath, size_t outBinPathSize,
    char* outManifestPath, size_t outManifestPathSize,
    char* outError, size_t outErrorSize);

/* Same as above but with an explicit kind. The caller is
 * responsible for the kind being correct; the function
 * still validates that the on-disk magic + version match
 * the requested kind and returns the appropriate result
 * code if not. */
FirestaffSaveExportResult FirestaffSaveExport_ExportFileWithKind(
    const char* sourcePath,
    const char* exportDir,
    FirestaffSaveExportKind kind,
    char* outBinPath, size_t outBinPathSize,
    char* outManifestPath, size_t outManifestPathSize,
    char* outError, size_t outErrorSize);

/* ── Import ─────────────────────────────────────────────── */

/* Read a sidecar manifest under importDir, validate the
 * kind / magic / version / CRC, and copy the bytes to
 * targetPath. Existing targetPath is preserved (returns
 * FIRESTAFF_SAVE_EXPORT_TARGET_EXISTS).
 *
 * If exportBasename is NULL, the first <basename>.savebin
 * + <basename>.savebin.json pair found in importDir is
 * used; otherwise the exportBasename (without any suffix)
 * is used as the lookup key.
 *
 * expectedKind / expectedMagic / expectedFormatVersion are
 * the values the runtime intends to load against. Pass
 * KIND_UNKNOWN / NULL / 0 to accept any kind / magic /
 * version (used by the sidecar tool's "list what's in this
 * directory" path).
 */
FirestaffSaveExportResult FirestaffSaveExport_ImportFile(
    const char* importDir,
    const char* exportBasename,
    FirestaffSaveExportKind expectedKind,
    const char* expectedMagic,
    uint32_t expectedFormatVersion,
    const char* targetPath,
    char* outBinPath, size_t outBinPathSize,
    char* outManifestPath, size_t outManifestPathSize,
    char* outError, size_t outErrorSize);

/* ── Helpers ────────────────────────────────────────────── */

/* Compute CRC32 (IEEE 802.3 reflected) of in_size bytes at
 * data. Returns 0 for NULL data or zero size (the same
 * convention as DM1_CRC32 and the rest of the codebase). */
uint32_t FirestaffSaveExport_CRC32(const unsigned char* data, size_t in_size);

/* Build <basename>.savebin and <basename>.savebin.json paths
 * inside dir for the given source basename. Returns 1 on
 * success and writes both paths (NUL terminated). */
int FirestaffSaveExport_BuildPaths(const char* dir,
                                   const char* sourceBasename,
                                   char* outBin, size_t outBinSize,
                                   char* outManifest, size_t outManifestSize);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SAVE_EXPORT_MANIFEST_H */
