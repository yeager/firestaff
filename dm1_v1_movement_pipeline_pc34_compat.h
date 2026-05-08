#ifndef DM1_V1_MOVEMENT_PIPELINE_PC34_COMPAT_H
#define DM1_V1_MOVEMENT_PIPELINE_PC34_COMPAT_H

/*
 * DM1 V1 Movement Command Pipeline — unified entry point.
 *
 * Wires the complete input→command→movement chain from ReDMCSB into a
 * single call that the game loop can invoke once per tick:
 *
 *   1. SDL event → DM1 V1 input event (keyboard/mouse)
 *   2. Input event → command queue (F0359/F0361 enqueue, lock/pending)
 *   3. Command queue → dequeue + movement-disabled gate (F0380)
 *   4. Dequeued turn → F0365: walk-off sensors, SetPartyDirection, walk-on sensors
 *   5. Dequeued step → F0366: resolve destination, wall/door/group check,
 *      F0267 move result, sensor leave/enter, timing cooldown
 *   6. Post-move environment: pit fall chain, teleporter chain (F0267 loop)
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   COMMAND.C: F0358 (hit matcher), F0359 (mouse enqueue), F0360 (replay),
 *              F0361 (keyboard enqueue), F0380 (process queue)
 *   CLIKMENU.C: F0365 (turn dispatch), F0366 (step dispatch)
 *   MOVESENS.C: F0267 (move result + sensor/pit/teleporter chain),
 *               F0276 (sensor process thing addition/removal)
 *   CHAMPION.C: F0310 (movement ticks per champion)
 *   GAMELOOP.C: cooldown decrement per game loop tick
 *
 * All downstream modules are pure-function compat layers with no side
 * effects beyond the returned result struct.  The pipeline itself is
 * deterministic and testable without SDL or graphics.
 */

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_movement_command_core_pc34_compat.h"
#include "dm1_v1_movement_timing_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Full pipeline state — caller allocates and persists across ticks.
 *
 * The pipeline does NOT own the dungeon/things/party; those live in the
 * world state.  It owns only the command queue and timing cooldowns.
 */
struct Dm1V1MovementPipelinePc34Compat {
    struct Dm1V1InputCommandQueuePc34Compat commandQueue;
    int disabledMovementTicks;
    int projectileDisabledMovementTicks;
    int lastProjectileDisabledMovementDirection;
    unsigned long lastPartyMovementTime;
    unsigned long gameTick;
};

/*
 * Full pipeline result — produced by ProcessOneTick.
 */
struct Dm1V1MovementPipelineProvenancePc34Compat {
    /* Deterministic compat provenance, not a DOSBox/original screenshot claim. */
    int commandAccepted;
    int movementApplied;
    int viewportPresent;
    int originalRuntimeObserved;
    int noPixelParityClaim;
    const char* commandAcceptedEvidence;
    const char* movementAppliedEvidence;
    const char* viewportPresentEvidence;
};

struct Dm1V1MovementPipelineResultPc34Compat {
    /* From the command core */
    struct Dm1V1MovementCommandCoreResultPc34Compat core;

    /* Post-move environment (pit/teleporter chain) */
    struct PostMoveResolution_Compat postMove;
    int postMoveResolved;

    /* Updated timing state (caller should copy back) */
    int newDisabledMovementTicks;
    int newProjectileDisabledMovementTicks;
    unsigned long newLastPartyMovementTime;

    /* Aggregate flags for the game loop */
    int anyMovementOccurred;
    int anyTurnOccurred;
    int viewportDirty;

    /* Source-locked compat trace for command→movement→viewport provenance. */
    struct Dm1V1MovementPipelineProvenancePc34Compat provenance;
};

/*
 * Initialize the pipeline state.  Call once at game start.
 */
void DM1_V1_MovementPipeline_InitPc34Compat(
    struct Dm1V1MovementPipelinePc34Compat* pipeline);

/*
 * Feed an SDL-translated input event into the pipeline's command queue.
 *
 * Source mapping: COMMAND.C F0359 (mouse) / F0361 (keyboard).
 * Returns 1 if the event was enqueued, 0 if dropped/pending.
 */
int DM1_V1_MovementPipeline_EnqueueInputPc34Compat(
    struct Dm1V1MovementPipelinePc34Compat* pipeline,
    struct Dm1V1InputEventPc34Compat event);

/*
 * Feed an already-resolved ReDMCSB command id into the pipeline queue.
 *
 * This is the direct command seam for I34E movement-table commands after
 * COMMAND.C F0361 has resolved 0x004B..0x0051 into C001..C006. It bypasses
 * OS/keypad delivery while preserving the F0380 queue/core/pipeline gates.
 *
 * x/y are retained because the ReDMCSB queue carries command coordinates;
 * movement commands C001..C006 ignore them during F0365/F0366 dispatch.
 *
 * Returns 1 if the command was queued, 0 if invalid/full.
 */
int DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(
    struct Dm1V1MovementPipelinePc34Compat* pipeline,
    int command,
    int x,
    int y);

/*
 * Process one tick of the movement pipeline.
 *
 * Dequeues at most one command from the queue, validates it against the
 * dungeon state, and if accepted: mutates party position/direction,
 * resolves post-move environment (pits/teleporters), and computes
 * movement timing cooldowns.
 *
 * footwearIcons: per-champion footwear icon IDs for speed calculation
 *                (pass NULL for default timing).
 *
 * Source mapping: COMMAND.C F0380 -> CLIKMENU.C F0365/F0366 ->
 *                MOVESENS.C F0267 -> CHAMPION.C F0310 -> GAMELOOP.C
 *
 * Returns 1 on success (result populated), 0 on invalid args.
 */
int DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
    struct Dm1V1MovementPipelinePc34Compat* pipeline,
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    struct PartyState_Compat* party,
    const int footwearIcons[CHAMPION_MAX_PARTY],
    struct Dm1V1MovementPipelineResultPc34Compat* outResult);

/*
 * Decrement movement cooldowns.  Call once per game loop tick.
 *
 * Source mapping: GAMELOOP.C:150-155.
 */
void DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat(
    struct Dm1V1MovementPipelinePc34Compat* pipeline);

/*
 * Source evidence string for the pipeline.
 */
const char* DM1_V1_MovementPipeline_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_MOVEMENT_PIPELINE_PC34_COMPAT_H */
