#include "nexus_v1_warning_dgt2_resource_corpus.h"

#include <string.h>

/* Canonical per-resource facts for the SHA-256-attested WARNING.BIN. These
 * are provenance bindings of the same class as the existing resource-zero
 * constants in nexus_v1_warning_dgt2_pp_execution.c: descriptor offsets and
 * lengths, PP dimensions, the 512-byte CLUT, and the two trailing bytes per
 * resource. They assign no colour, image, palette, record, or presentation
 * semantics beyond the already admitted ST-124 section-6 PP contract. */
static const uint32_t k_resource_offsets[NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT] = {
    0x48U, 0x5c58U, 0xb868U, 0xf8f8U
};
static const uint32_t k_resource_lengths[NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT] = {
    0x5c10U, 0x5c10U, 0x4090U, 0x9290U
};
static const uint16_t k_resource_widths[NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT] = {
    240U, 240U, 200U, 272U
};
static const uint16_t k_resource_heights[NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT] = {
    96U, 96U, 80U, 136U
};

static uint16_t be16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] << 8 | (uint16_t)p[1]);
}

static uint32_t be32(const uint8_t *p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 |
           (uint32_t)p[2] << 8 | (uint32_t)p[3];
}

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0; index < size; ++index) {
        value ^= bytes[index];
        value *= UINT64_C(1099511628211);
    }
    return value;
}

static uint8_t expand_5_to_6(uint16_t value)
{
    return (uint8_t)((value << 1) | (value >> 4));
}

static int identity_usable(const uint8_t *source_bytes, size_t source_size,
    const Nexus_V1_WarningDgt2SourceIdentity *identity, uint64_t *out_source_fnv)
{
    uint64_t source_fnv;
    if (!source_bytes || !identity ||
        source_size != NEXUS_V1_WARNING_BIN_BYTES || !identity->source_fnv1a64 ||
        (!((identity->sha256_verified && identity->sha256_hex &&
            strcmp(identity->sha256_hex, NEXUS_V1_WARNING_BIN_SHA256) == 0) ||
           identity->canonical_fnv1a64_verified))) {
        return 0;
    }
    source_fnv = fnv1a64(source_bytes, source_size);
    if (source_fnv != identity->source_fnv1a64 ||
        (source_fnv != NEXUS_V1_WARNING_BIN_FNV1A64_JA &&
         source_fnv != NEXUS_V1_WARNING_BIN_FNV1A64_EN &&
         source_fnv != NEXUS_V1_WARNING_BIN_FNV1A64_FR)) {
        return 0;
    }
    if (out_source_fnv) *out_source_fnv = source_fnv;
    return 1;
}

int nexus_v1_warning_dgt2_resource_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_WarningDgt2SourceIdentity *identity,
    uint32_t descriptor_index,
    Nexus_V1_WarningDgt2ResourceReceipt *out_receipt)
{
    Nexus_V1_WarningDgt2ResourceReceipt receipt;
    const uint8_t *entry;
    const uint8_t *resource;
    uint64_t source_fnv;
    uint32_t table_length;
    uint32_t resource_offset;
    uint32_t resource_end;
    uint32_t pp_offset;
    uint32_t clut_offset;
    uint32_t pixel_offset;
    uint32_t pixel_length;
    uint32_t expected_next;
    uint32_t index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity_usable(source_bytes, source_size, identity, &source_fnv) ||
        descriptor_index >= NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT ||
        source_size < 12U || memcmp(source_bytes, "RES*", 4) != 0 ||
        be32(source_bytes + 4U) != source_size ||
        be16(source_bytes + 8U) != NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT) {
        *out_receipt = receipt;
        return 0;
    }
    table_length = NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT * 12U;
    if (table_length > source_size - 12U) {
        *out_receipt = receipt;
        return 0;
    }
    for (index = 0U; index < NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT; ++index) {
        entry = source_bytes + 12U + (size_t)index * 12U;
        if (memcmp(entry, "DGT2", 4) != 0 || be32(entry + 4U) != index ||
            be32(entry + 8U) != k_resource_offsets[index]) {
            *out_receipt = receipt;
            return 0;
        }
    }
    resource_offset = k_resource_offsets[descriptor_index];
    resource_end = resource_offset + k_resource_lengths[descriptor_index];
    expected_next = descriptor_index + 1U < NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT ?
        k_resource_offsets[descriptor_index + 1U] : (uint32_t)source_size;
    pp_offset = resource_offset + 8U;
    clut_offset = pp_offset + 6U;
    pixel_offset = clut_offset + NEXUS_V1_WARNING_DGT2_CLUT_BYTES;
    pixel_length = (uint32_t)k_resource_widths[descriptor_index] *
                   (uint32_t)k_resource_heights[descriptor_index];
    if (resource_end != expected_next || resource_end > source_size ||
        resource_offset < 12U + table_length ||
        pixel_length != k_resource_lengths[descriptor_index] - 14U -
            NEXUS_V1_WARNING_DGT2_CLUT_BYTES - NEXUS_V1_WARNING_DGT2_TRAILING_BYTES ||
        pixel_offset > source_size || pixel_length > source_size - pixel_offset ||
        pixel_offset + pixel_length + NEXUS_V1_WARNING_DGT2_TRAILING_BYTES !=
            resource_end) {
        *out_receipt = receipt;
        return 0;
    }
    resource = source_bytes + resource_offset;
    if (memcmp(resource, "DGT2", 4) != 0 ||
        be32(resource + 4U) != descriptor_index ||
        memcmp(resource + 8U, "PP", 2) != 0 ||
        be16(resource + 10U) != k_resource_widths[descriptor_index] ||
        be16(resource + 12U) != k_resource_heights[descriptor_index]) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.valid = receipt.source_identity_bound = receipt.res_directory_bound =
        receipt.descriptor_bound = receipt.dgt2_header_bound =
        receipt.pp_header_bound = receipt.payload_bound = 1;
    receipt.source_fnv1a64 = source_fnv;
    receipt.descriptor_index = descriptor_index;
    receipt.descriptor_id = descriptor_index;
    receipt.descriptor_offset = resource_offset;
    receipt.resource_length = k_resource_lengths[descriptor_index];
    receipt.resource_fnv1a64 = fnv1a64(resource, receipt.resource_length);
    receipt.pp_header_offset = pp_offset;
    receipt.pp_width = k_resource_widths[descriptor_index];
    receipt.pp_height = k_resource_heights[descriptor_index];
    receipt.pp_header_fnv1a64 = fnv1a64(source_bytes + pp_offset, 6U);
    receipt.clut_offset = clut_offset;
    receipt.clut_length = NEXUS_V1_WARNING_DGT2_CLUT_BYTES;
    receipt.clut_fnv1a64 = fnv1a64(source_bytes + clut_offset, receipt.clut_length);
    receipt.pixel_offset = pixel_offset;
    receipt.pixel_length = pixel_length;
    receipt.pixel_fnv1a64 = fnv1a64(source_bytes + pixel_offset, pixel_length);
    receipt.trailing_offset = pixel_offset + pixel_length;
    receipt.trailing_length = NEXUS_V1_WARNING_DGT2_TRAILING_BYTES;
    receipt.trailing_fnv1a64 = fnv1a64(source_bytes + receipt.trailing_offset,
                                       receipt.trailing_length);
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_warning_dgt2_resource_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_WarningDgt2SourceIdentity *identity,
    Nexus_V1_WarningDgt2ResourceCorpusReceipt *out_receipt)
{
    Nexus_V1_WarningDgt2ResourceCorpusReceipt receipt;
    uint32_t index;
    uint64_t chain_end;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    for (index = 0U; index < NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT; ++index) {
        if (!nexus_v1_warning_dgt2_resource_admit(
                source_bytes, source_size, identity, index,
                &receipt.resources[index])) {
            memset(&receipt, 0, sizeof(receipt));
            *out_receipt = receipt;
            return 0;
        }
        if (index > 0U &&
            receipt.resources[index].descriptor_offset !=
                receipt.resources[index - 1U].descriptor_offset +
                    receipt.resources[index - 1U].resource_length) {
            memset(&receipt, 0, sizeof(receipt));
            *out_receipt = receipt;
            return 0;
        }
    }
    receipt.chain_offset = receipt.resources[0].descriptor_offset;
    receipt.chain_length = 0U;
    for (index = 0U; index < NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT; ++index) {
        receipt.chain_length += receipt.resources[index].resource_length;
    }
    chain_end = (uint64_t)receipt.chain_offset + (uint64_t)receipt.chain_length;
    if (chain_end > (uint64_t)source_size) {
        memset(&receipt, 0, sizeof(receipt));
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.all_resources_bound = 1;
    receipt.contiguous_chain_observed = 1;
    receipt.chain_covers_source_tail = chain_end == (uint64_t)source_size;
    receipt.source_fnv1a64 = receipt.resources[0].source_fnv1a64;
    receipt.resource_count = NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT;
    receipt.chain_fnv1a64 =
        fnv1a64(source_bytes + receipt.chain_offset, receipt.chain_length);
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_warning_dgt2_resource_execute(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_WarningDgt2ResourceReceipt *resource_receipt,
    uint8_t *indexed_pixels_out,
    size_t indexed_pixels_out_size,
    uint16_t bgr555_words_out[256],
    Nexus_V1_WarningDgt2ResourcePresentFn present,
    void *present_context,
    Nexus_V1_WarningDgt2ResourceExecutionReceipt *out_receipt)
{
    Nexus_V1_WarningDgt2ResourceExecutionReceipt receipt;
    uint32_t i;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!source_bytes || !resource_receipt || !indexed_pixels_out ||
        !bgr555_words_out || !present || !resource_receipt->valid ||
        !resource_receipt->source_identity_bound ||
        !resource_receipt->res_directory_bound ||
        !resource_receipt->descriptor_bound ||
        !resource_receipt->dgt2_header_bound ||
        !resource_receipt->pp_header_bound || !resource_receipt->payload_bound ||
        resource_receipt->pixel_decode_permitted ||
        resource_receipt->draw_permitted ||
        resource_receipt->descriptor_index >=
            NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT ||
        source_size != NEXUS_V1_WARNING_BIN_BYTES ||
        fnv1a64(source_bytes, source_size) != resource_receipt->source_fnv1a64 ||
        resource_receipt->descriptor_offset !=
            k_resource_offsets[resource_receipt->descriptor_index] ||
        resource_receipt->resource_length !=
            k_resource_lengths[resource_receipt->descriptor_index] ||
        resource_receipt->pp_width !=
            k_resource_widths[resource_receipt->descriptor_index] ||
        resource_receipt->pp_height !=
            k_resource_heights[resource_receipt->descriptor_index] ||
        resource_receipt->pixel_length !=
            (uint32_t)resource_receipt->pp_width *
                (uint32_t)resource_receipt->pp_height ||
        indexed_pixels_out_size != resource_receipt->pixel_length ||
        resource_receipt->clut_offset > source_size ||
        resource_receipt->clut_length > source_size - resource_receipt->clut_offset ||
        resource_receipt->pixel_offset > source_size ||
        resource_receipt->pixel_length >
            source_size - resource_receipt->pixel_offset ||
        resource_receipt->trailing_offset > source_size ||
        resource_receipt->trailing_length >
            source_size - resource_receipt->trailing_offset ||
        fnv1a64(source_bytes + resource_receipt->descriptor_offset,
                 resource_receipt->resource_length) !=
            resource_receipt->resource_fnv1a64 ||
        memcmp(source_bytes + resource_receipt->pp_header_offset, "PP", 2) != 0 ||
        be16(source_bytes + resource_receipt->pp_header_offset + 2U) !=
            resource_receipt->pp_width ||
        be16(source_bytes + resource_receipt->pp_header_offset + 4U) !=
            resource_receipt->pp_height ||
        fnv1a64(source_bytes + resource_receipt->pp_header_offset, 6U) !=
            resource_receipt->pp_header_fnv1a64 ||
        fnv1a64(source_bytes + resource_receipt->clut_offset,
                 resource_receipt->clut_length) !=
            resource_receipt->clut_fnv1a64 ||
        fnv1a64(source_bytes + resource_receipt->pixel_offset,
                 resource_receipt->pixel_length) !=
            resource_receipt->pixel_fnv1a64 ||
        fnv1a64(source_bytes + resource_receipt->trailing_offset,
                 resource_receipt->trailing_length) !=
            resource_receipt->trailing_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }

    for (i = 0U; i < 256U; ++i) {
        bgr555_words_out[i] =
            be16(source_bytes + resource_receipt->clut_offset + (size_t)i * 2U);
    }
    memcpy(indexed_pixels_out, source_bytes + resource_receipt->pixel_offset,
           resource_receipt->pixel_length);
    if (!present(present_context, indexed_pixels_out, resource_receipt->pp_width,
                 resource_receipt->pp_height, resource_receipt->pp_width,
                 bgr555_words_out, 256U)) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.valid = receipt.resource_receipt_bound = receipt.pp_256_indexed_proven =
        receipt.bgr555_clut_proven = receipt.stride_proven =
        receipt.index_plane_copied = receipt.presented = 1;
    receipt.source_fnv1a64 = resource_receipt->source_fnv1a64;
    receipt.descriptor_index = resource_receipt->descriptor_index;
    receipt.width = resource_receipt->pp_width;
    receipt.height = resource_receipt->pp_height;
    receipt.stride = resource_receipt->pp_width;
    receipt.bgr555_word_count = 256U;
    receipt.index_plane_fnv1a64 = resource_receipt->pixel_fnv1a64;
    receipt.bgr555_words_fnv1a64 = resource_receipt->clut_fnv1a64;
    receipt.trailing_raw_bytes = resource_receipt->trailing_length;
    *out_receipt = receipt;
    return 1;
}

typedef struct {
    uint8_t *framebuffer;
    uint8_t (*rgb6)[3];
    uint16_t expected_width;
    uint16_t expected_height;
    int called;
} Nexus_V1_WarningResourceM11Context;

static int nexus_v1_warning_resource_m11_callback(
    void *userdata, const uint8_t *pixels, uint16_t width, uint16_t height,
    uint32_t stride, const uint16_t *bgr555_words, uint32_t word_count)
{
    Nexus_V1_WarningResourceM11Context *context = userdata;
    uint8_t rgb6[256][3];
    uint32_t entry;
    uint16_t word;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t row;

    if (!context || !context->framebuffer || !context->rgb6 || !pixels ||
        !bgr555_words || width != context->expected_width ||
        height != context->expected_height || stride != width ||
        word_count != 256U || width > NEXUS_V1_WARNING_DGT2_M11_WIDTH ||
        height > NEXUS_V1_WARNING_DGT2_M11_HEIGHT) {
        return 0;
    }
    for (entry = 0U; entry < 256U; ++entry) {
        word = bgr555_words[entry];
        /* ST-124 section 6: B4..B0, G4..G0, R4..R0, high to low. */
        red = (uint16_t)(word & 0x1fU);
        green = (uint16_t)((word >> 5) & 0x1fU);
        blue = (uint16_t)((word >> 10) & 0x1fU);
        rgb6[entry][0] = expand_5_to_6(red);
        rgb6[entry][1] = expand_5_to_6(green);
        rgb6[entry][2] = expand_5_to_6(blue);
    }
    for (row = 0U; row < height; ++row) {
        memcpy(context->framebuffer + (size_t)row * NEXUS_V1_WARNING_DGT2_M11_WIDTH,
               pixels + (size_t)row * stride, width);
    }
    memcpy(context->rgb6, rgb6, sizeof(rgb6));
    context->called = 1;
    return 1;
}

int nexus_v1_warning_dgt2_resource_m11_present(
    const uint8_t *source_bytes,
    size_t source_size,
    uint32_t descriptor_index,
    uint8_t *m11_framebuffer,
    size_t m11_framebuffer_size,
    uint8_t out_rgb6[256][3],
    Nexus_V1_WarningDgt2ResourceM11Receipt *out_receipt)
{
    Nexus_V1_WarningDgt2SourceIdentity identity;
    Nexus_V1_WarningDgt2ResourceReceipt resource;
    Nexus_V1_WarningDgt2ResourceExecutionReceipt execution;
    Nexus_V1_WarningDgt2ResourceM11Receipt receipt;
    Nexus_V1_WarningResourceM11Context context;
    uint8_t indexed_pixels[NEXUS_V1_WARNING_DGT2_MAX_PIXELS];
    uint16_t bgr555_words[256];

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!source_bytes || !m11_framebuffer || !out_rgb6 ||
        descriptor_index >= NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT ||
        m11_framebuffer_size !=
            (size_t)NEXUS_V1_WARNING_DGT2_M11_WIDTH *
                NEXUS_V1_WARNING_DGT2_M11_HEIGHT) {
        *out_receipt = receipt;
        return 0;
    }
    memset(&identity, 0, sizeof(identity));
    identity.canonical_fnv1a64_verified = 1;
    identity.source_fnv1a64 = fnv1a64(source_bytes, source_size);
    if (!nexus_v1_warning_dgt2_resource_admit(source_bytes, source_size,
            &identity, descriptor_index, &resource)) {
        *out_receipt = receipt;
        return 0;
    }
    memset(&context, 0, sizeof(context));
    context.framebuffer = m11_framebuffer;
    context.rgb6 = out_rgb6;
    context.expected_width = resource.pp_width;
    context.expected_height = resource.pp_height;
    if (!nexus_v1_warning_dgt2_resource_execute(source_bytes, source_size,
            &resource, indexed_pixels, resource.pixel_length, bgr555_words,
            nexus_v1_warning_resource_m11_callback, &context, &execution) ||
        !context.called) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.canonical_source_bound = receipt.pp_execution_bound = 1;
    receipt.host_surface_written = receipt.bgr555_to_rgb6_exact = 1;
    receipt.source_fnv1a64 = execution.source_fnv1a64;
    receipt.descriptor_index = execution.descriptor_index;
    receipt.index_plane_fnv1a64 = execution.index_plane_fnv1a64;
    receipt.bgr555_words_fnv1a64 = execution.bgr555_words_fnv1a64;
    receipt.host_palette_rgb6_fnv1a64 = fnv1a64(&out_rgb6[0][0], 256U * 3U);
    receipt.width = execution.width;
    receipt.height = execution.height;
    receipt.stride = execution.stride;
    *out_receipt = receipt;
    return 1;
}
