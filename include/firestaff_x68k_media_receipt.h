/*
 * firestaff_x68k_media_receipt.h
 *
 * Skip-safe real-media receipt + classification gate for the
 * DM1 / CSB X68000 HDM/floppy media import lane
 * (docs/FIRESTAFF_GAP_LIST.md row "DM1 X68000 HDM/floppy
 * media import", "CSB X68000 (jp)" row, "FTL container
 * format (Amiga, X68000, MegaCD)" row).
 *
 * This module is a thin, data-side bridge on top of the
 * synthetic-only X68000 classifier
 * (firestaff_x68k_media_classify.h) and the FTL container
 * parser (firestaff_ftl_container.h). Its job is to:
 *
 *   1. Carry a documented known-hash list of the public DM1
 *      v3.0 (Japanese) and CSB v3.1 (Japanese) HDMs the
 *      dmweb + community preservation community currently
 *      has on hand:
 *
 *        - DM1 v3.0 (Japanese) "Original (Not working)"
 *        - DM1 v3.0 (Japanese) "Cracked"
 *        - DM1 v3.0 (Japanese) "Save Disk"
 *        - CSB v3.1 (Japanese) "Original (Not working)"
 *        - CSB v3.1 (Japanese) "Cracked"
 *        - CSB v3.1 (Japanese) "Save Disk"
 *
 *      The "Original (Not working)" label comes from DMWeb:
 *      the public release lacks the Track 1 Side 1 Sector 9
 *      "HPR-0007" copy-protection sentinel, so the game
 *      refuses to boot on a real X68000 (the protection check
 *      is documented as broken / bypassed on both DM and CSB;
 *      see DMWeb Sharp X68000 section).
 *
 *   2. For each kind, find a real file in the user's data
 *      tree (recursive + ZIP-aware via asset_find_all_by_md5_list),
 *      open it, compute a SHA-256 over its bytes, run the
 *      X68k media classifier, and surface a
 *      FirestaffX68kMediaReceipt that locks in:
 *
 *        - expected_size, actual_size
 *        - expected_md5, actual_md5
 *        - expected_sha256 (when known), actual_sha256
 *        - the X68k media_class + flags + sentinel_offset
 *        - the FTL magic candidate count (huge on full HDMs
 *          that bundle many FTL resources, 1 on single-
 *          resource .FTL payloads, 0 on save disks)
 *        - the "expected class" per DMWeb documentation
 *        - the FTL handoff verdict for the declared
 *          data_area1_memory_size (only meaningful if the
 *          media_class is FULL_DISK; documented as "fits
 *          within the on-disk 1232 KB" since the FTL resources
 *          are stored inside the HDM)
 *
 *   3. Provide a skip-safe scan API: when no data dir is set
 *      or no known X68k HDM is present, scan returns OK with
 *      `matched_count == 0` and the probe exits 0 with a
 *      SKIP message. Hosts without the documented HDMs do
 *      not block CI.
 *
 *   4. Provide a synthetic data-free SelfTest so the module
 *      is always CTest-gated, even on hosts that have no
 *      real HDMs.
 *
 * Source of truth:
 *   - DMWeb DM X68000 edition page:
 *     https://dmweb.free.fr/games/dungeon-master/editions/x68000
 *     Japanese v3.0 line, 1990-01-26 release, HDM original
 *     (cannot boot without copy-protection sectors), cracked
 *     image, blank save disk.
 *   - DMWeb CSB X68000 edition page:
 *     https://dmweb.free.fr/games/chaos-strikes-back/editions/x68000
 *     Japanese v3.1 HDM, same copy-protection scheme as DM
 *     (the same crack works because both games share the
 *     protection), blank save disk.
 *   - DMWeb copy-protection page, "Sharp X68000" section:
 *     2 sides x 77 tracks x 8 sectors x 1024 bytes = 1261568
 *     bytes = 1232 KB geometry; Track 1 Side 1 Sector 9
 *     holds the "HPR-0007" + 4 random bytes sentinel that
 *     the only operational copy-protection check reads.
 *   - firestaff_x68k_media_classify.h (synthetic-only media
 *     classifier we run on every receipted HDM).
 *   - firestaff_ftl_container.h + firestaff_ftl_hunk_data_zero_run.h
 *     for the FTL payload handoff (an FTL declared
 *     data_area1_memory_size must fit the on-disk HDM, which
 *     is exactly what FirestaffX68kMedia_FTLHandoffFits
 *     already verifies for the synthetic path).
 *
 * Scope (kept narrow on purpose):
 *   - Hash-based discovery only (no filename-based fallback).
 *   - Read-only. Never modifies the input file.
 *   - Never opens the HDM as a filesystem, never parses MFM
 *     flux, never decodes FTL/PAK/IMG1/IMG2.
 *   - The "expected class" is a documented classifier, not
 *     an authenticity judgement: we say "DMWeb documents the
 *     public Original HDM as lacking the HPR-0007 sentinel
 *     and being unbootable on a real X68000; this receipt
 *     verifies our classifier agrees". We do not claim the
 *     cracked image is "actually" a crack — the receipt just
 *     records that the byte shape matches the dmweb
 *     description of a cracked image (full 1232 KB, no
 *     sentinel, not a save disk).
 *   - We never claim a real X68000 will boot from these
 *     images. The copy-protection check, the MFM controller,
 *     and the runtime launch path all remain separate
 *     follow-up gaps against docs/FIRESTAFF_GAP_LIST.md.
 *
 * Thread safety: the scan reads files via the OS read()
 * syscall, so it is safe to call from a single thread per
 * invocation. The receipt struct is caller-owned.
 */

#ifndef FIRESTAFF_X68K_MEDIA_RECEIPT_H
#define FIRESTAFF_X68K_MEDIA_RECEIPT_H

#include <stddef.h>
#include <stdint.h>

#include "firestaff_x68k_media_classify.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward-declared opaque "data-dir" handle. The real type
 * lives in asset_find_by_hash.h; we keep the include surface
 * small here so callers do not need to pull the full header
 * for the simple "scan one path" path. */
struct FirestaffAssetScanResult;

/* ── Known-hash list ─────────────────────────────────────────────── */

/* Identifier for each documented DM1 / CSB X68000 HDM. The
 * list is intentionally narrow: only the three DMWeb-documented
 * shapes for each game. Future preserved masters (e.g. a flux
 * dump that DOES carry the HPR-0007 sentinel) can be added by
 * extending this list with a new FirestaffX68kMediaReceipt_Kind
 * entry, not by changing the existing entries. */
typedef enum {
    FIRESTAFF_X68K_RECEIPT_KIND_UNKNOWN = 0,

    /* DM1 v3.0 (Japanese), 1990-01-26. Public release whose
     * copy-protection sector is documented as missing, so it
     * cannot boot on a real X68000. Per DMWeb, the protection
     * check is broken / bypassed on the X68000 line for both
     * DM and CSB; the cracked image is functionally equivalent
     * at the runtime layer. We classify it as
     * "unprotected public HDM" on the byte level: full 1232 KB
     * MFM, no HPR-0007 sentinel, no FTL magic at offset 0. */
    FIRESTAFF_X68K_RECEIPT_KIND_DM1_V30_JP_ORIGINAL = 1,

    /* DM1 v3.0 (Japanese) cracked. The crack bypasses the
     * runtime copy-protection check by patching the boot path
     * in DM.X, not by inserting a HPR-0007 sentinel. Per
     * DMWeb, the cracked image is byte-distinct from the
     * public Original (different MD5, different SHA-256) and
     * is the form that the preservation community treats as
     * the "boots on a real X68000 with the protection check
     * bypassed" shape. We classify it the same way as the
     * Original: full 1232 KB, no sentinel, no FTL magic. */
    FIRESTAFF_X68K_RECEIPT_KIND_DM1_V30_JP_CRACKED = 2,

    /* DM1 v3.0 (Japanese) save disk: freshly formatted 1232 KB
     * image whose sector payloads are all zero. The classifier
     * reports BLANK_SAVE_DISK + PROTECTION_AREA_BLANK on this
     * shape. */
    FIRESTAFF_X68K_RECEIPT_KIND_DM1_V30_JP_SAVE_DISK = 3,

    /* CSB v3.1 (Japanese) Original (Not working). DMWeb notes
     * CSB shares the DM copy-protection scheme (the same crack
     * works), so the byte-level classifier verdict is the same
     * as DM1's Original. */
    FIRESTAFF_X68K_RECEIPT_KIND_CSB_V31_JP_ORIGINAL = 4,

    /* CSB v3.1 (Japanese) Cracked. The CSB crack is a separate
     * "CK.R" program per DMWeb CSB X68000 page, but the byte
     * shape of the cracked image is again the same: full 1232
     * KB, no HPR-0007 sentinel, no FTL magic at offset 0. */
    FIRESTAFF_X68K_RECEIPT_KIND_CSB_V31_JP_CRACKED = 5,

    /* CSB v3.1 (Japanese) save disk: freshly formatted 1232 KB
     * image whose sector payloads are all zero. */
    FIRESTAFF_X68K_RECEIPT_KIND_CSB_V31_JP_SAVE_DISK = 6
} FirestaffX68kMediaReceipt_Kind;

/* Per-kind expected media class. This is the DMWeb-documented
 * "what our classifier should say about a file of this kind".
 * A receipt is "EXPECTED_MATCH" iff
 *   media_class == expected_media_class
 * and
 *   (flags & expected_flags_mask) == expected_flags_value
 * and
 *   size_bytes == actual_size_bytes
 * and
 *   md5 == actual_md5
 * are all true.
 *
 * Sentinel and FTL flags are not required for the public
 * Original / Cracked / Save Disk kinds because the preservation
 * community has only the unprotected / unbooting shape on hand
 * (DMWeb Sharp X68000 copy-protection section). A future
 * preserved-master kind that DOES carry the HPR-0007 sentinel
 * would extend this enum, not change the existing values.
 *
 * On the real preservation HDMs the MFM controller fills the
 * protection-region bytes with 0xE5 ("deleted data" mark), so
 * the PROTECTION_AREA_BLANK flag does NOT fire even on the
 * public Original. The expected_flags_value for those kinds
 * is therefore 0u (we only assert that the bits we do expect
 * to be off really are off). The DM1/CSB save disks are
 * freshly formatted and DO have all-zero sector payloads, so
 * the BLANK_SAVE_DISK + PROTECTION_AREA_BLANK bits fire there. */
typedef enum {
    FIRESTAFF_X68K_RECEIPT_CLASS_UNPROTECTED_PUBLIC_HDM = 0,
    FIRESTAFF_X68K_RECEIPT_CLASS_BLANK_SAVE_DISK        = 1
} FirestaffX68kMediaReceipt_Class;

typedef struct {
    FirestaffX68kMediaReceipt_Kind  kind;
    const char                     *label;          /* human label */
    const char                     *md5;            /* 32-char hex */
    const char                     *sha256;         /* 64-char hex */
    size_t                          size_bytes;
    FirestaffX68kMediaReceipt_Class expected_class;
    /* Flags the classifier MUST report for an "expected_match"
     * receipt. The check is:
     *   (actual_flags & expected_flags_mask) == expected_flags_value
     * Unused flag bits are ignored. */
    uint32_t                        expected_flags_mask;
    uint32_t                        expected_flags_value;
} FirestaffX68kMediaReceipt_KnownHash;

const FirestaffX68kMediaReceipt_KnownHash *
firestaff_x68k_media_receipt_known_hashes(size_t *out_count);

/* ── Result codes ───────────────────────────────────────────────── */

typedef enum {
    FIRESTAFF_X68K_RECEIPT_OK              = 0,
    FIRESTAFF_X68K_RECEIPT_ERR_ARGUMENT    = -1,
    FIRESTAFF_X68K_RECEIPT_ERR_NO_DATA_DIR = -2,
    FIRESTAFF_X68K_RECEIPT_ERR_NOT_FOUND   = -3,  /* single-kind scan */
    FIRESTAFF_X68K_RECEIPT_ERR_READ        = -4,
    FIRESTAFF_X68K_RECEIPT_ERR_SIZE_MISMATCH = -5,
    FIRESTAFF_X68K_RECEIPT_ERR_HASH_MISMATCH = -6,
    FIRESTAFF_X68K_RECEIPT_ERR_CLASS_UNEXPECTED = -7
} FirestaffX68kMediaReceipt_Result;

/* ── Per-HDM receipt ────────────────────────────────────────────── */

#define FIRESTAFF_X68K_RECEIPT_PATH_CAP   512
#define FIRESTAFF_X68K_RECEIPT_HASH_CAP    65  /* 64 hex + NUL */

typedef struct {
    FirestaffX68kMediaReceipt_Kind  kind;
    char                             label[FIRESTAFF_X68K_RECEIPT_PATH_CAP];
    char                             expected_md5[FIRESTAFF_X68K_RECEIPT_HASH_CAP];
    char                             expected_sha256[FIRESTAFF_X68K_RECEIPT_HASH_CAP];
    size_t                           expected_size_bytes;

    /* Resolved path returned by the asset scanner. May be a
     * virtual container path (e.g. "set1.zip::*.hdm") for
     * ZIP-packaged HDMs. */
    char                             resolved_path[FIRESTAFF_X68K_RECEIPT_PATH_CAP];

    /* MD5 / SHA-256 actually computed over the input. */
    char                             actual_md5[FIRESTAFF_X68K_RECEIPT_HASH_CAP];
    char                             actual_sha256[FIRESTAFF_X68K_RECEIPT_HASH_CAP];
    size_t                           actual_size_bytes;

    /* The X68k media classifier output for this input. */
    uint32_t                         media_class;
    uint32_t                         flags;
    uint32_t                         bytes_per_sector;
    uint64_t                         sentinel_offset;
    int                              has_ftl_magic;
    uint32_t                         ftl_magic_candidate_count;

    /* FTL handoff: an FTL-declared data_area1_memory_size
     * that "fits" inside the on-disk HDM. We use the
     * documented X68k full-disk size (1232 KB) as the
     * comparison target since the FTL resources sit inside
     * the HDM, and we report the verdict through
     * FirestaffX68kMedia_FTLHandoffFits so the result is
     * reproducible against the synthetic-only test. */
    int                              ftl_handoff_fits_full_disk;
    int                              ftl_handoff_fits_single_side;

    /* The DMWeb-documented expected class for this kind and
     * whether the receipt matches it. */
    FirestaffX68kMediaReceipt_Class  expected_class;
    int                              expected_class_match;

    /* Top-level result code for this single receipt. OK when
     * the input exists and every documented invariant holds;
     * one of the negative FIRESTAFF_X68K_RECEIPT_ERR_* values
     * otherwise. */
    int                              result;

    /* 1 when the asset scanner matched this kind (i.e. the
     * file is present in the data tree). 0 means the kind is
     * not present on this host; the probe treats that as a
     * SKIP for that one row, not a failure. */
    int                              present;
} FirestaffX68kMediaReceipt;

/* ── Receipt lifecycle ──────────────────────────────────────────── */

void firestaff_x68k_media_receipt_init(FirestaffX68kMediaReceipt *r);

/* Compute the SHA-256 hex string of the bytes at `data` of
 * `data_size` bytes, writing a 64-char NUL-terminated hex
 * string to `out_hex` (capacity >= 65 bytes). The
 * implementation is a self-contained public-domain SHA-256
 * (no OpenSSL / CommonCrypto dependency) so the receipt
 * module stays portable. Returns 0 on success, -1 on
 * argument error. */
int firestaff_x68k_media_receipt_sha256_hex(const uint8_t *data,
                                            size_t data_size,
                                            char *out_hex,
                                            size_t out_hex_cap);

/* Compute the MD5 hex string of the bytes at `data` of
 * `data_size` bytes, writing a 32-char NUL-terminated hex
 * string to `out_hex` (capacity >= 33 bytes). Same
 * self-contained rationale as SHA-256. */
int firestaff_x68k_media_receipt_md5_hex(const uint8_t *data,
                                          size_t data_size,
                                          char *out_hex,
                                          size_t out_hex_cap);

/* Read `path` into a freshly malloc'd buffer, populate the
 * receipt's actual_md5 / actual_sha256 / actual_size_bytes,
 * and run the X68k media classifier on the buffer. The
 * buffer is owned by the caller (free with free()) and is
 * returned through *out_data / *out_data_size on success.
 *
 * The receipt is not modified before this call. After it
 * returns, the caller is expected to compare the actual
 * hash + size against the kind's expected values, then call
 * firestaff_x68k_media_receipt_finalize() to set the result
 * code and the expected_class_match flag.
 *
 * Returns FIRESTAFF_X68K_RECEIPT_OK on a successful read
 * and classification, or a negative error code otherwise.
 * On error, *out_data is set to NULL and *out_data_size to 0. */
int firestaff_x68k_media_receipt_ingest_path(
    FirestaffX68kMediaReceipt *r,
    const char *path,
    uint8_t **out_data,
    size_t   *out_data_size);

/* Set the receipt's result code based on the documented
 * expected values. The caller has already populated actual_*
 * fields via _ingest_path() (or by hand). After this call the
 * receipt's `result` is:
 *   OK
 *     when the MD5, SHA-256, size, and classifier verdict
 *     all match the documented expected values;
 *   ERR_HASH_MISMATCH
 *     when MD5 or SHA-256 differs;
 *   ERR_SIZE_MISMATCH
 *     when the actual byte count differs;
 *   ERR_CLASS_UNEXPECTED
 *     when the X68k classifier verdict does not match the
 *     DMWeb-documented expected class for the kind.
 * The `expected_class_match` flag is set to 1 in the OK
 * case and 0 in the ERR_CLASS_UNEXPECTED case. */
int firestaff_x68k_media_receipt_finalize(
    FirestaffX68kMediaReceipt *r);

/* Resolve the asset cache dir. Returns:
 *   - the explicit cache_dir when non-NULL and non-empty;
 *   - the user's "$HOME/.firestaff/asset-cache/x68k-receipt"
 *     directory otherwise.
 * The output buffer must be at least FIRESTAFF_X68K_RECEIPT_PATH_CAP
 * bytes; the resolved path is NUL-terminated on success. */
int firestaff_x68k_media_receipt_resolve_cache_dir(
    const char *cache_dir,
    char *out_dir,
    size_t out_dir_cap);

/* Scan `data_dir` recursively for one file matching the
 * known MD5 for `kind`. On match, materialize the file
 * (if it is a virtual container path) into `cache_dir` and
 * populate `*receipt`. The receipt's `present` is set to 1
 * on a successful match; the caller still has to call
 * firestaff_x68k_media_receipt_finalize() to lock in the
 * per-kind expected invariants.
 *
 * Returns:
 *   OK                       on a successful scan + ingest
 *   ERR_NOT_FOUND            when no known file is present
 *                            (callers may treat this as SKIP)
 *   ERR_ARGUMENT             on bad arguments
 *   ERR_NO_DATA_DIR          when data_dir is NULL/empty and
 *                            no usable default is available
 *   ERR_READ                 when file I/O fails
 *
 * The receipt's `result` and `expected_class_match` are
 * populated by this call (it invokes _finalize internally),
 * so callers can treat a returned OK as "this kind's
 * receipt matches DMWeb documentation". */
int firestaff_x68k_media_receipt_scan_one(
    FirestaffX68kMediaReceipt_Kind kind,
    const char *data_dir,
    const char *cache_dir,
    int max_depth,
    FirestaffX68kMediaReceipt *receipt);

/* Scan for every kind in the known-hash list. Each known
 * kind is independent: missing kinds are reported as
 * present == 0 and do not count as a failure. The total
 * count of present kinds is written to *out_present_count.
 *
 * Returns OK when at least one kind was scanned, or
 * ERR_NOT_FOUND when none of the known kinds is present on
 * this host. Other error codes propagate from _scan_one. */
int firestaff_x68k_media_receipt_scan_all(
    const char *data_dir,
    const char *cache_dir,
    int max_depth,
    FirestaffX68kMediaReceipt *receipts,
    size_t receipts_capacity,
    size_t *out_present_count);

/* Write a multi-line diagnostic / receipt report for `r`
 * into `out_buf` of capacity `out_buf_cap`. Always
 * NUL-terminates. The format is plain ASCII with one
 * field per line so it is easy to grep / diff / paste into
 * docs. Returns the number of bytes written (excluding the
 * trailing NUL), or -1 on argument error. */
int firestaff_x68k_media_receipt_write_report(
    const FirestaffX68kMediaReceipt *r,
    char *out_buf,
    size_t out_buf_cap);

/* ── Result-name tables ─────────────────────────────────────────── */

const char *firestaff_x68k_media_receipt_kind_name(
    FirestaffX68kMediaReceipt_Kind kind);
const char *firestaff_x68k_media_receipt_class_name(
    FirestaffX68kMediaReceipt_Class cls);
const char *firestaff_x68k_media_receipt_result_name(int result);
const char *firestaff_x68k_media_receipt_media_class_name(uint32_t media_class);
const char *firestaff_x68k_media_receipt_flag_name(uint32_t flag);

/* ── Self-test (data-free, always CTest-gated) ──────────────────── */

/* Bounded, data-free self-test. Builds a synthetic 1232 KB
 * buffer with a known byte pattern, exercises the
 * classifier + receipt finalizer on it, and verifies the
 * MD5 / SHA-256 helpers against the documented RFC 1321
 * MD5 "abc" / FIPS 180-4 SHA-256 "abc" test vectors.
 *
 * Returns 0 on success, -1 on first failed invariant. */
int firestaff_x68k_media_receipt_self_test(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_X68K_MEDIA_RECEIPT_H */
