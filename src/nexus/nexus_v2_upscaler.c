
#include "nexus_v2_upscaler.h"
#include <string.h>
#include <stdlib.h>

/* EPX/Scale2x: for each pixel P, check neighbors A,B,C,D:
 *     A
 *   C P B
 *     D
 * Output 2x2:
 *   P==A && P!=C → A  else P  |  P==B && P!=A → B  else P
 *   P==C && P!=D → C  else P  |  P==D && P!=B → D  else P
 */
void nexus_v2_epx_upscale(
    const uint8_t *src, int src_w, int src_h,
    uint32_t *dst, int dst_w, int dst_h,
    const uint32_t *palette)
{
    int x, y;
    (void)dst_w; (void)dst_h; /* always 2x */

    for (y = 0; y < src_h; y++) {
        for (x = 0; x < src_w; x++) {
            uint8_t P = src[y * src_w + x];
            uint8_t A = (y > 0) ? src[(y-1)*src_w+x] : P;
            uint8_t B = (x < src_w-1) ? src[y*src_w+x+1] : P;
            uint8_t C = (x > 0) ? src[y*src_w+x-1] : P;
            uint8_t D = (y < src_h-1) ? src[(y+1)*src_w+x] : P;

            int dx = x * 2, dy = y * 2;
            int dw = src_w * 2;

            /* Top-left */
            dst[dy*dw+dx] = palette[(P==A && P!=C && A!=B) ? A : P];
            /* Top-right */
            dst[dy*dw+dx+1] = palette[(P==B && P!=A && B!=D) ? B : P];
            /* Bottom-left */
            dst[(dy+1)*dw+dx] = palette[(P==C && P!=D && C!=A) ? C : P];
            /* Bottom-right */
            dst[(dy+1)*dw+dx+1] = palette[(P==D && P!=B && D!=C) ? D : P];
        }
    }
}

/* Simple 3x3 box blur for bilinear-ish smoothing */
void nexus_v2_bilinear_smooth(uint32_t *buf, int w, int h) {
    uint32_t *tmp;
    int x, y;
    tmp = (uint32_t *)malloc(w * h * sizeof(uint32_t));
    if (!tmp) return;
    memcpy(tmp, buf, w * h * sizeof(uint32_t));

    for (y = 1; y < h - 1; y++) {
        for (x = 1; x < w - 1; x++) {
            /* Average center with 4 neighbors (keep alpha) */
            uint32_t c = tmp[y*w+x];
            uint32_t t = tmp[(y-1)*w+x];
            uint32_t b2 = tmp[(y+1)*w+x];
            uint32_t l = tmp[y*w+x-1];
            uint32_t r = tmp[y*w+x+1];

            int rr = (int)((c>>16)&0xFF)*4 + (int)((t>>16)&0xFF) + (int)((b2>>16)&0xFF) + (int)((l>>16)&0xFF) + (int)((r>>16)&0xFF);
            int gg = (int)((c>>8)&0xFF)*4 + (int)((t>>8)&0xFF) + (int)((b2>>8)&0xFF) + (int)((l>>8)&0xFF) + (int)((r>>8)&0xFF);
            int bb = (int)(c&0xFF)*4 + (int)(t&0xFF) + (int)(b2&0xFF) + (int)(l&0xFF) + (int)(r&0xFF);
            rr /= 8; gg /= 8; bb /= 8;
            buf[y*w+x] = 0xFF000000 | (rr<<16) | (gg<<8) | bb;
        }
    }
    free(tmp);
}

