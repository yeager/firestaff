/*
 * test_csb_v1_csbgraphics_dat_inventory_pc34_compat.c
 *
 * Data-free contract tests for the CSBWin "CSBgraphics.dat"
 * custom-graphics shape classifier + inventory walker.
 *
 * Scope:
 *   - On-disk shape classification: distinguish CSBgraphics.dat
 *     from DM1 raw saves (RDMCSB15), CSB v2.0/v2.1 saves
 *     (CSBGAME\0), CSBWin 512-byte XOR-pad headers (CSB\1,
 *     DM\0\1, CEDT), too-small input, and unknown shapes. Each
 *     shape names a documented CSBWin / DM1 / CSB source file.
 *   - Inventory walker: walk the parallel size tables once,
 *     populate sparse / dense / zero-length / identical counts
 *     plus payload tail and end-aligned invariant, and verify
 *     the report matches a hand-derived reference.
 *   - Convenience from-bytes helper: classify + inventory in one
 *     call, including error propagation for malformed bytes.
 *   - Source-evidence string is non-empty.
 *
 * Non-claims:
 *   - No real CSBgraphics.dat is loaded.
 *   - No LZW / RLE decompression, no payload decode, no M11
 *     viewport override hook.
 *   - No claim of full CSBWin custom-resource support.
 */

#include "csb_v1_csbgraphics_dat_inventory_pc34_compat.h"
#include "csb_v1_csbgraphics_dat_classify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                                 \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); }                      \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); }                      \
} while (0)

#define ASSERT_TRUE(cond) do {                                                \
    if (!(cond)) {                                                            \
        fprintf(stderr, "ASSERTION FAILED at %s:%d: %s\n",                    \
                __FILE__, __LINE__, #cond);                                   \
        return 0;                                                             \
    }                                                                         \
} while (0)

/* ── Fixture helpers ──────────────────────────────────────────────── */

static void write_be16(uint8_t *buf, size_t off, uint16_t value)
{
    buf[off]     = (uint8_t)((value >> 8) & 0xffu);
    buf[off + 1] = (uint8_t)(value & 0xffu);
}

static void write_le16(uint8_t *buf, size_t off, uint16_t value)
{
    buf[off]     = (uint8_t)(value & 0xffu);
    buf[off + 1] = (uint8_t)((value >> 8) & 0xffu);
}

/* Build a big-endian CSBgraphics.dat-shape buffer with `count`
 * graphics, all entries sized `comp`/`deco`. The fixture ends
 * inside the payload region (no trailing signature bytes). */
static uint8_t *build_be(size_t count, uint16_t comp, uint16_t deco,
                         size_t *out_size)
{
    size_t tables = (size_t)count * 4u;
    size_t pad = (size_t)comp * count + 64u;
    if (pad < 4096u) pad = 4096u;
    size_t total = 2u + tables + pad;
    uint8_t *buf = (uint8_t *)calloc(1u, total);
    size_t i;
    if (!buf) return NULL;
    buf[0] = (uint8_t)((count >> 8) & 0xffu);
    buf[1] = (uint8_t)(count & 0xffu);
    for (i = 0u; i < count; ++i) {
        size_t comp_off = 2u + i * 2u;
        size_t deco_off = 2u + tables / 2u + i * 2u;
        write_be16(buf, comp_off, comp);
        write_be16(buf, deco_off, deco);
    }
    *out_size = total;
    return buf;
}

/* Build a little-endian-marker CSBgraphics.dat-shape buffer. */
static uint8_t *build_le(size_t count, uint16_t comp, uint16_t deco,
                         size_t *out_size)
{
    size_t tables = (size_t)count * 4u;
    size_t pad = (size_t)comp * count + 64u;
    if (pad < 4096u) pad = 4096u;
    size_t total = 4u + tables + pad;
    uint8_t *buf = (uint8_t *)calloc(1u, total);
    size_t i;
    if (!buf) return NULL;
    buf[0] = 0x80u;
    buf[1] = 0x01u;
    buf[2] = (uint8_t)(count & 0xffu);
    buf[3] = (uint8_t)((count >> 8) & 0xffu);
    for (i = 0u; i < count; ++i) {
        size_t comp_off = 4u + i * 2u;
        size_t deco_off = 4u + tables / 2u + i * 2u;
        write_le16(buf, comp_off, comp);
        write_le16(buf, deco_off, deco);
    }
    *out_size = total;
    return buf;
}

/* ── Shape classification tests ──────────────────────────────────── */

static int test_shape_csbgraphics_be(void)
{
    size_t size = 0u;
    uint8_t *buf = build_be(5u, 0x0010u, 0x0020u, &size);
    ASSERT_TRUE(buf != NULL);
    CHECK(csb_v1_csbgraphics_dat_shape_classify(buf, size) ==
          CSB_V1_CSBGRAPHICS_SHAPE_CSBGRAPHICS,
          "big-endian CSBgraphics.dat header classified as CSBGRAPHICS");
    free(buf);
    return 1;
}

static int test_shape_csbgraphics_le(void)
{
    size_t size = 0u;
    uint8_t *buf = build_le(3u, 0x0008u, 0x0010u, &size);
    ASSERT_TRUE(buf != NULL);
    CHECK(csb_v1_csbgraphics_dat_shape_classify(buf, size) ==
          CSB_V1_CSBGRAPHICS_SHAPE_CSBGRAPHICS,
          "LE-marker CSBgraphics.dat header classified as CSBGRAPHICS");
    free(buf);
    return 1;
}

static int test_shape_dm1_raw_rdmcsb15(void)
{
    uint8_t buf[16];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, "RDMCSB15", 8);
    buf[8] = 0xAAu; buf[9] = 0xBBu;
    CHECK(csb_v1_csbgraphics_dat_shape_classify(buf, sizeof(buf)) ==
          CSB_V1_CSBGRAPHICS_SHAPE_DM1_RAW_RDMCSB15,
          "RDMCSB15 magic classified as DM1 raw save shape");
    return 1;
}

static int test_shape_csb_save_csbgame(void)
{
    uint8_t buf[16];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, "CSBGAME\0", 8);
    buf[8] = 0x00u; buf[9] = 0x02u; /* v2.0 */
    CHECK(csb_v1_csbgraphics_dat_shape_classify(buf, sizeof(buf)) ==
          CSB_V1_CSBGRAPHICS_SHAPE_CSB_SAVE_CSBGAME,
          "CSBGAME\\0 magic classified as CSB save shape");
    return 1;
}

static int test_shape_csbwin_512_csb1(void)
{
    uint8_t buf[512];
    memset(buf, 0, sizeof(buf));
    buf[0] = 'C'; buf[1] = 'S'; buf[2] = 'B'; buf[3] = 0x01u;
    CHECK(csb_v1_csbgraphics_dat_shape_classify(buf, sizeof(buf)) ==
          CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_CSB1,
          "CSB\\1 magic classified as CSBWin 512-byte CSB1 shape");
    return 1;
}

static int test_shape_csbwin_512_dm01(void)
{
    uint8_t buf[512];
    memset(buf, 0, sizeof(buf));
    buf[0] = 'D'; buf[1] = 'M'; buf[2] = 0x00u; buf[3] = 0x01u;
    CHECK(csb_v1_csbgraphics_dat_shape_classify(buf, sizeof(buf)) ==
          CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_DM01,
          "DM\\0\\1 magic classified as CSBWin 512-byte DM01 shape");
    return 1;
}

static int test_shape_csbwin_512_cedt(void)
{
    uint8_t buf[512];
    memset(buf, 0, sizeof(buf));
    buf[0] = 'C'; buf[1] = 'E'; buf[2] = 'D'; buf[3] = 'T';
    CHECK(csb_v1_csbgraphics_dat_shape_classify(buf, sizeof(buf)) ==
          CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_CEDT,
          "CEDT magic classified as CSBWin 512-byte CEDT shape");
    return 1;
}

static int test_shape_too_small(void)
{
    uint8_t buf[2] = { 0x80u, 0x01u };
    CHECK(csb_v1_csbgraphics_dat_shape_classify(buf, 0u) ==
          CSB_V1_CSBGRAPHICS_SHAPE_TOO_SMALL,
          "size=0 buffer classified as TOO_SMALL");
    CHECK(csb_v1_csbgraphics_dat_shape_classify(buf, 1u) ==
          CSB_V1_CSBGRAPHICS_SHAPE_TOO_SMALL,
          "size=1 buffer classified as TOO_SMALL");
    CHECK(csb_v1_csbgraphics_dat_shape_classify(buf, sizeof(buf)) ==
          CSB_V1_CSBGRAPHICS_SHAPE_TOO_SMALL,
          "size=2 buffer classified as TOO_SMALL");
    CHECK(csb_v1_csbgraphics_dat_shape_classify(NULL, 16u) ==
          CSB_V1_CSBGRAPHICS_SHAPE_TOO_SMALL,
          "NULL buffer classified as TOO_SMALL");
    return 1;
}

static int test_shape_unknown(void)
{
    uint8_t buf[16];
    memset(buf, 0x7Eu, sizeof(buf));
    CHECK(csb_v1_csbgraphics_dat_shape_classify(buf, sizeof(buf)) ==
          CSB_V1_CSBGRAPHICS_SHAPE_UNKNOWN,
          "0x7E7E7E... buffer classified as UNKNOWN");
    return 1;
}

static int test_shape_names_non_null(void)
{
    int i;
    int all_non_null = 1;
    for (i = 0; i < (int)CSB_V1_CSBGRAPHICS_SHAPE_COUNT; ++i) {
        const char *n = csb_v1_csbgraphics_dat_shape_name(
            (CSB_V1_CSBGraphicsShape)i);
        if (!n) all_non_null = 0;
    }
    CHECK(all_non_null, "every CSB_V1_CSBGraphicsShape has a non-NULL name");
    return 1;
}

/* ── Inventory walker tests ──────────────────────────────────────── */

static int test_inventory_all_zero_length(void)
{
    size_t size = 0u;
    /* 4 entries, all comp=0 deco=0 ⇒ every byte after the tables
     * is unused (build_be pads with 4096 bytes of zero). */
    uint8_t *buf = build_be(4u, 0u, 0u, &size);
    CSB_V1_CSBGraphicsIndex index;
    CSB_V1_CSBGraphicsInventory inv;
    int rc;
    ASSERT_TRUE(buf != NULL);
    rc = csb_v1_csbgraphics_dat_inventory_from_bytes(
        buf, size, &index, &inv);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_INVENTORY_OK);
    ASSERT_TRUE(index.count == 4u);
    ASSERT_TRUE(inv.count == 4u);
    ASSERT_TRUE(inv.zero_length_count == 4u);
    ASSERT_TRUE(inv.sparse_count == 0u);
    ASSERT_TRUE(inv.dense_count == 0u);
    ASSERT_TRUE(inv.identical_count == 0u);
    ASSERT_TRUE(inv.total_compressed == 0u);
    ASSERT_TRUE(inv.payload_used == 0u);
    ASSERT_TRUE(inv.payload_tail_bytes == inv.payload_avail);
    CHECK(inv.end_aligned == 0,
          "all-zero fixture with pad reports end_aligned=0");
    free(buf);
    return 1;
}

static int test_inventory_all_dense_equal(void)
{
    size_t size = 0u;
    /* 3 entries, comp=4 deco=4 ⇒ total_compressed = 12.
     * build_be pads with 4096 bytes minimum so payload_tail is
     * 4096 - 12 = 4084, end_aligned = 0. */
    uint8_t *buf = build_be(3u, 4u, 4u, &size);
    CSB_V1_CSBGraphicsIndex index;
    CSB_V1_CSBGraphicsInventory inv;
    int rc;
    ASSERT_TRUE(buf != NULL);
    rc = csb_v1_csbgraphics_dat_inventory_from_bytes(
        buf, size, &index, &inv);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_INVENTORY_OK);
    ASSERT_TRUE(inv.count == 3u);
    ASSERT_TRUE(inv.dense_count == 3u);
    ASSERT_TRUE(inv.zero_length_count == 0u);
    ASSERT_TRUE(inv.sparse_count == 0u);
    ASSERT_TRUE(inv.identical_count == 3u); /* comp == deco */
    ASSERT_TRUE(inv.total_compressed == 12u);
    ASSERT_TRUE(inv.payload_used == 12u);
    CHECK(inv.end_aligned == 0,
          "dense-equal fixture with pad reports end_aligned=0");
    free(buf);
    return 1;
}

static int test_inventory_mixed(void)
{
    size_t size = 0u;
    /* 6 entries with mixed comp/deco:
     *   [0] comp=0  deco=0   → zero-length
     *   [1] comp=0  deco=10  → sparse
     *   [2] comp=5  deco=50  → dense
     *   [3] comp=10 deco=10  → dense + identical
     *   [4] comp=5  deco=0   → dense (impossible-looking but valid)
     *   [5] comp=8  deco=8   → dense + identical
     * total_compressed = 28, dense=4, sparse=1, zero-length=1,
     * identical=2 (entries 3 and 5). */
    uint8_t *buf = build_be(6u, 1u, 1u, &size);
    CSB_V1_CSBGraphicsIndex index;
    CSB_V1_CSBGraphicsInventory inv;
    int rc;
    ASSERT_TRUE(buf != NULL);
    /* Mutate the parallel size tables directly. */
    write_be16(buf, 2u +  0u, 0u);  write_be16(buf, 14u +  0u, 0u);
    write_be16(buf, 2u +  2u, 0u);  write_be16(buf, 14u +  2u, 10u);
    write_be16(buf, 2u +  4u, 5u);  write_be16(buf, 14u +  4u, 50u);
    write_be16(buf, 2u +  6u, 10u); write_be16(buf, 14u +  6u, 10u);
    write_be16(buf, 2u +  8u, 5u);  write_be16(buf, 14u +  8u, 0u);
    write_be16(buf, 2u + 10u, 8u);  write_be16(buf, 14u + 10u, 8u);
    rc = csb_v1_csbgraphics_dat_inventory_from_bytes(
        buf, size, &index, &inv);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_INVENTORY_OK);
    ASSERT_TRUE(inv.count == 6u);
    ASSERT_TRUE(inv.dense_count == 4u);
    ASSERT_TRUE(inv.sparse_count == 1u);
    ASSERT_TRUE(inv.zero_length_count == 1u);
    ASSERT_TRUE(inv.identical_count == 2u);
    ASSERT_TRUE(inv.total_compressed == 28u);
    ASSERT_TRUE(inv.payload_used == 28u);
    CHECK(inv.end_aligned == 0,
          "mixed fixture with pad reports end_aligned=0");
    free(buf);
    return 1;
}

static int test_inventory_payload_tail_arithmetic(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    CSB_V1_CSBGraphicsInventory inv;
    int rc;
    /* Hand-built index: 2 entries, total_compressed=8,
     * payload_offset=18, payload_avail=20 ⇒ payload_tail=12,
     * end_aligned=0. */
    memset(&idx, 0, sizeof(idx));
    idx.count = 2u;
    idx.total_compressed = 8u;
    idx.payload_offset = 18u;
    idx.payload_bytes_avail = 20u;
    idx.max_compressed = 4u;
    idx.byte_order = CSB_V1_CSBGRAPHICS_BYTE_ORDER_BIG_ENDIAN;
    rc = csb_v1_csbgraphics_dat_inventory(&idx, &inv);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_INVENTORY_OK);
    ASSERT_TRUE(inv.payload_used == 8u);
    ASSERT_TRUE(inv.payload_avail == 20u);
    ASSERT_TRUE(inv.payload_tail_bytes == 12u);
    CHECK(inv.end_aligned == 0,
          "tail=12 fixture reports end_aligned=0");
    return 1;
}

static int test_inventory_end_aligned(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    CSB_V1_CSBGraphicsInventory inv;
    int rc;
    /* Strict CSBWin contract: total_compressed == payload_avail
     * ⇒ end_aligned = 1, payload_tail = 0. */
    memset(&idx, 0, sizeof(idx));
    idx.count = 1u;
    idx.total_compressed = 100u;
    idx.payload_offset = 6u;
    idx.payload_bytes_avail = 100u;
    idx.max_compressed = 100u;
    idx.byte_order = CSB_V1_CSBGRAPHICS_BYTE_ORDER_BIG_ENDIAN;
    rc = csb_v1_csbgraphics_dat_inventory(&idx, &inv);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_INVENTORY_OK);
    ASSERT_TRUE(inv.payload_tail_bytes == 0u);
    CHECK(inv.end_aligned == 1,
          "tail=0 fixture reports end_aligned=1");
    return 1;
}

static int test_inventory_rejects_null(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    CSB_V1_CSBGraphicsInventory inv;
    uint8_t buf[16];
    CHECK(csb_v1_csbgraphics_dat_inventory(NULL, &inv) ==
          CSB_V1_CSBGRAPHICS_INVENTORY_ERR_ARGUMENT,
          "inventory(NULL, ...) returns ERR_ARGUMENT");
    CHECK(csb_v1_csbgraphics_dat_inventory(&idx, NULL) ==
          CSB_V1_CSBGRAPHICS_INVENTORY_ERR_ARGUMENT,
          "inventory(..., NULL) returns ERR_ARGUMENT");
    CHECK(csb_v1_csbgraphics_dat_inventory_from_bytes(NULL, 16, &idx, &inv) ==
          CSB_V1_CSBGRAPHICS_INVENTORY_ERR_ARGUMENT,
          "from_bytes(NULL, ...) returns ERR_ARGUMENT");
    CHECK(csb_v1_csbgraphics_dat_inventory_from_bytes(buf, sizeof(buf), NULL, &inv) ==
          CSB_V1_CSBGRAPHICS_INVENTORY_ERR_ARGUMENT,
          "from_bytes(..., NULL index) returns ERR_ARGUMENT");
    CHECK(csb_v1_csbgraphics_dat_inventory_from_bytes(buf, sizeof(buf), &idx, NULL) ==
          CSB_V1_CSBGRAPHICS_INVENTORY_ERR_ARGUMENT,
          "from_bytes(..., NULL inv) returns ERR_ARGUMENT");
    return 1;
}

static int test_inventory_rejects_malformed_bytes(void)
{
    CSB_V1_CSBGraphicsIndex index;
    CSB_V1_CSBGraphicsInventory inv;
    uint8_t buf[3] = { 0x80u, 0x01u, 0x05u }; /* LE marker, too-small */
    CHECK(csb_v1_csbgraphics_dat_inventory_from_bytes(
              buf, sizeof(buf), &index, &inv) ==
          CSB_V1_CSBGRAPHICS_INVENTORY_ERR_CLASSIFY,
          "from_bytes with truncated LE-marker header returns ERR_CLASSIFY");
    return 1;
}

static int test_inventory_source_evidence(void)
{
    const char *ev = csb_v1_csbgraphics_dat_inventory_source_evidence();
    int has_redmcsb = 0;
    int has_csbwin = 0;
    int has_gap_list = 0;
    if (ev) {
        if (strstr(ev, "ReDMCSB") != NULL) has_redmcsb = 1;
        if (strstr(ev, "CSBWin") != NULL) has_csbwin = 1;
        if (strstr(ev, "FIRESTAFF_GAP_LIST") != NULL) has_gap_list = 1;
    }
    CHECK(ev != NULL,
          "source-evidence string is non-NULL");
    CHECK(has_redmcsb,
          "source-evidence cites at least one ReDMCSB source");
    CHECK(has_csbwin,
          "source-evidence cites at least one CSBWin source");
    CHECK(has_gap_list,
          "source-evidence points at docs/FIRESTAFF_GAP_LIST.md");
    return 1;
}

static int test_inventory_le_marker_path(void)
{
    size_t size = 0u;
    /* 2 entries in LE-marker form, comp=4 deco=4 ⇒ identical=2. */
    uint8_t *buf = build_le(2u, 4u, 4u, &size);
    CSB_V1_CSBGraphicsIndex index;
    CSB_V1_CSBGraphicsInventory inv;
    int rc;
    ASSERT_TRUE(buf != NULL);
    rc = csb_v1_csbgraphics_dat_inventory_from_bytes(
        buf, size, &index, &inv);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_INVENTORY_OK);
    ASSERT_TRUE(index.byte_order ==
                CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER);
    ASSERT_TRUE(inv.count == 2u);
    ASSERT_TRUE(inv.dense_count == 2u);
    ASSERT_TRUE(inv.identical_count == 2u);
    ASSERT_TRUE(inv.total_compressed == 8u);
    free(buf);
    return 1;
}

int main(void)
{
    int ok = 1;
    printf("=== CSB V1 CSBgraphics.dat shape + inventory walker ===\n\n");

    /* ── Shape classification ── */
    ok &= test_shape_csbgraphics_be();
    ok &= test_shape_csbgraphics_le();
    ok &= test_shape_dm1_raw_rdmcsb15();
    ok &= test_shape_csb_save_csbgame();
    ok &= test_shape_csbwin_512_csb1();
    ok &= test_shape_csbwin_512_dm01();
    ok &= test_shape_csbwin_512_cedt();
    ok &= test_shape_too_small();
    ok &= test_shape_unknown();
    ok &= test_shape_names_non_null();

    /* ── Inventory walker ── */
    ok &= test_inventory_all_zero_length();
    ok &= test_inventory_all_dense_equal();
    ok &= test_inventory_mixed();
    ok &= test_inventory_payload_tail_arithmetic();
    ok &= test_inventory_end_aligned();
    ok &= test_inventory_rejects_null();
    ok &= test_inventory_rejects_malformed_bytes();
    ok &= test_inventory_le_marker_path();

    /* ── Source evidence ── */
    ok &= test_inventory_source_evidence();

    printf("\n--- %d/%d PASS ---\n", g_pass, g_pass + g_fail);
    if (!ok) {
        printf("FAIL\n");
        return 1;
    }
    printf("OK\n");
    return 0;
}
