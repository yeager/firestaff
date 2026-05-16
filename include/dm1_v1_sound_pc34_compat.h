#ifndef FIRESTAFF_DM1_V1_SOUND_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SOUND_PC34_COMPAT_H
/*
 * DM1 V1 Sound & Music System — source-locked to ReDMCSB SOUND.C / MUSIC.C / DEFS.H.
 *
 * ReDMCSB source anchors (Atari ST DM1 V1 / I34E PC):
 *   SOUND.C  — F0064_SOUND_RequestPlay_CPSD, F0065_SOUND_PlayPendingSound_CPSD,
 *              F0505_SOUND_GetVolume (directional 25x25 volume table)
 *   MUSIC.C  — F0742_MUSIC_SetTrack, F0743_MUSIC_Update, map-to-track table
 *   DEFS.H   — SOUND_DATA, SOUND_VOLUME, sound event constants, play modes
 */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DEFS.H sound event constants (I34E/MEDIA485) */
#define DM1_SND_METALLIC_THUD                     0
#define DM1_SND_SWITCH                            1
#define DM1_SND_DOOR_RATTLE                       2
#define DM1_SND_DOOR_RATTLE_ENTRANCE              3
#define DM1_SND_WOODEN_THUD                       4
#define DM1_SND_STRONG_EXPLOSION                  5
#define DM1_SND_WEAK_EXPLOSION                    6
#define DM1_SND_SCREAM                            7
#define DM1_SND_SWALLOW                           8
#define DM1_SND_CHAMPION_0_DAMAGED                9
#define DM1_SND_CHAMPION_1_DAMAGED               10
#define DM1_SND_CHAMPION_2_DAMAGED               11
#define DM1_SND_CHAMPION_3_DAMAGED               12
#define DM1_SND_COMBAT                           13
#define DM1_SND_BUZZ                             14
#define DM1_SND_PARTY_DAMAGED                    15
#define DM1_SND_SPELL                            16
#define DM1_SND_WAR_CRY                          17
#define DM1_SND_BLOW_HORN                        18
#define DM1_SND_ATTACK_SCREAMER_OITU             19
#define DM1_SND_ATTACK_SCORPION                  20
#define DM1_SND_ATTACK_WORM                      21
#define DM1_SND_ATTACK_GIGGLER                   22
#define DM1_SND_ATTACK_PAIN_RAT                  23
#define DM1_SND_ATTACK_ROCK                      24
#define DM1_SND_ATTACK_MUMMY_GHOST               25
#define DM1_SND_ATTACK_WATER_ELEMENTAL           26
#define DM1_SND_ATTACK_COUATL                    27
#define DM1_SND_MOVE_ANIMATED_ARMOUR             28
#define DM1_SND_MOVE_COUATL_WASP                 29
#define DM1_SND_MOVE_MUMMY_TROLIN                30
#define DM1_SND_MOVE_SCREAMER_ROCK               31
#define DM1_SND_MOVE_SLIME_WATER                 32
#define DM1_SND_MOVE_RED_DRAGON                  33
#define DM1_SND_MOVE_SKELETON                    34

#define DM1_SND_NONE                             (-1)
#define DM1_SND_COUNT                            35
#define DM1_SND_FIRST_ATTACK                     19
#define DM1_SND_FIRST_MOVEMENT                   28

/* ReDMCSB play modes */
#define DM1_MODE_DO_NOT_PLAY              (-1)
#define DM1_MODE_PLAY_IMMEDIATELY           0
#define DM1_MODE_PLAY_IF_PRIORITIZED        1
#define DM1_MODE_PLAY_ONE_TICK_LATER        2

typedef struct { uint8_t left; uint8_t right; } DM1_SoundVolume;

typedef struct {
    int16_t  graphicIndex;
    uint8_t  period;
    uint8_t  priority;
    uint8_t  loudDistance;
    uint8_t  softDistance;
} DM1_SoundData;

#define DM1_PENDING_NONE (-1)

typedef struct {
    int16_t pendingSoundIndex;
    int16_t pendingSoundVolume;
} DM1_PendingSound;

#define DM1_MUSIC_MAP_COUNT         14
#define DM1_MUSIC_TRACK_NONE       (-1)

typedef struct {
    int16_t mapToTrack[DM1_MUSIC_MAP_COUNT];
    int16_t currentMapIndex;
    int16_t pendingTrack;
    int16_t playingTrack;
    int     countdownBeforeStart;
    int     musicOn;
} DM1_MusicState;

typedef struct {
    DM1_SoundData    soundData[DM1_SND_COUNT];
    DM1_PendingSound pending;
    int16_t partyMapX;
    int16_t partyMapY;
    int16_t partyDirection;
    int16_t partyMapIndex;
    DM1_MusicState   music;
    int masterVolume;
    int sfxVolume;
    int musicVolume;
    int muted;
    int lastPlayedSoundIndex;
    int totalSoundRequests;
    int totalSoundsPlayed;
    int totalPendingFlushes;
} DM1_SoundSystem;

void DM1_Sound_Init(DM1_SoundSystem* sys);
int  DM1_Sound_GetVolume(const DM1_SoundSystem* sys,
                         int16_t sourceMapX, int16_t sourceMapY,
                         DM1_SoundVolume* outVolume);
void DM1_Sound_RequestPlay(DM1_SoundSystem* sys,
                           int16_t soundIndex,
                           int16_t mapX, int16_t mapY,
                           int mode);
void DM1_Sound_PlayPending(DM1_SoundSystem* sys);
void DM1_Sound_SetPartyPosition(DM1_SoundSystem* sys,
                                int16_t mapX, int16_t mapY,
                                int16_t direction, int16_t mapIndex);
void DM1_Music_SetTrack(DM1_SoundSystem* sys, int16_t mapIndex);
void DM1_Music_Update(DM1_SoundSystem* sys);
void DM1_Music_Stop(DM1_SoundSystem* sys);
int  DM1_Sound_GetLastPlayedIndex(const DM1_SoundSystem* sys);
const DM1_SoundData* DM1_Sound_GetSoundData(const DM1_SoundSystem* sys, int16_t soundIndex);
const char* DM1_Sound_Name(int16_t soundIndex);

#ifdef __cplusplus
}
#endif
#endif
