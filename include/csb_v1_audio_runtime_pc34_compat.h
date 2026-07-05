#ifndef FIRESTAFF_CSB_V1_AUDIO_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_AUDIO_RUNTIME_PC34_COMPAT_H
/*
 * CSB V1 audio runtime boundary.
 *
 * Source anchors:
 *   ReDMCSB DEFS.H:135-138                  sound play modes
 *   ReDMCSB SOUND.C F0064:1632-1638         pending sound arbitration
 *   ReDMCSB SOUND.C F0065:1804-1865         one pending sound flushed per tick
 *   ReDMCSB GAMELOOP.C:114-115              flush before damage/time advance
 *   ReDMCSB LOADSAVE.C F0433:1530/F0435:2739 LastCreatureAttackTime save/load
 *   ReDMCSB PROJEXPL.C:5                    LastCreatureAttackTime initial -200
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_SOUND_NONE (-1)
#define CSB_V1_SOUND_COUNT 35

#define CSB_V1_SOUND_METALLIC_THUD 0
#define CSB_V1_SOUND_SWITCH 1
#define CSB_V1_SOUND_DOOR_RATTLE 2
#define CSB_V1_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM 4
#define CSB_V1_SOUND_STRONG_EXPLOSION 5
#define CSB_V1_SOUND_SCREAM 7
#define CSB_V1_SOUND_COMBAT 13
#define CSB_V1_SOUND_BUZZ 14
#define CSB_V1_SOUND_PARTY_DAMAGED 15
#define CSB_V1_SOUND_SPELL 16
#define CSB_V1_SOUND_ATTACK_SCORPION 20
#define CSB_V1_SOUND_MOVE_SKELETON 34

#define CSB_V1_MODE_DO_NOT_PLAY (-1)
#define CSB_V1_MODE_PLAY_IMMEDIATELY 0
#define CSB_V1_MODE_PLAY_IF_PRIORITIZED 1
#define CSB_V1_MODE_PLAY_ONE_TICK_LATER 2

typedef struct CsbV1AudioRequest {
    int16_t soundIndex;
    int16_t mapX;
    int16_t mapY;
    int16_t mode;
    int16_t volume;
    uint8_t priority;
} CsbV1AudioRequest;

typedef struct CsbV1AudioRuntime {
    int16_t pendingSoundIndex;
    int16_t pendingVolume;
    uint8_t pendingPriority;
    int16_t lastPlayedSoundIndex;
    int32_t lastCreatureAttackTime;
    uint32_t totalRequests;
    uint32_t totalImmediatePlays;
    uint32_t totalPendingFlushes;
    uint32_t totalRejectedRequests;
} CsbV1AudioRuntime;

typedef struct CsbV1AudioSaveSnapshot {
    int32_t lastCreatureAttackTime;
} CsbV1AudioSaveSnapshot;

void csb_v1_audio_runtime_init(CsbV1AudioRuntime* runtime);
int csb_v1_audio_runtime_request(CsbV1AudioRuntime* runtime,
                                 const CsbV1AudioRequest* request);
int csb_v1_audio_runtime_flush_pending(CsbV1AudioRuntime* runtime);
void csb_v1_audio_runtime_record_creature_attack(CsbV1AudioRuntime* runtime,
                                                 int32_t gameTime);
void csb_v1_audio_runtime_save_snapshot(const CsbV1AudioRuntime* runtime,
                                        CsbV1AudioSaveSnapshot* outSnapshot);
void csb_v1_audio_runtime_load_snapshot(CsbV1AudioRuntime* runtime,
                                        const CsbV1AudioSaveSnapshot* snapshot);
const char* csb_v1_audio_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_AUDIO_RUNTIME_PC34_COMPAT_H */
