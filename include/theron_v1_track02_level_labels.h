#ifndef THERON_V1_TRACK02_LEVEL_LABELS_H
#define THERON_V1_TRACK02_LEVEL_LABELS_H

#include <stddef.h>
#include <stdint.h>

#define THERON_TRACK02_LEVEL_LABEL_COUNT  16u
#define THERON_TRACK02_LEVEL_LABEL_WIDTH   8u

const char *theron_v1_track02_us_level_label(unsigned int index);
size_t theron_v1_track02_us_level_label_count(void);

#endif /* THERON_V1_TRACK02_LEVEL_LABELS_H */
