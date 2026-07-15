#ifndef THERON_V1_ISO_END_RECEIPT_H
#define THERON_V1_ISO_END_RECEIPT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t offset;
    size_t byte_count;
} Theron_V1IsoEndSpan;

typedef struct {
    int valid;
    int opaque_only;
    int loader_usable;
    int bitmap_usable;
    int level_route_usable;
} Theron_V1IsoEndReceipt;

int theron_v1_iso_end_receipt(const char *track02_md5,
                              size_t track02_bytes,
                              const Theron_V1IsoEndSpan *opaque_spans,
                              size_t opaque_span_count,
                              Theron_V1IsoEndReceipt *out_receipt);

int theron_v1_iso_end_compare(const uint8_t *left,
                              size_t left_bytes,
                              const uint8_t *right,
                              size_t right_bytes,
                              const Theron_V1IsoEndSpan *opaque_spans,
                              size_t opaque_span_count,
                              unsigned int *out_same_spans,
                              unsigned int *out_different_spans);

#endif
