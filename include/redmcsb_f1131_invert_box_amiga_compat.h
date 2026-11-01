/* ReDMCSB FILLBOX.C F1131_InvertBox, Amiga A31/A35 route. */
#ifndef FIRESTAFF_REDMCSB_F1131_INVERT_BOX_AMIGA_COMPAT_H
#define FIRESTAFF_REDMCSB_F1131_INVERT_BOX_AMIGA_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
} RedmcsbF1131AmigaBoxCompat;

typedef struct {
    void *context;
    void (*set_draw_mode)(void *context, int32_t mode);
    void (*set_pen)(void *context, int32_t pen);
    void (*set_mask)(void *context, uint8_t mask);
    void (*wait_for_scan_line)(void *context, int32_t line);
    void (*rect_fill)(void *context, int32_t left, int32_t top,
                      int32_t right, int32_t bottom);
    void (*wait_blit)(void *context);
} RedmcsbF1131AmigaRasterCompat;

/* A NULL box is the source no-op and returns true. Invalid backend returns
 * false before any raster callback is issued. */
bool redmcsb_f1131_invert_box_amiga_compat(
    const RedmcsbF1131AmigaRasterCompat *raster,
    const RedmcsbF1131AmigaBoxCompat *box);

const char *redmcsb_f1131_invert_box_amiga_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
