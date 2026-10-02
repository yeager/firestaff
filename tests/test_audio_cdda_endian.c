#include "audio_sdl_m11.h"
#include "firestaff_fmtowns_disc.h"
#include "firestaff_zip_extract.h"

/* Assertions include transport calls and must remain active in Release. */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>

static void test_cdda_repeat_on_dummy_device(const uint8_t *pcm,
                                              size_t pcm_size);

static void test_original_cdda_gain_if_selected(void)
{
    const char* archive = getenv("FIRESTAFF_DM1_FMTOWNS_ZIP");
    uint8_t* cue = NULL;
    uint8_t* image = NULL;
    size_t cue_size = 0U, image_size = 0U;
    uint32_t starts[24];
    char image_member[256];
    size_t offset, span, index;
    int nonzero = 0;
    M11_AudioState audio;
    SDL_AudioStream* stream;
    int queued;
    static const int master[] = {64, 0, 128, 128};
    static const int music[] = {32, 128, 0, 128};
    static const float gains[] = {0.125f, 0.0f, 0.0f, 1.0f};

    if (!archive || !archive[0]) {
        puts("SKIP: original CDDA gain probe needs FIRESTAFF_DM1_FMTOWNS_ZIP");
        return;
    }
    /* Same native CUE/BIN ownership as test_dm1_v1_fmtowns_cd_audio:
     * this archive stores track 1 at 2048 bytes/sector, then raw CDDA. */
    assert(firestaff_zip_extract_by_suffix(archive, ".cue", &cue, &cue_size) == 0);
    assert(cue && fmtowns_cue_parse_image_member((const char*)cue, cue_size,
                                                image_member, sizeof(image_member)));
    memset(starts, 0, sizeof(starts));
    assert(fmtowns_cue_parse_track_starts((const char*)cue, cue_size, starts, 24) == 21);
    free(cue);
    assert(firestaff_zip_extract_by_suffix(archive, image_member,
                                            &image, &image_size) == 0 && image);
    offset = (size_t)starts[2] * 2048U;
    assert(starts[3] > starts[2]);
    span = (size_t)(starts[3] - starts[2]) * FMTOWNS_CDDA_SECTOR_SIZE;
    /* Queue at most one second of the real title track, never the full disc. */
    if (span > 75U * FMTOWNS_CDDA_SECTOR_SIZE)
        span = 75U * FMTOWNS_CDDA_SECTOR_SIZE;
    assert(offset < image_size && span > 0U && span <= image_size - offset);
    for (index = 0; index < span; ++index) nonzero |= image[offset + index];
    assert(nonzero);
    /* Repeat the first ten milliseconds of this authenticated CD track, so
     * the transport regression does not substitute a fabricated sound. */
    assert(span >= 1764u);
    test_cdda_repeat_on_dummy_device(image + offset, 1764u);
    assert(M11_Audio_Init(&audio));
    assert(M11_Audio_IsAvailable(&audio) && audio.cddaStream);
    stream = (SDL_AudioStream*)audio.cddaStream;
    assert(M11_Audio_SetVolumes(&audio, 64, 128, 32, 128));
    assert(SDL_GetAudioStreamGain(stream) == 0.125f);
    assert(SDL_GetAudioStreamQueued(stream) == 0);
    assert(M11_Audio_SetHostPaused(&audio, 1));
    assert(M11_Audio_PlayCdda(&audio, image + offset, span, 0));
    free(image);
    assert(SDL_GetAudioStreamGain(stream) == 0.125f);
    queued = SDL_GetAudioStreamQueued(stream);
    assert(queued == (int)span && audio.cddaPlaying && audio.hostPaused &&
           SDL_AudioStreamDevicePaused(stream));
    for (index = 0; index < sizeof(gains) / sizeof(gains[0]); ++index) {
        float gain;
        assert(M11_Audio_SetVolumes(&audio, master[index], 128, music[index], 128));
        gain = SDL_GetAudioStreamGain(stream);
        assert(gain > gains[index] - 0.00001f && gain < gains[index] + 0.00001f);
        assert(SDL_GetAudioStreamQueued(stream) == queued &&
               SDL_AudioStreamDevicePaused(stream) && audio.cddaPlaying && audio.hostPaused);
    }
    /* A source pause must remain owned by F0740 after host focus returns. */
    assert(M11_Audio_PauseCdda(&audio));
    assert(M11_Audio_SetHostPaused(&audio, 0));
    assert(audio.cddaPaused && SDL_AudioStreamDevicePaused(stream) &&
           SDL_GetAudioStreamQueued(stream) == queued);
    assert(M11_Audio_ResumeCdda(&audio));
    assert(!audio.cddaPaused && !SDL_AudioStreamDevicePaused(stream));
    M11_Audio_Shutdown(&audio);
    puts("PASS: original FM Towns CDDA obeys live master/music gain and preserves paused PCM");
}

static void test_cdda_repeat_on_dummy_device(const uint8_t *pcm,
                                             size_t pcm_size)
{
    M11_AudioState audio;
    SDL_AudioStream *stream;

    assert(SDL_setenv_unsafe("SDL_AUDIODRIVER", "dummy", 1) == 0);
    assert(M11_Audio_Init(&audio));
    assert(audio.cddaStream);
    stream = (SDL_AudioStream *)audio.cddaStream;
    assert(M11_Audio_PlayCdda(&audio, pcm, pcm_size, 1));
    assert(audio.cddaLoopPcmSize == pcm_size);
    SDL_Delay(100);
    assert(SDL_LockAudioStream(stream));
    assert(audio.cddaLoopRefillCount > 0);
    SDL_UnlockAudioStream(stream);
    assert(M11_Audio_StopCdda(&audio));
    assert(audio.cddaLoopPcm == NULL);
    assert(SDL_GetAudioStreamQueued(stream) == 0);
    assert(M11_Audio_PlayCdda(&audio, pcm, pcm_size, 0));
    SDL_Delay(100);
    assert(SDL_GetAudioStreamQueued(stream) == 0);
    assert(audio.cddaLoopPcm == NULL && audio.cddaLoopRefillCount == 0);
    M11_Audio_Shutdown(&audio);
    puts("PASS: repeated CDDA survives the first track and stops cleanly");
}

int main(void)
{
    const uint8_t red_book[8] = { 0x12, 0x34, 0xab, 0xcd,
                                  0x80, 0x00, 0x7f, 0xff };
    uint8_t s16le[8] = { 0 };
    const uint8_t expected[8] = { 0x12, 0x34, 0xab, 0xcd,
                                  0x80, 0x00, 0x7f, 0xff };

    assert(M11_Audio_ConvertRedBookPcmToS16Le(
        red_book, s16le, sizeof(red_book)) == 1);
    for (size_t index = 0u; index < sizeof(expected); ++index) {
        assert(s16le[index] == expected[index]);
    }
    assert(M11_Audio_ConvertRedBookPcmToS16Le(NULL, s16le,
                                               sizeof(red_book)) == 0);
    assert(M11_Audio_ConvertRedBookPcmToS16Le(red_book, NULL,
                                               sizeof(red_book)) == 0);
    assert(M11_Audio_ConvertRedBookPcmToS16Le(red_book, s16le, 6u) == 0);

    puts("PASS: original FM Towns CD-DA sample bytes reach SDL S16LE intact");
    test_original_cdda_gain_if_selected();
    return 0;
}
