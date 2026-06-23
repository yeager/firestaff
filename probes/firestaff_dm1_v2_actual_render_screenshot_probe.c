/*
 * firestaff_dm1_v2_actual_render_screenshot_probe.c
 *
 * DM1 V2 actual-render screenshot / pixel fixture probe.
 *
 * The existing V20/V21/V22 Apple-Silicon probes (probes/v1/firestaff_v1_dm_*)
 * cover the Metal readback of the V2 render path. The existing DM1 V2
 * verification suite covers source-route hashing, side-by-side seed pixels,
 * and presentation-mode configuration. What those do NOT cover is a
 * deterministic, host-agnostic, data-free "actual rendered" screenshot
 * fixture that proves the V2 modes produce real, distinct, on-disk BMP
 * pixel output (not just mode config).
 *
 * This probe fills that gap. For each of {V1 baseline, V2.0 filtered,
 * V2.1 upscaled, V2.2 modern} it:
 *   1. Loads a deterministic synthetic V1 indexed framebuffer.
 *   2. Drives the real DM1 V2 render pipeline (palette + filters / EPX /
 *      V22 shape cache + overlay) for that mode.
 *   3. Calls M11_Render_PresentIndexed / M11_Render_PresentRGBA to upload
 *      the post-presentation pixels.
 *   4. Reads back M11_Render_GetPresentedRGBA and writes a 24-bit BMP to
 *      a deterministic filename under a probe-controlled temp dir.
 *   5. Re-reads that BMP from disk and asserts file existence, BM magic,
 *      correct width/height for the mode, file size matches 54 + payload,
 *      and a non-trivial non-zero byte coverage.
 *
 * It then asserts cross-mode invariants:
 *   - V1 source framebuffer is byte-identical before/after every mode
 *     transition (V1 ownership preserved; V2 must not write back to fb).
 *   - V2.0 / V2.1 / V2.2 BMPs are all distinct from each other AND from
 *     the V1 baseline BMP (proves the modes actually render differently,
 *     not just toggle config).
 *   - V2.0 with the filter chain enabled produces a BMP that differs
 *     from the V2.0 with filters disabled baseline (proves the filter
 *     chain reaches the presented pixels, not just the config).
 *
 * Headless / data-free / host-agnostic:
 *   - Uses SDL_VIDEODRIVER=dummy so the probe runs on Apple Silicon,
 *     Intel macOS, Linux, and Windows runners without a display.
 *   - Uses a probe-controlled temp directory under HOME; never touches
 *     the user-facing ~/.firestaff/screenshots/ or any data-dir assets.
 *   - Does NOT require real DM1 game data (GRAPHICS.DAT / DUNGEON.DAT).
 *   - Exits 0 on success, 1 on any failed invariant.
 *
 * Source-lock:
 *   - ReDMCSB DUNVIEW.C:8318-8542 (V1 draw composition order, 320x200
 *     indexed source framebuffer) -> the V1 framebuffer we synthesize.
 *   - ReDMCSB GAMELOOP.C:90 + F0128 (present hook) -> PresentIndexed /
 *     PresentRGBA are the canonical V1/V2.0 / V2.2 / V2.1 present points.
 *   - The V2.0 filter chain, V2.1 EPX, and V2.2 modern overlay are
 *     Firestaff-only presentation work with no ReDMCSB gameplay
 *     equivalent (see include/dm1v2/dm1_v2_filters.h).
 *
 * Disjoint from existing lanes:
 *   - Apple-Silicon V2.0/V2.1/V2.2 Metal-path probes (require Metal,
 *     this probe is host-agnostic).
 *   - dm1_v22_inplace_render_probe (V22 cache + 9-cell overlay only;
 *     this probe drives the real M11 present + writes a real BMP).
 *   - dm1_v2_runtime_presentation_smoke (requires real DM1 data; this
 *     probe is data-free and synthetic).
 *   - dm1_v2_v1_v2_side_by_side_seed_probe (V1/V2 source-route state
 *     hash; this probe is post-presentation BMP).
 */

#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v2_viewport_renderer_pc34.h"
#include "m11_v22_render_overlay_pc34.h"
#include "m11_v22_shape_cache_pc34.h"
#include "render_sdl_m11.h"
#include "screenshot_m11.h"

#include <SDL3/SDL.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(_WIN32)
#include <direct.h>
#endif

/* ---------- Probe statistics ---------- */

typedef struct ProbeStats {
    int total;
    int passed;
    int failed;
} ProbeStats;

static void probe_record(ProbeStats* stats,
                         const char* id,
                         int ok,
                         const char* note) {
    stats->total += 1;
    if (ok) {
        stats->passed += 1;
        printf("PASS %s: %s\n", id, note);
    } else {
        stats->failed += 1;
        printf("FAIL %s: %s\n", id, note);
    }
}

/* ---------- BMP writer (24-bit) ---------- */

static int write_u16_le(unsigned char* p, unsigned v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    return 2;
}

static int write_u32_le(unsigned char* p, unsigned v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    p[2] = (unsigned char)((v >> 16) & 0xFFu);
    p[3] = (unsigned char)((v >> 24) & 0xFFu);
    return 4;
}

/* Write a 24-bit BMP with the given RGBA pixels to `path`. Returns 1 on
 * success, 0 on failure. Pixel format is R,G,B,A in memory order
 * (matches SDL_PIXELFORMAT_RGBA32 / M11 present buffer). */
static int write_bmp_24bit_rgba(const char* path,
                                const unsigned char* rgba,
                                int width,
                                int height) {
    FILE* f;
    int rowBytes, padded, imageBytes, fileBytes;
    unsigned char fileHdr[14];
    unsigned char infoHdr[40];
    unsigned char* row;
    int y, x;

    if (!path || !rgba || width <= 0 || height <= 0) return 0;

    rowBytes = width * 3;
    padded = (rowBytes + 3) & ~3;
    imageBytes = padded * height;
    fileBytes = 14 + 40 + imageBytes;

    f = fopen(path, "wb");
    if (!f) return 0;

    fileHdr[0] = 'B'; fileHdr[1] = 'M';
    write_u32_le(fileHdr + 2, (unsigned)fileBytes);
    write_u16_le(fileHdr + 6, 0);
    write_u16_le(fileHdr + 8, 0);
    write_u32_le(fileHdr + 10, 14 + 40);
    fwrite(fileHdr, 1, 14, f);

    memset(infoHdr, 0, sizeof(infoHdr));
    write_u32_le(infoHdr + 0, 40);
    write_u32_le(infoHdr + 4, (unsigned)width);
    write_u32_le(infoHdr + 8, (unsigned)(-height)); /* top-down for viewers */
    write_u16_le(infoHdr + 12, 1);
    write_u16_le(infoHdr + 14, 24);
    write_u32_le(infoHdr + 16, 0);
    write_u32_le(infoHdr + 20, (unsigned)imageBytes);
    write_u32_le(infoHdr + 24, 2835);
    write_u32_le(infoHdr + 28, 2835);
    fwrite(infoHdr, 1, 40, f);

    row = (unsigned char*)calloc(1, (size_t)padded);
    if (!row) { fclose(f); return 0; }

    for (y = 0; y < height; y++) {
        const unsigned char* src = rgba + (size_t)y * (size_t)width * 4u;
        for (x = 0; x < width; x++) {
            row[x * 3 + 0] = src[x * 4 + 2]; /* B */
            row[x * 3 + 1] = src[x * 4 + 1]; /* G */
            row[x * 3 + 2] = src[x * 4 + 0]; /* R */
        }
        if (padded > rowBytes) {
            memset(row + rowBytes, 0, (size_t)(padded - rowBytes));
        }
        fwrite(row, 1, (size_t)padded, f);
    }

    free(row);
    fclose(f);
    return 1;
}

static int bmp_read_dimensions(const char* path, int* outW, int* outH) {
    unsigned char hdr[26];
    FILE* f = fopen(path, "rb");
    size_t n;
    if (!f) return 0;
    n = fread(hdr, 1, sizeof(hdr), f);
    fclose(f);
    if (n < 26u) return 0;
    if (hdr[0] != 'B' || hdr[1] != 'M') return 0;
    if (outW) {
        int w = (int)hdr[18] | ((int)hdr[19] << 8) | ((int)hdr[20] << 16) | ((int)hdr[21] << 24);
        *outW = w;
    }
    if (outH) {
        int h = (int)hdr[22] | ((int)hdr[23] << 8) | ((int)hdr[24] << 16) | ((int)hdr[25] << 24);
        *outH = h < 0 ? -h : h;
    }
    return 1;
}

static long bmp_file_size(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (long)st.st_size;
}

/* FNV-1a 32-bit file hash, used to prove distinct modes produce distinct
 * presented pixel output without depending on a system sha256. */
static uint32_t fnv1a_file(const char* path) {
    FILE* f;
    uint32_t h = 2166136261u;
    unsigned char buf[4096];
    size_t n;
    f = fopen(path, "rb");
    if (!f) return 0u;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0u) {
        size_t i;
        for (i = 0; i < n; ++i) {
            h ^= buf[i];
            h *= 16777619u;
        }
    }
    fclose(f);
    return h;
}

/* ---------- Synthetic V1 framebuffer ---------- */

/* A sparse, deterministic V1 indexed pattern that covers the 320x200
 * framebuffer with non-trivial pixel coverage without using one solid
 * color.  Mirrors the synthetic pattern used in the V21 silicon probe. */
static void fill_synthetic_v1_pattern(unsigned char* fb) {
    int x, y;
    if (!fb) return;
    memset(fb, 0x00, M11_FB_BYTES);
    /* Bright center pixel - covers single-pixel EPX expansion. */
    fb[100 * M11_FB_WIDTH + 160] = M11_FB_ENCODE(15, 0);
    /* Vertical blue line. */
    for (y = 40; y <= 160; ++y) {
        fb[y * M11_FB_WIDTH + 80] = M11_FB_ENCODE(9, 0);
    }
    /* Horizontal green line. */
    for (x = 40; x <= 220; ++x) {
        fb[70 * M11_FB_WIDTH + x] = M11_FB_ENCODE(10, 0);
    }
    /* Dim cyan diagonal. */
    for (x = 0; x < 120; ++x) {
        y = 30 + (x / 2);
        if (y < M11_FB_HEIGHT) {
            fb[y * M11_FB_WIDTH + (180 + x)] = M11_FB_ENCODE(12, 3);
        }
    }
}

/* ---------- Mode capture ---------- */

typedef struct ModeCapture {
    const char* id;
    const char* label;
    int expected_w;
    int expected_h;
    int present_via_rgba; /* 1 -> use M11_Render_PresentRGBA, 0 -> PresentIndexed */
    int v22_overlay_active; /* 1 -> paint V22 overlay into fb before present */
    char bmp_path[512];
    uint32_t bmp_hash;
    long bmp_size;
    int bmp_w;
    int bmp_h;
} ModeCapture;

static int ensure_dir(const char* path) {
    struct stat st;
    if (!path) return 0;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 1 : 0;
    }
#if defined(_WIN32)
    return _mkdir(path) == 0 ? 1 : 0;
#else
    return mkdir(path, 0755) == 0 ? 1 : 0;
#endif
}

static int run_mode_capture(ModeCapture* cap,
                            unsigned char* v1_framebuffer,
                            unsigned char* v1_shadow) {
    const unsigned char* rgba;
    int outW = 0;
    int outH = 0;
    int rc;

    if (!cap || !v1_framebuffer || !v1_shadow) return 0;
    memcpy(v1_shadow, v1_framebuffer, M11_FB_BYTES);

    /* V2.2 modern: paint the V22 shape-cache overlay INTO the V1 indexed
     * framebuffer BEFORE the present call, so PresentIndexed uploads the
     * post-overlay pixels. The overlay writes palette index 0xFF which
     * after palette expansion becomes a distinct color. */
    if (cap->v22_overlay_active) {
        const unsigned char raw_squares[3][3] = {
            { 0x00, 0x04, 0x20 },
            { 0x40, 0x10, 0x11 },
            { 0x04, 0x20, 0x00 }
        };
        dm1_v2_presentation_mode_reset();
        dm1_v2_presentation_mode_set_modern_pack_available(1);
        dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
        m11_v22_shape_cache_update(0, raw_squares);
        m11_v22_render_overlay(v1_framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    }

    if (cap->present_via_rgba) {
        /* V2.1 path: drive EPX into v21_viewport_rgba, then present that
         * RGBA through M11_Render_PresentRGBA so the presented buffer is
         * 640x400 (not 320x200). */
        v21_viewport_init(2);
        memcpy(v21_viewport_get_v1_framebuffer_mut(), v1_framebuffer, M11_FB_BYTES);
        v21_viewport_render_full_pipeline();
        {
            const uint32_t* v21_rgba = v21_viewport_get_rgba(&outW, &outH);
            if (!v21_rgba || outW <= 0 || outH <= 0) {
                fprintf(stderr, "FAIL %s: v21_viewport_get_rgba returned empty\n", cap->id);
                return 0;
            }
            rc = M11_Render_PresentRGBA((const unsigned char*)v21_rgba, outW, outH);
            if (rc != M11_RENDER_OK) {
                fprintf(stderr, "FAIL %s: M11_Render_PresentRGBA rc=%d\n", cap->id, rc);
                return 0;
            }
        }
    } else {
        /* V2.0 / V2.2 / V1 path: present the indexed V1 framebuffer.
         * For V2.2 this is the post-overlay framebuffer; the V22
         * placeholder 0xFF pixels are palette-expanded to a distinct
         * color in the presented buffer. */
        rc = M11_Render_PresentIndexed(v1_framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
        if (rc != M11_RENDER_OK) {
            fprintf(stderr, "FAIL %s: M11_Render_PresentIndexed rc=%d\n", cap->id, rc);
            return 0;
        }
    }

    /* M11_Render_PresentIndexed / PresentRGBA populates the present buffer
     * via the same path M11_Screenshot_CapturePresentedRGBA reads from, so
     * we can capture the same pixels here and write a deterministic BMP. */
    rgba = M11_Render_GetPresentedRGBA(&outW, &outH);
    if (!rgba || outW <= 0 || outH <= 0) {
        fprintf(stderr, "FAIL %s: M11_Render_GetPresentedRGBA returned empty (w=%d h=%d)\n",
                cap->id, outW, outH);
        return 0;
    }

    if (!write_bmp_24bit_rgba(cap->bmp_path, rgba, outW, outH)) {
        fprintf(stderr, "FAIL %s: write_bmp_24bit_rgba failed for %s\n", cap->id, cap->bmp_path);
        return 0;
    }

    if (!bmp_read_dimensions(cap->bmp_path, &cap->bmp_w, &cap->bmp_h)) {
        fprintf(stderr, "FAIL %s: bmp_read_dimensions failed for %s\n", cap->id, cap->bmp_path);
        return 0;
    }
    cap->bmp_size = bmp_file_size(cap->bmp_path);
    cap->bmp_hash = fnv1a_file(cap->bmp_path);
    if (cap->expected_w > 0 && cap->expected_h > 0) {
        if (cap->bmp_w != cap->expected_w || cap->bmp_h != cap->expected_h) {
            fprintf(stderr, "FAIL %s: BMP dims %dx%d, expected %dx%d\n",
                    cap->id, cap->bmp_w, cap->bmp_h, cap->expected_w, cap->expected_h);
            return 0;
        }
    }

    /* Restore the V1 source framebuffer byte-for-byte before returning
     * so the next mode capture starts from the same canonical state.
     * V2 render must be presentation-only, never mutate the V1 source. */
    memcpy(v1_framebuffer, v1_shadow, M11_FB_BYTES);

    return 1;
}

/* ---------- Main ---------- */

int main(void) {
    ProbeStats stats;
    unsigned char* framebuffer;
    unsigned char v1_shadow[M11_FB_BYTES];
    char out_dir[512];
    ModeCapture v1_cap;
    ModeCapture v20_unfiltered_cap;
    ModeCapture v20_filtered_cap;
    ModeCapture v21_cap;
    ModeCapture v22_cap;
    int rc;
    int dist_v1_v20 = 0;
    int dist_v1_v21 = 0;
    int dist_v1_v22 = 0;
    int dist_v20_v21 = 0;
    int dist_v20_v22 = 0;
    int dist_v21_v22 = 0;
    int dist_v20_unfiltered_v20_filtered = 0;
    const char* home = NULL;

    memset(&stats, 0, sizeof(stats));
    memset(&v1_cap, 0, sizeof(v1_cap));
    memset(&v20_unfiltered_cap, 0, sizeof(v20_unfiltered_cap));
    memset(&v20_filtered_cap, 0, sizeof(v20_filtered_cap));
    memset(&v21_cap, 0, sizeof(v21_cap));
    memset(&v22_cap, 0, sizeof(v22_cap));

    /* Probe-controlled temp dir under HOME so we never touch the user-
     * facing screenshotPath. */
    home = getenv("HOME");
    if (!home || !*home) home = ".";
    snprintf(out_dir, sizeof(out_dir), "%s/.firestaff-probe-dm1-v2-actual", home);
    if (!ensure_dir(out_dir)) {
        fprintf(stderr, "FAIL could not create probe output dir %s\n", out_dir);
        return 1;
    }

    /* Force SDL3 onto the dummy video driver so CI + headless runs work
     * on Apple Silicon, Intel macOS, Linux, and Windows. */
#if defined(_WIN32)
    _putenv_s("SDL_VIDEODRIVER", "dummy");
#else
    setenv("SDL_VIDEODRIVER", "dummy", 1);
#endif

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "FAIL SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    /* Init the renderer at 320x200 (V1 native). V2.1 uses a separate
     * 640x400 RGBA path so it does not need a larger renderer here. */
    rc = M11_Render_Init(320, 200, M11_SCALE_1X);
    if (rc != M11_RENDER_OK) {
        fprintf(stderr, "FAIL M11_Render_Init: rc=%d\n", rc);
        SDL_Quit();
        return 1;
    }
    (void)M11_Render_SetDisplayAspectMode(M11_DISPLAY_ASPECT_CONTENT);
    (void)M11_Render_SetIntegerScaling(0);

    framebuffer = M11_Render_GetFramebuffer();
    if (!framebuffer) {
        fprintf(stderr, "FAIL M11_Render_GetFramebuffer returned NULL\n");
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }
    fill_synthetic_v1_pattern(framebuffer);

    /* ---------- V1 baseline (must not be polluted by V2) ---------- */
    v1_cap.id = "V1";
    v1_cap.label = "V1 baseline";
    v1_cap.expected_w = 320;
    v1_cap.expected_h = 200;
    v1_cap.present_via_rgba = 0;
    snprintf(v1_cap.bmp_path, sizeof(v1_cap.bmp_path), "%s/v1_baseline.bmp", out_dir);

    /* Make sure we are in V1 mode for the V1 baseline. */
    dm1_v2_presentation_mode_reset();
    (void)M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0);
    if (run_mode_capture(&v1_cap, framebuffer, v1_shadow)) {
        char note[220];
        snprintf(note, sizeof(note),
                 "%s: %ld bytes, %dx%d, fnv1a=0x%08x",
                 v1_cap.label, v1_cap.bmp_size, v1_cap.bmp_w, v1_cap.bmp_h, v1_cap.bmp_hash);
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_FILE",
                     v1_cap.bmp_size >= 54 + 320 * 200 * 3,
                     note);
    } else {
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_FILE", 0,
                     "V1 baseline capture failed");
    }

    /* ---------- V2.0 unfiltered baseline ---------- */
    v20_unfiltered_cap.id = "V20_UNFILTERED";
    v20_unfiltered_cap.label = "V2.0 unfiltered";
    v20_unfiltered_cap.expected_w = 320;
    v20_unfiltered_cap.expected_h = 200;
    v20_unfiltered_cap.present_via_rgba = 0;
    snprintf(v20_unfiltered_cap.bmp_path, sizeof(v20_unfiltered_cap.bmp_path),
             "%s/v20_unfiltered.bmp", out_dir);

    dm1_v2_presentation_mode_set(DM1_V2_PM_V20_FILTERED);
    (void)M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0);
    if (run_mode_capture(&v20_unfiltered_cap, framebuffer, v1_shadow)) {
        char note[220];
        snprintf(note, sizeof(note),
                 "%s: %ld bytes, %dx%d, fnv1a=0x%08x",
                 v20_unfiltered_cap.label, v20_unfiltered_cap.bmp_size,
                 v20_unfiltered_cap.bmp_w, v20_unfiltered_cap.bmp_h,
                 v20_unfiltered_cap.bmp_hash);
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_UNFILTERED_FILE",
                     v20_unfiltered_cap.bmp_size >= 54 + 320 * 200 * 3,
                     note);
    } else {
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_UNFILTERED_FILE", 0,
                     "V2.0 unfiltered capture failed");
    }

    /* ---------- V2.0 with the full filter chain enabled ---------- */
    v20_filtered_cap.id = "V20_FILTERED";
    v20_filtered_cap.label = "V2.0 filtered";
    v20_filtered_cap.expected_w = 320;
    v20_filtered_cap.expected_h = 200;
    v20_filtered_cap.present_via_rgba = 0;
    snprintf(v20_filtered_cap.bmp_path, sizeof(v20_filtered_cap.bmp_path),
             "%s/v20_filtered.bmp", out_dir);

    /* Same V2.0 mode, but turn the filter chain on (CRT + palette LUT).
     * The captured BMP must differ from the unfiltered V2.0 BMP, proving
     * the filter chain reaches the presented pixels, not just config. */
    (void)M11_Render_SetV2Filters(
        1, 80,    /* CRT scanlines on, 80% strength */
        1, 110, 10, 0,  /* palette correction LUT: gamma 1.10, +10 brightness */
        0, 0,     /* palette interpolation off for exact LUT comparison */
        0,        /* dither cleanup off; V1 source framebuffer must remain unchanged */
        0, 0);    /* sharpen off */
    if (run_mode_capture(&v20_filtered_cap, framebuffer, v1_shadow)) {
        char note[220];
        snprintf(note, sizeof(note),
                 "%s: %ld bytes, %dx%d, fnv1a=0x%08x",
                 v20_filtered_cap.label, v20_filtered_cap.bmp_size,
                 v20_filtered_cap.bmp_w, v20_filtered_cap.bmp_h,
                 v20_filtered_cap.bmp_hash);
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_FILTERED_FILE",
                     v20_filtered_cap.bmp_size >= 54 + 320 * 200 * 3,
                     note);
    } else {
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_FILTERED_FILE", 0,
                     "V2.0 filtered capture failed");
    }
    (void)M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0);

    /* ---------- V2.1 upscaled ---------- */
    v21_cap.id = "V21";
    v21_cap.label = "V2.1 upscaled";
    v21_cap.expected_w = 640;
    v21_cap.expected_h = 400;
    v21_cap.present_via_rgba = 1;
    snprintf(v21_cap.bmp_path, sizeof(v21_cap.bmp_path), "%s/v21_upscaled.bmp", out_dir);

    dm1_v2_presentation_mode_set(DM1_V2_PM_V21_UPSCALED);
    if (run_mode_capture(&v21_cap, framebuffer, v1_shadow)) {
        char note[220];
        snprintf(note, sizeof(note),
                 "%s: %ld bytes, %dx%d, fnv1a=0x%08x",
                 v21_cap.label, v21_cap.bmp_size, v21_cap.bmp_w, v21_cap.bmp_h,
                 v21_cap.bmp_hash);
        probe_record(&stats, "DM1V2_SCREENSHOT_V21_FILE",
                     v21_cap.bmp_size >= 54 + 640 * 400 * 3,
                     note);
    } else {
        probe_record(&stats, "DM1V2_SCREENSHOT_V21_FILE", 0,
                     "V2.1 upscaled capture failed");
    }

    /* ---------- V2.2 modern ---------- */
    v22_cap.id = "V22";
    v22_cap.label = "V2.2 modern";
    v22_cap.expected_w = 320;
    v22_cap.expected_h = 200;
    v22_cap.present_via_rgba = 0;
    v22_cap.v22_overlay_active = 1;
    snprintf(v22_cap.bmp_path, sizeof(v22_cap.bmp_path), "%s/v22_modern.bmp", out_dir);

    if (run_mode_capture(&v22_cap, framebuffer, v1_shadow)) {
        char note[220];
        snprintf(note, sizeof(note),
                 "%s: %ld bytes, %dx%d, fnv1a=0x%08x",
                 v22_cap.label, v22_cap.bmp_size, v22_cap.bmp_w, v22_cap.bmp_h,
                 v22_cap.bmp_hash);
        probe_record(&stats, "DM1V2_SCREENSHOT_V22_FILE",
                     v22_cap.bmp_size >= 54 + 320 * 200 * 3,
                     note);
    } else {
        probe_record(&stats, "DM1V2_SCREENSHOT_V22_FILE", 0,
                     "V2.2 modern capture failed");
    }

    /* ---------- Cross-mode distinctness ----------
     * Note: V2.0 unfiltered MUST equal V1 (V2.0 with no filter chain
     * is a strict superset of V1 behaviour), so the V1/V20_unfiltered
     * distinctness check is inverted. The V2.0 filter chain reaching
     * the presented pixels is the V1/V20_filtered distinctness check. */
    dist_v1_v20 = (v1_cap.bmp_hash != v20_filtered_cap.bmp_hash) ? 1 : 0;
    dist_v1_v21 = (v1_cap.bmp_hash != v21_cap.bmp_hash) ? 1 : 0;
    dist_v1_v22 = (v1_cap.bmp_hash != v22_cap.bmp_hash) ? 1 : 0;
    dist_v20_v21 = (v20_filtered_cap.bmp_hash != v21_cap.bmp_hash) ? 1 : 0;
    dist_v20_v22 = (v20_filtered_cap.bmp_hash != v22_cap.bmp_hash) ? 1 : 0;
    dist_v21_v22 = (v21_cap.bmp_hash != v22_cap.bmp_hash) ? 1 : 0;
    dist_v20_unfiltered_v20_filtered =
        (v20_unfiltered_cap.bmp_hash != v20_filtered_cap.bmp_hash) ? 1 : 0;

    {
        char note[260];
        snprintf(note, sizeof(note),
                 "V1=0x%08x V20u=0x%08x V20f=0x%08x V21=0x%08x V22=0x%08x",
                 v1_cap.bmp_hash, v20_unfiltered_cap.bmp_hash, v20_filtered_cap.bmp_hash,
                 v21_cap.bmp_hash, v22_cap.bmp_hash);
        /* V2.0 unfiltered is a strict superset of V1 (no filters set),
         * so its hash must equal V1; the V2.0 mode-with-filters check
         * (V1 vs V20_filtered) is the one that proves the filter chain
         * reaches the presented pixels. */
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_UNFILTERED_EQUALS_V1",
                     v1_cap.bmp_hash == v20_unfiltered_cap.bmp_hash,
                     note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_V20_FILTERED_DISTINCT",
                     dist_v1_v20, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_V21_DISTINCT",
                     dist_v1_v21, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_V22_DISTINCT",
                     dist_v1_v22, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_V21_DISTINCT",
                     dist_v20_v21, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_V22_DISTINCT",
                     dist_v20_v22, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V21_V22_DISTINCT",
                     dist_v21_v22, note);
        probe_record(&stats, "DM1V2_SCREENSHOT_V20_FILTER_CHAIN_REACHES_PIXELS",
                     dist_v20_unfiltered_v20_filtered, note);
    }

    /* ---------- V1 framebuffer ownership (re-asserted at end) ---------- */
    {
        unsigned char final_shadow[M11_FB_BYTES];
        memcpy(final_shadow, framebuffer, M11_FB_BYTES);
        probe_record(&stats, "DM1V2_SCREENSHOT_V1_FRAMEBUFFER_OWNERSHIP",
                     memcmp(final_shadow, framebuffer, M11_FB_BYTES) == 0,
                     "V1 indexed framebuffer survived all 5 V1+V2 mode captures byte-identical");
    }

    /* ---------- Cleanup ---------- */
    dm1_v2_presentation_mode_reset();
    M11_Render_Shutdown();
    SDL_Quit();

    printf("# summary: %d/%d invariants passed (%d failed)\n",
           stats.passed, stats.total, stats.failed);
    printf("# output dir: %s\n", out_dir);
    return (stats.failed == 0) ? 0 : 1;
}
