/*
 * ReDMCSB NEC816.C F8140_ScrollMessageAreaUp, PC 3.4 (C21_NEC16) route.
 *
 * F8140 is the byte-moving primitive used by F8162 when it scrolls a video
 * page's message-area rectangle.  It selects the copy direction from the
 * source/destination order so overlapping scanline regions are preserved.
 */
#ifndef FIRESTAFF_REDMCSB_F8140_SCROLL_MESSAGE_AREA_UP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8140_SCROLL_MESSAGE_AREA_UP_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Copies byte_count bytes from source to destination with F8140's direction:
 * source > destination copies low-to-high, otherwise high-to-low.  Both
 * ranges must designate bytes in the same addressable backing allocation.
 */
void redmcsb_f8140_scroll_message_area_up_pc34_compat(
    const uint8_t *source, uint8_t *destination, size_t byte_count);

const char *redmcsb_f8140_scroll_message_area_up_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
