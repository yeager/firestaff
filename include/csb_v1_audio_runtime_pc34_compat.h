#ifndef FIRESTAFF_CSB_V1_AUDIO_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_AUDIO_RUNTIME_PC34_COMPAT_H

/*
 * CSB V1 audio runtime boundary.
 *
 * Source anchors:
 *   ReDMCSB DEFS.H:135-138                  sound play modes
 *   ReDMCSB SOUND.C F0060:887-931,1164-1246 Atari ST sound playback
 *   ReDMCSB SOUND.C F0061:1144-1307         PSG loud-table amplitude writes
 *   ReDMCSB SOUND.C F0064:1632-1638         pending sound arbitration
 *   ReDMCSB SOUND.C F0065:1804-1865         one pending sound flushed per tick
 *   ReDMCSB GAMELOOP.C:114-115              flush before damage/time advance
 *   ReDMCSB LOADSAVE.C F0433:1530/F0435:2739 LastCreatureAttackTime save/load
 *   ReDMCSB PROJEXPL.C:5                    LastCreatureAttackTime initial -200
 */

#include <stddef.h>
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
#define CSB_V1_SOUND_MOVE_ANIMATED_ARMOUR_DETH_KNIGHT 28
#define CSB_V1_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER 29
#define CSB_V1_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON 30
#define CSB_V1_SOUND_MOVE_SCREAMER_ROCKPILE_WORM_PAIN_RAT_SCORPION_OITU 31
#define CSB_V1_SOUND_MOVE_SWAMP_SLIME_WATER_ELEMENTAL 32
#define CSB_V1_SOUND_MOVE_RED_DRAGON 33
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

/* PC 3.4 source-owned sound routing. ReDMCSB DATA.C keeps this table in the
 * executable; it is not encoded inside GRAPHICS.DAT graphic 562. */
typedef struct CsbV1Pc34SoundSpec {
    uint16_t graphicIndex;
    uint8_t period;
    uint8_t priority;
    uint8_t loudDistance;
    uint8_t softDistance;
} CsbV1Pc34SoundSpec;

/* A bounded, source-owned PC3.4 sound payload. DATA.C selects its graphic
 * index and IO.C F0060 passes exactly byteCount bytes beginning at bytes[2]
 * to the active PC sound driver. The caller owns no storage and must release
 * the payload with csb_v1_audio_runtime_pc34_sound_payload_free(). */
typedef struct CsbV1Pc34SoundPayload {
    uint8_t *bytes;
    size_t byteCount;
    CsbV1Pc34SoundSpec spec;
} CsbV1Pc34SoundPayload;

/* ReDMCSB SOUND.C F0060 supplies a u16be output-count followed by packed
 * high-nibble-first amplitude commands. A zero command repeats initialLevel
 * or the previous nonzero command for a variable-length run. */
typedef struct CsbV1StSoundDecodeResult {
    size_t sampleCount;
    size_t encodedBytesConsumed;
} CsbV1StSoundDecodeResult;

/* F0061 selects all three PSG amplitude registers from its loud table. */
typedef struct CsbV1PsgChannelAmplitudes {
    uint8_t channelA;
    uint8_t channelB;
    uint8_t channelC;
} CsbV1PsgChannelAmplitudes;

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

/* Returns the immutable ReDMCSB DATA.C I34E/I34M table row for soundIndex,
 * or NULL when the index is outside the PC 3.4 sound domain. */
const CsbV1Pc34SoundSpec*
csb_v1_audio_runtime_pc34_sound_spec(int16_t soundIndex);

/* Load one original PC3.4 GRAPHICS.DAT sound record. This accepts only an
 * uncompressed source entry whose F0060 u16be byte count exactly consumes the
 * record apart from the original two-byte GRAPHICS.DAT tail. It returns 1 on
 * success, 0 for malformed/unavailable source data, and never fabricates
 * sample data. */
int csb_v1_audio_runtime_load_pc34_sound_payload(
    const char *graphicsDatPath,
    int16_t soundIndex,
    CsbV1Pc34SoundPayload *outPayload);
void csb_v1_audio_runtime_pc34_sound_payload_free(
    CsbV1Pc34SoundPayload *payload);

/* Resolves the three writes made by F0061_SOUND_SetChannelAmplitudes. The
 * original routine masks amplitudeIndex with 0x000f before indexing its loud
 * PSG table, so negative and oversized inputs wrap identically. */
CsbV1PsgChannelAmplitudes
csb_v1_audio_runtime_channel_amplitudes(int16_t amplitudeIndex);

/* Expands F0060's Atari ST packed stream into the PSG amplitude indices that
 * its Timer-A handler would apply. Returns 0 on success, -1 for invalid
 * arguments, -2 for malformed/truncated data, and -3 for insufficient output
 * storage. initialLevel is the already-active PSG level (0..15). */
int csb_v1_audio_runtime_decode_st_sound(const uint8_t* encoded,
                                         size_t encodedSize,
                                         uint8_t initialLevel,
                                         uint8_t* outLevels,
                                         size_t outLevelCapacity,
                                         CsbV1StSoundDecodeResult* outResult);

const char* csb_v1_audio_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_AUDIO_RUNTIME_PC34_COMPAT_H */
