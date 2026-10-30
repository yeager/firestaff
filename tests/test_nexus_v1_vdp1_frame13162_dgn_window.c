#include "firestaff_x68k_media_receipt.h"
#include "nexus_v1_vdp1_dgn_material_resolver.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRAME_INDEX 13162U
#define FIRST_COMMAND 8U
#define LAST_COMMAND 40U
#define VDP1_VRAM_BYTES 0x80000U
#define COMMAND_BYTES 32U

typedef struct { uint8_t *data; size_t size; } Blob;

static Blob read_blob(const char *path)
{
    Blob out = {NULL, 0U};
    FILE *file = fopen(path, "rb");
    long length;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) < 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return out;
    }
    out.data = (uint8_t *)malloc((size_t)length + 1U);
    if (!out.data || fread(out.data, 1U, (size_t)length, file) != (size_t)length) {
        free(out.data); out.data = NULL; fclose(file); return out;
    }
    fclose(file);
    out.data[length] = 0U;
    out.size = (size_t)length;
    return out;
}

static void release_blob(Blob *blob)
{
    free(blob->data); blob->data = NULL; blob->size = 0U;
}

static int blob_hash_is(const Blob *blob, const char *expected)
{
    char actual[65];
    return blob->data && firestaff_x68k_media_receipt_sha256_hex(
        blob->data, blob->size, actual, sizeof(actual)) == 0 &&
        strcmp(actual, expected) == 0;
}

static int manifest_has(const Blob *manifest, const char *relative,
                        const char *expected)
{
    const char *line = (const char *)manifest->data;
    size_t path_size = strlen(relative);
    while (line && *line) {
        const char *end = strchr(line, '\n');
        size_t length = end ? (size_t)(end - line) : strlen(line);
        if (length == 66U + path_size && memcmp(line, expected, 64U) == 0 &&
            line[64] == ' ' && line[65] == ' ' &&
            memcmp(line + 66U, relative, path_size) == 0) return 1;
        line = end ? end + 1 : NULL;
    }
    return 0;
}

static int write_rgba_capture(const char *path,
    const Nexus_V1_Vdp1RgbaCaptureFramebuffer *framebuffer)
{
    FILE *file;
    size_t pixel;
    if (!path || !*path || !framebuffer) return 0;
    file = fopen(path, "wb");
    if (!file) return 0;
    for (pixel = 0U; pixel < NEXUS_FB_W * NEXUS_FB_H; ++pixel) {
        uint32_t argb = framebuffer->rgba_buffer[pixel];
        uint8_t rgba[4] = {(uint8_t)(argb >> 16U), (uint8_t)(argb >> 8U),
            (uint8_t)argb, (uint8_t)(argb >> 24U)};
        if (fwrite(rgba, 1U, sizeof(rgba), file) != sizeof(rgba)) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0;
}

static int execution_receipt_contains_window(const Blob *receipt)
{
    const char *cursor = strstr((const char *)receipt->data,
                                "\"executed_command_indices\": [");
    uint8_t found[LAST_COMMAND + 1U] = {0};
    if (!cursor) return 0;
    cursor = strchr(cursor, '[');
    if (!cursor) return 0;
    ++cursor;
    while (*cursor && *cursor != ']') {
        char *end = NULL;
        unsigned long value;
        while (isspace((unsigned char)*cursor) || *cursor == ',') ++cursor;
        if (*cursor == ']') break;
        value = strtoul(cursor, &end, 10);
        if (end == cursor) return 0;
        if (value <= LAST_COMMAND) found[value] = 1;
        cursor = end;
    }
    if (!found[6U] || !found[7U]) return 0;
    for (size_t i = FIRST_COMMAND; i <= LAST_COMMAND; ++i)
        if (!found[i]) return 0;
    return 1;
}

static void make_capture_path(char *out, size_t capacity,
                              const char *root, const char *relative)
{
    (void)snprintf(out, capacity, "%s/%s", root, relative);
}

static uint16_t read_le16(const uint8_t *p);

static int verify_window(const Blob *trace, const Blob *vram,
                        const Blob *dgn, unsigned int *out_mode1,
                        unsigned int *out_mode5)
{
    char *copy = (char *)malloc(trace->size + 1U);
    char *line;
    char *line_save = NULL;
    uint8_t found[LAST_COMMAND - FIRST_COMMAND + 1U] = {0};
    Nexus_V1_Vdp1DgnMaterialResolverInput resolver_input;
    Nexus_V1_Vdp1DirectColorFramebuffer direct_framebuffer;
    Nexus_V1_Vdp1DirectColorCaptureReceipt direct_receipt;
    Nexus_V1_Vdp1CaptureCompositeInput rgba_commands[
        LAST_COMMAND - FIRST_COMMAND + 1U];
    uint8_t rgba_command_bytes[
        LAST_COMMAND - FIRST_COMMAND + 1U][COMMAND_BYTES];
    uint32_t rgba_trace_indices[LAST_COMMAND - FIRST_COMMAND + 1U];
    int rgba_origin_x[LAST_COMMAND - FIRST_COMMAND + 1U];
    int rgba_origin_y[LAST_COMMAND - FIRST_COMMAND + 1U];
    uint8_t rgba_origin_verified[
        LAST_COMMAND - FIRST_COMMAND + 1U];
    Nexus_V1_Vdp1RgbaCaptureFramebuffer *rgba_framebuffer = NULL;
    Nexus_V1_Vdp1RgbaCaptureSequenceInput rgba_sequence_input;
    Nexus_V1_Vdp1RgbaCaptureSequenceReceipt rgba_sequence_receipt;
    unsigned int rgba_command_count = 0U;
    unsigned int mode1_joined = 0U, mode5_joined = 0U;
    unsigned int mode5_empty_decoded = 0U;
    unsigned int mode1_seen = 0U, mode5_seen = 0U;
    int ok = 1;

    if (!copy || vram->size != VDP1_VRAM_BYTES) { free(copy); return 0; }
    memcpy(copy, trace->data, trace->size);
    copy[trace->size] = '\0';
    memset(&resolver_input, 0, sizeof(resolver_input));
    resolver_input.dgn_bytes = dgn->data;
    resolver_input.dgn_byte_count = (int)dgn->size;
    resolver_input.source_hash_verified = 1;
    resolver_input.palette_slot_base = 0;

    line = strtok_r(copy, "\n", &line_save);
    while ((line = strtok_r(NULL, "\n", &line_save)) != NULL) {
        char *fields[22];
        char *field_save = NULL;
        char *field;
        size_t count = 0U, i;
        unsigned long frame, index, address;
        uint8_t command[COMMAND_BYTES];
        Nexus_V1_Vdp1TextureCommand parsed;
        Nexus_V1_Vdp1CaptureCompositeInput material;
        int resolved;

        field = strtok_r(line, ",", &field_save);
        while (field && count < sizeof(fields) / sizeof(fields[0])) {
            fields[count++] = field;
            field = strtok_r(NULL, ",", &field_save);
        }
        if (count != 22U) continue;
        frame = strtoul(fields[0], NULL, 10);
        index = strtoul(fields[1], NULL, 10);
        if (frame != FRAME_INDEX || index < FIRST_COMMAND || index > LAST_COMMAND)
            continue;
        i = (size_t)(index - FIRST_COMMAND);
        if (found[i]) { ok = 0; break; }
        address = strtoul(fields[2], NULL, 0);
        if ((address & (COMMAND_BYTES - 1U)) != 0U ||
            address > vram->size - COMMAND_BYTES) { ok = 0; break; }
        for (size_t word = 0U; word < 16U; ++word) {
            unsigned long value = strtoul(fields[3U + word], NULL, 0);
            command[word * 2U] = (uint8_t)value;
            command[word * 2U + 1U] = (uint8_t)(value >> 8U);
        }
        if (memcmp(command, vram->data + address, COMMAND_BYTES) != 0 ||
            nexus_v1_vdp1_texture_command_parse(command, COMMAND_BYTES,
                                                 &parsed) != 0 ||
            !parsed.texture_command || parsed.command_type != 2U ||
            !parsed.texture_source_range_valid ||
            (parsed.control & 0x4000U) != 0U) { ok = 0; break; }
        if (parsed.colour_mode == 1U) ++mode1_seen;
        else if (parsed.colour_mode == 5U) ++mode5_seen;
        else { ok = 0; break; }

        memset(&material, 0, sizeof(material));
        resolved = nexus_v1_vdp1_dgn_material_resolver(
            vram->data, (int)vram->size, command, COMMAND_BYTES, &parsed,
            (uint32_t)address, &material, &resolver_input);
        if (parsed.colour_mode == 1U) {
            if (!resolved || !material.dgn_source_hash_verified ||
                !material.dgn_image || !material.dgn_palette ||
                material.dgn_image_size != (int)parsed.texture_byte_count ||
                material.dgn_palette_size != 32) { ok = 0; break; }
            if (rgba_command_count >=
                    LAST_COMMAND - FIRST_COMMAND + 1U) { ok = 0; break; }
            {
                uint32_t palette_offset =
                    (((uint32_t)parsed.colour_control & ~UINT32_C(3)) << 3U);
                Nexus_V1_Vdp1CaptureCompositeInput *rgba_command =
                    &rgba_commands[rgba_command_count];
                if (palette_offset > vram->size - 32U) { ok = 0; break; }
                memcpy(rgba_command_bytes[rgba_command_count], command,
                       COMMAND_BYTES);
                *rgba_command = material;
                rgba_command->command =
                    rgba_command_bytes[rgba_command_count];
                rgba_command->command_size = COMMAND_BYTES;
                rgba_command->texture_span = vram->data +
                    parsed.texture_source_byte_offset;
                rgba_command->texture_span_size =
                    (int)parsed.texture_byte_count;
                rgba_command->palette_state = vram->data + palette_offset;
                rgba_command->palette_state_size = 32;
                rgba_command->original_saturn_capture_verified = 1;
                rgba_command->screen_origin_x =
                    (int)strtol(fields[20], NULL, 0);
                rgba_command->screen_origin_y =
                    (int)strtol(fields[21], NULL, 0);
                rgba_command->system_clip_state_verified = 1;
                rgba_command->system_clip_x = 320;
                rgba_command->system_clip_y = 224;
                if ((parsed.draw_mode & UINT16_C(0x0400)) != 0U) {
                    rgba_command->user_clip_state_verified = 1;
                    rgba_command->user_clip_x0 = 0;
                    rgba_command->user_clip_y0 = 0;
                    rgba_command->user_clip_x1 = 320;
                    rgba_command->user_clip_y1 = 224;
                }
                rgba_trace_indices[rgba_command_count] = (uint32_t)index;
                rgba_origin_x[rgba_command_count] =
                    rgba_command->screen_origin_x;
                rgba_origin_y[rgba_command_count] =
                    rgba_command->screen_origin_y;
                rgba_origin_verified[rgba_command_count] = 1U;
                ++rgba_command_count;
            }
            ++mode1_joined;
        } else {
            int exact_dgn_join = index != 8U && index != 9U;
            if (exact_dgn_join &&
                (!resolved || !material.dgn_direct_source_hash_verified ||
                 !material.dgn_direct_image ||
                 material.dgn_direct_image_size !=
                    (int)parsed.texture_byte_count)) {
                ok = 0; break;
            }
            if (index == 8U || index == 9U || index == 13U ||
                index == 14U || index == 28U || index == 29U ||
                index == 33U || index == 34U) {
                /* Rows 8 and 9 have authentic captured bytes but an
                 * ambiguous retail-fill source match. Decode them from the
                 * attested Saturn capture only; do not claim DGN ownership. */
                if (!exact_dgn_join) memset(&material, 0, sizeof(material));
                material.command = command;
                material.command_size = COMMAND_BYTES;
                material.texture_span = vram->data +
                    parsed.texture_source_byte_offset;
                material.texture_span_size = (int)parsed.texture_byte_count;
                material.original_saturn_capture_verified = 1;
                material.screen_origin_x = (int)strtol(fields[20], NULL, 0);
                material.screen_origin_y = (int)strtol(fields[21], NULL, 0);
                material.system_clip_state_verified = 1;
                material.system_clip_x = 320;
                material.system_clip_y = 224;
                for (size_t byte = 0U; byte < parsed.texture_byte_count;
                     byte += 2U) {
                    if (read_le16(material.texture_span + byte) != 0x000fU) {
                        ok = 0;
                        break;
                    }
                }
                memset(&direct_framebuffer, 0, sizeof(direct_framebuffer));
                if (!ok || !nexus_v1_vdp1_capture_decode_direct_color(
                        &direct_framebuffer, &material, &direct_receipt) ||
                    !direct_receipt.valid ||
                    (exact_dgn_join &&
                     !direct_receipt.source_join_verified) ||
                    (!exact_dgn_join &&
                     direct_receipt.source_join_verified) ||
                    direct_receipt.written_pixels != 0 ||
                    direct_receipt.transparent_pixels == 0) {
                    fprintf(stderr, "frame-13162 mode-5 row %lu must decode "
                        "as an authenticated all-transparent draw\n", index);
                    ok = 0;
                    break;
                }
                ++mode5_empty_decoded;
            }
            if (index != 8U && index != 9U) ++mode5_joined;
        }
        found[i] = 1;
    }
    free(copy);
    for (size_t i = 0U; i < sizeof(found); ++i)
    if (!found[i]) ok = 0;
    if (mode5_empty_decoded != 8U) ok = 0;
    if (ok && mode1_joined > 0U) {
        rgba_framebuffer = (Nexus_V1_Vdp1RgbaCaptureFramebuffer *)
            calloc(1U, sizeof(*rgba_framebuffer));
        memset(&rgba_sequence_input, 0, sizeof(rgba_sequence_input));
        memset(&rgba_sequence_receipt, 0, sizeof(rgba_sequence_receipt));
        if (!rgba_framebuffer || rgba_command_count != mode1_joined) {
            ok = 0;
        } else {
            rgba_sequence_input.commands = rgba_commands;
            rgba_sequence_input.vdp1_vram = vram->data;
            rgba_sequence_input.vdp1_vram_size = (int)vram->size;
            rgba_sequence_input.vdp1_vram_hash_verified = 1;
            rgba_sequence_input.command_trace_indices = rgba_trace_indices;
            rgba_sequence_input.command_origin_x = rgba_origin_x;
            rgba_sequence_input.command_origin_y = rgba_origin_y;
            rgba_sequence_input.command_origin_verified =
                rgba_origin_verified;
            rgba_sequence_input.command_count = (int)rgba_command_count;
            rgba_sequence_input.trace_draw_command_count =
                LAST_COMMAND - FIRST_COMMAND + 1U;
            rgba_sequence_input.trace_row_count = 174;
            rgba_sequence_input.trace_complete_verified = 1;
            rgba_sequence_input.command_order_verified = 1;
            rgba_sequence_input.local_coordinate_state_verified = 1;
            rgba_sequence_input.end_record_verified = 1;
            rgba_sequence_input.system_clip_state_verified = 1;
            rgba_sequence_input.system_clip_x = 320;
            rgba_sequence_input.system_clip_y = 224;
            ok = nexus_v1_vdp1_capture_composite_mode1_rgba_sequence(
                rgba_framebuffer, &rgba_sequence_input,
                &rgba_sequence_receipt) && rgba_sequence_receipt.valid &&
                rgba_sequence_receipt.capture_only &&
                rgba_sequence_receipt.command_order_verified &&
                rgba_sequence_receipt.trace_complete_verified &&
                rgba_sequence_receipt.end_record_verified &&
                rgba_sequence_receipt.local_coordinate_state_verified &&
                rgba_sequence_receipt.system_clip_state_verified &&
                rgba_sequence_receipt.command_count == (int)rgba_command_count &&
                rgba_sequence_receipt.source_joins_verified ==
                    (int)rgba_command_count &&
                rgba_sequence_receipt.palette_joins_verified ==
                    (int)rgba_command_count &&
                rgba_sequence_receipt.capture_gap_commands ==
                    (int)(LAST_COMMAND - FIRST_COMMAND + 1U - rgba_command_count) &&
                rgba_sequence_receipt.gouraud_commands_applied ==
                    (int)rgba_command_count &&
                rgba_sequence_receipt.gouraud_commands_unrendered == 0 &&
                rgba_sequence_receipt.user_clip_commands_verified ==
                    (int)rgba_command_count &&
                !rgba_sequence_receipt.renderer_permitted;
            if (ok) {
                const char *rgba_output = getenv(
                    "FIRESTAFF_NEXUS_FRAME13162_MODE1_RGBA_OUT");
                if (rgba_output && *rgba_output &&
                    !write_rgba_capture(rgba_output, rgba_framebuffer))
                    ok = 0;
                if (ok) printf("RGBA CAPTURE: mode1=%u gaps=%d "
                    "gouraud_applied=%d pixels_written=%d "
                    "renderer_permitted=false\n", rgba_command_count,
                    rgba_sequence_receipt.capture_gap_commands,
                    rgba_sequence_receipt.gouraud_commands_applied,
                    rgba_sequence_receipt.written_pixels);
            }
        }
        free(rgba_framebuffer);
    }
    *out_mode1 = mode1_joined;
    *out_mode5 = mode5_joined;
    return ok && mode1_seen == mode1_joined &&
           mode5_seen == mode5_joined + 2U;
}

/* Resolve the state at the start of rows 8..40 against Ymir's VDP1 rules.
 * The active inclusive user rectangle is [0,0]..[320,224]; all target draws
 * enable inside clipping, so every pixel in the captured 320x224 viewport is
 * accepted by user clipping. */
static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8U);
}

static int verify_window_user_clip_accepts_viewport(const Blob *trace,
                                                    const Blob *vram)
{
    char *copy = (char *)malloc(trace->size + 1U);
    char *line, *line_save = NULL;
    unsigned int seen = 0U;
    unsigned int gouraud_uniform = 0U;
    unsigned int gouraud_non_neutral = 0U;
    unsigned long current_index = 0U;
    int clip_found = 0, local_found = 0, unexpected_state = 0;
    int ok = 0;

    if (!copy) goto done;
    memcpy(copy, trace->data, trace->size);
    copy[trace->size] = '\0';
    line = strtok_r(copy, "\n", &line_save);
    while ((line = strtok_r(NULL, "\n", &line_save)) != NULL) {
        char *fields[22], *field_save = NULL, *field;
        size_t count = 0U, i;
        unsigned long frame, index, address;
        uint8_t command[COMMAND_BYTES];
        Nexus_V1_Vdp1TextureCommand parsed;
        int command_type;

        field = strtok_r(line, ",", &field_save);
        while (field && count < sizeof(fields) / sizeof(fields[0])) {
            fields[count++] = field;
            field = strtok_r(NULL, ",", &field_save);
        }
        if (count != 22U) continue;
        frame = strtoul(fields[0], NULL, 10);
        index = strtoul(fields[1], NULL, 10);
        current_index = index;
        if (frame != FRAME_INDEX || index > LAST_COMMAND)
            continue;
        address = strtoul(fields[2], NULL, 0);
        if ((address & (COMMAND_BYTES - 1U)) != 0U ||
            address > vram->size - COMMAND_BYTES) goto done;
        for (i = 0U; i < 16U; ++i) {
            unsigned long word = strtoul(fields[3U + i], NULL, 0);
            command[i * 2U] = (uint8_t)word;
            command[i * 2U + 1U] = (uint8_t)(word >> 8U);
        }
        if (memcmp(command, vram->data + address, COMMAND_BYTES) != 0)
            goto done;
        command_type = command[0] & 0x0fU;
        if (index == 6U) {
            if (command_type != 8 ||
                read_le16(command + 12U) != 0U ||
                read_le16(command + 14U) != 0U ||
                read_le16(command + 20U) != 320U ||
                read_le16(command + 22U) != 224U) goto done;
            clip_found = 1;
        } else if (index == 7U) {
            if (command_type != 10 ||
                read_le16(command + 12U) != 160U ||
                read_le16(command + 14U) != 112U) goto done;
            local_found = 1;
        } else if (index > 7U && index <= LAST_COMMAND &&
                   (command_type == 8 || command_type == 11)) {
            unexpected_state = 1;
        }
        if (index >= FIRST_COMMAND && index <= LAST_COMMAND) {
            uint32_t table_offset;
            uint16_t gouraud[4];
            if (!clip_found || !local_found || command_type != 2 ||
                nexus_v1_vdp1_texture_command_parse(command, COMMAND_BYTES,
                                                     &parsed) != 0 ||
                !parsed.texture_command ||
                !(parsed.draw_mode & (1U << 10U)) ||
                (parsed.draw_mode & (1U << 9U)) != 0U ||
                (int)strtol(fields[20], NULL, 0) != 160 ||
                (int)strtol(fields[21], NULL, 0) != 112) goto done;
            if ((parsed.draw_mode & (1U << 2U)) == 0U) goto done;
            table_offset = (uint32_t)parsed.gouraud_table_word << 3U;
            if (table_offset > vram->size - 8U) goto done;
            for (i = 0U; i < 4U; ++i)
                gouraud[i] = read_le16(vram->data + table_offset + i * 2U);
            if (gouraud[0] == gouraud[1] && gouraud[0] == gouraud[2] &&
                gouraud[0] == gouraud[3]) {
                ++gouraud_uniform;
                if (gouraud[0] != UINT16_C(0x4210)) ++gouraud_non_neutral;
            }
            ++seen;
        }
    }
    if (seen != LAST_COMMAND - FIRST_COMMAND + 1U ||
        gouraud_uniform != seen || gouraud_non_neutral != seen || !clip_found ||
        !local_found || unexpected_state) goto done;
    printf("CLIP PROOF: rows %u..%u use inside clipping against the captured "
           "viewport rectangle 0,0..320,224; no viewport pixel is user-clipped\n",
           FIRST_COMMAND, LAST_COMMAND);
    printf("GOURAUD PROOF: all %u draws enable non-neutral Gouraud shading "
           "with four identical captured vertex colors (diagnostic "
           "simplification only)\n", gouraud_non_neutral);
    ok = 1;
done:
    if (!ok) {
        fprintf(stderr, "clip-state details: stopped_at=%lu seen=%u clip=%d local=%d unexpected=%d\n",
                current_index, seen, clip_found, local_found, unexpected_state);
    }
    free(copy);
    return ok;
}

int main(int argc, char **argv)
{
    char path[4096];
    const char *root;
    Blob manifest = {0}, trace = {0}, vram = {0}, execution = {0}, dgn = {0};
    Blob composition_rgb = {0};
    unsigned int mode1_joined = 0U, mode5_joined = 0U;
    int result = 1;
    static const struct { const char *name; const char *sha; } inputs[] = {
        {"./dumps/vdp1-command-trace.csv", "a963e0d6e7667ac4a947a546933d0b306ea5caf13a511702a898c30472c12a88"},
        {"./vdp1-command-execution-frame13162.json", "2417d30d651f385ef384881d3b14524576069c22ac1f2fe53f71f9938e3355b4"},
        {"./vdp1-vram-command-trace-reconstructed-le.bin", "90b85d0358c7605c8186828d2e42ab46412f504f12c40348197a46db8f770913"}
    };
    if (argc != 4) {
        fprintf(stderr, "usage: %s <frame-13162-capture-root> <LEV00.DGN> <frame-trace.csv>\n", argv[0]);
        return 2;
    }
    root = argv[1];
#define LOAD(var, relative) do { \
        make_capture_path(path, sizeof(path), root, relative); \
        var = read_blob(path); \
        if (!var.data) { fprintf(stderr, "SKIP: missing local Ymir evidence %s\n", path); result = 77; goto done; } \
    } while (0)
    LOAD(manifest, "SHA256SUMS.txt");
    trace = read_blob(argv[3]);
    if (!trace.data || !blob_hash_is(&trace,
            "aa0cd750413a4c6618f91bbeb5953703c65cbc5d8481499135167f858421aebb")) {
        fprintf(stderr, "frame-13162 trace fixture missing or hash mismatch\n");
        goto done;
    }
    LOAD(vram, "vdp1-vram-command-trace-reconstructed-le.bin");
    LOAD(execution, "vdp1-command-execution-frame13162.json");
    LOAD(composition_rgb, "vdp12-frame13162-command171.rgb");
#undef LOAD
    dgn = read_blob(argv[2]);
    if (!dgn.data) {
        fprintf(stderr, "SKIP: authentic LEV00.DGN is not available at %s\n", argv[2]);
        result = 77;
        goto done;
    }
    if (!blob_hash_is(&dgn,
            "24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a")) {
        fprintf(stderr, "SKIP: canonical retail LEV00.DGN is unavailable\n");
        result = 77;
        goto done;
    }
    for (size_t i = 0U; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        Blob *blob = i == 0U ? NULL : (i == 1U ? &execution : &vram);
        if ((blob && !blob_hash_is(blob, inputs[i].sha)) ||
            !manifest_has(&manifest, inputs[i].name, inputs[i].sha)) {
            fprintf(stderr, "capture manifest/hash mismatch: %s\n", inputs[i].name);
            goto done;
        }
    }
    if (vram.size != VDP1_VRAM_BYTES) {
        fprintf(stderr, "invalid reconstructed VDP1 VRAM size\n");
        goto done;
    }
    if (!execution_receipt_contains_window(&execution)) {
        fprintf(stderr, "execution receipt does not contain clip/local state and rows 8..40\n");
        goto done;
    }
    if (!verify_window(&trace, &vram, &dgn, &mode1_joined, &mode5_joined)) {
        fprintf(stderr, "frame-13162 command window 8..40 failed source checks\n");
        goto done;
    }
    if (!verify_window_user_clip_accepts_viewport(&trace, &vram)) {
        fprintf(stderr, "frame-13162 command window user-clip state proof failed\n");
        goto done;
    }
    if (composition_rgb.size != 320U * 224U * 3U ||
        !blob_hash_is(&composition_rgb,
            "05e8aaaa5cf7459a9ff93589c45ddc681be837748c0a344461f246cd61b18875") ||
        !manifest_has(&manifest, "./vdp12-frame13162-command171.rgb",
            "05e8aaaa5cf7459a9ff93589c45ddc681be837748c0a344461f246cd61b18875")) {
        fprintf(stderr, "frame-13162 verified composition reference failed its manifest/hash\n");
        goto done;
    }
    printf("MODE-5 PROOF: eight authenticated direct-color draws have an "
           "all-transparent 0x000f source and decode to zero opaque writes\n");
    printf("PASS: frame-13162 rows 8..40 checked against authentic captures and LEV00.DGN "
           "(mode1=%u mode5=%u) with their active user-clip and local-origin "
           "state verified; the retained "
           "FBS+NBG0+row171 composition reference is hash-verified; "
           "renderer_permitted=false\n", mode1_joined, mode5_joined);
    result = 0;
done:
    release_blob(&manifest); release_blob(&trace); release_blob(&vram);
    release_blob(&execution); release_blob(&dgn);
    release_blob(&composition_rgb);
    return result;
}
