#include "firestaff_x68k_media_receipt.h"
#include "nexus_v1_vdp1_dgn_material_resolver.h"
#include "nexus_v1_vdp1_ymir_stepper_capture.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRAME_INDEX 1605UL
#define EXPECTED_ROWS 151UL
#define EXPECTED_TEXTURE_DRAWS 138U
#define EXPECTED_MODE1_DRAWS 131U
#define EXPECTED_MODE5_DRAWS 7U
#define VDP1_BUFFER_SIZE 0x80000U

typedef struct { uint8_t *bytes; size_t size; } Blob;

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
    out.bytes = (uint8_t *)malloc((size_t)length + 1U);
    if (!out.bytes || fread(out.bytes, 1U, (size_t)length, file) != (size_t)length) {
        free(out.bytes); out.bytes = NULL; fclose(file); return out;
    }
    fclose(file);
    out.bytes[length] = 0U;
    out.size = (size_t)length;
    return out;
}

static void free_blob(Blob *blob)
{ free(blob->bytes); blob->bytes = NULL; blob->size = 0U; }

static int hash_is(const Blob *blob, const char *expected)
{
    char actual[65];
    return blob->bytes && firestaff_x68k_media_receipt_sha256_hex(
        blob->bytes, blob->size, actual, sizeof(actual)) == 0 &&
        strcmp(actual, expected) == 0;
}

static uint16_t read_le16(const uint8_t *p)
{ return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8U)); }

static uint16_t read_be16(const uint8_t *p)
{ return (uint16_t)(((uint16_t)p[0] << 8U) | p[1]); }

static void set_le16(uint8_t *p, uint16_t value)
{ p[0] = (uint8_t)value; p[1] = (uint8_t)(value >> 8U); }

static uint16_t rgba_to_555(uint32_t rgba)
{
    return (uint16_t)((((rgba >> 16U) & 0xffU) >> 3U) |
        (((((rgba >> 8U) & 0xffU) >> 3U) & 31U) << 5U) |
        (((rgba & 0xffU) >> 3U) << 10U));
}

static int parse_long(const char *text, long *out)
{
    char *end = NULL;
    long value;
    while (isspace((unsigned char)*text)) ++text;
    value = strtol(text, &end, 0);
    if (end == text || (*end && *end != '\r' && *end != '\n')) return 0;
    *out = value;
    return 1;
}

static int parse_ulong(const char *text, unsigned long *out)
{
    char *end = NULL;
    unsigned long value;
    while (isspace((unsigned char)*text)) ++text;
    if (*text == '-') return 0;
    value = strtoul(text, &end, 0);
    if (end == text || (*end && *end != '\r' && *end != '\n')) return 0;
    *out = value;
    return 1;
}

static int split_csv_row(char *line, char *fields[22])
{
    char *cursor = line;
    unsigned int count;
    for (count = 0U; count < 22U; ++count) {
        fields[count] = cursor;
        cursor = strchr(cursor, ',');
        if (!cursor) return count == 21U;
        *cursor++ = '\0';
    }
    return strchr(cursor, ',') == NULL;
}

static int read_capture_root(const char *root, char *out, size_t capacity)
{
    static const char *const suffixes[] = {"", "/dumps", "/f364-correlation/dumps"};
    size_t i;
    for (i = 0U; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        char path[4096];
        int n = snprintf(path, sizeof(path), "%s%s/vdp1-fbs.bin", root, suffixes[i]);
        FILE *probe;
        if (n < 0 || (size_t)n >= sizeof(path)) continue;
        probe = fopen(path, "rb");
        if (!probe) continue;
        fclose(probe);
        n = snprintf(out, capacity, "%s%s", root, suffixes[i]);
        return n >= 0 && (size_t)n < capacity;
    }
    return 0;
}

static int bundle_is_complete(const Blob *bundle)
{
    static const char *const required[] = {
        "format=ymir-vdp1-capture-bundle-v1",
        "capture_frame_index=1605",
        "capture_next_command_index=151",
        "trace_enabled=1",
        "trace_file_complete=1",
        "trace_rows=228449",
        "trace_rows_for_capture_frame=151",
        "trace_dropped_total=0",
        "trace_first_frame=0",
        "trace_last_frame=1605"
    };
    size_t i;
    for (i = 0U; i < sizeof(required) / sizeof(required[0]); ++i)
        if (!strstr((const char *)bundle->bytes, required[i])) return 0;
    return 1;
}

static int load_required_file(const char *root, const char *name,
                              const char *sha, size_t expected_size,
                              Blob *out)
{
    char path[4096];
    int n = snprintf(path, sizeof(path), "%s/%s", root, name);
    if (n < 0 || (size_t)n >= sizeof(path)) return 0;
    *out = read_blob(path);
    if (!out->bytes) return 0;
    if ((expected_size && out->size != expected_size) || !hash_is(out, sha)) {
        fprintf(stderr, "capture hash or size mismatch: %s\n", name);
        return -1;
    }
    return 1;
}

int main(void)
{
    const char *capture_env = getenv("FIRESTAFF_NEXUS_YMIR_FRAME1605_CAPTURE_DIR");
    const char *data_env = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    static const struct { const char *name, *sha; size_t size; } files[] = {
        {"vdp1-fbs.bin", "d38b9b7128ef22e78346696f83fd1a7ae4c92ecfcba73aed95786d5125af9e90", VDP1_BUFFER_SIZE},
        {"vdp1-vram.bin", "331a6675207f86de643ea3b7a1a8c82fed0937647734182775b33c44d93f90e7", VDP1_BUFFER_SIZE},
        {"vdp1-registers.bin", "408b16eaa494d3e282368aa85caacf6c81cf53930043945a272816676f4e3642", 32U},
        {"vdp2-registers.bin", "e853e43bfc7d73d3425c56bf9c5258217c6db63d12859d4cbf587662be0799cd", 512U},
        {"vdp1-command-trace.csv", "6108792b8ab554a6e1e0ee6a62ccfbc665e917be22c68cb4c1f5b4b0263b99e9", 0U},
        {"vdp1-capture-bundle.txt", "95c7f2a81aa7602a5e59b291b501a9b137cb697133a94f7c0da401125e947fa8", 0U}
    };
    Blob blobs[sizeof(files) / sizeof(files[0])] = {{0}};
    Blob dgn = {NULL, 0U};
    char dump_root[4096], path[4096];
    uint8_t *le_vram = NULL, *trace_copy = NULL;
    Nexus_V1_Vdp1YmirCaptureSurface *surface = NULL;
    Nexus_V1_Vdp1DgnMaterialResolverInput dgn_input = {0};
    Nexus_V1_Vdp1YmirCaptureInput input;
    Nexus_V1_Vdp1YmirCaptureReceipt receipt;
    uint8_t seen[EXPECTED_ROWS] = {0};
    int user_x0 = 0, user_y0 = 0, user_x1 = 512, user_y1 = 256;
    int sys_clip_h = 511, sys_clip_v = 255;
    unsigned long frame_rows = 0UL;
    unsigned int texture_draws = 0U, mode1_draws = 0U, mode5_draws = 0U;
    unsigned int polygon_draws = 0U, skipped_draws = 0U, end_draws = 0U;
    unsigned int local_updates = 0U, user_clip_updates = 0U, system_clip_updates = 0U;
    unsigned int dgn_image_joins = 0U, dgn_palette_joins = 0U;
    uint64_t opaque = 0U, rgb_exact = 0U, alpha_exact = 0U;
    char *line;
    size_t i, file_count = sizeof(files) / sizeof(files[0]);
    int result = 1;

    if (!capture_env || !*capture_env || !data_env || !*data_env) {
        fprintf(stderr, "SKIP: set FIRESTAFF_NEXUS_YMIR_FRAME1605_CAPTURE_DIR and FIRESTAFF_NEXUS_DATA_DIR to authentic capture and retail data\n");
        return 77;
    }
    if (!read_capture_root(capture_env, dump_root, sizeof(dump_root))) {
        fprintf(stderr, "SKIP: authentic frame-1605 Ymir dump directory is unavailable\n");
        return 77;
    }
    for (i = 0U; i < file_count; ++i) {
        int loaded = load_required_file(dump_root, files[i].name, files[i].sha,
                                       files[i].size, &blobs[i]);
        if (loaded == 0) {
            fprintf(stderr, "SKIP: missing authentic frame-1605 file %s\n", files[i].name);
            result = 77; goto done;
        }
        if (loaded < 0) goto done;
    }
    if (!bundle_is_complete(&blobs[5])) {
        fprintf(stderr, "Ymir receipt does not attest a complete, zero-drop frame-1605 trace\n");
        goto done;
    }
    {
        int n = snprintf(path, sizeof(path), "%s/LEV01.DGN", data_env);
        if (n < 0 || (size_t)n >= sizeof(path)) goto done;
        dgn = read_blob(path);
        if (!dgn.bytes) {
            fprintf(stderr, "SKIP: authentic Japanese LEV01.DGN is unavailable\n");
            result = 77; goto done;
        }
        if (dgn.size > (size_t)INT_MAX ||
            !hash_is(&dgn, "b359a89ffa344439b6d0d15de223f3211caa7725b13dfdb4461b579c48f06723")) {
            fprintf(stderr, "LEV01.DGN hash does not match the canonical Japanese retail file\n");
            goto done;
        }
        dgn_input.dgn_bytes = dgn.bytes;
        dgn_input.dgn_byte_count = (int)dgn.size;
        dgn_input.source_hash_verified = 1;
        dgn_input.palette_slot_base = 0;
    }
    if (read_be16(blobs[3].bytes) != 0x8000U ||
        ((read_be16(blobs[3].bytes) >> 6U) & 3U) != 0U ||
        (read_be16(blobs[2].bytes + 2U) & 0x0008U) != 0U) {
        fprintf(stderr, "register snapshots do not identify single-density 320x224 mode\n");
        goto done;
    }

    le_vram = (uint8_t *)malloc(blobs[1].size);
    trace_copy = (uint8_t *)malloc(blobs[4].size + 1U);
    surface = (Nexus_V1_Vdp1YmirCaptureSurface *)calloc(1U, sizeof(*surface));
    if (!le_vram || !trace_copy || !surface) { fprintf(stderr, "out of memory\n"); goto done; }
    for (i = 0U; i < blobs[1].size; i += 2U) {
        le_vram[i] = blobs[1].bytes[i + 1U];
        le_vram[i + 1U] = blobs[1].bytes[i];
    }
    memcpy(trace_copy, blobs[4].bytes, blobs[4].size);
    trace_copy[blobs[4].size] = '\0';
    line = (char *)trace_copy;
    {
        char *newline = strchr(line, '\n');
        static const char header[] = "frame,index,address,word0,word1,word2,word3,word4,word5,word6,word7,word8,word9,word10,word11,word12,word13,word14,word15,frame_root,local_x,local_y";
        if (!newline) { fprintf(stderr, "VDP1 command trace header is missing\n"); goto done; }
        *newline = '\0';
        if (strcmp(line, header) != 0) { fprintf(stderr, "unexpected VDP1 command trace schema\n"); goto done; }
        line = newline + 1;
    }
    while (line && *line) {
        char *current = line;
        char *next = strchr(current, '\n');
        char *fields[22];
        unsigned long row_frame, index, address, words[16];
        long local_x, local_y;
        uint8_t command[NEXUS_V1_VDP1_COMMAND_BYTES];
        Nexus_V1_Vdp1TextureCommand parsed;
        if (next) *next++ = '\0';
        line = next;
        if (!split_csv_row(current, fields)) { fprintf(stderr, "malformed VDP1 trace row\n"); goto done; }
        if (!parse_ulong(fields[0], &row_frame) || row_frame != FRAME_INDEX) continue;
        if (!parse_ulong(fields[1], &index) || index >= EXPECTED_ROWS || seen[index] ||
            !parse_ulong(fields[2], &address) || (address & 31UL) != 0UL ||
            address > blobs[1].size - NEXUS_V1_VDP1_COMMAND_BYTES ||
            !parse_long(fields[20], &local_x) || !parse_long(fields[21], &local_y)) {
            fprintf(stderr, "invalid or duplicate frame-1605 command row\n"); goto done;
        }
        for (i = 0U; i < 16U; ++i) {
            if (!parse_ulong(fields[i + 3U], &words[i]) || words[i] > 0xffffUL) {
                fprintf(stderr, "invalid VDP1 command word\n"); goto done;
            }
            set_le16(command + i * 2U, (uint16_t)words[i]);
        }
        if ((index != 0UL && memcmp(command, le_vram + address, sizeof(command)) != 0) ||
            nexus_v1_vdp1_texture_command_parse(command, sizeof(command), &parsed) != 0) {
            fprintf(stderr, "trace command does not match captured VRAM at row %lu\n", index);
            goto done;
        }
        seen[index] = 1U;
        ++frame_rows;
        if (parsed.end_command) { ++end_draws; continue; }
        if (parsed.command_type == 10U) { ++local_updates; continue; }
        if (parsed.command_type == 8U) {
            user_x0 = (int)read_le16(command + 12U);
            user_y0 = (int)read_le16(command + 14U);
            user_x1 = (int)read_le16(command + 20U);
            user_y1 = (int)read_le16(command + 22U);
            ++user_clip_updates;
            continue;
        }
        if (parsed.command_type == 9U) {
            sys_clip_h = (int)read_le16(command + 20U);
            sys_clip_v = (int)read_le16(command + 22U);
            ++system_clip_updates;
            continue;
        }
        if ((parsed.control & 0x4000U) != 0U) { ++skipped_draws; continue; }
        if (parsed.command_type == 4U) {
            memset(&input, 0, sizeof(input));
            input.command = command; input.command_size = (int)sizeof(command);
            input.command_capture_verified = 1;
            input.command_index = (uint16_t)index;
            input.vdp1_vram = le_vram; input.vdp1_vram_size = (int)blobs[1].size;
            input.vdp1_vram_hash_verified = 1;
            input.local_origin_verified = 1; input.local_origin_x = (int)local_x; input.local_origin_y = (int)local_y;
            input.system_clip_verified = 1; input.sys_clip_h = sys_clip_h; input.sys_clip_v = sys_clip_v;
            input.user_clip_verified = 1; input.user_clip_x0 = user_x0; input.user_clip_y0 = user_y0;
            input.user_clip_x1 = user_x1; input.user_clip_y1 = user_y1;
            input.single_density_verified = 1;
            if (!nexus_v1_vdp1_ymir_capture_rasterize_polygon(surface, &input, &receipt) ||
                !receipt.valid || receipt.renderer_permitted || !receipt.capture_only) {
                fprintf(stderr, "stepper rejected captured VDP1 polygon at row %lu\n", index);
                goto done;
            }
            ++polygon_draws;
            continue;
        }
        if (!parsed.texture_command) continue;
        if (parsed.command_type != 2U || (parsed.colour_mode != 1U && parsed.colour_mode != 5U) ||
            !parsed.texture_source_range_valid || parsed.texture_byte_count == 0U) {
            fprintf(stderr, "unsupported textured command at frame row %lu\n", index);
            goto done;
        }
        {
            Nexus_V1_Vdp1CaptureCompositeInput material;
            uint8_t palette_le[32];
            const uint8_t *texture, *palette = NULL;
            int joined;
            unsigned int byte;
            memset(&material, 0, sizeof(material));
            joined = nexus_v1_vdp1_dgn_material_resolver(
                le_vram, (int)blobs[1].size, command, (int)sizeof(command),
                &parsed, (uint32_t)address, &material, &dgn_input);
            if (!joined) {
                fprintf(stderr, "retail DGN image/CLUT join failed at frame row %lu\n", index);
                goto done;
            }
            if (parsed.colour_mode == 1U) {
                if (!material.dgn_image || material.dgn_image_size != (int)parsed.texture_byte_count ||
                    !material.dgn_palette || material.dgn_palette_size != 32) {
                    fprintf(stderr, "incomplete retail indexed material join at row %lu\n", index);
                    goto done;
                }
                texture = material.dgn_image;
                for (byte = 0U; byte < 32U; byte += 2U) {
                    palette_le[byte] = material.dgn_palette[byte + 1U];
                    palette_le[byte + 1U] = material.dgn_palette[byte];
                }
                palette = palette_le;
                ++mode1_draws; ++dgn_palette_joins;
            } else {
                if (!material.dgn_direct_image ||
                    material.dgn_direct_image_size != (int)parsed.texture_byte_count) {
                    fprintf(stderr, "incomplete retail direct-colour join at row %lu\n", index);
                    goto done;
                }
                texture = material.dgn_direct_image;
                ++mode5_draws;
            }
            memset(&input, 0, sizeof(input));
            input.command = command; input.command_size = (int)sizeof(command);
            input.texture_bytes = texture; input.texture_size = (int)parsed.texture_byte_count;
            input.palette_bytes = palette; input.palette_size = palette ? 32 : 0;
            input.vdp1_vram = le_vram; input.vdp1_vram_size = (int)blobs[1].size;
            input.command_capture_verified = 1; input.texture_capture_verified = 1;
            input.command_index = (uint16_t)index;
            input.source_image_join_verified = parsed.colour_mode == 1U;
            input.direct_source_join_verified = parsed.colour_mode == 5U;
            input.palette_join_verified = parsed.colour_mode == 1U;
            input.vdp1_vram_hash_verified = 1;
            input.local_origin_verified = 1; input.local_origin_x = (int)local_x; input.local_origin_y = (int)local_y;
            input.system_clip_verified = 1; input.sys_clip_h = sys_clip_h; input.sys_clip_v = sys_clip_v;
            input.user_clip_verified = 1; input.user_clip_x0 = user_x0; input.user_clip_y0 = user_y0;
            input.user_clip_x1 = user_x1; input.user_clip_y1 = user_y1;
            input.even_odd_coord_select_verified = 1;
            input.even_odd_coord_select = (read_be16(blobs[2].bytes + 2U) >> 4U) & 1U;
            input.single_density_verified = 1;
            if (!(parsed.colour_mode == 1U
                    ? nexus_v1_vdp1_ymir_capture_rasterize_mode1_distorted(surface, &input, &receipt)
                    : nexus_v1_vdp1_ymir_capture_rasterize_mode5_distorted(surface, &input, &receipt)) ||
                !receipt.valid || receipt.renderer_permitted || !receipt.capture_only) {
                fprintf(stderr, "stepper rejected captured draw at row %lu\n", index);
                goto done;
            }
            ++texture_draws; ++dgn_image_joins;
        }
    }
    if (frame_rows != EXPECTED_ROWS || texture_draws != EXPECTED_TEXTURE_DRAWS ||
        mode1_draws != EXPECTED_MODE1_DRAWS || mode5_draws != EXPECTED_MODE5_DRAWS ||
        dgn_image_joins != EXPECTED_TEXTURE_DRAWS || dgn_palette_joins != EXPECTED_MODE1_DRAWS ||
        polygon_draws != 1U || skipped_draws != 5U || end_draws != 1U ||
        local_updates != 4U || user_clip_updates != 2U || system_clip_updates != 0U) {
        fprintf(stderr, "incomplete frame replay rows=%lu textures=%u mode1=%u mode5=%u polygon=%u skipped=%u end=%u local=%u userclip=%u sysclip=%u\n",
            frame_rows, texture_draws, mode1_draws, mode5_draws, polygon_draws,
            skipped_draws, end_draws, local_updates, user_clip_updates, system_clip_updates);
        goto done;
    }
    for (i = 0U; i < EXPECTED_ROWS; ++i)
        if (!seen[i]) { fprintf(stderr, "missing frame-1605 command index %lu\n", (unsigned long)i); goto done; }

    for (int y = 0; y < NEXUS_FB_H; ++y) {
        for (int x = 0; x < NEXUS_FB_W; ++x) {
            size_t pixel = (size_t)y * NEXUS_FB_W + (size_t)x;
            uint32_t actual = surface->rgba[pixel];
            uint16_t expected = read_be16(blobs[0].bytes + ((size_t)y * 512U + (size_t)x) * 2U);
            if ((actual >> 24U) != 0U) ++opaque;
            if ((((actual >> 24U) != 0U) == ((expected & 0x8000U) != 0U))) ++alpha_exact;
            if (rgba_to_555(actual) == (expected & 0x7fffU)) ++rgb_exact;
        }
    }
    if (opaque != UINT64_C(71680) || rgb_exact != UINT64_C(71680) ||
        alpha_exact != UINT64_C(71680)) {
        fprintf(stderr, "frame-1605 VDP1 mismatch opaque=%llu rgb=%llu alpha=%llu expected=71680\n",
            (unsigned long long)opaque, (unsigned long long)rgb_exact,
            (unsigned long long)alpha_exact);
        goto done;
    }
    printf("Ymir VDP1 frame-1605 capture replay: rows=%lu retail_image_joins=%u indexed=%u retail_palette_joins=%u direct_rgb=%u flat_polygons=%u opaque=%llu rgb_exact=%llu alpha_exact=%llu renderer_permitted=false\n",
        frame_rows, dgn_image_joins, mode1_draws, dgn_palette_joins,
        mode5_draws, polygon_draws, (unsigned long long)opaque,
        (unsigned long long)rgb_exact, (unsigned long long)alpha_exact);
    printf("PASS: all 71,680 viewport RGB555 pixels and opacity bits match authentic Ymir VDP1 FBS; production renderer admission remains blocked\n");
    result = 0;
done:
    free(le_vram); free(trace_copy); free(surface); free_blob(&dgn);
    for (i = 0U; i < file_count; ++i) free_blob(&blobs[i]);
    return result;
}
