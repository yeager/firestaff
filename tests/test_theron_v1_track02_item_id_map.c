#include "theron_v1_track02_item_id_map.h"
#include <assert.h>
#include <stdio.h>

static void test_item_id_translations(void) {
    assert(theron_v1_track02_translate_item_id(0x38) == 0x7F);
    assert(theron_v1_track02_translate_item_id(0x55) == 0xB0);
    assert(theron_v1_track02_translate_item_id(0x56) == 0xB8);
    assert(theron_v1_track02_translate_item_id(0x57) == 0xBE);
    assert(theron_v1_track02_translate_item_id(0x32) == 0x6E);
    assert(theron_v1_track02_translate_item_id(0x01) == 0x01);
    printf("  Item ID translations OK\n");
}

static void test_item_id_map_table(void) {
    unsigned int count = theron_v1_track02_item_id_map_count();
    assert(count == 5);
    const Theron_ItemIdMapping *m = theron_v1_track02_item_id_map(0);
    assert(m != NULL);
    assert(m->tq_id == 0x32);
    assert(theron_v1_track02_item_id_map(5) == NULL);
    printf("  Item ID map table OK\n");
}

static void test_akutuba_wall_ornaments(void) {
    unsigned int count;
    const Theron_WallOrnament *walls;

    walls = theron_v1_track02_akutuba_wall_ornaments(0, &count);
    assert(walls && count == 1);
    assert(walls[0].dm1_ornament_id == 0x2B);

    walls = theron_v1_track02_akutuba_wall_ornaments(1, &count);
    assert(walls && count == 14);
    assert(walls[0].dm1_ornament_id == 0x05);
    assert(walls[5].dm1_ornament_id == 0x1A);

    walls = theron_v1_track02_akutuba_wall_ornaments(2, &count);
    assert(walls && count == 14);
    assert(walls[0].dm1_ornament_id == 0x11);

    walls = theron_v1_track02_akutuba_wall_ornaments(3, &count);
    assert(walls && count == 6);
    assert(walls[2].dm1_ornament_id == 0x23);

    walls = theron_v1_track02_akutuba_wall_ornaments(4, &count);
    assert(!walls && count == 0);

    printf("  AKUTUBA wall ornaments OK\n");
}

int main(void) {
    printf("test_theron_v1_track02_item_id_map\n");
    test_item_id_translations();
    test_item_id_map_table();
    test_akutuba_wall_ornaments();
    printf("PASS\n");
    return 0;
}
