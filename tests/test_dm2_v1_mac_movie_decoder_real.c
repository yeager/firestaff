#include "dm2_v1_mac_media.h"
#include "dm2_v1_mac_movie.h"
#include "dm2_v1_mac_movie_decoder.h"
#include "audio_sdl_m11.h"
#include <SDL3/SDL.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A pull-only host stream prevents an audio callback from racing the receipt.
 * Every sample comes from the authenticated movie decoder below. */
static int check_movie_pcm_gain(const int16_t *source, int count, int rate,
                                int *heard_nonzero)
{
    static const int masters[] = {128, 64, 0, 128};
    static const int music[] = {0, 0, 128, 64};
    SDL_AudioSpec spec = {SDL_AUDIO_F32, 1, M11_AUDIO_SAMPLE_RATE};
    M11_AudioState state;
    size_t trial;
    int ok = 1;
    memset(&state, 0, sizeof(state));
    state.initialized = 1;
    state.backend = M11_AUDIO_BACKEND_SDL3;
    state.sdlStream = SDL_CreateAudioStream(&spec, &spec);
    if (!state.sdlStream) return 0;
    for (trial = 0; ok && trial < sizeof(masters) / sizeof(masters[0]); ++trial) {
        float *receipt;
        int bytes;
        int index;
        SDL_ClearAudioStream((SDL_AudioStream *)state.sdlStream);
        ok = M11_Audio_SetVolumes(&state, masters[trial], 0, music[trial], 0) &&
             M11_Audio_PlayDm2MacMoviePcm(&state, source, count, rate);
        bytes = state.dm2MacMoviePcm.sampleCount * (int)sizeof(float);
        receipt = bytes > 0 ? (float *)malloc((size_t)bytes) : NULL;
        if (!ok || !receipt ||
            SDL_GetAudioStreamGain((SDL_AudioStream *)state.sdlStream) !=
                (float)masters[trial] / 128.0f ||
            SDL_GetAudioStreamQueued((SDL_AudioStream *)state.sdlStream) != bytes ||
            !SDL_FlushAudioStream((SDL_AudioStream *)state.sdlStream) ||
            SDL_GetAudioStreamData((SDL_AudioStream *)state.sdlStream,
                                   receipt, bytes) != bytes) {
            free(receipt);
            ok = 0;
            break;
        }
        for (index = 0; index < state.dm2MacMoviePcm.sampleCount; ++index) {
            unsigned int source_index = (unsigned int)index * (unsigned int)rate /
                                        M11_AUDIO_SAMPLE_RATE;
            float expected;
            if (source_index >= (unsigned int)count) source_index = (unsigned int)count - 1u;
            expected = ((float)source[source_index] / 32768.0f) *
                       ((float)masters[trial] / 128.0f);
            if (fabsf(receipt[index] - expected) > 0.000001f) ok = 0;
            if (trial == 0 && receipt[index] != 0.0f) *heard_nonzero = 1;
        }
        free(receipt);
    }
    SDL_DestroyAudioStream((SDL_AudioStream *)state.sdlStream);
    free(state.dm2MacMoviePcm.samples);
    return ok;
}

int main(void)
{
    DM2_V1_MacMedia media;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    static const int movie_indices[] = {
        DM2_V1_MAC_MOVIE_TITLE,
        DM2_V1_MAC_MOVIE_SWOOSH,
        DM2_V1_MAC_MOVIE_CREDITS,
        DM2_V1_MAC_MOVIE_ENDING
    };
    static const char *const movie_names[] = {
        "Title.MooV", "Swoosh.MooV", "Credits.MooV", "Ending.MooV"
    };
    size_t movie;

    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac ZIP environment is not set");
        return 77;
    }
    memset(&media, 0, sizeof(media));
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0 ||
        media.movie_present_mask != (((uint32_t)1u << DM2_V1_MAC_MOVIE_TITLE) |
                                     ((uint32_t)1u << DM2_V1_MAC_MOVIE_SWOOSH) |
                                     ((uint32_t)1u << DM2_V1_MAC_MOVIE_CREDITS) |
                                     ((uint32_t)1u << DM2_V1_MAC_MOVIE_ENDING)) ||
        (media.movie_present_mask & ((uint32_t)1u << DM2_V1_MAC_MOVIE_STORY)) != 0u) {
        fprintf(stderr, "authentic Mac retail movie presence changed: mask=0x%08x\n",
                media.movie_present_mask);
        dm2_v1_mac_media_free(&media);
        return 1;
    }

    for (movie = 0u; movie < sizeof(movie_indices) / sizeof(movie_indices[0]); ++movie) {
        DM2_V1_MacMovieView view;
        DM2_V1_MacMovieDecoder decoder;
        int opened;
        int advanced;
        int audio_total = 0;
        int heard_nonzero = 0;
        int frame;
        uint64_t previous_time = 0u;
        const int index = movie_indices[movie];

        memset(&view, 0, sizeof(view));
        memset(&decoder, 0, sizeof(decoder));
        if (dm2_v1_mac_movie_view_build(
                media.movie[index], media.movie_size[index],
                media.movie_moov[index], media.movie_moov_size[index],
                &view) != 0) {
            fprintf(stderr, "authentic Mac %s view was not built\n",
                    movie_names[movie]);
            dm2_v1_mac_media_free(&media);
            return 1;
        }
        opened = dm2_v1_mac_movie_decoder_open(&decoder, view.bytes, view.size);
        advanced = opened && dm2_v1_mac_movie_decoder_next(&decoder);
        for (frame = 0; advanced && frame < 12; ++frame) {
            const int16_t *audio = NULL;
            int count = 0;
            int rate = 0;
            if (dm2_v1_mac_movie_decoder_take_audio(&decoder, &audio,
                                                    &count, &rate) &&
                audio && count > 0 && rate > 0) {
                audio_total += count;
                if (!check_movie_pcm_gain(audio, count, rate, &heard_nonzero)) {
                    fprintf(stderr, "authentic Mac %s master-only PCM receipt failed\n",
                            movie_names[movie]);
                    dm2_v1_mac_movie_decoder_close(&decoder);
                    dm2_v1_mac_movie_view_free(&view);
                    dm2_v1_mac_media_free(&media);
                    return 1;
                }
            }
            if (decoder.frame_duration_us == 0u ||
                (frame > 0 && decoder.presentation_time_us <= previous_time)) {
                fprintf(stderr, "authentic Mac %s has invalid frame timing: frame=%d time=%llu duration=%llu previous=%llu\n",
                        movie_names[movie], frame,
                        (unsigned long long)decoder.presentation_time_us,
                        (unsigned long long)decoder.frame_duration_us,
                        (unsigned long long)previous_time);
                dm2_v1_mac_movie_decoder_close(&decoder);
                dm2_v1_mac_movie_view_free(&view);
                dm2_v1_mac_media_free(&media);
                return 1;
            }
            previous_time = decoder.presentation_time_us;
            if (frame != 11) advanced = dm2_v1_mac_movie_decoder_next(&decoder);
        }
        if (!(opened && advanced && decoder.frame_ready &&
              decoder.width == 320 && decoder.height == 200 &&
              decoder.frame_index >= 1u && audio_total > 0 && heard_nonzero)) {
            fprintf(stderr, "authentic Mac %s decode failed: open=%d next=%d ready=%d rejected=%d ended=%d size=%zu audio=%d\n",
                    movie_names[movie], opened, advanced,
                    decoder.frame_ready, decoder.rejected, decoder.ended,
                    view.size, audio_total);
            dm2_v1_mac_movie_decoder_close(&decoder);
            dm2_v1_mac_movie_view_free(&view);
            dm2_v1_mac_media_free(&media);
            return 1;
        }
        printf("PASS: authentic Mac %s decoded: %dx%d time=%llu\n",
               movie_names[movie], decoder.width, decoder.height,
               (unsigned long long)decoder.presentation_time_us);
        dm2_v1_mac_movie_decoder_close(&decoder);
        dm2_v1_mac_movie_view_free(&view);
    }
    dm2_v1_mac_media_free(&media);
    return 0;
}
