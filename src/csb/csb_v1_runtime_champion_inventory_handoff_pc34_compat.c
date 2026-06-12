#include "csb_v1_runtime_champion_inventory_handoff_pc34_compat.h"

#include <string.h>

#define CSB_V1_THING_NONE_PC34_COMPAT 0xFFFFu
#define CSB_V1_MASK_LOAD_PC34_COMPAT  0x0200u

static uint64_t fnv1a_u64(uint64_t h, uint64_t v)
{
    int i;
    for (i = 0; i < 8; i++) {
        h ^= (uint8_t)((v >> (unsigned)(i * 8)) & 0xFFu);
        h *= 1099511628211ULL;
    }
    return h;
}

const char *
csb_v1_runtime_champion_inventory_handoff_source_evidence_pc34_compat(void)
{
    return
        "ReDMCSB CHAMPION.C F0297 lines 243-266: put object in leader hand, update leader load flag\n"
        "ReDMCSB CHAMPION.C F0298 lines 270-295: remove object from leader hand, update leader load flag\n"
        "ReDMCSB CHAMPION.C F0302 lines 662-710: slot-box command swaps leader hand and champion slot\n"
        "ReDMCSB COMMAND.C F0380 lines 1964-1986 and 2175-2176: command dispatch reaches F0302\n"
        "ReDMCSB LOADSAVE.C F0435 lines 2733-2744: load restores leader index and leader hand object\n"
        "ReDMCSB DEFS.H lines 541, 547, 5324, 5861-5863, 7921-7946: global-data/slot/leader-hand declarations";
}

uint64_t csb_v1_runtime_champion_inventory_handoff_hash_pc34_compat(
    const CSB_V1_RuntimeProfile *profile)
{
    uint64_t h = 1469598103934665603ULL;
    int i;
    int slot;

    if (!profile || !profile->party_state_valid) {
        return fnv1a_u64(h, 0xC5B0u);
    }

    h = fnv1a_u64(h, (uint64_t)(uint32_t)profile->party_state.ChampionCount);
    h = fnv1a_u64(h, (uint64_t)(uint32_t)profile->party_state.ImportedFromDM1);
    h = fnv1a_u64(h, (uint64_t)(uint32_t)profile->party_state.LeaderIndex);
    h = fnv1a_u64(h, (uint64_t)(uint32_t)profile->leader_index);
    h = fnv1a_u64(h, (uint64_t)profile->tick_count);
    h = fnv1a_u64(h, (uint64_t)profile->game_time);

    for (i = 0; i < profile->party_state.ChampionCount &&
                i < CSB_V1_MAX_CHAMPIONS; i++) {
        const CSB_V1_Champion *c = &profile->party_state.Champions[i];
        h = fnv1a_u64(h, (uint64_t)(uint16_t)c->CurrentHealth);
        h = fnv1a_u64(h, (uint64_t)(uint16_t)c->CurrentStamina);
        h = fnv1a_u64(h, (uint64_t)c->Attributes);
        h = fnv1a_u64(h, (uint64_t)c->Load);
        h = fnv1a_u64(h, (uint64_t)c->Cell);
        h = fnv1a_u64(h, (uint64_t)c->Direction);
        for (slot = 0; slot < CSB_V1_SLOT_COUNT; slot++) {
            h = fnv1a_u64(h, (uint64_t)c->Slots[slot]);
        }
    }

    return h;
}

int csb_v1_runtime_champion_inventory_handoff_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int slot_index,
    uint16_t leader_hand_thing,
    CSB_V1_RuntimeChampionInventoryHandoffResultPc34Compat *out_result)
{
    CSB_V1_Champion *champion;
    uint16_t slot_thing;

    if (out_result) {
        memset(out_result, 0, sizeof(*out_result));
        out_result->champion_index = champion_index;
        out_result->slot_index = slot_index;
        out_result->leader_hand_before = leader_hand_thing;
        out_result->leader_hand_after = CSB_V1_THING_NONE_PC34_COMPAT;
    }

    if (!profile || !profile->party_state_valid) {
        return -1;
    }
    if (champion_index < 0 ||
        champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS ||
        slot_index < 0 ||
        slot_index >= CSB_V1_SLOT_COUNT) {
        return -1;
    }

    champion = &profile->party_state.Champions[champion_index];
    if (csb_v1_champion_is_dead(champion) || champion->CurrentHealth <= 0) {
        return -1;
    }

    slot_thing = champion->Slots[slot_index];
    if (slot_thing == CSB_V1_THING_NONE_PC34_COMPAT &&
        leader_hand_thing == CSB_V1_THING_NONE_PC34_COMPAT) {
        return 0;
    }

    if (out_result) {
        out_result->leader_index_before = profile->leader_index;
        out_result->leader_index_after = profile->leader_index;
        out_result->slot_thing_before = slot_thing;
        out_result->slot_thing_after = leader_hand_thing;
        out_result->leader_hand_after = slot_thing;
        out_result->attributes_before = champion->Attributes;
        out_result->load_before = champion->Load;
        out_result->imported_from_dm1 = profile->party_state.ImportedFromDM1;
        out_result->party_state_valid = profile->party_state_valid;
    }

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 snapshots
     * G4055_s_LeaderHandObject.Thing, removes it through F0298 when present,
     * removes the clicked slot through F0300 when present, puts that slot
     * object back in the transient leader hand through F0297, then writes the
     * original leader-hand object to the champion slot through F0301.  This
     * narrow CSB runtime helper applies only the imported-party slot mutation;
     * the returned leader_hand_after models the transient G4055 hand object.
     * F0297/F0298 lines 263-295 set MASK0x0200_LOAD for the current leader. */
    champion->Slots[slot_index] = leader_hand_thing;
    champion->Attributes |= CSB_V1_MASK_LOAD_PC34_COMPAT;

    if (out_result) {
        out_result->attributes_after = champion->Attributes;
        out_result->load_after = champion->Load;
        out_result->state_hash =
            csb_v1_runtime_champion_inventory_handoff_hash_pc34_compat(profile);
    }

    return 1;
}
