#include "nexus_v1_font256_s2d_subrecord_grammar.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYNTH_BYTES NEXUS_V1_FONT256_S2D_BYTES

static const uint32_t k_offsets[4] = { 0x0120U, 0x2130U, 0x5dc0U, 0x5fd0U };
static const uint32_t k_lengths[4] = { 0x2010U, 0x3c90U, 0x0210U, 0x01e4U };
static const uint32_t k_table_indices[4] = { 0U, 2U, 4U, 6U };

static const uint16_t k_preamble[NEXUS_V1_FONT256_S2D_SECTION0_PREAMBLE_WORDS] = {
    0x0010U, 0x0000U, 0x4000U, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU
};
static const uint16_t k_record0[NEXUS_V1_FONT256_S2D_SECTION4_RECORD_WORDS] = {
    0x0000U, 0x0100U, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU
};
static const uint16_t k_record1[NEXUS_V1_FONT256_S2D_SECTION4_RECORD_WORDS] = {
    0x8000U, 0x8000U, 0x8000U, 0xfffeU, 0x8000U, 0x8000U, 0x8000U, 0x8000U
};
static const uint16_t k_record2[NEXUS_V1_FONT256_S2D_SECTION4_RECORD_WORDS] = {
    0x8000U, 0x8000U, 0x8000U, 0x8000U, 0x8000U, 0x8000U, 0x8000U, 0xfc20U
};

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
    *out_size = 0; file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) { if (file) fclose(file); return NULL; }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) { free(bytes); fclose(file); return NULL; }
    fclose(file); *out_size = (size_t)length; return bytes;
}

static void make_identity(const uint8_t *bytes, size_t size,
                          Nexus_V1_Font256S2DSourceIdentity *out_identity)
{
    memset(out_identity, 0, sizeof(*out_identity));
    out_identity->sha256_verified = 1;
    out_identity->sha256_hex = NEXUS_V1_FONT256_S2D_SHA256;
    out_identity->source_fnv1a64 = fnv1a64(bytes, size);
}

static void make_real_identity(const uint8_t *bytes, size_t size,
                               Nexus_V1_Font256S2DSourceIdentity *out_identity)
{
    const char *sha256 = getenv("FIRESTAFF_NEXUS_FONT256_SHA256");
    make_identity(bytes, size, out_identity);
    if (sha256 && strcmp(sha256, NEXUS_V1_FONT256_S2D_SHA256_ENGLISH) == 0)
        out_identity->sha256_hex = NEXUS_V1_FONT256_S2D_SHA256_ENGLISH;
}

static void put_record(uint8_t *record, const uint16_t *words)
{
    uint32_t index;
    for (index = 0U; index < NEXUS_V1_FONT256_S2D_SECTION4_RECORD_WORDS;
            ++index) {
        put_be16(record + index * 2U, words[index]);
    }
}

/* Builds a synthetic SCR envelope carrying the canonical FONT256.S2D
 * section table framing plus the observed subrecord contents: the
 * canonical preamble and dual step-2 ramp for ordinal 0, the canonical
 * opaque composition inventory (52 populated runs, byte alphabet
 * {0x00,0x03,0x0f,0xff}, all-0xff lead block) for ordinal 1, the 33
 * canonical records for ordinal 2, and an all-zero body for ordinal 3.
 * None of these bytes claims glyph/palette meaning. */
static uint8_t *build_synthetic(void)
{
    uint8_t *bytes = (uint8_t *)calloc(SYNTH_BYTES, 1U);
    uint32_t i;
    if (!bytes) return NULL;
    memcpy(bytes, NEXUS_V1_FONT_SCR_MAGIC, NEXUS_V1_FONT_SCR_MAGIC_SIZE);
    put_be32(bytes + 0x10U, 256U);
    put_be32(bytes + 0x14U, 0x12U);
    for (i = 0U; i < 4U; ++i) {
        uint8_t *entry = bytes + NEXUS_V1_FONT_SCR_SECTION_TABLE_OFFSET +
            k_table_indices[i] * NEXUS_V1_FONT_SCR_SECTION_ENTRY_SIZE;
        put_be32(entry, k_offsets[i]);
        put_be32(entry + 4U, k_lengths[i]);
    }

    /* Ordinal 0: canonical preamble words + dual step-2 half ramps. */
    for (i = 0U; i < NEXUS_V1_FONT256_S2D_SECTION0_PREAMBLE_WORDS; ++i) {
        put_be16(bytes + k_offsets[0] + i * 2U, k_preamble[i]);
    }
    for (i = 0U; i < NEXUS_V1_FONT256_S2D_SECTION0_RAMP_WORDS; ++i) {
        put_be16(bytes + k_offsets[0] + 16U + (size_t)i * 2U,
            (uint16_t)(2U *
                (i & (NEXUS_V1_FONT256_S2D_SECTION0_RAMP_HALF_WORDS - 1U))));
    }

    /* Ordinal 1: the canonical opaque composition inventory — 742
     * populated of 969 16-byte blocks in exactly 52 runs from block 0
     * through block 968; the lead block alone is all 0xff; the other
     * populated blocks carry exactly 2730 bytes of 0x03 then 1453 bytes
     * of 0x0f (each nonzero byte below 0x10). */
    {
        uint32_t run;
        uint32_t block_cursor = 0U;
        uint32_t filled_blocks = 0U;
        uint32_t b03_left = NEXUS_V1_FONT256_S2D_SECTION2_BYTE_03_COUNT;
        uint32_t b0f_left = NEXUS_V1_FONT256_S2D_SECTION2_BYTE_0F_COUNT;
        for (run = 0U;
                run < NEXUS_V1_FONT256_S2D_SECTION2_POPULATED_RUN_COUNT;
                ++run) {
            uint32_t run_length =
                (run == 0U) ? 1U : (14U + ((run - 1U) < 27U ? 1U : 0U));
            uint32_t j;
            for (j = 0U; j < run_length; ++j) {
                uint8_t *blk = bytes + k_offsets[1] +
                    (size_t)block_cursor *
                        NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES;
                if (block_cursor == 0U) {
                    memset(blk, 0xff,
                        NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES);
                } else {
                    uint32_t nonzero = 5U + (filled_blocks < 478U ? 1U : 0U);
                    uint32_t k;
                    for (k = 1U; k <= nonzero; ++k) {
                        if (b03_left != 0U) {
                            blk[k] = 0x03U;
                            --b03_left;
                        } else if (b0f_left != 0U) {
                            blk[k] = 0x0fU;
                            --b0f_left;
                        }
                    }
                    ++filled_blocks;
                }
                ++block_cursor;
            }
            if (run + 1U <
                    NEXUS_V1_FONT256_S2D_SECTION2_POPULATED_RUN_COUNT) {
                block_cursor += (run < 23U) ? 5U : 4U;
            }
        }
        if (block_cursor != NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_COUNT ||
                b03_left != 0U || b0f_left != 0U) {
            free(bytes);
            return NULL;
        }
    }

    /* Ordinal 2: the 33 canonical records. */
    put_record(bytes + k_offsets[2], k_record0);
    put_record(bytes + k_offsets[2] + NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES,
        k_record1);
    put_record(bytes + k_offsets[2] +
        2U * NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES, k_record2);
    for (i = 3U; i < NEXUS_V1_FONT256_S2D_SECTION4_RECORD_COUNT; ++i) {
        uint32_t word;
        for (word = 0U; word < NEXUS_V1_FONT256_S2D_SECTION4_RECORD_WORDS;
                ++word) {
            put_be16(bytes + k_offsets[2] +
                i * NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES + word * 2U,
                NEXUS_V1_FONT256_S2D_SECTION4_BASE_WORD);
        }
    }

    /* Ordinal 3 body stays zero-filled. */
    return bytes;
}

static void check_corpus_common(const uint8_t *bytes, size_t size,
                                const Nexus_V1_Font256S2DAdmissionReceipt *admission,
                                int english_revision)
{
    Nexus_V1_Font256S2DSubrecordCorpusReceipt corpus;
    Nexus_V1_Font256S2DSubrecordReceipt section;
    Nexus_V1_Font256S2DSubrecordSpanIterator iterator;
    Nexus_V1_Font256S2DSubrecordSpan span;
    uint32_t i;
    uint32_t span_total = 0U;
    uint32_t span_count = 0U;

    CHECK(nexus_v1_font256_s2d_subrecord_corpus_admit(
              bytes, size, admission, &corpus) == 1);
    CHECK(corpus.valid && corpus.source_admission_bound &&
          corpus.all_sections_bound);
    CHECK(corpus.section0_grammar_bound == 1);
    CHECK(corpus.section2_grammar_negative == 1);
    CHECK(corpus.section2_composition_bound == 1);
    CHECK(corpus.section4_grammar_bound == 1);
    CHECK(corpus.section6_zero_bound == 1);
    CHECK(corpus.capture_required == 1);
    CHECK(!corpus.glyph_layout_proven && !corpus.palette_proven &&
          !corpus.pixel_decode_permitted && !corpus.draw_permitted);
    CHECK(corpus.populated_section_count == 4U);
    CHECK(corpus.source_fnv1a64 == admission->source_fnv1a64);

    CHECK(corpus.sections[0].subrecord_grammar_bound == 1);
    CHECK(corpus.sections[0].preamble_word_count ==
          NEXUS_V1_FONT256_S2D_SECTION0_PREAMBLE_WORDS);
    CHECK(corpus.sections[0].ramp_word_count ==
          NEXUS_V1_FONT256_S2D_SECTION0_RAMP_WORDS);
    CHECK(corpus.sections[0].ramp_half_word_count ==
          NEXUS_V1_FONT256_S2D_SECTION0_RAMP_HALF_WORDS);
    CHECK(corpus.sections[1].subrecord_grammar_bound == 0);
    CHECK(corpus.sections[1].block_count ==
          NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_COUNT);
    CHECK(corpus.sections[1].populated_block_count == (english_revision ?
          NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_POPULATED_BLOCK_COUNT :
          NEXUS_V1_FONT256_S2D_SECTION2_POPULATED_BLOCK_COUNT));
    CHECK(corpus.sections[1].populated_run_count == (english_revision ?
          NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_POPULATED_RUN_COUNT :
          NEXUS_V1_FONT256_S2D_SECTION2_POPULATED_RUN_COUNT));
    CHECK(corpus.sections[1].first_populated_block ==
          NEXUS_V1_FONT256_S2D_SECTION2_FIRST_POPULATED_BLOCK);
    CHECK(corpus.sections[1].last_populated_block ==
          NEXUS_V1_FONT256_S2D_SECTION2_LAST_POPULATED_BLOCK);
    CHECK(corpus.sections[1].byte_zero_count == (english_revision ?
          NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_BYTE_ZERO_COUNT :
          NEXUS_V1_FONT256_S2D_SECTION2_BYTE_ZERO_COUNT));
    CHECK(corpus.sections[1].byte_03_count == (english_revision ?
          NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_BYTE_03_COUNT :
          NEXUS_V1_FONT256_S2D_SECTION2_BYTE_03_COUNT));
    CHECK(corpus.sections[1].byte_0f_count == (english_revision ?
          NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_BYTE_0F_COUNT :
          NEXUS_V1_FONT256_S2D_SECTION2_BYTE_0F_COUNT));
    CHECK(corpus.sections[1].byte_ff_count == (english_revision ?
          NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_BYTE_FF_COUNT :
          NEXUS_V1_FONT256_S2D_SECTION2_BYTE_FF_COUNT));
    CHECK(corpus.sections[1].lead_block_all_ones == 1);
    CHECK(corpus.sections[1].nonlead_high_nibble_clear == 1);
    CHECK(corpus.sections[2].subrecord_grammar_bound == 1);
    CHECK(corpus.sections[2].record_count ==
          NEXUS_V1_FONT256_S2D_SECTION4_RECORD_COUNT);
    CHECK(corpus.sections[2].base_record_count ==
          NEXUS_V1_FONT256_S2D_SECTION4_BASE_RECORD_COUNT);
    CHECK(corpus.sections[3].section_all_zero == 1);

    for (i = 0U; i < 4U; ++i) {
        CHECK(nexus_v1_font256_s2d_subrecord_admit(
                  bytes, size, admission, i, &section) == 1);
        CHECK(section.valid && section.section_bound);
        CHECK(section.section_offset == k_offsets[i]);
        CHECK(section.section_length == k_lengths[i]);
        CHECK(section.section_fnv1a64 ==
              fnv1a64(bytes + k_offsets[i], k_lengths[i]));
    }

    CHECK(nexus_v1_font256_s2d_subrecord_span_iterator_init(
              &iterator, &corpus) == 1);
    while (nexus_v1_font256_s2d_subrecord_span_iterator_next(
               &iterator, &span) == 1) {
        CHECK(span.source_fnv1a64 ==
              fnv1a64(bytes + span.source_offset, span.source_length));
        span_total += span.source_length;
        ++span_count;
    }
    CHECK(span_count == NEXUS_V1_FONT256_S2D_SUBRECORD_SPAN_COUNT);
    CHECK(span_total == 24724U);
    CHECK(nexus_v1_font256_s2d_subrecord_span_iterator_next(NULL, &span) == -1);
    CHECK(nexus_v1_font256_s2d_subrecord_span_iterator_next(&iterator, NULL) == -1);
    CHECK(nexus_v1_font256_s2d_subrecord_span_iterator_init(NULL, &corpus) == 0);
    CHECK(nexus_v1_font256_s2d_subrecord_span_iterator_init(&iterator, NULL) == 0);
}

static void check_rejections(const uint8_t *bytes, size_t size,
                             const Nexus_V1_Font256S2DAdmissionReceipt *admission)
{
    Nexus_V1_Font256S2DSubrecordCorpusReceipt corpus;
    Nexus_V1_Font256S2DSubrecordReceipt section;
    Nexus_V1_Font256S2DAdmissionReceipt drifted;
    Nexus_V1_Font256S2DSourceIdentity identity;
    uint8_t *tampered;

    CHECK(nexus_v1_font256_s2d_subrecord_admit(
              NULL, size, admission, 0U, &section) == 0);
    CHECK(nexus_v1_font256_s2d_subrecord_admit(
              bytes, size, NULL, 0U, &section) == 0);
    CHECK(nexus_v1_font256_s2d_subrecord_admit(
              bytes, size, admission, 0U, NULL) == 0);
    CHECK(nexus_v1_font256_s2d_subrecord_admit(
              bytes, size, admission, 4U, &section) == 0);
    CHECK(nexus_v1_font256_s2d_subrecord_corpus_admit(
              bytes, size, NULL, &corpus) == 0);
    CHECK(nexus_v1_font256_s2d_subrecord_corpus_admit(
              bytes, size, admission, NULL) == 0);

    drifted = *admission;
    drifted.section_fnv1a64[0] ^= 1U;
    CHECK(nexus_v1_font256_s2d_subrecord_corpus_admit(
              bytes, size, &drifted, &corpus) == 0);

    tampered = (uint8_t *)malloc(size);
    CHECK(tampered != NULL);
    if (!tampered) return;

#define TAMPER_REJECT(offset, xor_value) do { \
        memcpy(tampered, bytes, size); \
        tampered[(offset)] ^= (xor_value); \
        make_identity(tampered, size, &identity); \
        { \
            Nexus_V1_Font256S2DAdmissionReceipt fresh; \
            CHECK(nexus_v1_font256_s2d_admit( \
                      tampered, size, &identity, &fresh) == 1); \
            CHECK(nexus_v1_font256_s2d_subrecord_corpus_admit( \
                      tampered, size, &fresh, &corpus) == 0); \
        } \
    } while (0)

    /* Preamble word tamper. */
    TAMPER_REJECT(k_offsets[0] + 4U, 0x01U);
    /* Ramp word tamper in both halves. */
    TAMPER_REJECT(k_offsets[0] + 16U + 10U, 0x02U);
    TAMPER_REJECT(k_offsets[0] + 16U + 2U * 2048U + 10U, 0x02U);
    /* Section 2 alphabet violation: a zero byte flipped to 0x01 leaves
     * the canonical {0x00,0x03,0x0f,0xff} alphabet. */
    TAMPER_REJECT(k_offsets[1] + 100U *
                  NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES, 0x01U);
    /* Section 2 block population drift: populate one empty gap block
     * (742 -> 743). */
    TAMPER_REJECT(k_offsets[1] + 21U *
                  NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES, 0x03U);
    /* Section 2 alphabet drift inside a populated block (0x03 -> 0x07). */
    TAMPER_REJECT(k_offsets[1] + 6U *
                  NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES + 1U, 0x04U);
    /* Section 2 byte-count drift inside the alphabet (0x03 -> 0x0f). */
    TAMPER_REJECT(k_offsets[1] + 6U *
                  NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES + 1U, 0x0cU);
    /* Section 2 lead-block tamper (0xff -> 0xfe). */
    TAMPER_REJECT(k_offsets[1], 0x01U);
    /* Section 4 canonical record and base record tamper. */
    TAMPER_REJECT(k_offsets[2] + 2U, 0x01U);
    TAMPER_REJECT(k_offsets[2] + NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES +
                  7U, 0x01U);
    TAMPER_REJECT(k_offsets[2] + 2U * NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES +
                  15U, 0x01U);
    TAMPER_REJECT(k_offsets[2] + 10U * NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES +
                  1U, 0x01U);
    /* Section 6 zero tamper. */
    TAMPER_REJECT(k_offsets[3] + 100U, 0x01U);

#undef TAMPER_REJECT

    /* Section 2 run-structure drift at constant population and constant
     * byte counts: move the whole content of run 1's last block (block
     * 20) into the following gap (block 22). Populated blocks stay 742
     * and every byte count is preserved, but the populated run count
     * moves 52 -> 53, so admission must fail. */
    memcpy(tampered, bytes, size);
    memcpy(tampered + k_offsets[1] + 22U *
               NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES,
           tampered + k_offsets[1] + 20U *
               NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES,
           NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES);
    memset(tampered + k_offsets[1] + 20U *
               NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES,
           0, NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES);
    {
        make_identity(tampered, size, &identity);
        {
            Nexus_V1_Font256S2DAdmissionReceipt fresh;
            CHECK(nexus_v1_font256_s2d_admit(
                      tampered, size, &identity, &fresh) == 1);
            CHECK(nexus_v1_font256_s2d_subrecord_corpus_admit(
                      tampered, size, &fresh, &corpus) == 0);
        }
    }

    /* Section 2 content tamper that keeps the whole composition
     * inventory (move one 0x03 byte inside its own block, from
     * position 1 to position 8) still admits; only the recorded
     * digests move. */
    memcpy(tampered, bytes, size);
    tampered[k_offsets[1] + 6U * NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES +
             1U] = 0x00U;
    tampered[k_offsets[1] + 6U * NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES +
             8U] = 0x03U;
    {
        uint64_t original_fnv;
        make_identity(tampered, size, &identity);
        {
            Nexus_V1_Font256S2DAdmissionReceipt fresh;
            CHECK(nexus_v1_font256_s2d_admit(
                      tampered, size, &identity, &fresh) == 1);
            CHECK(nexus_v1_font256_s2d_subrecord_corpus_admit(
                      tampered, size, &fresh, &corpus) == 1);
        }
        CHECK(corpus.valid && corpus.section2_grammar_negative == 1);
        CHECK(corpus.section2_composition_bound == 1);
        original_fnv = corpus.sections[1].section_fnv1a64;
        CHECK(original_fnv == fnv1a64(tampered + k_offsets[1], k_lengths[1]));
        CHECK(original_fnv != fnv1a64(bytes + k_offsets[1], k_lengths[1]));
    }
    free(tampered);
}

int main(int argc, char **argv)
{
    uint8_t *synthetic = build_synthetic();
    Nexus_V1_Font256S2DSourceIdentity identity;
    Nexus_V1_Font256S2DAdmissionReceipt admission;
    if (!synthetic) {
        fprintf(stderr, "allocation failure\n");
        return 1;
    }
    make_identity(synthetic, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_font256_s2d_admit(
              synthetic, SYNTH_BYTES, &identity, &admission) == 1);
    check_corpus_common(synthetic, SYNTH_BYTES, &admission, 0);
    check_rejections(synthetic, SYNTH_BYTES, &admission);

    if (argc > 1) {
        size_t real_size = 0;
        uint8_t *real = read_file(argv[1], &real_size);
        Nexus_V1_Font256S2DAdmissionReceipt real_admission;
        if (!real) {
            printf("SKIP: cannot read %s\n", argv[1]);
            free(synthetic);
            return 77;
        }
        CHECK(real_size == (size_t)SYNTH_BYTES);
        if (real_size == (size_t)SYNTH_BYTES) {
            make_real_identity(real, real_size, &identity);
            CHECK(nexus_v1_font256_s2d_admit(
                      real, real_size, &identity, &real_admission) == 1);
            check_corpus_common(real, real_size, &real_admission,
                identity.sha256_hex &&
                strcmp(identity.sha256_hex,
                       NEXUS_V1_FONT256_S2D_SHA256_ENGLISH) == 0);
        }
        free(real);
    }

    free(synthetic);
    if (failures) {
        fprintf(stderr, "FAILURES: %d\n", failures);
        return 1;
    }
    printf("PASS nexus_v1_font256_s2d_subrecord_grammar%s\n",
           argc > 1 ? " (real)" : " (synthetic)");
    return 0;
}
