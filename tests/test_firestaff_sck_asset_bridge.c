/*
 * test_firestaff_sck_asset_bridge.c
 *
 * CTest for the bounded Greatstone/SCK mapfile -> Firestaff asset-loader
 * bridge.  Covers:
 *   - Parsing a synthetic `_mapping.xml` with multiple <map> rows
 *     (one self-closing, one with nested <game id="..."/> children).
 *   - Lookup by MD5 only, by file only, and by both.
 *   - Selecting a single asset slice by item number with a type
 *     prefix filter (IMG).
 *   - RAW selector visibility plus bounded RAW identity decoder handoff.
 *   - Selecting by description substring.
 *   - Bounded rejection paths: unknown MD5, oversized slice,
 *     missing SIZE attribute, malformed mapfile.
 *
 * The test is data-free: it never touches the real Greatstone corpus
 * or any Firestaff asset file.  Real-asset handoff is tracked
 * separately under docs/FIRESTAFF_GAP_LIST.md A2.
 */

#include "firestaff_sck_asset_bridge.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++g_failures; \
    } \
} while (0)

#define CHECK_STR_EQ(actual, expected) do { \
    if (strcmp((actual), (expected)) != 0) { \
        fprintf(stderr, \
                "CHECK failed at %s:%d: \"%s\" != \"%s\"\n", \
                __FILE__, __LINE__, (actual), (expected)); \
        ++g_failures; \
    } \
} while (0)

/* Synthetic `_mapping.xml` mirroring the shape that ships inside
 * `sck.jar@1.5.1` at `db/map/_mapping.xml`. */
static const char* kMappingXml =
    "<?xml version='1.0' encoding='utf-8'?>\n"
    "<mapping>\n"
    "  <default_parameters>\n"
    "    <strings_default>dm_en.xml</strings_default>\n"
    "  </default_parameters>\n"
    "<maps>\n"
    "  <!-- DM Atari demo executable: IMG1 graphics -->\n"
    "  <map md5=\"AAAA1111AAAA1111AAAA1111AAAA1111\"\n"
    "    path=\"dm_atari_demo.map\"\n"
    "    file=\"game.prg\"\n"
    "    date=\"01-NOV-2010\">\n"
    "    <games>\n"
    "      <game id=\"DM_Atari_demo\"/>\n"
    "    </games>\n"
    "  </map>\n"
    "  <!-- DM PC 3.4 graphics.dat: legacy non-sized IMG3 rows -->\n"
    "  <map md5=\"BBBB2222BBBB2222BBBB2222BBBB2222\"\n"
    "    path=\"dm_pc43_en_gd.map\"\n"
    "    file=\"graphics.dat\"\n"
    "    date=\"01-JAN-2009\">\n"
    "    <games>\n"
    "      <game id=\"DM_PC_34_en\"/>\n"
    "      <game id=\"DM_PC_34_fr\"/>\n"
    "    </games>\n"
    "  </map>\n"
    "  <!-- Self-closing <map .../> row, no nested games. -->\n"
    "  <map md5=\"CCCC3333CCCC3333CCCC3333CCCC3333\"\n"
    "    path=\"dummy.map\"\n"
    "    file=\"dummy.bin\"\n"
    "    date=\"01-JAN-2010\"/>\n"
    "</maps>\n"
    "</mapping>\n";

/* Real-shape DM Atari demo SCK 2.x mapfile, abridged from the corpus. */
static const char* kAtariDemoMap =
    "MAPFORMATVERSION=2.0,MAPVERSION=1.0,FORMAT=EXE,ENDIAN=BIG,CLOCKMODE=PAL\n"
    "# Author: Pierre Monnot\n"
    "# Creation: 01-NOV-2010\n"
    "000000,RAW1,SIZE=12288,Unknown,Not yet decoded,\n"
    "012288,IMG1,SIZE=2560,Dungeon Graphics,Ceiling,\n"
    "014848,IMG1,SIZE=68,Dungeon Graphics,Floor Pit Left Side 3,\n"
    "014916,IMG1,SIZE=62,Dungeon Graphics,Floor Pit Front 3,\n"
    "015434,IMG1,SIZE=532,Dungeon Graphics,Floor Pit Front 1,\n"
    "100000,IMG3,PAL1,Interface - Dialog Box,,\n"
    "100001,RAW1,NULL,Code,Not yet decoded,\n";

/* Smaller mapfile that only has unsized IMG3 entries (PC 3.4 style). */
static const char* kPc34Map =
    "ENDIAN=LITTLE,FORMAT=DMCSB2\n"
    "0000,IMG3,PAL1,Interface - Dialog Box,,\n"
    "0001,IMG3,PAL=DM_PAL_TITLE,Interface - Main Title,,\n"
    "0017,IMG3,PAL1,Interface - Character Sheet,,\n";

static void test_mapping_xml_parse(void) {
    FirestaffSckBridgeMapping mapping;
    FirestaffSckBridgeResult r;

    memset(&mapping, 0, sizeof(mapping));
    r = FirestaffSckBridge_ParseMappingXml(kMappingXml, &mapping);
    CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
    CHECK(mapping.rowCount == 3u);

    CHECK_STR_EQ(mapping.rows[0].md5, "AAAA1111AAAA1111AAAA1111AAAA1111");
    CHECK_STR_EQ(mapping.rows[0].path, "dm_atari_demo.map");
    CHECK_STR_EQ(mapping.rows[0].file, "game.prg");
    CHECK(mapping.rows[0].gameCount == 1u);
    CHECK_STR_EQ(mapping.rows[0].games[0].id, "DM_Atari_demo");

    CHECK_STR_EQ(mapping.rows[1].md5, "BBBB2222BBBB2222BBBB2222BBBB2222");
    CHECK_STR_EQ(mapping.rows[1].path, "dm_pc43_en_gd.map");
    CHECK_STR_EQ(mapping.rows[1].file, "graphics.dat");
    CHECK(mapping.rows[1].gameCount == 2u);
    CHECK_STR_EQ(mapping.rows[1].games[1].id, "DM_PC_34_fr");

    CHECK_STR_EQ(mapping.rows[2].md5, "CCCC3333CCCC3333CCCC3333CCCC3333");
    CHECK_STR_EQ(mapping.rows[2].path, "dummy.map");
    CHECK_STR_EQ(mapping.rows[2].file, "dummy.bin");
    CHECK(mapping.rows[2].gameCount == 0u);
}

static void test_mapping_xml_rejections(void) {
    FirestaffSckBridgeMapping mapping;
    FirestaffSckBridgeResult r;

    r = FirestaffSckBridge_ParseMappingXml(NULL, &mapping);
    CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_NULL_ARG);

    r = FirestaffSckBridge_ParseMappingXml("not xml at all\n", &mapping);
    CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
    CHECK(mapping.rowCount == 0u);
}

static void test_lookup_combinations(void) {
    FirestaffSckBridgeMapping mapping;
    const FirestaffSckBridgeMappingRow* row = NULL;
    FirestaffSckBridgeResult r;

    memset(&mapping, 0, sizeof(mapping));
    CHECK(FirestaffSckBridge_ParseMappingXml(kMappingXml, &mapping) ==
          FIRESTAFF_SCK_BRIDGE_OK);

    /* MD5 + file exact match. */
    r = FirestaffSckBridge_Lookup(&mapping,
                                  "AAAA1111AAAA1111AAAA1111AAAA1111",
                                  "game.prg",
                                  &row);
    CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
    CHECK(row != NULL);
    CHECK_STR_EQ(row->path, "dm_atari_demo.map");

    /* Lowercase MD5 must still match (case-insensitive). */
    r = FirestaffSckBridge_Lookup(&mapping,
                                  "aaaa1111aaaa1111aaaa1111aaaa1111",
                                  "GAME.PRG",
                                  &row);
    CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
    CHECK(row != NULL);

    /* File-only lookup. */
    r = FirestaffSckBridge_Lookup(&mapping, NULL, "graphics.dat", &row);
    CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
    CHECK_STR_EQ(row->md5, "BBBB2222BBBB2222BBBB2222BBBB2222");

    /* MD5-only lookup. */
    r = FirestaffSckBridge_Lookup(&mapping,
                                  "CCCC3333CCCC3333CCCC3333CCCC3333",
                                  NULL,
                                  &row);
    CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
    CHECK_STR_EQ(row->path, "dummy.map");

    /* Unknown MD5 + file combo. */
    r = FirestaffSckBridge_Lookup(&mapping,
                                  "DEAD1111DEAD1111DEAD1111DEAD1111",
                                  "game.prg",
                                  &row);
    CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_NOT_FOUND);

    /* Empty filters are rejected. */
    r = FirestaffSckBridge_Lookup(&mapping, NULL, NULL, &row);
    CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_NOT_FOUND);

    r = FirestaffSckBridge_Lookup(NULL, "AAAA1111AAAA1111AAAA1111AAAA1111", "x", &row);
    CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_NULL_ARG);
}

static void test_select_slice_by_number(void) {
    FirestaffSckBridgeSelection sel;
    char err[128];
    FirestaffSckBridgeResult r;

    memset(&sel, 0, sizeof(sel));
    memset(err, 0, sizeof(err));

    /* Real DM Atari demo target file would be ~165 KB; we synthesize
     * 200 KB so the largest sized slice (12288) still fits. */
    r = FirestaffSckBridge_SelectSlice(kAtariDemoMap,
                                       "012288",
                                       "IMG",
                                       200000u,
                                       &sel,
                                       err,
                                       sizeof(err));
    CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
    CHECK_STR_EQ(sel.itemNumber, "012288");
    CHECK_STR_EQ(sel.itemType, "IMG1");
    CHECK(sel.slice.offset == 12288u);
    CHECK(sel.slice.size == 2560u);
    CHECK(sel.hasNumericNumber == 1);
    CHECK(sel.hasSizeBytes == 1);
    CHECK(sel.slice.offset + sel.slice.size <= 200000u);

    /* Filter rejects unsized IMG3 rows. */
    r = FirestaffSckBridge_SelectSlice(kAtariDemoMap,
                                       "100000",
                                       "IMG",
                                       200000u,
                                       &sel,
                                       err,
                                       sizeof(err));
    CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_NOT_SIZED);

    /* Out-of-bounds slice is rejected. */
    r = FirestaffSckBridge_SelectSlice(kAtariDemoMap,
                                       "012288",
                                       "IMG",
                                       10000u,
                                       &sel,
                                       err,
                                       sizeof(err));
    CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_SLICE_OUT_OF_BOUNDS);

    /* Unknown item number. */
    r = FirestaffSckBridge_SelectSlice(kAtariDemoMap,
                                       "999999",
                                       "IMG",
                                       200000u,
                                       &sel,
                                       err,
                                       sizeof(err));
    CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_NOT_FOUND);

    /* A sized item with the wrong type prefix is a clean miss, not
     * NOT_SIZED.  This keeps RAW/PAL/SND decoder gates from accepting
     * an IMG row by number alone. */
    r = FirestaffSckBridge_SelectSlice(kAtariDemoMap,
                                       "012288",
                                       "RAW",
                                       200000u,
                                       &sel,
                                       err,
                                       sizeof(err));
    CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_NOT_FOUND);
}

static uint32_t test_checksum32_fnv1a(const uint8_t* bytes, uint32_t byteCount) {
    uint32_t h = 2166136261u;
    uint32_t i;
    for (i = 0u; i < byteCount; ++i) {
        h ^= (uint32_t)bytes[i];
        h *= 16777619u;
    }
    return h;
}

static void test_raw_decoder_handoff(void) {
    FirestaffSckBridgeSelection sel;
    FirestaffSckBridgeRawHandoff raw;
    uint8_t asset[256];
    char err[128];
    FirestaffSckBridgeResult r;
    unsigned int i;

    for (i = 0u; i < sizeof(asset); ++i) {
        asset[i] = (uint8_t)((i * 17u + 3u) & 0xffu);
    }

    memset(&sel, 0, sizeof(sel));
    memset(&raw, 0, sizeof(raw));
    memset(err, 0, sizeof(err));
    r = FirestaffSckBridge_SelectSlice(kAtariDemoMap,
                                       "000000",
                                       "RAW",
                                       200000u,
                                       &sel,
                                       err,
                                       sizeof(err));
    CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
    CHECK_STR_EQ(sel.itemNumber, "000000");
    CHECK_STR_EQ(sel.itemType, "RAW1");
    CHECK(sel.slice.offset == 0u);
    CHECK(sel.slice.size == 12288u);

    /* Use a compact second map to prove the handoff without allocating
     * corpus-scale bytes or storing any Greatstone/original payload. */
    {
        static const char* kTinyRawMap =
            "MAPFORMATVERSION=2.0,MAPVERSION=1.0,FORMAT=EXE,ENDIAN=BIG\n"
            "000016,RAW1,SIZE=32,Header,Identity bytes,\n"
            "000064,PAL1,SIZE=48,Palette,Unsupported here,\n"
            "000096,SND2,SIZE=24,Sound,Unsupported here,\n";
        memset(&sel, 0, sizeof(sel));
        memset(&raw, 0, sizeof(raw));
        memset(err, 0, sizeof(err));
        r = FirestaffSckBridge_SelectSlice(kTinyRawMap,
                                           "000016",
                                           "RAW",
                                           (uint32_t)sizeof(asset),
                                           &sel,
                                           err,
                                           sizeof(err));
        CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
        CHECK_STR_EQ(sel.itemType, "RAW1");
        CHECK(sel.slice.offset == 16u);
        CHECK(sel.slice.size == 32u);

        r = FirestaffSckBridge_DecodeRawSelection(asset,
                                                  (uint32_t)sizeof(asset),
                                                  &sel,
                                                  &raw,
                                                  err,
                                                  sizeof(err));
        CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
        CHECK(raw.bytes == asset + 16u);
        CHECK(raw.byteCount == 32u);
        CHECK(raw.offset == 16u);
        CHECK(raw.bytes[0] == asset[16]);
        CHECK(raw.bytes[31] == asset[47]);
        CHECK(raw.checksum32 == test_checksum32_fnv1a(asset + 16u, 32u));
        CHECK_STR_EQ(raw.itemNumber, "000016");
        CHECK_STR_EQ(raw.itemType, "RAW1");
        CHECK_STR_EQ(raw.itemDescription, "Header");

        r = FirestaffSckBridge_SelectSlice(kTinyRawMap,
                                           "000064",
                                           "PAL",
                                           (uint32_t)sizeof(asset),
                                           &sel,
                                           err,
                                           sizeof(err));
        CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
        r = FirestaffSckBridge_DecodeRawSelection(asset,
                                                  (uint32_t)sizeof(asset),
                                                  &sel,
                                                  &raw,
                                                  err,
                                                  sizeof(err));
        CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_UNSUPPORTED_DECODER);

        r = FirestaffSckBridge_SelectSlice(kTinyRawMap,
                                           "000096",
                                           "SND",
                                           (uint32_t)sizeof(asset),
                                           &sel,
                                           err,
                                           sizeof(err));
        CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
        r = FirestaffSckBridge_DecodeRawSelection(asset,
                                                  (uint32_t)sizeof(asset),
                                                  &sel,
                                                  &raw,
                                                  err,
                                                  sizeof(err));
        CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_UNSUPPORTED_DECODER);

        r = FirestaffSckBridge_SelectSlice(kTinyRawMap,
                                           "000016",
                                           "RAW",
                                           32u,
                                           &sel,
                                           err,
                                           sizeof(err));
        CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_SLICE_OUT_OF_BOUNDS);
    }
}

static void test_select_slice_by_description(void) {
    FirestaffSckBridgeSelection sel;
    char err[128];
    FirestaffSckBridgeResult r;

    memset(&sel, 0, sizeof(sel));
    memset(err, 0, sizeof(err));

    r = FirestaffSckBridge_SelectSliceByDescription(kAtariDemoMap,
                                                    "Floor Pit Front 1",
                                                    "IMG",
                                                    200000u,
                                                    &sel,
                                                    err,
                                                    sizeof(err));
    CHECK(r == FIRESTAFF_SCK_BRIDGE_OK);
    CHECK_STR_EQ(sel.itemNumber, "015434");
    CHECK(sel.slice.offset == 15434u);
    CHECK(sel.slice.size == 532u);

    /* Description substring that matches an unsized row only. */
    r = FirestaffSckBridge_SelectSliceByDescription(kAtariDemoMap,
                                                    "Dialog Box",
                                                    "IMG",
                                                    200000u,
                                                    &sel,
                                                    err,
                                                    sizeof(err));
    CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_NOT_SIZED);

    /* Description substring that doesn't match anything. */
    r = FirestaffSckBridge_SelectSliceByDescription(kAtariDemoMap,
                                                    "no such label",
                                                    "IMG",
                                                    200000u,
                                                    &sel,
                                                    err,
                                                    sizeof(err));
    CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_NOT_FOUND);
}

static void test_select_slice_against_pc34_map(void) {
    /* The PC 3.4 graphics.dat map has only unsized IMG3 entries; the
     * bridge must report NOT_SIZED rather than fabricating slices. */
    FirestaffSckBridgeSelection sel;
    char err[128];
    FirestaffSckBridgeResult r;

    memset(&sel, 0, sizeof(sel));
    memset(err, 0, sizeof(err));

    r = FirestaffSckBridge_SelectSlice(kPc34Map,
                                       "0017",
                                       "IMG",
                                       600000u,
                                       &sel,
                                       err,
                                       sizeof(err));
    CHECK(r == FIRESTAFF_SCK_BRIDGE_ERR_NOT_SIZED);
}

static void test_select_slice_null_argument(void) {
    FirestaffSckBridgeSelection sel;
    char err[128];

    CHECK(FirestaffSckBridge_SelectSlice(NULL, "0", "IMG", 1u, &sel, err, sizeof(err)) ==
          FIRESTAFF_SCK_BRIDGE_ERR_NULL_ARG);
    CHECK(FirestaffSckBridge_SelectSlice(kAtariDemoMap, "0", "IMG", 1u, NULL, err, sizeof(err)) ==
          FIRESTAFF_SCK_BRIDGE_ERR_NULL_ARG);
    CHECK(FirestaffSckBridge_SelectSliceByDescription(NULL, "x", "IMG", 1u, &sel, err, sizeof(err)) ==
          FIRESTAFF_SCK_BRIDGE_ERR_NULL_ARG);
}

static void test_result_string_contract(void) {
    /* Result codes must be stable so log scrapers stay deterministic. */
    CHECK(strcmp(FirestaffSckBridge_ResultString(FIRESTAFF_SCK_BRIDGE_OK), "ok") == 0);
    CHECK(strcmp(FirestaffSckBridge_ResultString(FIRESTAFF_SCK_BRIDGE_ERR_NOT_FOUND),
                 "row/item not found") == 0);
    CHECK(strcmp(FirestaffSckBridge_ResultString(FIRESTAFF_SCK_BRIDGE_ERR_NOT_SIZED),
                 "item has no SIZE attribute") == 0);
    CHECK(strcmp(FirestaffSckBridge_ResultString(FIRESTAFF_SCK_BRIDGE_ERR_SLICE_OUT_OF_BOUNDS),
                 "slice exceeds target file size") == 0);
    CHECK(strcmp(FirestaffSckBridge_ResultString(FIRESTAFF_SCK_BRIDGE_ERR_UNSUPPORTED_DECODER),
                 "unsupported asset decoder") == 0);
}

int main(void) {
    test_mapping_xml_parse();
    test_mapping_xml_rejections();
    test_lookup_combinations();
    test_select_slice_by_number();
    test_raw_decoder_handoff();
    test_select_slice_by_description();
    test_select_slice_against_pc34_map();
    test_select_slice_null_argument();
    test_result_string_contract();

    if (g_failures != 0) {
        fprintf(stderr, "FAIL firestaff_sck_asset_bridge failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS firestaff_sck_asset_bridge\n");
    return 0;
}
