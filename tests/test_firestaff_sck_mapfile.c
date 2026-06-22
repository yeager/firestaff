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

static void test_parse_sck2_rows(void) {
    const char* text =
        "MAPFORMATVERSION=2.0,MAPVERSION=0.1,ENDIAN=LITTLE,FORMAT=ROM\n"
        "0001,RAW1,SIZE=16,Unknown Header,,\n"
        "0017,IMG3,PAL=PAL1&SIZE=8,Interface - Character Sheet,,\n"
        "0033,NULL,NULL,No Item Data,,\n";
    FirestaffSckMapfileV2 map;
    FirestaffSckAssetSlice slices[4];
    const FirestaffSckMapfileV2Item* item;
    unsigned int sliceCount = 99u;
    char err[128];

    memset(&map, 0, sizeof(map));
    memset(slices, 0, sizeof(slices));
    memset(err, 0, sizeof(err));

    check(FirestaffSckMapfile_ParseSck2Text(text, &map, err, sizeof(err)) == 1,
          "SCK 2.x mapfile parses");
    check(strcmp(map.format, "ROM") == 0, "SCK format property captured");
    check(strcmp(map.endian, "LITTLE") == 0, "SCK endian property captured");
    check(map.itemCount == 3u, "SCK item count captured");

    item = FirestaffSckMapfileV2_FindByNumber(&map, "0017");
    check(item != NULL && strcmp(item->type, "IMG3") == 0,
          "SCK item lookup by number works");
    check(item != NULL && strstr(item->attributes, "PAL=PAL1") != NULL,
          "SCK attributes preserved");
    check(item != NULL && item->hasNumericNumber && item->numericNumber == 17u,
          "SCK numeric item number captured");
    check(item != NULL && item->hasSizeBytes && item->sizeBytes == 8u,
          "SCK SIZE attribute captured");
    check(item != NULL && strcmp(item->description, "Interface - Character Sheet") == 0,
          "SCK description captured");

    check(FirestaffSckMapfileV2_BuildSizedSlices(&map,
                                                 64u,
                                                 slices,
                                                 4u,
                                                 &sliceCount,
                                                 err,
                                                 sizeof(err)) == 1,
          "SCK sized slices build");
    check(sliceCount == 2u, "only SIZE-backed SCK rows become slices");
    check(slices[0].offset == 1u && slices[0].size == 16u,
          "first SCK slice uses numeric row as offset");
    check(strcmp(slices[1].type, "IMG3") == 0 &&
              strcmp(slices[1].description, "Interface - Character Sheet") == 0,
          "SCK slice preserves type and label");

    check(FirestaffSckMapfileV2_BuildSizedSlices(&map,
                                                 20u,
                                                 slices,
                                                 4u,
                                                 &sliceCount,
                                                 err,
                                                 sizeof(err)) == 0,
          "out-of-bounds SCK slice rejected");
}

static void test_sck2_rejections(void) {
    FirestaffSckMapfileV2 map;
    char err[128];

    check(FirestaffSckMapfile_ParseSck2Text(NULL, &map, err, sizeof(err)) == 0,
          "NULL SCK text rejected");
    check(FirestaffSckMapfile_ParseSck2Text("# no header\n", &map, err, sizeof(err)) == 0,
          "SCK missing header rejected");
    check(FirestaffSckMapfile_ParseSck2Text("ENDIAN=LITTLE,FORMAT=ROM\n", &map, err, sizeof(err)) == 0,
          "SCK empty item list rejected");
    check(FirestaffSckMapfile_ParseSck2Text("ENDIAN=LITTLE,FORMAT=ROM\n0001,RAW1\n",
                                            &map,
                                            err,
                                            sizeof(err)) == 0,
          "short SCK item rejected");
}

int main(void) {
    test_parse_core_rows();
    test_rejections();
    test_parse_sck2_rows();
    test_sck2_rejections();
    if (failures) {
        printf("test_firestaff_sck_mapfile: FAIL %d\n", failures);
        return 1;
    }
    puts("test_firestaff_sck_mapfile: PASS");
    return 0;
}
