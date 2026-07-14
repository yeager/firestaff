#ifndef FIRESTAFF_REDMCSB_F8137_PIXELS_NEC16_COMPAT_H
#define FIRESTAFF_REDMCSB_F8137_PIXELS_NEC16_COMPAT_H
#include <stddef.h>
#include <stdint.h>

void redmcsb_f8137_set_multiple_pixels_nec16_compat(uint8_t *video, size_t bytes, uint16_t pixel, uint8_t color, uint16_t count);
const char *redmcsb_f8137_pixels_nec16_source_evidence(void);
#endif
