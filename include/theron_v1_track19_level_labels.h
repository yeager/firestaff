#ifndef THERON_V1_TRACK19_LEVEL_LABELS_H
#define THERON_V1_TRACK19_LEVEL_LABELS_H

#include <stddef.h>
#include <stdint.h>

#define THERON_TRACK19_US_LEVEL_LABEL_COUNT 15u
#define THERON_TRACK19_US_LEVEL_LABEL_OFFSET 2112059u

const char *theron_v1_track19_us_level_label(unsigned int index);

/* Validate the complete source-owned label table before returning one label. */
int theron_v1_track19_us_level_label_from_iso(
    const uint8_t *iso, size_t iso_size, unsigned int index,
    char *out, size_t out_capacity);

#endif
