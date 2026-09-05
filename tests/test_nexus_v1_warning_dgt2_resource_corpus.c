#include "nexus_v1_warning_dgt2_resource_corpus.h"
#include "nexus_v1_test_retail_member.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint32_t k_offsets[4] = { 0x48U, 0x5c58U, 0xb868U, 0xf8f8U };
static const uint32_t k_lengths[4] = { 0x5c10U, 0x5c10U, 0x4090U, 0x9290U };
static const uint16_t k_widths[4] = { 240U, 240U, 200U, 272U };
static const uint16_t k_heights[4] = { 96U, 96U, 80U, 136U };

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0; index < size; ++index) { value ^= bytes[index]; value *= UINT64_C(1099511628211); }
    return value;
}

static uint8_t *read_file(const char *path, size_t *size)
{
    FILE *file;
    long length;
    uint8_t *bytes;
    char hash[65];
    if (strstr(path, "::"))
        return nexus_v1_test_read_retail_member(path, size, hash);
    *size = 0U;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET)) { if (file) fclose(file); return NULL; }
    bytes = malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) {
        free(bytes); fclose(file); return NULL;
    }
    fclose(file);
    *size = (size_t)length;
    return bytes;
}

static uint8_t expand_5_to_6(uint16_t value)
{
    return (uint8_t)((value << 1) | (value >> 4));
}

typedef struct {
    int calls;
    int fail;
    uint16_t expected_width;
    uint16_t expected_height;
    const uint8_t *expected_pixels;
    const uint8_t *expected_clut;
} PresentState;

static int counting_present(void *userdata, const uint8_t *pixels,
    uint16_t width, uint16_t height, uint32_t stride,
    const uint16_t *bgr555_words, uint32_t word_count)
{
    PresentState *state = userdata;
    uint32_t i;
    if (!state || state->fail) return 0;
    ++state->calls;
    if (!pixels || !bgr555_words || width != state->expected_width ||
        height != state->expected_height || stride != width ||
        word_count != 256U) return 0;
    if (state->expected_pixels &&
        memcmp(pixels, state->expected_pixels,
               (size_t)width * (size_t)height) != 0) return 0;
    if (state->expected_clut) {
        for (i = 0U; i < 256U; ++i) {
            uint16_t word = (uint16_t)((uint16_t)state->expected_clut[i * 2U] << 8 |
                                       state->expected_clut[i * 2U + 1U]);
            if (bgr555_words[i] != word) return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv)
{
    const char *path = argc == 2 ? argv[1] : getenv("FIRESTAFF_NEXUS_WARNING_PATH");
    Nexus_V1_WarningDgt2SourceIdentity identity;
    Nexus_V1_WarningDgt2ResourceCorpusReceipt corpus;
    Nexus_V1_WarningDgt2ResourceReceipt resource;
    Nexus_V1_WarningDgt2ResourceExecutionReceipt execution;
    Nexus_V1_WarningDgt2ResourceM11Receipt m11;
    PresentState present;
    uint8_t *bytes;
    uint8_t *tampered;
    uint8_t framebuffer[320U * 200U];
    uint8_t rgb6[256][3];
    uint8_t indexed[NEXUS_V1_WARNING_DGT2_MAX_PIXELS];
    uint16_t words[256];
    size_t size;
    uint32_t index;
    uint32_t entry;
    uint16_t row;
    uint8_t old;

    if (!path || !*path || !(bytes = read_file(path, &size))) return 77;
    if (size != NEXUS_V1_WARNING_BIN_BYTES) { free(bytes); return 1; }

    memset(&identity, 0, sizeof(identity));
    identity.sha256_verified = 1;
    identity.sha256_hex = NEXUS_V1_WARNING_BIN_SHA256;
    identity.source_fnv1a64 = fnv1a64(bytes, size);

    /* Corpus admission binds all four canonical resources and their chain. */
    if (!nexus_v1_warning_dgt2_resource_corpus_admit(bytes, size, &identity, &corpus) ||
        !corpus.valid || !corpus.all_resources_bound ||
        !corpus.contiguous_chain_observed || !corpus.chain_covers_source_tail ||
        corpus.pixel_decode_permitted || corpus.draw_permitted ||
        corpus.resource_count != 4U || corpus.chain_offset != 0x48U ||
        corpus.chain_length != size - 0x48U || !corpus.chain_fnv1a64 ||
        corpus.source_fnv1a64 != identity.source_fnv1a64) { free(bytes); return 1; }

    for (index = 0U; index < 4U; ++index) {
        const Nexus_V1_WarningDgt2ResourceReceipt *r = &corpus.resources[index];
        if (!r->valid || !r->source_identity_bound || !r->res_directory_bound ||
            !r->descriptor_bound || !r->dgt2_header_bound || !r->pp_header_bound ||
            !r->payload_bound || r->pixel_decode_permitted || r->draw_permitted ||
            r->descriptor_index != index || r->descriptor_id != index ||
            r->descriptor_offset != k_offsets[index] ||
            r->resource_length != k_lengths[index] || !r->resource_fnv1a64 ||
            r->pp_header_offset != k_offsets[index] + 8U ||
            r->pp_width != k_widths[index] || r->pp_height != k_heights[index] ||
            !r->pp_header_fnv1a64 ||
            r->clut_offset != k_offsets[index] + 14U || r->clut_length != 512U ||
            !r->clut_fnv1a64 ||
            r->pixel_offset != k_offsets[index] + 526U ||
            r->pixel_length !=
                (uint32_t)k_widths[index] * (uint32_t)k_heights[index] ||
            !r->pixel_fnv1a64 ||
            r->trailing_offset != r->pixel_offset + r->pixel_length ||
            r->trailing_length != 2U || !r->trailing_fnv1a64) { free(bytes); return 1; }
        if (index > 0U &&
            r->descriptor_offset != corpus.resources[index - 1U].descriptor_offset +
                corpus.resources[index - 1U].resource_length) { free(bytes); return 1; }

        /* Per-resource admission agrees with the corpus receipt. */
        if (!nexus_v1_warning_dgt2_resource_admit(bytes, size, &identity, index,
                &resource) ||
            resource.descriptor_offset != r->descriptor_offset ||
            resource.resource_fnv1a64 != r->resource_fnv1a64 ||
            resource.pixel_fnv1a64 != r->pixel_fnv1a64) { free(bytes); return 1; }

        /* Execution copies the exact index plane and original BGR555 words. */
        memset(&present, 0, sizeof(present));
        present.expected_width = k_widths[index];
        present.expected_height = k_heights[index];
        present.expected_pixels = bytes + r->pixel_offset;
        present.expected_clut = bytes + r->clut_offset;
        if (!nexus_v1_warning_dgt2_resource_execute(bytes, size, r, indexed,
                r->pixel_length, words, counting_present, &present, &execution) ||
            present.calls != 1 || !execution.valid || !execution.presented ||
            execution.fallback_permitted ||
            execution.descriptor_index != index ||
            execution.width != k_widths[index] ||
            execution.height != k_heights[index] ||
            execution.stride != k_widths[index] ||
            execution.bgr555_word_count != 256U ||
            execution.index_plane_fnv1a64 != r->pixel_fnv1a64 ||
            execution.bgr555_words_fnv1a64 != r->clut_fnv1a64 ||
            execution.trailing_raw_bytes != 2U ||
            memcmp(indexed, bytes + r->pixel_offset, r->pixel_length) != 0) {
            free(bytes); return 1;
        }
        for (entry = 0U; entry < 256U; ++entry) {
            uint16_t word = (uint16_t)((uint16_t)bytes[r->clut_offset + entry * 2U] << 8 |
                                       bytes[r->clut_offset + entry * 2U + 1U]);
            if (words[entry] != word) { free(bytes); return 1; }
        }

        /* M11 presentation writes the exact top-left plane and RGB6 palette. */
        memset(framebuffer, 0xcc, sizeof(framebuffer));
        memset(rgb6, 0, sizeof(rgb6));
        if (!nexus_v1_warning_dgt2_resource_m11_present(bytes, size, index,
                framebuffer, sizeof(framebuffer), rgb6, &m11) ||
            !m11.valid || !m11.host_surface_written || !m11.bgr555_to_rgb6_exact ||
            m11.fallback_permitted || m11.descriptor_index != index ||
            m11.width != k_widths[index] || m11.height != k_heights[index] ||
            m11.stride != k_widths[index] ||
            m11.index_plane_fnv1a64 != r->pixel_fnv1a64 ||
            !m11.host_palette_rgb6_fnv1a64) { free(bytes); return 1; }
        for (row = 0U; row < k_heights[index]; ++row) {
            if (memcmp(framebuffer + (size_t)row * 320U,
                       bytes + r->pixel_offset + (size_t)row * k_widths[index],
                       k_widths[index]) != 0) { free(bytes); return 1; }
            if (k_widths[index] < 320U &&
                framebuffer[(size_t)row * 320U + k_widths[index]] != 0xccU) {
                free(bytes); return 1;
            }
        }
        if (k_heights[index] < 200U &&
            framebuffer[(size_t)k_heights[index] * 320U] != 0xccU) {
            free(bytes); return 1;
        }
        for (entry = 0U; entry < 256U; ++entry) {
            uint16_t word = (uint16_t)((uint16_t)bytes[r->clut_offset + entry * 2U] << 8 |
                                       bytes[r->clut_offset + entry * 2U + 1U]);
            if (rgb6[entry][0] != expand_5_to_6((uint16_t)(word & 0x1fU)) ||
                rgb6[entry][1] != expand_5_to_6((uint16_t)((word >> 5) & 0x1fU)) ||
                rgb6[entry][2] != expand_5_to_6((uint16_t)((word >> 10) & 0x1fU))) {
                free(bytes); return 1;
            }
        }
    }

    /* Rejection matrix: drift anywhere in the chain must fail closed. */
    if (nexus_v1_warning_dgt2_resource_corpus_admit(NULL, size, &identity, &corpus) ||
        nexus_v1_warning_dgt2_resource_corpus_admit(bytes, size, NULL, &corpus) ||
        nexus_v1_warning_dgt2_resource_corpus_admit(bytes, size, &identity, NULL) ||
        nexus_v1_warning_dgt2_resource_admit(bytes, size, &identity, 4U, &resource) ||
        nexus_v1_warning_dgt2_resource_admit(bytes, size, NULL, 0U, &resource) ||
        nexus_v1_warning_dgt2_resource_admit(bytes, size, &identity, 0U, NULL) ||
        nexus_v1_warning_dgt2_resource_execute(bytes, size, NULL, indexed,
            sizeof(indexed), words, counting_present, &present, &execution) ||
        nexus_v1_warning_dgt2_resource_execute(bytes, size,
            &corpus.resources[0], indexed, 4U, words, counting_present, &present,
            &execution) ||
        nexus_v1_warning_dgt2_resource_execute(bytes, size,
            &corpus.resources[1], NULL, sizeof(indexed), words, counting_present,
            &present, &execution) ||
        nexus_v1_warning_dgt2_resource_m11_present(bytes, size, 4U, framebuffer,
            sizeof(framebuffer), rgb6, &m11) ||
        nexus_v1_warning_dgt2_resource_m11_present(bytes, size, 0U, framebuffer,
            sizeof(framebuffer) - 1U, rgb6, &m11) ||
        nexus_v1_warning_dgt2_resource_m11_present(bytes, size, 0U, NULL,
            sizeof(framebuffer), rgb6, &m11)) { free(bytes); return 1; }

    memset(&present, 0, sizeof(present));
    present.fail = 1;
    if (nexus_v1_warning_dgt2_resource_execute(bytes, size, &corpus.resources[2],
            indexed, corpus.resources[2].pixel_length, words, counting_present,
            &present, &execution)) { free(bytes); return 1; }

    tampered = malloc(size);
    if (!tampered) { free(bytes); return 1; }
    for (index = 0U; index < 4U; ++index) {
        memcpy(tampered, bytes, size);
        tampered[k_offsets[index] + 526U] ^= 1U;
        if (nexus_v1_warning_dgt2_resource_corpus_admit(tampered, size, &identity,
                &corpus)) { free(tampered); free(bytes); return 1; }
        memcpy(tampered, bytes, size);
        tampered[k_offsets[index] + 14U] ^= 1U;
        if (nexus_v1_warning_dgt2_resource_admit(tampered, size, &identity, index,
                &resource)) { free(tampered); free(bytes); return 1; }
        memcpy(tampered, bytes, size);
        tampered[k_offsets[index] + 9U] ^= 1U;
        if (nexus_v1_warning_dgt2_resource_admit(tampered, size, &identity, index,
                &resource)) { free(tampered); free(bytes); return 1; }
    }
    memcpy(tampered, bytes, size);
    tampered[12U] ^= 1U;
    if (nexus_v1_warning_dgt2_resource_corpus_admit(tampered, size, &identity,
            &corpus)) { free(tampered); free(bytes); return 1; }

    /* A stale receipt must not execute against drifted bytes. */
    if (!nexus_v1_warning_dgt2_resource_admit(bytes, size, &identity, 3U,
            &resource)) { free(tampered); free(bytes); return 1; }
    memcpy(tampered, bytes, size);
    old = tampered[resource.pixel_offset];
    tampered[resource.pixel_offset] ^= 1U;
    memset(&present, 0, sizeof(present));
    present.expected_width = k_widths[3];
    present.expected_height = k_heights[3];
    if (nexus_v1_warning_dgt2_resource_execute(tampered, size, &resource, indexed,
            resource.pixel_length, words, counting_present, &present, &execution)) {
        free(tampered); free(bytes); return 1;
    }
    tampered[resource.pixel_offset] = old;

    /* Identity drift is rejected. */
    identity.source_fnv1a64 ^= 1U;
    if (nexus_v1_warning_dgt2_resource_corpus_admit(bytes, size, &identity,
            &corpus)) { free(tampered); free(bytes); return 1; }
    free(tampered);
    free(bytes);
    puts("WARNING.BIN DGT2 resource corpus: PASS");
    return 0;
}
