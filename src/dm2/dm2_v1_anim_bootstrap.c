#include "dm2_v1_anim_bootstrap.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DM2_V1_ANIM_MAX_FILE_HANDLES 16
#define DM2_V1_ANIM_READ_CHUNK_SIZE 0x8000u

static FILE *dm2_v1_anim_files[DM2_V1_ANIM_MAX_FILE_HANDLES];

static uint32_t dm2_v1_anim_hash_bytes(const void *data, size_t size)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t hash = 2166136261u;
    size_t i;

    if (!data) return 0u;
    for (i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static void dm2_v1_anim_file_receipt_set(DM2_V1_AnimFileReceipt *receipt,
                                         const char *symbol,
                                         int source_line,
                                         int handle)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
        receipt->valid = 1;
        receipt->symbol = symbol;
        receipt->source_file = "SKWIN/SkWinCore.cpp";
        receipt->source_line = source_line;
        receipt->handle = handle;
    }
}

static void dm2_v1_anim_bootstrap_clear(DM2_V1_AnimBootstrapReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
    }
}

static int dm2_v1_anim_bootstrap_set(DM2_V1_AnimBootstrapReceipt *receipt,
                                     const char *symbol,
                                     int source_line,
                                     int argc,
                                     const char *const *argv)
{
    int i;
    size_t used = 0;

    dm2_v1_anim_bootstrap_clear(receipt);
    if (!receipt || !symbol || !argv || argc <= 0 ||
        argc > DM2_V1_ANIM_BOOTSTRAP_MAX_ARGS) {
        return 0;
    }
    receipt->valid = 1;
    receipt->symbol = symbol;
    receipt->source_file = "SKWIN/SkWinCore.cpp";
    receipt->source_line = source_line;
    receipt->argc = argc;
    for (i = 0; i < argc; ++i) {
        int wrote;
        receipt->argv[i] = argv[i];
        wrote = snprintf(receipt->command_line + used,
                         sizeof(receipt->command_line) - used,
                         "%s%s", i ? " " : "", argv[i]);
        if (wrote < 0 || (size_t)wrote >= sizeof(receipt->command_line) - used) {
            dm2_v1_anim_bootstrap_clear(receipt);
            return 0;
        }
        used += (size_t)wrote;
    }
    return 1;
}

int dm2_v1_anim_bootstrap_swoosh(DM2_V1_AnimBootstrapReceipt *out_receipt)
{
    static const char *const argv[] = {"ANIM", "swoosh", "+pm", "+sb"};
    return dm2_v1_anim_bootstrap_set(out_receipt,
                                     "ANIM_BOOTSTRAP_SWOOSH",
                                     2034,
                                     4,
                                     argv);
}

int dm2_v1_anim_bootstrap_title(DM2_V1_AnimBootstrapReceipt *out_receipt)
{
    static const char *const argv[] = {
        "ANIM", "title", "+ah", "+as", "+ab", "+pm", "+sb"
    };
    return dm2_v1_anim_bootstrap_set(out_receipt,
                                     "ANIM_BOOTSTRAP_TITLE",
                                     2045,
                                     7,
                                     argv);
}

int dm2_v1_anim_file_open(const char *filename,
                          DM2_V1_AnimFileReceipt *out_receipt)
{
    int i;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!filename) {
        return 0;
    }
    for (i = 1; i < DM2_V1_ANIM_MAX_FILE_HANDLES; ++i) {
        if (!dm2_v1_anim_files[i]) {
            FILE *fp = fopen(filename, "rb");
            if (!fp) {
                return 0;
            }
            dm2_v1_anim_files[i] = fp;
            dm2_v1_anim_file_receipt_set(
                out_receipt, "ANIM_FILE_OPEN", 915, i);
            return i;
        }
    }
    return 0;
}

uint32_t dm2_v1_anim_get_file_size(int handle,
                                   DM2_V1_AnimFileReceipt *out_receipt)
{
    FILE *fp;
    long current;
    long size;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (handle <= 0 || handle >= DM2_V1_ANIM_MAX_FILE_HANDLES ||
        !dm2_v1_anim_files[handle]) {
        return 0;
    }
    fp = dm2_v1_anim_files[handle];
    current = ftell(fp);
    if (current < 0 || fseek(fp, 0, SEEK_END) != 0) {
        return 0;
    }
    size = ftell(fp);
    if (size < 0 || fseek(fp, current, SEEK_SET) != 0) {
        return 0;
    }
    dm2_v1_anim_file_receipt_set(
        out_receipt, "ANIM_GET_FILE_SIZE", 924, handle);
    if (out_receipt) {
        out_receipt->file_size = (uint32_t)size;
    }
    return (uint32_t)size;
}

int dm2_v1_anim_read_huge_file(int handle,
                               uint32_t read_size,
                               uint8_t *buffer,
                               DM2_V1_AnimFileReceipt *out_receipt)
{
    FILE *fp;
    uint8_t *cursor = buffer;
    uint32_t remaining = read_size;
    uint32_t chunks = 0;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (handle <= 0 || handle >= DM2_V1_ANIM_MAX_FILE_HANDLES ||
        !dm2_v1_anim_files[handle] || (!buffer && read_size != 0u)) {
        return 0;
    }
    fp = dm2_v1_anim_files[handle];
    while (remaining > 0u) {
        uint32_t chunk = remaining > DM2_V1_ANIM_READ_CHUNK_SIZE
                             ? DM2_V1_ANIM_READ_CHUNK_SIZE
                             : remaining;
        if (fread(cursor, 1, chunk, fp) != chunk) {
            return 0;
        }
        cursor += chunk;
        remaining -= chunk;
        ++chunks;
    }
    dm2_v1_anim_file_receipt_set(
        out_receipt, "ANIM_READ_HUGE_FILE", 939, handle);
    if (out_receipt) {
        out_receipt->requested_bytes = read_size;
        out_receipt->transferred_bytes = read_size;
        out_receipt->chunk_count = chunks;
    }
    return 1;
}

void dm2_v1_anim_file_close(int handle,
                            DM2_V1_AnimFileReceipt *out_receipt)
{
    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (handle > 0 && handle < DM2_V1_ANIM_MAX_FILE_HANDLES &&
        dm2_v1_anim_files[handle]) {
        fclose(dm2_v1_anim_files[handle]);
        dm2_v1_anim_files[handle] = NULL;
        dm2_v1_anim_file_receipt_set(
            out_receipt, "ANIM_FILE_CLOSE", 970, handle);
    }
}

char *dm2_v1_anim_strcpy(char *dst,
                         const char *src,
                         DM2_V1_AnimFileReceipt *out_receipt)
{
    if (!dst || !src) {
        if (out_receipt) {
            memset(out_receipt, 0, sizeof(*out_receipt));
        }
        return NULL;
    }
    dm2_v1_anim_file_receipt_set(out_receipt, "ANIM_STRCPY", 984, 0);
    return strcpy(dst, src);
}

int dm2_v1_anim_toupper(int value,
                        DM2_V1_AnimFileReceipt *out_receipt)
{
    if (value == -1) {
        dm2_v1_anim_file_receipt_set(out_receipt, "ANIM_TOUPPER", 1308, 0);
        return -1;
    }
    dm2_v1_anim_file_receipt_set(out_receipt, "ANIM_TOUPPER", 1308, 0);
    return (unsigned char)toupper((unsigned char)value);
}

void *dm2_v1_anim_farmalloc(uint32_t size,
                            DM2_V1_AnimFileReceipt *out_receipt)
{
    void *buffer;

    if (size == 0u) {
        if (out_receipt) {
            memset(out_receipt, 0, sizeof(*out_receipt));
        }
        return NULL;
    }
    buffer = malloc((size_t)size);
    if (!buffer) {
        if (out_receipt) {
            memset(out_receipt, 0, sizeof(*out_receipt));
        }
        return NULL;
    }
    dm2_v1_anim_file_receipt_set(out_receipt, "ANIM_farmalloc", 934, 0);
    if (out_receipt) {
        out_receipt->requested_bytes = size;
        out_receipt->transferred_bytes = size;
    }
    return buffer;
}

void dm2_v1_anim_farfree(void *buffer,
                         DM2_V1_AnimFileReceipt *out_receipt)
{
    if (buffer) {
        free(buffer);
    }
    dm2_v1_anim_file_receipt_set(out_receipt, "ANIM_farfree", 1054, 0);
}

uint32_t dm2_v1_anim_farcoreleft(DM2_V1_AnimFileReceipt *out_receipt)
{
    dm2_v1_anim_file_receipt_set(out_receipt, "ANIM_farcoreleft", 1060, 0);
    if (out_receipt) {
        out_receipt->file_size = 1024u * 1024u;
    }
    return 1024u * 1024u;
}

void dm2_v1_anim_stream_state_init(DM2_V1_AnimStreamState *state,
                                   uint8_t *arena,
                                   uint32_t arena_size)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->arena = arena;
    state->arena_size = arena_size;
}

int dm2_v1_anim_0759_0855_reset_stream(
    DM2_V1_AnimStreamState *state,
    DM2_V1_AnimStreamResetReceipt *out_receipt)
{
    DM2_V1_AnimStreamResetReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.symbol = "_0759_0855";
    receipt.source_file = "SKWIN/SkWinCore.cpp";
    receipt.source_line = 1074;
    if (!state) {
        receipt.blocked_missing_state = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    state->active = 0u;
    state->last_offset = 0u;
    state->reset_count++;
    receipt.valid = 1;
    receipt.reset_stream = 1u;
    receipt.receipt_hash = dm2_v1_anim_hash_bytes(
        &receipt, sizeof(receipt) - sizeof(receipt.receipt_hash));
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

uint8_t *dm2_v1_anim_0759_0869_resolve_stream_ptr(
    DM2_V1_AnimStreamState *state,
    uint32_t offset,
    DM2_V1_AnimStreamPtrReceipt *out_receipt)
{
    DM2_V1_AnimStreamPtrReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.symbol = "_0759_0869";
    receipt.source_file = "SKWIN/SkWinCore.cpp";
    receipt.source_line = 1077;
    receipt.offset = offset;
    if (!state) {
        receipt.blocked_missing_state = 1u;
        if (out_receipt) *out_receipt = receipt;
        return NULL;
    }
    receipt.arena_size = state->arena_size;
    if (!state->active || !state->arena) {
        receipt.blocked_inactive_stream = 1u;
        if (out_receipt) *out_receipt = receipt;
        return NULL;
    }
    if (offset >= state->arena_size) {
        receipt.blocked_out_of_bounds = 1u;
        if (out_receipt) *out_receipt = receipt;
        return NULL;
    }
    state->last_offset = offset;
    receipt.ptr = state->arena + offset;
    receipt.resolved_pointer = 1u;
    receipt.valid = 1;
    receipt.receipt_hash = dm2_v1_anim_hash_bytes(
        &receipt, sizeof(receipt) - sizeof(receipt.receipt_hash));
    if (out_receipt) *out_receipt = receipt;
    return receipt.ptr;
}

static int dm2_v1_anim_arg_is(const char *arg, char flag)
{
    return arg && arg[0] == '+' && dm2_v1_anim_toupper(arg[1], NULL) == 'A' &&
           dm2_v1_anim_toupper(arg[2], NULL) == flag;
}

int dm2_v1_anim_0759_08e7_plan_main(
    const DM2_V1_AnimMainRequest *request,
    DM2_V1_AnimStreamState *stream_state,
    DM2_V1_AnimMainReceipt *out_receipt)
{
    DM2_V1_AnimMainReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.symbol = "_0759_08e7";
    receipt.source_file = "SKWIN/SkWinCore.cpp";
    receipt.source_line = 1565;
    receipt.volume = 0x0fu;
    if (!request) {
        receipt.blocked_missing_request = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.argc = request->argc;
    for (int i = 1; i < request->argc &&
                    i < DM2_V1_ANIM_BOOTSTRAP_MAX_ARGS; ++i) {
        const char *arg = request->argv[i];
        if (dm2_v1_anim_arg_is(arg, 'B')) receipt.flag_abort_on_event = 1u;
        else if (dm2_v1_anim_arg_is(arg, 'L')) {
            receipt.flag_abort_on_event = 1u;
            receipt.flag_loop = 1u;
        } else if (dm2_v1_anim_arg_is(arg, 'D')) {
            receipt.flag_no_display = 1u;
        } else if (dm2_v1_anim_arg_is(arg, 'H')) {
            receipt.flag_hide_after = 1u;
        } else if (dm2_v1_anim_arg_is(arg, 'E')) {
            receipt.flag_clear_after = 1u;
        } else if (dm2_v1_anim_arg_is(arg, 'S')) {
            receipt.flag_clear_before = 1u;
        } else if (dm2_v1_anim_arg_is(arg, 'M')) {
            receipt.flag_force_stream = 1u;
        } else if (dm2_v1_anim_arg_is(arg, 'F')) {
            receipt.flag_preload_file = 1u;
        } else if (dm2_v1_anim_arg_is(arg, 'V') && arg[3] >= '0' &&
                   arg[3] <= '9') {
            receipt.volume = (uint8_t)(arg[3] - '0');
            if (receipt.volume > 15u) receipt.volume = 15u;
        } else if (dm2_v1_anim_arg_is(arg, 'O') && arg[3] >= '0' &&
                   arg[3] <= '9') {
            receipt.output_channel = (uint16_t)(arg[3] - '0');
        } else if (!receipt.parsed_filename && arg && arg[0] != '+') {
            receipt.parsed_filename = 1u;
        }
    }
    if (request->input_name && request->input_name[0] != '\0')
        receipt.parsed_filename = 1u;
    receipt.input_size = request->input_size;
    receipt.farcoreleft_bytes = request->farcoreleft_bytes;
    receipt.stream_capacity = request->stream_capacity;
    receipt.requested_capture_int_ff = 1u;
    receipt.requested_install_timer = 1u;
    receipt.drained_pending_events = 1u;
    if (!receipt.parsed_filename || request->input_size == 0u) {
        receipt.blocked_missing_input = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.allocated_screen_buffer = 1u;
    if (request->sound_available) {
        receipt.requested_sound_reload = 1u;
        receipt.allocated_sound_buffer = request->sound_reload_ok ? 1u : 0u;
    }
    if (receipt.flag_force_stream ||
        request->farcoreleft_bytes < request->input_size) {
        DM2_V1_AnimStreamPtrReceipt ptr_receipt;
        receipt.stream_mode = 1u;
        if (!stream_state || request->stream_capacity < request->input_size) {
            receipt.blocked_stream_capacity = 1u;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        stream_state->active = 1u;
        stream_state->arena_size = request->stream_capacity;
        if (!dm2_v1_anim_0759_0869_resolve_stream_ptr(
                stream_state, 0u, &ptr_receipt)) {
            receipt.blocked_stream_capacity = 1u;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        receipt.requested_0759_0869 = 1u;
        receipt.first_stream_offset = ptr_receipt.offset;
    } else {
        receipt.heap_mode = 1u;
    }
    receipt.requested_palette_zero = 1u;
    receipt.requested_screen_clear_before = receipt.flag_clear_before;
    receipt.requested_screen_clear_after = receipt.flag_clear_after;
    if (receipt.stream_mode) {
        DM2_V1_AnimStreamResetReceipt reset_receipt;
        dm2_v1_anim_0759_0855_reset_stream(stream_state, &reset_receipt);
        receipt.requested_0759_0855 = reset_receipt.valid ? 1u : 0u;
    }
    if (receipt.flag_preload_file || receipt.allocated_sound_buffer)
        receipt.requested_mouse_sound_release = 1u;
    receipt.valid = 1;
    receipt.receipt_hash = dm2_v1_anim_hash_bytes(
        &receipt, sizeof(receipt) - sizeof(receipt.receipt_hash));
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_anim_setpixel_seq_4bpp(uint8_t *dst,
                                  size_t dst_size,
                                  uint16_t pixel_offset,
                                  uint8_t color)
{
    size_t byte_offset = (size_t)pixel_offset >> 1;
    uint8_t nibble = (uint8_t)(color & 0x0fu);

    if (!dst || byte_offset >= dst_size) {
        return 0;
    }
    if ((pixel_offset & 1u) != 0u) {
        dst[byte_offset] = (uint8_t)((dst[byte_offset] & 0xf0u) | nibble);
    } else {
        dst[byte_offset] = (uint8_t)((dst[byte_offset] & 0x0fu) |
                                     (uint8_t)(nibble << 4));
    }
    return 1;
}

int dm2_v1_anim_fill_seq_4bpp(uint8_t *dst,
                              size_t dst_size,
                              uint16_t pixel_offset,
                              uint8_t color,
                              uint16_t count)
{
    uint16_t i;

    if (count == 0u) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (!dm2_v1_anim_setpixel_seq_4bpp(
                dst, dst_size, (uint16_t)(pixel_offset + i), color)) {
            return 0;
        }
    }
    return 1;
}

static int dm2_v1_anim_get_nibble(const uint8_t *src,
                                  size_t src_size,
                                  size_t pixel_offset,
                                  uint8_t *out_color)
{
    size_t byte_offset = pixel_offset >> 1;
    if (!src || !out_color || byte_offset >= src_size) {
        return 0;
    }
    if ((pixel_offset & 1u) != 0u) {
        *out_color = (uint8_t)(src[byte_offset] & 0x0fu);
    } else {
        *out_color = (uint8_t)((src[byte_offset] >> 4) & 0x0fu);
    }
    return 1;
}

static int dm2_v1_anim_copy_literal_nibbles(const uint8_t *src,
                                            size_t src_size,
                                            size_t src_offset,
                                            uint8_t *dst,
                                            size_t dst_size,
                                            uint16_t pixel_offset,
                                            uint16_t count)
{
    uint16_t i;
    for (i = 0; i < count; ++i) {
        uint8_t color;
        if (!dm2_v1_anim_get_nibble(src, src_size, (size_t)i, &color) ||
            !dm2_v1_anim_setpixel_seq_4bpp(
                dst, dst_size, (uint16_t)(pixel_offset + i), color)) {
            return 0;
        }
    }
    (void)src_offset;
    return 1;
}

int dm2_v1_anim_decode_img1(const uint8_t *src,
                            size_t src_size,
                            uint8_t *dst,
                            size_t dst_size,
                            DM2_V1_AnimDecodeImg1Receipt *out_receipt)
{
    size_t pos = 4;
    uint16_t width;
    uint16_t height;
    uint16_t even_width;
    uint32_t total_pixels;
    uint32_t di = 0;
    uint32_t previous_flush = 0;
    DM2_V1_AnimDecodeImg1Receipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!src || !dst || src_size < 4u) {
        return 0;
    }
    width = (uint16_t)(((uint16_t)src[0] << 8) | src[1]);
    height = (uint16_t)(((uint16_t)src[2] << 8) | src[3]);
    even_width = (uint16_t)((width + 1u) & 0xfffeu);
    total_pixels = (uint32_t)even_width * (uint32_t)height;
    if (width == 0u || height == 0u ||
        ((total_pixels + 1u) >> 1) > dst_size) {
        return 0;
    }

    while (di < total_pixels) {
        uint8_t op;
        uint16_t count;
        if (pos >= src_size) {
            return 0;
        }
        op = src[pos++];
        if ((op & 0x80u) == 0u) {
            count = (uint16_t)((op >> 4) + 1u);
            if (di + count > total_pixels ||
                !dm2_v1_anim_fill_seq_4bpp(
                    dst, dst_size, (uint16_t)di, op, count)) {
                return 0;
            }
            di += count;
            ++receipt.fill_run_count;
            continue;
        }

        switch (op & 0x30u) {
        case 0x00u:
            if ((op & 0x40u) == 0u) {
                if (pos >= src_size) return 0;
                count = (uint16_t)(src[pos++] + 1u);
            } else {
                if (pos + 1u >= src_size) return 0;
                count = (uint16_t)(((uint16_t)src[pos] << 8) |
                                   src[pos + 1u]);
                pos += 2u;
                ++count;
            }
            if (di + count > total_pixels ||
                !dm2_v1_anim_fill_seq_4bpp(
                    dst, dst_size, (uint16_t)di, op, count)) {
                return 0;
            }
            di += count;
            ++receipt.fill_run_count;
            break;
        case 0x10u:
            if ((op & 0x40u) == 0u) {
                if (pos >= src_size) return 0;
                count = (uint16_t)(src[pos++] + 1u);
            } else {
                if (pos + 1u >= src_size) return 0;
                count = (uint16_t)(((uint16_t)src[pos] << 8) |
                                   src[pos + 1u]);
                pos += 2u;
                ++count;
            }
            if ((count & 1u) != 0u) {
                if (di >= total_pixels ||
                    !dm2_v1_anim_setpixel_seq_4bpp(
                        dst, dst_size, (uint16_t)di, op)) {
                    return 0;
                }
                ++di;
                --count;
            }
            if (di + count > total_pixels ||
                pos + ((size_t)count >> 1) > src_size ||
                !dm2_v1_anim_copy_literal_nibbles(
                    src + pos, src_size - pos, pos, dst, dst_size,
                    (uint16_t)di, count)) {
                return 0;
            }
            pos += (size_t)count >> 1;
            di += count;
            ++receipt.literal_run_count;
            break;
        case 0x20u:
            count = (uint16_t)(((op >> 2) & 16u) | (op & 15u));
            if (count == 0x1du) {
                if (pos >= src_size) return 0;
                count = (uint16_t)(src[pos++] + 1u);
            } else if (count == 0x1eu) {
                if (pos >= src_size) return 0;
                count = (uint16_t)(src[pos++] + 0x101u);
            } else if (count == 0x1fu) {
                if (pos + 1u >= src_size) return 0;
                count = (uint16_t)(((uint16_t)src[pos] << 8) |
                                   src[pos + 1u]);
                pos += 2u;
                ++count;
            } else {
                ++count;
            }
            if (di + count > total_pixels) {
                return 0;
            }
            di += count;
            previous_flush = di;
            ++receipt.skip_run_count;
            break;
        case 0x30u:
            if ((op & 0x40u) == 0u) {
                if (pos >= src_size) return 0;
                count = (uint16_t)(src[pos++] + 1u);
            } else {
                if (pos + 1u >= src_size) return 0;
                count = (uint16_t)(((uint16_t)src[pos] << 8) |
                                   src[pos + 1u]);
                pos += 2u;
                ++count;
            }
            if (di < even_width || di + count + 1u > total_pixels) {
                return 0;
            }
            {
                uint16_t i;
                for (i = 0; i < count; ++i) {
                    uint8_t color;
                    if (!dm2_v1_anim_get_nibble(
                            dst, dst_size, (size_t)(di - even_width + i),
                            &color) ||
                        !dm2_v1_anim_setpixel_seq_4bpp(
                            dst, dst_size, (uint16_t)(di + i), color)) {
                        return 0;
                    }
                }
            }
            di += count;
            if (!dm2_v1_anim_setpixel_seq_4bpp(
                    dst, dst_size, (uint16_t)di, op)) {
                return 0;
            }
            ++di;
            ++receipt.previous_row_run_count;
            break;
        default:
            return 0;
        }
    }

    receipt.valid = 1;
    receipt.symbol = "ANIM_DECODE_IMG1";
    receipt.source_file = "SKWIN/SkWinCore.cpp";
    receipt.source_line = 1110;
    receipt.width = width;
    receipt.height = height;
    receipt.even_width = even_width;
    receipt.decoded_pixels = di;
    receipt.consumed_bytes = (uint32_t)pos;
    (void)previous_flush;
    if (out_receipt) {
        *out_receipt = receipt;
    }
    return 1;
}

int dm2_v1_anim_blit_to_memory_row_4to4bpp(const uint8_t *src,
                                           size_t src_size,
                                           uint16_t off_src,
                                           uint8_t *dst,
                                           size_t dst_size,
                                           uint16_t off_dst,
                                           uint16_t width,
                                           DM2_V1_AnimFileReceipt *out_receipt)
{
    uint16_t i;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!src || !dst || width == 0u) {
        return 0;
    }
    for (i = 0; i < width; ++i) {
        uint8_t color;
        if (!dm2_v1_anim_get_nibble(src,
                                    src_size,
                                    (size_t)off_src + i,
                                    &color) ||
            !dm2_v1_anim_setpixel_seq_4bpp(dst,
                                           dst_size,
                                           (uint16_t)(off_dst + i),
                                           color)) {
            return 0;
        }
    }
    dm2_v1_anim_file_receipt_set(
        out_receipt, "ANIM_BLIT_TO_MEMORY_ROW_4TO4BPP", 1399, 0);
    if (out_receipt) {
        out_receipt->requested_bytes = width;
        out_receipt->transferred_bytes = width;
    }
    return 1;
}
