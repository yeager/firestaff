#include "audio_sdl_m11.h"
#include "dm2_v1_mac_media.h"
#include "dm2_v1_mac_sound.h"

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int fnv1a(const unsigned char *bytes, size_t count)
{
    unsigned int hash = 2166136261u;
    size_t index;
    for (index = 0; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

int main(void)
{
    DM2_V1_MacMedia media;
    DM2_V1_MacSoundSample sample;
    M11_AudioState audio;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    int source_rate;
    SDL_AudioDeviceID playback_device = 0;
    int queued_audio_bytes = 0;
    int initial_audio_bytes = 0;
    unsigned int hash;
    int nonzero_samples = 0;
    int64_t centered_sum = 0;
    int ok = 1;
    int8_t *signed_pcm = NULL;
    int require_native_audio = getenv("FIRESTAFF_REQUIRE_NATIVE_AUDIO") != NULL;

    {
        FILE *archive = zip && zip[0] ? fopen(zip, "rb") : NULL;
        if (!archive) {
            puts("SKIP: authentic DM2 Mac retail ZIP is not staged");
            return 77;
        }
        fclose(archive);
    }
    memset(&media, 0, sizeof(media));
    memset(&audio, 0, sizeof(audio));
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0) {
        fprintf(stderr, "authentic Mac media parse failed\n");
        return 1;
    }
    if (dm2_v1_mac_sound_find(
            media.sound_resource_fork[DM2_V1_MAC_SOUND_GENERAL],
            media.sound_resource_fork_size[DM2_V1_MAC_SOUND_GENERAL],
            10001, &sample) != 0 &&
        dm2_v1_mac_sound_find(media.application_resource,
                              media.application_resource_size,
                              6000, &sample) != 0) {
        fprintf(stderr, "authentic Mac sound transport source not found\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    if (!sample.valid || sample.encode != 0u ||
        sample.sample_data_size == 0u || sample.sample_rate_fixed == 0u ||
        sample.sample_data_size > 120000u) {
        fprintf(stderr, "authentic Mac sound transport source not found\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    signed_pcm = (int8_t *)malloc(sample.sample_data_size);
    if (!signed_pcm) {
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    /* A nil samplePtr means the standard Macintosh sample area stores
     * unsigned 8-bit values with 0x80 as silence.  SDL's source queue accepts
     * signed 8-bit PCM, so center each authentic sample before transport. */
    for (size_t i = 0; i < sample.sample_data_size; ++i) {
        signed_pcm[i] = (int8_t)((int)sample.sample_data[i] - 128);
        centered_sum += signed_pcm[i];
    }
    if (sample.format == 2u && sample.resource_id == 6000 &&
        (centered_sum < -(int64_t)sample.sample_data_size ||
         centered_sum > (int64_t)sample.sample_data_size)) {
        fprintf(stderr, "Mac format-2 sample was not centered around silence\n");
        free(signed_pcm);
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    source_rate = (int)((sample.sample_rate_fixed + 0x8000u) >> 16);
    hash = fnv1a((const unsigned char *)signed_pcm, sample.sample_data_size);
    ok &= M11_Audio_Init(&audio);
    if (require_native_audio &&
        (audio.backend != M11_AUDIO_BACKEND_SDL3 || !audio.sdlStream)) {
        puts("SKIP: native Mac audio output is unavailable");
        M11_Audio_Shutdown(&audio);
        free(signed_pcm);
        dm2_v1_mac_media_free(&media);
        return 77;
    }
    ok &= M11_Audio_PlayDm2MacSndPcm(&audio,
                                     signed_pcm,
                                     (int)sample.sample_data_size,
                                     source_rate, sample.resource_id, hash);
    ok &= audio.dm2MacSndAccepted &&
          audio.dm2MacSndByteCount == (int)sample.sample_data_size &&
          audio.dm2MacSndRateHz == source_rate &&
          audio.dm2MacSndResourceId == sample.resource_id &&
          audio.dm2MacSndHash == hash && audio.dm2MacSndPcm.sampleCount > 0;
    for (int i = 0; i < audio.dm2MacSndPcm.sampleCount; ++i) {
        if (audio.dm2MacSndPcm.samples[i] != 0.0f) ++nonzero_samples;
    }
    ok &= audio.backend == M11_AUDIO_BACKEND_SDL3 && audio.sdlStream != NULL &&
          audio.dm2MacSndQueuedCount == 1 && nonzero_samples > 0;
    if (audio.sdlStream) {
        playback_device = SDL_GetAudioStreamDevice(
            (SDL_AudioStream *)audio.sdlStream);
        queued_audio_bytes = SDL_GetAudioStreamQueued(
            (SDL_AudioStream *)audio.sdlStream);
    }
    initial_audio_bytes = queued_audio_bytes;
    ok &= playback_device != 0 && queued_audio_bytes >= 0;
    if (require_native_audio && playback_device != 0 &&
        SDL_AudioDevicePaused(playback_device)) {
        fprintf(stderr, "native Mac audio device remained paused\n");
        ok = 0;
    }
    /* A successful SDL_PutAudioStreamData only proves queue admission. Give
     * the selected device time to consume this authentic Macintosh sound. */
    if (playback_device != 0 && queued_audio_bytes > 0) {
        int queue_tail_tolerance = initial_audio_bytes / 100;
        if (queue_tail_tolerance < 32) queue_tail_tolerance = 32;
        for (int wait_ms = 0; wait_ms < 10000 &&
             queued_audio_bytes > queue_tail_tolerance;
             wait_ms += 10) {
            SDL_Delay(10);
            queued_audio_bytes = SDL_GetAudioStreamQueued(
                (SDL_AudioStream *)audio.sdlStream);
            if (queued_audio_bytes < 0) {
                ok = 0;
                break;
            }
        }
        if (queued_audio_bytes > queue_tail_tolerance) {
            fprintf(stderr,
                    "Mac audio device did not consume PCM queue (initial=%d remaining=%d bytes device=%u paused=%d)\n",
                    initial_audio_bytes, queued_audio_bytes,
                    (unsigned)playback_device,
                    SDL_AudioDevicePaused(playback_device));
            ok = 0;
        }
    }
    if (!ok) {
        fprintf(stderr,
                "authentic Mac snd PCM playback queue failed (backend=%d stream=%d accepted=%d queued=%d samples=%d nonzero=%d)\n",
                audio.backend, audio.sdlStream != NULL,
                audio.dm2MacSndAccepted, audio.dm2MacSndQueuedCount,
                audio.dm2MacSndPcm.sampleCount, nonzero_samples);
    }
    M11_Audio_Shutdown(&audio);
    free(signed_pcm);
    dm2_v1_mac_media_free(&media);
    if (!ok) return 1;
    printf("PASS: authentic Mac snd PCM transported: format=%u id=%d bytes=%zu rate=%d\n",
           sample.format, sample.resource_id, sample.sample_data_size,
           source_rate);
    return 0;
}
