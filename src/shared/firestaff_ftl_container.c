#include "firestaff_ftl_container.h"

#include <stdio.h>
#include <string.h>

static uint16_t rd16_be(const uint8_t* p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static uint32_t rd32_be(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
            (uint32_t)p[3];
}

static void wr16_be(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xffu);
}

static void wr32_be(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xffu);
    p[2] = (uint8_t)((v >> 8) & 0xffu);
    p[3] = (uint8_t)(v & 0xffu);
}

static uint16_t checksum_add(uint32_t sum, uint32_t add) {
    sum += add;
    sum %= 0xffffu;
    return (uint16_t)sum;
}

uint16_t FirestaffFtlContainer_CommonChecksum(const uint8_t* data,
                                              size_t data_size,
                                              size_t hunk_count) {
    if (!data || data_size < 20u) return 0;
    if (hunk_count > FIRESTAFF_FTL_MAX_HUNKS) return 0;
    size_t table_bytes = hunk_count * 12u;
    if (data_size < 20u + table_bytes) return 0;

    uint16_t checksum = 0;
    for (size_t i = 4u; i < 20u; ++i) {
        checksum = checksum_add(checksum, (uint32_t)data[i] * (uint32_t)i);
    }
    const uint8_t* headers = data + 20u;
    for (size_t h = 0; h < hunk_count; ++h) {
        for (size_t j = 0; j < 12u; ++j) {
            uint32_t multiplier = (uint32_t)(j + (12u * h) + 1u);
            checksum = checksum_add(checksum,
                                    (uint32_t)headers[h * 12u + j] * multiplier);
        }
    }
    return checksum;
}

uint16_t FirestaffFtlContainer_BssChecksum(const uint8_t* hunk,
                                           size_t hunk_size) {
    if (!hunk) return 0;
    uint16_t checksum = 0;
    for (size_t i = 0; i + 1u < hunk_size; i += 2u) {
        if (i == 34u) continue;
        checksum = checksum_add(checksum, rd16_be(hunk + i));
    }
    return checksum;
}

uint16_t FirestaffFtlContainer_ByteChecksum(const uint8_t* data,
                                            size_t data_size) {
    if (!data) return 0;
    uint16_t checksum = 0;
    for (size_t i = 0; i < data_size; ++i) {
        checksum = checksum_add(checksum, data[i]);
    }
    return checksum;
}

static int read_bss_metadata(const FirestaffFtlHunkHeader* hunk,
                             FirestaffFtlBssMetadata* out) {
    if (!hunk || !out || !hunk->payload || hunk->size < 40u) return 0;
    const uint8_t* p = hunk->payload;
    memset(out, 0, sizeof(*out));
    out->bss_and_jump_memory_size = rd32_be(p + 0u);
    out->data_area1_memory_size = rd32_be(p + 4u);
    out->jump_table_memory_size = rd32_be(p + 8u);
    out->bss_memory_size = rd32_be(p + 12u);
    out->unknown_16 = rd32_be(p + 16u);
    out->code_memory_size = rd32_be(p + 20u);
    out->data_file_size = rd32_be(p + 24u);
    out->unknown_28 = rd16_be(p + 28u);
    out->unknown_30 = rd16_be(p + 30u);
    out->unknown_32 = rd16_be(p + 32u);
    out->bss_checksum = rd16_be(p + 34u);
    out->code_checksum = rd16_be(p + 36u);
    out->data_checksum = rd16_be(p + 38u);
    return 1;
}

int FirestaffFtlContainer_Parse(const uint8_t* data,
                                size_t data_size,
                                FirestaffFtlContainer* out) {
    if (!data || !out || data_size < 20u) return -1;
    memset(out, 0, sizeof(*out));
    out->bss_index = -1;
    out->data_index = -1;
    out->code_index = -1;

    out->header.magic = rd16_be(data + 0u);
    out->header.common_checksum = rd16_be(data + 2u);
    out->header.unknown_word = rd16_be(data + 4u);
    memcpy(out->header.unknown_bytes, data + 6u, sizeof(out->header.unknown_bytes));
    out->header.hunk_count = rd16_be(data + 18u);

    if (out->header.magic != FIRESTAFF_FTL_CONTAINER_MAGIC) return -1;
    if (out->header.hunk_count == 0u ||
        out->header.hunk_count > FIRESTAFF_FTL_MAX_HUNKS) {
        return -1;
    }

    size_t hunk_count = (size_t)out->header.hunk_count;
    if (data_size < 20u + hunk_count * 12u) return -1;
    out->hunk_count = hunk_count;

    for (size_t i = 0; i < hunk_count; ++i) {
        const uint8_t* hp = data + 20u + i * 12u;
        FirestaffFtlHunkHeader* h = &out->hunks[i];
        h->type = rd16_be(hp + 0u);
        h->unknown = rd16_be(hp + 2u);
        h->offset = rd32_be(hp + 4u);
        h->size = rd32_be(hp + 8u);
        if ((uint64_t)h->offset + (uint64_t)h->size > (uint64_t)data_size) {
            return -1;
        }
        h->payload = data + h->offset;

        if (h->type == FIRESTAFF_FTL_HUNK_BSS && out->bss_index < 0) {
            out->bss_index = (int)i;
        } else if (h->type == FIRESTAFF_FTL_HUNK_DATA && out->data_index < 0) {
            out->data_index = (int)i;
        } else if (h->type == FIRESTAFF_FTL_HUNK_CODE && out->code_index < 0) {
            out->code_index = (int)i;
        }
    }

    if (out->bss_index >= 0) {
        out->has_bss_metadata =
            read_bss_metadata(&out->hunks[out->bss_index], &out->bss);
    }
    return 0;
}

static void fill_result(FirestaffFtlChecksumResult* r,
                        FirestaffFtlChecksumStatus status,
                        uint16_t expected,
                        uint16_t actual) {
    r->status = status;
    r->expected = expected;
    r->actual = actual;
}

int FirestaffFtlContainer_VerifyChecksums(const uint8_t* data,
                                          size_t data_size,
                                          const FirestaffFtlContainer* ftl,
                                          FirestaffFtlChecksumReport* out) {
    if (!data || !ftl || !out) return -1;
    memset(out, 0, sizeof(*out));
    out->has_required_bss_data_code =
        (ftl->bss_index >= 0 && ftl->data_index >= 0 && ftl->code_index >= 0);

    uint16_t common = FirestaffFtlContainer_CommonChecksum(data, data_size,
                                                           ftl->hunk_count);
    fill_result(&out->common_header,
                common == ftl->header.common_checksum ?
                    FIRESTAFF_FTL_CHECK_MATCH : FIRESTAFF_FTL_CHECK_MISMATCH,
                ftl->header.common_checksum,
                common);

    if (ftl->bss_index < 0 || !ftl->has_bss_metadata) {
        fill_result(&out->bss_hunk, FIRESTAFF_FTL_CHECK_NOT_AVAILABLE, 0, 0);
        fill_result(&out->data_hunk, FIRESTAFF_FTL_CHECK_NOT_AVAILABLE, 0, 0);
        fill_result(&out->code_hunk, FIRESTAFF_FTL_CHECK_NOT_AVAILABLE, 0, 0);
        return 0;
    }

    const FirestaffFtlHunkHeader* bss = &ftl->hunks[ftl->bss_index];
    uint16_t bss_sum = FirestaffFtlContainer_BssChecksum(bss->payload, bss->size);
    fill_result(&out->bss_hunk,
                bss_sum == ftl->bss.bss_checksum ?
                    FIRESTAFF_FTL_CHECK_MATCH : FIRESTAFF_FTL_CHECK_MISMATCH,
                ftl->bss.bss_checksum,
                bss_sum);

    if (ftl->data_index >= 0) {
        const FirestaffFtlHunkHeader* h = &ftl->hunks[ftl->data_index];
        uint16_t sum = FirestaffFtlContainer_ByteChecksum(h->payload, h->size);
        fill_result(&out->data_hunk,
                    sum == ftl->bss.data_checksum ?
                        FIRESTAFF_FTL_CHECK_MATCH : FIRESTAFF_FTL_CHECK_MISMATCH,
                    ftl->bss.data_checksum,
                    sum);
    } else {
        fill_result(&out->data_hunk, FIRESTAFF_FTL_CHECK_NOT_AVAILABLE,
                    ftl->bss.data_checksum, 0);
    }

    if (ftl->code_index >= 0) {
        const FirestaffFtlHunkHeader* h = &ftl->hunks[ftl->code_index];
        /* The BSS metadata's code_checksum (greatstone checksum 4) is
         * the byte checksum of the on-disk CODE payload, regardless
         * of whether the payload is compressed (0x5223 + table +
         * nibbles) or uncompressed. Both shapes can therefore be
         * verified with the same byte-checksum helper.
         *
         * Decoding the 0x5223 compressed bytes is the job of the
         * separate firestaff_ftl_decode.c HUNK_CODE decoder, not the
         * checksum verifier; that path is exercised by
         * firestaff_ftl_decode_unit. */
        uint16_t sum = FirestaffFtlContainer_ByteChecksum(h->payload, h->size);
        fill_result(&out->code_hunk,
                    sum == ftl->bss.code_checksum ?
                        FIRESTAFF_FTL_CHECK_MATCH : FIRESTAFF_FTL_CHECK_MISMATCH,
                    ftl->bss.code_checksum,
                    sum);
    } else {
        fill_result(&out->code_hunk, FIRESTAFF_FTL_CHECK_NOT_AVAILABLE,
                    ftl->bss.code_checksum, 0);
    }
    return 0;
}

#define ST_ASSERT(cond, msg) do {                                      \
    if (!(cond)) {                                                     \
        fprintf(stderr, "%s:%d: %s (%s)\n", __FILE__, __LINE__,        \
                msg, #cond);                                           \
        return 0;                                                      \
    }                                                                  \
} while (0)

static size_t build_valid_ftl(uint8_t* buf, size_t cap, int compressed_code) {
    if (!buf || cap < 128u) return 0;
    memset(buf, 0, cap);

    const size_t hunk_count = 3u;
    const size_t bss_off = 20u + hunk_count * 12u;
    const size_t bss_size = 40u;
    const size_t data_off = bss_off + bss_size;
    const size_t data_size = 12u;
    const size_t code_off = data_off + data_size;
    const size_t code_size = compressed_code ? 8u : 4u;
    const size_t total = code_off + code_size;

    wr16_be(buf + 0u, FIRESTAFF_FTL_CONTAINER_MAGIC);
    wr16_be(buf + 4u, 0x0002u);
    buf[6] = 0x01u;
    buf[11] = 0x01u;
    buf[12] = 0x04u;
    buf[13] = 0x01u;
    wr16_be(buf + 18u, (uint16_t)hunk_count);

    uint8_t* h = buf + 20u;
    wr16_be(h + 0u, FIRESTAFF_FTL_HUNK_BSS);
    wr32_be(h + 4u, (uint32_t)bss_off);
    wr32_be(h + 8u, (uint32_t)bss_size);
    h += 12u;
    wr16_be(h + 0u, FIRESTAFF_FTL_HUNK_DATA);
    wr32_be(h + 4u, (uint32_t)data_off);
    wr32_be(h + 8u, (uint32_t)data_size);
    h += 12u;
    wr16_be(h + 0u, FIRESTAFF_FTL_HUNK_CODE);
    wr32_be(h + 4u, (uint32_t)code_off);
    wr32_be(h + 8u, (uint32_t)code_size);

    uint8_t* bss = buf + bss_off;
    wr32_be(bss + 4u, 2u);
    wr32_be(bss + 20u, (uint32_t)code_size);
    wr32_be(bss + 24u, (uint32_t)data_size);

    uint8_t* data = buf + data_off;
    data[5] = 2u;
    data[10] = 0xaau;
    data[11] = 0xbbu;

    uint8_t* code = buf + code_off;
    if (compressed_code) {
        code[0] = 0x52u;
        code[1] = 0x23u;
        code[2] = 0x52u;
        code[3] = 0x23u;
    } else {
        code[0] = 0x12u;
        code[1] = 0x34u;
        code[2] = 0x56u;
        code[3] = 0x78u;
    }

    wr16_be(bss + 36u, FirestaffFtlContainer_ByteChecksum(code, code_size));
    wr16_be(bss + 38u, FirestaffFtlContainer_ByteChecksum(data, data_size));
    wr16_be(bss + 34u, FirestaffFtlContainer_BssChecksum(bss, bss_size));
    wr16_be(buf + 2u, FirestaffFtlContainer_CommonChecksum(buf, total, hunk_count));
    return total;
}

static int test_valid_uncompressed(void) {
    uint8_t buf[128];
    size_t size = build_valid_ftl(buf, sizeof(buf), 0);
    ST_ASSERT(size > 0u, "build");

    FirestaffFtlContainer ftl;
    ST_ASSERT(FirestaffFtlContainer_Parse(buf, size, &ftl) == 0, "parse");
    ST_ASSERT(ftl.hunk_count == 3u, "hunk count");
    ST_ASSERT(ftl.bss_index == 0, "bss index");
    ST_ASSERT(ftl.data_index == 1, "data index");
    ST_ASSERT(ftl.code_index == 2, "code index");
    ST_ASSERT(ftl.has_bss_metadata, "bss metadata");

    FirestaffFtlChecksumReport r;
    ST_ASSERT(FirestaffFtlContainer_VerifyChecksums(buf, size, &ftl, &r) == 0,
              "verify");
    ST_ASSERT(r.has_required_bss_data_code, "required hunks");
    ST_ASSERT(r.common_header.status == FIRESTAFF_FTL_CHECK_MATCH, "common");
    ST_ASSERT(r.bss_hunk.status == FIRESTAFF_FTL_CHECK_MATCH, "bss");
    ST_ASSERT(r.data_hunk.status == FIRESTAFF_FTL_CHECK_MATCH, "data");
    ST_ASSERT(r.code_hunk.status == FIRESTAFF_FTL_CHECK_MATCH, "code");
    return 1;
}

static int test_bad_magic(void) {
    uint8_t buf[128];
    size_t size = build_valid_ftl(buf, sizeof(buf), 0);
    buf[0] = 0;
    FirestaffFtlContainer ftl;
    ST_ASSERT(FirestaffFtlContainer_Parse(buf, size, &ftl) != 0,
              "bad magic rejects");
    return 1;
}

static int test_truncated_hunk_table(void) {
    uint8_t buf[128];
    size_t size = build_valid_ftl(buf, sizeof(buf), 0);
    FirestaffFtlContainer ftl;
    ST_ASSERT(FirestaffFtlContainer_Parse(buf, 30u, &ftl) != 0,
              "truncated table rejects");
    (void)size;
    return 1;
}

static int test_out_of_bounds_hunk(void) {
    uint8_t buf[128];
    size_t size = build_valid_ftl(buf, sizeof(buf), 0);
    wr32_be(buf + 20u + 4u, 0xffffu);
    FirestaffFtlContainer ftl;
    ST_ASSERT(FirestaffFtlContainer_Parse(buf, size, &ftl) != 0,
              "out of bounds hunk rejects");
    return 1;
}

static int test_checksum_mismatch_reports(void) {
    uint8_t buf[128];
    size_t size = build_valid_ftl(buf, sizeof(buf), 0);
    buf[96u + 10u] ^= 0x01u;

    FirestaffFtlContainer ftl;
    ST_ASSERT(FirestaffFtlContainer_Parse(buf, size, &ftl) == 0, "parse");
    FirestaffFtlChecksumReport r;
    ST_ASSERT(FirestaffFtlContainer_VerifyChecksums(buf, size, &ftl, &r) == 0,
              "verify");
    ST_ASSERT(r.data_hunk.status == FIRESTAFF_FTL_CHECK_MISMATCH,
              "data mismatch");
    ST_ASSERT(r.bss_hunk.status == FIRESTAFF_FTL_CHECK_MATCH,
              "bss still matches");
    return 1;
}

static int test_common_checksum_mismatch_reports(void) {
    uint8_t buf[128];
    size_t size = build_valid_ftl(buf, sizeof(buf), 0);
    buf[7] ^= 0x01u;

    FirestaffFtlContainer ftl;
    ST_ASSERT(FirestaffFtlContainer_Parse(buf, size, &ftl) == 0, "parse");
    FirestaffFtlChecksumReport r;
    ST_ASSERT(FirestaffFtlContainer_VerifyChecksums(buf, size, &ftl, &r) == 0,
              "verify");
    ST_ASSERT(r.common_header.status == FIRESTAFF_FTL_CHECK_MISMATCH,
              "common mismatch");
    return 1;
}

static int test_compressed_code_byte_checksum_matches(void) {
    uint8_t buf[128];
    size_t size = build_valid_ftl(buf, sizeof(buf), 1);

    FirestaffFtlContainer ftl;
    ST_ASSERT(FirestaffFtlContainer_Parse(buf, size, &ftl) == 0, "parse");
    FirestaffFtlChecksumReport r;
    ST_ASSERT(FirestaffFtlContainer_VerifyChecksums(buf, size, &ftl, &r) == 0,
              "verify");
    /* The CODE hunk byte-checksum contract is independent of whether
     * the payload is compressed (0x5223 + table + nibbles) or
     * uncompressed. The greatstone checksum 4 covers the on-disk
     * bytes, so a correctly-built synthetic compressed-CODE container
     * MUST report MATCH. The compressed bytes themselves are decoded
     * separately by firestaff_ftl_decode (firestaff_ftl_decode_unit). */
    ST_ASSERT(r.code_hunk.status == FIRESTAFF_FTL_CHECK_MATCH,
              "compressed CODE byte checksum matches");
    ST_ASSERT(r.common_header.status == FIRESTAFF_FTL_CHECK_MATCH, "common");
    ST_ASSERT(r.bss_hunk.status == FIRESTAFF_FTL_CHECK_MATCH, "bss");
    ST_ASSERT(r.data_hunk.status == FIRESTAFF_FTL_CHECK_MATCH, "data");
    return 1;
}

int FirestaffFtlContainer_SelfTest(void) {
    int total = 0;
    int passed = 0;
#define RUN(test) do { ++total; if (test()) ++passed; } while (0)
    RUN(test_valid_uncompressed);
    RUN(test_bad_magic);
    RUN(test_truncated_hunk_table);
    RUN(test_out_of_bounds_hunk);
    RUN(test_checksum_mismatch_reports);
    RUN(test_common_checksum_mismatch_reports);
    RUN(test_compressed_code_byte_checksum_matches);
#undef RUN
    if (passed != total) {
        fprintf(stderr, "firestaff_ftl_container self-test: %d/%d passed\n",
                passed, total);
    }
    return passed == total ? 0 : -1;
}
