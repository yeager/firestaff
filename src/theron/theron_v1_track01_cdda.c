#include "theron_v1_track02.h"

#include <stdio.h>
#include <string.h>

#ifndef FIRESTAFF_NO_SDL_AUDIO
#include <SDL3/SDL.h>
#define THERON_HAVE_SDL_AUDIO 1
#else
#define THERON_HAVE_SDL_AUDIO 0
#endif

int theron_v1_track01_cdda_stream_start(
    const Theron_Track01CddaHandoff *handoff,
    Theron_Track01CddaStream *out_stream) {
#if THERON_HAVE_SDL_AUDIO
    FILE *audio_file;
    SDL_AudioSpec spec;
    SDL_AudioStream *sdl_stream;

    if (!handoff || !out_stream || handoff->status != THERON_TRACK01_CDDA_AVAILABLE ||
        !handoff->original_cdda || !handoff->playback_handoff_ready ||
        handoff->audio_sector_count == 0u ||
        handoff->audio_start_byte > handoff->audio_file_bytes ||
        handoff->audio_file_bytes - handoff->audio_start_byte !=
            handoff->audio_sector_count * THERON_TRACK01_CDDA_SECTOR_BYTES) {
        return 0;
    }
    memset(out_stream, 0, sizeof(*out_stream));
    audio_file = fopen(handoff->audio_path, "rb");
    if (!audio_file || fseek(audio_file, (long)handoff->audio_start_byte, SEEK_SET) != 0) {
        if (audio_file) fclose(audio_file);
        return 0;
    }
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        fclose(audio_file);
        return 0;
    }
    spec.format = SDL_AUDIO_S16LE;
    spec.channels = THERON_TRACK01_CDDA_CHANNELS;
    spec.freq = THERON_TRACK01_CDDA_SAMPLE_RATE;
    sdl_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                           &spec, NULL, NULL);
    if (!sdl_stream) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        fclose(audio_file);
        return 0;
    }
    SDL_ResumeAudioStreamDevice(sdl_stream);
    out_stream->audio_file = audio_file;
    out_stream->sdl_stream = sdl_stream;
    out_stream->audio_start_byte = handoff->audio_start_byte;
    out_stream->audio_sector_count = handoff->audio_sector_count;
    out_stream->output_started = 1;
    return 1;
#else
    (void)handoff;
    (void)out_stream;
    return 0;
#endif
}

int theron_v1_track01_cdda_stream_pump(Theron_Track01CddaStream *stream) {
#if THERON_HAVE_SDL_AUDIO
    uint8_t sector[THERON_TRACK01_CDDA_SECTOR_BYTES];
    size_t queued_bytes;
    size_t queued_sectors;

    if (!stream || !stream->output_started || !stream->audio_file ||
        !stream->sdl_stream || stream->audio_sector_count == 0u) {
        return 0;
    }
    queued_bytes = (size_t)SDL_GetAudioStreamQueued(
        (SDL_AudioStream *)stream->sdl_stream);
    queued_sectors = queued_bytes / THERON_TRACK01_CDDA_SECTOR_BYTES;
    while (queued_sectors < THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS) {
        if (stream->sectors_read == stream->audio_sector_count) {
            if (fseek((FILE *)stream->audio_file,
                      (long)stream->audio_start_byte,
                      SEEK_SET) != 0) {
                return 0;
            }
            stream->sectors_read = 0u;
            ++stream->loop_count;
        }
        if (fread(sector, 1u, sizeof(sector), (FILE *)stream->audio_file) != sizeof(sector) ||
            !SDL_PutAudioStreamData((SDL_AudioStream *)stream->sdl_stream,
                                    sector, (int)sizeof(sector))) {
            return 0;
        }
        ++stream->sectors_read;
        ++stream->sectors_queued;
        ++queued_sectors;
    }
    return 1;
#else
    (void)stream;
    return 0;
#endif
}

void theron_v1_track01_cdda_stream_stop(Theron_Track01CddaStream *stream) {
    if (!stream) return;
#if THERON_HAVE_SDL_AUDIO
    if (stream->sdl_stream) {
        SDL_DestroyAudioStream((SDL_AudioStream *)stream->sdl_stream);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    if (stream->audio_file) fclose((FILE *)stream->audio_file);
#endif
    memset(stream, 0, sizeof(*stream));
}

int theron_v1_track01_cdda_lifecycle_update(
    const Theron_Track01CddaHandoff *handoff,
    int title_active,
    Theron_Track01CddaStream *stream) {
    if (!stream) {
        return 0;
    }
    if (!title_active) {
        theron_v1_track01_cdda_stream_stop(stream);
        return 1;
    }
    if (!handoff || handoff->status != THERON_TRACK01_CDDA_AVAILABLE ||
        !handoff->original_cdda || !handoff->playback_handoff_ready) {
        theron_v1_track01_cdda_stream_stop(stream);
        return 0;
    }
    if (!stream->output_started &&
        !theron_v1_track01_cdda_stream_start(handoff, stream)) {
        return 0;
    }
    return theron_v1_track01_cdda_stream_pump(stream);
}
