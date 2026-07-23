#include "dm1_v1_f0740_f0743_music_source_pc34_compat.h"

#include "asset_find_by_hash.h"

#include <string.h>

static const char kDm1SongDatMd5Pc34[] = "c20e5b8f756e360a631595cc9260f62d";
static const int kMapToTrackPc34[DM1_V1_F0740_F0743_MAP_COUNT_PC34] = {
    2, 14, 9, 6, 11, 6, 5, 12, 15, 16, 17, 10, 19, 3
};

static void receipt_clear(DM1_V1_F0740F0743MusicReceiptPc34* receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

static int source_valid(const DM1_V1_F0740F0743MusicSourcePc34* source)
{
    unsigned int index;

    if (!source || !source->authenticated ||
        strcmp(source->md5, kDm1SongDatMd5Pc34) != 0 ||
        source->manifest.signature != V1_SONG_DAT_SIGNATURE ||
        source->manifest.itemCount != V1_SONG_DAT_ITEM_COUNT ||
        source->manifest.fileBytes != V1_SONG_DAT_EXPECTED_SIZE ||
        !source->sequence.hasEndMarker || source->sequence.wordCount == 0) {
        return 0;
    }
    for (index = 0; index < source->sequence.wordCount; ++index) {
        unsigned int item = source->sequence.words[index] & 0x7fffu;
        if (source->sequence.words[index] & 0x8000u) return item <= 9u;
        if (item < V1_SONG_DAT_FIRST_SND8_INDEX ||
            item > V1_SONG_DAT_LAST_SND8_INDEX) return 0;
    }
    return 0;
}

static void receipt_write(DM1_V1_F0740F0743MusicReceiptPc34* receipt,
                          int functionId, int sourceTrackId, int mapIndex,
                          const DM1_V1_F0740F0743MusicSourcePc34* source)
{
    receipt->valid = 1;
    receipt->suppressSyntheticPlayback = 1;
    receipt->functionId = functionId;
    receipt->sourceTrackId = sourceTrackId;
    receipt->mapIndex = mapIndex;
    receipt->sequenceWordCount = (int)source->sequence.wordCount;
    memcpy(receipt->sourceMd5, source->md5, sizeof(receipt->sourceMd5));
}

int dm1_v1_f0740_f0743_bind_song_dat_pc34(
    const char* songDatPath,
    DM1_V1_F0740F0743MusicSourcePc34* outSource)
{
    char error[128];

    if (!outSource) return 0;
    memset(outSource, 0, sizeof(*outSource));
    if (!songDatPath ||
        !asset_file_matches_md5(songDatPath, kDm1SongDatMd5Pc34) ||
        !V1_Song_ParseManifest(songDatPath, &outSource->manifest,
                                error, sizeof(error)) ||
        !V1_Song_DecodeSequence(songDatPath, &outSource->manifest,
                                 &outSource->sequence,
                                 error, sizeof(error))) {
        memset(outSource, 0, sizeof(*outSource));
        return 0;
    }
    memcpy(outSource->md5, kDm1SongDatMd5Pc34, sizeof(outSource->md5));
    outSource->authenticated = 1;
    if (!source_valid(outSource)) {
        memset(outSource, 0, sizeof(*outSource));
        return 0;
    }
    return 1;
}

void dm1_v1_f0740_f0743_music_state_init_pc34(
    DM1_V1_F0740F0743MusicStatePc34* outState)
{
    if (!outState) return;
    memset(outState, 0, sizeof(*outState));
    outState->musicOn = 1;
    outState->currentMapIndex = DM1_V1_F0740_F0743_TRACK_NONE_PC34;
    outState->pendingTrack = DM1_V1_F0740_F0743_TRACK_NONE_PC34;
    outState->playingTrack = DM1_V1_F0740_F0743_TRACK_NONE_PC34;
    outState->startCountdown = DM1_V1_F0740_F0743_TRACK_NONE_PC34;
}

int dm1_v1_f0740_music_pause_pc34(
    const DM1_V1_F0740F0743MusicSourcePc34* source,
    DM1_V1_F0740F0743MusicStatePc34* state,
    const DM1_V1_F0740F0743MusicDriverPc34* driver,
    DM1_V1_F0740F0743MusicReceiptPc34* outReceipt)
{
    receipt_clear(outReceipt);
    if (!outReceipt || !state || !driver || !driver->pause ||
        !source_valid(source)) return 0;
    driver->pause(driver->context);
    state->paused = 1;
    receipt_write(outReceipt, 740, state->playingTrack, state->currentMapIndex,
                  source);
    return 1;
}

int dm1_v1_f0741_play_game_music_pc34(
    const DM1_V1_F0740F0743MusicSourcePc34* source,
    DM1_V1_F0740F0743MusicStatePc34* state,
    int sourceTrackId,
    const DM1_V1_F0740F0743MusicDriverPc34* driver,
    DM1_V1_F0740F0743MusicReceiptPc34* outReceipt)
{
    receipt_clear(outReceipt);
    if (!outReceipt || !state || !driver || !driver->play ||
        !source_valid(source) ||
        sourceTrackId != DM1_V1_F0741_GAME_WON_TRACK_PC34) {
        return 0;
    }
    driver->play(driver->context, sourceTrackId, &source->sequence);
    state->playingTrack = sourceTrackId;
    state->pendingTrack = sourceTrackId;
    state->startCountdown = DM1_V1_F0740_F0743_TRACK_NONE_PC34;
    state->paused = 0;
    receipt_write(outReceipt, 741, sourceTrackId, state->currentMapIndex, source);
    return 1;
}

int dm1_v1_f0742_set_map_track_pc34(
    const DM1_V1_F0740F0743MusicSourcePc34* source,
    DM1_V1_F0740F0743MusicStatePc34* state,
    int mapIndex,
    DM1_V1_F0740F0743MusicReceiptPc34* outReceipt)
{
    receipt_clear(outReceipt);
    if (!outReceipt || !state || !source_valid(source) || !state->musicOn ||
        mapIndex < 0 || mapIndex >= DM1_V1_F0740_F0743_MAP_COUNT_PC34) {
        return 0;
    }
    if (state->currentMapIndex == mapIndex) return 0;
    state->currentMapIndex = mapIndex;
    state->pendingTrack = kMapToTrackPc34[mapIndex];
    state->startCountdown = DM1_V1_F0740_F0743_START_DELAY_PC34;
    receipt_write(outReceipt, 742, state->pendingTrack, mapIndex, source);
    return 1;
}

int dm1_v1_f0743_update_music_pc34(
    const DM1_V1_F0740F0743MusicSourcePc34* source,
    DM1_V1_F0740F0743MusicStatePc34* state,
    const DM1_V1_F0740F0743MusicDriverPc34* driver,
    DM1_V1_F0740F0743MusicReceiptPc34* outReceipt)
{
    receipt_clear(outReceipt);
    if (!outReceipt || !state || !driver || !source_valid(source)) return 0;
    if (!state->musicOn) {
        if (!driver->pause) return 0;
        driver->pause(driver->context);
        state->paused = 1;
        state->currentMapIndex = DM1_V1_F0740_F0743_TRACK_NONE_PC34;
        state->pendingTrack = DM1_V1_F0740_F0743_TRACK_NONE_PC34;
        state->playingTrack = DM1_V1_F0740_F0743_TRACK_NONE_PC34;
        state->startCountdown = DM1_V1_F0740_F0743_TRACK_NONE_PC34;
        receipt_write(outReceipt, 743, DM1_V1_F0740_F0743_TRACK_NONE_PC34,
                      DM1_V1_F0740_F0743_TRACK_NONE_PC34, source);
        return 1;
    }
    if (state->startCountdown > 0) {
        --state->startCountdown;
        receipt_write(outReceipt, 743, state->pendingTrack,
                      state->currentMapIndex, source);
        return 1;
    }
    if (state->startCountdown == 0 &&
        state->pendingTrack != state->playingTrack &&
        state->pendingTrack == DM1_V1_F0741_GAME_WON_TRACK_PC34 &&
        driver->play) {
        driver->play(driver->context, state->pendingTrack, &source->sequence);
        state->playingTrack = state->pendingTrack;
        state->startCountdown = DM1_V1_F0740_F0743_TRACK_NONE_PC34;
        state->paused = 0;
        receipt_write(outReceipt, 743, state->playingTrack,
                      state->currentMapIndex, source);
        return 1;
    }
    return 0;
}
