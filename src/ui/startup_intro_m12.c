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

static const unsigned char* m12_intro_glyph(char c) {
    static const unsigned char glyphs[][5] = {
        {0x00, 0x00, 0x00, 0x00, 0x00}, /* space */
        {0x1f, 0x05, 0x05, 0x05, 0x01}, /* A */
        {0x1f, 0x15, 0x15, 0x15, 0x0a}, /* B */
        {0x0e, 0x11, 0x11, 0x11, 0x0a}, /* C */
        {0x1f, 0x11, 0x11, 0x0a, 0x04}, /* D */
        {0x1f, 0x15, 0x15, 0x15, 0x11}, /* E */
        {0x1f, 0x05, 0x05, 0x05, 0x01}, /* F */
        {0x0e, 0x11, 0x15, 0x15, 0x1d}, /* G */
        {0x1f, 0x04, 0x04, 0x04, 0x1f}, /* H */
        {0x00, 0x11, 0x1f, 0x11, 0x00}, /* I */
        {0x08, 0x10, 0x10, 0x10, 0x0f}, /* J */
        {0x1f, 0x04, 0x0a, 0x11, 0x00}, /* K */
        {0x1f, 0x10, 0x10, 0x10, 0x10}, /* L */
        {0x1f, 0x02, 0x04, 0x02, 0x1f}, /* M */
        {0x1f, 0x02, 0x04, 0x08, 0x1f}, /* N */
        {0x0e, 0x11, 0x11, 0x11, 0x0e}, /* O */
        {0x1f, 0x05, 0x05, 0x05, 0x02}, /* P */
        {0x0e, 0x11, 0x19, 0x11, 0x1e}, /* Q */
        {0x1f, 0x05, 0x0d, 0x15, 0x02}, /* R */
        {0x12, 0x15, 0x15, 0x15, 0x09}, /* S */
        {0x01, 0x01, 0x1f, 0x01, 0x01}, /* T */
        {0x0f, 0x10, 0x10, 0x10, 0x0f}, /* U */
        {0x07, 0x08, 0x10, 0x08, 0x07}, /* V */
        {0x1f, 0x08, 0x04, 0x08, 0x1f}, /* W */
        {0x1b, 0x04, 0x04, 0x04, 0x1b}, /* X */
        {0x03, 0x04, 0x18, 0x04, 0x03}, /* Y */
        {0x19, 0x15, 0x15, 0x13, 0x00}, /* Z */
        {0x0e, 0x11, 0x11, 0x11, 0x0e}, /* 0 */
        {0x00, 0x12, 0x1f, 0x10, 0x00}, /* 1 */
        {0x19, 0x15, 0x15, 0x15, 0x12}, /* 2 */
        {0x11, 0x15, 0x15, 0x15, 0x0a}, /* 3 */
        {0x07, 0x04, 0x04, 0x04, 0x1f}, /* 4 */
        {0x17, 0x15, 0x15, 0x15, 0x09}, /* 5 */
        {0x0e, 0x15, 0x15, 0x15, 0x08}, /* 6 */
        {0x01, 0x19, 0x05, 0x03, 0x00}, /* 7 */
        {0x0a, 0x15, 0x15, 0x15, 0x0a}, /* 8 */
        {0x02, 0x05, 0x05, 0x05, 0x1e}, /* 9 */
        {0x00, 0x00, 0x0a, 0x00, 0x00}  /* . */
    };
    if (c == ' ') return glyphs[0];
    if (c >= 'A' && c <= 'Z') return glyphs[1 + c - 'A'];
    if (c >= '0' && c <= '9') return glyphs[27 + c - '0'];
    if (c == '.') return glyphs[37];
    return glyphs[0];
}

static void m12_intro_text(unsigned char* rgba, int w, int h, int x, int y,
                           int scale, const char* text, M12_IntroColor color) {
    const char* p = text;
    int cursor = x;
    if (!text || scale < 1) return;
    while (*p) {
        const unsigned char* glyph = m12_intro_glyph(*p++);
        int gy;
        int gx;
        for (gy = 0; gy < 5; ++gy) {
            for (gx = 0; gx < 5; ++gx) {
                int dy;
                int dx;
                if ((glyph[gx] & (1U << gy)) == 0U) continue;
                for (dy = 0; dy < scale; ++dy) {
                    for (dx = 0; dx < scale; ++dx) {
                        m12_intro_pixel(rgba, w, h, cursor + gx * scale + dx,
                                        y + gy * scale + dy, color);
                    }
                }
            }
        }
        cursor += 6 * scale;
    }
}

static void m12_intro_live_embers(unsigned char* rgba, int w, int h,
                                  uint32_t elapsedMs, int alpha) {
    int i;
    int staffX = w / 2 + 78;
    for (i = 0; i < 130; ++i) {
        int phase = (int)(elapsedMs / 42U) + i * 17;
        int rise = (phase * 7 + i * 11) % 142;
        int spread = (i * 13 + phase * 3) % 31 - 15;
        int x = staffX + spread * (142 - rise) / 142;
        int yPos = 200 - rise;
        int heat = 255 - rise;
        M12_IntroColor flame;
        flame.r = 255;
        flame.g = (unsigned char)m12_intro_clamp(55 + heat * 3 / 4);
        flame.b = (unsigned char)m12_intro_clamp(8 + heat / 7);
        m12_intro_blend(rgba, w, h, x, yPos, flame, alpha * (100 + heat / 2) / 255);
        if ((i & 3) == 0) {
            M12_IntroColor core = {255, 236, 136};
            m12_intro_blend(rgba, w, h, x, yPos + 1, core, alpha / 2);
        }
    }
}

void M12_StartupIntro_Render(unsigned char* rgba, int width, int height,
                             uint32_t elapsedMs, uint32_t durationMs,
                             const char* version) {
    int alpha = 255;
    M12_IntroColor footer = {231, 179, 100};
    char versionLine[48] = "VERSION ";
    if (!rgba || width < 320 || height < 200) return;
    if (durationMs > 0U) {
        if (elapsedMs < 500U) alpha = (int)(elapsedMs * 255U / 500U);
        else if (elapsedMs + 700U > durationMs) alpha = (int)((durationMs - elapsedMs) * 255U / 700U);
        if (alpha < 0) alpha = 0;
        if (alpha > 255) alpha = 255;
    }
    m12_intro_background(rgba, width, height, elapsedMs);
    m12_intro_live_embers(rgba, width, height, elapsedMs, alpha);
    if (version) {
        size_t room = sizeof(versionLine) - strlen(versionLine) - 1U;
        strncat(versionLine, version, room);
    }
    m12_intro_text(rgba, width, height, 16, height - 28, 2, versionLine, footer);
    m12_intro_text(rgba, width, height, width - 184, height - 28, 2,
                   "DANIEL NYLANDER", footer);
}
