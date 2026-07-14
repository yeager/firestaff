#include "redmcsb_f0687_f0688_img3_pc34_compat.h"

bool redmcsb_f0687_img3_get_nibble_pc34_compat(
    redmcsb_f0687_img3_stream_pc34_compat *stream, uint8_t *out_nibble)
{
    size_t pixel;

    if (stream == NULL || out_nibble == NULL || stream->bytes == NULL ||
        stream->byte_count > SIZE_MAX / 2U ||
        stream->pixel_index >= stream->byte_count * 2U) return false;
    pixel = stream->pixel_index++;
    *out_nibble = (uint8_t)(((pixel & 1U) ? stream->bytes[pixel >> 1U] :
                            (stream->bytes[pixel >> 1U] >> 4U)) & 15U);
    return true;
}

bool redmcsb_f0688_img3_get_pixel_count_pc34_compat(
    redmcsb_f0687_img3_stream_pc34_compat *stream, uint16_t *out_count)
{
    uint8_t a, b, c, d, e;

    if (out_count == NULL || !redmcsb_f0687_img3_get_nibble_pc34_compat(stream, &a))
        return false;
    if (a != 15U) {
        *out_count = (uint16_t)(a + 2U);
        return true;
    }
    if (!redmcsb_f0687_img3_get_nibble_pc34_compat(stream, &b) ||
        !redmcsb_f0687_img3_get_nibble_pc34_compat(stream, &c)) return false;
    a = (uint8_t)((b << 4U) | c);
    if (a != 255U) {
        *out_count = (uint16_t)(a + 17U);
        return true;
    }
    if (!redmcsb_f0687_img3_get_nibble_pc34_compat(stream, &b) ||
        !redmcsb_f0687_img3_get_nibble_pc34_compat(stream, &c) ||
        !redmcsb_f0687_img3_get_nibble_pc34_compat(stream, &d) ||
        !redmcsb_f0687_img3_get_nibble_pc34_compat(stream, &e)) return false;
    *out_count = (uint16_t)((b << 12U) | (c << 8U) | (d << 4U) | e);
    return true;
}
const char *redmcsb_f0687_f0688_img3_source_evidence_pc34(void){return "ReDMCSB IMAGE2.C F0687_IMG3_GetNibble (91-99); F0688_IMG3_GetPixelCount (101-116), PC I34E/I34M";}
