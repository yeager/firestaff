#include "nexus_v1_0dmstrt_structure_admission.h"
#include "nexus_v1_iso_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYNTH_BYTES NEXUS_V1_0DMSTRT_BYTES

/* Canonical fixup-table values mirrored from the attested retail
 * structure (provenance facts only, no code/data/relocation meaning). */
static const uint16_t k_head_value[NEXUS_V1_0DMSTRT_HEAD_TABLE_COUNT] = {
    0x363cU, 0x0074U, 0x0894U, 0x0898U, 0x0888U, 0x088cU, 0x0890U
};
static const uint16_t k_table_a_value[NEXUS_V1_0DMSTRT_TABLE_A_COUNT] = {
    0x6e30U, 0x7098U, 0x715cU, 0x71bcU
};
static const uint16_t k_table_b_value[NEXUS_V1_0DMSTRT_TABLE_B_COUNT] = {
    0x5ff8U, 0x5ffcU, 0x6092U, 0x61f2U, 0x61f6U, 0x621cU,
    0x6220U, 0x6224U, 0x624aU, 0x624eU, 0x6252U, 0x6290U
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

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file; long length; uint8_t *bytes;
    *out_size = 0; file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) { if (file) fclose(file); return NULL; }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) { free(bytes); fclose(file); return NULL; }
    fclose(file); *out_size = (size_t)length; return bytes;
}

/* Keep the real-data gate on the native CUE/ISO path when the user's media
 * has not been unpacked.  The member exists only in this process buffer; no
 * game-data file is materialized on disk. */
static uint8_t *read_cue_member(const char *cue_path, size_t *out_size)
{
    Nexus_ISOReader iso;
    const Nexus_ISOFile *member;
    uint8_t *bytes = NULL;
    uint32_t member_size;

    *out_size = 0U;
    memset(&iso, 0, sizeof(iso));
    if (!cue_path || nexus_iso_open_cue(&iso, cue_path) <= 0 ||
        !(member = nexus_iso_find(&iso, "0DMSTRT.BIN")) ||
        member->size != NEXUS_V1_0DMSTRT_BYTES) {
        nexus_iso_close(&iso);
        return NULL;
    }
    member_size = member->size;
    bytes = (uint8_t *)malloc(member_size);
    if (!bytes || nexus_iso_read_file(&iso, member, bytes,
                                      (int)member_size) != (int)member_size) {
        free(bytes);
        nexus_iso_close(&iso);
        return NULL;
    }
    nexus_iso_close(&iso);
    *out_size = member_size;
    return bytes;
}

static void make_identity(const uint8_t *bytes, size_t size,
    Nexus_V1_0DmstrtSourceIdentity *out_identity)
{
    out_identity->sha256_verified = 1;
    out_identity->sha256_hex = NEXUS_V1_0DMSTRT_SHA256;
    out_identity->source_fnv1a64 = fnv1a64(bytes, size);
}

static void put_fixup_table(uint8_t *bytes, uint32_t offset,
    const uint16_t *values, uint32_t count)
{
    uint32_t index;
    for (index = 0U; index < count; ++index) {
        put_be16(bytes + offset + index * 4U, NEXUS_V1_0DMSTRT_FIXUP_TAG);
        put_be16(bytes + offset + index * 4U + 2U, values[index]);
    }
}

static uint8_t *build_synthetic(void)
{
    uint8_t *bytes = (uint8_t *)calloc(SYNTH_BYTES, 1U);
    uint32_t index;
    if (!bytes) return NULL;

    /* Region A: dense bytes with the canonical non-zero population.
     * 113 zeroed positions plus the single 0x00 byte inside the
     * canonical head-table value 0x0074 yield 2216 - 113 - 1 = 2102
     * non-zero bytes; the head-table area [0x56, 0x74) stays clear of
     * the zeroed positions. */
    memset(bytes + NEXUS_V1_0DMSTRT_REGION_A_OFFSET, 0x01,
        NEXUS_V1_0DMSTRT_REGION_A_END - NEXUS_V1_0DMSTRT_REGION_A_OFFSET);
    for (index = 0U; index < 113U; ++index) {
        bytes[0x100U + index * 17U] = 0x00U;
    }
    put_be16(bytes + NEXUS_V1_0DMSTRT_HEAD_SENTINEL_OFFSET,
        NEXUS_V1_0DMSTRT_HEAD_SENTINEL);
    put_fixup_table(bytes, NEXUS_V1_0DMSTRT_HEAD_TABLE_OFFSET,
        k_head_value, NEXUS_V1_0DMSTRT_HEAD_TABLE_COUNT);

    /* Region B: dense bytes with the canonical non-zero population
     * (25400 - 1323 = 24077 non-zero). */
    memset(bytes + NEXUS_V1_0DMSTRT_REGION_B_OFFSET, 0x01,
        NEXUS_V1_0DMSTRT_REGION_B_END - NEXUS_V1_0DMSTRT_REGION_B_OFFSET);
    for (index = 0U; index < 1323U; ++index) {
        bytes[NEXUS_V1_0DMSTRT_REGION_B_OFFSET + index * 19U] = 0x00U;
    }

    /* Tail descriptor: separator, 31-byte printable stamp leading with
     * the class tag, NUL terminator, byte 0x01. */
    bytes[NEXUS_V1_0DMSTRT_TAIL_OFFSET] = NEXUS_V1_0DMSTRT_TAIL_SEPARATOR;
    memcpy(bytes + NEXUS_V1_0DMSTRT_STAMP_OFFSET,
        "GFS_SBL SYNTHETIC MIRROR 000000", NEXUS_V1_0DMSTRT_STAMP_LENGTH);
    bytes[NEXUS_V1_0DMSTRT_ISO_STUB_OFFSET] = 0x00U;
    bytes[NEXUS_V1_0DMSTRT_ISO_STUB_OFFSET + 1U] = 0x01U;

    /* ISO-style stub: standard identifier plus the "." and ".."
     * directory-id bytes with the canonical 0xff population. */
    memcpy(bytes + NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET, "CD001", 5U);
    bytes[NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET + 5U] = 0xffU;
    bytes[NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET + 6U] = 0xffU;
    bytes[NEXUS_V1_0DMSTRT_ISO_DOT_OFFSET] = 0x2eU;
    bytes[NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET + 9U] = 0xffU;
    bytes[NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET + 10U] = 0xffU;
    bytes[NEXUS_V1_0DMSTRT_ISO_DOTDOT_OFFSET] = 0x2eU;
    bytes[NEXUS_V1_0DMSTRT_ISO_DOTDOT_OFFSET + 1U] = 0x2eU;
    bytes[NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET + 14U] = 0xffU;

    /* Fixup tables A and B; gap 3 stays zero. */
    put_fixup_table(bytes, NEXUS_V1_0DMSTRT_TABLE_A_OFFSET,
        k_table_a_value, NEXUS_V1_0DMSTRT_TABLE_A_COUNT);
    put_fixup_table(bytes, NEXUS_V1_0DMSTRT_TABLE_B_OFFSET,
        k_table_b_value, NEXUS_V1_0DMSTRT_TABLE_B_COUNT);
    return bytes;
}

static void check_structure(const uint8_t *bytes, size_t size, int synthetic)
{
    Nexus_V1_0DmstrtSourceIdentity identity;
    Nexus_V1_0DmstrtStructureReceipt receipt;
    Nexus_V1_0DmstrtRegionSpanIterator iterator;
    Nexus_V1_0DmstrtRegionSpan span;
    uint32_t span_total = 0U;
    uint32_t span_count = 0U;

    make_identity(bytes, size, &identity);
    CHECK(nexus_v1_0dmstrt_structure_admit(bytes, size, &identity, &receipt) == 1);
    CHECK(receipt.valid);
    CHECK(receipt.source_identity_bound);
    CHECK(receipt.size_arithmetic_bound);
    CHECK(receipt.region_a_bound);
    CHECK(receipt.gap1_zero_bound);
    CHECK(receipt.region_b_bound);
    CHECK(receipt.gap2_zero_bound);
    CHECK(receipt.tail_descriptor_bound);
    CHECK(receipt.iso_stub_bound);
    CHECK(receipt.head_table_bound);
    CHECK(receipt.fixup_table_a_bound);
    CHECK(receipt.gap3_zero_bound);
    CHECK(receipt.fixup_table_b_bound);
    CHECK(receipt.regions_cover_source);
    CHECK(!receipt.code_proven && !receipt.data_proven &&
        !receipt.relocation_proven && !receipt.load_permitted &&
        !receipt.execution_permitted && !receipt.presentation_permitted);
    CHECK(receipt.region_a_nonzero == NEXUS_V1_0DMSTRT_REGION_A_NONZERO);
    CHECK(receipt.region_b_nonzero == NEXUS_V1_0DMSTRT_REGION_B_NONZERO);
    CHECK(receipt.source_fnv1a64 == fnv1a64(bytes, size));
    CHECK(receipt.region_a_fnv1a64 == fnv1a64(bytes, NEXUS_V1_0DMSTRT_REGION_A_END));

    CHECK(nexus_v1_0dmstrt_region_span_iterator_init(&iterator, &receipt) == 1);
    while (nexus_v1_0dmstrt_region_span_iterator_next(&iterator, &span) == 1) {
        CHECK(span.source_fnv1a64 ==
            fnv1a64(bytes + span.source_offset, span.source_length));
        span_total += span.source_length;
        ++span_count;
    }
    CHECK(span_count == NEXUS_V1_0DMSTRT_REGION_SPAN_COUNT);
    CHECK(span_total == NEXUS_V1_0DMSTRT_BYTES);
    CHECK(nexus_v1_0dmstrt_region_span_iterator_next(NULL, &span) == -1);
    CHECK(nexus_v1_0dmstrt_region_span_iterator_next(&iterator, NULL) == -1);
    CHECK(nexus_v1_0dmstrt_region_span_iterator_init(NULL, &receipt) == 0);
    CHECK(nexus_v1_0dmstrt_region_span_iterator_init(&iterator, NULL) == 0);

    if (synthetic) {
        CHECK(span.source_offset == NEXUS_V1_0DMSTRT_TABLE_B_OFFSET);
    }
}

static void check_rejections(const uint8_t *pristine)
{
    Nexus_V1_0DmstrtSourceIdentity identity;
    Nexus_V1_0DmstrtStructureReceipt receipt;
    uint8_t *copy;

    make_identity(pristine, SYNTH_BYTES, &identity);

    CHECK(nexus_v1_0dmstrt_structure_admit(NULL, SYNTH_BYTES, &identity, &receipt) == 0);
    CHECK(nexus_v1_0dmstrt_structure_admit(pristine, SYNTH_BYTES, NULL, &receipt) == 0);
    CHECK(nexus_v1_0dmstrt_structure_admit(pristine, SYNTH_BYTES, &identity, NULL) == 0);
    CHECK(nexus_v1_0dmstrt_structure_admit(pristine, SYNTH_BYTES - 1U, &identity, &receipt) == 0);

    {
        Nexus_V1_0DmstrtSourceIdentity drifted = identity;
        drifted.source_fnv1a64 ^= UINT64_C(1);
        CHECK(nexus_v1_0dmstrt_structure_admit(pristine, SYNTH_BYTES, &drifted, &receipt) == 0);
    }
    {
        Nexus_V1_0DmstrtSourceIdentity drifted = identity;
        drifted.sha256_hex = "0000000000000000000000000000000000000000000000000000000000000000";
        CHECK(nexus_v1_0dmstrt_structure_admit(pristine, SYNTH_BYTES, &drifted, &receipt) == 0);
    }
    {
        Nexus_V1_0DmstrtSourceIdentity drifted = identity;
        drifted.sha256_verified = 0;
        CHECK(nexus_v1_0dmstrt_structure_admit(pristine, SYNTH_BYTES, &drifted, &receipt) == 0);
    }

    copy = (uint8_t *)malloc(SYNTH_BYTES);
    CHECK(copy != NULL);
    if (!copy) return;

#define TAMPER_REJECT(offset, xor_value) do { \
        memcpy(copy, pristine, SYNTH_BYTES); \
        copy[(offset)] ^= (xor_value); \
        make_identity(copy, SYNTH_BYTES, &identity); \
        CHECK(nexus_v1_0dmstrt_structure_admit(copy, SYNTH_BYTES, &identity, &receipt) == 0); \
    } while (0)

    /* Gap tamper breaks the all-zero bindings. */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_GAP1_OFFSET + 100U, 0x01U);
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_GAP2_OFFSET + 10U, 0x01U);
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_GAP3_OFFSET + 10U, 0x01U);
    /* Non-zero population drift in the dense regions. */
    TAMPER_REJECT(0x200U, 0x01U);          /* region A: 0x01 -> 0x00 */
    TAMPER_REJECT(0x4000U, 0x01U);         /* region B: 0x01 -> 0x00 */
    /* Tail descriptor tamper. */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_TAIL_OFFSET, 0x01U);        /* separator */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_STAMP_OFFSET + 6U, 0x01U);  /* class tag */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_STAMP_OFFSET + 10U, 0xffU); /* printable */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_ISO_STUB_OFFSET, 0x02U);    /* NUL term */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_ISO_STUB_OFFSET + 1U, 0x02U); /* 0x01 */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET + 2U, 0x01U); /* CD001 */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_ISO_DOT_OFFSET, 0x01U);        /* "." */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_ISO_DOTDOT_OFFSET + 1U, 0x01U); /* ".." */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET + 5U, 0x01U); /* ff pop */
    /* Fixup tables. */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_HEAD_SENTINEL_OFFSET, 0x01U);
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_HEAD_TABLE_OFFSET + 1U, 0x01U); /* tag */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_HEAD_TABLE_OFFSET + 2U, 0x01U); /* value */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_TABLE_A_OFFSET, 0x01U);         /* tag */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_TABLE_A_OFFSET + 3U, 0x01U);    /* value */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_TABLE_B_OFFSET + 44U, 0x01U);   /* tag */
    TAMPER_REJECT(NEXUS_V1_0DMSTRT_TABLE_B_OFFSET + 46U, 0x01U);   /* value */

#undef TAMPER_REJECT

    /* Dense-region content tamper that keeps the non-zero population
     * (0x01 -> 0x02) still admits; only the recorded digests move. */
    memcpy(copy, pristine, SYNTH_BYTES);
    copy[0x4000U] = 0x02U;
    {
        uint64_t original_region_fnv;
        make_identity(pristine, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_0dmstrt_structure_admit(pristine, SYNTH_BYTES, &identity, &receipt) == 1);
        original_region_fnv = receipt.region_b_fnv1a64;
        make_identity(copy, SYNTH_BYTES, &identity);
        CHECK(nexus_v1_0dmstrt_structure_admit(copy, SYNTH_BYTES, &identity, &receipt) == 1);
        CHECK(receipt.valid && receipt.region_b_fnv1a64 != original_region_fnv);
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
    check_structure(synthetic, SYNTH_BYTES, 1);
    check_rejections(synthetic);

    if (argc > 1) {
        size_t real_size = 0;
        int loaded_from_cue = argc == 3 && strcmp(argv[1], "--cue") == 0;
        uint8_t *real = loaded_from_cue ? read_cue_member(argv[2], &real_size) :
            (argc == 2 ? read_file(argv[1], &real_size) : NULL);
        if (!real) {
            printf("SKIP: cannot read %s\n", loaded_from_cue ? argv[2] : argv[1]);
            free(synthetic);
            return 77;
        }
        if (loaded_from_cue && fnv1a64(real, real_size) !=
                UINT64_C(0xf00bae379d7c2e54)) {
            fprintf(stderr, "FAIL: CUE 0DMSTRT.BIN identity mismatch\n");
            free(real);
            free(synthetic);
            return 1;
        }
        CHECK(real_size == (size_t)NEXUS_V1_0DMSTRT_BYTES);
        if (real_size == (size_t)NEXUS_V1_0DMSTRT_BYTES) {
            check_structure(real, real_size, 0);
        }
        free(real);
    }

    free(synthetic);
    if (failures) {
        fprintf(stderr, "FAILURES: %d\n", failures);
        return 1;
    }
    printf("PASS nexus_v1_0dmstrt_structure_admission%s\n", argc > 1 ? " (real)" : " (synthetic)");
    return 0;
}
