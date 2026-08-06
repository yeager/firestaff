#include "nexus_v1_title_titl_pp_payload_admission.h"

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

static const uint16_t k_width[4] = { 304U, 160U, 304U, 256U };
static const uint16_t k_height[4] = { 104U, 28U, 22U, 16U };
static const uint32_t k_plane_len[4] = { 31616U, 4480U, 6688U, 4096U };
static const uint32_t k_record_len[4] = { 0x7d90U, 0x1390U, 0x1c30U, 0x1210U };

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

static uint32_t full_record_length(uint32_t i)
{
    return i + 1U < NEXUS_V1_TITLE_RES_ENTRY_COUNT ?
        k_offsets[i + 1U] - k_offsets[i] : (uint32_t)SYNTH_BYTES - k_offsets[i];
}

/* Builds a synthetic RES* envelope carrying the canonical TITLE.BIN
 * directory framing, with the four TITL records populated as PP payloads:
 * canonical PP header dimensions, a deterministic 512-byte prefix shared
 * byte-identically by all four records, a deterministic width*height
 * plane, and two zero trailing bytes. No byte claims colour, palette, or
 * pixel meaning. */
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
            uint32_t t = i - 22U;
            record[8] = record[9] = 0x50U;
            put_be16(record + 10U, k_width[t]);
            put_be16(record + 12U, k_height[t]);
            for (j = 0U; j < 512U; ++j) {
                record[14U + j] = (uint8_t)(0xa0U + j * 3U);
            }
            record[14U] = 0x82U;
            record[15U] = 0x20U;
            for (j = 0U; j < k_plane_len[t]; ++j) {
                record[14U + 512U + j] = (uint8_t)(0x40U + t * 11U + j * 7U);
            }
            record[14U + 512U + k_plane_len[t]] = 0U;
            record[14U + 512U + k_plane_len[t] + 1U] = 0U;
        } else {
            record[8] = record[9] = 0x70U;
            put_be16(record + 10U, (uint16_t)(i + 1U));
            put_be16(record + 12U, (uint16_t)(0x20U | i));
            put_be16(record + 14U, (uint16_t)(0x8000U + i));
        }
        for (j = (k_class[i] == 1U) ? length : 16U; j < length; ++j) {
            if (k_class[i] != 1U) record[j] = (uint8_t)(0x5aU + i * 7U + j * 13U);
        }
    }
    return bytes;
}

static void make_identity(const uint8_t *bytes, size_t size,
                          Nexus_V1_TitleResSourceIdentity *out_identity)
{
    const char *real_sha256 = getenv("FIRESTAFF_NEXUS_TITLE_BIN_SHA256");
    memset(out_identity, 0, sizeof(*out_identity));
    out_identity->sha256_verified = 1;
    out_identity->sha256_hex = real_sha256 && real_sha256[0]
        ? real_sha256 : NEXUS_V1_TITLE_BIN_SHA256;
    out_identity->source_fnv1a64 = fnv1a64(bytes, size);
}

static void check_corpus(const uint8_t *bytes, size_t size, int synthetic)
{
    Nexus_V1_TitleResSourceIdentity identity;
    Nexus_V1_TitleTitlPpCorpusReceipt corpus;
    Nexus_V1_TitleTitlPpPlaneSpanIterator iterator;
    Nexus_V1_TitleTitlPpPlaneSpan span;
    uint32_t i;
    int rc;

    make_identity(bytes, size, &identity);
    CHECK(nexus_v1_title_titl_pp_corpus_admit(bytes, size, &identity, &corpus) == 1);
    CHECK(corpus.valid && corpus.source_identity_bound &&
          corpus.res_directory_bound && corpus.all_titl_bound);
    CHECK(corpus.contiguous_chain_observed == 1);
    if (synthetic) {
        CHECK(corpus.shared_prefix_observed == 1);
    } else if (strcmp(identity.sha256_hex,
                      NEXUS_V1_TITLE_BIN_ENGLISH_SHA256) == 0) {
        /* English ISO: records 0, 2 and 3 share the prefix; record 1 is
         * distinct. This is a real revision receipt, not a wildcard. */
        CHECK(corpus.shared_prefix_observed == 0);
        CHECK(corpus.records[0].prefix_fnv1a64 ==
              corpus.records[2].prefix_fnv1a64);
        CHECK(corpus.records[0].prefix_fnv1a64 ==
              corpus.records[3].prefix_fnv1a64);
        CHECK(corpus.records[0].prefix_fnv1a64 !=
              corpus.records[1].prefix_fnv1a64);
    } else {
        CHECK(corpus.shared_prefix_observed == 1);
    }
    CHECK(!corpus.colour_proven && !corpus.palette_proven &&
          !corpus.pixel_decode_permitted && !corpus.draw_permitted &&
          !corpus.presentation_permitted);
    CHECK(corpus.titl_count == 4U);
    CHECK(corpus.chain_offset == 0x2318U);
    CHECK(corpus.chain_length == 0xbf60U);
    CHECK(corpus.chain_fnv1a64 != 0U);
    CHECK(corpus.shared_prefix_fnv1a64 == corpus.records[0].prefix_fnv1a64);
    CHECK(corpus.source_fnv1a64 == identity.source_fnv1a64);

    for (i = 0U; i < 4U; ++i) {
        const Nexus_V1_TitleTitlPpRecordReceipt *r = &corpus.records[i];
        Nexus_V1_TitleTitlPpRecordReceipt single;
        CHECK(r->valid && r->titl_head_bound && r->pp_header_bound &&
              r->prefix_span_bound && r->plane_span_bound &&
              r->trailing_span_bound && r->length_arithmetic_bound);
        CHECK(r->source_identity_bound && r->res_directory_bound);
        CHECK(!r->colour_proven && !r->palette_proven &&
              !r->pixel_decode_permitted && !r->draw_permitted &&
              !r->presentation_permitted);
        CHECK(r->source_fnv1a64 == identity.source_fnv1a64);
        CHECK(r->titl_index == i);
        CHECK(r->entry_index == 22U + i);
        CHECK(r->entry_id == i);
        CHECK(r->record_offset == k_offsets[22U + i]);
        CHECK(r->record_length == k_record_len[i]);
        CHECK(r->record_length == 14U + 512U + (uint32_t)r->width * r->height + 2U);
        CHECK(r->width == k_width[i] && r->height == k_height[i]);
        CHECK(r->prefix_leading_word == 0x8220U);
        CHECK(r->prefix_offset == r->record_offset + 14U);
        CHECK(r->prefix_fnv1a64 != 0U);
        CHECK(r->plane_offset == r->prefix_offset + 512U);
        CHECK(r->plane_length == k_plane_len[i]);
        CHECK(r->plane_fnv1a64 != 0U);
        CHECK(r->trailing_offset == r->plane_offset + r->plane_length);
        CHECK(r->trailing_fnv1a64 != 0U);
        CHECK(r->record_fnv1a64 != 0U);
        if (!synthetic) {
            CHECK(r->trailing_be16 == 0U);
        }
        CHECK(nexus_v1_title_titl_pp_record_admit(bytes, size, &identity, i, &single) == 1);
        CHECK(single.record_offset == r->record_offset &&
              single.plane_fnv1a64 == r->plane_fnv1a64 &&
              single.prefix_fnv1a64 == r->prefix_fnv1a64);
    }

    CHECK(nexus_v1_title_titl_pp_plane_span_iterator_init(&iterator, &corpus) == 0);
    for (i = 0U; i < 4U; ++i) {
        rc = nexus_v1_title_titl_pp_plane_span_iterator_next(&iterator, &span);
        CHECK(rc == 1);
        if (rc == 1) {
            CHECK(span.source_offset == corpus.records[i].plane_offset);
            CHECK(span.source_length == corpus.records[i].plane_length);
            CHECK(span.source_fnv1a64 == corpus.records[i].plane_fnv1a64);
        }
    }
    CHECK(nexus_v1_title_titl_pp_plane_span_iterator_next(&iterator, &span) == 0);

    if (!synthetic) {
        /* Opaque real-asset witnesses verified against the canonical
         * TITLE.BIN: per-plane nonzero byte counts. No pixel meaning. */
        static const uint32_t k_nonzero_canonical[4] = {
            15187U, 912U, 1572U, 885U
        };
        static const uint32_t k_nonzero_english[4] = {
            15187U, 410U, 1572U, 885U
        };
        const uint32_t *k_nonzero =
            strcmp(identity.sha256_hex, NEXUS_V1_TITLE_BIN_ENGLISH_SHA256) == 0
                ? k_nonzero_english : k_nonzero_canonical;
        for (i = 0U; i < 4U; ++i) {
            const Nexus_V1_TitleTitlPpRecordReceipt *r = &corpus.records[i];
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
    Nexus_V1_TitleTitlPpCorpusReceipt corpus;
    Nexus_V1_TitleTitlPpRecordReceipt record;
    uint8_t *copy;

    make_identity(pristine, SYNTH_BYTES, &identity);

    CHECK(nexus_v1_title_titl_pp_corpus_admit(NULL, SYNTH_BYTES, &identity, &corpus) == 0);
    CHECK(nexus_v1_title_titl_pp_corpus_admit(pristine, SYNTH_BYTES, NULL, &corpus) == 0);
    CHECK(nexus_v1_title_titl_pp_corpus_admit(pristine, SYNTH_BYTES, &identity, NULL) == 0);
    CHECK(nexus_v1_title_titl_pp_record_admit(pristine, SYNTH_BYTES, &identity, 4U, &record) == 0);
    CHECK(nexus_v1_title_titl_pp_record_admit(pristine, SYNTH_BYTES, &identity, 0U, NULL) == 0);

    {
        Nexus_V1_TitleResSourceIdentity drifted = identity;
        drifted.source_fnv1a64 ^= UINT64_C(1);
        CHECK(nexus_v1_title_titl_pp_corpus_admit(pristine, SYNTH_BYTES, &drifted, &corpus) == 0);
    }

    copy = (uint8_t *)malloc(SYNTH_BYTES);
    CHECK(copy != NULL);
    if (!copy) return;

    /* PP header dimension tamper breaks the canonical bindings. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[k_offsets[22U] + 10U] ^= 0x01U;
    make_identity(copy, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_title_titl_pp_record_admit(copy, SYNTH_BYTES, &identity, 0U, &record) == 0);
    CHECK(nexus_v1_title_titl_pp_corpus_admit(copy, SYNTH_BYTES, &identity, &corpus) == 0);

    /* Prefix leading-word tamper. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[k_offsets[23U] + 14U] ^= 0xffU;
    make_identity(copy, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_title_titl_pp_record_admit(copy, SYNTH_BYTES, &identity, 1U, &record) == 0);
    CHECK(nexus_v1_title_titl_pp_corpus_admit(copy, SYNTH_BYTES, &identity, &corpus) == 0);

    /* Prefix divergence past the leading word: admission still succeeds
     * and the shared-prefix observation flips to 0. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[k_offsets[23U] + 14U + 100U] ^= 0x01U;
    make_identity(copy, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_title_titl_pp_corpus_admit(copy, SYNTH_BYTES, &identity, &corpus) == 1);
    CHECK(corpus.valid && corpus.shared_prefix_observed == 0);

    /* Plane-byte tamper rebinds the live FNV and moves only the recorded
     * plane digest. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[k_offsets[22U] + 14U + 512U + 1000U] ^= 0x01U;
    {
        uint64_t original_plane_fnv;
        make_identity(pristine, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_title_titl_pp_record_admit(pristine, SYNTH_BYTES, &identity, 0U, &record) == 1);
        original_plane_fnv = record.plane_fnv1a64;
        make_identity(copy, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_title_titl_pp_record_admit(copy, SYNTH_BYTES, &identity, 0U, &record) == 1);
        CHECK(record.plane_fnv1a64 != original_plane_fnv);
    }

    /* Trailing-byte tamper likewise moves only the trailing digest. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[k_offsets[22U] + 14U + 512U + k_plane_len[0]] ^= 0x01U;
    {
        uint64_t original_trailing_fnv;
        make_identity(pristine, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_title_titl_pp_record_admit(pristine, SYNTH_BYTES, &identity, 0U, &record) == 1);
        original_trailing_fnv = record.trailing_fnv1a64;
        make_identity(copy, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_title_titl_pp_record_admit(copy, SYNTH_BYTES, &identity, 0U, &record) == 1);
        CHECK(record.trailing_fnv1a64 != original_trailing_fnv);
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
    printf("PASS nexus_v1_title_titl_pp_payload_admission%s\n", argc > 1 ? " (real)" : " (synthetic)");
    return 0;
}
