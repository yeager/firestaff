#include "dm1_v1_i34e_title_anim_pc34.h"

#include <string.h>

#define I34E_TITLE_MAX_BYTES (1024u * 1024u)
#define I34E_TITLE_MAX_RECORDS 512u
#define I34E_PC34_SCREEN_PIXELS 64000u
#define I34E_PC34_LFSR_STEPS 65535u

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

int dm1_v1_i34e_title_anim_pc34_init(DM1_V1_I34ETitleAnimPc34 *state,
                                     const uint8_t *data, size_t size) {
    if (!state) return 0;
    memset(state, 0, sizeof(*state));
    if (!data || size < 14u || size > I34E_TITLE_MAX_BYTES ||
        data[0] != 'A' || data[1] != 'N' ||
        read_be16(data + 6u) != 320u ||
        read_be16(data + 8u) != 200u ||
        read_be16(data + 10u) != 4u) return 0;
    state->data = data;
    state->size = size;
    state->offset = 14u;
    state->width = 320u;
    state->height = 200u;
    state->valid = 1;
    return 1;
}

int dm1_v1_i34e_title_anim_pc34_step(
    DM1_V1_I34ETitleAnimPc34 *state,
    DM1_V1_I34ETitleAnimFramePc34 *out_frame) {
    size_t records = 0u;
    if (!state || !out_frame || !state->valid || state->finished) return 0;
    memset(out_frame, 0, sizeof(*out_frame));
    while (state->offset < state->size && records++ < I34E_TITLE_MAX_RECORDS) {
        size_t pos = state->offset;
        uint16_t bytes;
        uint16_t attributes;
        const uint8_t *payload;
        size_t payload_bytes;
        size_t next;
        if (state->size - pos < 6u) goto malformed;
        bytes = read_be16(state->data + pos + 2u);
        attributes = read_be16(state->data + pos + 4u);
        if ((size_t)bytes > state->size - pos - 6u) goto malformed;
        next = pos + 6u + (size_t)bytes;
        payload = state->data + pos + 6u;
        payload_bytes = bytes;
        state->offset = next;

        if (state->data[pos] == 'B' && state->data[pos + 1u] == 'R') {
            state->break_allowed = attributes != 0u;
            continue;
        }
        if (state->data[pos] == 'P' && state->data[pos + 1u] == '8') {
            /* PC34 F8294_SetSingleColorInPalette is F8170_Return0. */
            continue;
        }
        if (state->data[pos] == 'P' && state->data[pos + 1u] == 'L') {
            unsigned int i;
            if (bytes < 66u || read_be16(payload) != 16u) goto malformed;
            for (i = 0u; i < 16u; ++i) {
                uint8_t index = payload[2u + i * 4u];
                if (index >= 16u || payload[3u + i * 4u] > 15u ||
                    payload[4u + i * 4u] > 15u ||
                    payload[5u + i * 4u] > 15u) goto malformed;
                /* ReDMCSB ANIM.C F8292_ dispatches to VGA VIDEODRV.C
                 * F8230, which writes (channel << 2) + 3 into the DAC6
                 * palette. Preserve both the expansion and offset. */
                state->palette[index].r =
                    (uint8_t)((payload[3u + i * 4u] << 2) + 3u);
                state->palette[index].g =
                    (uint8_t)((payload[4u + i * 4u] << 2) + 3u);
                state->palette[index].b =
                    (uint8_t)((payload[5u + i * 4u] << 2) + 3u);
            }
            continue;
        }
        if (state->data[pos] == 'D' && state->data[pos + 1u] == 'O') {
            if (state->offset != state->size) goto malformed;
            state->finished = 1;
            return 0;
        }
        if ((state->data[pos] == 'E' && state->data[pos + 1u] == 'N') ||
            (state->data[pos] == 'D' && state->data[pos + 1u] == 'L')) {
            size_t prefix = bytes >= 2u && payload[0] == 0xffu &&
                            payload[1] == 0x81u ? 2u : 0u;
            if (payload_bytes < prefix + 5u ||
                !csb_v1_fmtowns_anm_decode_f8288(
                    payload + prefix, payload_bytes - prefix,
                    state->pixels, sizeof(state->pixels),
                    state->width, state->height)) goto malformed;
            out_frame->pixels = state->pixels;
            out_frame->palette = state->palette;
            out_frame->width = state->width;
            out_frame->height = state->height;
            /* ReDMCSB ANIM.C:F2275 clamps every EN/DL frame to at least five
             * VBlanks before decoding it. F8298 decrements this from the
             * platform's vertical-blank callback. */
            out_frame->source_delay_vblanks = attributes < 5u ? 5u : attributes;
            out_frame->frame_index = state->frame_index++;
            out_frame->break_allowed = state->break_allowed;
            return 1;
        }
        /* ReDMCSB advances over unhandled record types by their declared size. */
    }
    if (state->offset == state->size) {
        state->finished = 1;
        return 0;
    }
malformed:
    state->valid = 0;
    return -1;
}

void dm1_v1_i34e_title_erase_pc34_init(DM1_V1_I34ETitleErasePc34 *state) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->lfsr = 1u;
    state->valid = 1;
}

int dm1_v1_i34e_title_erase_pc34_step(DM1_V1_I34ETitleErasePc34 *state,
                                      uint8_t *pixels, size_t pixel_count,
                                      uint32_t max_steps) {
    uint32_t remaining;
    uint32_t count;
    if (!state || !state->valid || !pixels ||
        pixel_count < I34E_PC34_SCREEN_PIXELS || max_steps == 0u)
        return -1;
    if (state->finished) return 0;
    remaining = I34E_PC34_LFSR_STEPS - state->lfsr_steps;
    count = max_steps < remaining ? max_steps : remaining;
    while (count--) {
        uint16_t pixel = state->lfsr;
        if (pixel < I34E_PC34_SCREEN_PIXELS) {
            pixels[pixel] = 0u;
            ++state->pixels_cleared;
        }
        state->lfsr = (pixel & 1u)
            ? (uint16_t)((pixel >> 1) ^ 0xb400u)
            : (uint16_t)(pixel >> 1);
        ++state->lfsr_steps;
    }
    if (state->lfsr_steps == I34E_PC34_LFSR_STEPS) {
        /* The source LFSR visits every nonzero 16-bit state and clears pixel
         * zero explicitly after it wraps back to its seed. */
        pixels[0] = 0u;
        ++state->pixels_cleared;
        state->finished = 1;
        return 0;
    }
    return 1;
}
