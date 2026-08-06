#include "nexus_v1_title_mapd_tibg_admission.h"
#include "nexus_v1_title.h"

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

static const uint32_t k_hdr_fields[13] = {
    0x00008c6cU, 0x00000020U, 0x00008c2cU, 0x00008c4cU,
    0x00000020U, 0x00008c6cU, 0x00029020U, 0x00000018U,
    0x00001c1cU, 0x00003820U, 0x00005424U, 0x00007028U,
    0x00000000U
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

static uint32_t full_record_length(uint32_t i)
{
    return i + 1U < NEXUS_V1_TITLE_RES_ENTRY_COUNT ?
        k_offsets[i + 1U] - k_offsets[i] : (uint32_t)SYNTH_BYTES - k_offsets[i];
}

/* Builds a synthetic RES* envelope carrying the canonical TITLE.BIN
 * directory framing, with the MAPD record populated in the observed
 * TIBG shape: canonical header fields, five marker cells at
 * 0x40 + k*0x1c04, exactly 3360 filler cells among the 8965 cells, a
 * deterministic non-filler population, and a 32-byte tail of sixteen
 * BE16 words ending in 0xffff. No byte claims tile, map, palette, or
 * colour meaning. */
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
            uint32_t cell;
            uint32_t filler_left = 3360U;
            memcpy(record + 8U, "TIBG", 4U);
            for (j = 0U; j < 13U; ++j) {
                put_be32(record + 0x0cU + j * 4U, k_hdr_fields[j]);
            }
            for (cell = 0U; cell < 8965U; ++cell) {
                uint8_t *at = record + 0x40U + cell * 4U;
                uint32_t rel = cell * 4U;
                if (rel % 0x1c04U == 0U && cell / (0x1c04U / 4U) < 5U &&
                    rel < 5U * 0x1c04U) {
                    at[0] = 0x00U; at[1] = 0x40U; at[2] = 0x00U; at[3] = 0x1cU;
                } else if (filler_left) {
                    --filler_left;
                    at[0] = 0x20U; at[1] = 0x20U; at[2] = 0x12U; at[3] = 0x00U;
                } else {
                    at[0] = 0x99U;
                    at[1] = (uint8_t)(cell >> 8);
                    at[2] = (uint8_t)cell;
                    at[3] = (uint8_t)(0xa5U ^ cell);
                }
            }
            for (j = 0U; j < 16U; ++j) {
                put_be16(record + 0x8c54U + j * 2U,
                         j == 15U ? 0xffffU : (uint16_t)(0x1000U + j * 0x111U));
            }
        } else if (k_class[i] == 1U) {
            record[8] = record[9] = 0x50U;
            put_be16(record + 10U, (uint16_t)(i + 1U));
            put_be16(record + 12U, (uint16_t)(0x20U | i));
            put_be16(record + 14U, (uint16_t)(0x8000U + i));
        } else {
            record[8] = record[9] = 0x70U;
            put_be16(record + 10U, (uint16_t)(i + 1U));
            put_be16(record + 12U, (uint16_t)(0x20U | i));
            put_be16(record + 14U, (uint16_t)(0x8000U + i));
        }
        if (k_class[i] != 2U) {
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
    out_identity->sha256_hex = NEXUS_V1_TITLE_BIN_SHA256;
    out_identity->source_fnv1a64 = fnv1a64(bytes, size);
}

static void check_receipt(const uint8_t *bytes, size_t size)
{
    Nexus_V1_TitleResSourceIdentity identity;
    Nexus_V1_TitleMapdTibgReceipt receipt;
    Nexus_V1_TitleMapdTibgSpanIterator iterator;
    Nexus_V1_TitleMapdTibgSpan span;

    make_identity(bytes, size, &identity);
    CHECK(nexus_v1_title_mapd_tibg_admit(bytes, size, &identity, &receipt) == 1);
    CHECK(receipt.valid && receipt.source_identity_bound &&
          receipt.res_directory_bound && receipt.mapd_head_bound &&
          receipt.header_fields_bound && receipt.marker_chain_bound &&
          receipt.cell_span_bound && receipt.tail_span_bound);
    CHECK(!receipt.tile_proven && !receipt.map_proven &&
          !receipt.palette_proven && !receipt.pixel_decode_permitted &&
          !receipt.draw_permitted && !receipt.presentation_permitted);
    CHECK(receipt.source_fnv1a64 == identity.source_fnv1a64);
    CHECK(receipt.entry_index == 26U && receipt.entry_id == 0U);
    CHECK(receipt.record_offset == 0xe278U);
    CHECK(receipt.record_length == 0x8c74U);
    CHECK(receipt.record_fnv1a64 != 0U);
    CHECK(receipt.header_offset == 0xe278U);
    CHECK(receipt.header_fnv1a64 != 0U);
    CHECK(receipt.header_payload_size_field == 0x8c6cU);
    CHECK(receipt.header_field_0x24 == 0x29020U);
    CHECK(receipt.cell_span_offset == 0xe278U + 0x40U);
    CHECK(receipt.cell_span_length == 8965U * 4U);
    CHECK(receipt.cell_span_fnv1a64 != 0U);
    CHECK(receipt.filler_cell_count == 3360U);
    CHECK(receipt.marker_count == 5U);
    CHECK(receipt.tail_offset == 0xe278U + 0x8c54U);
    CHECK(receipt.tail_fnv1a64 != 0U);
    CHECK(receipt.tail_last_word == 0xffffU);

    CHECK(nexus_v1_title_mapd_tibg_span_iterator_init(&iterator, &receipt) == 0);
    CHECK(nexus_v1_title_mapd_tibg_span_iterator_next(&iterator, &span) == 1);
    CHECK(span.source_offset == receipt.cell_span_offset);
    CHECK(span.source_length == receipt.cell_span_length);
    CHECK(span.source_fnv1a64 == receipt.cell_span_fnv1a64);
    CHECK(nexus_v1_title_mapd_tibg_span_iterator_next(&iterator, &span) == 1);
    CHECK(span.source_offset == receipt.tail_offset);
    CHECK(span.source_length == 32U);
    CHECK(span.source_fnv1a64 == receipt.tail_fnv1a64);
    CHECK(nexus_v1_title_mapd_tibg_span_iterator_next(&iterator, &span) == 0);
}

static void check_rejections(const uint8_t *pristine)
{
    Nexus_V1_TitleResSourceIdentity identity;
    Nexus_V1_TitleMapdTibgReceipt receipt;
    uint8_t *copy;

    make_identity(pristine, SYNTH_BYTES, &identity);

    CHECK(nexus_v1_title_mapd_tibg_admit(NULL, SYNTH_BYTES, &identity, &receipt) == 0);
    CHECK(nexus_v1_title_mapd_tibg_admit(pristine, SYNTH_BYTES, NULL, &receipt) == 0);
    CHECK(nexus_v1_title_mapd_tibg_admit(pristine, SYNTH_BYTES, &identity, NULL) == 0);

    {
        Nexus_V1_TitleResSourceIdentity drifted = identity;
        drifted.source_fnv1a64 ^= UINT64_C(1);
        CHECK(nexus_v1_title_mapd_tibg_admit(pristine, SYNTH_BYTES, &drifted, &receipt) == 0);
    }

    copy = (uint8_t *)malloc(SYNTH_BYTES);
    CHECK(copy != NULL);
    if (!copy) return;

    /* Header field tamper. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[0xe278U + 0x24U] ^= 0x01U;
    make_identity(copy, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_title_mapd_tibg_admit(copy, SYNTH_BYTES, &identity, &receipt) == 0);

    /* Marker cell tamper. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[0xe278U + 0x40U + 2U * 0x1c04U + 1U] ^= 0x01U;
    make_identity(copy, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_title_mapd_tibg_admit(copy, SYNTH_BYTES, &identity, &receipt) == 0);

    /* Filler population drift: one filler cell replaced. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[0xe278U + 0x40U + 4U] ^= 0x01U;
    make_identity(copy, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_title_mapd_tibg_admit(copy, SYNTH_BYTES, &identity, &receipt) == 0);

    /* Tail last-word tamper. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[0xe278U + 0x8c54U + 31U] ^= 0x01U;
    make_identity(copy, SYNTH_BYTES, &identity);
    CHECK(nexus_v1_title_mapd_tibg_admit(copy, SYNTH_BYTES, &identity, &receipt) == 0);

    /* Non-filler cell-byte tamper rebinds the live FNV and moves only the
     * recorded cell-span digest. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[0xe278U + 0x8c54U - 4U] ^= 0x01U;
    {
        uint64_t original_cell_fnv;
        make_identity(pristine, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_title_mapd_tibg_admit(pristine, SYNTH_BYTES, &identity, &receipt) == 1);
        original_cell_fnv = receipt.cell_span_fnv1a64;
        make_identity(copy, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_title_mapd_tibg_admit(copy, SYNTH_BYTES, &identity, &receipt) == 1);
        CHECK(receipt.cell_span_fnv1a64 != original_cell_fnv);
    }
    free(copy);
}

static void check_real_mapd_tile_join(const uint8_t *title_bin,
                                      size_t title_bin_size,
                                      const uint8_t *title_cg,
                                      size_t title_cg_size)
{
    Nexus_TitleScreen title;
    int map;

    memset(&title, 0, sizeof(title));
    CHECK(title_bin && title_cg &&
          title_bin_size > 0x0e278U &&
          nexus_v1_title_decode_mapd(title_bin + 0x0e278U,
                                     title_bin_size - 0x0e278U,
                                     title_cg, title_cg_size,
                                     &title) == 1);
    CHECK(title.decoded_map_source_bound == 1 &&
          title.decoded_map_count == NEXUS_V1_TITLE_MAP_COUNT);
    for (map = 0; map < NEXUS_V1_TITLE_MAP_COUNT; ++map) {
        size_t pixel;
        int nonzero = 0;
        CHECK(title.decoded_map_pixels[map] != NULL);
        if (!title.decoded_map_pixels[map]) continue;
        for (pixel = 0;
             pixel < (size_t)NEXUS_V1_TITLE_MAP_WIDTH *
                         (size_t)NEXUS_V1_TITLE_MAP_HEIGHT;
             ++pixel) {
            if (title.decoded_map_pixels[map][pixel] != 0U) {
                nonzero = 1;
                break;
            }
        }
        CHECK(nonzero);
    }
    /* This join is deliberately source-format evidence only. The decoder
     * does not establish Saturn VDP2 tilemap, CLUT, timing, or presentation
     * ownership. */
    nexus_title_free(&title);
}

int main(int argc, char **argv)
{
    uint8_t *synthetic = build_synthetic();
    if (!synthetic) {
        fprintf(stderr, "allocation failure\n");
        return 1;
    }
    check_receipt(synthetic, SYNTH_BYTES);
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
            check_receipt(real, real_size);
            if (argc > 2) {
                size_t cg_size = 0;
                uint8_t *cg = read_file(argv[2], &cg_size);
                if (!cg) {
                    printf("SKIP: cannot read %s\n", argv[2]);
                    free(real);
                    free(synthetic);
                    return 77;
                }
                check_real_mapd_tile_join(real, real_size, cg, cg_size);
                free(cg);
            }
        }
        free(real);
    }

    free(synthetic);
    if (failures) {
        fprintf(stderr, "FAILURES: %d\n", failures);
        return 1;
    }
    printf("PASS nexus_v1_title_mapd_tibg_admission%s\n", argc > 1 ? " (real)" : " (synthetic)");
    return 0;
}
