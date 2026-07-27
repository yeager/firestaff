#include "startup_intro_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char* g_m12_intro_background_rgb;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} M12_IntroColor;

static int m12_intro_clamp(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

static void m12_intro_pixel(unsigned char* rgba, int w, int h, int x, int y,
                            M12_IntroColor color) {
    unsigned char* p;
    if (!rgba || x < 0 || y < 0 || x >= w || y >= h) return;
    p = rgba + (((size_t)y * (size_t)w + (size_t)x) * 4U);
    p[0] = color.r;
    p[1] = color.g;
    p[2] = color.b;
    p[3] = 255U;
}

static void m12_intro_blend(unsigned char* rgba, int w, int h, int x, int y,
                            M12_IntroColor color, int alpha) {
    unsigned char* p;
    int inverse;
    if (!rgba || x < 0 || y < 0 || x >= w || y >= h || alpha <= 0) return;
    if (alpha >= 255) {
        m12_intro_pixel(rgba, w, h, x, y, color);
        return;
    }
    p = rgba + (((size_t)y * (size_t)w + (size_t)x) * 4U);
    inverse = 255 - alpha;
    p[0] = (unsigned char)((p[0] * inverse + color.r * alpha) / 255);
    p[1] = (unsigned char)((p[1] * inverse + color.g * alpha) / 255);
    p[2] = (unsigned char)((p[2] * inverse + color.b * alpha) / 255);
    p[3] = 255U;
}

int M12_StartupIntro_LoadBackground(const char* path) {
    FILE* file;
    char magic[3] = {0, 0, 0};
    int width = 0;
    int height = 0;
    int maxValue = 0;
    unsigned char* bytes;
    size_t byteCount;
    if (!path || !*path || g_m12_intro_background_rgb) return g_m12_intro_background_rgb != NULL;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fscanf(file, "%2s %d %d %d", magic, &width, &height, &maxValue) != 4 ||
        strcmp(magic, "P6") != 0 || width != M12_STARTUP_INTRO_WIDTH ||
        height != M12_STARTUP_INTRO_HEIGHT || maxValue != 255) {
        fclose(file);
        return 0;
    }
    if (fgetc(file) == EOF) {
        fclose(file);
        return 0;
    }
    byteCount = (size_t)width * (size_t)height * 3U;
    bytes = (unsigned char*)malloc(byteCount);
    if (!bytes || fread(bytes, 1, byteCount, file) != byteCount) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    g_m12_intro_background_rgb = bytes;
    return 1;
}

static void m12_intro_background(unsigned char* rgba, int w, int h, uint32_t tick) {
    int x;
    int y;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            M12_IntroColor color;
            if (g_m12_intro_background_rgb &&
                w == M12_STARTUP_INTRO_WIDTH && h == M12_STARTUP_INTRO_HEIGHT) {
                const unsigned char* source = g_m12_intro_background_rgb +
                    (((size_t)y * (size_t)w + (size_t)x) * 3U);
                int shade = x < (w * 3) / 5 ? 72 : 100;
                color.r = (unsigned char)(source[0] * shade / 100);
                color.g = (unsigned char)(source[1] * shade / 100);
                color.b = (unsigned char)(source[2] * shade / 100);
            } else {
                int ember = (int)((x * 13 + y * 7 + (int)(tick / 31U)) % 43U);
                color.r = (unsigned char)(5 + ember / 10);
                color.g = (unsigned char)(7 + ember / 18);
                color.b = (unsigned char)(13 + ember / 12);
            }
            m12_intro_pixel(rgba, w, h, x, y, color);
        }
    }
}

/* 4x6 bitmap font — each glyph is 4 columns of 6-bit rows (bit 0 = top).
 * Compact and clean at 1x scale; no scaling needed at 480x270. */
enum { M12_FONT_W = 4, M12_FONT_H = 6 };

static const unsigned char* m12_intro_glyph(char c) {
    static const unsigned char glyphs[][4] = {
        {0x00, 0x00, 0x00, 0x00}, /* space */
        {0x3e, 0x09, 0x09, 0x3e}, /* A */
        {0x3f, 0x25, 0x25, 0x1a}, /* B */
        {0x1e, 0x21, 0x21, 0x12}, /* C */
        {0x3f, 0x21, 0x21, 0x1e}, /* D */
        {0x3f, 0x25, 0x25, 0x21}, /* E */
        {0x3f, 0x05, 0x05, 0x01}, /* F */
        {0x1e, 0x21, 0x29, 0x3a}, /* G */
        {0x3f, 0x04, 0x04, 0x3f}, /* H */
        {0x21, 0x3f, 0x21, 0x00}, /* I */
        {0x10, 0x20, 0x20, 0x1f}, /* J */
        {0x3f, 0x04, 0x0a, 0x31}, /* K */
        {0x3f, 0x20, 0x20, 0x20}, /* L */
        {0x3f, 0x02, 0x02, 0x3f}, /* M */
        {0x3f, 0x02, 0x04, 0x3f}, /* N */
        {0x1e, 0x21, 0x21, 0x1e}, /* O */
        {0x3f, 0x09, 0x09, 0x06}, /* P */
        {0x1e, 0x21, 0x31, 0x3e}, /* Q */
        {0x3f, 0x09, 0x19, 0x26}, /* R */
        {0x22, 0x25, 0x25, 0x19}, /* S */
        {0x01, 0x3f, 0x01, 0x00}, /* T */
        {0x1f, 0x20, 0x20, 0x1f}, /* U */
        {0x0f, 0x10, 0x20, 0x1f}, /* V */
        {0x3f, 0x10, 0x10, 0x3f}, /* W */
        {0x33, 0x0c, 0x0c, 0x33}, /* X */
        {0x03, 0x04, 0x38, 0x07}, /* Y */
        {0x31, 0x29, 0x25, 0x23}, /* Z */
        {0x1e, 0x21, 0x21, 0x1e}, /* 0 */
        {0x22, 0x3f, 0x20, 0x00}, /* 1 */
        {0x32, 0x29, 0x29, 0x26}, /* 2 */
        {0x21, 0x25, 0x25, 0x1a}, /* 3 */
        {0x0f, 0x08, 0x08, 0x3f}, /* 4 */
        {0x27, 0x25, 0x25, 0x19}, /* 5 */
        {0x1e, 0x25, 0x25, 0x18}, /* 6 */
        {0x01, 0x39, 0x05, 0x03}, /* 7 */
        {0x1a, 0x25, 0x25, 0x1a}, /* 8 */
        {0x06, 0x29, 0x29, 0x1e}, /* 9 */
        {0x00, 0x20, 0x00, 0x00}, /* . */
        {0x08, 0x08, 0x08, 0x00}, /* - */
    };
    if (c == ' ') return glyphs[0];
    if (c >= 'A' && c <= 'Z') return glyphs[1 + c - 'A'];
    if (c >= 'a' && c <= 'z') return glyphs[1 + c - 'a'];
    if (c >= '0' && c <= '9') return glyphs[27 + c - '0'];
    if (c == '.') return glyphs[37];
    if (c == '-') return glyphs[38];
    return glyphs[0];
}

static void m12_intro_text(unsigned char* rgba, int w, int h, int x, int y,
                           const char* text, M12_IntroColor color, int alpha) {
    const char* p = text;
    int cursor = x;
    if (!text) return;
    while (*p) {
        const unsigned char* glyph = m12_intro_glyph(*p++);
        int gy;
        int gx;
        for (gx = 0; gx < M12_FONT_W; ++gx) {
            for (gy = 0; gy < M12_FONT_H; ++gy) {
                if ((glyph[gx] & (1U << gy)) == 0U) continue;
                m12_intro_blend(rgba, w, h, cursor + gx, y + gy, color, alpha);
            }
        }
        cursor += M12_FONT_W + 1;
    }
}

static int m12_intro_text_width(const char* text) {
    int len;
    if (!text || !*text) return 0;
    len = (int)strlen(text);
    return len * (M12_FONT_W + 1) - 1;
}

/* Deterministic hash for pseudo-random values without Date/random. */
static unsigned int m12_hash(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3bU;
    x = ((x >> 16) ^ x) * 0x45d9f3bU;
    x = (x >> 16) ^ x;
    return x;
}

/* Smooth noise interpolation for organic motion. */
static int m12_noise(int x, int y, int seed) {
    unsigned int h00 = m12_hash((unsigned int)(x + y * 373 + seed * 7919));
    unsigned int h10 = m12_hash((unsigned int)(x + 1 + y * 373 + seed * 7919));
    unsigned int h01 = m12_hash((unsigned int)(x + (y + 1) * 373 + seed * 7919));
    unsigned int h11 = m12_hash((unsigned int)(x + 1 + (y + 1) * 373 + seed * 7919));
    int v00 = (int)(h00 & 0xFFU);
    int v10 = (int)(h10 & 0xFFU);
    int v01 = (int)(h01 & 0xFFU);
    int v11 = (int)(h11 & 0xFFU);
    return (v00 + v10 + v01 + v11) / 4;
}

/* Layered smoke: multiple soft blobs rising with turbulence. Each blob
 * is a radial gradient disc that drifts, expands, and fades. */
static void m12_intro_smoke(unsigned char* rgba, int w, int h,
                            uint32_t elapsedMs, int masterAlpha) {
    int i;
    int staffX = w / 2 + 78;
    int staffY = 72;
    int particleCount = 80;

    for (i = 0; i < particleCount; ++i) {
        unsigned int seed = m12_hash((unsigned int)i * 2654435761U);
        int cycleMs = 2800 + (int)(seed % 1800U);
        int offsetMs = (int)(seed % (unsigned int)cycleMs);
        int age = ((int)elapsedMs + offsetMs) % cycleMs;
        float t = (float)age / (float)cycleMs;

        float riseSpeed = 0.6f + (float)(seed % 100U) / 250.0f;
        float riseY = t * riseSpeed * 160.0f;

        int turbSeed = (int)((elapsedMs / 60U) + seed);
        int turb = m12_noise((int)(t * 8.0f), (int)seed, turbSeed) - 128;
        float drift = (float)(((int)(seed % 41U)) - 20) * t * 0.8f;
        float turbX = drift + (float)turb * t * 0.12f;

        float radius = 2.0f + t * 12.0f + (float)(seed % 5U);
        float fadeIn = t < 0.08f ? t / 0.08f : 1.0f;
        float fadeOut = t > 0.6f ? (1.0f - t) / 0.4f : 1.0f;
        float opacity = fadeIn * fadeOut * 0.7f;

        float heat = 1.0f - t;
        int cr = m12_intro_clamp((int)(60 + heat * 140));
        int cg = m12_intro_clamp((int)(45 + heat * 70));
        int cb = m12_intro_clamp((int)(35 + heat * 30));

        float cx = (float)staffX + turbX;
        float cy = (float)staffY - riseY;
        int irad = (int)(radius + 1.0f);
        int px;
        int py;
        int ix = (int)cx;
        int iy = (int)cy;

        if (opacity <= 0.0f) continue;

        for (py = -irad; py <= irad; ++py) {
            for (px = -irad; px <= irad; ++px) {
                float dx = (float)px + (cx - (float)ix);
                float dy = (float)py + (cy - (float)iy);
                float dist2 = dx * dx + dy * dy;
                float r2 = radius * radius;
                float falloff;
                int a;
                M12_IntroColor sc;

                if (dist2 > r2) continue;
                falloff = 1.0f - dist2 / r2;
                falloff = falloff * falloff;
                a = (int)(falloff * opacity * (float)masterAlpha);
                if (a <= 0) continue;
                if (a > 255) a = 255;
                sc.r = (unsigned char)cr;
                sc.g = (unsigned char)cg;
                sc.b = (unsigned char)cb;
                m12_intro_blend(rgba, w, h, ix + px, iy + py, sc, a);
            }
        }
    }
}

/* Small bright sparks that drift upward from the staff tip. */
static void m12_intro_sparks(unsigned char* rgba, int w, int h,
                             uint32_t elapsedMs, int masterAlpha) {
    int i;
    int staffX = w / 2 + 78;
    int staffY = 72;

    for (i = 0; i < 25; ++i) {
        unsigned int seed = m12_hash((unsigned int)i * 48271U + 12345U);
        int cycleMs = 900 + (int)(seed % 600U);
        int offsetMs = (int)(seed % (unsigned int)cycleMs);
        int age = ((int)elapsedMs + offsetMs) % cycleMs;
        float t = (float)age / (float)cycleMs;

        float rise = t * 70.0f;
        float sway = (float)(m12_noise((int)(t * 12.0f), (int)i, (int)(elapsedMs / 40U)) - 128);
        float sx = (float)staffX + sway * t * 0.06f + (float)(((int)(seed % 11U)) - 5);
        float sy = (float)staffY - rise;

        float bright = t < 0.1f ? t / 0.1f : (1.0f - t) / 0.9f;
        int a = (int)(bright * (float)masterAlpha);
        M12_IntroColor spark;

        if (a <= 0) continue;
        if (a > 255) a = 255;
        spark.r = 255;
        spark.g = (unsigned char)m12_intro_clamp((int)(180 + bright * 75));
        spark.b = (unsigned char)m12_intro_clamp((int)(40 + bright * 100));
        m12_intro_blend(rgba, w, h, (int)sx, (int)sy, spark, a);
        m12_intro_blend(rgba, w, h, (int)sx + 1, (int)sy, spark, a / 3);
        m12_intro_blend(rgba, w, h, (int)sx, (int)sy + 1, spark, a / 3);
    }
}

void M12_StartupIntro_Render(unsigned char* rgba, int width, int height,
                             uint32_t elapsedMs, uint32_t durationMs,
                             const char* version) {
    int alpha = 255;
    M12_IntroColor footer = {190, 160, 120};
    char versionLine[48] = "";
    int nameW;
    if (!rgba || width < 320 || height < 200) return;
    if (durationMs > 0U) {
        if (elapsedMs < 500U) alpha = (int)(elapsedMs * 255U / 500U);
        else if (elapsedMs + 700U > durationMs) alpha = (int)((durationMs - elapsedMs) * 255U / 700U);
        if (alpha < 0) alpha = 0;
        if (alpha > 255) alpha = 255;
    }
    m12_intro_background(rgba, width, height, elapsedMs);
    m12_intro_smoke(rgba, width, height, elapsedMs, alpha);
    m12_intro_sparks(rgba, width, height, elapsedMs, alpha);
    if (version) {
        snprintf(versionLine, sizeof(versionLine), "v%s", version);
    }
    m12_intro_text(rgba, width, height, 12, height - 14, versionLine, footer, alpha);
    nameW = m12_intro_text_width("DANIEL NYLANDER");
    m12_intro_text(rgba, width, height, width - nameW - 12, height - 14,
                   "DANIEL NYLANDER", footer, alpha);
}
