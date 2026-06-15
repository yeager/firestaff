#include "csb_v1_movement_command_step_runtime_pc34_compat.h"

#include <string.h>

static int csb_v1_step_is_movement_command(int command)
{
    return command >= DM1_V1_COMMAND_MOVE_FORWARD &&
           command <= DM1_V1_COMMAND_MOVE_LEFT;
}

static void csb_v1_step_seed_result(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    if (!profile) return;
    result->old_party_x = profile->party_x;
    result->old_party_y = profile->party_y;
    result->old_party_dir = profile->party_dir & 3;
    result->destination_x = profile->party_x;
    result->destination_y = profile->party_y;
    result->new_party_x = profile->party_x;
    result->new_party_y = profile->party_y;
    result->new_party_dir = profile->party_dir & 3;
}

static void csb_v1_step_destination(
    int party_x,
    int party_y,
    int party_dir,
    int command,
    int *out_x,
    int *out_y)
{
    static const int east_count[4] = { 0, 1, 0, -1 };
    static const int north_count[4] = { -1, 0, 1, 0 };
    static const int forward_count[4] = { 1, 0, -1, 0 };
    static const int right_count[4] = { 0, 1, 0, -1 };
    int arrow_index = command - DM1_V1_COMMAND_MOVE_FORWARD;
    int dir = party_dir & 3;

    *out_x = party_x +
        east_count[dir] * forward_count[arrow_index] +
        east_count[(dir + 1) & 3] * right_count[arrow_index];
    *out_y = party_y +
        north_count[dir] * forward_count[arrow_index] +
        north_count[(dir + 1) & 3] * right_count[arrow_index];
}

int csb_v1_movement_command_step_runtime_apply_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    int command,
    CSB_V1_MovementStepWallProbePc34Compat wall_probe,
    void *wall_probe_context,
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat *out_result)
{
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat local_result;
    int wall_blocked = 0;

    if (!profile) return -1;
    csb_v1_step_seed_result(profile, &local_result);
    local_result.queue_result.command = command;

    if (!csb_v1_step_is_movement_command(command)) {
        local_result.unsupported_runtime_command = 1;
        if (out_result) *out_result = local_result;
        return 0;
    }

    local_result.command_handled = 1;
    local_result.step_attempted = 1;

    /* Source-lock: ReDMCSB CLIKMENU.C F0366 lines 224-233 maps
     * C003..C006 to forward/right step counts, then line 269 calls
     * DUNGEON.C F0150 lines 1389-1391 to resolve destination coordinates
     * from G0308_i_PartyDirection using G0233/G0234. */
    csb_v1_step_destination(profile->party_x,
                            profile->party_y,
                            profile->party_dir,
                            command,
                            &local_result.destination_x,
                            &local_result.destination_y);

    if (wall_probe) {
        local_result.wall_probe_called = 1;
        wall_blocked = wall_probe(profile,
                                  local_result.destination_x,
                                  local_result.destination_y,
                                  wall_probe_context) ? 1 : 0;
    }

    if (wall_blocked) {
        /* Source-lock: ReDMCSB CLIKMENU.C F0366 lines 277-304 marks
         * C00_ELEMENT_WALL as blocked, requests self-damage against the
         * leading party cells, discards input, waits one VBlank, and
         * returns without committing G0306/G0307. */
        local_result.blocked_by_wall = 1;
        local_result.new_party_x = profile->party_x;
        local_result.new_party_y = profile->party_y;
        local_result.new_party_dir = profile->party_dir & 3;
        if (out_result) *out_result = local_result;
        return 1;
    }

    /* Source-lock: ReDMCSB CLIKMENU.C F0366 lines 325-351 calls
     * MOVESENS.C F0267_MOVE_GetMoveResult_CPSCE for the party, then
     * commits movement timing.  This narrow CSB runtime slice commits only
     * the one-cell party coordinate step and mirrors the imported party
     * snapshot fields; sensors, stamina, stairs, doors, groups, and VBlank
     * timing remain owned by later CSB-specific gates. */
    profile->party_x = local_result.destination_x;
    profile->party_y = local_result.destination_y;
    local_result.step_applied = 1;
    local_result.disabled_movement_ticks_after = 1;

    if (profile->party_state_valid) {
        profile->party_state.PartyMapX = profile->party_x;
        profile->party_state.PartyMapY = profile->party_y;
        profile->party_state.PartyDirection = profile->party_dir & 3;
    }

    local_result.new_party_x = profile->party_x;
    local_result.new_party_y = profile->party_y;
    local_result.new_party_dir = profile->party_dir & 3;
    local_result.party_state_changed =
        (local_result.old_party_x != local_result.new_party_x) ||
        (local_result.old_party_y != local_result.new_party_y) ||
        (local_result.old_party_dir != local_result.new_party_dir);

    if (out_result) *out_result = local_result;
    return 1;
}

int csb_v1_movement_command_step_runtime_process_queue_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    struct Dm1V1InputCommandQueuePc34Compat *queue,
    int disabled_movement_ticks,
    int projectile_disabled_movement_ticks,
    int last_projectile_disabled_movement_direction,
    CSB_V1_MovementStepWallProbePc34Compat wall_probe,
    void *wall_probe_context,
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat *out_result)
{
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat local_result;
    struct Dm1V1InputQueueProcessResultPc34Compat queue_result;
    int apply_status;

    if (!profile || !queue) return -1;
    csb_v1_step_seed_result(profile, &local_result);

    /* Source-lock: ReDMCSB COMMAND.C F0380 lines 2075-2127 keeps movement
     * commands queued while G0310/G0311 gates are active, otherwise dequeues
     * exactly one command before dispatching C003..C006 to CLIKMENU.C F0366
     * at lines 2154-2156. */
    queue_result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
        queue,
        profile->party_dir,
        disabled_movement_ticks,
        projectile_disabled_movement_ticks,
        last_projectile_disabled_movement_direction);
    local_result.queue_result = queue_result;

    if (!queue_result.dequeued) {
        if (out_result) *out_result = local_result;
        return 0;
    }

    apply_status = csb_v1_movement_command_step_runtime_apply_pc34_compat(
        profile,
        queue_result.command,
        wall_probe,
        wall_probe_context,
        &local_result);
    if (apply_status < 0) return apply_status;
    local_result.queue_result = queue_result;
    if (out_result) *out_result = local_result;
    return 1;
}

const char *csb_v1_movement_command_step_runtime_source_evidence_pc34_compat(void)
{
    return "ReDMCSB COMMAND.C F0380 lines 2075-2127,2154-2156 queue/gate one C003-C006 move command; "
           "CLIKMENU.C F0366 lines 224-233 movement-arrow forward/right counts, line 269 F0150 destination, lines 277-304 wall blocked false path, lines 325-351 commit step timing; "
           "DUNGEON.C F0150 lines 1389-1391 G0233/G0234 relative coordinate update; "
           "MOVESENS.C F0267 lines 316-328 party move-result entrypoint; "
           "CHAMPION.C F0284 lines 117-130 imported champion Cell/Direction rotate only on party turns.";
}
