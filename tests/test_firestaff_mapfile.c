#include "firestaff_mapfile.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++g_failures; \
    } \
} while (0)

static void test_greatstone_row_contract(void)
{
    static const char *fixture =
        "# Synthetic fixture mirroring Greatstone d_mapfile_20.html 2.5 rows.\n"
        "MAPFORMATVERSION=2.5, MAPVERSION=1.0, FORMAT=DMCSB2, ENDIAN=LITTLE\n"
        "0000, IMG3, PAL1, Interface - Dialog Box,,\n"
        "0001, IMG3, PAL=DM_PAL_TITLE &SIZE=3632, Interface - Main Title, Title screen bitmap, from synthetic fixture\n"
        "0558, NULL, NULL, No Item Data,,\n"
        "PAL_INGAME, PAL, ID=PALETTE_INGAME &SIZE=32, In-game palette,,\n";
    Firestaff_MapfileDocument doc;
    Firestaff_MapfileItem items[8];
    char value[64];
    Firestaff_MapfileResult r;

    r = Firestaff_Mapfile_ParseText(fixture, &doc, items, 8);
    CHECK(r == FIRESTAFF_MAPFILE_OK);
    CHECK(doc.property_count == 4);
    CHECK(strcmp(doc.properties[0].key, "MAPFORMATVERSION") == 0);
    CHECK(strcmp(doc.properties[0].value, "2.5") == 0);
    CHECK(strcmp(doc.properties[3].key, "ENDIAN") == 0);
    CHECK(strcmp(doc.properties[3].value, "LITTLE") == 0);
    CHECK(doc.item_count == 4);

    CHECK(items[0].line == 3);
    CHECK(strcmp(items[0].number, "0000") == 0);
    CHECK(strcmp(items[0].type, "IMG3") == 0);
    CHECK(strcmp(items[0].attributes, "PAL1") == 0);
    CHECK(strcmp(items[0].description, "Interface - Dialog Box") == 0);
    CHECK(items[0].has_size == 0);

    CHECK(strcmp(items[1].number, "0001") == 0);
    CHECK(strcmp(items[1].description, "Interface - Main Title") == 0);
    CHECK(strcmp(items[1].long_description, "Title screen bitmap") == 0);
    CHECK(strcmp(items[1].comment, "from synthetic fixture") == 0);
    CHECK(items[1].has_size == 1);
    CHECK(items[1].size == 3632U);
    r = Firestaff_Mapfile_FindAttribute(&items[1], "PAL", value, sizeof(value));
    CHECK(r == FIRESTAFF_MAPFILE_OK);
    CHECK(strcmp(value, "DM_PAL_TITLE") == 0);
    r = Firestaff_Mapfile_FindAttribute(&items[1], "SIZE", value, sizeof(value));
    CHECK(r == FIRESTAFF_MAPFILE_OK);
    CHECK(strcmp(value, "3632") == 0);

    CHECK(strcmp(items[2].type, "NULL") == 0);
    CHECK(strcmp(items[2].description, "No Item Data") == 0);

    CHECK(strcmp(items[3].number, "PAL_INGAME") == 0);
    CHECK(strcmp(items[3].type, "PAL") == 0);
    CHECK(items[3].has_size == 1);
    CHECK(items[3].size == 32U);
}

static void test_quoted_header_values_and_crlf(void)
{
    static const char *fixture =
        "FORMAT = \"FTL\", ENDIAN='BIG'\r\n"
        "PAL_INGAME, PAL, ID=PAL_INGAME &SIZE=0x20, Palette,,\r\n";
    Firestaff_MapfileDocument doc;
    Firestaff_MapfileItem items[2];
    Firestaff_MapfileResult r;

    r = Firestaff_Mapfile_ParseText(fixture, &doc, items, 2);
    CHECK(r == FIRESTAFF_MAPFILE_OK);
    CHECK(doc.property_count == 2);
    CHECK(strcmp(doc.properties[0].value, "FTL") == 0);
    CHECK(strcmp(doc.properties[1].value, "BIG") == 0);
    CHECK(doc.item_count == 1);
    CHECK(items[0].has_size == 1);
    CHECK(items[0].size == 32U);
}

static void test_capacity_and_malformed_rows(void)
{
    static const char *two_items =
        "FORMAT=DMCSB2,ENDIAN=LITTLE\n"
        "0000,IMG3,PAL1,One,,\n"
        "0001,IMG3,PAL1,Two,,\n";
    Firestaff_MapfileDocument doc;
    Firestaff_MapfileItem items[1];
    Firestaff_MapfileResult r;

    r = Firestaff_Mapfile_ParseText(two_items, &doc, items, 1);
    CHECK(r == FIRESTAFF_MAPFILE_ERROR_TOO_MANY_ITEMS);
    CHECK(doc.error_line == 3);

    r = Firestaff_Mapfile_ParseText("FORMAT=DMCSB2\n000-1,IMG3,PAL1,Bad,,\n", &doc, items, 1);
    CHECK(r == FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_NUMBER);
    CHECK(doc.error_line == 2);

    r = Firestaff_Mapfile_ParseText("FORMAT=DMCSB2\n0001,img3,PAL1,Bad,,\n", &doc, items, 1);
    CHECK(r == FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_TYPE);
    CHECK(doc.error_line == 2);

    r = Firestaff_Mapfile_ParseText("FORMAT=DMCSB2\n0001,IMG3,PAL1\n", &doc, items, 1);
    CHECK(r == FIRESTAFF_MAPFILE_ERROR_BAD_ITEM_ROW);
    CHECK(doc.error_line == 2);
}

static void test_attribute_lookup_bounds(void)
{
    Firestaff_MapfileItem item;
    char value[8];
    Firestaff_MapfileResult r;

    memset(&item, 0, sizeof(item));
    strcpy(item.attributes, "ID=PALETTE_INGAME &TILES=TILES_INTERFACE1 &TILES=TILES_INTERFACE2");

    r = Firestaff_Mapfile_FindAttribute(&item, "TILES", value, sizeof(value));
    CHECK(r == FIRESTAFF_MAPFILE_ERROR_FIELD_TOO_LONG);

    r = Firestaff_Mapfile_FindAttribute(&item, "MISSING", value, sizeof(value));
    CHECK(r == FIRESTAFF_MAPFILE_ERROR_BAD_ATTRIBUTE);
}

int main(void)
{
    test_greatstone_row_contract();
    test_quoted_header_values_and_crlf();
    test_capacity_and_malformed_rows();
    test_attribute_lookup_bounds();

    if (g_failures != 0) {
        fprintf(stderr, "FAIL firestaff_mapfile_unit failures=%d\n", g_failures);
        return 1;
    }

    printf("PASS firestaff_mapfile_unit\n");
    return 0;
}
