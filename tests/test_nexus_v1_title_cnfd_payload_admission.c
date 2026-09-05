#include "nexus_v1_title_cnfd_payload_admission.h"
#include "nexus_v1_test_retail_member.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYNTH_BYTES NEXUS_V1_TITLE_BIN_BYTES

static const uint8_t k_class[NEXUS_V1_TITLE_RES_ENTRY_COUNT] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,
    2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
};

static const uint32_t k_id[NEXUS_V1_TITLE_RES_ENTRY_COUNT] = {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,
    0,1,2,3,
    0,
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
    24,25,26,27,28,29,30,31,32
};

static const uint32_t k_offsets[NEXUS_V1_TITLE_RES_ENTRY_COUNT] = {
    0x002e8U, 0x00418U, 0x00548U, 0x00678U, 0x007a8U, 0x00978U,
    0x00b48U, 0x00c98U, 0x00de8U, 0x00f38U, 0x01088U, 0x011d8U,
    0x01328U, 0x01478U, 0x015c8U, 0x01718U, 0x01868U, 0x019b8U,
    0x01b08U, 0x01c58U, 0x01da8U, 0x01ef8U, 0x02318U, 0x0a0a8U,
    0x0b438U, 0x0d068U, 0x0e278U, 0x16eecU, 0x1709cU, 0x172c4U,
    0x17390U, 0x1745cU, 0x1756cU, 0x1767cU, 0x17850U, 0x17a5cU,
    0x17c0cU, 0x17dbcU, 0x17f3cU, 0x180bcU, 0x183c4U, 0x18734U,
    0x188d0U, 0x18a6cU, 0x18ba0U, 0x18cd4U, 0x19104U, 0x195b4U,
    0x19680U, 0x1974cU, 0x1985cU, 0x1996cU, 0x19bf4U, 0x19eccU,
    0x19f98U, 0x1a064U, 0x1a174U, 0x1a284U, 0x1a3e0U, 0x1a564U
};

static const char k_magic[NEXUS_V1_TITLE_RES_ENTRY_COUNT][4] = {
    "DGT2","DGT2","DGT2","DGT2","DGT2","DGT2","DGT2","DGT2",
    "DGT2","DGT2","DGT2","DGT2","DGT2","DGT2","DGT2","DGT2",
    "DGT2","DGT2","DGT2","DGT2","DGT2","DGT2",
    "TITL","TITL","TITL","TITL",
    "MAPD",
    "CNFD","CNFD","CNFD","CNFD","CNFD","CNFD","CNFD","CNFD",
    "CNFD","CNFD","CNFD","CNFD","CNFD","CNFD","CNFD","CNFD",
    "CNFD","CNFD","CNFD","CNFD","CNFD","CNFD","CNFD","CNFD",
    "CNFD","CNFD","CNFD","CNFD","CNFD","CNFD","CNFD","CNFD","CNFD"
};

static const uint16_t k_width[33] = {
    0x30U,0x38U,0x18U,0x18U,0x20U,0x20U,0x38U,0x38U,
    0x40U,0x40U,0x38U,0x38U,0x68U,0x68U,0x38U,0x38U,
    0x28U,0x28U,0x80U,0x80U,0x18U,0x18U,0x20U,0x20U,
    0x50U,0x50U,0x18U,0x18U,0x20U,0x20U,0x28U,0x28U,0xe8U
};
static const uint16_t k_height[33] = {
    0x10U,0x12U,0x0dU,0x0dU,0x0eU,0x0eU,0x0fU,0x11U,
    0x0cU,0x0cU,0x0cU,0x0cU,0x0eU,0x10U,0x0dU,0x0dU,
    0x0dU,0x0dU,0x10U,0x12U,0x0dU,0x0dU,0x0eU,0x0eU,
    0x0fU,0x11U,0x0dU,0x0dU,0x0eU,0x0eU,0x0fU,0x11U,0x25U
};
static const uint16_t k_flag[33] = {
    0x8000U,0x8b00U,0x8b00U,0x8b00U,0x8b00U,0x8b00U,0x8000U,0x8b00U,
    0x8b00U,0x8b00U,0x8b00U,0x8b00U,0x8000U,0x8b00U,0x8b00U,0x8b00U,
    0x8b00U,0x8b00U,0x8000U,0x8b00U,0x8b00U,0x8b00U,0x8b00U,0x8b00U,
    0x8000U,0x8b00U,0x8b00U,0x8b00U,0x8b00U,0x8b00U,0x8000U,0x8b00U,
    0x8b00U
};
/* Canonical prefix-sharing classes, yielding the observed 8 distinct
 * prefixes: A={0,12,18,24,30} B={1,3,5,7,11,13,19,21,23,25,27,29,31,32}
 * C={2,4,10,20,22,26,28} D={8,16} E={9,17} F={6} G={14} H={15}. */
static const uint8_t k_prefix_class[33] = {
    0,1,2,1,2,1,5,1,3,4,2,1,0,1,6,7,3,4,0,1,2,1,2,1,0,1,2,1,2,1,0,1,1
};

static int failures = 0;
static char real_sha256[65];

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
    if (strstr(path, "::"))
        return nexus_v1_test_read_retail_member(path, out_size, real_sha256);
    *out_size = 0; file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) { if (file) fclose(file); return NULL; }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) { free(bytes); fclose(file); return NULL; }
    fclose(file); *out_size = (size_t)length; return bytes;
}

static uint32_t full_record_length(uint32_t i)
{
    return i + 1U < NEXUS_V1_TITLE_RES_ENTRY_COUNT ?
        k_offsets[i + 1U] - k_offsets[i] : (uint32_t)SYNTH_BYTES - k_offsets[i];
}

/* Builds a synthetic RES* envelope carrying the canonical TITLE.BIN
 * directory framing, with the 33 CNFD records populated in the observed
 * payload shape: canonical head words, a deterministic 32-byte prefix
 * shared per canonical class (8 distinct values), and a deterministic
 * width*height/2 packed plane. No byte claims colour, palette, or pixel
 * meaning. */
static uint8_t *build_synthetic(void)
{
    uint8_t *bytes = (uint8_t *)calloc(SYNTH_BYTES, 1U);
    uint32_t i;
    if (!bytes) return NULL;
    memcpy(bytes, "RES*", 4U);
    put_be32(bytes + 4U, (uint32_t)SYNTH_BYTES);
    put_be16(bytes + 8U, (uint16_t)NEXUS_V1_TITLE_RES_ENTRY_COUNT);
    put_be16(bytes + 10U, 0U);
    for (i = 0U; i < NEXUS_V1_TITLE_RES_ENTRY_COUNT; ++i) {
        uint8_t *entry = bytes + 12U + (size_t)i * NEXUS_V1_TITLE_RES_ENTRY_BYTES;
        memcpy(entry, k_magic[i], 4U);
        put_be32(entry + 4U, k_id[i]);
        put_be32(entry + 8U, k_offsets[i]);
    }
    for (i = 0U; i < NEXUS_V1_TITLE_RES_ENTRY_COUNT; ++i) {
        uint8_t *record = bytes + k_offsets[i];
        uint32_t j;
        uint32_t length = full_record_length(i);
        memcpy(record, k_magic[i], 4U);
        put_be32(record + 4U, k_id[i]);
        if (k_class[i] == 2U) {
            memcpy(record + 8U, "TIBG", 4U);
            put_be16(record + 12U, (uint16_t)(0x20U | i));
            put_be16(record + 14U, (uint16_t)(0x8000U + i));
        } else if (k_class[i] == 1U) {
            record[8] = record[9] = 0x50U;
            put_be16(record + 10U, (uint16_t)(i + 1U));
            put_be16(record + 12U, (uint16_t)(0x20U | i));
            put_be16(record + 14U, (uint16_t)(0x8000U + i));
        } else if (k_class[i] == 3U) {
            uint32_t t = i - 27U;
            uint32_t plane = (uint32_t)k_width[t] * k_height[t] / 2U;
            record[8] = record[9] = 0x70U;
            put_be16(record + 10U, k_width[t]);
            put_be16(record + 12U, k_height[t]);
            put_be16(record + 14U, k_flag[t]);
            for (j = 0U; j < 32U; ++j) {
                record[16U + j] = (uint8_t)(0x40U + k_prefix_class[t] * 9U + j * 5U);
            }
            for (j = 0U; j < plane; ++j) {
                record[48U + j] = (uint8_t)(0x21U + t * 3U + j * 5U);
            }
        } else {
            record[8] = record[9] = 0x70U;
            put_be16(record + 10U, (uint16_t)(i + 1U));
            put_be16(record + 12U, (uint16_t)(0x20U | i));
            put_be16(record + 14U, (uint16_t)(0x8000U + i));
        }
        if (k_class[i] != 3U) {
            for (j = 16U; j < length; ++j) {
                record[j] = (uint8_t)(0x5aU + i * 7U + j * 13U);
            }
        }
    }
    return bytes;
}

static void make_identity(const uint8_t *bytes, size_t size,
                          Nexus_V1_TitleResSourceIdentity *out_identity)
{
    memset(out_identity, 0, sizeof(*out_identity));
    out_identity->sha256_verified = 1;
    out_identity->sha256_hex = real_sha256[0] ? real_sha256 : NEXUS_V1_TITLE_BIN_SHA256;
    out_identity->source_fnv1a64 = fnv1a64(bytes, size);
}

static void check_corpus(const uint8_t *bytes, size_t size, int synthetic)
{
    Nexus_V1_TitleResSourceIdentity identity;
    Nexus_V1_TitleCnfdCorpusReceipt corpus;
    Nexus_V1_TitleCnfdPlaneSpanIterator iterator;
    Nexus_V1_TitleCnfdPlaneSpan span;
    uint32_t i;
    int rc;

    make_identity(bytes, size, &identity);
    CHECK(nexus_v1_title_cnfd_corpus_admit(bytes, size, &identity, &corpus) == 1);
    CHECK(corpus.valid && corpus.source_identity_bound &&
          corpus.res_directory_bound && corpus.all_cnfd_bound);
    CHECK(corpus.contiguous_chain_observed == 1);
    CHECK(corpus.chain_covers_source_tail == 1);
    CHECK(corpus.distinct_prefix_count == 8U);
    CHECK(!corpus.colour_proven && !corpus.palette_proven &&
          !corpus.pixel_decode_permitted && !corpus.draw_permitted &&
          !corpus.presentation_permitted);
    CHECK(corpus.cnfd_count == 33U);
    CHECK(corpus.chain_offset == 0x16eecU);
    CHECK(corpus.chain_length == 0x476cU);
    CHECK(corpus.chain_fnv1a64 != 0U);
    CHECK(corpus.source_fnv1a64 == identity.source_fnv1a64);

    for (i = 0U; i < 33U; ++i) {
        const Nexus_V1_TitleCnfdRecordReceipt *r = &corpus.records[i];
        Nexus_V1_TitleCnfdRecordReceipt single;
        CHECK(r->valid && r->cnfd_head_bound && r->pp_header_bound &&
              r->prefix_span_bound && r->plane_span_bound &&
              r->length_arithmetic_bound);
        CHECK(r->source_identity_bound && r->res_directory_bound);
        CHECK(!r->colour_proven && !r->palette_proven &&
              !r->pixel_decode_permitted && !r->draw_permitted &&
              !r->presentation_permitted);
        CHECK(r->source_fnv1a64 == identity.source_fnv1a64);
        CHECK(r->cnfd_index == i);
        CHECK(r->entry_index == 27U + i);
        CHECK(r->entry_id == i);
        CHECK(r->record_offset == k_offsets[27U + i]);
        CHECK(r->record_length == full_record_length(27U + i));
        CHECK(r->record_length ==
              16U + 32U + (uint32_t)r->width * r->height / 2U);
        CHECK(r->width == k_width[i] && r->height == k_height[i]);
        CHECK(r->flag_word == k_flag[i]);
        CHECK(r->prefix_offset == r->record_offset + 16U);
        CHECK(r->prefix_fnv1a64 != 0U);
        CHECK(r->plane_offset == r->prefix_offset + 32U);
        CHECK(r->plane_length == (uint32_t)k_width[i] * k_height[i] / 2U);
        CHECK(r->plane_fnv1a64 != 0U);
        CHECK(nexus_v1_title_cnfd_record_admit(bytes, size, &identity, i, &single) == 1);
        CHECK(single.record_offset == r->record_offset &&
              single.prefix_fnv1a64 == r->prefix_fnv1a64 &&
              single.plane_fnv1a64 == r->plane_fnv1a64);
    }

    CHECK(nexus_v1_title_cnfd_plane_span_iterator_init(&iterator, &corpus) == 0);
    for (i = 0U; i < 33U; ++i) {
        rc = nexus_v1_title_cnfd_plane_span_iterator_next(&iterator, &span);
        CHECK(rc == 1);
        if (rc == 1) {
            CHECK(span.source_offset == corpus.records[i].plane_offset);
            CHECK(span.source_length == corpus.records[i].plane_length);
            CHECK(span.source_fnv1a64 == corpus.records[i].plane_fnv1a64);
        }
    }
    CHECK(nexus_v1_title_cnfd_plane_span_iterator_next(&iterator, &span) == 0);

    if (!synthetic) {
        /* Opaque real-asset witnesses verified against the canonical
         * TITLE.BIN: per-plane nonzero byte counts. No pixel meaning. */
        static const uint32_t k_nonzero[33] = {
            158U, 282U, 112U, 112U, 154U, 154U, 161U, 295U,
            327U, 327U, 280U, 280U, 242U, 440U, 289U, 289U,
            184U, 184U, 365U, 639U, 112U, 112U, 151U, 151U,
            224U, 414U, 112U, 112U, 151U, 151U, 111U, 218U,
            3349U
        };
        for (i = 0U; i < 33U; ++i) {
            const Nexus_V1_TitleCnfdRecordReceipt *r = &corpus.records[i];
            uint32_t nonzero = 0U;
            uint32_t j;
            for (j = 0U; j < r->plane_length; ++j) {
                if (bytes[r->plane_offset + j]) ++nonzero;
            }
            CHECK(nonzero == k_nonzero[i]);
        }
    }
}

static void check_rejections(const uint8_t *pristine)
{
    Nexus_V1_TitleResSourceIdentity identity;
    Nexus_V1_TitleCnfdCorpusReceipt corpus;
    Nexus_V1_TitleCnfdRecordReceipt record;
    uint8_t *copy;

    make_identity(pristine, SYNTH_BYTES, &identity);

    CHECK(nexus_v1_title_cnfd_corpus_admit(NULL, SYNTH_BYTES, &identity, &corpus) == 0);
    CHECK(nexus_v1_title_cnfd_corpus_admit(pristine, SYNTH_BYTES, NULL, &corpus) == 0);
    CHECK(nexus_v1_title_cnfd_corpus_admit(pristine, SYNTH_BYTES, &identity, NULL) == 0);
    CHECK(nexus_v1_title_cnfd_record_admit(pristine, SYNTH_BYTES, &identity, 33U, &record) == 0);
    CHECK(nexus_v1_title_cnfd_record_admit(pristine, SYNTH_BYTES, &identity, 0U, NULL) == 0);

    {
        Nexus_V1_TitleResSourceIdentity drifted = identity;
        drifted.source_fnv1a64 ^= UINT64_C(1);
        CHECK(nexus_v1_title_cnfd_corpus_admit(pristine, SYNTH_BYTES, &drifted, &corpus) == 0);
    }

    copy = (uint8_t *)malloc(SYNTH_BYTES);
    CHECK(copy != NULL);
    if (!copy) return;

    /* Head dimension tamper breaks the canonical bindings. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[k_offsets[27U] + 10U] ^= 0x01U;
    make_identity(copy, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_title_cnfd_record_admit(copy, SYNTH_BYTES, &identity, 0U, &record) == 0);
    CHECK(nexus_v1_title_cnfd_corpus_admit(copy, SYNTH_BYTES, &identity, &corpus) == 0);

    /* Flag-word tamper. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[k_offsets[27U + 6U] + 14U] ^= 0xffU;
    make_identity(copy, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_title_cnfd_record_admit(copy, SYNTH_BYTES, &identity, 6U, &record) == 0);
    CHECK(nexus_v1_title_cnfd_corpus_admit(copy, SYNTH_BYTES, &identity, &corpus) == 0);

    /* Prefix divergence in record 2: admission still succeeds and the
     * distinct-prefix count rises from 8 to 9. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[k_offsets[27U + 2U] + 16U + 10U] ^= 0x01U;
    make_identity(copy, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_title_cnfd_corpus_admit(copy, SYNTH_BYTES, &identity, &corpus) == 1);
    CHECK(corpus.valid && corpus.distinct_prefix_count == 9U);

    /* Plane-byte tamper rebinds the live FNV and moves only the recorded
     * plane digest. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[k_offsets[27U] + 48U + 100U] ^= 0x01U;
    {
        uint64_t original_plane_fnv;
        make_identity(pristine, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_title_cnfd_record_admit(pristine, SYNTH_BYTES, &identity, 0U, &record) == 1);
        original_plane_fnv = record.plane_fnv1a64;
        make_identity(copy, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_title_cnfd_record_admit(copy, SYNTH_BYTES, &identity, 0U, &record) == 1);
        CHECK(record.plane_fnv1a64 != original_plane_fnv);
    }
    free(copy);
}

int main(int argc, char **argv)
{
    uint8_t *synthetic = build_synthetic();
    if (!synthetic) {
        fprintf(stderr, "allocation failure\n");
        return 1;
    }
    check_corpus(synthetic, SYNTH_BYTES, 1);
    check_rejections(synthetic);

    if (argc > 1) {
        size_t real_size = 0;
        uint8_t *real = read_file(argv[1], &real_size);
        if (!real) {
            printf("SKIP: cannot read %s\n", argv[1]);
            free(synthetic);
            return 77;
        }
        CHECK(real_size == (size_t)NEXUS_V1_TITLE_BIN_BYTES);
        if (real_size == (size_t)NEXUS_V1_TITLE_BIN_BYTES) {
            check_corpus(real, real_size, 0);
        }
        free(real);
    }

    free(synthetic);
    if (failures) {
        fprintf(stderr, "FAILURES: %d\n", failures);
        return 1;
    }
    printf("PASS nexus_v1_title_cnfd_payload_admission%s\n", argc > 1 ? " (real)" : " (synthetic)");
    return 0;
}
