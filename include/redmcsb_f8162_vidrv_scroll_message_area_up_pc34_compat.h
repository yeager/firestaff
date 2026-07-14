/* ReDMCSB NEC816.C F8162_VIDRV_10_ScrollMessageAreaUp, PC 3.4 route. */
#ifndef FIRESTAFF_REDMCSB_F8162_VIDRV_SCROLL_MESSAGE_AREA_UP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8162_VIDRV_SCROLL_MESSAGE_AREA_UP_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8162_SCREEN_STRIDE_BYTES_PC34 160U
#define REDMCSB_F8162_REQUIRED_PLANES_PC34 3U
#define REDMCSB_F8162_MAX_PLANES_PC34 4U

typedef struct {
    int16_t left;
    int16_t top;
    int16_t right;
    int16_t bottom;
} RedmcsbF8162BoxPc34Compat;

/* Each plane is a 160-byte-stride PC 3.4 logical-screen surface. */
typedef struct {
    uint8_t *planes[REDMCSB_F8162_MAX_PLANES_PC34];
    size_t plane_byte_count;
    size_t plane_count;
} RedmcsbF8162VideoPagesPc34Compat;

/*
 * Exact F8162 transfer: for every row top+scroll_rows through bottom, copy
 * ((right-left+1)>>2) bytes at the beginning of that scanline from each plane
 * to the scanline scroll_rows above it.  The source routine leaves the newly
 * exposed rows untouched; its caller supplies the new text bitmap afterwards.
 */
bool redmcsb_f8162_vidrv_scroll_message_area_up_pc34_compat(
    RedmcsbF8162VideoPagesPc34Compat *pages,
    const RedmcsbF8162BoxPc34Compat *box,
    uint16_t scroll_rows);

const char *redmcsb_f8162_vidrv_scroll_message_area_up_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
