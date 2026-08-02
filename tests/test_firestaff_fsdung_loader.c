#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "firestaff_fsdung_loader.h"

static void write_u16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)(v >> 8);
}

static void test_roundtrip(void) {
    /* Build a minimal .fsdung in memory: 1 map, 4x3, 2 things */
    const char *mapName = "Test Level";
    uint16_t nameLen = (uint16_t)strlen(mapName);
    uint16_t w = 4, h = 3;
    uint16_t thingCount = 2;

    size_t totalSize = FSDUNG_HEADER_SIZE
        + FSDUNG_MAP_DESC_SIZE + nameLen
        + (size_t)(w * h)
        + 2 + thingCount * FSDUNG_THING_ENTRY_SIZE;

    uint8_t *buf = (uint8_t *)calloc(1, totalSize);
    size_t off = 0;

    /* Header */
    buf[0] = 'F'; buf[1] = 'S'; buf[2] = 'D'; buf[3] = 'G';
    write_u16(buf + 4, FSDUNG_VERSION);
    buf[6] = 1;  /* mapCount */
    write_u16(buf + 8, 1);   /* partyX */
    write_u16(buf + 10, 2);  /* partyY */
    write_u16(buf + 12, 3);  /* partyDir */
    off = FSDUNG_HEADER_SIZE;

    /* Map descriptor */
    write_u16(buf + off, nameLen);
    write_u16(buf + off + 2, w);
    write_u16(buf + off + 4, h);
    buf[off + 6] = 2;  /* wallSet */
    buf[off + 7] = 1;  /* floorSet */
    buf[off + 8] = 0;  /* doorSet0 */
    buf[off + 9] = 1;  /* doorSet1 */
    buf[off + 10] = 5; /* difficulty */
    off += FSDUNG_MAP_DESC_SIZE;

    /* Map name */
    memcpy(buf + off, mapName, nameLen);
    off += nameLen;

    /* Tile data: column-major, 4 columns x 3 rows */
    /* Column 0: all walls */
    buf[off + 0] = (FSDUNG_TILE_WALL << 5);
    buf[off + 1] = (FSDUNG_TILE_WALL << 5);
    buf[off + 2] = (FSDUNG_TILE_WALL << 5);
    /* Column 1: corridor with thing */
    buf[off + 3] = (FSDUNG_TILE_CORRIDOR << 5) | 0x10;
    buf[off + 4] = (FSDUNG_TILE_STAIRS << 5);
    buf[off + 5] = (FSDUNG_TILE_WALL << 5);
    /* Column 2: door */
    buf[off + 6] = (FSDUNG_TILE_WALL << 5);
    buf[off + 7] = (FSDUNG_TILE_DOOR << 5) | 0x10;
    buf[off + 8] = (FSDUNG_TILE_WALL << 5);
    /* Column 3: all walls */
    buf[off + 9]  = (FSDUNG_TILE_WALL << 5);
    buf[off + 10] = (FSDUNG_TILE_WALL << 5);
    buf[off + 11] = (FSDUNG_TILE_WALL << 5);
    off += w * h;

    /* Thing count */
    write_u16(buf + off, thingCount);
    off += 2;

    /* Thing 0: weapon at (1,0) cell 0 */
    write_u16(buf + off, 1); write_u16(buf + off + 2, 0);
    buf[off + 4] = FSDUNG_THING_WEAPON; buf[off + 5] = 0;
    write_u16(buf + off + 6, 0);
    off += FSDUNG_THING_ENTRY_SIZE;

    /* Thing 1: sensor at (2,1) cell 2 */
    write_u16(buf + off, 2); write_u16(buf + off + 2, 1);
    buf[off + 4] = FSDUNG_THING_SENSOR; buf[off + 5] = 2;
    write_u16(buf + off + 6, 0);
    off += FSDUNG_THING_ENTRY_SIZE;

    assert(off == totalSize);

    /* Load */
    FsdungDungeon d;
    bool ok = fsdung_load(buf, totalSize, &d);
    assert(ok);

    assert(d.version == FSDUNG_VERSION);
    assert(d.mapCount == 1);
    assert(d.partyX == 1);
    assert(d.partyY == 2);
    assert(d.partyDir == 3);

    FsdungMap *m = &d.maps[0];
    assert(strcmp(m->name, "Test Level") == 0);
    assert(m->width == 4);
    assert(m->height == 3);
    assert(m->wallSet == 2);
    assert(m->floorSet == 1);
    assert(m->doorSet1 == 1);
    assert(m->difficulty == 5);

    /* Check tile types (column-major: index = col * height + row) */
    assert(fsdung_tile_type(m->tileData[0 * 3 + 0]) == FSDUNG_TILE_WALL);
    assert(fsdung_tile_type(m->tileData[1 * 3 + 0]) == FSDUNG_TILE_CORRIDOR);
    assert(fsdung_tile_has_things(m->tileData[1 * 3 + 0]));
    assert(fsdung_tile_type(m->tileData[1 * 3 + 1]) == FSDUNG_TILE_STAIRS);
    assert(fsdung_tile_type(m->tileData[2 * 3 + 1]) == FSDUNG_TILE_DOOR);
    assert(fsdung_tile_has_things(m->tileData[2 * 3 + 1]));

    /* Check things */
    assert(m->thingCount == 2);
    assert(m->things[0].x == 1);
    assert(m->things[0].y == 0);
    assert(m->things[0].type == FSDUNG_THING_WEAPON);
    assert(m->things[0].cell == 0);
    assert(m->things[1].x == 2);
    assert(m->things[1].y == 1);
    assert(m->things[1].type == FSDUNG_THING_SENSOR);
    assert(m->things[1].cell == 2);

    fsdung_free(&d);
    free(buf);
    printf("test_roundtrip: PASSED\n");
}

static void test_bad_magic(void) {
    uint8_t buf[16] = { 'X', 'S', 'D', 'G' };
    FsdungDungeon d;
    assert(!fsdung_load(buf, sizeof(buf), &d));
    printf("test_bad_magic: PASSED\n");
}

static void test_truncated(void) {
    uint8_t buf[8] = { 'F', 'S', 'D', 'G', 1, 0, 1, 0 };
    FsdungDungeon d;
    assert(!fsdung_load(buf, sizeof(buf), &d));
    printf("test_truncated: PASSED\n");
}

int main(void) {
    test_roundtrip();
    test_bad_magic();
    test_truncated();
    printf("All fsdung loader tests passed.\n");
    return 0;
}
