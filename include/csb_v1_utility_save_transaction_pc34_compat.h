#ifndef FIRESTAFF_CSB_V1_UTILITY_SAVE_TRANSACTION_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_UTILITY_SAVE_TRANSACTION_PC34_COMPAT_H

/*
 * CSB Utility Disk save-to-runtime transaction.
 *
 * CEDT's Load Saved Game route must not install a partly decoded roster.
 * This contract-only boundary accepts a Firestaff FSSB envelope around a
 * reconstructed CSBGAME-shaped party buffer, decodes it through the
 * production CSBGAME reader into a local candidate, validates the candidate's
 * champion ownership, and publishes it to the live profile as one final
 * operation. It is deliberately not linked into the production archive:
 * original CSBGAME/CSBWin files use their source-owned resume routes.
 *
 * Sources: ReDMCSB CEDTINC8.C, LOADSAVE.C F0435, CHAMPION.C F0297-F0302;
 * CSBWin SaveGame.cpp LoadGame/SaveGame.
 */

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_runtime_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_UTILITY_SAVE_TRANSACTION_OK = 0,
    CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_NULL = -1,
    CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_ENVELOPE = -2,
    CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_IMPORT = -3,
    CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_PARTY = -4,
    CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_RUNTIME = -5
} CSB_V1_UtilitySaveTransactionResult;

typedef struct {
    int result;
    int candidate_champion_count;
    int committed_leader_index;
    uint16_t payload_kind;
    uint64_t runtime_hash_before;
    uint64_t runtime_hash_after;
    int committed;
    char source_path[64];
} CSB_V1_UtilitySaveTransactionReceipt;

/* Contract-only test helper. Applies a checksum-valid Firestaff FSSB envelope
 * to a live CSB profile. No default champions, inventory objects, EXPOOL
 * records, or UI state are created by this API. A rejected envelope leaves
 * `profile` byte unchanged. Returns the imported champion count, or a
 * negative result. */
int csb_v1_utility_save_transaction_commit_runtime_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    const uint8_t *envelope,
    size_t envelope_size,
    CSB_V1_UtilitySaveTransactionReceipt *out_receipt);

const char *csb_v1_utility_save_transaction_source_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
