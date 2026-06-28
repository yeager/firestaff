/*
 * CSB V1 audio runtime boundary.
 *
 * This module mirrors the ReDMCSB PC/Atari pending-sound runtime contract only.
 * It deliberately does not decode or mix real audio payloads.
 */

#include "csb_v1_audio_runtime_pc34_compat.h"

#include <string.h>

static int csb_v1_audio_valid_index(int16_t soundIndex)
{
    return soundIndex >= 0 && soundIndex < CSB_V1_SOUND_COUNT;
}

static void csb_v1_audio_clear_pending(CsbV1AudioRuntime* runtime)
{
    runtime->pendingSoundIndex = CSB_V1_SOUND_NONE;
    runtime->pendingVolume = 0;
    runtime->pendingPriority = 0;
}

void csb_v1_audio_runtime_init(CsbV1AudioRuntime* runtime)
{
    if (!runtime) {
        return;
    }
    memset(runtime, 0, sizeof(*runtime));
    csb_v1_audio_clear_pending(runtime);
    runtime->lastPlayedSoundIndex = CSB_V1_SOUND_NONE;
    runtime->lastCreatureAttackTime = -200;
}

int csb_v1_audio_runtime_request(CsbV1AudioRuntime* runtime,
                                 const CsbV1AudioRequest* request)
{
    if (!runtime || !request || !csb_v1_audio_valid_index(request->soundIndex) ||
        request->volume <= 0 || request->mode < CSB_V1_MODE_DO_NOT_PLAY) {
        if (runtime) {
            runtime->totalRejectedRequests++;
        }
        return 0;
    }
    if (request->mode == CSB_V1_MODE_DO_NOT_PLAY) {
        runtime->totalRejectedRequests++;
        return 0;
    }

    runtime->totalRequests++;
    if (request->mode == CSB_V1_MODE_PLAY_IMMEDIATELY) {
        runtime->lastPlayedSoundIndex = request->soundIndex;
        runtime->totalImmediatePlays++;
        csb_v1_audio_clear_pending(runtime);
        return 1;
    }

    /*
     * ReDMCSB SOUND.C F0064 lines 1632-1638:
     * nonzero play modes update G0583_i_PendingSoundIndex only if the new
     * request is louder, or equally loud with a higher SOUND_DATA priority.
     */
    if (runtime->pendingSoundIndex == CSB_V1_SOUND_NONE ||
        request->volume > runtime->pendingVolume ||
        (request->volume == runtime->pendingVolume &&
         request->priority > runtime->pendingPriority)) {
        runtime->pendingSoundIndex = request->soundIndex;
        runtime->pendingVolume = request->volume;
        runtime->pendingPriority = request->priority;
        return 1;
    }
    return 0;
}

int csb_v1_audio_runtime_flush_pending(CsbV1AudioRuntime* runtime)
{
    if (!runtime || runtime->pendingSoundIndex == CSB_V1_SOUND_NONE) {
        return 0;
    }

    /*
     * ReDMCSB SOUND.C F0065 lines 1804-1865 and GAMELOOP.C lines 114-115:
     * one pending sound is played during the game-loop tick and the pending
     * index/volume are reset before damage and time advance.
     */
    runtime->lastPlayedSoundIndex = runtime->pendingSoundIndex;
    runtime->totalPendingFlushes++;
    csb_v1_audio_clear_pending(runtime);
    return 1;
}

void csb_v1_audio_runtime_record_creature_attack(CsbV1AudioRuntime* runtime,
                                                 int32_t gameTime)
{
    if (!runtime) {
        return;
    }
    runtime->lastCreatureAttackTime = gameTime;
}

void csb_v1_audio_runtime_save_snapshot(const CsbV1AudioRuntime* runtime,
                                        CsbV1AudioSaveSnapshot* outSnapshot)
{
    if (!runtime || !outSnapshot) {
        return;
    }
    outSnapshot->lastCreatureAttackTime = runtime->lastCreatureAttackTime;
}

void csb_v1_audio_runtime_load_snapshot(CsbV1AudioRuntime* runtime,
                                        const CsbV1AudioSaveSnapshot* snapshot)
{
    if (!runtime || !snapshot) {
        return;
    }
    runtime->lastCreatureAttackTime = snapshot->lastCreatureAttackTime;
    csb_v1_audio_clear_pending(runtime);
}

const char* csb_v1_audio_runtime_source_evidence(void)
{
    return "DEFS.H:135-138; SOUND.C:1632-1638; SOUND.C:1804-1865; "
           "GAMELOOP.C:114-115; LOADSAVE.C:1530/2739; PROJEXPL.C:5";
}
