/*
 * csb_v1_startup_real_asset_receipt.h
 *
 * CSB V1 startup real-asset receipt.
 *
 * A "receipt" here is a small, deterministic, hash-bound summary of
 * the assets a real CSB V1 boot would consume from the user's
 * data tree. It is intentionally narrow: paths, sizes, MD5 hashes,
 * variant id, archive kind, and a deterministic FNV-1a receipt
 * hash computed over those fields. The receipt is the
 * "real-asset side" of the CSB startup handoff; the existing
 * `csb_v1_pc_real_asset_launch` probe is the "boot handoff" side.
 *
 * Skip-safe by design. When no verified CSB pair is staged the
 * receipt stays cleared (md5 paths empty, file_size == 0,
 * receipt_hash == 0) and the public scan function returns
 * CSB_V1_STARTUP_REAL_OK with `matched == 0` so callers can
 * branch on the empty receipt without crashing.
 *
 * Source-lock boundary:
 *   - ReDMCSB ENTRANCE.C F0806 lines 409-441 (CSB entrance setup,
 *     C28_ENTRANCE_CSB palette selection).
 *   - ReDMCSB DUNGEON.C F0237 (hash-verified DUNGEON.DAT load).
 *   - ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755 (heap-allocated
 *     dungeon ownership transferred into the runtime).
 *   - ReDMCSB LOADSAVE.C F0435 lines 1936-1944 (new-game map 0).
 *   - dmweb Data Files page: CSB dungeon hash is shared across all
 *     platforms; graphics hash + archive kind pick the variant.
 *   - docs/VERIFIED_HASHES.md: source-cited hash list.
 *
 * See also:
 *   - include/csb_v1_boot.h (the boot/profile boundary this
 *     receipt layer sits above).
 *   - include/csb_v1_runtime_pc34_compat.h (the runtime handoff
 *     target the boot profile ultimately enters).
 *   - probes/csb/firestaff_csb_v1_pc_real_asset_launch_probe.c
 *     (the live-boot companion; this receipt is the
 *     pre-boot metadata handoff).
 */

#ifndef FIRESTAFF_CSB_V1_STARTUP_REAL_ASSET_RECEIPT_H
#define FIRESTAFF_CSB_V1_STARTUP_REAL_ASSET_RECEIPT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_runtime_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Result codes ─────────────────────────────────────────────────── */

typedef enum {
    CSB_V1_STARTUP_REAL_OK = 0,
    CSB_V1_STARTUP_REAL_ERR_ARGUMENT = -1,
    CSB_V1_STARTUP_REAL_ERR_NO_DATA_DIR = -2,
    CSB_V1_STARTUP_REAL_ERR_BOOT_SCAN = -3
} CSB_V1_StartupRealResult;

/* ── Receipt size constants ───────────────────────────────────────── */

/*
 * Buffers are sized to match the CSB V1 boot profile so the
 * receipt lines up with `CSB_V1_BootProfile` without forcing
 * callers to copy bytes between two profiles. 512 bytes is the
 * Firestaff asset_find_by_hash path cap (`ASSET_PATH_MAX`).
 */
#define CSB_V1_STARTUP_REAL_PATH_CAP 512
#define CSB_V1_STARTUP_REAL_MD5_CAP  33
#define CSB_V1_STARTUP_REAL_HASH_HEX_CAP 17 /* 64-bit FNV-1a + NUL */

/* ── Receipt structure ────────────────────────────────────────────── */

/*
 * One real-asset receipt. `matched` is 1 when both
 * graphics + dungeon paths were found by hash, 0 otherwise.
 * The receipt is "valid" (deterministic hash meaningful) only
 * when `matched == 1`. The empty receipt is itself a valid
 * skip-safe state.
 *
 * Field order is stable. The FNV-1a receipt hash folds
 * graphics_path, dungeon_path, graphics_md5, dungeon_md5,
 * file sizes, variant id, archive kind, and a magic salt so
 * the hash is never 0 for a matched receipt.
 */
typedef struct {
    /* Resolved on-disk paths. Empty when no match. */
    char graphics_path[CSB_V1_STARTUP_REAL_PATH_CAP];
    char dungeon_path[CSB_V1_STARTUP_REAL_PATH_CAP];

    /* 32-char hex MD5 hashes (the matched-known hash for the
     * graphics + dungeon, not a recomputed file digest). Empty
     * when no match. */
    char graphics_md5[CSB_V1_STARTUP_REAL_MD5_CAP];
    char dungeon_md5[CSB_V1_STARTUP_REAL_MD5_CAP];

    /* File sizes in bytes. 0 when no match. */
    uint64_t graphics_size_bytes;
    uint64_t dungeon_size_bytes;

    /* Variant + archive kind. UNKNOWN / NONE when no match. */
    CSB_V1_VariantId variant_id;
    CSB_V1_AssetGfxArchiveType graphics_kind;

    /* Hash discovery depth used by the scan (passed through for
     * diagnostics; default 4 in csb_v1_boot_scan_assets). */
    int max_depth;

    /* The boot profile's assets_verified bit, copied through so
     * callers can confirm both halves of the boot pair are
     * present without a second scan. */
    int assets_verified;
    int graphics_verified;
    int dungeon_verified;

    /* 1 when both graphics + dungeon hashes were matched and
     * paths/sizes populated. 0 when no known CSB pair is staged
     * under data_dir (skip-safe empty receipt). */
    int matched;

    /* FNV-1a 64-bit hash folded over the metadata above + a
     * salt. 0 only for an empty (unmatched) receipt. 16-char hex
     * NUL-terminated string. */
    uint64_t receipt_hash;
    char    receipt_hash_hex[CSB_V1_STARTUP_REAL_HASH_HEX_CAP];

    /* asset_root from the boot profile (data_dir that was
     * scanned). Empty when no match. */
    char asset_root[CSB_V1_STARTUP_REAL_PATH_CAP];
} CSB_V1_StartupRealReceipt;

/* Initialize an empty (skip-safe) receipt. */
void csb_v1_startup_real_receipt_init(CSB_V1_StartupRealReceipt *receipt);

/*
 * Scan `data_dir` recursively and build a real-asset receipt.
 *
 * The scan reuses `csb_v1_boot_scan_assets()` to:
 *   1. Find CSB DUNGEON.DAT by the shared CSB dungeon hash
 *      (6695d2acebce49f95db1d8f3a5c733de).
 *   2. Find CSB GRAPHICS.DAT (or CSB.DAT / CSBGRAPH.DAT) by the
 *      known per-variant graphics hashes.
 *   3. Detect the variant id from the matched graphics hash.
 *   4. Compute the file sizes by stat().
 *   5. Fold all of the above into a deterministic FNV-1a 64-bit
 *      receipt hash.
 *
 * On a successful matched receipt, `receipt->matched == 1` and
 * the receipt hash is non-zero. When no known CSB pair is
 * staged, `receipt->matched == 0` and the receipt stays
 * cleared; the function still returns
 * CSB_V1_STARTUP_REAL_OK so the skip-safe path is the
 * default.
 *
 * On argument / boot-scan failure (only when the boot scan
 * itself errored out for non-SKIP reasons), a negative
 * CSB_V1_StartupRealResult is returned and the receipt is
 * reset to the empty state.
 */
int csb_v1_startup_real_scan_and_receipt(const char *data_dir,
                                          int max_depth,
                                          CSB_V1_StartupRealReceipt *receipt);

/*
 * Recompute the FNV-1a receipt hash in place over an already
 * populated receipt. Used by the probe to prove that two
 * independent builds of the same metadata produce the same
 * hash (determinism gate). Returns 1 when the hash matches the
 * stored hex value (or when the receipt is empty), 0 when
 * the stored hash does not match a fresh recompute.
 */
int csb_v1_startup_real_receipt_recompute_hash(
    CSB_V1_StartupRealReceipt *receipt);

/*
 * Return a stable, NUL-terminated string for a result code.
 */
const char *csb_v1_startup_real_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_STARTUP_REAL_ASSET_RECEIPT_H */
