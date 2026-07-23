#include "csb_v1_utility_save_transaction_pc34_compat.h"
#include "csb_v1_runtime_champion_inventory_handoff_pc34_compat.h"
#include "csb_v1_save_export_import_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(c, m) do { if (!(c)) { ++failures; printf("FAIL: %s\n", m); } } while (0)

static void make_source_party(CSB_V1_PartyState *party)
{
    memset(party, 0, sizeof(*party));
    party->ChampionCount = 2;
    party->LeaderIndex = 0;
    party->PartyDirection = 3;
    party->LeaderHandThing = 0x1234u;
    party->MagicCasterIndex = -1;
    memcpy(party->Champions[0].Name, "TIGGY", 5);
    party->Champions[0].CurrentHealth = 77;
    party->Champions[0].MaximumHealth = 88;
    party->Champions[0].CurrentStamina = 99;
    party->Champions[0].MaximumStamina = 100;
    party->Champions[0].CurrentMana = 20;
    party->Champions[0].MaximumMana = 30;
    party->Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] = 0x2345u;
    memcpy(party->Champions[1].Name, "HALK", 4);
    party->Champions[1].CurrentHealth = 61;
    party->Champions[1].MaximumHealth = 70;
    party->Champions[1].CurrentStamina = 80;
    party->Champions[1].MaximumStamina = 90;
    party->Champions[1].CurrentMana = 11;
    party->Champions[1].MaximumMana = 22;
}

int main(void)
{
    CSB_V1_PartyState source;
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_UtilitySaveTransactionReceipt receipt;
    uint8_t envelope[CSB_V1_SAVE_EXPORT_MAX_ENVELOPE];
    long envelope_size;
    uint64_t before;

    make_source_party(&source);
    envelope_size = csb_v1_save_export_roundtrip(
        &source, CSB_SAVE_VERSION_V21, "/real/csbgame.dat", envelope,
        sizeof(envelope));
    CHECK(envelope_size > 0, "source-labelled CSBGAME envelope builds");
    csb_v1_runtime_init(&runtime, NULL);
    before = csb_v1_runtime_champion_inventory_handoff_hash_pc34_compat(&runtime);
    CHECK(csb_v1_utility_save_transaction_commit_runtime_pc34_compat(
              &runtime, envelope, (size_t)envelope_size, &receipt) == 2,
          "valid source envelope commits two champions");
    CHECK(receipt.committed && receipt.payload_kind == 1u,
          "receipt records one v2.1 commit");
    CHECK(strcmp(receipt.source_path, "/real/csbgame.dat") == 0,
          "receipt preserves source artifact path");
    CHECK(runtime.party_state_valid && runtime.party_state.ChampionCount == 2,
          "runtime receives complete candidate party");
    CHECK(strcmp(runtime.party_state.Champions[0].Name, "TIGGY") == 0,
          "runtime receives decoded champion identity");
    CHECK(runtime.party_state.LeaderHandThing == 0x1234u &&
              runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] ==
                  0x2345u &&
              runtime.party_state.PartyDirection == 3,
          "save transaction preserves leader hand and champion inventory ownership");
    CHECK(before != receipt.runtime_hash_after,
          "successful commit changes runtime ownership hash");

    /* Corruption must fail before the candidate reaches runtime. */
    before = csb_v1_runtime_champion_inventory_handoff_hash_pc34_compat(&runtime);
    envelope[CSB_V1_SAVE_EXPORT_HEADER_LEN + 8u] ^= 0x80u;
    CHECK(csb_v1_utility_save_transaction_commit_runtime_pc34_compat(
              &runtime, envelope, (size_t)envelope_size, &receipt) ==
              CSB_V1_UTILITY_SAVE_TRANSACTION_ERR_ENVELOPE,
          "CRC-invalid envelope rejects");
    CHECK(!receipt.committed && receipt.runtime_hash_before == before,
          "rejection records original runtime owner");
    CHECK(csb_v1_runtime_champion_inventory_handoff_hash_pc34_compat(&runtime) == before,
          "rejection leaves runtime exactly unchanged");

    if (failures) return 1;
    puts("PASS csb_v1_utility_save_transaction_pc34_compat");
    return 0;
}
