#include "firestaff_sck_mapfile.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(int ok, const char* name) {
    if (!ok) {
        ++failures;
        printf("FAIL %s\n", name);
    } else {
        printf("PASS %s\n", name);
    }
}

static void test_parse_core_rows(void) {
    const char* text =
        "# Greatstone/SCK mapfile rows: type name offset size\n"
        "IMG1 image00 0 256\n"
        "IMG5 image01 0x100 32 # inline comment\n"
        "\r\n"
        "\tPAL palette00 288 16\r\n";
    FirestaffSckMapfile map;
    const FirestaffSckMapfileItem* item;
    char err[128];

    memset(&map, 0, sizeof(map));
    memset(err, 0, sizeof(err));
    check(FirestaffSckMapfile_ParseText(text, &map, err, sizeof(err)) == 1,
          "mapfile text parses");
    check(map.itemCount == 3u, "comments and blank lines ignored");
    check(strcmp(map.items[0].type, "IMG1") == 0, "first type captured");
    check(strcmp(map.items[0].name, "image00") == 0, "first name captured");
    check(map.items[0].offset == 0u && map.items[0].size == 256u,
          "decimal offset and size captured");
    check(map.items[1].offset == 256u && map.items[1].size == 32u,
          "hex offset captured");
    check(map.items[2].lineNumber == 5u, "CRLF line number tracked");

    item = FirestaffSckMapfile_FindByName(&map, "palette00");
    check(item != NULL && strcmp(item->type, "PAL") == 0,
          "find by name returns matching item");
    check(FirestaffSckMapfile_FindByName(&map, "missing") == NULL,
          "missing name returns NULL");
    check(FirestaffSckMapfile_ValidateBounds(&map, 304u, err, sizeof(err)) == 1,
          "items fit target file size");
}

static void test_rejections(void) {
    FirestaffSckMapfile map;
    char err[128];

    check(FirestaffSckMapfile_ParseText(NULL, &map, err, sizeof(err)) == 0,
          "NULL text rejected");
    check(FirestaffSckMapfile_ParseText("# only comments\n\n", &map, err, sizeof(err)) == 0,
          "empty mapfile rejected");
    check(FirestaffSckMapfile_ParseText("IMG1 only_name 0\n", &map, err, sizeof(err)) == 0,
          "missing size rejected");
    check(FirestaffSckMapfile_ParseText("IMG1 image00 0x100000000 1\n", &map, err, sizeof(err)) == 0,
          "u32 overflow rejected");
    check(FirestaffSckMapfile_ParseText("IMG1 image00 0 1 junk\n", &map, err, sizeof(err)) == 0,
          "trailing junk rejected");

    check(FirestaffSckMapfile_ParseText("IMG1 image00 8 8\n", &map, err, sizeof(err)) == 1,
          "bounds test fixture parses");
    check(FirestaffSckMapfile_ValidateBounds(&map, 15u, err, sizeof(err)) == 0,
          "out-of-bounds item rejected");
}

int main(void) {
    test_parse_core_rows();
    test_rejections();
    if (failures) {
        printf("test_firestaff_sck_mapfile: FAIL %d\n", failures);
        return 1;
    }
    puts("test_firestaff_sck_mapfile: PASS");
    return 0;
}
