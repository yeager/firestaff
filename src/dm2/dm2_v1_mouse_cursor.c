#include "dm2_v1_mouse_cursor.h"

#include <limits.h>
#include <string.h>

void dm2_v1_mouse_cursor_receipt_clear(DM2_V1_MouseCursorReceipt* receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
}

void dm2_v1_mouse_cursor_state_init(DM2_V1_MouseCursorState* state)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->mouse_visibility = 1;
}

int dm2_v1_FIRE_HIDE_MOUSE_CURSOR(
    DM2_V1_MouseCursorState* state,
    DM2_V1_MouseCursorReceipt* out_receipt)
{
    dm2_v1_mouse_cursor_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "FIRE_HIDE_MOUSE_CURSOR";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:4547";
    }
    if (!state || !out_receipt) {
        if (out_receipt) out_receipt->blocked = 1;
        return 0;
    }
    if (state->ibmio_hide_depth == INT_MAX) {
        out_receipt->blocked = 1;
        return 0;
    }
    ++state->ibmio_hide_depth;
    state->mouse_visibility = 0;
    out_receipt->admitted = 1;
    out_receipt->queue_count = state->queued_count;
    return 1;
}

int dm2_v1_FIRE_QUEUE_MOUSE_EVENT(
    DM2_V1_MouseCursorState* state,
    uint16_t x,
    uint16_t y,
    uint16_t button,
    DM2_V1_MouseCursorReceipt* out_receipt)
{
    int threshold;
    int next;

    dm2_v1_mouse_cursor_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "FIRE_QUEUE_MOUSE_EVENT";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:8547";
    }
    if (!state || !out_receipt) {
        if (out_receipt) out_receipt->blocked = 1;
        return 0;
    }
    if (state->queue_locked) {
        state->deferred_pending = 1;
        state->deferred.x = x;
        state->deferred.y = y;
        state->deferred.button = button;
        out_receipt->deferred = 1;
        out_receipt->queue_count = state->queued_count;
        return 1;
    }
    state->queue_locked = 1;
    threshold = ((button == 4U && state->right_button_latched == 0) ||
                 button == 0x0040U || button == 0x0060U) ? 9 : 7;
    state->right_button_latched = 0;
    next = state->write_index + 1;
    if (next > 10) next -= 11;
    if (state->queued_count < threshold) {
        ++state->queued_count;
        state->write_index = next;
        state->events[next].button = button;
        state->events[next].x = x;
        state->events[next].y = y;
        out_receipt->admitted = 1;
    } else if (button == 2U) {
        state->right_button_latched = 1;
    }
    state->queue_locked = 0;
    out_receipt->queue_count = state->queued_count;
    return 1;
}

static uint8_t dm2_cursor_get_4bpp(
    const uint8_t* src,
    uint16_t even_width,
    uint16_t x,
    uint16_t y)
{
    uint16_t pixel = (uint16_t)(even_width * y + x);
    uint8_t packed = src[pixel >> 1];
    return (pixel & 1U) ? (uint8_t)(packed & 0x0fU) : (uint8_t)(packed >> 4);
}

int dm2_v1_IBMIO_SET_CURSOR_PATTERN(
    DM2_V1_MouseCursorState* state,
    int index,
    const uint8_t* src,
    size_t src_size,
    uint8_t hot_x,
    uint8_t hot_y,
    uint16_t src_width,
    uint16_t src_height,
    uint16_t src_bits,
    const uint8_t* local_pal,
    size_t local_pal_size,
    uint16_t colorkey,
    DM2_V1_MouseCursorReceipt* out_receipt)
{
    uint32_t pixels;
    uint32_t i;
    DM2_V1_CursorPattern* pattern;

    dm2_v1_mouse_cursor_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "IBMIO_SET_CURSOR_PATTERN";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:4595";
    }
    if (!state || !out_receipt || index < 0 || index >= 4 || !src ||
        src_width == 0U || src_height == 0U) {
        if (out_receipt) out_receipt->blocked = 1;
        return 0;
    }
    pixels = (uint32_t)src_width * (uint32_t)src_height;
    if (pixels > DM2_V1_CURSOR_PATTERN_CAPACITY) {
        out_receipt->blocked = 1;
        return 0;
    }
    pattern = &state->patterns[index];
    memset(pattern, 0, sizeof(*pattern));
    pattern->hot_x = hot_x;
    pattern->hot_y = hot_y;
    pattern->width = (uint8_t)src_width;
    pattern->height = (uint8_t)src_height;
    pattern->pixel_count = (uint16_t)pixels;
    if (src_bits == 4U) {
        uint16_t even_width = (uint16_t)((src_width + 1U) & 0xfffeU);
        size_t required = ((size_t)even_width * src_height + 1U) >> 1;
        /* DM2_INITBASICCURSORS passes its active original palette
         * (SKProject skmcursr.cpp::generate_cursor).  An identity palette
         * would turn the source nibbles into fabricated display indices. */
        if (!local_pal || local_pal_size < 16U || src_size < required) {
            out_receipt->blocked = 1;
            return 0;
        }
        for (i = 0; i < pixels; ++i) {
            uint16_t x = (uint16_t)(i % src_width);
            uint16_t y = (uint16_t)(i / src_width);
            pattern->pixels[i] =
                local_pal[dm2_cursor_get_4bpp(src, even_width, x, y)];
        }
        pattern->transparent_color = local_pal[colorkey & 0x0fU];
    } else {
        if (src_size < pixels) {
            out_receipt->blocked = 1;
            return 0;
        }
        memcpy(pattern->pixels, src, pixels);
        pattern->transparent_color = (uint8_t)colorkey;
    }
    pattern->valid = 1;
    out_receipt->admitted = 1;
    out_receipt->copied_pixels = (int)pixels;
    return 1;
}

const char* dm2_v1_mouse_cursor_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp FIRE_HIDE_MOUSE_CURSOR:4547, "
           "FIRE_QUEUE_MOUSE_EVENT:8547, and IBMIO_SET_CURSOR_PATTERN:4595";
}
