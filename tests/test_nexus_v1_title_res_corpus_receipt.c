#include "nexus_v1_title_res_corpus_receipt.h"
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

static uint32_t record_length(uint32_t i)
{
    return i + 1U < NEXUS_V1_TITLE_RES_ENTRY_COUNT ?
        k_offsets[i + 1U] - k_offsets[i] : (uint32_t)SYNTH_BYTES - k_offsets[i];
}

/* Builds a synthetic RES* envelope carrying the canonical TITLE.BIN
 * directory framing: identical entry classes, ids, offsets, record-head
 * magics, and tag bytes. Record bodies are a deterministic synthetic
 * pattern; no byte claims image, palette, or subrecord meaning. */
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
        memcpy(record, k_magic[i], 4U);
        put_be32(record + 4U, k_id[i]);
        if (k_class[i] == 2U) {
            memcpy(record + 8U, "TIBG", 4U);
        } else {
            record[8] = record[9] = k_class[i] == 1U ? 0x50U : 0x70U;
            put_be16(record + 10U, (uint16_t)(i + 1U));
        }
        put_be16(record + 12U, (uint16_t)(0x20U | i));
        put_be16(record + 14U, (uint16_t)(0x8000U + i));
        for (j = 16U; j < record_length(i); ++j) {
            record[j] = (uint8_t)(0x5aU + i * 7U + j * 13U);
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
    Nexus_V1_TitleResCorpusReceipt corpus;
    Nexus_V1_TitleResSpanIterator iterator;
    Nexus_V1_TitleResSpan span;
    uint32_t i;
    int rc;

    make_identity(bytes, size, &identity);
    CHECK(nexus_v1_title_res_corpus_admit(bytes, size, &identity, &corpus) == 1);
    CHECK(corpus.valid && corpus.source_identity_bound && corpus.res_directory_bound &&
          corpus.all_records_bound);
    CHECK(corpus.contiguous_chain_observed == 1);
    CHECK(corpus.chain_covers_source_tail == 1);
    CHECK(!corpus.record_grammar_proven && !corpus.palette_proven &&
          !corpus.pixel_decode_permitted && !corpus.draw_permitted);
    CHECK(corpus.entry_count == 60U);
    CHECK(corpus.dgt2_count == 22U && corpus.titl_count == 4U &&
          corpus.mapd_count == 1U && corpus.cnfd_count == 33U);
    CHECK(corpus.chain_offset == 0x2e8U);
    CHECK(corpus.chain_length == (uint32_t)size - 0x2e8U);
    CHECK(corpus.chain_fnv1a64 != 0U && corpus.table_fnv1a64 != 0U);
    CHECK(corpus.source_fnv1a64 == identity.source_fnv1a64);

    for (i = 0U; i < NEXUS_V1_TITLE_RES_ENTRY_COUNT; ++i) {
        const Nexus_V1_TitleResRecordReceipt *r = &corpus.records[i];
        Nexus_V1_TitleResRecordReceipt single;
        CHECK(r->valid && r->entry_bound && r->record_head_bound && r->record_span_bound);
        CHECK(r->source_identity_bound && r->res_directory_bound);
        CHECK(r->source_fnv1a64 == identity.source_fnv1a64);
        CHECK(r->entry_index == i);
        CHECK(r->entry_class == k_class[i]);
        CHECK(r->entry_id == k_id[i]);
        CHECK(r->record_offset == k_offsets[i]);
        CHECK(r->record_length == record_length(i));
        CHECK(r->record_fnv1a64 != 0U && r->entry_fnv1a64 != 0U &&
              r->record_head_fnv1a64 != 0U);
        CHECK(r->record_inner_id == k_id[i]);
        CHECK(r->pp_tag_observed == (k_class[i] == 0U || k_class[i] == 3U));
        CHECK(r->PP_tag_observed == (k_class[i] == 1U));
        CHECK(r->tibg_tag_observed == (k_class[i] == 2U));
        CHECK(r->head_tag_word ==
              (k_class[i] == 2U ? 0x5449U : k_class[i] == 1U ? 0x5050U : 0x7070U));
        if (synthetic) {
            CHECK(r->head_word0 ==
                  (k_class[i] == 2U ? 0x4247U : (uint16_t)(i + 1U)));
            CHECK(r->head_word1 == (uint16_t)(0x20U | i));
            CHECK(r->head_word2 == (uint16_t)(0x8000U + i));
        } else if (k_class[i] == 0U) {
            /* Canonical DGT2 head-word groups verified against the retail
             * asset: 0..3 (64,8), 4..5 (104,8), 6..20 (24,24,0x81e0),
             * 21 (168,8->12). Opaque measurements only. */
            if (i <= 3U) {
                CHECK(r->head_word0 == 64U && r->head_word1 == 8U &&
                      r->head_word2 == 0x8220U);
            } else if (i <= 5U) {
                CHECK(r->head_word0 == 104U && r->head_word1 == 8U &&
                      r->head_word2 == 0x8220U);
            } else if (i <= 20U) {
                CHECK(r->head_word0 == 24U && r->head_word1 == 24U &&
                      r->head_word2 == 0x81e0U);
            } else {
                CHECK(r->head_word0 == 168U && r->head_word1 == 12U &&
                      r->head_word2 == 0x8220U);
            }
        }
        CHECK(nexus_v1_title_res_record_admit(bytes, size, &identity, i, &single) == 1);
        CHECK(single.record_offset == r->record_offset &&
              single.record_length == r->record_length &&
              single.record_fnv1a64 == r->record_fnv1a64);
    }

    CHECK(nexus_v1_title_res_span_iterator_init(&iterator, &corpus) == 0);
    for (i = 0U; i < NEXUS_V1_TITLE_RES_ENTRY_COUNT; ++i) {
        rc = nexus_v1_title_res_span_iterator_next(&iterator, &span);
        CHECK(rc == 1);
        if (rc == 1) {
            CHECK(span.source_offset == corpus.records[i].record_offset);
            CHECK(span.source_length == corpus.records[i].record_length);
            CHECK(span.source_fnv1a64 == corpus.records[i].record_fnv1a64);
        }
    }
    CHECK(nexus_v1_title_res_span_iterator_next(&iterator, &span) == 0);
}

static void check_rejections(const uint8_t *pristine)
{
    Nexus_V1_TitleResSourceIdentity identity;
    Nexus_V1_TitleResCorpusReceipt corpus;
    Nexus_V1_TitleResRecordReceipt record;
    uint8_t *copy;
    static const uint32_t head_probe[4] = { 0U, 22U, 26U, 27U };
    uint32_t probe;

    make_identity(pristine, SYNTH_BYTES, &identity);

    CHECK(nexus_v1_title_res_corpus_admit(NULL, SYNTH_BYTES, &identity, &corpus) == 0);
    CHECK(nexus_v1_title_res_corpus_admit(pristine, SYNTH_BYTES, NULL, &corpus) == 0);
    CHECK(nexus_v1_title_res_corpus_admit(pristine, SYNTH_BYTES - 1U, &identity, &corpus) == 0);
    CHECK(nexus_v1_title_res_corpus_admit(pristine, SYNTH_BYTES, &identity, NULL) == 0);
    CHECK(nexus_v1_title_res_record_admit(pristine, SYNTH_BYTES, &identity, 60U, &record) == 0);
    CHECK(nexus_v1_title_res_record_admit(pristine, SYNTH_BYTES, &identity, 0U, NULL) == 0);

    /* Identity drift: wrong live FNV, unverified SHA, or wrong SHA string. */
    {
        Nexus_V1_TitleResSourceIdentity drifted = identity;
        drifted.source_fnv1a64 ^= UINT64_C(1);
        CHECK(nexus_v1_title_res_corpus_admit(pristine, SYNTH_BYTES, &drifted, &corpus) == 0);
    }
    {
        Nexus_V1_TitleResSourceIdentity drifted = identity;
        drifted.sha256_verified = 0;
        CHECK(nexus_v1_title_res_corpus_admit(pristine, SYNTH_BYTES, &drifted, &corpus) == 0);
    }
    {
        Nexus_V1_TitleResSourceIdentity drifted = identity;
        drifted.sha256_hex = "0000000000000000000000000000000000000000000000000000000000000000";
        CHECK(nexus_v1_title_res_corpus_admit(pristine, SYNTH_BYTES, &drifted, &corpus) == 0);
    }

    /* Directory table tamper: one offset byte in entry 10. */
    copy = (uint8_t *)malloc(SYNTH_BYTES);
    CHECK(copy != NULL);
    if (!copy) return;
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[12U + 10U * NEXUS_V1_TITLE_RES_ENTRY_BYTES + 8U] ^= 0x01U;
    make_identity(copy, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_title_res_corpus_admit(copy, SYNTH_BYTES, &identity, &corpus) == 0);

    /* Record-head tag tamper, one probe per class. */
    for (probe = 0U; probe < 4U; ++probe) {
        uint32_t entry = head_probe[probe];
        memcpy(copy, pristine, SYNTH_BYTES);
        copy[k_offsets[entry] + 8U] ^= 0xffU;
        make_identity(copy, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_title_res_record_admit(copy, SYNTH_BYTES, &identity, entry, &record) == 0);
        CHECK(nexus_v1_title_res_corpus_admit(copy, SYNTH_BYTES, &identity, &corpus) == 0);
    }

    /* Body-byte tamper outside any head: admission still succeeds because
     * the live FNV re-binds the bytes; only the recorded digests move. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[k_offsets[0] + 100U] ^= 0x01U;
    {
        uint64_t original_record_fnv;
        make_identity(pristine, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_title_res_record_admit(pristine, SYNTH_BYTES, &identity, 0U, &record) == 1);
        original_record_fnv = record.record_fnv1a64;
        make_identity(copy, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_title_res_corpus_admit(copy, SYNTH_BYTES, &identity, &corpus) == 1);
        CHECK(corpus.valid && corpus.source_fnv1a64 == identity.source_fnv1a64);
        CHECK(corpus.records[0].record_fnv1a64 != original_record_fnv);
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
    printf("PASS nexus_v1_title_res_corpus_receipt%s\n", argc > 1 ? " (real)" : " (synthetic)");
    return 0;
}
