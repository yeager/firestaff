#include "dm2_v1_mac_movie_decoder.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(FIRESTAFF_HAVE_FFMPEG)
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

typedef struct {
    const uint8_t *bytes;
    size_t size;
    size_t position;
} MacMovieIo;

typedef struct {
    MacMovieIo io;
    AVFormatContext *format;
    AVCodecContext *codec;
    AVIOContext *avio;
    AVFrame *frame;
    AVPacket *packet;
    struct SwsContext *scale;
    SwrContext *audio_scale;
    AVCodecContext *audio_codec;
    AVFrame *audio_frame;
    int audio_stream;
    int audio_rate;
    int16_t *audio_samples;
    int audio_sample_count;
    int audio_capacity;
    uint8_t *rgb;
    int video_stream;
    int width;
    int height;
    AVRational time_base;
} MacMovieImpl;

static int mac_movie_read(void *opaque, uint8_t *buffer, int size)
{
    MacMovieIo *io = (MacMovieIo *)opaque;
    size_t remaining;
    if (!io || !buffer || size <= 0 || io->position >= io->size)
        return AVERROR_EOF;
    remaining = io->size - io->position;
    if ((size_t)size > remaining) size = (int)remaining;
    memcpy(buffer, io->bytes + io->position, (size_t)size);
    io->position += (size_t)size;
    return size;
}

static int64_t mac_movie_seek(void *opaque, int64_t offset, int whence)
{
    MacMovieIo *io = (MacMovieIo *)opaque;
    int64_t target;
    if (!io) return -1;
    if ((whence & ~AVSEEK_FORCE) == AVSEEK_SIZE)
        return (int64_t)io->size;
    switch (whence & ~AVSEEK_FORCE) {
    case SEEK_SET: target = offset; break;
    case SEEK_CUR: target = (int64_t)io->position + offset; break;
    case SEEK_END: target = (int64_t)io->size + offset; break;
    default: return -1;
    }
    if (target < 0 || (uint64_t)target > (uint64_t)io->size) return -1;
    io->position = (size_t)target;
    return target;
}

static void mac_movie_palette(uint8_t palette[256][3])
{
    int index;
    for (index = 0; index < 256; ++index) {
        palette[index][0] = (uint8_t)(((index >> 5) & 7) * 63 / 7);
        palette[index][1] = (uint8_t)(((index >> 2) & 7) * 63 / 7);
        palette[index][2] = (uint8_t)((index & 3) * 63 / 3);
    }
}

static int mac_movie_convert(MacMovieImpl *impl,
                             DM2_V1_MacMovieDecoder *decoder)
{
    int y;
    if (!impl || !decoder || !impl->frame || !impl->rgb) return 0;
    if (!impl->scale) {
        impl->scale = sws_getContext(
            impl->width, impl->height, (enum AVPixelFormat)impl->frame->format,
            impl->width, impl->height, AV_PIX_FMT_RGB24,
            SWS_POINT, NULL, NULL, NULL);
        if (!impl->scale) return 0;
    }
    if (sws_scale(impl->scale, (const uint8_t * const *)impl->frame->data,
                  impl->frame->linesize, 0, impl->height,
                  &impl->rgb, (int[4]){impl->width * 3, 0, 0, 0}) <= 0)
        return 0;
    for (y = 0; y < 200; ++y) {
        int sy = y * impl->height / 200;
        int x;
        for (x = 0; x < 320; ++x) {
            int sx = x * impl->width / 320;
            const uint8_t *pixel = impl->rgb + (size_t)sy * (size_t)impl->width * 3u +
                                   (size_t)sx * 3u;
            decoder->pixels[(size_t)y * 320u + (size_t)x] =
                (uint8_t)((pixel[0] >> 5u << 5u) |
                          (pixel[1] >> 5u << 2u) |
                          (pixel[2] >> 6u));
        }
    }
    mac_movie_palette(decoder->palette_rgb6);
    return 1;
}

static int mac_movie_audio_append(MacMovieImpl *impl, const AVFrame *frame)
{
    int output_count;
    int needed;
    int16_t *target;
    if (!impl || !frame || !impl->audio_scale || impl->audio_rate <= 0)
        return 0;
    output_count = swr_get_out_samples(impl->audio_scale, frame->nb_samples);
    if (output_count <= 0 || output_count > 1000000) return 0;
    needed = impl->audio_sample_count + output_count;
    if (needed > impl->audio_capacity) {
        int capacity = impl->audio_capacity ? impl->audio_capacity : 4096;
        while (capacity < needed) {
            if (capacity > 1000000 / 2) { capacity = needed; break; }
            capacity *= 2;
        }
        target = (int16_t *)realloc(impl->audio_samples,
                                    (size_t)capacity * sizeof(*target));
        if (!target) return 0;
        impl->audio_samples = target;
        impl->audio_capacity = capacity;
    }
    {
        uint8_t *output = (uint8_t *)(impl->audio_samples +
                                      impl->audio_sample_count);
        uint8_t *output_planes[1] = { output };
        output_count = swr_convert(
            impl->audio_scale, output_planes, output_count,
            (const uint8_t * const *)frame->extended_data, frame->nb_samples);
    }
    if (output_count < 0) return 0;
    impl->audio_sample_count += output_count;
    return 1;
}

int dm2_v1_mac_movie_decoder_open(DM2_V1_MacMovieDecoder *decoder,
                                  const uint8_t *movie_bytes,
                                  size_t movie_size)
{
    MacMovieImpl *impl;
    const AVCodec *codec;
    unsigned char *buffer;
    if (!decoder) return 0;
    memset(decoder, 0, sizeof(*decoder));
    if (!movie_bytes || movie_size == 0u) return 0;
    impl = (MacMovieImpl *)calloc(1, sizeof(*impl));
    if (!impl) return 0;
    impl->io.bytes = movie_bytes;
    impl->io.size = movie_size;
    impl->format = avformat_alloc_context();
    buffer = (unsigned char *)av_malloc(4096u);
    if (!impl->format || !buffer) goto fail;
    impl->avio = avio_alloc_context(buffer, 4096, 0, &impl->io,
                                    mac_movie_read, NULL, mac_movie_seek);
    if (!impl->avio) goto fail;
    impl->format->pb = impl->avio;
    impl->format->flags |= AVFMT_FLAG_CUSTOM_IO;
    if (avformat_open_input(&impl->format, NULL, NULL, NULL) < 0 ||
        avformat_find_stream_info(impl->format, NULL) < 0) goto fail;
    impl->video_stream = av_find_best_stream(impl->format, AVMEDIA_TYPE_VIDEO,
                                             -1, -1, &codec, 0);
    if (impl->video_stream < 0 || !codec) goto fail;
    impl->codec = avcodec_alloc_context3(codec);
    if (!impl->codec || avcodec_parameters_to_context(
            impl->codec, impl->format->streams[impl->video_stream]->codecpar) < 0 ||
        avcodec_open2(impl->codec, codec, NULL) < 0) goto fail;
    impl->width = impl->codec->width;
    impl->height = impl->codec->height;
    if (impl->width <= 0 || impl->height <= 0) goto fail;
    impl->time_base = impl->format->streams[impl->video_stream]->time_base;
    impl->frame = av_frame_alloc();
    impl->packet = av_packet_alloc();
    impl->rgb = (uint8_t *)av_malloc((size_t)impl->width * (size_t)impl->height * 3u);
    if (!impl->frame || !impl->packet || !impl->rgb) goto fail;
    impl->audio_stream = av_find_best_stream(impl->format, AVMEDIA_TYPE_AUDIO,
                                              -1, -1, &codec, 0);
    if (impl->audio_stream >= 0 && codec) {
        AVStream *stream = impl->format->streams[impl->audio_stream];
        const AVCodecParameters *parameters = stream->codecpar;
        AVChannelLayout output_layout = AV_CHANNEL_LAYOUT_MONO;
        impl->audio_codec = avcodec_alloc_context3(codec);
        if (!impl->audio_codec ||
            avcodec_parameters_to_context(impl->audio_codec, parameters) < 0 ||
            avcodec_open2(impl->audio_codec, codec, NULL) < 0) goto fail;
        impl->audio_rate = impl->audio_codec->sample_rate;
        if (impl->audio_rate <= 0) goto fail;
        if (swr_alloc_set_opts2(
                &impl->audio_scale, &output_layout, AV_SAMPLE_FMT_S16,
                impl->audio_rate, &impl->audio_codec->ch_layout,
                impl->audio_codec->sample_fmt, impl->audio_rate, 0, NULL) < 0)
            impl->audio_scale = NULL;
        impl->audio_frame = av_frame_alloc();
        if (!impl->audio_scale || !impl->audio_frame ||
            swr_init(impl->audio_scale) < 0) goto fail;
    }
    decoder->opaque = impl;
    decoder->width = impl->width;
    decoder->height = impl->height;
    return 1;
fail:
    decoder->opaque = impl;
    dm2_v1_mac_movie_decoder_close(decoder);
    return 0;
}

int dm2_v1_mac_movie_decoder_next(DM2_V1_MacMovieDecoder *decoder)
{
    MacMovieImpl *impl;
    if (!decoder || !(impl = (MacMovieImpl *)decoder->opaque) || decoder->ended)
        return 0;
    for (;;) {
        int result = av_read_frame(impl->format, impl->packet);
        if (result < 0) {
            avcodec_send_packet(impl->codec, NULL);
            decoder->ended = 1;
            return 0;
        }
        if (impl->packet->stream_index != impl->video_stream) {
            if (impl->audio_codec &&
                impl->packet->stream_index == impl->audio_stream) {
                result = avcodec_send_packet(impl->audio_codec, impl->packet);
                if (result >= 0) {
                    while ((result = avcodec_receive_frame(
                                impl->audio_codec, impl->audio_frame)) >= 0) {
                        if (!mac_movie_audio_append(impl, impl->audio_frame)) {
                            decoder->rejected = 1;
                            av_packet_unref(impl->packet);
                            return 0;
                        }
                    }
                }
            }
            av_packet_unref(impl->packet);
            continue;
        }
        result = avcodec_send_packet(impl->codec, impl->packet);
        av_packet_unref(impl->packet);
        if (result < 0) continue;
        for (;;) {
            result = avcodec_receive_frame(impl->codec, impl->frame);
            if (result == AVERROR(EAGAIN)) break;
            if (result < 0) { decoder->rejected = 1; return 0; }
            if (!mac_movie_convert(impl, decoder)) {
                decoder->rejected = 1;
                return 0;
            }
            if (impl->frame->best_effort_timestamp == AV_NOPTS_VALUE ||
                impl->frame->best_effort_timestamp < 0 ||
                impl->frame->duration <= 0) {
                decoder->rejected = 1;
                return 0;
            }
            decoder->frame_index++;
            decoder->frame_ready = 1;
            decoder->presentation_time_us = av_rescale_q(
                impl->frame->best_effort_timestamp,
                impl->time_base, (AVRational){1, 1000000});
            decoder->frame_duration_us = av_rescale_q(
                impl->frame->duration, impl->time_base,
                (AVRational){1, 1000000});
            return 1;
        }
    }
}

void dm2_v1_mac_movie_decoder_close(DM2_V1_MacMovieDecoder *decoder)
{
    MacMovieImpl *impl;
    if (!decoder || !(impl = (MacMovieImpl *)decoder->opaque)) return;
    av_free(impl->rgb);
    free(impl->audio_samples);
    av_frame_free(&impl->audio_frame);
    swr_free(&impl->audio_scale);
    avcodec_free_context(&impl->audio_codec);
    av_packet_free(&impl->packet);
    av_frame_free(&impl->frame);
    sws_freeContext(impl->scale);
    avcodec_free_context(&impl->codec);
    if (impl->format) avformat_close_input(&impl->format);
    if (impl->avio) avio_context_free(&impl->avio);
    free(impl);
    memset(decoder, 0, sizeof(*decoder));
}

int dm2_v1_mac_movie_decoder_take_audio(DM2_V1_MacMovieDecoder *decoder,
                                        const int16_t **samples,
                                        int *sample_count,
                                        int *rate_hz)
{
    MacMovieImpl *impl;
    if (samples) *samples = NULL;
    if (sample_count) *sample_count = 0;
    if (rate_hz) *rate_hz = 0;
    if (!decoder || !(impl = (MacMovieImpl *)decoder->opaque) ||
        impl->audio_sample_count <= 0) return 0;
    if (samples) *samples = impl->audio_samples;
    if (sample_count) *sample_count = impl->audio_sample_count;
    if (rate_hz) *rate_hz = impl->audio_rate;
    decoder->audio_samples = impl->audio_samples;
    decoder->audio_sample_count = impl->audio_sample_count;
    decoder->audio_rate_hz = impl->audio_rate;
    impl->audio_sample_count = 0;
    return 1;
}

#else

int dm2_v1_mac_movie_decoder_open(DM2_V1_MacMovieDecoder *decoder,
                                  const uint8_t *movie_bytes,
                                  size_t movie_size)
{
    (void)movie_bytes; (void)movie_size;
    if (decoder) { memset(decoder, 0, sizeof(*decoder)); decoder->rejected = 1; }
    return 0;
}
int dm2_v1_mac_movie_decoder_next(DM2_V1_MacMovieDecoder *decoder)
{
    (void)decoder; return 0;
}
int dm2_v1_mac_movie_decoder_take_audio(DM2_V1_MacMovieDecoder *decoder,
                                        const int16_t **samples,
                                        int *sample_count, int *rate_hz)
{
    (void)decoder;
    if (samples) *samples = NULL;
    if (sample_count) *sample_count = 0;
    if (rate_hz) *rate_hz = 0;
    return 0;
}
void dm2_v1_mac_movie_decoder_close(DM2_V1_MacMovieDecoder *decoder)
{
    if (decoder) memset(decoder, 0, sizeof(*decoder));
}

#endif
