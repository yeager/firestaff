#include "theron_v1_iso_end_receipt.h"

#include "theron_v1_track02.h"

#include <string.h>

int theron_v1_iso_end_receipt(const char *md5, size_t image_bytes,
                              const Theron_V1IsoEndSpan *spans, size_t count,
                              Theron_V1IsoEndReceipt *out) {
    size_t i;
    const char *variant;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    variant = md5 && strcmp(md5, THERON_TRACK02_MD5_US_ISO) == 0 ? "us-iso-end" :
        md5 && strcmp(md5, THERON_TRACK02_MD5_JP_REV1_ISO) == 0 ? "jp-iso-end" : NULL;
    if (!variant || !image_bytes || !spans || !count) return 0;
    for (i = 0; i < count; i++) {
        if (!spans[i].bytes || spans[i].offset > image_bytes ||
            spans[i].bytes > image_bytes - spans[i].offset ||
            (i && spans[i - 1].offset + spans[i - 1].bytes > spans[i].offset)) return 0;
    }
    out->valid = 1;
    out->opaque_only = 1;
    out->image_bytes = image_bytes;
    out->span_count = count;
    out->variant = variant;
    return 1;
}

int theron_v1_iso_end_compare(const unsigned char *jp, size_t jp_size,
                              const unsigned char *us, size_t us_size,
                              const Theron_V1IsoEndSpan *spans, size_t count,
                              unsigned int *matching_mask,
                              unsigned int *different_mask) {
    size_t i;
    unsigned int same = 0, different = 0;

    if (!jp || !us || !spans || !count || count > sizeof(unsigned int) * 8u) return 0;
    for (i = 0; i < count; i++) {
        if (!spans[i].bytes || spans[i].offset > jp_size ||
            spans[i].bytes > jp_size - spans[i].offset || spans[i].offset > us_size ||
            spans[i].bytes > us_size - spans[i].offset) return 0;
        if (memcmp(jp + spans[i].offset, us + spans[i].offset, spans[i].bytes) == 0)
            same |= 1u << i;
        else
            different |= 1u << i;
    }
    if (matching_mask) *matching_mask = same;
    if (different_mask) *different_mask = different;
    return 1;
}
