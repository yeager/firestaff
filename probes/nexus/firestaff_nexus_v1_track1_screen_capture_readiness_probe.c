/*
 * firestaff_nexus_v1_track1_screen_capture_readiness_probe.c
 * ============================================================
 *
 * Skip-safe Nexus V1 real-screen capture readiness gate.
 *
 * Scope:
 *   - no-data hosts: write an explicit readiness manifest with status SKIP;
 *   - real Track 1 hosts: drive the runtime handoff across DM.BIN,
 *     FONT256.S2D, LEV00.DGN, and SCORPION.MNS, then render one 320x200
 *     Nexus viewport into a local PPM receipt.
 *
 * This is a readiness/provenance tool, not a final public screenshot
 * promotion. The PPM and JSON receipts are operator-local artifacts and
 * must be reviewed before any README/parity claim.
 *
 * Source anchors:
 *   src/nexus/nexus_v1_engine.c      nexus_v1_init/read_file/load_level/load_model
 *   src/nexus/nexus_v1_viewport.c    nexus_viewport_render/to_rgba
 *   src/nexus/nexus_v1_saturn_font.c FONT256.S2D parser/glyph draw
 *   docs/NEXUS_FILE_CLASSIFICATION.md DM.BIN/FONT256.S2D/MNS inventory
 *   docs/FIRESTAFF_GAP_LIST.md       Nexus Track 1 real screen capture row
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define FS_MKDIR(path_) _mkdir(path_)
#else
#include <unistd.h>
#define FS_MKDIR(path_) mkdir((path_), 0755)
#endif

#include "nexus_v1_dmdf_model.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_rasterizer.h"
#include "nexus_v1_saturn_font.h"
#include "nexus_v1_viewport.h"

static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;

#define CHECK(cond_, msg_) do {                                      \
    if (cond_) {                                                     \
        printf("  [PASS] %s\n", (msg_));                             \
        ++g_pass;                                                    \
    } else {                                                         \
        printf("  [FAIL] %s\n", (msg_));                             \
        ++g_fail;                                                    \
    }                                                                \
} while (0)

#define SKIP(msg_) do {                                              \
    printf("  [SKIP] %s\n", (msg_));                                 \
    ++g_skip;                                                        \
} while (0)

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t h = UINT64_C(0xcbf29ce484222325);
    size_t i;
    if (!data) return 0;
    for (i = 0; i < size; ++i) {
        h ^= data[i];
        h *= UINT64_C(0x100000001b3);
    }
    return h;
}

static int path_exists(const char *path)
{
    struct stat st;
    return path && path[0] && stat(path, &st) == 0;
}

static int ensure_dir(const char *path)
{
    if (!path || !path[0]) return 0;
    if (FS_MKDIR(path) == 0 || errno == EEXIST) return 1;
    return 0;
}

static const char *default_data_dir(char *buf, size_t cap)
{
    const char *env = getenv("FIRESTAFF_NEXUS_TRACK1_DATA_DIR");
    const char *home;
    if (env && env[0]) return env;
    home = getenv("HOME");
    if (!home || !home[0] || cap == 0) return NULL;
    snprintf(buf, cap, "%s/.firestaff/data/nexus", home);
    return buf;
}

static const char *default_output_dir(char *buf, size_t cap)
{
    const char *env = getenv("FIRESTAFF_NEXUS_TRACK1_SCREEN_READY_OUT");
    const char *home;
    if (env && env[0]) return env;
    home = getenv("HOME");
    if (!home || !home[0] || cap == 0) return NULL;
    snprintf(buf, cap, "%s/.firestaff/nexus-track1-screen-readiness", home);
    return buf;
}

static int prepare_output_dir(const char *dir)
{
    char parent[1024];
    const char *home;

    if (!dir || !dir[0]) return 0;
    home = getenv("HOME");
    if (home && home[0]) {
        snprintf(parent, sizeof(parent), "%s/.firestaff", home);
        (void)ensure_dir(parent);
    }
    return ensure_dir(dir);
}

static void json_escape(FILE *fp, const char *s)
{
    const unsigned char *p = (const unsigned char *)s;
    if (!fp) return;
    if (!p) p = (const unsigned char *)"";
    while (*p) {
        if (*p == '\\' || *p == '"') {
            fputc('\\', fp);
            fputc((int)*p, fp);
        } else if (*p >= 32 && *p < 127) {
            fputc((int)*p, fp);
        } else {
            fprintf(fp, "\\u%04x", (unsigned int)*p);
        }
        ++p;
    }
}

static int write_manifest(const char *path,
                          const char *status,
                          const char *data_dir,
                          const char *source_name,
                          const char *capture_path,
                          uint64_t dm_hash,
                          int dm_size,
                          uint64_t font_hash,
                          int font_size,
                          int model_index,
                          int model_vertices,
                          int model_faces,
                          int level_w,
                          int level_h,
                          int viewport_nonzero,
                          int font_glyph_index,
                          int font_writes,
                          uint64_t viewport_hash)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"schema\": \"firestaff.nexus_v1.track1_screen_capture_readiness.v1\",\n");
    fprintf(fp, "  \"status\": \"");
    json_escape(fp, status);
    fprintf(fp, "\",\n");
    fprintf(fp, "  \"data_dir\": \"");
    json_escape(fp, data_dir);
    fprintf(fp, "\",\n");
    fprintf(fp, "  \"source\": \"");
    json_escape(fp, source_name);
    fprintf(fp, "\",\n");
    fprintf(fp, "  \"dm_bin\": { \"size\": %d, \"fnv1a64\": \"0x%016llx\", \"min_size_ok\": %s },\n",
            dm_size, (unsigned long long)dm_hash,
            dm_size >= 542144 ? "true" : "false");
    fprintf(fp, "  \"font256_s2d\": { \"size\": %d, \"fnv1a64\": \"0x%016llx\", \"glyph_index\": %d, \"glyph_writes\": %d },\n",
            font_size, (unsigned long long)font_hash,
            font_glyph_index, font_writes);
    fprintf(fp, "  \"mns_model\": { \"name\": \"SCORPION.MNS\", \"index\": %d, \"vertices\": %d, \"faces\": %d },\n",
            model_index, model_vertices, model_faces);
    fprintf(fp, "  \"level0\": { \"width\": %d, \"height\": %d },\n", level_w, level_h);
    fprintf(fp, "  \"viewport\": { \"width\": %d, \"height\": %d, \"nonzero_pixels\": %d, \"fnv1a64\": \"0x%016llx\", \"ppm_path\": \"",
            NEXUS_FB_W, NEXUS_FB_H, viewport_nonzero,
            (unsigned long long)viewport_hash);
    json_escape(fp, capture_path);
    fprintf(fp, "\" },\n");
    fprintf(fp, "  \"non_claims\": [\n");
    fprintf(fp, "    \"readiness receipt only\",\n");
    fprintf(fp, "    \"not original Saturn pixel parity\",\n");
    fprintf(fp, "    \"not README/public screenshot promotion\",\n");
    fprintf(fp, "    \"MNS model is loaded for handoff provenance but not yet rendered as a creature\"\n");
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    fclose(fp);
    return 1;
}

static int write_ppm_rgba(const char *path, const uint32_t *rgba, int w, int h)
{
    FILE *fp;
    int i;
    if (!path || !rgba || w <= 0 || h <= 0) return 0;
    fp = fopen(path, "wb");
    if (!fp) return 0;
    fprintf(fp, "P6\n%d %d\n255\n", w, h);
    for (i = 0; i < w * h; ++i) {
        uint32_t px = rgba[i];
        fputc((int)((px >> 16) & 0xFFu), fp);
        fputc((int)((px >> 8) & 0xFFu), fp);
        fputc((int)(px & 0xFFu), fp);
    }
    fclose(fp);
    return 1;
}

static int count_nonzero(const uint8_t *fb, int pixels)
{
    int i;
    int n = 0;
    if (!fb || pixels <= 0) return 0;
    for (i = 0; i < pixels; ++i) {
        if (fb[i] != 0) ++n;
    }
    return n;
}

static int glyph_foreground_count(const Nexus_V1_Font *font, int glyph)
{
    int x, y;
    int n = 0;
    if (!font) return 0;
    for (y = 0; y < font->char_height; ++y) {
        for (x = 0; x < font->char_width; ++x) {
            if (nexus_v1_font_get_glyph_pixel(font, glyph, x, y)) ++n;
        }
    }
    return n;
}

static int find_drawable_glyph(const Nexus_V1_Font *font)
{
    int i;
    if (!font) return -1;
    for (i = 0; i < font->char_count; ++i) {
        if (glyph_foreground_count(font, i) > 0) return i;
    }
    return -1;
}

static void probe_no_data_skip(const char *data_dir, const char *manifest_path)
{
    printf("\n[No-data readiness manifest]\n");
    SKIP("no Nexus Track 1 data root found; wrote SKIP readiness manifest");
    CHECK(write_manifest(manifest_path, "SKIP_NO_TRACK1_DATA",
                         data_dir ? data_dir : "",
                         "none", "", 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, -1, 0, 0),
          "skip manifest written");
}

static void probe_real_data(const char *data_dir,
                            const char *manifest_path,
                            const char *capture_path)
{
    Nexus_V1_Engine engine;
    Nexus_Viewport vp;
    uint8_t *dm = NULL;
    uint8_t *font = NULL;
    uint32_t rgba[NEXUS_FB_W * NEXUS_FB_H];
    int dm_size = 0;
    int font_size = 0;
    int model_index = -1;
    int font_glyph = -1;
    int font_writes = -1;
    int nonzero = 0;
    uint64_t dm_hash = 0;
    uint64_t font_hash = 0;
    uint64_t viewport_hash = 0;
    const char *source_name = "none";

    printf("\n[Real Track 1 runtime handoff]\n");
    memset(&engine, 0, sizeof(engine));
    CHECK(nexus_v1_init(&engine, data_dir) == 0,
          "nexus_v1_init succeeds on supplied Track 1 root");
    if (!engine.initialized) return;

    source_name = (engine.source == NEXUS_SRC_ISO) ? "iso" :
                  (engine.source == NEXUS_SRC_EXTRACTED) ? "extracted" : "none";
    CHECK(engine.source == NEXUS_SRC_ISO || engine.source == NEXUS_SRC_EXTRACTED,
          "engine source is ISO or extracted");

    dm = nexus_v1_read_file(&engine, "DM.BIN", &dm_size);
    CHECK(dm != NULL && dm_size >= 542144,
          "DM.BIN is readable and meets the documented 542 KB floor");
    if (dm) dm_hash = fnv1a64(dm, (size_t)dm_size);

    CHECK(nexus_v1_load_level(&engine, 0) == 0,
          "LEV00.DGN loads through nexus_v1_load_level");
    CHECK(engine.level_loaded == 1 && engine.current_level.width == 64 &&
          engine.current_level.height == 64,
          "level 0 is resident as a 64x64 Nexus level");

    font = nexus_v1_read_file(&engine, "FONT256.S2D", &font_size);
    CHECK(font != NULL && font_size > 0,
          "FONT256.S2D is readable through engine file reader");
    if (font) font_hash = fnv1a64(font, (size_t)font_size);
    CHECK(engine.font_loaded == 1 && engine.font.char_count >= 256,
          "engine font is loaded with at least 256 glyph slots");

    model_index = nexus_v1_load_model(&engine, "SCORPION.MNS");
    CHECK(model_index >= 0, "SCORPION.MNS loads through nexus_v1_load_model");
    CHECK(model_index >= 0 &&
          engine.models[model_index].header.magic == 0x444D4446u &&
          engine.model_count == model_index + 1,
          "SCORPION.MNS is recorded in the model pool with a DMDF header");

    nexus_viewport_init(&vp);
    nexus_viewport_render(&vp, &engine);
    nonzero = count_nonzero(vp.fb.color_buffer, NEXUS_FB_W * NEXUS_FB_H);
    CHECK(nonzero > 0, "runtime viewport render writes non-zero indexed pixels");

    font_glyph = find_drawable_glyph(&engine.font);
    CHECK(font_glyph >= 0, "real FONT256.S2D exposes at least one drawable glyph");
    if (font_glyph >= 0) {
        font_writes = nexus_v1_font_draw_glyph_indexed(
            &engine.font, vp.fb.color_buffer,
            NEXUS_FB_W, NEXUS_FB_H, NEXUS_FB_W,
            8, 8, font_glyph, 15, -1);
    }
    CHECK(font_writes > 0, "real font glyph stamps into the readiness framebuffer");

    viewport_hash = fnv1a64(vp.fb.color_buffer, sizeof(vp.fb.color_buffer));
    nexus_viewport_to_rgba(&vp, rgba);
    CHECK(write_ppm_rgba(capture_path, rgba, NEXUS_FB_W, NEXUS_FB_H),
          "readiness PPM capture written");
    CHECK(write_manifest(manifest_path, "READY_FOR_OPERATOR_REVIEW",
                         data_dir, source_name, capture_path,
                         dm_hash, dm_size, font_hash, font_size,
                         model_index,
                         model_index >= 0 ? engine.models[model_index].vertex_count : 0,
                         model_index >= 0 ? engine.models[model_index].face_count : 0,
                         engine.current_level.width, engine.current_level.height,
                         nonzero, font_glyph, font_writes, viewport_hash),
          "readiness JSON manifest written");

    free(dm);
    free(font);
    nexus_v1_shutdown(&engine);
}

int main(int argc, char **argv)
{
    char data_buf[1024];
    char out_buf[1024];
    char manifest_path[1280];
    char capture_path[1280];
    const char *data_dir = (argc > 1 && argv[1] && argv[1][0])
        ? argv[1]
        : default_data_dir(data_buf, sizeof(data_buf));
    const char *out_dir = default_output_dir(out_buf, sizeof(out_buf));

    printf("Nexus V1 Track 1 screen capture readiness probe\n");
    printf("  data_dir: %s\n", data_dir ? data_dir : "(none)");
    printf("  output:   %s\n", out_dir ? out_dir : "(none)");

    if (!out_dir || !prepare_output_dir(out_dir)) {
        CHECK(0, "output directory is available");
        printf("\nRESULT: %d passed, %d failed, %d skipped\n", g_pass, g_fail, g_skip);
        return 1;
    }

    snprintf(manifest_path, sizeof(manifest_path),
             "%s/nexus_track1_screen_readiness.json", out_dir);
    snprintf(capture_path, sizeof(capture_path),
             "%s/nexus_track1_readiness.ppm", out_dir);

    if (!data_dir || !path_exists(data_dir)) {
        probe_no_data_skip(data_dir, manifest_path);
    } else {
        probe_real_data(data_dir, manifest_path, capture_path);
    }

    printf("\nRESULT: %d passed, %d failed, %d skipped\n", g_pass, g_fail, g_skip);
    printf("Manifest: %s\n", manifest_path);
    if (path_exists(capture_path)) {
        printf("Capture:  %s\n", capture_path);
    }

    return g_fail == 0 ? 0 : 1;
}
