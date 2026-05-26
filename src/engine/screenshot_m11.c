/*
 * screenshot_m11.c — BMP screenshot capture.
 *
 * Writes a top-down 24-bit BMP from an indexed framebuffer with the
 * supplied 256-entry palette.  No external image libraries needed —
 * BITMAPFILEHEADER (14) + BITMAPINFOHEADER (40) + padded pixel rows.
 *
 * Also owns the "screenshot mode" toggle used by the engine to hide
 * the HUD and pause the game while composing a shot.
 */

#include "screenshot_m11.h"
#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <unistd.h>
#define MKDIR(p) mkdir((p), 0755)
#endif

/* ── Screenshot-mode state ──────────────────────────────────────────── */

static int s_modeActive = 0;

void M11_Screenshot_ToggleMode(void) { s_modeActive = !s_modeActive; }
int  M11_Screenshot_IsModeActive(void) { return s_modeActive; }
void M11_Screenshot_ExitMode(void)   { s_modeActive = 0; }

/* ── Helpers ────────────────────────────────────────────────────────── */

static void write_u16_le(unsigned char* dst, unsigned int v) {
    dst[0] = (unsigned char)(v & 0xFF);
    dst[1] = (unsigned char)((v >> 8) & 0xFF);
}

static void write_u32_le(unsigned char* dst, unsigned int v) {
    dst[0] = (unsigned char)(v & 0xFF);
    dst[1] = (unsigned char)((v >> 8) & 0xFF);
    dst[2] = (unsigned char)((v >> 16) & 0xFF);
    dst[3] = (unsigned char)((v >> 24) & 0xFF);
}

static void ensure_dir(const char* dir) {
    if (!dir || !*dir) return;
    (void)MKDIR(dir);
}

const char* M11_Screenshot_DefaultDir(void) {
    static char path[1024];
    const char* home = getenv("HOME");
    char base[1024];
    if (!home || !*home) home = ".";
    snprintf(base, sizeof(base), "%s/.firestaff", home);
    ensure_dir(base);
    snprintf(path, sizeof(path), "%s/.firestaff/screenshots", home);
    ensure_dir(path);
    return path;
}

/* ── BMP writer ─────────────────────────────────────────────────────── */

int M11_Screenshot_Capture(const unsigned char* framebuffer,
                           int width, int height,
                           const unsigned char* palette,
                           const char* outputDir,
                           char* outPath, int outPathCap) {
    FILE* f;
    int rowBytes, padded, imageBytes, fileBytes;
    unsigned char fileHdr[14];
    unsigned char infoHdr[40];
    unsigned char* row;
    char dirBuf[1024];
    char path[1280];
    time_t now;
    struct tm* lt;
    char stamp[64];
    int y, x;
    static unsigned char grayscale[256 * 3];
    static int grayscaleInit = 0;
    const unsigned char* pal = palette;

    if (!framebuffer || width <= 0 || height <= 0) return 0;

    if (outputDir && *outputDir) {
        snprintf(dirBuf, sizeof(dirBuf), "%s", outputDir);
        ensure_dir(dirBuf);
    } else {
        snprintf(dirBuf, sizeof(dirBuf), "%s", M11_Screenshot_DefaultDir());
    }

    now = time(NULL);
    lt = localtime(&now);
    if (lt) {
        strftime(stamp, sizeof(stamp), "%Y%m%d-%H%M%S", lt);
    } else {
        snprintf(stamp, sizeof(stamp), "unknown");
    }
    snprintf(path, sizeof(path), "%s/firestaff-%s.bmp", dirBuf, stamp);

    if (!pal) {
        if (!grayscaleInit) {
            int i;
            for (i = 0; i < 256; i++) {
                grayscale[i * 3 + 0] = (unsigned char)i;
                grayscale[i * 3 + 1] = (unsigned char)i;
                grayscale[i * 3 + 2] = (unsigned char)i;
            }
            grayscaleInit = 1;
        }
        pal = grayscale;
    }

    rowBytes   = width * 3;
    padded     = (rowBytes + 3) & ~3;
    imageBytes = padded * height;
    fileBytes  = 14 + 40 + imageBytes;

    f = fopen(path, "wb");
    if (!f) return 0;

    fileHdr[0] = 'B'; fileHdr[1] = 'M';
    write_u32_le(fileHdr + 2, (unsigned)fileBytes);
    write_u16_le(fileHdr + 6, 0);
    write_u16_le(fileHdr + 8, 0);
    write_u32_le(fileHdr + 10, 14 + 40);
    fwrite(fileHdr, 1, 14, f);

    memset(infoHdr, 0, sizeof(infoHdr));
    write_u32_le(infoHdr + 0,  40);
    write_u32_le(infoHdr + 4,  (unsigned)width);
    write_u32_le(infoHdr + 8,  (unsigned)(-height));
    write_u16_le(infoHdr + 12, 1);
    write_u16_le(infoHdr + 14, 24);
    write_u32_le(infoHdr + 16, 0);
    write_u32_le(infoHdr + 20, (unsigned)imageBytes);
    write_u32_le(infoHdr + 24, 2835);
    write_u32_le(infoHdr + 28, 2835);
    fwrite(infoHdr, 1, 40, f);

    row = (unsigned char*)calloc(1, (size_t)padded);
    if (!row) {
        fclose(f);
        return 0;
    }

    for (y = 0; y < height; y++) {
        const unsigned char* src = framebuffer + (size_t)y * (size_t)width;
        for (x = 0; x < width; x++) {
            unsigned char idx = src[x];
            const unsigned char* rgb = &pal[(int)idx * 3];
            row[x * 3 + 0] = rgb[2];   /* B */
            row[x * 3 + 1] = rgb[1];   /* G */
            row[x * 3 + 2] = rgb[0];   /* R */
        }
        if (padded > rowBytes) {
            memset(row + rowBytes, 0, (size_t)(padded - rowBytes));
        }
        fwrite(row, 1, (size_t)padded, f);
    }

    free(row);
    fclose(f);

    if (outPath && outPathCap > 0) {
        snprintf(outPath, (size_t)outPathCap, "%s", path);
    }
    return 1;
}

/* Capture the m11 indexed framebuffer with the current 16-entry VGA
 * palette expanded into a 256-entry RGB table (high indices repeat
 * the active row to remain in-range with the BMP writer). */
int M11_Screenshot_CaptureCurrent(const char* outputDir,
                                  char* outPath, int outPathCap) {
    unsigned char* fb;
    static unsigned char palette[256 * 3];
    static unsigned char masked[M11_FB_BYTES];
    int level, i;
    int n = M11_FB_BYTES;

    fb = M11_Render_GetFramebuffer();
    if (!fb) return 0;

    level = M11_Render_GetPaletteLevel();
    if (level < 0) level = 0;
    if (level >= M11_PALETTE_LEVELS) level = M11_PALETTE_LEVELS - 1;

    /* Drop level bits so the index fits the 16-entry VGA palette. */
    for (i = 0; i < n; i++) {
        masked[i] = (unsigned char)(fb[i] & M11_FB_INDEX_MASK);
    }
    /* Expand 16-entry palette into a 256-entry RGB array (wraparound). */
    for (i = 0; i < 256; i++) {
        int idx = i & 0x0F;
        palette[i * 3 + 0] = G9010_auc_VgaPaletteAll_Compat[level][idx][0];
        palette[i * 3 + 1] = G9010_auc_VgaPaletteAll_Compat[level][idx][1];
        palette[i * 3 + 2] = G9010_auc_VgaPaletteAll_Compat[level][idx][2];
    }
    return M11_Screenshot_Capture(masked, M11_FB_WIDTH, M11_FB_HEIGHT,
                                  palette, outputDir, outPath, outPathCap);
}
