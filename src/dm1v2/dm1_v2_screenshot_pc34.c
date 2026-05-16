#include "dm1_v2_screenshot_pc34.h"
#include <stdio.h>
#include <time.h>
#include "dm1_v2_screenshot_pc34.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

void v2_screenshot_init(void) {
    srand((unsigned int)time(NULL));
}

void v2_screenshot_auto_name(char* buf, int bufsize) {
    if (!buf || bufsize <= 0) return;
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buf, bufsize, "screenshot_%Y%m%d_%H%M%S.bmp", tm_info);
}

int v2_screenshot_capture(uint8_t* fb, int w, int h, uint32_t* palette, int palette_size, const char* path) {
    if (!fb || !path || w <= 0 || h <= 0) return -1;
    if (palette_size <= 0) palette_size = 256;
    if (palette_size > 256) palette_size = 256;

    int row_size = (w + 3) & ~3;
    int img_size = row_size * h;
    int file_size = 14 + 40 + (palette_size * 4) + img_size;

    FILE* f = fopen(path, "wb");
    if (!f) return -1;

    uint8_t header[14];
    header[0] = 'B'; header[1] = 'M';
    *(uint32_t*)&header[2] = (uint32_t)file_size;
    *(uint16_t*)&header[6] = 0;
    *(uint16_t*)&header[8] = 0;
    *(uint32_t*)&header[10] = 54 + (palette_size * 4);
    fwrite(header, 1, 14, f);

    uint8_t dib[40];
    *(uint32_t*)&dib[0] = 40;
    *(int32_t*)&dib[4] = w;
    *(int32_t*)&dib[8] = h;
    *(uint16_t*)&dib[12] = 1;
    *(uint16_t*)&dib[14] = 8;
    *(uint32_t*)&dib[16] = 0;
    *(uint32_t*)&dib[20] = (uint32_t)img_size;
    *(uint32_t*)&dib[24] = 0;
    *(uint32_t*)&dib[28] = 0;
    *(uint32_t*)&dib[32] = (uint32_t)palette_size;
    *(uint32_t*)&dib[36] = (uint32_t)palette_size;
    fwrite(dib, 1, 40, f);

    uint8_t pal_buf[4];
    for (int i = 0; i < palette_size; i++) {
        uint32_t col = palette ? palette[i] : 0;
        pal_buf[0] = (uint8_t)(col & 0xFF);
        pal_buf[1] = (uint8_t)((col >> 8) & 0xFF);
        pal_buf[2] = (uint8_t)((col >> 16) & 0xFF);
        pal_buf[3] = 0;
        fwrite(pal_buf, 1, 4, f);
    }

    uint8_t* row_buf = (uint8_t*)malloc(row_size);
    if (!row_buf) { fclose(f); return -1; }

    for (int y = h - 1; y >= 0; y--) {
        uint8_t* src = fb + (y * w);
        memcpy(row_buf, src, w);
        memset(row_buf + w, 0, row_size - w);
        fwrite(row_buf, 1, row_size, f);
    }

    free(row_buf);
    fclose(f);
    return 0;
}

/* V2.2 Screenshot — key binding integration */

static int g_screenshot_pending = 0;
static char g_screenshot_last_path[256] = {0};

void v22_screenshot_request(void) { g_screenshot_pending = 1; }

int v22_screenshot_is_pending(void) { return g_screenshot_pending; }

int v22_screenshot_process(const uint32_t *rgba, int w, int h,
    const char *save_dir)
{
    char path[256];
    int result;
    if (!g_screenshot_pending) return 0;
    g_screenshot_pending = 0;

    if (!save_dir) save_dir = ".";
    snprintf(path, sizeof(path), "%s/", save_dir);
    v2_screenshot_auto_path(path + strlen(path),
        (int)(sizeof(path) - strlen(path)), "firestaff");

    result = v2_screenshot_capture((uint8_t*)rgba, w, h, NULL, 0, path);
    if (result == 0) {
        strncpy(g_screenshot_last_path, path, sizeof(g_screenshot_last_path) - 1);
    }
    return result;
}

const char *v22_screenshot_last_path(void) {
    return g_screenshot_last_path[0] ? g_screenshot_last_path : NULL;
}

