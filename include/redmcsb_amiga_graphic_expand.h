#ifndef REDMCSB_AMIGA_GRAPHIC_EXPAND_H
#define REDMCSB_AMIGA_GRAPHIC_EXPAND_H

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB EXPAND.C F0466: expand one bounded Amiga graphics run stream into
 * 4-bit color indices. Width and height are the stream's big-endian header. */
int redmcsb_amiga_graphic_expand(const uint8_t *stream, size_t stream_size,
                                 uint8_t *indexed_pixels,
                                 size_t pixel_capacity,
                                 uint16_t *out_width,
                                 uint16_t *out_height);

#endif
