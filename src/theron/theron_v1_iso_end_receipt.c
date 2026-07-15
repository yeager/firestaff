#include "theron_v1_iso_end_receipt.h"

#include "theron_v1_track02.h"

#include <string.h>

static int theron_v1_iso_end_span_valid(size_t total_bytes,
                                        const Theron_V1IsoEndSpan *span) {
    return span && span->byte_count != 0u &&
        span->offset <= total_bytes &&
        span->byte_count <= total_bytes - span->offset;
}

static int theron_v1_iso_end_spans_valid(size_t total_bytes,
                                         const Theron_V1IsoEndSpan *spans,
                                         size_t span_count) {
    size_t i;

    if (!spans || span_count == 0u) {
        return 0;
    }
    for (i = 0u; i < span_count; ++i) {
        if (!theron_v1_iso_end_span_valid(total_bytes, &spans[i])) {
            return 0;
        }
    }
    return 1;
}

static int theron_v1_iso_end_md5_supported(const char *track02_md5) {
    return track02_md5 &&
        (strcmp(track02_md5, THERON_TRACK02_MD5_US_ISO) == 0 ||
         strcmp(track02_md5, THERON_TRACK02_MD5_JP_REV1_ISO) == 0);
}

int theron_v1_iso_end_receipt(const char *track02_md5,
                              size_t track02_bytes,
                              const Theron_V1IsoEndSpan *opaque_spans,
                              size_t opaque_span_count,
                              Theron_V1IsoEndReceipt *out_receipt) {
    Theron_V1IsoEndReceipt receipt = {0};

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!out_receipt || !track02_md5 || track02_bytes == 0u ||
        !theron_v1_iso_end_spans_valid(track02_bytes, opaque_spans,
                                       opaque_span_count)) {
        return 0;
    }

    if (!theron_v1_iso_end_md5_supported(track02_md5)) {
        return 0;
    }

    receipt.valid = 1;
    receipt.opaque_only = 1;
    receipt.loader_usable = 0;
    receipt.bitmap_usable = 0;
    receipt.level_route_usable = 0;
    *out_receipt = receipt;
    return 1;
}

int theron_v1_iso_end_compare(const uint8_t *left,
                              size_t left_bytes,
                              const uint8_t *right,
                              size_t right_bytes,
                              const Theron_V1IsoEndSpan *opaque_spans,
                              size_t opaque_span_count,
                              unsigned int *out_same_spans,
                              unsigned int *out_different_spans) {
    unsigned int same = 0u;
    unsigned int different = 0u;
    size_t i;

    if (out_same_spans) {
        *out_same_spans = 0u;
    }
    if (out_different_spans) {
        *out_different_spans = 0u;
    }
    if (!left || !right || !out_same_spans || !out_different_spans ||
        !theron_v1_iso_end_spans_valid(left_bytes, opaque_spans,
                                       opaque_span_count) ||
        !theron_v1_iso_end_spans_valid(right_bytes, opaque_spans,
                                       opaque_span_count)) {
        return 0;
    }

    for (i = 0u; i < opaque_span_count; ++i) {
        const Theron_V1IsoEndSpan *span = &opaque_spans[i];
        if (memcmp(left + span->offset, right + span->offset,
                   span->byte_count) == 0) {
            ++same;
        } else {
            ++different;
        }
    }
    *out_same_spans = same;
    *out_different_spans = different;
    return 1;
}
