#include "csb_v1_movement_command_rotation_between_steps_runtime_pc34_compat.h"

#include <string.h>

static void csb_v1_rotation_between_steps_seed_party(
    CSB_V1_PartyState *party,
    int party_x,
    int party_y,
    int party_dir,
    int champion_count)
{
    int i;

    csb_v1_character_init_default(party);
    party->ChampionCount = champion_count;
    party->ImportedFromDM1 = 1;
    party->PartyDirection = (uint8_t)(party_dir & 3);
    party->LeaderIndex = 0;
    party->MagicCasterIndex = -1;
    party->PartyMapX = party_x;
    party->PartyMapY = party_y;

    for (i = 0; i < champion_count && i < CSB_V1_MAX_CHAMPIONS; ++i) {
        party->Champions[i].CurrentHealth = (int16_t)(90 + i);
        party->Champions[i].MaximumHealth = (int16_t)(120 + i);
        party->Champions[i].Cell = (uint8_t)i;
        party->Champions[i].Direction = (uint8_t)(party_dir & 3);
    }
}

static void csb_v1_rotation_between_steps_capture_champions(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_MovementCommandRotationChampionSnapshotPc34 out_champions[CSB_V1_MAX_CHAMPIONS])
{
    int i;

    for (i = 0; i < CSB_V1_MAX_CHAMPIONS; ++i) {
        out_champions[i].cell = profile->party_state.Champions[i].Cell;
        out_champions[i].direction = profile->party_state.Champions[i].Direction;
    }
}

const char *csb_v1_movement_command_rotation_between_steps_source_evidence_pc34(void)
{
    return "ReDMCSB COMMAND.C F0380 lines 2075-2156 dispatches one queued command at a time; "
           "COMMAND.C F0359 lines 1452-1661 writes mouse command queue entries; "
           "COMMAND.C F0361 lines 1709-1813 writes keyboard command queue entries; "
           "CLIKMENU.C F0366 lines 224-351 handles movement clicks and C003-C006 movement commands; "
           "DUNGEON.C F0150 lines 1389-1391 computes party-forward destination deltas; "
           "DUNGEON.C F0149 updates party position after a legal move; "
           "DUNGEON.C F0151 rotates party direction; "
           "DUNGEON.C F0163 lines 1769-1838 refreshes square contents after movement; "
           "MOVESENS.C F0267 lines 316-328 runs party movement sensors; "
           "CHAMPION.C F0284 lines 93-130 cascades party rotation into champion Cell/Direction; "
           "CHAMPION.C F0287 lines 243-268 keeps leader-hand state on champion transitions; "
           "DEFS.H C30 stores champion Cell and Direction offsets.";
}

int csb_v1_movement_command_rotation_between_steps_simulate_pc34(
    const CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Spec *spec,
    CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Result *out_result)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Result result;
    int champion_count;
    int base_x;
    int base_y;
    int initial_dir;

    if (!spec || !out_result) return -1;

    memset(&result, 0, sizeof(result));
    champion_count = spec->champion_count;
    if (champion_count < 1 || champion_count > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }
    base_x = spec->base_party_x;
    base_y = spec->base_party_y;
    initial_dir = spec->initial_party_dir & 3;

    csb_v1_runtime_init(&profile, NULL);
    csb_v1_rotation_between_steps_seed_party(
        &party, base_x, base_y, initial_dir, champion_count);
    if (csb_v1_runtime_set_party_state(&profile, &party) != 0) {
        return -1;
    }
    profile.party_x = base_x;
    profile.party_y = base_y;
    profile.party_dir = initial_dir;
    profile.party_state.PartyMapX = base_x;
    profile.party_state.PartyMapY = base_y;
    profile.party_state.PartyDirection = (uint8_t)initial_dir;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);

    /* Source-lock: ReDMCSB COMMAND.C F0361 lines 1709-1813 queues the
     * PC movement keyboard command.  Firestaff canonical constants keep
     * C003 as forward; C001/C002 are left/right turns. */
    result.first_forward_queued =
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
            &queue,
            (struct Dm1V1InputEventPc34Compat){
                DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 });
    if (!result.first_forward_queued) return -1;

    /* Source-lock: ReDMCSB COMMAND.C F0380 lines 2075-2156 dispatches only
     * the head queue command; CLIKMENU.C F0366 lines 224-351 routes C003
     * movement through DUNGEON.C F0150 lines 1389-1391 and F0149. */
    result.first_forward_processed =
        csb_v1_movement_command_step_runtime_process_queue_pc34_compat(
            &profile, &queue, 0, 0, 0, NULL, NULL, &result.step1_result);
    if (result.first_forward_processed != 1) return -1;
    result.step1_queue_result = result.step1_result.queue_result;
    result.party_x_after_step1 = profile.party_x;
    result.party_y_after_step1 = profile.party_y;
    result.party_dir_after_step1 = profile.party_dir & 3;
    result.queue_count_after_step1 = (int)queue.count;
    result.queue_empty_after_step1 = queue.count == 0u;
    csb_v1_rotation_between_steps_capture_champions(
        &profile, result.champions_after_step1);

    /* Source-lock: ReDMCSB COMMAND.C F0361 lines 1709-1813 queues C002
     * turn-right; F0380 lines 2075-2156 dispatches it before any later
     * forward command, and DUNGEON.C F0151 with CHAMPION.C F0284 lines
     * 93-130 commits the party and champion rotation. */
    result.turn_right_queued =
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
            &queue,
            (struct Dm1V1InputEventPc34Compat){
                DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 });
    if (!result.turn_right_queued) return -1;
    result.turn_right_processed =
        csb_v1_runtime_process_input_queue(
            &profile, &queue, 0, 0, 0, &result.turn_result);
    if (result.turn_right_processed != 1) return -1;
    result.turn_queue_result = result.turn_result.queue_result;
    result.party_x_after_rotation = profile.party_x;
    result.party_y_after_rotation = profile.party_y;
    result.party_dir_after_rotation = profile.party_dir & 3;
    result.queue_count_after_rotation = (int)queue.count;
    result.queue_empty_after_rotation = queue.count == 0u;
    csb_v1_rotation_between_steps_capture_champions(
        &profile, result.champions_after_rotation);

    /* Source-lock: ReDMCSB COMMAND.C F0361 lines 1709-1813 queues the
     * second forward command after the turn has completed.  F0380
     * lines 2075-2156 then dispatches that C003 step against the mutated
     * party_dir, so DUNGEON.C F0150 lines 1389-1391 advances east here. */
    result.second_forward_queued =
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
            &queue,
            (struct Dm1V1InputEventPc34Compat){
                DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 });
    if (!result.second_forward_queued) return -1;
    result.second_forward_processed =
        csb_v1_movement_command_step_runtime_process_queue_pc34_compat(
            &profile, &queue, 0, 0, 0, NULL, NULL, &result.step2_result);
    if (result.second_forward_processed != 1) return -1;
    result.step2_queue_result = result.step2_result.queue_result;
    result.party_x_after_step2 = profile.party_x;
    result.party_y_after_step2 = profile.party_y;
    result.party_dir_after_step2 = profile.party_dir & 3;
    result.queue_count_after_step2 = (int)queue.count;
    result.queue_empty_after_step2 = queue.count == 0u;
    csb_v1_rotation_between_steps_capture_champions(
        &profile, result.champions_after_step2);

    /* Source-lock: ReDMCSB DUNGEON.C F0163 lines 1769-1838 and MOVESENS.C
     * F0267 lines 316-328 are intentionally represented only as deterministic
     * post-dispatch snapshots in this no-game-data regression; full square
     * materialization and movement sensors stay with the dungeon-runtime
     * gates.  CHAMPION.C F0287 lines 243-268 and DEFS.H C30 are observed
     * here through the imported champion Cell/Direction fields. */
    *out_result = result;
    return 0;
}
