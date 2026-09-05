#include "nexus_v1_font256_s2d_section_corpus_receipt.h"
#include "nexus_v1_test_retail_member.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYNTH_BYTES NEXUS_V1_FONT256_S2D_BYTES
static char real_sha256[65];

static const uint32_t k_offsets[4] = { 0x0120U, 0x2130U, 0x5dc0U, 0x5fd0U };
static const uint32_t k_lengths[4] = { 0x2010U, 0x3c90U, 0x0210U, 0x01e4U };
static const uint32_t k_table_indices[4] = { 0U, 2U, 4U, 6U };

static int failures = 0;

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        ++failures; \
    } \
} while (0)

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0; index < size; ++index) { value ^= bytes[index]; value *= UINT64_C(1099511628211); }
    return value;
}

static void put_be16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8);
    p[1] = (uint8_t)(value & 0xffU);
}

static void put_be32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)((value >> 16) & 0xffU);
    p[2] = (uint8_t)((value >> 8) & 0xffU);
    p[3] = (uint8_t)(value & 0xffU);
}

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file; long length; uint8_t *bytes;
    if (strstr(path, "::")) return nexus_v1_test_read_retail_member(path,out_size,real_sha256);
    *out_size = 0; file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) { if (file) fclose(file); return NULL; }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) { free(bytes); fclose(file); return NULL; }
    fclose(file); *out_size = (size_t)length; return bytes;
}

/* Builds a synthetic SCR envelope carrying the canonical FONT256.S2D section
 * table framing. Section contents are deterministic synthetic patterns: a
 * full post-preamble BE16 ramp for ordinal 0, a flat nonzero fill for
 * ordinal 1, a bounded 10-word ramp prefix for ordinal 2, and an all-zero
 * body for ordinal 3. None of these bytes claims glyph/palette meaning. */
static uint8_t *build_synthetic(void)
{
    uint8_t *bytes = (uint8_t *)calloc(SYNTH_BYTES, 1U);
    uint32_t i;
    if (!bytes) return NULL;
    memcpy(bytes, NEXUS_V1_FONT_SCR_MAGIC, NEXUS_V1_FONT_SCR_MAGIC_SIZE);
    put_be32(bytes + 0x10U, 256U);
    put_be32(bytes + 0x14U, 0x12U);
    for (i = 0; i < 4U; ++i) {
        uint8_t *entry = bytes + NEXUS_V1_FONT_SCR_SECTION_TABLE_OFFSET +
            k_table_indices[i] * NEXUS_V1_FONT_SCR_SECTION_ENTRY_SIZE;
        put_be32(entry, k_offsets[i]);
        put_be32(entry + 4U, k_lengths[i]);
    }
    for (i = 0; i < 16U; ++i) {
        bytes[k_offsets[0] + i] = (uint8_t)(0xa0U + i);
        bytes[k_offsets[1] + i] = (uint8_t)(0xb0U + i);
        bytes[k_offsets[2] + i] = (uint8_t)(0xc0U + i);
        bytes[k_offsets[3] + i] = (uint8_t)(0xd0U + i);
    }
    for (i = 0; i < (k_lengths[0] - 16U) / 2U; ++i) {
        put_be16(bytes + k_offsets[0] + 16U + (size_t)i * 2U, (uint16_t)i);
    }
    for (i = 0; i < (k_lengths[1] - 16U) / 2U; ++i) {
        put_be16(bytes + k_offsets[1] + 16U + (size_t)i * 2U, 0xbeefU);
    }
    for (i = 0; i < (k_lengths[2] - 16U) / 2U; ++i) {
        put_be16(bytes + k_offsets[2] + 16U + (size_t)i * 2U,
                 i < 10U ? (uint16_t)i : (uint16_t)0x1111U);
    }
    /* ordinal 3 body stays zero-filled. */
    return bytes;
}

static void make_identity(const uint8_t *bytes, size_t size,
                          Nexus_V1_Font256S2DSourceIdentity *out_identity)
{
    memset(out_identity, 0, sizeof(*out_identity));
    out_identity->sha256_verified = 1;
    out_identity->sha256_hex = real_sha256[0] ? real_sha256 : NEXUS_V1_FONT256_S2D_SHA256;
    out_identity->source_fnv1a64 = fnv1a64(bytes, size);
}

static void check_corpus_common(const uint8_t *bytes, size_t size,
                                const Nexus_V1_Font256S2DAdmissionReceipt *admission,
                                int synthetic)
{
    Nexus_V1_Font256S2DSectionCorpusReceipt corpus;
    Nexus_V1_Font256S2DPopulatedSectionReceipt section;
    Nexus_V1_Font256S2DSectionCorpusSpanIterator iterator;
    Nexus_V1_Font256S2DSectionCorpusSpan span;
    uint32_t i;

    CHECK(nexus_v1_font256_s2d_section_corpus_admit(bytes, size, admission, &corpus) == 1);
    CHECK(corpus.valid && corpus.source_admission_bound && corpus.all_sections_bound);
    CHECK(corpus.capture_required == 1);
    CHECK(!corpus.glyph_layout_proven && !corpus.palette_proven &&
          !corpus.pixel_decode_permitted && !corpus.draw_permitted);
    CHECK(corpus.populated_section_count == 4U);
    CHECK(corpus.contiguous_chain_observed == 1);
    CHECK(corpus.chain_covers_source_tail == 1);
    CHECK(corpus.chain_offset == 0x120U);
    CHECK(corpus.chain_length == 24724U);
    CHECK(corpus.chain_fnv1a64 != 0U);
    CHECK(corpus.source_fnv1a64 == admission->source_fnv1a64);
    for (i = 0; i < 4U; ++i) {
        CHECK(corpus.sections[i].valid && corpus.sections[i].section_bound &&
              corpus.sections[i].preamble_bound);
        CHECK(corpus.sections[i].capture_required == 1);
        CHECK(!corpus.sections[i].glyph_layout_proven &&
              !corpus.sections[i].palette_proven &&
              !corpus.sections[i].pixel_decode_permitted &&
              !corpus.sections[i].draw_permitted);
        CHECK(corpus.sections[i].admission_ordinal == i);
        CHECK(corpus.sections[i].section_table_index == k_table_indices[i]);
        CHECK(corpus.sections[i].section_offset == k_offsets[i]);
        CHECK(corpus.sections[i].section_length == k_lengths[i]);
        CHECK(corpus.sections[i].section_fnv1a64 == admission->section_fnv1a64[i]);
        CHECK(corpus.sections[i].preamble_offset == k_offsets[i]);
        CHECK(corpus.sections[i].preamble_length == 16U);
        CHECK(corpus.sections[i].preamble_fnv1a64 != 0U);
        CHECK(corpus.sections[i].zero_byte_count + corpus.sections[i].nonzero_byte_count ==
              k_lengths[i]);
        CHECK(corpus.sections[i].post_preamble_word_count == (k_lengths[i] - 16U) / 2U);
        CHECK(corpus.sections[i].be16_ramp_prefix_word_count <=
              corpus.sections[i].post_preamble_word_count);
        CHECK(nexus_v1_font256_s2d_populated_section_receipt_admit(
                  bytes, size, admission, i, &section) == 1);
        CHECK(section.section_offset == k_offsets[i] &&
              section.section_length == k_lengths[i]);
    }

    if (synthetic) {
        CHECK(corpus.sections[0].be16_ramp_full == 1);
        CHECK(corpus.sections[0].be16_ramp_prefix_word_count == 4096U);
        CHECK(corpus.sections[1].be16_ramp_full == 0);
        CHECK(corpus.sections[1].be16_ramp_prefix_word_count == 0U);
        CHECK(corpus.sections[1].zero_byte_count == 0U);
        CHECK(corpus.sections[1].nonzero_byte_count == 0x3c90U);
        CHECK(corpus.sections[2].be16_ramp_full == 0);
        CHECK(corpus.sections[2].be16_ramp_prefix_word_count == 10U);
        CHECK(corpus.sections[3].be16_ramp_full == 0);
        CHECK(corpus.sections[3].be16_ramp_prefix_word_count == 1U);
        CHECK(corpus.sections[3].zero_byte_count == 468U);
        CHECK(corpus.sections[3].nonzero_byte_count == 16U);
    }

    CHECK(nexus_v1_font256_s2d_section_corpus_span_iterator_init(&iterator, &corpus) == 0);
    for (i = 0; i < 4U; ++i) {
        CHECK(nexus_v1_font256_s2d_section_corpus_span_iterator_next(&iterator, &span) == 1);
        CHECK(span.source_offset == k_offsets[i]);
        CHECK(span.source_length == k_lengths[i]);
        CHECK(span.source_fnv1a64 == admission->section_fnv1a64[i]);
    }
    CHECK(nexus_v1_font256_s2d_section_corpus_span_iterator_next(&iterator, &span) == 0);
    CHECK(nexus_v1_font256_s2d_section_corpus_span_iterator_next(&iterator, &span) == 0);
}

static void check_rejections(const uint8_t *bytes, size_t size,
                             const Nexus_V1_Font256S2DAdmissionReceipt *admission)
{
    Nexus_V1_Font256S2DAdmissionReceipt drifted;
    Nexus_V1_Font256S2DSectionCorpusReceipt corpus;
    Nexus_V1_Font256S2DPopulatedSectionReceipt section;
    Nexus_V1_Font256S2DSectionCorpusSpanIterator iterator;
    Nexus_V1_Font256S2DSectionCorpusSpan span;
    uint8_t *tampered;
    uint32_t i;

    CHECK(nexus_v1_font256_s2d_section_corpus_admit(NULL, size, admission, &corpus) == 0);
    CHECK(nexus_v1_font256_s2d_section_corpus_admit(bytes, size, NULL, &corpus) == 0);
    CHECK(nexus_v1_font256_s2d_section_corpus_admit(bytes, size, admission, NULL) == 0);
    CHECK(nexus_v1_font256_s2d_populated_section_receipt_admit(
              bytes, size, admission, 4U, &section) == 0);
    CHECK(nexus_v1_font256_s2d_populated_section_receipt_admit(
              NULL, size, admission, 0U, &section) == 0);
    CHECK(nexus_v1_font256_s2d_populated_section_receipt_admit(
              bytes, size, NULL, 0U, &section) == 0);
    CHECK(nexus_v1_font256_s2d_populated_section_receipt_admit(
              bytes, size, admission, 0U, NULL) == 0);
    CHECK(nexus_v1_font256_s2d_section_corpus_span_iterator_init(NULL, &corpus) == -1);
    CHECK(nexus_v1_font256_s2d_section_corpus_span_iterator_init(&iterator, NULL) == -1);
    memset(&corpus, 0, sizeof(corpus));
    CHECK(nexus_v1_font256_s2d_section_corpus_span_iterator_init(&iterator, &corpus) == -1);
    CHECK(nexus_v1_font256_s2d_section_corpus_span_iterator_next(NULL, &span) == -1);
    CHECK(nexus_v1_font256_s2d_section_corpus_span_iterator_next(&iterator, NULL) == -1);

    drifted = *admission;
    drifted.section_count = 3U;
    CHECK(nexus_v1_font256_s2d_section_corpus_admit(bytes, size, &drifted, &corpus) == 0);
    CHECK(nexus_v1_font256_s2d_populated_section_receipt_admit(
              bytes, size, &drifted, 0U, &section) == 0);
    drifted = *admission;
    drifted.draw_permitted = 1;
    CHECK(nexus_v1_font256_s2d_section_corpus_admit(bytes, size, &drifted, &corpus) == 0);
    drifted = *admission;
    drifted.section_fnv1a64[1] ^= 1U;
    CHECK(nexus_v1_font256_s2d_section_corpus_admit(bytes, size, &drifted, &corpus) == 0);
    CHECK(nexus_v1_font256_s2d_populated_section_receipt_admit(
              bytes, size, &drifted, 1U, &section) == 0);
    drifted = *admission;
    drifted.section_table_fnv1a64 ^= 1U;
    CHECK(nexus_v1_font256_s2d_section_corpus_admit(bytes, size, &drifted, &corpus) == 0);
    drifted = *admission;
    drifted.sections[2].file_offset += 2U;
    CHECK(nexus_v1_font256_s2d_section_corpus_admit(bytes, size, &drifted, &corpus) == 0);

    tampered = (uint8_t *)malloc(size);
    CHECK(tampered != NULL);
    if (!tampered) return;
    for (i = 0; i < 4U; ++i) {
        memcpy(tampered, bytes, size);
        tampered[k_offsets[i] + k_lengths[i] / 2U] ^= 1U;
        CHECK(nexus_v1_font256_s2d_section_corpus_admit(
                  tampered, size, admission, &corpus) == 0);
        CHECK(nexus_v1_font256_s2d_populated_section_receipt_admit(
                  tampered, size, admission, i, &section) == 0);
        memcpy(tampered, bytes, size);
        tampered[k_offsets[i]] ^= 1U;
        CHECK(nexus_v1_font256_s2d_section_corpus_admit(
                  tampered, size, admission, &corpus) == 0);
    }
    memcpy(tampered, bytes, size);
    tampered[NEXUS_V1_FONT_SCR_SECTION_TABLE_OFFSET] ^= 1U;
    CHECK(nexus_v1_font256_s2d_section_corpus_admit(
              tampered, size, admission, &corpus) == 0);
    free(tampered);
}

static int run_real_mode(const char *path)
{
    uint8_t *bytes;
    size_t size;
    Nexus_V1_Font256S2DSourceIdentity identity;
    Nexus_V1_Font256S2DAdmissionReceipt admission;

    bytes = read_file(path, &size);
    if (!bytes) return 77;
    CHECK(size == SYNTH_BYTES);
    if (size != SYNTH_BYTES) { free(bytes); return 1; }
    make_identity(bytes, size, &identity);
    CHECK(nexus_v1_font256_s2d_admit(bytes, size, &identity, &admission) == 1);
    if (!admission.valid) { free(bytes); return 1; }
    check_corpus_common(bytes, size, &admission, 0);
    check_rejections(bytes, size, &admission);
    free(bytes);
    if (failures) return 1;
    puts("FONT256.S2D section corpus receipt (real): PASS");
    return 0;
}

static int run_synthetic_mode(void)
{
    uint8_t *bytes;
    Nexus_V1_Font256S2DSourceIdentity identity;
    Nexus_V1_Font256S2DAdmissionReceipt admission;

    bytes = build_synthetic();
    CHECK(bytes != NULL);
    if (!bytes) return 1;
    make_identity(bytes, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_font256_s2d_admit(bytes, SYNTH_BYTES, &identity, &admission) == 1);
    CHECK(admission.valid && admission.section_count == 4U);
    if (!admission.valid) { free(bytes); return 1; }
    check_corpus_common(bytes, SYNTH_BYTES, &admission, 1);
    check_rejections(bytes, SYNTH_BYTES, &admission);
    free(bytes);
    if (failures) return 1;
    puts("FONT256.S2D section corpus receipt (synthetic): PASS");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2) {
        return run_real_mode(argv[1]);
    }
    return run_synthetic_mode();
}
