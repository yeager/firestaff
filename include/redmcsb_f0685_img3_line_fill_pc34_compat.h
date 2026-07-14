#ifndef FIRESTAFF_REDMCSB_F0685_IMG3_LINE_FILL_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0685_IMG3_LINE_FILL_PC34_COMPAT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* ReDMCSB IMAGE4.C F0685, PC I34E/I34M packed 4bpp destination route. */
bool redmcsb_f0685_img3_line_fill_pc34_compat(uint8_t *destination, size_t destination_byte_count, size_t pixel_index, uint8_t color, size_t pixel_count);
const char *redmcsb_f0685_img3_line_fill_source_evidence_pc34(void);
#endif
