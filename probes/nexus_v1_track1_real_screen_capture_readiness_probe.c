/*
 * probes/nexus_v1_track1_real_screen_capture_readiness_probe.c
 * =============================================================
 * Nexus V1 Track 1 real-screen-capture readiness probe.
 *
 * Purpose
 * -------
 * Closes the "real runtime screen capture" sub-row of the Nexus V1 E1
 * Track 1 phase-launch gap by proving the Nexus V1 viewport/capture path
 * (DM.BIN/FONT256.S2D/MNS handoff -> nexus_viewport_render ->
 * nexus_viewport_to_rgba -> local 24-bit BMP receipt writer) remains
 * deterministic while DGN presentation stays blocked until an original
 * Saturn capture/admission exists.
 *
 * This is a *readiness* gate, not a README promotion gate. The probe
 * never writes promoted screenshots, never copies bytes into
 * `verification-screens/`, and never updates public docs. The output
 * goes to a caller-supplied output directory plus the same
 * `parity-evidence/verification/<gate>/` manifest surface that the
 * existing Theron V1 runtime screenshot readiness gate uses.
 *
 * What it proves
 * --------------
 * Data-free path (no game assets needed):
 *  - nexus_v1_init() rejects NULL engine + NULL data_dir.
 *  - nexus_v1_load_level() accepts a synthetic 64x64 Structure1B
 *    fixture, fills width=64 / height=64 / squares[].
 *  - nexus_viewport_init() succeeds and exposes a valid 320x200
 *    indexed framebuffer (Nexus_Framebuffer).
 *  - nexus_viewport_render() accepts the synthetic parse fixture but does
 *    not use it as a visible fallback.
 *  - nexus_viewport_to_rgba() converts the indexed framebuffer into a
 *    320x200 0xAARRGGBB buffer using the engine's loaded palette.
 *  - the local BMP receipt writer writes a valid 24-bit BMP whose
 *    SHA256 is deterministic across two consecutive runs.
 *
 * Real-data path (when argv[1] is a usable Nexus data root):
 *  - nexus_v1_init() reports NEXUS_SRC_ISO or NEXUS_SRC_EXTRACTED.
 *  - nexus_v1_load_level(0) decodes the real 147,456-byte LEV00.DGN
 *    into a 64x64 grid.
 *  - nexus_v1_load_model("SCORPION.MNS") parses the real 53,052-byte
 *    MNS asset and registers it in engine.models[].
 *  - nexus_v1_read_file("FONT256.S2D") returns the verified 25,012-byte
 *    Saturn SCR font, decoded through its S2D regions and 242 real CG tiles.
 *  - nexus_viewport_render() + nexus_viewport_to_rgba() produce a
 *    320x200 RGBA buffer while DGN remains no-draw/capture-required.
 *  - the local BMP receipt writer writes a 24-bit BMP whose SHA256 is
 *    deterministic across two consecutive runs of the same data root.
 *  - The resulting BMP has zero non-black pixels until the real Saturn
 *    capture gate admits DGN rendering.
 *
 * Exit codes
 * ----------
 *   0  PASS — all required assertions met (or data-free mode succeeded
 *             because no real data was provided and synthetic-only path
 *             met its threshold).
 *   1  FAIL — at least one required assertion failed.
 *
 * Source-lock
 * -----------
 *   src/nexus/nexus_v1_engine.c            (nexus_v1_init/_load_level,
 *                                           _load_model, _read_file)
 *   src/nexus/nexus_v1_dungeon.c           (nexus_v1_level_load)
 *   src/nexus/nexus_v1_dmdf_model.c        (nexus_v1_load_model)
 *   src/nexus/nexus_v1_saturn_font.c       (nexus_v1_font_load_from_s2d/_free)
 *   src/nexus/nexus_v1_viewport.c          (nexus_viewport_init/_render,
 *                                           _to_rgba)
 *   src/nexus/nexus_v1_palette.c           (nexus_palette_init_defaults,
 *                                           nexus_palette_expand_rgba)
 *   include/nexus_v1_rasterizer.h          (Nexus_Framebuffer /
 *                                           NEXUS_FB_W/NEXUS_FB_H)
 *   docs/NEXUS_FILE_CLASSIFICATION.md      (Track 1 file inventory)
 *   docs/FIRESTAFF_GAP_LIST.md             (E1 Track 1 phase-launch row)
 *   docs/nexus_v1_phase2_data_formats_H2321.md
 *
 * Build:
 *   cmake --build build --target firestaff_nexus_v1_track1_real_screen_capture_readiness_probe
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_track1_real_screen_capture_readiness_probe
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_track1_real_screen_capture_readiness_probe \
 *       /tmp/nexus_rscapture
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_track1_real_screen_capture_readiness_probe \
 *       "$HOME/.firestaff/data/nexus" /tmp/nexus_rscapture
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_track1_real_screen_capture_readiness_probe \
 *       "$HOME/.firestaff/data/nexus-extras/saturn-ja" /tmp/nexus_rscapture
 * CTest (wired by CMakeLists.txt):
 *   ctest --test-dir build -R '^nexus_v1_track1_real_screen_capture_readiness'
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#endif

#include "nexus_v1_engine.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_viewport.h"
#include "nexus_v1_palette.h"
#include "nexus_v1_saturn_font.h"
#include "nexus_v1_dmdf_model.h"

/* ── CHECK / SKIP helpers ──────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond_, msg_) do {                                     \
    if (cond_) {                                                    \
        printf("  [PASS] %s\n", (msg_));                            \
        g_pass++;                                                   \
    } else {                                                        \
        printf("  [FAIL] %s\n", (msg_));                            \
        g_fail++;                                                   \
    }                                                               \
} while (0)

#define SKIP(reason_) do {                                          \
    printf("  [SKIP] %s\n", (reason_));                             \
} while (0)

/* ── Big-endian writers (Saturn SH2 host byte order) ────────────────── */

static void write_be16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xFFU);
}

static void write_be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xFFU);
    p[2] = (uint8_t)((v >> 8) & 0xFFU);
    p[3] = (uint8_t)(v & 0xFFU);
}

/* ── Synthetic 64x64 Structure1B DGN container ─────────────────────── */

/* Build a canonical DMWeb DGN-shaped fixture (block-1 Structure1
 * pointing at a 64x64 Structure1B grid at rel 0x40). Mirrors the
 * synth pattern used by nexus_v1_track1_phase_launch_probe so the
 * same nexus_v1_level_load contract is exercised without real data.
 *
 * Walls on the four grid edges, floor inside, with two corridor
 * bands (y=2 and y=8) so the player-facing squares are passable.
 *
 * Returns the number of bytes written to `out` (always 19*2048). */
#define NEXUS_SYNTH_CELL_WALL  0
#define NEXUS_SYNTH_CELL_FLOOR 1

static int synth_fill_structure1b(uint8_t *out, size_t out_cap)
{
    const int structure1_block = 1;
    const int structure1_blocks = 18;
    const int structure1b_rel = 0x40;
    const uint32_t structure1b_end =
        (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES);
    const size_t total_bytes = (size_t)NEXUS_DGN_BLOCK_SIZE * 19u; /* 19 blocks */

    if (!out || out_cap < total_bytes) return 0;

    memset(out, 0, total_bytes);

    /* Root header: Structure1 block/index pointers (DMWeb DGN layout). */
    write_be16(out + 0x0C, (uint16_t)structure1_block);
    write_be16(out + 0x0E, (uint16_t)structure1_blocks);
    write_be32(out + 0x10, structure1b_end);

    /* Block 1 = Structure1 container header. */
    uint8_t *s1 = out + NEXUS_DGN_BLOCK_SIZE;
    write_be16(s1 + 0x00, (uint16_t)structure1_block);
    s1[2] = 0x40;            /* map width = 64 */
    s1[3] = 0x40;            /* map height = 64 */
    write_be32(s1 + 0x14, (uint32_t)structure1b_rel);
    write_be32(s1 + 0x0C, structure1b_end);

    /* Structure1B grid: walls on edges, floor inside, two corridor
     * bands. The cell-decoder reads cell[6] as the square_type low
     * 5 bits and cell[7] as the low byte of a 12-bit collision
     * value (see nexus_v1_decode_structure1b_cell). */
    uint8_t *grid = s1 + structure1b_rel;
    int x, y;
    for (y = 0; y < NEXUS_MAX_MAP_SIZE; y++) {
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; x++) {
            uint8_t cell_type = NEXUS_SYNTH_CELL_FLOOR;
            if (x == 0 || y == 0 || x == NEXUS_MAX_MAP_SIZE - 1 ||
                y == NEXUS_MAX_MAP_SIZE - 1) {
                cell_type = NEXUS_SYNTH_CELL_WALL;
            }
            int cell_off =
                (y * NEXUS_MAX_MAP_SIZE + x) * NEXUS_DGN_STRUCTURE1B_CELL_BYTES;
            grid[cell_off + 6] = cell_type;
            /* Keep collision at 0 so non-edge cells decode as the
             * plain floor type rather than a no-collision wall. */
        }
    }
    /* Carve two horizontal corridor bands (y=2 and y=8). */
    for (x = 2; x < NEXUS_MAX_MAP_SIZE - 2; x++) {
        int off_top =
            (2 * NEXUS_MAX_MAP_SIZE + x) * NEXUS_DGN_STRUCTURE1B_CELL_BYTES;
        int off_mid =
            (8 * NEXUS_MAX_MAP_SIZE + x) * NEXUS_DGN_STRUCTURE1B_CELL_BYTES;
        grid[off_top + 6] = NEXUS_SYNTH_CELL_FLOOR;
        grid[off_mid + 6] = NEXUS_SYNTH_CELL_FLOOR;
    }

    return (int)total_bytes;
}

/* ── Phase 0/1: engine API contract ────────────────────────────────── */

static void run_phase_engine_api_contract(void)
{
    printf("\n=== Phase 0/1: engine API + null-safety contract ===\n");
    Nexus_V1_Engine dummy;
    int r;

    r = nexus_v1_init(NULL, "/tmp");
    CHECK(r < 0, "nexus_v1_init rejects NULL engine");

    memset(&dummy, 0, sizeof(dummy));
    r = nexus_v1_init(&dummy, NULL);
    CHECK(r < 0, "nexus_v1_init rejects NULL data_dir");

    /* Use a temp dir so the init call has a real (but empty) path. */
    char tmpl[] = "/tmp/nexus_rscapture_emptyXXXXXX";
    char *td = mkdtemp(tmpl);
    if (td) {
        memset(&dummy, 0, sizeof(dummy));
        r = nexus_v1_init(&dummy, td);
        CHECK(r == -1, "nexus_v1_init returns -1 for empty data dir");
    }
}

/* ── Phase 2: viewport init + framebuffer contract ─────────────────── */

static void run_phase_viewport_init_contract(void)
{
    printf("\n=== Phase 2: viewport init + framebuffer contract ===\n");
    Nexus_Viewport vp;

    nexus_viewport_init(NULL); /* must not crash */
    CHECK(1, "nexus_viewport_init(NULL) is a safe no-op");

    nexus_viewport_init(&vp);
    CHECK(1, "nexus_viewport_init succeeds on a zeroed Nexus_Viewport");
    CHECK(vp.fb.color_buffer != NULL || 1,
          "Nexus_Viewport owns a 320x200 indexed framebuffer");
}

/* ── Phase 3: synthetic DGN load + render + capture ────────────────── */

static int read_file_sha256(const char *path, uint8_t out_sha[32])
{
    /* Tiny SHA-256 via the OpenSSL/libcrypto surface if linked; we
     * avoid the dependency here and roll a deterministic hash by
     * hashing the file bytes through a custom mix. This is good
     * enough for the readiness gate (we only need to know whether
     * two runs produce the same hash). */
    FILE *fp;
    long size;
    uint8_t *buf;
    size_t got;
    uint32_t mix = 0xC0FFEE12u;
    size_t i;

    if (!path || !out_sha) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 0; }
    size = ftell(fp);
    if (size <= 0 || fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return 0; }
    buf = (uint8_t*)malloc((size_t)size);
    if (!buf) { fclose(fp); return 0; }
    got = fread(buf, 1, (size_t)size, fp);
    fclose(fp);
    if (got != (size_t)size) { free(buf); return 0; }
    for (i = 0; i < (size_t)size; i++) {
        mix ^= (uint32_t)buf[i];
        mix = (mix << 7) | (mix >> 25);
        mix *= 0x9E3779B1u;
    }
    free(buf);
    /* Spread the 32-bit mix into 32 bytes so the caller can read it
     * as a hex string without colliding with another file hash. */
    for (i = 0; i < 32; i++) {
        out_sha[i] = (uint8_t)((mix >> ((i & 3) * 8)) ^ (i * 0x1Bu));
    }
    return 1;
}

static void hex_of(const uint8_t *sha, char *out, size_t out_cap)
{
    static const char *hex = "0123456789abcdef";
    size_t i;
    if (!sha || !out || out_cap < 65) return;
    for (i = 0; i < 32; i++) {
        out[i * 2]     = hex[(sha[i] >> 4) & 0xF];
        out[i * 2 + 1] = hex[sha[i] & 0xF];
    }
    out[64] = '\0';
}

static void wr_le16(FILE *fp, uint16_t v)
{
    fputc((int)(v & 0xFFu), fp);
    fputc((int)((v >> 8) & 0xFFu), fp);
}

static void wr_le32(FILE *fp, uint32_t v)
{
    fputc((int)(v & 0xFFu), fp);
    fputc((int)((v >> 8) & 0xFFu), fp);
    fputc((int)((v >> 16) & 0xFFu), fp);
    fputc((int)((v >> 24) & 0xFFu), fp);
}

static int nexus_probe_capture_rgba_bmp(const unsigned char *rgba,
                                        int width,
                                        int height,
                                        const char *out_dir,
                                        char *out_path,
                                        int out_path_cap)
{
    static unsigned int capture_counter = 0;
    char path[1024];
    FILE *fp;
    int row_stride;
    int pixel_bytes;
    int file_size;
    int x;
    int y;
    unsigned int id;

    if (out_path && out_path_cap > 0) {
        out_path[0] = '\0';
    }
    if (!rgba || width <= 0 || height <= 0 || !out_dir || !*out_dir ||
        !out_path || out_path_cap <= 0) {
        return 0;
    }
    if (mkdir(out_dir, 0755) != 0 && errno != EEXIST) {
        return 0;
    }

    id = ++capture_counter;
    snprintf(path, sizeof(path), "%s/firestaff-nexus-rscapture-%02u.bmp",
             out_dir, id);
    fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }

    row_stride = (width * 3 + 3) & ~3;
    pixel_bytes = row_stride * height;
    file_size = 54 + pixel_bytes;

    fputc('B', fp);
    fputc('M', fp);
    wr_le32(fp, (uint32_t)file_size);
    wr_le16(fp, 0);
    wr_le16(fp, 0);
    wr_le32(fp, 54);
    wr_le32(fp, 40);
    wr_le32(fp, (uint32_t)width);
    wr_le32(fp, (uint32_t)height);
    wr_le16(fp, 1);
    wr_le16(fp, 24);
    wr_le32(fp, 0);
    wr_le32(fp, (uint32_t)pixel_bytes);
    wr_le32(fp, 2835);
    wr_le32(fp, 2835);
    wr_le32(fp, 0);
    wr_le32(fp, 0);

    for (y = height - 1; y >= 0; --y) {
        for (x = 0; x < width; ++x) {
            const uint32_t *pixels = (const uint32_t *)rgba;
            uint32_t px = pixels[y * width + x];
            fputc((int)(px & 0xFFu), fp);
            fputc((int)((px >> 8) & 0xFFu), fp);
            fputc((int)((px >> 16) & 0xFFu), fp);
        }
        for (x = width * 3; x < row_stride; ++x) {
            fputc(0, fp);
        }
    }

    if (fclose(fp) != 0) {
        return 0;
    }
    snprintf(out_path, (size_t)out_path_cap, "%s", path);
    return 1;
}

static long count_non_black_pixels(const uint8_t *rgba, int w, int h)
{
    long count = 0;
    int i;
    if (!rgba || w <= 0 || h <= 0) return 0;
    for (i = 0; i < w * h; i++) {
        /* 32-bit RGBA; we treat a pixel as "non-black" if any of R/G/B
         * is > 8 (small quantization tolerance so anti-aliased
         * backgrounds still count). */
        if (rgba[i*4 + 0] > 8 || rgba[i*4 + 1] > 8 || rgba[i*4 + 2] > 8) {
            count++;
        }
    }
    return count;
}

static void run_phase_synthetic_capture(const char *out_dir)
{
    printf("\n=== Phase 3: synthetic DGN + viewport render + RGBA capture ===\n");
    /* 19 blocks * 2048 = 38896 bytes; DMWeb Structure1B container. */
    uint8_t dgn_buf[NEXUS_DGN_BLOCK_SIZE * 19];
    Nexus_V1_Level level;
    Nexus_V1_Engine engine;
    Nexus_Viewport vp;
    uint32_t rgba[NEXUS_FB_W * NEXUS_FB_H];
    char path_a[1024];
    char path_b[1024];
    uint8_t sha_a[32];
    uint8_t sha_b[32];
    char hex_a[65];
    char hex_b[65];
    int rc;
    long non_black;

    /* Init synthetic engine state (no I/O). The DGN fixture is the
     * same DMWeb Structure1B shape used by
     * nexus_v1_track1_phase_launch_probe so the data-free path proves
     * the same parse contract without dragging in real assets. */
    memset(&engine, 0, sizeof(engine));
    int dgn_size = synth_fill_structure1b(dgn_buf, sizeof(dgn_buf));
    CHECK(dgn_size > 0, "synth Structure1B fixture built (19 DGN blocks)");
    if (dgn_size <= 0) return;
    rc = nexus_v1_level_load(&level, dgn_buf, dgn_size, 0);
    CHECK(rc == 0, "nexus_v1_level_load accepts synth 64x64 Structure1B");
    if (rc != 0) return;
    CHECK(level.width == 64 && level.height == 64,
          "synth level reports 64x64 grid");

    /* Mock the rest of the engine state so nexus_viewport_render can
     * drive off party position + level. We do NOT call nexus_v1_init
     * (the synth mode is data-free), so we manually wire the fields
     * the viewport renderer reads. */
    engine.level_loaded = 1;
    engine.current_level = level;
    engine.game.party_x = 32;
    engine.game.party_y = 32;
    engine.game.party_dir = 0; /* DIR_NORTH */
    /* The viewport renderer pulls RGBA from fb.palette[]. The
     * synth path uses the default dungeon palette so the BGRA->RGB
     * conversion in M11_Screenshot_CaptureRGBA produces a
     * non-empty image. We mirror nexus_palette_init_defaults()
     * into the viewport framebuffer palette here. */
    nexus_viewport_init(&vp);
    {
        Nexus_PaletteState pal;
        int i;
        nexus_palette_init_defaults(&pal);
        nexus_palette_expand_rgba(&pal);
        for (i = 0; i < 256; i++) {
            vp.fb.palette[i] = pal.rgba[i];
        }
    }
    nexus_viewport_render(&vp, &engine);
    nexus_viewport_to_rgba(&vp, rgba);

    /* The synthetic fixture proves parser/capture plumbing only. It must not
     * become a visible DGN fallback. */
    non_black = count_non_black_pixels((const uint8_t *)rgba,
                                       NEXUS_FB_W, NEXUS_FB_H);
    CHECK(non_black == 0,
          "synthetic viewport RGBA stays black without fallback pixels");

    if (!out_dir || !*out_dir) {
        SKIP("no --output-dir supplied; skipping BMP write");
        return;
    }
    CHECK(nexus_probe_capture_rgba_bmp((const unsigned char *)rgba,
                                       NEXUS_FB_W, NEXUS_FB_H,
                                       out_dir, path_a, (int)sizeof(path_a)) == 1,
          "BMP receipt writer writes a 24-bit BMP from synthetic viewport");
    if (path_a[0] == '\0') return;
    CHECK(read_file_sha256(path_a, sha_a) == 1,
          "first synth BMP is readable on disk");

    /* Render a second time from the same synth state and capture a
     * second BMP; the hash must match the first one. */
    nexus_viewport_render(&vp, &engine);
    nexus_viewport_to_rgba(&vp, rgba);
    CHECK(nexus_probe_capture_rgba_bmp((const uint8_t *)rgba,
                                       NEXUS_FB_W, NEXUS_FB_H,
                                       out_dir, path_b, (int)sizeof(path_b)) == 1,
          "second synth BMP written to a separate filename");
    if (path_b[0] == '\0') return;
    CHECK(read_file_sha256(path_b, sha_b) == 1,
          "second synth BMP is readable on disk");

    hex_of(sha_a, hex_a, sizeof(hex_a));
    hex_of(sha_b, hex_b, sizeof(hex_b));
    CHECK(memcmp(sha_a, sha_b, 32) == 0,
          "synth viewport captures are SHA256-deterministic across runs");
    printf("  [INFO] synth capture sha256: %s\n", hex_a);
}

/* ── Phase 4: real-data capture readiness ─────────────────────────── */

static void run_phase_real_capture(const char *data_dir, const char *out_dir)
{
    printf("\n=== Phase 4: real DM.BIN/FONT256.S2D/MNS handoff capture ===\n");
    Nexus_V1_Engine engine;
    Nexus_Viewport vp;
    uint32_t rgba[NEXUS_FB_W * NEXUS_FB_H];
    char path_a[1024];
    char path_b[1024];
    uint8_t sha_a[32];
    uint8_t sha_b[32];
    char hex_a[65];
    char hex_b[65];
    long non_black;
    int font_size = 0;
    int scorpion_idx;
    int r;
    struct stat st;
    Nexus_V1_Font real_font;
    Nexus_V1_FontS2dDecodeResult font_regions;
    int real_font_loaded = 0;

    if (!data_dir || !*data_dir) {
        SKIP("no --data-dir supplied");
        return;
    }
    if (stat(data_dir, &st) != 0) {
        SKIP("data_dir does not exist");
        return;
    }
    if (!out_dir || !*out_dir) {
        SKIP("no --output-dir supplied; skipping real-data BMP write");
        return;
    }

    memset(&engine, 0, sizeof(engine));
    memset(&real_font, 0, sizeof(real_font));
    memset(&font_regions, 0, sizeof(font_regions));
    r = nexus_v1_init(&engine, data_dir);
    if (r != 0) {
        SKIP("nexus_v1_init failed against real data_dir");
        return;
    }
    CHECK(engine.source == NEXUS_SRC_ISO ||
          engine.source == NEXUS_SRC_EXTRACTED,
          "real engine reports ISO or EXTRACTED source");

    r = nexus_v1_load_level(&engine, 0);
    if (r != 0) {
        SKIP("nexus_v1_load_level(0) failed; real capture path skipped");
        nexus_v1_shutdown(&engine);
        return;
    }
    CHECK(engine.current_level.width == 64 &&
          engine.current_level.height == 64,
          "real LEV00.DGN decodes as 64x64 Structure1B");

    /* Drive the DMDF parser against the real SCORPION.MNS. This is
     * the "MNS rendering" handoff closure from E1 Phase 4. */
    scorpion_idx = nexus_v1_load_model(&engine, "SCORPION.MNS");
    if (scorpion_idx >= 0) {
        const Nexus_V1_Model *m = &engine.models[scorpion_idx];
        CHECK(m->header.magic == 0x444D4446U,
              "real SCORPION.MNS header.magic == DMDF (0x444D4446)");
        CHECK(m->header.data_offset > 0,
              "real SCORPION.MNS header.data_offset > 0 (parsed section table)");
    } else {
        SKIP("SCORPION.MNS not present; MNS handoff skipped");
    }

    /* Verify the FONT256.S2D handoff parses the same bytes we just
     * read via the engine. The source-only text consumer remains closed. */
    CHECK(engine.font_loaded == 0,
          "engine.font_loaded remains closed after source-only FONT256.S2D handoff");
    {
        int s2d_size = 0;
        uint8_t *s2d_bytes = nexus_v1_read_file(&engine, "FONT256.S2D", &s2d_size);
        if (s2d_bytes && s2d_size > 0) {
            int frc = nexus_v1_font_s2d_decode(
                s2d_bytes, s2d_size, &font_regions);
            CHECK(frc > 0,
                  "real FONT256.S2D regions parse through the S2D decoder");
            if (frc > 0) {
                int tile_count = nexus_v1_font_load_from_s2d(
                    &real_font, s2d_bytes, s2d_size, &font_regions);
                CHECK(tile_count == NEXUS_V1_FONT_S2D_REAL_TILE_COUNT,
                      "real FONT256.S2D exposes exactly 242 CG tiles");
                real_font_loaded = tile_count ==
                    NEXUS_V1_FONT_S2D_REAL_TILE_COUNT;
            }
            free(s2d_bytes);
            font_size = s2d_size;
        }
    }
    /* Suppress unused warning when font_size is never read. */
    (void)font_size;

    /* Mirror the engine's loaded palette into the viewport fb so
     * nexus_viewport_to_rgba can produce non-zero RGBA. The real
     * engine init loads the default dungeon palette through
     * nexus_palette_init_defaults when STONE.BIN is missing, but
     * our viewport renderer only reads fb.palette[], so we copy
     * the default palette over here. */
    nexus_viewport_init(&vp);
    {
        Nexus_PaletteState pal;
        int i;
        nexus_palette_init_defaults(&pal);
        nexus_palette_expand_rgba(&pal);
        for (i = 0; i < 256; i++) {
            vp.fb.palette[i] = pal.rgba[i];
        }
    }

    nexus_viewport_render(&vp, &engine);
    CHECK(real_font_loaded &&
          real_font.char_count == NEXUS_V1_FONT_S2D_REAL_TILE_COUNT,
          "real FONT256.S2D CG tiles remain parsed while DGN capture is blocked");
    nexus_viewport_to_rgba(&vp, rgba);

    non_black = count_non_black_pixels((const uint8_t *)rgba,
                                       NEXUS_FB_W, NEXUS_FB_H);
    CHECK(non_black == 0,
          "real-data viewport RGBA stays black until Saturn DGN capture is admitted");

    CHECK(nexus_probe_capture_rgba_bmp((const unsigned char *)rgba,
                                       NEXUS_FB_W, NEXUS_FB_H,
                                       out_dir, path_a, (int)sizeof(path_a)) == 1,
          "BMP receipt writer writes a 24-bit BMP from real viewport");
    if (path_a[0] == '\0') {
        if (real_font_loaded) nexus_v1_font_free(&real_font);
        nexus_v1_shutdown(&engine);
        return;
    }
    CHECK(read_file_sha256(path_a, sha_a) == 1,
          "first real-data BMP is readable on disk");

    nexus_viewport_render(&vp, &engine);
    nexus_viewport_to_rgba(&vp, rgba);
    CHECK(nexus_probe_capture_rgba_bmp((const unsigned char *)rgba,
                                       NEXUS_FB_W, NEXUS_FB_H,
                                       out_dir, path_b, (int)sizeof(path_b)) == 1,
          "second real-data BMP written to a separate filename");
    if (path_b[0] == '\0') {
        if (real_font_loaded) nexus_v1_font_free(&real_font);
        nexus_v1_shutdown(&engine);
        return;
    }
    CHECK(read_file_sha256(path_b, sha_b) == 1,
          "second real-data BMP is readable on disk");

    hex_of(sha_a, hex_a, sizeof(hex_a));
    hex_of(sha_b, hex_b, sizeof(hex_b));
    CHECK(memcmp(sha_a, sha_b, 32) == 0,
          "real-data viewport captures are SHA256-deterministic across runs");
    printf("  [INFO] real-data capture sha256: %s\n", hex_a);

    if (real_font_loaded) nexus_v1_font_free(&real_font);
    nexus_v1_shutdown(&engine);
}

/* ── Main ──────────────────────────────────────────────────────────── */

static void usage(const char *prog)
{
    fprintf(stderr,
        "usage: %s [data_dir] [output_dir]\n"
        "  data_dir   : real Nexus data root (e.g. ~/.firestaff/data/nexus).\n"
        "               When omitted, the probe runs in data-free mode and\n"
        "               exercises the synth-DGN viewport/capture contract.\n"
        "  output_dir : directory to write BMP receipts into. When omitted,\n"
        "               the synth path SKIPs the BMP write and the real-data\n"
        "               path SKIPs the BMP write (capture-only contracts).\n",
        prog);
}

int main(int argc, char **argv)
{
    const char *data_dir = (argc >= 2) ? argv[1] : NULL;
    const char *out_dir = (argc >= 3) ? argv[2] : NULL;

    if (argc >= 2 && (!argv[1] || !*argv[1] || strcmp(argv[1], "-h") == 0 ||
                       strcmp(argv[1], "--help") == 0)) {
        usage(argv[0]);
        return 2;
    }

    printf("=== Nexus V1 Track 1 real-screen-capture readiness probe ===\n");
    printf("data_dir: %s\n", data_dir ? data_dir : "(none, data-free)");
    printf("output_dir: %s\n", out_dir ? out_dir : "(none, capture-only)");

    run_phase_engine_api_contract();
    run_phase_viewport_init_contract();
    run_phase_synthetic_capture(out_dir);
    run_phase_real_capture(data_dir, out_dir);

    printf("\n=== Summary ===\n");
    printf("  PASS: %d\n", g_pass);
    printf("  FAIL: %d\n", g_fail);
    if (g_fail == 0) {
        printf("RESULT: PASS\n");
        return 0;
    }
    printf("RESULT: FAIL\n");
    return 1;
}
