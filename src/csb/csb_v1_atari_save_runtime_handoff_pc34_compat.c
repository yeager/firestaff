#include "csb_v1_atari_save_runtime_handoff_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

const char *csb_v1_atari_save_runtime_handoff_source_evidence_pc34_compat(void)
{
    return
        "DMWeb Saved Game Files: MINI.DAT stores the CSB campaign as a save\n"
        "ReDMCSB LOADSAVE.C F0435: commit dungeon then restore party pose\n"
        "ReDMCSB CEDTINC8.C: Utility/Prison owns champion transfer\n";
}

int csb_v1_atari_save_handoff_runtime_pc34_compat(
    CSB_V1_RuntimeProfile *profile, const uint8_t *bytes, size_t size,
    CSB_V1_AtariSaveInfo *out_info)
{
    CSB_V1_DungeonData *candidate;
    CSB_V1_AtariSaveInfo info;
    CSB_V1_PartyState party;
    int result;

    if (!profile || !bytes) return CSB_V1_ATARI_RUNTIME_ERR_NULL;
    if (!profile->party_state_valid || profile->party_state.ChampionCount < 1 ||
        profile->party_state.ChampionCount > CSB_V1_MAX_CHAMPIONS) {
        return CSB_V1_ATARI_RUNTIME_ERR_PARTY;
    }
    candidate = (CSB_V1_DungeonData *)calloc(1u, sizeof(*candidate));
    if (!candidate) return CSB_V1_ATARI_RUNTIME_ERR_DUNGEON;
    result = csb_v1_atari_save_load_dungeon_pc34_compat(bytes, size,
                                                         candidate, &info);
    if (result != CSB_V1_ATARI_SAVE_OK || info.party_map_index < 0 ||
        info.party_map_index >= candidate->level_count || info.party_x < 0 ||
        info.party_y < 0 || info.party_x >= candidate->level_widths[info.party_map_index] ||
        info.party_y >= candidate->level_heights[info.party_map_index]) {
        csb_v1_dungeon_free(candidate);
        free(candidate);
        return result == CSB_V1_ATARI_SAVE_OK ? CSB_V1_ATARI_RUNTIME_ERR_POSE :
                                                 CSB_V1_ATARI_RUNTIME_ERR_DUNGEON;
    }

    /* All fallible validation happens before this one ownership transition. */
    party = profile->party_state;
    party.PartyMapX = info.party_x;
    party.PartyMapY = info.party_y;
    party.PartyDirection = info.party_direction & 3;
    party.LeaderHandThing = (uint16_t)info.leader_hand_thing;
    if (csb_v1_runtime_set_party_state(profile, &party) != 0) {
        csb_v1_dungeon_free(candidate);
        free(candidate);
        return CSB_V1_ATARI_RUNTIME_ERR_PARTY;
    }
    csb_v1_dungeon_unload();
    free(profile->dungeon_handle);
    profile->dungeon_handle = candidate;
    csb_v1_dungeon_set_current(candidate);
    profile->current_level = info.party_map_index;
    profile->level_count = candidate->level_count;
    profile->party_x = info.party_x;
    profile->party_y = info.party_y;
    profile->party_dir = info.party_direction & 3;
    profile->game_time = info.game_time;
    profile->timeline_queue.gameTick = info.game_time;
    csb_v1_dungeon_set_current_level(profile->current_level);
    if (out_info) *out_info = info;
    return CSB_V1_ATARI_RUNTIME_OK;
}
