/*
 * dm1_v22_finished_pack_receipt_pc34.h
 *
 * DM1 V2.2 finished-art pack reviewer-receipt promotion gate.
 *
 * Companion to dm1_v22_finished_art_material_gate_pc34.h. The
 * material gate classifies a V2.2 modern_asset_manifest.json as
 * NO_MANIFEST / SYNTHETIC_PLACEHOLDER / PARTIAL / FINISHED_REAL.
 * FINISHED_REAL is reachable purely from on-disk evidence (every
 * tracked slot has generator != "placeholder" and source_file
 * resolves on disk), but that proves nothing about who reviewed
 * the artwork.
 *
 * This header introduces a parallel "reviewer-receipt" layer that
 * an operator (the Firestaff maintainer or a designated reviewer)
 * can drop on disk to record "I have looked at the manifest
 * described by this hash, and I signed off on every required
 * slot". The receipt is a small JSON file:
 *
 *   ~/.firestaff/assets/dm1/modern/finish_receipt.json
 *
 * with the following shape:
 *
 *   {
 *     "receiptVersion": "1.0.0",
 *     "manifestPath": "<canonical manifest path>",
 *     "manifestHashFnv1a": "<8-hex-digit uint32>",
 *     "reviewer": "<operator handle, free-form>",
 *     "reviewedAtUtc": "<ISO-8601 timestamp>",
 *     "gateTarget":   "FINISHED_REAL",
 *     "reviewedSlots": [
 *       "wall_d3_carved_hero_01",
 *       "floor_plain_hero_01",
 *       ...
 *     ],
 *     "notes": "<optional, free-form>"
 *   }
 *
 * The receipt state machine:
 *
 *   NOT_INSTALLED          — no receipt file present (the CI-safe
 *                             default; the existing FINISHED_REAL
 *                             gate flow is unchanged). This is the
 *                             "skip-safe" path the gap-list row
 *                             calls out.
 *   INSTALLED_UNVERIFIED   — receipt present but state() not yet
 *                             called; same posture as NOT_INSTALLED
 *                             for promotion purposes.
 *   MALFORMED              — receipt present but missing required
 *                             fields or not valid JSON object
 *   STALE                  — receipt present but its manifestHashFnv1a
 *                             does not match the on-disk manifest
 *                             file content (reviewer signed off on a
 *                             previous revision)
 *   MATERIAL_NOT_REAL      — receipt matches manifest hash but the
 *                             material gate is not FINISHED_REAL
 *                             (some slot regressed to PLACEHOLDER or
 *                             PARTIAL after the reviewer signed off)
 *   MATCH_PARTIAL          — receipt matches manifest hash, material
 *                             gate is FINISHED_REAL, but the
 *                             reviewedSlots list does not cover every
 *                             required slot
 *   MATCH_FINISHED_REAL    — receipt matches, material gate is
 *                             FINISHED_REAL, every required slot is
 *                             listed in reviewedSlots
 *
 * is_promoted() returns 1 only for MATCH_FINISHED_REAL. All other
 * states preserve the existing FINISHED_REAL gate flow (i.e. the
 * material gate still reports its honest classification; the
 * receipt never overrides material evidence).
 *
 * Source-lock:
 *   - ReDMCSB DUNVIEW.C:6697-6816 (DM1 viewport composition order)
 *   - ReDMCSB DUNGEON.C:2238-2246 (square-type decode feeding
 *     m11_v22_shape_for_cell)
 *   - include/dm1_v22_finished_art_material_gate_pc34.h (sibling
 *     material gate we cross-check against)
 *   - include/dm1_v2_asset_pipeline_pc34.h (modern asset root
 *     resolution)
 *
 * Honest boundary: the receipt is an advisory review note. We do
 * NOT perform crypto/PKI verification of the reviewer handle; the
 * integrity model is "the manifest content hash the reviewer
 * signed off on still matches what's on disk". If a malicious
 * operator rewrites both the receipt and the manifest together
 * to claim promotion, that is a Firestaff workflow integrity
 * problem, not a cryptographically-soluble one. The gate's
 * value is to make the promotion a deliberate, single-file edit
 * the operator must stamp when finished art is shipped, instead
 * of relying on inspection of every CI run.
 */

#ifndef FIRESTAFF_DM1_V22_FINISHED_PACK_RECEIPT_PC34_H
#define FIRESTAFF_DM1_V22_FINISHED_PACK_RECEIPT_PC34_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Receipt gate state ────────────────────────────────────────── */
typedef enum {
    DM1_V22_FPR_NOT_INSTALLED         = 0,
    DM1_V22_FPR_INSTALLED_UNVERIFIED  = 1,
    DM1_V22_FPR_MALFORMED             = 2,
    DM1_V22_FPR_STALE                 = 3,
    DM1_V22_FPR_MATERIAL_NOT_REAL     = 4,
    DM1_V22_FPR_MATCH_PARTIAL         = 5,
    DM1_V22_FPR_MATCH_FINISHED_REAL   = 6
} DM1_V22_FprState;

/* ── Public API ────────────────────────────────────────────────── */

/* Set the modern asset root directory. Mirrors
 * dm1_v22_famg_set_manifest_path: dataDir is the DM1 game data dir
 * (e.g. ~/.firestaff/data/dm1); the receipt is read from
 * ~/.firestaff/assets/dm1/modern/finish_receipt.json by walking up
 * two parents from dataDir and appending assets/dm1/modern/
 * finish_receipt.json.
 *
 * Passing NULL or "" clears the stored receipt path. */
void dm1_v22_fpr_set_receipt_path(const char* dataDir);

/* Returns the resolved receipt path (read-only). May be empty if the
 * path has not been set. */
const char* dm1_v22_fpr_get_receipt_path(void);

/* Returns the modern asset manifest path the gate cross-checks
 * against. Mirrors dm1_v22_famg_get_manifest_path() so callers can
 * surface both paths in diagnostic output. */
const char* dm1_v22_fpr_get_manifest_path(void);

/* Compute the receipt state. Reads the receipt file (when present)
 * and the modern asset manifest, hashes them with FNV-1a, and
 * cross-checks the reviewed slot list against the sibling material
 * gate's required slots. Cached after the first invocation; call
 * dm1_v22_fpr_reset_state() to force re-computation. */
DM1_V22_FprState dm1_v22_fpr_state(void);

/* Force the next dm1_v22_fpr_state() call to re-read the receipt
 * file from disk. Used by the probe/test harness after rewriting
 * fixtures. */
void dm1_v22_fpr_reset_state(void);

/* Returns 1 only when:
 *   - dm1_v22_fpr_state() == DM1_V22_FPR_MATCH_FINISHED_REAL
 *   - dm1_v22_famg_is_finished_real() == 1
 * Mirrors the sibling gate's contract. */
int dm1_v22_fpr_is_promoted(void);

/* Returns 1 if the receipt file is on disk at the resolved path
 * (regardless of its content). Useful for the probe diagnostic
 * frame so the operator can see whether the receipt was even
 * delivered. */
int dm1_v22_fpr_receipt_present(void);

/* Returns 1 if a receipt file is present AND its manifestHashFnv1a
 * matches the on-disk manifest content hash; -1 if no receipt file
 * is present; 0 if the hash did not match. The probe/test harness
 * surfaces this so reviewers can tell STALE from MATCH_*. */
int dm1_v22_fpr_receipt_hash_matches(void);

/* Counts how many of the gate's required slots are listed in the
 * receipt's reviewedSlots array. The int* out_required argument is
 * filled with the total required slot count from the sibling
 * material gate. Returns the reviewed count; 0 means
 * either the receipt had no reviewedSlots array, or the array did
 * not contain any required slot id. */
int dm1_v22_fpr_receipt_slot_count(int* out_required);

/* Counts how many required slots the receipt lists but whose
 * manifest entry is not classified as REAL by the material gate.
 * Used by the probe/test harness to confirm a MATCH_PARTIAL state
 * (some slots missing from the reviewer list AND some reviewed
 * slots regressed). */
int dm1_v22_fpr_receipt_stale_review_count(void);

/* Returns the human-readable state name (stable across runs). */
const char* dm1_v22_fpr_state_name(DM1_V22_FprState state);

/* FNV-1a 32-bit hash helpers. Exposed so the synthetic harness can
 * stamp receipts with the same hash the gate will compute. */
uint32_t dm1_v22_fpr_fnv1a_buf(const void* data, size_t len);
uint32_t dm1_v22_fpr_fnv1a_file(const char* path);

/* Source evidence citation for source-lock tests. */
const char* dm1_v22_fpr_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V22_FINISHED_PACK_RECEIPT_PC34_H */
