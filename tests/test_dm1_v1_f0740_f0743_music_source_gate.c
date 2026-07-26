#include "dm1_v1_f0740_f0743_music_source_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct MusicCapturePc34 {
    int pauses;
    int plays;
    int track;
    int sequenceWords;
} MusicCapturePc34;

static void capture_pause(void* context)
{
    MusicCapturePc34* capture = (MusicCapturePc34*)context;
    if (capture) ++capture->pauses;
}

static void capture_play(void* context, int track,
                         const V1_SongSequence* sequence)
{
    MusicCapturePc34* capture = (MusicCapturePc34*)context;
    if (!capture || !sequence) return;
    ++capture->plays;
    capture->track = track;
    capture->sequenceWords = (int)sequence->wordCount;
}

static const char* song_path(char* path, size_t pathSize)
{
    const char* explicitPath = getenv("FIRESTAFF_SONG_DAT");
    const char* home = getenv("HOME");
    FILE* file;

    if (explicitPath && explicitPath[0]) return explicitPath;
    if (!home || !home[0]) return NULL;
    snprintf(path, pathSize, "%s/.firestaff/data/dm1/SONG.DAT", home);
    file = fopen(path, "rb");
    if (!file) return NULL;
    fclose(file);
    return path;
}

int main(void)
{
    DM1_V1_F0740F0743MusicSourcePc34 source;
    DM1_V1_F0740F0743MusicStatePc34 state;
    DM1_V1_F0740F0743MusicDriverPc34 driver;
    DM1_V1_F0740F0743MusicReceiptPc34 receipt;
    MusicCapturePc34 capture;
    char localPath[1024];
    const char* path;
    int tick;

    memset(&capture, 0, sizeof(capture));
    memset(&driver, 0, sizeof(driver));
    driver.pause = capture_pause;
    driver.play = capture_play;
    driver.context = &capture;
    if (dm1_v1_f0740_f0743_bind_song_dat_pc34(NULL, &source)) return 1;
    path = song_path(localPath, sizeof(localPath));
    if (!path) {
        puts("SKIP: authenticated PC34 SONG.DAT not installed");
        return 0;
    }
    if (!dm1_v1_f0740_f0743_bind_song_dat_pc34(path, &source)) {
        if (getenv("FIRESTAFF_SONG_DAT")) return 1;
        puts("SKIP: local SONG.DAT is not the authenticated PC34 corpus");
        return 0;
    }
    dm1_v1_f0740_f0743_music_state_init_pc34(&state);
    if (!dm1_v1_f0742_set_map_track_pc34(&source, &state, 0, &receipt) ||
        !receipt.valid || receipt.functionId != 742 || receipt.sourceTrackId != 2 ||
        state.startCountdown != 100) return 1;
    for (tick = 0; tick < 100; ++tick) {
        if (!dm1_v1_f0743_update_music_pc34(&source, &state, &driver, &receipt)) return 1;
    }
    if (capture.plays != 0 || state.startCountdown != 0) return 1;
    if (!dm1_v1_f0743_update_music_pc34(&source, &state, &driver, &receipt) ||
        capture.plays != 1 || capture.track != 2 || capture.sequenceWords <= 0 ||
        !receipt.valid || receipt.functionId != 743) return 1;
    if (!dm1_v1_f0740_music_pause_pc34(&source, &state, &driver, &receipt) ||
        capture.pauses != 1 || !receipt.valid || receipt.functionId != 740) return 1;
    if (!dm1_v1_f0741_play_game_music_pc34(&source, &state, 2, &driver, &receipt) ||
        capture.plays != 2 || receipt.functionId != 741) return 1;
    if (!dm1_v1_f0741_play_game_music_pc34(&source, &state, 3, &driver, &receipt) ||
        capture.plays != 3 || receipt.functionId != 741 ||
        receipt.sourceTrackId != 3) return 1;
    {
        int mapIdx;
        dm1_v1_f0740_f0743_music_state_init_pc34(&state);
        memset(&capture, 0, sizeof(capture));
        for (mapIdx = 0; mapIdx < DM1_V1_F0740_F0743_MAP_COUNT_PC34; ++mapIdx) {
            if (!dm1_v1_f0742_set_map_track_pc34(&source, &state, mapIdx, &receipt) ||
                !receipt.valid || receipt.functionId != 742) return 1;
            state.startCountdown = 0;
            if (!dm1_v1_f0743_update_music_pc34(&source, &state, &driver, &receipt) ||
                !receipt.valid) return 1;
        }
        if (capture.plays != DM1_V1_F0740_F0743_MAP_COUNT_PC34) return 1;
    }
    puts("ok: real PC34 F0740-F0743 SONG.DAT source gate");
    return 0;
}
