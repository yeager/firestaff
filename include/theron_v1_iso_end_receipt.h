#ifndef THERON_V1_ISO_END_RECEIPT_H
#define THERON_V1_ISO_END_RECEIPT_H

#include <stddef.h>

typedef struct { size_t offset, bytes; } Theron_V1IsoEndSpan;
typedef struct {
    int valid;
    int opaque_only;
    size_t image_bytes, span_count;
    const char *variant;
} Theron_V1IsoEndReceipt;

int theron_v1_iso_end_receipt(const char *md5, size_t image_bytes,
                              const Theron_V1IsoEndSpan *spans, size_t count,
                              Theron_V1IsoEndReceipt *out);
int theron_v1_iso_end_compare(const unsigned char *jp, size_t jp_size,
                              const unsigned char *us, size_t us_size,
                              const Theron_V1IsoEndSpan *spans, size_t count,
                              unsigned int *matching_mask,
                              unsigned int *different_mask);
#endif
