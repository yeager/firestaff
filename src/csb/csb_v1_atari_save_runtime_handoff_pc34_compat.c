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

int csb_v1_atari_save_prepare_runtime_handoff_pc34_compat(
    const uint8_t *bytes, size_t size,
    CSB_V1_AtariSaveHandoffCandidate *out_candidate)
{
    CSB_V1_DungeonData *candidate;
    CSB_V1_AtariSaveInfo info;
    CSB_V1_PartyState party;
    int result;

    if (!bytes || !out_candidate) return CSB_V1_ATARI_RUNTIME_ERR_NULL;
    memset(out_candidate, 0, sizeof(*out_candidate));
    result = csb_v1_atari_save_decode_party_pc34_compat(bytes, size, &party, &info);
    if (result != CSB_V1_ATARI_SAVE_OK) return CSB_V1_ATARI_RUNTIME_ERR_PARTY;
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

    /* ReDMCSB LOADSAVE.C F0435 completes all decode/pose checks before it
     * swaps the loaded dungeon into the live runtime.  Keep that boundary
     * explicit so a BAK slot is not renamed until every fallible operation
     * has passed. */
    out_candidate->dungeon = candidate;
    out_candidate->party = party;
    out_candidate->info = info;
    return CSB_V1_ATARI_RUNTIME_OK;
}

void csb_v1_atari_save_discard_runtime_handoff_candidate_pc34_compat(
    CSB_V1_AtariSaveHandoffCandidate *candidate)
{
    if (!candidate) return;
    if (candidate->dungeon) {
        csb_v1_dungeon_free(candidate->dungeon);
        free(candidate->dungeon);
    }
    memset(candidate, 0, sizeof(*candidate));
}

int csb_v1_atari_save_commit_runtime_handoff_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_AtariSaveHandoffCandidate *candidate,
    CSB_V1_AtariSaveInfo *out_info)
{
    CSB_V1_DungeonData *dungeon;
    const CSB_V1_AtariSaveInfo *info;

    if (!profile || !candidate || !candidate->dungeon) {
        return CSB_V1_ATARI_RUNTIME_ERR_NULL;
    }
    dungeon = candidate->dungeon;
    info = &candidate->info;
    /* prepare() already validates ChampionCount, so this cannot fail here. */
    if (csb_v1_runtime_set_party_state(profile, &candidate->party) != 0) {
        return CSB_V1_ATARI_RUNTIME_ERR_PARTY;
    }
    csb_v1_dungeon_unload();
    free(profile->dungeon_handle);
    profile->dungeon_handle = dungeon;
    csb_v1_dungeon_set_current(dungeon);
    profile->current_level = info->party_map_index;
    profile->level_count = dungeon->level_count;
    profile->party_x = info->party_x;
    profile->party_y = info->party_y;
    profile->party_dir = info->party_direction & 3;
    profile->game_time = info->game_time;
    profile->timeline_queue.gameTick = info->game_time;
    /* ReDMCSB LOADSAVE.C F0435 restores GAMEBLOCK2's random state together
     * with the party pose before play resumes.  The native Atari/Amiga
     * writer uses the common runtime random-state mirror, so retain the
     * authenticated source value here rather than falling back to the
     * dungeon seed during a later F0433 write-back. */
    profile->csbwin_random_seed_valid = 1;
    profile->csbwin_random_seed = info->random_seed;
    csb_v1_dungeon_set_current_level(profile->current_level);
    if (out_info) *out_info = *info;
    candidate->dungeon = NULL;
    memset(&candidate->party, 0, sizeof(candidate->party));
    memset(&candidate->info, 0, sizeof(candidate->info));
    return CSB_V1_ATARI_RUNTIME_OK;
}

int csb_v1_atari_save_handoff_runtime_pc34_compat(
    CSB_V1_RuntimeProfile *profile, const uint8_t *bytes, size_t size,
    CSB_V1_AtariSaveInfo *out_info)
{
    CSB_V1_AtariSaveHandoffCandidate candidate;
    int result;

    if (!profile) return CSB_V1_ATARI_RUNTIME_ERR_NULL;
    result = csb_v1_atari_save_prepare_runtime_handoff_pc34_compat(
        bytes, size, &candidate);
    if (result != CSB_V1_ATARI_RUNTIME_OK) return result;
    result = csb_v1_atari_save_commit_runtime_handoff_pc34_compat(
        profile, &candidate, out_info);
    csb_v1_atari_save_discard_runtime_handoff_candidate_pc34_compat(
        &candidate);
    return result;
}
