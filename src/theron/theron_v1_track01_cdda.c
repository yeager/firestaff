#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_NO_SDL_AUDIO
#include <SDL3/SDL.h>
#define THERON_HAVE_SDL_AUDIO 1
#else
#define THERON_HAVE_SDL_AUDIO 0
#endif

#if defined(FIRESTAFF_HAVE_VORBISFILE)
#include <vorbis/vorbisfile.h>
#define THERON_HAVE_VORBISFILE 1
#else
#define THERON_HAVE_VORBISFILE 0
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
        (!handoff->audio_is_vorbis &&
         (handoff->audio_sector_count == 0u ||
          handoff->audio_start_byte > handoff->audio_file_bytes ||
          handoff->audio_file_bytes - handoff->audio_start_byte !=
              handoff->audio_sector_count * THERON_TRACK01_CDDA_SECTOR_BYTES))) {
        return 0;
    }
    memset(out_stream, 0, sizeof(*out_stream));
    audio_file = fopen(handoff->audio_path, "rb");
    if (!audio_file || (!handoff->audio_is_vorbis &&
                        fseek(audio_file, (long)handoff->audio_start_byte, SEEK_SET) != 0)) {
        if (audio_file) fclose(audio_file);
        return 0;
    }
#if !THERON_HAVE_VORBISFILE
    if (handoff->audio_is_vorbis) {
        fclose(audio_file);
        return 0;
    }
#endif
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
#if THERON_HAVE_VORBISFILE
    if (handoff->audio_is_vorbis) {
        OggVorbis_File *vorbis = (OggVorbis_File *)calloc(1u, sizeof(*vorbis));
        vorbis_info *info;
        if (!vorbis || ov_open_callbacks(audio_file, vorbis, NULL, 0u,
                                         OV_CALLBACKS_DEFAULT) != 0) {
            free(vorbis);
            SDL_DestroyAudioStream(sdl_stream);
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            fclose(audio_file);
            return 0;
        }
        info = ov_info(vorbis, -1);
        if (!info || info->rate != THERON_TRACK01_CDDA_SAMPLE_RATE ||
            info->channels != THERON_TRACK01_CDDA_CHANNELS) {
            ov_clear(vorbis);
            SDL_DestroyAudioStream(sdl_stream);
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return 0;
        }
        out_stream->audio_file = vorbis;
    } else {
        out_stream->audio_file = audio_file;
    }
#else
    out_stream->audio_file = audio_file;
#endif
    SDL_ResumeAudioStreamDevice(sdl_stream);
    out_stream->sdl_stream = sdl_stream;
    out_stream->audio_start_byte = handoff->audio_start_byte;
    out_stream->audio_sector_count = handoff->audio_sector_count;
    out_stream->audio_is_vorbis = handoff->audio_is_vorbis;
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
#if THERON_HAVE_VORBISFILE
    if (stream && stream->audio_is_vorbis) {
        uint8_t pcm[THERON_TRACK01_CDDA_SECTOR_BYTES];
        int queued_bytes;
        if (!stream->output_started || !stream->sdl_stream || !stream->audio_file) return 0;
        queued_bytes = SDL_GetAudioStreamQueued((SDL_AudioStream *)stream->sdl_stream);
        while (queued_bytes < (int)(THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS *
                                    THERON_TRACK01_CDDA_SECTOR_BYTES)) {
            int bitstream = 0;
            long decoded = ov_read((OggVorbis_File *)stream->audio_file,
                                   (char *)pcm, (int)sizeof(pcm), 0, 2, 1,
                                   &bitstream);
            (void)bitstream;
            if (decoded == 0) {
                if (ov_pcm_seek((OggVorbis_File *)stream->audio_file, 0) != 0) {
                    return 0;
                }
                ++stream->loop_count;
                continue;
            }
            if (decoded < 0 ||
                !SDL_PutAudioStreamData((SDL_AudioStream *)stream->sdl_stream,
                                        pcm, (int)decoded)) {
                return 0;
            }
            queued_bytes += (int)decoded;
            ++stream->sectors_queued;
        }
        return 1;
    }
#endif
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
#if THERON_HAVE_VORBISFILE
    if (stream->audio_is_vorbis) {
        if (stream->audio_file) {
            ov_clear((OggVorbis_File *)stream->audio_file);
            free(stream->audio_file);
        }
    } else
#endif
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
