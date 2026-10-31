#include "firestaff_x68k_media_receipt.h"
#include "nexus_v1_vdp1_ymir_stepper_capture.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRAME_INDEX 13162U
#define FIRST_DRAW 8U
#define LAST_DRAW 167U
#define EXPECTED_FRAME_ROWS 168U
#define VDP1_VRAM_SIZE 0x80000U
#define VDP1_FBS_SIZE 0x80000U
#define VDP1_DRAW_BUFFER_SIZE 0x40000U

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
{
    free(blob->bytes); blob->bytes = NULL; blob->size = 0U;
}

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

static void set_word(uint8_t *p, uint16_t value)
{ p[0] = (uint8_t)value; p[1] = (uint8_t)(value >> 8U); }

static int is_frame_receipt(const Blob *bundle)
{
    static const char *const required[] = {
        "format=ymir-vdp1-capture-bundle-v1",
        "capture_frame_index=13162",
        "capture_next_command_index=168",
        "capture_local_x=160",
        "capture_local_y=112",
        "trace_enabled=1",
        "trace_file_complete=1",
        "trace_rows=430292",
        "trace_rows_for_capture_frame=168",
        "trace_dropped_total=0"
    };
    size_t i;
    for (i = 0U; i < sizeof(required) / sizeof(required[0]); ++i)
        if (!strstr((const char *)bundle->bytes, required[i])) return 0;
    return 1;
}

static int choose_dump_root(const char *root, char *out, size_t cap)
{
    char path[4096];
    FILE *file;
    const char *suffixes[] = {"", "/dumps", "/profile-frame13162/dumps"};
    size_t i;
    for (i = 0U; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        int written = snprintf(path, sizeof(path), "%s%s", root, suffixes[i]);
        if (written < 0 || (size_t)written >= sizeof(path))
            continue;
        {
            char probe[4096];
            int probe_written = snprintf(probe, sizeof(probe), "%s/vdp1-fbs.bin", path);
            if (probe_written < 0 || (size_t)probe_written >= sizeof(probe))
                continue;
            file = fopen(probe, "rb");
            if (!file) continue;
            fclose(file);
            written = snprintf(out, cap, "%s", path);
            if (written < 0 || (size_t)written >= cap)
                return 0;
            return 1;
        }
    }
    return 0;
}

static int read_trace_fields(char *line, char *fields[22])
{
    int count = 0;
    char *p = line;
    while (count < 22) {
        fields[count++] = p;
        p = strchr(p, ',');
        if (!p) return count == 22;
        *p++ = '\0';
    }
    /* Reject extra columns rather than silently ignoring unpinned fields. */
    return strchr(p, ',') == NULL;
}

static int parse_long_field(const char *field, long *out)
{
    char *end = NULL;
    long value;
    while (isspace((unsigned char)*field)) ++field;
    value = strtol(field, &end, 0);
    if (end == field || (*end && *end != '\r' && *end != '\n')) return 0;
    *out = value;
    return 1;
}

static int parse_ulong_field(const char *field, unsigned long *out)
{
    char *end = NULL;
    unsigned long value;
    while (isspace((unsigned char)*field)) ++field;
    if (*field == '-') return 0;
    value = strtoul(field, &end, 0);
    if (end == field || (*end && *end != '\r' && *end != '\n')) return 0;
    *out = value;
    return 1;
}

static uint16_t rgba_to_555(uint32_t rgba)
{
    return (uint16_t)((((rgba >> 16U) & 0xffU) >> 3U) |
        (((((rgba >> 8U) & 0xffU) >> 3U) & 31U) << 5U) |
        (((rgba & 0xffU) >> 3U) << 10U));
}

static int write_rgba(const char *path,
                      const Nexus_V1_Vdp1YmirCaptureSurface *surface)
{
    FILE *file;
    size_t i;
    if (!path || !*path || !surface) return 0;
    file = fopen(path, "wb");
    if (!file) return 0;
    for (i = 0U; i < (size_t)NEXUS_FB_W * NEXUS_FB_H; ++i) {
        uint32_t pixel = surface->rgba[i];
        uint8_t rgba[4] = {(uint8_t)(pixel >> 16U),
            (uint8_t)(pixel >> 8U), (uint8_t)pixel,
            (uint8_t)(pixel >> 24U)};
        if (fwrite(rgba, 1U, sizeof(rgba), file) != sizeof(rgba)) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0;
}

int main(int argc, char **argv)
{
    static const struct { const char *name; const char *sha; size_t size; } files[] = {
        {"vdp1-fbs.bin", "683c8db18a11f3a74c616146baa6aee3f42f07ece0281409bbee2752c7862d9d", VDP1_FBS_SIZE},
        {"vdp1-vram.bin", "9974b1716c4ee91aa1d44d3ed2705cc09ac27b17c642e68bdaf96293fd36c4e7", VDP1_VRAM_SIZE},
        {"vdp1-registers.bin", "00477de68c306db0fabcbfef46b9d24c8815c3069ab2223ef2908c975be37368", 32U},
        {"vdp2-registers.bin", "e69d03094ef856922413e073cb9078015e52849786268d6421c7df00a655965e", 512U},
        {"vdp1-command-trace.csv", "42e087933edea690aac824012492bb7144f718e94a7ee6f7b27551791aacae72", 0U},
        {"vdp1-capture-bundle.txt", "32ce0c4db50ab600aec8b345f5372453c2e4242e35a0d0518fbf33cf45781cbc", 0U}
    };
    const char *capture_env = getenv("FIRESTAFF_NEXUS_YMIR_FRAME13162_CAPTURE_DIR");
    char dump_root[4096], paths[sizeof(files) / sizeof(files[0])][4096];
    Blob blobs[sizeof(files) / sizeof(files[0])] = {{0}};
    uint8_t *le_vram = NULL;
    Nexus_V1_Vdp1YmirCaptureSurface *surface = NULL;
    Nexus_V1_Vdp1YmirCaptureReceipt receipt;
    Nexus_V1_Vdp1YmirCaptureInput input;
    uint8_t seen[EXPECTED_FRAME_ROWS] = {0};
    int16_t local_x = 0, local_y = 0;
    int user_x0 = 0, user_y0 = 0, user_x1 = 512, user_y1 = 256;
    int sys_clip_h = 511, sys_clip_v = 255;
    unsigned int frame_rows = 0U, draws = 0U, mode1_count = 0U;
    unsigned int other_modes = 0U, polygon_count = 0U;
    unsigned int user_clip_updates = 0U, system_clip_updates = 0U, local_updates = 0U;
    uint64_t opaque = 0U, exact_matches = 0U, alpha_matches = 0U;
    char *trace_copy = NULL, *line;
    size_t i, file_count = sizeof(files) / sizeof(files[0]);
    int result = 1;
    (void)argv;

    if (argc != 1 || !capture_env || !*capture_env) {
        fprintf(stderr, "SKIP: set FIRESTAFF_NEXUS_YMIR_FRAME13162_CAPTURE_DIR to the external authentic Ymir capture\n");
        return 77;
    }
    if (!choose_dump_root(capture_env, dump_root, sizeof(dump_root))) {
        fprintf(stderr, "SKIP: frame-13162 Ymir capture dumps are unavailable\n");
        return 77;
    }
    for (i = 0U; i < file_count; ++i) {
        if (snprintf(paths[i], sizeof(paths[i]), "%s/%s", dump_root,
                     files[i].name) < 0 || strlen(dump_root) + 1U + strlen(files[i].name) >= sizeof(paths[i])) {
            fprintf(stderr, "capture path is too long\n"); goto done;
        }
        blobs[i] = read_blob(paths[i]);
        if (!blobs[i].bytes) {
            fprintf(stderr, "SKIP: missing authentic frame-13162 capture file %s\n", files[i].name);
            result = 77; goto done;
        }
        if ((files[i].size && blobs[i].size != files[i].size) ||
            !hash_is(&blobs[i], files[i].sha)) {
            fprintf(stderr, "capture hash or size mismatch: %s\n", files[i].name);
            goto done;
        }
    }
    if (!is_frame_receipt(&blobs[5])) {
        fprintf(stderr, "capture bundle receipt does not identify complete frame 13162\n");
        goto done;
    }
    /* VDP1 FBCR bit 3 is DIE; bit 4 is EOS. This capture disables
     * double-interlace and selects even X coordinates for HSS. VDP2 TVMD
     * confirms the single-density 320x224 screen mode. */
    if (blobs[2].size != 32U || blobs[3].size != 512U ||
        read_be16(blobs[3].bytes) != 0x8000U ||
        ((read_be16(blobs[3].bytes) >> 6U) & 3U) != 0U ||
        (read_be16(blobs[2].bytes + 2U) & 0x0008U) != 0U) {
        fprintf(stderr, "VDP register snapshot is not the expected single-density 320x224 capture\n");
        goto done;
    }

    trace_copy = (char *)malloc(blobs[4].size + 1U);
    le_vram = (uint8_t *)malloc(blobs[1].size);
    surface = (Nexus_V1_Vdp1YmirCaptureSurface *)calloc(1U, sizeof(*surface));
    if (!trace_copy || !le_vram || !surface) { fprintf(stderr, "out of memory\n"); goto done; }
    /* Ymir dumps VRAM as big-endian words. Normalize word-oriented command,
     * palette, and Gouraud reads to LE; texture nibbles retain raw byte order. */
    for (i = 0U; i < blobs[1].size; i += 2U) {
        le_vram[i] = blobs[1].bytes[i + 1U];
        le_vram[i + 1U] = blobs[1].bytes[i];
    }
    memcpy(trace_copy, blobs[4].bytes, blobs[4].size); trace_copy[blobs[4].size] = '\0';
    line = trace_copy;
    {
        char *nl = strchr(line, '\n');
        static const char header[] = "frame,index,address,word0,word1,word2,word3,word4,word5,word6,word7,word8,word9,word10,word11,word12,word13,word14,word15,frame_root,local_x,local_y";
        if (!nl) { fprintf(stderr, "trace header missing\n"); goto done; }
        *nl = '\0';
        if (strcmp(line, header) != 0) { fprintf(stderr, "unexpected VDP1 trace schema\n"); goto done; }
        line = nl + 1;
    }
    while (line && *line) {
        char *current_line = line;
        char *next = strchr(current_line, '\n');
        char *fields[22];
        unsigned long frame, index, address, values[16];
        long row_local_x, row_local_y;
        uint8_t command[NEXUS_V1_VDP1_COMMAND_BYTES];
        Nexus_V1_Vdp1TextureCommand parsed;
        if (next) *next++ = '\0';
        line = next;
        if (read_trace_fields(current_line, fields)) {
            if (!parse_ulong_field(fields[0], &frame) ||
                !parse_ulong_field(fields[1], &index)) { fprintf(stderr,"invalid trace frame/index\n"); goto done; }
            if (frame == FRAME_INDEX) {
                if (index >= EXPECTED_FRAME_ROWS || seen[index]) { fprintf(stderr,"duplicate or out-of-range frame row\n"); goto done; }
                if (!parse_ulong_field(fields[2], &address) ||
                    !parse_long_field(fields[20], &row_local_x) ||
                    !parse_long_field(fields[21], &row_local_y)) { fprintf(stderr,"invalid trace state fields\n"); goto done; }
                if ((address & 31U) != 0U || address > blobs[1].size - NEXUS_V1_VDP1_COMMAND_BYTES) { fprintf(stderr,"invalid command address\n"); goto done; }
                for (i = 0U; i < 16U; ++i) {
                    if (!parse_ulong_field(fields[3U+i], &values[i]) || values[i] > 0xffffU) { fprintf(stderr,"invalid command word\n"); goto done; }
                    set_word(command + i*2U, (uint16_t)values[i]);
                }
                /* The attested post-frame VRAM differs from trace row 0 only
                 * in its link word; row 0 is outside the rendered draw window.
                 * Every later trace row must match captured VRAM byte-for-byte. */
                if ((index != 0U && memcmp(command, le_vram + address,
                                           sizeof(command)) != 0) ||
                    nexus_v1_vdp1_texture_command_parse(command, sizeof(command), &parsed) != 0) {
                    fprintf(stderr,"invalid authenticated trace command at row %lu\n",index); goto done;
                }
                if ((int32_t)row_local_x != local_x || (int32_t)row_local_y != local_y) {
                    fprintf(stderr,"trace local-coordinate receipt mismatch at row %lu\n",index); goto done;
                }
                seen[index] = 1U; ++frame_rows;
                if (index == 0U) continue;
                if (parsed.command_type == 10U) {
                    local_x = parsed.xa; local_y = parsed.ya; ++local_updates;
                    continue;
                }
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
                if (index < FIRST_DRAW || index > LAST_DRAW) continue;
                if (parsed.command_type <= 7U) ++draws;
                if (parsed.command_type == 4U) {
                    memset(&input, 0, sizeof(input));
                    input.command = command;
                    input.command_size = (int)sizeof(command);
                    input.command_capture_verified = 1;
                    input.vdp1_vram = le_vram;
                    input.vdp1_vram_size = (int)blobs[1].size;
                    input.vdp1_vram_hash_verified = 1;
                    input.local_origin_verified = 1;
                    input.local_origin_x = (int)row_local_x;
                    input.local_origin_y = (int)row_local_y;
                    input.system_clip_verified = 1;
                    input.sys_clip_h = sys_clip_h;
                    input.sys_clip_v = sys_clip_v;
                    input.user_clip_verified = 1;
                    input.user_clip_x0 = user_x0;
                    input.user_clip_y0 = user_y0;
                    input.user_clip_x1 = user_x1;
                    input.user_clip_y1 = user_y1;
                    input.single_density_verified = 1;
                    if (!nexus_v1_vdp1_ymir_capture_rasterize_polygon(
                            surface, &input, &receipt) || !receipt.valid ||
                        receipt.renderer_permitted || !receipt.capture_only) {
                        fprintf(stderr, "stepper rejected authentic polygon row %lu\n", index);
                        goto done;
                    }
                    ++polygon_count;
                    continue;
                }
                if (parsed.command_type != 2U) continue;
                if (!parsed.texture_command || (parsed.control & 0x4000U) != 0U ||
                    (parsed.colour_mode != 1U && parsed.colour_mode != 5U)) {
                    fprintf(stderr,"unexpected textured command mode in frame row %lu\n",index); goto done;
                }
                if (parsed.colour_mode == 5U) { ++other_modes; continue; }
                if (parsed.colour_mode != 1U) { fprintf(stderr,"unsupported texture mode\n"); goto done; }
                if (!parsed.texture_source_range_valid || !parsed.texture_byte_count ||
                    parsed.texture_source_byte_end > blobs[1].size) { fprintf(stderr,"invalid captured texture span\n"); goto done; }
                {
                    uint32_t palette_offset = (((uint32_t)parsed.colour_control & ~UINT32_C(3)) << 3U);
                    uint32_t texture_offset = parsed.texture_source_byte_offset;
                    if (palette_offset > blobs[1].size - 32U ||
                        texture_offset > blobs[1].size || parsed.texture_byte_count > blobs[1].size - texture_offset) {
                        fprintf(stderr,"captured texture or palette span out of bounds\n"); goto done;
                    }
                    memset(&input, 0, sizeof(input));
                    input.command = command; input.command_size = (int)sizeof(command);
                    input.texture_bytes = blobs[1].bytes + texture_offset;
                    input.texture_size = (int)parsed.texture_byte_count;
                    input.palette_bytes = le_vram + palette_offset; input.palette_size = 32;
                    input.vdp1_vram = le_vram; input.vdp1_vram_size = (int)blobs[1].size;
                    input.command_capture_verified = 1;
                    /* These attest the exact command-to-captured-VRAM spans,
                     * not retail DGN ownership. No unjoined row is called a DGN material. */
                    input.source_image_join_verified = 1;
                    input.palette_join_verified = 1;
                    input.vdp1_vram_hash_verified = 1;
                    input.local_origin_verified = 1;
                    input.local_origin_x = (int)row_local_x; input.local_origin_y = (int)row_local_y;
                    input.system_clip_verified = 1;
                    input.sys_clip_h = sys_clip_h; input.sys_clip_v = sys_clip_v;
                    input.user_clip_verified = 1;
                    input.user_clip_x0=user_x0;input.user_clip_y0=user_y0;
                    input.user_clip_x1=user_x1;input.user_clip_y1=user_y1;
                    input.even_odd_coord_select_verified = 1;
                    input.even_odd_coord_select =
                        (read_be16(blobs[2].bytes + 2U) >> 4U) & 1U;
                    input.single_density_verified = 1;
                    if (!nexus_v1_vdp1_ymir_capture_rasterize_mode1_distorted(
                            surface, &input, &receipt) || !receipt.valid ||
                        receipt.renderer_permitted || !receipt.capture_only) {
                        fprintf(stderr,"stepper rejected authentic frame row %lu\n",index); goto done;
                    }
                    mode1_count++;
                }
            }
        }
    }
    if (frame_rows != EXPECTED_FRAME_ROWS || draws != LAST_DRAW-FIRST_DRAW+1U ||
        mode1_count != 124U || other_modes != 8U || user_clip_updates != 1U || local_updates != 3U ||
        polygon_count != 28U || system_clip_updates != 0U || !seen[0] || !seen[LAST_DRAW]) {
        fprintf(stderr,"incomplete authenticated frame receipt rows=%u draws=%u mode1=%u polygon=%u userclip=%u local=%u sysclip=%u\n",
            frame_rows,draws,mode1_count,polygon_count,user_clip_updates,local_updates,system_clip_updates); goto done;
    }
    if (!is_frame_receipt(&blobs[5])) { fprintf(stderr,"frame receipt changed during validation\n"); goto done; }

    /* FBS buffer 0 is 512x256 BGR555, big-endian. Compare the 320x224
     * captured viewport only. The test intentionally records partial
     * command-set agreement; a nonzero residual is not renderer parity. */
    for (int y = 0; y < NEXUS_FB_H; ++y) for (int x = 0; x < NEXUS_FB_W; ++x) {
        size_t p=(size_t)y*NEXUS_FB_W+(size_t)x;
        uint32_t rgba=surface->rgba[p];
        uint16_t expected=(uint16_t)read_be16(blobs[0].bytes+((size_t)y*512U+(size_t)x)*2U);
        if ((rgba >> 24U) != 0U) ++opaque;
        if (((rgba >> 24U) != 0U) == ((expected & 0x8000U) != 0U))
            ++alpha_matches;
        {
            if (rgba_to_555(rgba)==(expected & 0x7fffU)) ++exact_matches;
        }
    }
    if (opaque != UINT64_C(71680) || exact_matches != UINT64_C(71680) ||
        alpha_matches != UINT64_C(71680)) {
        fprintf(stderr,"captured VDP1 output differs: opaque=%llu rgb_exact=%llu alpha_exact=%llu; expected 71680 each\n",
            (unsigned long long)opaque,(unsigned long long)exact_matches,
            (unsigned long long)alpha_matches); goto done;
    }
    {
        const char *out_path = getenv("FIRESTAFF_NEXUS_YMIR_STEPPER_RGBA_OUT");
        if (out_path && *out_path && !write_rgba(out_path, surface)) {
            fprintf(stderr, "unable to write stepper diagnostic output: %s\n", out_path);
            goto done;
        }
    }
    printf("Ymir stepper composition: frame=%u draws=%u mode1=%u polygons=%u other_mode=%u opaque=%llu rgb_exact=%llu alpha_exact=%llu; DGN ownership claims=0; renderer_permitted=false\n",
        FRAME_INDEX,draws,mode1_count,polygon_count,other_modes,
        (unsigned long long)opaque,(unsigned long long)exact_matches,
        (unsigned long long)alpha_matches);
    printf("PASS: authentic 168-row trace and captured VDP1 framebuffer match exactly at 320x224; production renderer admission remains blocked\n");
    result=0;
done:
    free(trace_copy); free(le_vram); free(surface);
    for(i=0U;i<file_count;i++) free_blob(&blobs[i]);
    return result;
}
