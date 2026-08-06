#include "csb_v1_utility_save_transaction_pc34_compat.h"

#include "csb_v1_runtime_champion_inventory_handoff_pc34_compat.h"
#include "csb_v1_save_export_import_pc34_compat.h"

#include <string.h>

/* This compatibility transaction is intentionally compiled only by its
 * focused contract target. ReDMCSB LOADSAVE.C F0435 stages native save bytes,
 * whereas FSSB is a Firestaff test envelope with no original-media owner. */

static int csb_v1_utility_party_is_publishable(
    const CSB_V1_PartyState *party)
{
    int i;

    if (!party || party->ChampionCount < 1 ||
        party->ChampionCount > CSB_V1_MAX_CHAMPIONS ||
        party->LeaderIndex < 0 || party->LeaderIndex >= party->ChampionCount) {
        return 0;
    }
    for (i = 0; i < party->ChampionCount; ++i) {
        const CSB_V1_Champion *champion = &party->Champions[i];
        if (champion->Name[0] == '\0' || champion->MaximumHealth <= 0 ||
            champion->MaximumStamina <= 0 || champion->MaximumMana < 0) {
            return 0;
        }
    }
    return 1;
}

const char *csb_v1_utility_save_transaction_source_evidence_pc34_compat(void)
{
    return
        "ReDMCSB CEDTINC8.C: CSBGAME utility-disk save routing\n"
        "ReDMCSB LOADSAVE.C F0435: stage and validate save before live state\n"
        "ReDMCSB CHAMPION.C F0297-F0302: leader/slot ownership remains runtime-owned\n"
        "CSBWin SaveGame.cpp: LoadGame/SaveGame CSBGAME ownership\n";
}

int csb_v1_utility_save_transaction_commit_runtime_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    const uint8_t *envelope,
    size_t envelope_size,
    CSB_V1_UtilitySaveTransactionReceipt *out_receipt)
{
    CSB_V1_SaveExportHeader header;
    CSB_V1_PartyState candidate;
    int imported;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->result = CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_NULL;
    }
    if (!profile || !envelope) {
        return CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_NULL;
    }
    if (out_receipt) {
        out_receipt->runtime_hash_before =
            csb_v1_runtime_champion_inventory_handoff_hash_pc34_compat(profile);
    }
    if (csb_v1_save_export_validate_importable_envelope(envelope,
                                                         envelope_size) !=
            CSB_V1_SAVE_EXPORT_OK ||
        csb_v1_save_export_parse_header(envelope, envelope_size, &header) !=
            CSB_V1_SAVE_EXPORT_OK) {
        if (out_receipt) out_receipt->result =
            CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_ENVELOPE;
        return CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_ENVELOPE;
    }

    memset(&candidate, 0, sizeof(candidate));
    imported = csb_v1_save_export_import_envelope(&candidate, envelope,
                                                   envelope_size);
    if (imported < 1) {
        if (out_receipt) out_receipt->result =
            CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_IMPORT;
        return CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_IMPORT;
    }
    if (!csb_v1_utility_party_is_publishable(&candidate)) {
        if (out_receipt) out_receipt->result =
            CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_PARTY;
        return CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_PARTY;
    }

    /* `candidate` is fully validated before the one runtime mutation. */
    if (csb_v1_runtime_set_party_state(profile, &candidate) != 0) {
        if (out_receipt) out_receipt->result =
            CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_RUNTIME;
        return CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_RUNTIME;
    }
    if (out_receipt) {
        out_receipt->result = CSB_V1_UTILITY_SAVE_TRANSACTION_OK;
        out_receipt->candidate_champion_count = imported;
        out_receipt->committed_leader_index = profile->leader_index;
        out_receipt->payload_kind = header.payload_kind;
        memcpy(out_receipt->source_path, header.source_path,
               sizeof(out_receipt->source_path) - 1u);
        out_receipt->runtime_hash_after =
            csb_v1_runtime_champion_inventory_handoff_hash_pc34_compat(profile);
        out_receipt->committed = 1;
    }
    return imported;
}
