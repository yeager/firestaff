#include "firestaff_x68k_media_receipt.h"
#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_vdp1_dgn_material_resolver.h"
#include "nexus_v1_vdp1_ymir_stepper_capture.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRAME_INDEX 300U
#define FIRST_DRAW 8U
#define LAST_DRAW 76U
#define EXPECTED_FRAME_ROWS 83U
#define VDP1_VRAM_SIZE 0x80000U
#define VDP1_FBS_SIZE 0x80000U
#define VDP1_DRAW_BUFFER_SIZE 0x40000U

typedef struct { uint8_t *bytes; size_t size; } Blob;

typedef enum {
    ROW_UNSET = 0,
    ROW_STATE,
    ROW_DRAW_SKIP,
    ROW_END,
    ROW_CLIPPED_NOOP,
    ROW_DGN_INDEXED,
    ROW_DGN_DIRECT_RGB,
    ROW_CAPTURED_TRANSPARENT,
    ROW_CAPTURED_POLYLINE,
    ROW_CAPTURED_POLYGON,
    ROW_CAPTURED_MODE4,
    ROW_RETAIL_MENU_BPK_MODE4
} RowSourceKind;

static const char *row_source_kind_name(RowSourceKind kind)
{
    switch (kind) {
    case ROW_STATE: return "state";
    case ROW_DRAW_SKIP: return "draw-skip";
    case ROW_END: return "end";
    case ROW_CLIPPED_NOOP: return "clipped-noop";
    case ROW_DGN_INDEXED: return "retail-dgn-indexed";
    case ROW_DGN_DIRECT_RGB: return "retail-dgn-direct-rgb";
    case ROW_CAPTURED_TRANSPARENT: return "captured-transparent";
    case ROW_CAPTURED_POLYLINE: return "captured-polyline";
    case ROW_CAPTURED_POLYGON: return "captured-polygon";
    case ROW_CAPTURED_MODE4: return "captured-mode4-vram";
    case ROW_RETAIL_MENU_BPK_MODE4: return "retail-menu-bpk-mode4";
    case ROW_UNSET: default: return "unset";
    }
}

static int write_owner_plane(const char *path,
    const Nexus_V1_Vdp1YmirCaptureSurface *surface)
{
    FILE *file;
    size_t i;
    if (!path || !*path || !surface) return 0;
    file = fopen(path, "wb");
    if (!file) return 0;
    for (i = 0U; i < (size_t)NEXUS_FB_W * NEXUS_FB_H; ++i) {
        uint16_t owner = surface->command_owner[i];
        uint8_t bytes[2] = {(uint8_t)owner, (uint8_t)(owner >> 8U)};
        if (fwrite(bytes, 1U, sizeof(bytes), file) != sizeof(bytes)) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0;
}

static int write_row_sources(const char *path,
    const RowSourceKind row_sources[EXPECTED_FRAME_ROWS])
{
    FILE *file;
    size_t i;
    if (!path || !*path || !row_sources) return 0;
    file = fopen(path, "wb");
    if (!file) return 0;
    if (fputs("frame,index,source_kind\n", file) < 0) {
        fclose(file);
        return 0;
    }
    for (i = 0U; i < EXPECTED_FRAME_ROWS; ++i) {
        if (fprintf(file, "%u,%lu,%s\n", FRAME_INDEX,
                    (unsigned long)i, row_source_kind_name(row_sources[i])) < 0) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0;
}

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
        "capture_frame_index=300",
        "capture_next_command_index=83",
        "capture_local_x=0",
        "capture_local_y=0",
        "trace_enabled=1",
        "trace_file_complete=1",
        "trace_rows=24983",
        "trace_rows_for_capture_frame=83",
        "trace_dropped_total=0"
    };
    size_t i;
    for (i = 0U; i < sizeof(required) / sizeof(required[0]); ++i)
        if (!strstr((const char *)bundle->bytes, required[i])) return 0;
    return 1;
}

static int is_frame300_clipped_mode0(const Nexus_V1_Vdp1TextureCommand *cmd,
                                     unsigned long address,
                                     long local_x, long local_y,
                                     int user_x0, int user_y0,
                                     int user_x1, int user_y1)
{
    int x, y;
    if (!cmd || (address != 0x0cbe0UL && address != 0x0cc20UL) ||
        cmd->command_type != 2U || cmd->colour_mode != 0U ||
        cmd->texture_byte_count != 0U ||
        cmd->xa != cmd->xb || cmd->xa != cmd->xc || cmd->xa != cmd->xd ||
        cmd->ya != cmd->yb || cmd->ya != cmd->yc || cmd->ya != cmd->yd ||
        (cmd->draw_mode & 0x0400U) == 0U ||
        (cmd->draw_mode & 0x0200U) != 0U ||
        user_x0 != 0 || user_y0 != 0 || user_x1 != 320 || user_y1 != 224)
        return 0;
    x = (int)local_x + cmd->xa;
    y = (int)local_y + cmd->ya;
    return x == 159 && y == -16;
}

static int choose_dump_root(const char *root, char *out, size_t cap)
{
    char path[4096];
    FILE *file;
    const char *suffixes[] = {"", "/dumps", "/profile-frame300/dumps"};
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
        {"vdp1-fbs.bin", "82caef1162741e5baca99e3a8747abb62bd0568df84c75cc0a781176b6636f04", VDP1_FBS_SIZE},
        {"vdp1-vram.bin", "223ae7def1993ae4b62d0e180044a831d194fa772429b102ff4ce47ecdb55be0", VDP1_VRAM_SIZE},
        {"vdp1-registers.bin", "8a37e4688358a55caaa29e8f9b437d95ebb288b19cb94e11f102e1b117f08313", 32U},
        {"vdp2-registers.bin", "e26d0632deaad24ae9672eff670536749cf86ae34fdf39b7ad8e06999ee9d607", 512U},
        {"vdp1-command-trace.csv", "5b898067f27b39710e4773ab027de202e9fc14e855e8eebca845f779c8f7875d", 0U},
        {"vdp1-capture-bundle.txt", "7825022e760eecaec329b1a376106635815f84817afde1212f691a4c227c7965", 0U}
    };
    const char *capture_env = getenv("FIRESTAFF_NEXUS_YMIR_GAMEPLAY_FRAME300_CAPTURE_DIR");
    const char *dgn_env = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char dump_root[4096], paths[sizeof(files) / sizeof(files[0])][4096];
    Blob blobs[sizeof(files) / sizeof(files[0])] = {{0}};
    Blob dgn = {NULL, 0U};
    Blob menu = {NULL, 0U}, cram = {NULL, 0U};
    uint8_t menu_sprite[16U * 41U];
    uint16_t menu_palette[NEXUS_V1_BPK_PALT_ENTRY_COUNT];
    uint8_t *le_vram = NULL;
    Nexus_V1_Vdp1YmirCaptureSurface *surface = NULL;
    Nexus_V1_Vdp1YmirCaptureReceipt receipt;
    Nexus_V1_Vdp1YmirCaptureInput input;
    Nexus_V1_Vdp1DgnMaterialResolverInput dgn_resolver = {0};
    uint8_t seen[EXPECTED_FRAME_ROWS] = {0};
    RowSourceKind row_sources[EXPECTED_FRAME_ROWS] = {ROW_UNSET};
    int16_t local_x = 0, local_y = 0;
    int user_x0 = 0, user_y0 = 0, user_x1 = 512, user_y1 = 256;
    int sys_clip_h = 511, sys_clip_v = 255;
    unsigned int frame_rows = 0U, draws = 0U, mode1_count = 0U;
    unsigned int polygon_count = 0U;
    unsigned int dgn_source_count = 0U, transparent_mode1_count = 0U;
    unsigned int direct_source_count = 0U, captured_mode4_count = 0U;
    unsigned int retail_menu_bpk_count = 0U;
    unsigned int mode0_noops = 0U;
    unsigned int skipped_draw_commands = 0U, end_commands = 0U;
    unsigned int user_clip_updates = 0U, system_clip_updates = 0U, local_updates = 0U;
    uint64_t opaque = 0U, exact_matches = 0U, alpha_matches = 0U;
    char *trace_copy = NULL, *line;
    size_t i, file_count = sizeof(files) / sizeof(files[0]);
    int result = 1;
    (void)argv;

    if (argc != 1 || !capture_env || !*capture_env || !dgn_env || !*dgn_env) {
        fprintf(stderr, "SKIP: set FIRESTAFF_NEXUS_YMIR_GAMEPLAY_FRAME300_CAPTURE_DIR and FIRESTAFF_NEXUS_DATA_DIR to authentic capture and retail game data\n");
        return 77;
    }
    if (!choose_dump_root(capture_env, dump_root, sizeof(dump_root))) {
        fprintf(stderr, "SKIP: frame-300 Ymir capture dumps are unavailable\n");
        return 77;
    }
    for (i = 0U; i < file_count; ++i) {
        if (snprintf(paths[i], sizeof(paths[i]), "%s/%s", dump_root,
                     files[i].name) < 0 || strlen(dump_root) + 1U + strlen(files[i].name) >= sizeof(paths[i])) {
            fprintf(stderr, "capture path is too long\n"); goto done;
        }
        blobs[i] = read_blob(paths[i]);
        if (!blobs[i].bytes) {
            fprintf(stderr, "SKIP: missing authentic frame-300 capture file %s\n", files[i].name);
            result = 77; goto done;
        }
        if ((files[i].size && blobs[i].size != files[i].size) ||
            !hash_is(&blobs[i], files[i].sha)) {
            fprintf(stderr, "capture hash or size mismatch: %s\n", files[i].name);
            goto done;
        }
    }
    if (!is_frame_receipt(&blobs[5])) {
        fprintf(stderr, "capture bundle receipt does not identify complete frame 300\n");
        goto done;
    }
    {
        char dgn_path[4096];
        int written = snprintf(dgn_path, sizeof(dgn_path), "%s/LEV01.DGN", dgn_env);
        if (written < 0 || (size_t)written >= sizeof(dgn_path)) {
            fprintf(stderr, "Nexus data path is too long\n"); goto done;
        }
        dgn = read_blob(dgn_path);
        if (!dgn.bytes) {
            fprintf(stderr, "SKIP: authentic Japanese LEV01.DGN is unavailable: %s\n", dgn_path);
            result = 77; goto done;
        }
        if (!hash_is(&dgn, "b359a89ffa344439b6d0d15de223f3211caa7725b13dfdb4461b579c48f06723") ||
            dgn.size > (size_t)INT_MAX) {
            fprintf(stderr, "retail LEV01.DGN hash or size does not match the pinned Japanese source\n");
            goto done;
        }
        dgn_resolver.dgn_bytes = dgn.bytes;
        dgn_resolver.dgn_byte_count = (int)dgn.size;
        dgn_resolver.source_hash_verified = 1;
        dgn_resolver.palette_slot_base = 0;
    }
    {
        char menu_path[4096], cram_path[4096];
        size_t menu_written = 0U;
        Nexus_V1_BpkSurfaceEntry menu_surface;
        Nexus_V1_BpkArchiveInfo menu_archive;
        uint64_t palette_hash = 0U;
        int n = snprintf(menu_path, sizeof(menu_path), "%s/MENU.BPK", dgn_env);
        int c = snprintf(cram_path, sizeof(cram_path), "%s/vdp2-cram.bin", dump_root);
        if (n < 0 || (size_t)n >= sizeof(menu_path) ||
            c < 0 || (size_t)c >= sizeof(cram_path)) {
            fprintf(stderr, "Nexus MENU.BPK or CRAM path is too long\n"); goto done;
        }
        menu = read_blob(menu_path);
        cram = read_blob(cram_path);
        if (!menu.bytes || !cram.bytes) {
            fprintf(stderr, "SKIP: authentic Japanese MENU.BPK or frame-300 VDP2 CRAM is unavailable\n");
            result = 77; goto done;
        }
        if (!hash_is(&menu, "740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636") ||
            cram.size != 4096U ||
            !hash_is(&cram, "faf2c8f3adbd4d89a562e28b744f772683cbbc822e0413e2088298a51a35f85a") ||
            nexus_v1_bpk_archive_parse(menu.bytes, menu.size, &menu_archive) != 0 ||
            menu_archive.entry_count_hint != 163U || menu_archive.prs3_payload_count != 162U ||
            nexus_v1_bpk_archive_decode_surface(menu.bytes, menu.size, 144U,
                menu_sprite, sizeof(menu_sprite), &menu_surface, &menu_written) != NEXUS_V1_BPK_DECODE_OK ||
            menu_written != sizeof(menu_sprite) || menu_surface.entry_index != 144U ||
            menu_surface.width != 16U || menu_surface.height != 41U ||
            nexus_v1_bpk_archive_copy_palette_words_be16(menu.bytes, menu.size,
                menu_palette, &palette_hash) != 0) {
            fprintf(stderr, "Japanese MENU.BPK entry 144 or PALT decode did not match its pinned retail form\n");
            goto done;
        }
        (void)palette_hash;
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
    for (i = 0U; i < (size_t)NEXUS_FB_W * NEXUS_FB_H; ++i)
        surface->command_owner[i] = NEXUS_V1_VDP1_YMIR_CAPTURE_NO_OWNER;
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
                 * in its link word. Every later trace row must match captured
                 * VRAM byte-for-byte. */
                if ((index != 0U && memcmp(command, le_vram + address,
                                           sizeof(command)) != 0) ||
                    nexus_v1_vdp1_texture_command_parse(command, sizeof(command), &parsed) != 0) {
                    fprintf(stderr,"invalid authenticated trace command at row %lu\n",index); goto done;
                }
                if ((int32_t)row_local_x != local_x || (int32_t)row_local_y != local_y) {
                    fprintf(stderr,"trace local-coordinate receipt mismatch at row %lu\n",index); goto done;
                }
                seen[index] = 1U; ++frame_rows;
                if (index == 0U) {
                    if ((parsed.control & 0x4000U) == 0U) {
                        fprintf(stderr, "frame-300 root command is not an authenticated draw skip\n");
                        goto done;
                    }
                    row_sources[index] = ROW_DRAW_SKIP;
                    ++skipped_draw_commands;
                    continue;
                }
                if (parsed.command_type == 10U) {
                    local_x = parsed.xa; local_y = parsed.ya; ++local_updates;
                    row_sources[index] = ROW_STATE;
                    continue;
                }
                if (parsed.command_type == 8U) {
                    user_x0 = (int)read_le16(command + 12U);
                    user_y0 = (int)read_le16(command + 14U);
                    user_x1 = (int)read_le16(command + 20U);
                    user_y1 = (int)read_le16(command + 22U);
                    ++user_clip_updates;
                    row_sources[index] = ROW_STATE;
                    continue;
                }
                if (parsed.command_type == 9U) {
                    sys_clip_h = (int)read_le16(command + 20U);
                    sys_clip_v = (int)read_le16(command + 22U);
                    ++system_clip_updates;
                    row_sources[index] = ROW_STATE;
                    continue;
                }
                if ((parsed.control & 0x4000U) != 0U) {
                    ++skipped_draw_commands;
                    row_sources[index] = ROW_DRAW_SKIP;
                    continue;
                }
                if (parsed.end_command) {
                    if (index != EXPECTED_FRAME_ROWS - 1U) {
                        fprintf(stderr, "unexpected early VDP1 end command at row %lu\n", index);
                        goto done;
                    }
                    ++end_commands;
                    row_sources[index] = ROW_END;
                    continue;
                }
                if ((index < FIRST_DRAW || index > LAST_DRAW) &&
                    index != 2U && index != 79U && index != 80U) {
                    row_sources[index] = ROW_STATE;
                    continue;
                }
                if (parsed.command_type <= 7U) ++draws;
                if (index == 2U) {
                    memset(&input, 0, sizeof(input));
                    input.command = command;
                    input.command_size = (int)sizeof(command);
                    input.command_capture_verified = 1;
                    input.command_index = (uint16_t)index;
                    input.local_origin_verified = 1;
                    input.local_origin_x = (int)row_local_x;
                    input.local_origin_y = (int)row_local_y;
                    input.system_clip_verified = 1;
                    input.sys_clip_h = sys_clip_h;
                    input.sys_clip_v = sys_clip_v;
                    input.single_density_verified = 1;
                    if (parsed.command_type != 4U ||
                        !nexus_v1_vdp1_ymir_capture_rasterize_polygon(
                            surface, &input, &receipt) || !receipt.valid ||
                        receipt.renderer_permitted || !receipt.capture_only) {
                        fprintf(stderr, "stepper rejected authentic frame-300 clear polygon row %lu\n", index);
                        goto done;
                    }
                    ++polygon_count;
                    row_sources[index] = ROW_CAPTURED_POLYGON;
                    continue;
                }
                if (index == 79U) {
                    memset(&input, 0, sizeof(input));
                    input.command = command;
                    input.command_size = (int)sizeof(command);
                    input.command_capture_verified = 1;
                    input.command_index = (uint16_t)index;
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
                    if (parsed.command_type != 5U ||
                        !nexus_v1_vdp1_ymir_capture_rasterize_polyline(
                            surface, &input, &receipt) || !receipt.valid ||
                        receipt.renderer_permitted || !receipt.capture_only) {
                        fprintf(stderr, "stepper rejected authentic frame-300 polyline row %lu\n", index);
                        goto done;
                    }
                    row_sources[index] = ROW_CAPTURED_POLYLINE;
                    continue;
                }
                if (index == 80U) {
                    if (parsed.command_type != 0U || parsed.colour_mode != 4U ||
                        !parsed.texture_source_range_valid ||
                        parsed.texture_byte_count != 656U) {
                        fprintf(stderr, "frame-300 normal sprite row %lu does not match the authenticated 8bpp bank-mode span\n", index);
                        goto done;
                    }
                    memset(&input, 0, sizeof(input));
                    input.command = command;
                    input.command_size = (int)sizeof(command);
                    input.texture_bytes = blobs[1].bytes + parsed.texture_source_byte_offset;
                    input.texture_size = (int)parsed.texture_byte_count;
                    input.command_capture_verified = 1;
                    input.texture_capture_verified = 1;
                    input.command_index = (uint16_t)index;
                    input.local_origin_verified = 1;
                    input.local_origin_x = (int)row_local_x;
                    input.local_origin_y = (int)row_local_y;
                    input.system_clip_verified = 1;
                    input.sys_clip_h = sys_clip_h;
                    input.sys_clip_v = sys_clip_v;
                    input.single_density_verified = 1;
                    if (parsed.texture_source_byte_offset != 0x643c0U ||
                        memcmp(blobs[1].bytes + parsed.texture_source_byte_offset,
                               menu_sprite, sizeof(menu_sprite)) != 0) {
                        fprintf(stderr, "frame-300 row 80 does not exactly join Japanese MENU.BPK entry 144\n");
                        goto done;
                    }
                    {
                        unsigned int used_index;
                        unsigned int matched_palette_indices = 0U;
                        uint8_t used[256] = {0};
                        size_t pixel;
                        for (pixel = 0U; pixel < sizeof(menu_sprite); ++pixel)
                            if (menu_sprite[pixel] != 0U) used[menu_sprite[pixel]] = 1U;
                        for (used_index = 1U; used_index < 256U; ++used_index) {
                            if (used[used_index] &&
                                menu_palette[used_index] != read_be16(cram.bytes + used_index * 2U)) {
                                fprintf(stderr, "MENU.BPK entry 144 PALT index %u differs from same-frame VDP2 CRAM\n", used_index);
                                goto done;
                            }
                            if (used[used_index]) ++matched_palette_indices;
                        }
                        if (matched_palette_indices != 3U) {
                            fprintf(stderr, "MENU.BPK entry 144 used palette-index count changed: %u\n",
                                    matched_palette_indices);
                            goto done;
                        }
                    }
                    ++retail_menu_bpk_count;
                    if (!nexus_v1_vdp1_ymir_capture_rasterize_mode4_normal_capture(
                            surface, &input, &receipt) || !receipt.valid ||
                        receipt.renderer_permitted || !receipt.capture_only ||
                        !receipt.texture_capture_verified ||
                        receipt.source_image_join_verified) {
                        fprintf(stderr, "stepper rejected authentic frame-300 normal sprite row %lu\n", index);
                        goto done;
                    }
                    ++captured_mode4_count;
                    row_sources[index] = ROW_RETAIL_MENU_BPK_MODE4;
                    continue;
                }
                if (parsed.command_type == 4U) {
                    memset(&input, 0, sizeof(input));
                    input.command = command;
                    input.command_size = (int)sizeof(command);
                    input.command_capture_verified = 1;
                    input.command_index = (uint16_t)index;
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
                    row_sources[index] = ROW_CAPTURED_POLYGON;
                    continue;
                }
                if (parsed.command_type != 2U) {
                    row_sources[index] = ROW_STATE;
                    continue;
                }
                if (parsed.colour_mode == 0U) {
                    if (!is_frame300_clipped_mode0(&parsed, address,
                            row_local_x, row_local_y, user_x0, user_y0,
                            user_x1, user_y1)) {
                        fprintf(stderr, "frame-300 mode-0 command is not proven clipped at row %lu\n", index);
                        goto done;
                    }
                    ++mode0_noops;
                    row_sources[index] = ROW_CLIPPED_NOOP;
                    continue;
                }
                if (!parsed.texture_command || (parsed.control & 0x4000U) != 0U ||
                    (parsed.colour_mode != 1U && parsed.colour_mode != 5U)) {
                    fprintf(stderr,"unexpected textured command mode in frame row %lu\n",index); goto done;
                }
                if (!parsed.texture_source_range_valid || !parsed.texture_byte_count ||
                    parsed.texture_source_byte_end > blobs[1].size) { fprintf(stderr,"invalid captured texture span\n"); goto done; }
                {
                    uint32_t palette_offset = parsed.colour_mode == 1U
                        ? (((uint32_t)parsed.colour_control & ~UINT32_C(3)) << 3U)
                        : 0U;
                    uint32_t texture_offset = parsed.texture_source_byte_offset;
                    Nexus_V1_Vdp1CaptureCompositeInput material;
                    const uint8_t *texture_bytes;
                    const uint8_t *palette_bytes;
                    uint8_t dgn_palette_le[32];
                    int dgn_joined;
                    if ((parsed.colour_mode == 1U && palette_offset > blobs[1].size - 32U) ||
                        texture_offset > blobs[1].size || parsed.texture_byte_count > blobs[1].size - texture_offset) {
                        fprintf(stderr,"captured texture or palette span out of bounds\n"); goto done;
                    }
                    memset(&material, 0, sizeof(material));
                    dgn_joined = nexus_v1_vdp1_dgn_material_resolver(
                        le_vram, (int)blobs[1].size, command,
                        (int)sizeof(command), &parsed, (uint32_t)address,
                        &material, &dgn_resolver);
                    if (parsed.colour_mode == 5U && dgn_joined &&
                        material.dgn_direct_source_hash_verified &&
                        material.dgn_direct_image &&
                        material.dgn_direct_image_size == (int)parsed.texture_byte_count) {
                        texture_bytes = material.dgn_direct_image;
                        palette_bytes = NULL;
                        ++direct_source_count;
                        row_sources[index] = ROW_DGN_DIRECT_RGB;
                    } else if (parsed.colour_mode == 1U && dgn_joined && material.dgn_source_hash_verified &&
                        material.dgn_image && material.dgn_palette &&
                        material.dgn_image_size == (int)parsed.texture_byte_count &&
                        material.dgn_palette_size == 32) {
                        unsigned int pal_byte;
                        texture_bytes = material.dgn_image;
                        for (pal_byte = 0U; pal_byte < 32U; pal_byte += 2U) {
                            dgn_palette_le[pal_byte] = material.dgn_palette[pal_byte + 1U];
                            dgn_palette_le[pal_byte + 1U] = material.dgn_palette[pal_byte];
                        }
                        palette_bytes = dgn_palette_le;
                        ++dgn_source_count;
                        row_sources[index] = ROW_DGN_INDEXED;
                    } else if (parsed.colour_mode == 1U && dgn_joined && material.transparent_capture_noop_verified) {
                        /* The 11 all-zero captured spans are verified transparent
                         * no-ops, not DGN-owned textures. Preserve their real bytes. */
                        texture_bytes = blobs[1].bytes + texture_offset;
                        palette_bytes = le_vram + palette_offset;
                        ++transparent_mode1_count;
                        row_sources[index] = ROW_CAPTURED_TRANSPARENT;
                    } else {
                        fprintf(stderr, "frame mode-1 row %lu has no unique retail DGN join or verified transparent no-op\n", index);
                        goto done;
                    }
                    memset(&input, 0, sizeof(input));
                    input.command = command; input.command_size = (int)sizeof(command);
                    input.texture_bytes = texture_bytes;
                    input.texture_size = (int)parsed.texture_byte_count;
                    input.palette_bytes = palette_bytes;
                    input.palette_size = parsed.colour_mode == 1U ? 32 : 0;
                    input.vdp1_vram = le_vram; input.vdp1_vram_size = (int)blobs[1].size;
                    input.command_capture_verified = 1;
                    input.command_index = (uint16_t)index;
                    /* Image and palette are retail DGN bytes for uniquely joined
                     * draws; the captured zero patterns above remain no-ops. */
                    input.source_image_join_verified = parsed.colour_mode == 1U;
                    input.direct_source_join_verified = parsed.colour_mode == 5U;
                    input.palette_join_verified = parsed.colour_mode == 1U;
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
                    if (!(parsed.colour_mode == 1U
                            ? nexus_v1_vdp1_ymir_capture_rasterize_mode1_distorted(
                                surface, &input, &receipt)
                            : nexus_v1_vdp1_ymir_capture_rasterize_mode5_distorted(
                                surface, &input, &receipt)) || !receipt.valid ||
                        receipt.renderer_permitted || !receipt.capture_only) {
                        fprintf(stderr,"stepper rejected authentic frame row %lu\n",index); goto done;
                    }
                    if (parsed.colour_mode == 1U) mode1_count++;
                }
            }
        }
    }
    if (frame_rows != EXPECTED_FRAME_ROWS ||
        mode1_count != 51U || dgn_source_count != 51U ||
        transparent_mode1_count != 0U || direct_source_count != 15U ||
        captured_mode4_count != 1U || retail_menu_bpk_count != 1U ||
        mode0_noops != 2U || draws != 71U ||
        skipped_draw_commands != 5U || end_commands != 1U ||
        user_clip_updates != 2U || local_updates != 4U ||
        polygon_count != 1U || system_clip_updates != 0U || !seen[0] || !seen[LAST_DRAW]) {
        fprintf(stderr,"incomplete authenticated frame receipt rows=%u draws=%u skipped=%u end=%u mode1=%u dgn=%u transparent=%u polygon=%u userclip=%u local=%u sysclip=%u\n",
            frame_rows,draws,skipped_draw_commands,end_commands,
            mode1_count,dgn_source_count,transparent_mode1_count,
            polygon_count,user_clip_updates,local_updates,system_clip_updates); goto done;
    }
    if (!is_frame_receipt(&blobs[5])) { fprintf(stderr,"frame receipt changed during validation\n"); goto done; }

    /* FBS buffer 0 is 512x256 BGR555, big-endian. Compare the 320x224
     * captured viewport after the retail DGN sources, authenticated no-op
     * spans, polylines, and mode-4 banked sprites have been replayed. */
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
    {
        const char *out_path = getenv("FIRESTAFF_NEXUS_YMIR_STEPPER_RGBA_OUT");
        if (out_path && *out_path && !write_rgba(out_path, surface)) {
            fprintf(stderr, "unable to write stepper diagnostic output: %s\n", out_path);
            goto done;
        }
    }
    if (opaque != UINT64_C(71189) || exact_matches != UINT64_C(71680) ||
        alpha_matches != UINT64_C(71680)) {
        fprintf(stderr,"captured VDP1 output differs: opaque=%llu rgb_exact=%llu alpha_exact=%llu; expected 71189 opaque and 71680 exact\n",
            (unsigned long long)opaque,(unsigned long long)exact_matches,
            (unsigned long long)alpha_matches); goto done;
    }
    {
        const char *owner_path = getenv("FIRESTAFF_NEXUS_YMIR_STEPPER_OWNER_OUT");
        const char *source_path = getenv("FIRESTAFF_NEXUS_YMIR_STEPPER_SOURCE_RECEIPT_OUT");
        for (i = 0U; i < EXPECTED_FRAME_ROWS; ++i) {
            if (row_sources[i] == ROW_UNSET) {
                fprintf(stderr, "frame-300 source receipt omitted command row %lu\n",
                        (unsigned long)i);
                goto done;
            }
        }
        if ((owner_path && *owner_path && !write_owner_plane(owner_path, surface)) ||
            (source_path && *source_path && !write_row_sources(source_path, row_sources))) {
            fprintf(stderr, "unable to write frame-300 command ownership evidence\n");
            goto done;
        }
    }
    printf("Ymir stepper composition: frame=%u draws=%u mode1=%u retail_dgn=%u direct_rgb_dgn=%u captured_mode4=%u retail_menu_bpk=%u mode0_clipped=%u skipped=%u ends=%u transparent_mode1=%u polygons=%u opaque=%llu rgb_exact=%llu alpha_exact=%llu; renderer_permitted=false\n",
        FRAME_INDEX,draws,mode1_count,dgn_source_count,direct_source_count,
        captured_mode4_count,retail_menu_bpk_count,
        mode0_noops,skipped_draw_commands,end_commands,
        transparent_mode1_count,polygon_count,
        (unsigned long long)opaque,(unsigned long long)exact_matches,
        (unsigned long long)alpha_matches);
    printf("PASS: retail DGN and Japanese MENU.BPK source joins plus captured-VRAM Ymir-stepper replay match the authentic VDP1 framebuffer exactly at 320x224; production renderer admission remains blocked\n");
    result=0;
done:
    free(trace_copy); free(le_vram); free(surface); free_blob(&dgn);
    free_blob(&menu); free_blob(&cram);
    for(i=0U;i<file_count;i++) free_blob(&blobs[i]);
    return result;
}
