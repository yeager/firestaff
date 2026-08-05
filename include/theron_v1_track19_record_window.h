#ifndef THERON_V1_TRACK19_RECORD_WINDOW_H
#define THERON_V1_TRACK19_RECORD_WINDOW_H

#include <stddef.h>
#include <stdint.h>

/* A byte-identical US/JP Track 19 window immediately follows each variant's
 * 69-entry item-name table. The window is retained as opaque evidence only:
 * it is not a map, object, item-property, bitmap or palette contract. */
#define THERON_TRACK19_OPAQUE_RECORD_WINDOW_US_OFFSET 0x0E951Eu
#define THERON_TRACK19_OPAQUE_RECORD_WINDOW_JP_OFFSET 0x0E955Eu
#define THERON_TRACK19_OPAQUE_RECORD_WINDOW_BYTES 502u

int theron_v1_track19_opaque_record_window_validate(
    const uint8_t *iso, size_t iso_size, int japanese_variant,
    size_t *out_offset, size_t *out_bytes);

#endif
