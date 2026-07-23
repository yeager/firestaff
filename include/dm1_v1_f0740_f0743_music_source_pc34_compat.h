#ifndef FIRESTAFF_DM1_V1_F0740_F0743_MUSIC_SOURCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0740_F0743_MUSIC_SOURCE_PC34_COMPAT_H

#include "song_dat_loader_v1.h"

enum {
    DM1_V1_F0740_F0743_MAP_COUNT_PC34 = 14,
    DM1_V1_F0740_F0743_START_DELAY_PC34 = 100,
    DM1_V1_F0741_GAME_WON_TRACK_PC34 = 2,
    DM1_V1_F0740_F0743_TRACK_NONE_PC34 = -1
};

typedef struct DM1_V1_F0740F0743MusicSourcePc34 {
    int authenticated;
    char md5[33];
    V1_SongManifest manifest;
    V1_SongSequence sequence;
} DM1_V1_F0740F0743MusicSourcePc34;

typedef struct DM1_V1_F0740F0743MusicStatePc34 {
    int musicOn;
    int paused;
    int currentMapIndex;
    int pendingTrack;
    int playingTrack;
    int startCountdown;
} DM1_V1_F0740F0743MusicStatePc34;

typedef void (*DM1_V1_F0740PauseMusicPc34)(void* context);
typedef void (*DM1_V1_F0741PlayMusicPc34)(
    void* context, int sourceTrackId, const V1_SongSequence* sequence);

typedef struct DM1_V1_F0740F0743MusicDriverPc34 {
    DM1_V1_F0740PauseMusicPc34 pause;
    DM1_V1_F0741PlayMusicPc34 play;
    void* context;
} DM1_V1_F0740F0743MusicDriverPc34;

typedef struct DM1_V1_F0740F0743MusicReceiptPc34 {
    int valid;
    int suppressSyntheticPlayback;
    int functionId;
    int sourceTrackId;
    int mapIndex;
    int sequenceWordCount;
    char sourceMd5[33];
} DM1_V1_F0740F0743MusicReceiptPc34;

/* Only the known DM PC 3.4 SONG.DAT hash, DMCSB2 manifest and SEQ2 sequence
 * admit a music source. A filename, host PCM buffer, or generated tone is
 * never evidence of source audio. */
int dm1_v1_f0740_f0743_bind_song_dat_pc34(
    const char* songDatPath,
    DM1_V1_F0740F0743MusicSourcePc34* outSource);

void dm1_v1_f0740_f0743_music_state_init_pc34(
    DM1_V1_F0740F0743MusicStatePc34* outState);

int dm1_v1_f0740_music_pause_pc34(
    const DM1_V1_F0740F0743MusicSourcePc34* source,
    DM1_V1_F0740F0743MusicStatePc34* state,
    const DM1_V1_F0740F0743MusicDriverPc34* driver,
    DM1_V1_F0740F0743MusicReceiptPc34* outReceipt);

/* ENDGAME.C's proven C2 game-won route. Other F0741 track identities remain
 * fail-closed until their SONG.DAT sequence binding is recovered. */
int dm1_v1_f0741_play_game_music_pc34(
    const DM1_V1_F0740F0743MusicSourcePc34* source,
    DM1_V1_F0740F0743MusicStatePc34* state,
    int sourceTrackId,
    const DM1_V1_F0740F0743MusicDriverPc34* driver,
    DM1_V1_F0740F0743MusicReceiptPc34* outReceipt);

int dm1_v1_f0742_set_map_track_pc34(
    const DM1_V1_F0740F0743MusicSourcePc34* source,
    DM1_V1_F0740F0743MusicStatePc34* state,
    int mapIndex,
    DM1_V1_F0740F0743MusicReceiptPc34* outReceipt);

int dm1_v1_f0743_update_music_pc34(
    const DM1_V1_F0740F0743MusicSourcePc34* source,
    DM1_V1_F0740F0743MusicStatePc34* state,
    const DM1_V1_F0740F0743MusicDriverPc34* driver,
    DM1_V1_F0740F0743MusicReceiptPc34* outReceipt);

#endif
