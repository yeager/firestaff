#ifndef FIRESTAFF_FSDUNG_LOADER_H
#define FIRESTAFF_FSDUNG_LOADER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define FSDUNG_MAGIC_0 0x46  /* 'F' */
#define FSDUNG_MAGIC_1 0x53  /* 'S' */
#define FSDUNG_MAGIC_2 0x44  /* 'D' */
#define FSDUNG_MAGIC_3 0x47  /* 'G' */
#define FSDUNG_VERSION 1

#define FSDUNG_HEADER_SIZE      16
#define FSDUNG_MAP_DESC_SIZE    32
#define FSDUNG_THING_ENTRY_SIZE  8
#define FSDUNG_MAX_MAPS         32
#define FSDUNG_MAX_MAP_DIM      64
#define FSDUNG_MAX_NAME_LEN    128

#define FSDUNG_TILE_WALL        0
#define FSDUNG_TILE_CORRIDOR    1
#define FSDUNG_TILE_PIT         2
#define FSDUNG_TILE_STAIRS      3
#define FSDUNG_TILE_DOOR        4
#define FSDUNG_TILE_TELEPORTER  5
#define FSDUNG_TILE_FAKEWALL    6

#define FSDUNG_THING_DOOR        0
#define FSDUNG_THING_TELEPORTER  1
#define FSDUNG_THING_TEXT        2
#define FSDUNG_THING_SENSOR      3
#define FSDUNG_THING_GROUP       4
#define FSDUNG_THING_WEAPON      5
#define FSDUNG_THING_ARMOUR      6
#define FSDUNG_THING_SCROLL      7
#define FSDUNG_THING_POTION      8
#define FSDUNG_THING_CONTAINER   9
#define FSDUNG_THING_JUNK       10

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t  type;
    uint8_t  cell;
    uint16_t subtype;
} FsdungThing;

typedef struct {
    char     name[FSDUNG_MAX_NAME_LEN + 1];
    uint16_t width;
    uint16_t height;
    uint8_t  wallSet;
    uint8_t  floorSet;
    uint8_t  doorSet0;
    uint8_t  doorSet1;
    uint8_t  difficulty;
    uint8_t  *tileData;      /* width * height bytes, column-major. bits 7-5 = type, bit 4 = hasThings */
    FsdungThing *things;
    uint16_t thingCount;
} FsdungMap;

typedef struct {
    uint16_t version;
    uint8_t  mapCount;
    uint16_t partyX;
    uint16_t partyY;
    uint16_t partyDir;
    FsdungMap maps[FSDUNG_MAX_MAPS];
} FsdungDungeon;

static inline uint16_t fsdung_read_u16(const uint8_t *p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}

/* Returns true on success. Caller must call fsdung_free() when done. */
static bool fsdung_load(const uint8_t *data, size_t dataSize, FsdungDungeon *out) {
    if (!data || dataSize < FSDUNG_HEADER_SIZE || !out) return false;

    if (data[0] != FSDUNG_MAGIC_0 || data[1] != FSDUNG_MAGIC_1 ||
        data[2] != FSDUNG_MAGIC_2 || data[3] != FSDUNG_MAGIC_3) return false;

    memset(out, 0, sizeof(FsdungDungeon));
    out->version  = fsdung_read_u16(data + 4);
    if (out->version != FSDUNG_VERSION) return false;

    out->mapCount = data[6];
    if (out->mapCount == 0 || out->mapCount > FSDUNG_MAX_MAPS) return false;

    out->partyX   = fsdung_read_u16(data + 8);
    out->partyY   = fsdung_read_u16(data + 10);
    out->partyDir  = fsdung_read_u16(data + 12);

    size_t off = FSDUNG_HEADER_SIZE;

    /* Read map descriptors + names */
    for (int i = 0; i < out->mapCount; i++) {
        if (off + FSDUNG_MAP_DESC_SIZE > dataSize) return false;
        FsdungMap *m = &out->maps[i];

        uint16_t nameLen = fsdung_read_u16(data + off);
        m->width      = fsdung_read_u16(data + off + 2);
        m->height     = fsdung_read_u16(data + off + 4);
        m->wallSet    = data[off + 6];
        m->floorSet   = data[off + 7];
        m->doorSet0   = data[off + 8];
        m->doorSet1   = data[off + 9];
        m->difficulty = data[off + 10];
        off += FSDUNG_MAP_DESC_SIZE;

        if (m->width == 0 || m->width > FSDUNG_MAX_MAP_DIM ||
            m->height == 0 || m->height > FSDUNG_MAX_MAP_DIM) return false;

        if (nameLen > FSDUNG_MAX_NAME_LEN) nameLen = FSDUNG_MAX_NAME_LEN;
        if (off + nameLen > dataSize) return false;
        memcpy(m->name, data + off, nameLen);
        m->name[nameLen] = '\0';
        off += nameLen;
    }

    /* Read tile data + things per map */
    for (int i = 0; i < out->mapCount; i++) {
        FsdungMap *m = &out->maps[i];
        size_t tileBytes = (size_t)m->width * m->height;
        if (off + tileBytes > dataSize) return false;

        m->tileData = (uint8_t *)malloc(tileBytes);
        if (!m->tileData) return false;
        memcpy(m->tileData, data + off, tileBytes);
        off += tileBytes;

        if (off + 2 > dataSize) return false;
        m->thingCount = fsdung_read_u16(data + off);
        off += 2;

        if (m->thingCount > 0) {
            size_t thingBytes = (size_t)m->thingCount * FSDUNG_THING_ENTRY_SIZE;
            if (off + thingBytes > dataSize) return false;

            m->things = (FsdungThing *)malloc(m->thingCount * sizeof(FsdungThing));
            if (!m->things) return false;

            for (int t = 0; t < m->thingCount; t++) {
                const uint8_t *tp = data + off + t * FSDUNG_THING_ENTRY_SIZE;
                m->things[t].x       = fsdung_read_u16(tp);
                m->things[t].y       = fsdung_read_u16(tp + 2);
                m->things[t].type    = tp[4];
                m->things[t].cell    = tp[5];
                m->things[t].subtype = fsdung_read_u16(tp + 6);
            }
            off += thingBytes;
        }
    }

    return true;
}

static void fsdung_free(FsdungDungeon *d) {
    if (!d) return;
    for (int i = 0; i < d->mapCount; i++) {
        free(d->maps[i].tileData);
        free(d->maps[i].things);
        d->maps[i].tileData = NULL;
        d->maps[i].things = NULL;
    }
}

/* Extract tile type from a tile byte */
static inline uint8_t fsdung_tile_type(uint8_t tileByte) {
    return tileByte >> 5;
}

/* Check if tile has things */
static inline bool fsdung_tile_has_things(uint8_t tileByte) {
    return (tileByte & 0x10) != 0;
}

#endif /* FIRESTAFF_FSDUNG_LOADER_H */
