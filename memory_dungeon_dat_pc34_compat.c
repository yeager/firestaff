#include <stdlib.h>
#include <string.h>
#include "memory_dungeon_dat_pc34_compat.h"

/* ---- low-level I/O helpers (same pattern as graphics_dat seams) ---- */

static int read_u8(FILE* file, unsigned char* outValue) {
        if (fread(outValue, 1, 1, file) != 1) {
                return 0;
        }
        return 1;
}

static int read_u16_le(FILE* file, unsigned short* outValue) {
        unsigned char bytes[2];

        if (fread(bytes, 1, 2, file) != 2) {
                return 0;
        }
        *outValue = (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
        return 1;
}

/* ---- bitfield decode (PC/Watcom LSB-first packing) ---- */

static void decode_map_bitfield_a(unsigned short raw,
        unsigned char* outLevel,
        unsigned char* outWidth,
        unsigned char* outHeight) {
        /* bits  5:0  = Level
           bits 10:6  = Width  - 1
           bits 15:11 = Height - 1 */
        *outLevel  = (unsigned char)(raw & 0x3Fu);
        *outWidth  = (unsigned char)(((raw >> 6) & 0x1Fu) + 1);
        *outHeight = (unsigned char)(((raw >> 11) & 0x1Fu) + 1);
}

static void decode_map_bitfield_b(unsigned short raw,
        struct DungeonMapDesc_Compat* m) {
        /* PC LSB-first (MEDIA016 / I34E):
           bits  3:0  = WallOrnamentCount
           bits  7:4  = RandomWallOrnamentCount
           bits 11:8  = FloorOrnamentCount
           bits 15:12 = RandomFloorOrnamentCount */
        m->wallOrnamentCount        = (unsigned char)(raw & 0x0Fu);
        m->randomWallOrnamentCount  = (unsigned char)((raw >> 4) & 0x0Fu);
        m->floorOrnamentCount       = (unsigned char)((raw >> 8) & 0x0Fu);
        m->randomFloorOrnamentCount = (unsigned char)((raw >> 12) & 0x0Fu);
}

static void decode_map_bitfield_c(unsigned short raw,
        struct DungeonMapDesc_Compat* m) {
        /* PC LSB-first (MEDIA016 / I34E):
           bits  3:0  = DoorOrnamentCount
           bits  7:4  = CreatureTypeCount
           bits 11:8  = Unreferenced
           bits 15:12 = Difficulty */
        m->doorOrnamentCount  = (unsigned char)(raw & 0x0Fu);
        m->creatureTypeCount  = (unsigned char)((raw >> 4) & 0x0Fu);
        /* bits 11:8 unreferenced */
        m->difficulty         = (unsigned char)((raw >> 12) & 0x0Fu);
}

static void decode_map_bitfield_d(unsigned short raw,
        struct DungeonMapDesc_Compat* m) {
        /* PC LSB-first (MEDIA016 / I34E):
           bits  3:0  = FloorSet
           bits  7:4  = WallSet
           bits 11:8  = DoorSet0
           bits 15:12 = DoorSet1 */
        m->floorSet = (unsigned char)(raw & 0x0Fu);
        m->wallSet  = (unsigned char)((raw >> 4) & 0x0Fu);
        m->doorSet0 = (unsigned char)((raw >> 8) & 0x0Fu);
        m->doorSet1 = (unsigned char)((raw >> 12) & 0x0Fu);
}

/* ---- public API ---- */

void F0501_DUNGEON_DecodePartyLocation_Compat(
        unsigned short partyLocation,
        int* outDirection,
        int* outY,
        int* outX)
{
        *outDirection = (partyLocation >> 10) & 0x03;
        *outY         = (partyLocation >> 5)  & 0x1F;
        *outX         =  partyLocation        & 0x1F;
}

int F0500_DUNGEON_LoadDatHeader_Compat(
        const char* path,
        struct DungeonDatState_Compat* state)
{
        FILE* file;
        unsigned short sig;
        unsigned short rawBitA;
        int i;

        memset(state, 0, sizeof(*state));

        file = fopen(path, "rb");
        if (!file) {
                return 0;
        }

        /* Determine file size */
        fseek(file, 0, SEEK_END);
        state->fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        /* ---- read DUNGEON_HEADER (44 bytes) ---- */

        /* First field is OrnamentRandomSeed. Check for compressed signature. */
        if (!read_u16_le(file, &sig)) {
                fclose(file);
                return 0;
        }
        if (sig == DUNGEON_COMPRESSED_SIGNATURE) {
                /* Compressed dungeon (save game format) — not supported by this seam */
                fclose(file);
                return 0;
        }
        state->header.ornamentRandomSeed = sig;

        if (!read_u16_le(file, &state->header.rawMapDataByteCount)) goto fail;
        if (!read_u8(file, &state->header.mapCount))                goto fail;
        if (!read_u8(file, &state->header.unreferenced))            goto fail;
        if (!read_u16_le(file, &state->header.textDataWordCount))   goto fail;
        if (!read_u16_le(file, &state->header.initialPartyLocation)) goto fail;
        if (!read_u16_le(file, &state->header.squareFirstThingCount)) goto fail;

        for (i = 0; i < DUNGEON_THING_TYPE_COUNT; i++) {
                if (!read_u16_le(file, &state->header.thingCounts[i])) goto fail;
        }

        /* Sanity: MapCount must be 1..DUNGEON_MAX_MAPS */
        if (state->header.mapCount == 0 || state->header.mapCount > DUNGEON_MAX_MAPS) {
                goto fail;
        }

        /* ---- read MAP descriptors ---- */

        state->maps = (struct DungeonMapDesc_Compat*)calloc(
                state->header.mapCount, sizeof(struct DungeonMapDesc_Compat));
        if (!state->maps) {
                goto fail;
        }

        for (i = 0; i < (int)state->header.mapCount; i++) {
                struct DungeonMapDesc_Compat* m = &state->maps[i];

                if (!read_u16_le(file, &m->rawMapDataByteOffset)) goto fail_maps;
                if (!read_u16_le(file, &m->aUnreferenced))        goto fail_maps;
                if (!read_u16_le(file, &m->bUnreferenced))        goto fail_maps;
                if (!read_u8(file, &m->offsetMapX))                goto fail_maps;
                if (!read_u8(file, &m->offsetMapY))                goto fail_maps;

                /* Bitfield A: Level, Width, Height */
                if (!read_u16_le(file, &rawBitA))                  goto fail_maps;
                decode_map_bitfield_a(rawBitA, &m->level, &m->width, &m->height);

                /* Bitfields B, C, D — store raw + decode */
                if (!read_u16_le(file, &m->rawBitfieldB))          goto fail_maps;
                if (!read_u16_le(file, &m->rawBitfieldC))          goto fail_maps;
                if (!read_u16_le(file, &m->rawBitfieldD))          goto fail_maps;
                decode_map_bitfield_b(m->rawBitfieldB, m);
                decode_map_bitfield_c(m->rawBitfieldC, m);
                decode_map_bitfield_d(m->rawBitfieldD, m);
        }

        state->loaded = 1;
        fclose(file);
        return 1;

fail_maps:
        free(state->maps);
        state->maps = NULL;
fail:
        fclose(file);
        return 0;
}

void F0500_DUNGEON_FreeDatHeader_Compat(
        struct DungeonDatState_Compat* state)
{
        F0502_DUNGEON_FreeTileData_Compat(state);
        if (state->maps) {
                free(state->maps);
                state->maps = NULL;
        }
        state->loaded = 0;
}

/* ---- element type names ---- */

static const char* s_elementNames[DUNGEON_ELEMENT_COUNT] = {
        "Wall", "Corridor", "Pit", "Stairs", "Door", "Teleporter", "FakeWall"
};

const char* F0503_DUNGEON_GetElementName_Compat(int elementType) {
        if (elementType >= 0 && elementType < DUNGEON_ELEMENT_COUNT) {
                return s_elementNames[elementType];
        }
        return "Unknown";
}

/* ---- tile data loading ---- */

int F0502_DUNGEON_LoadTileData_Compat(
        const char* path,
        struct DungeonDatState_Compat* state)
{
        FILE* file;
        long rawDataFileOffset;
        int i;

        if (!state->loaded || state->tilesLoaded) {
                return 0;
        }

        file = fopen(path, "rb");
        if (!file) {
                return 0;
        }

        /* File layout after header + map descriptors:
         *   ColumnsCumulativeSquareThingCount table (totalColumns * 2 bytes)
         *   SquareFirstThings (squareFirstThingCount * 2 bytes)
         *   Thing data for types 0-15 (thingCounts[i] * ThingDataByteCount[i])
         *   Raw map data (rawMapDataByteCount bytes)
         *   Text data (textDataWordCount * 2 bytes)
         *   2-byte checksum (PC 3.4)
         */
        {
                int totalColumns = 0;
                long thingDataTotalBytes = 0;
                for (i = 0; i < (int)state->header.mapCount; i++) {
                        totalColumns += state->maps[i].width;
                }
                for (i = 0; i < 16; i++) {
                        thingDataTotalBytes += (long)state->header.thingCounts[i] * (long)s_thingDataByteCount[i];
                }
                rawDataFileOffset = DUNGEON_HEADER_SIZE +
                        (long)state->header.mapCount * DUNGEON_MAP_DESC_SIZE +
                        (long)totalColumns * 2 +
                        (long)state->header.squareFirstThingCount * 2 +
                        thingDataTotalBytes;
        }

        state->tiles = (struct DungeonMapTiles_Compat*)calloc(
                state->header.mapCount, sizeof(struct DungeonMapTiles_Compat));
        if (!state->tiles) {
                fclose(file);
                return 0;
        }

        for (i = 0; i < (int)state->header.mapCount; i++) {
                struct DungeonMapDesc_Compat* m = &state->maps[i];
                struct DungeonMapTiles_Compat* t = &state->tiles[i];
                long mapFileOffset;
                int squareCount;

                squareCount = (int)m->width * (int)m->height;
                t->squareCount = squareCount;

                t->squareData = (unsigned char*)malloc(squareCount);
                if (!t->squareData) {
                        goto fail_tiles;
                }

                /* Each map's rawMapDataByteOffset is relative to raw data section */
                mapFileOffset = rawDataFileOffset + (long)m->rawMapDataByteOffset;

                if (fseek(file, mapFileOffset, SEEK_SET) != 0) {
                        goto fail_tiles;
                }

                /* Square data is column-major: width columns of height bytes each.
                 * After square data: metadata (creature types, ornament indices).
                 * We read width * height bytes = just the squares. */
                if ((int)fread(t->squareData, 1, squareCount, file) != squareCount) {
                        goto fail_tiles;
                }
        }

        state->tilesLoaded = 1;
        fclose(file);
        return 1;

fail_tiles:
        /* Clean up partially allocated tiles */
        for (i = 0; i < (int)state->header.mapCount; i++) {
                if (state->tiles[i].squareData) {
                        free(state->tiles[i].squareData);
                        state->tiles[i].squareData = NULL;
                }
        }
        free(state->tiles);
        state->tiles = NULL;
        fclose(file);
        return 0;
}

void F0502_DUNGEON_FreeTileData_Compat(
        struct DungeonDatState_Compat* state)
{
        int i;
        if (state->tiles) {
                for (i = 0; i < (int)state->header.mapCount; i++) {
                        if (state->tiles[i].squareData) {
                                free(state->tiles[i].squareData);
                        }
                }
                free(state->tiles);
                state->tiles = NULL;
        }
        state->tilesLoaded = 0;
}

/* ---- Thing data loading (Phase 7) ---- */

const char* F0505_DUNGEON_GetThingTypeName_Compat(int thingType) {
        if (thingType >= 0 && thingType < 16) {
                return s_thingTypeNames[thingType];
        }
        return "Unknown";
}

static void decode_door(const unsigned char* raw, struct DungeonDoor_Compat* d) {
        /* Bytes 0-1: Next (THING, little-endian) */
        d->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        /* Bytes 2-3: bitfield (PC LSB-first / MEDIA016):
         *   bit  0    = Type
         *   bits 4:1  = OrnamentOrdinal
         *   bit  5    = Vertical
         *   bit  6    = Button
         *   bit  7    = MagicDestructible
         *   bit  8    = MeleeDestructible
         *   bits 15:9 = Unreferenced
         */
        unsigned short bf = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        d->type              = (unsigned char)(bf & 0x01u);
        d->ornamentOrdinal   = (unsigned char)((bf >> 1) & 0x0Fu);
        d->vertical          = (unsigned char)((bf >> 5) & 0x01u);
        d->button            = (unsigned char)((bf >> 6) & 0x01u);
        d->magicDestructible = (unsigned char)((bf >> 7) & 0x01u);
        d->meleeDestructible = (unsigned char)((bf >> 8) & 0x01u);
}

static void decode_textstring(const unsigned char* raw, struct DungeonTextString_Compat* ts) {
        ts->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        /* Bytes 2-3: bitfield (PC LSB-first / MEDIA016):
         *   bit  0     = Visible
         *   bits 2:1   = Unreferenced
         *   bits 15:3  = TextDataWordOffset
         */
        unsigned short bf = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        ts->visible           = (unsigned char)(bf & 0x01u);
        ts->textDataWordOffset = (unsigned short)((bf >> 3) & 0x1FFFu);
}

static void decode_sensor(const unsigned char* raw, struct DungeonSensor_Compat* s) {
        /* Bytes 0-1: Next (THING, little-endian) */
        s->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        /* Bytes 2-3: Type_Data (little-endian)
         *   bits 6:0  = Type   (M039_TYPE)
         *   bits 15:7 = Data   (M040_DATA)
         */
        unsigned short typeData = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        s->sensorType = (unsigned char)(typeData & 0x007Fu);
        s->sensorData = (unsigned short)(typeData >> 7);
        /* Bytes 4-5: common bitfield (PC LSB-first / MEDIA016):
         *   bits 1:0   = aUnreferenced
         *   bit  2     = OnceOnly
         *   bits 4:3   = Effect
         *   bit  5     = RevertEffect
         *   bit  6     = Audible
         *   bits 10:7  = Value
         *   bit  11    = LocalEffect
         *   bits 15:12 = OrnamentOrdinal
         */
        unsigned short bf1 = (unsigned short)(raw[4] | ((unsigned short)raw[5] << 8));
        s->onceOnly        = (unsigned char)((bf1 >> 2) & 0x01u);
        s->effect          = (unsigned char)((bf1 >> 3) & 0x03u);
        s->revertEffect    = (unsigned char)((bf1 >> 5) & 0x01u);
        s->audible         = (unsigned char)((bf1 >> 6) & 0x01u);
        s->value           = (unsigned char)((bf1 >> 7) & 0x0Fu);
        s->localEffect     = (unsigned char)((bf1 >> 11) & 0x01u);
        s->ornamentOrdinal = (unsigned char)((bf1 >> 12) & 0x0Fu);
        /* Bytes 6-7: Remote interpretation:
         *   bits 3:0   = bUnreferenced
         *   bits 5:4   = TargetCell
         *   bits 10:6  = TargetMapX
         *   bits 15:11 = TargetMapY
         * Local interpretation:
         *   bits 11:0  = Multiple
         *   bits 15:12 = bUnreferenced
         */
        unsigned short bf2 = (unsigned short)(raw[6] | ((unsigned short)raw[7] << 8));
        s->targetCell    = (unsigned char)((bf2 >> 4) & 0x03u);
        s->targetMapX    = (unsigned char)((bf2 >> 6) & 0x1Fu);
        s->targetMapY    = (unsigned char)((bf2 >> 11) & 0x1Fu);
        s->localMultiple = (unsigned short)(bf2 & 0x0FFFu);
}

static void decode_teleporter(const unsigned char* raw, struct DungeonTeleporter_Compat* tp) {
        tp->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        /* Bytes 2-3: bitfield word 1 (PC LSB-first / MEDIA016):
         *   bits 4:0   = TargetMapX
         *   bits 9:5   = TargetMapY
         *   bits 11:10 = Rotation
         *   bit  12    = AbsoluteRotation
         *   bits 14:13 = Scope
         *   bit  15    = Audible
         */
        unsigned short bf1 = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        tp->targetMapX       = (unsigned char)(bf1 & 0x1Fu);
        tp->targetMapY       = (unsigned char)((bf1 >> 5) & 0x1Fu);
        tp->rotation         = (unsigned char)((bf1 >> 10) & 0x03u);
        tp->absoluteRotation = (unsigned char)((bf1 >> 12) & 0x01u);
        tp->scope            = (unsigned char)((bf1 >> 13) & 0x03u);
        tp->audible          = (unsigned char)((bf1 >> 15) & 0x01u);
        /* Bytes 4-5: bitfield word 2:
         *   bits 7:0   = Unreferenced
         *   bits 15:8  = TargetMapIndex
         */
        unsigned short bf2 = (unsigned short)(raw[4] | ((unsigned short)raw[5] << 8));
        tp->targetMapIndex = (unsigned char)((bf2 >> 8) & 0xFFu);
}

/* ---- Phase 9: decode Group (type 4, 16 bytes) ---- */
static void decode_group(const unsigned char* raw, struct DungeonGroup_Compat* g) {
        g->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        g->slot = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        g->creatureType = raw[4];
        g->cells        = raw[5];
        g->health[0] = (unsigned short)(raw[6]  | ((unsigned short)raw[7]  << 8));
        g->health[1] = (unsigned short)(raw[8]  | ((unsigned short)raw[9]  << 8));
        g->health[2] = (unsigned short)(raw[10] | ((unsigned short)raw[11] << 8));
        g->health[3] = (unsigned short)(raw[12] | ((unsigned short)raw[13] << 8));
        /* Bytes 14-15: bitfield (PC LSB-first / MEDIA016):
         *   bits 3:0  = Behavior
         *   bit  4    = aUnreferenced
         *   bits 6:5  = Count
         *   bit  7    = bUnreferenced
         *   bits 9:8  = Direction
         *   bit  10   = DoNotDiscard
         *   bits 15:11= cUnreferenced
         */
        unsigned short bf = (unsigned short)(raw[14] | ((unsigned short)raw[15] << 8));
        g->behavior     = (unsigned char)(bf & 0x0Fu);
        g->count        = (unsigned char)((bf >> 5) & 0x03u);
        g->direction    = (unsigned char)((bf >> 8) & 0x03u);
        g->doNotDiscard = (unsigned char)((bf >> 10) & 0x01u);
}

/* ---- Phase 9: decode Weapon (type 5, 4 bytes) ---- */
static void decode_weapon(const unsigned char* raw, struct DungeonWeapon_Compat* w) {
        w->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        /* Bytes 2-3: bitfield (MEDIA016 / PC LSB-first):
         *   bits 6:0  = Type
         *   bit  7    = DoNotDiscard
         *   bit  8    = Cursed
         *   bit  9    = Poisoned
         *   bits 13:10= ChargeCount
         *   bit  14   = Broken
         *   bit  15   = Lit
         */
        unsigned short bf = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        w->type         = (unsigned char)(bf & 0x7Fu);
        w->doNotDiscard = (unsigned char)((bf >> 7) & 0x01u);
        w->cursed       = (unsigned char)((bf >> 8) & 0x01u);
        w->poisoned     = (unsigned char)((bf >> 9) & 0x01u);
        w->chargeCount  = (unsigned char)((bf >> 10) & 0x0Fu);
        w->broken       = (unsigned char)((bf >> 14) & 0x01u);
        w->lit          = (unsigned char)((bf >> 15) & 0x01u);
}

/* ---- Phase 9: decode Armour (type 6, 4 bytes) ---- */
static void decode_armour(const unsigned char* raw, struct DungeonArmour_Compat* a) {
        a->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        /* Bytes 2-3: bitfield (MEDIA016):
         *   bits 6:0  = Type
         *   bit  7    = DoNotDiscard
         *   bit  8    = Cursed
         *   bits 12:9 = ChargeCount
         *   bit  13   = Broken
         *   bits 15:14= Unreferenced
         */
        unsigned short bf = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        a->type         = (unsigned char)(bf & 0x7Fu);
        a->doNotDiscard = (unsigned char)((bf >> 7) & 0x01u);
        a->cursed       = (unsigned char)((bf >> 8) & 0x01u);
        a->chargeCount  = (unsigned char)((bf >> 9) & 0x0Fu);
        a->broken       = (unsigned char)((bf >> 13) & 0x01u);
}

/* ---- Phase 9: decode Scroll (type 7, 4 bytes) ---- */
static void decode_scroll(const unsigned char* raw, struct DungeonScroll_Compat* s) {
        s->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        /* Bytes 2-3 (MEDIA016):
         *   bits 9:0  = TextStringThingIndex
         *   bits 15:10= Closed
         */
        unsigned short bf = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        s->textStringThingIndex = (unsigned short)(bf & 0x03FFu);
        s->closed               = (unsigned char)((bf >> 10) & 0x3Fu);
}

/* ---- Phase 9: decode Potion (type 8, 4 bytes) ---- */
static void decode_potion(const unsigned char* raw, struct DungeonPotion_Compat* p) {
        p->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        /* Bytes 2-3 (MEDIA016):
         *   bits 7:0  = Power
         *   bits 14:8 = Type
         *   bit  15   = DoNotDiscard
         */
        unsigned short bf = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        p->power        = (unsigned char)(bf & 0xFFu);
        p->type         = (unsigned char)((bf >> 8) & 0x7Fu);
        p->doNotDiscard = (unsigned char)((bf >> 15) & 0x01u);
}

/* ---- Phase 9: decode Container (type 9, 8 bytes) ---- */
static void decode_container(const unsigned char* raw, struct DungeonContainer_Compat* c) {
        c->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        c->slot = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        /* Bytes 4-5 (MEDIA016):
         *   bit  0    = aUnreferenced
         *   bits 2:1  = Type
         *   bits 15:3 = bUnreferenced
         */
        unsigned short bf = (unsigned short)(raw[4] | ((unsigned short)raw[5] << 8));
        c->type = (unsigned char)((bf >> 1) & 0x03u);
        /* Bytes 6-7: cUnreferenced (BUG0_00) */
}

/* ---- Phase 9: decode Junk (type 10, 4 bytes) ---- */
static void decode_junk(const unsigned char* raw, struct DungeonJunk_Compat* j) {
        j->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        /* Bytes 2-3 (MEDIA016):
         *   bits 6:0  = Type
         *   bit  7    = DoNotDiscard
         *   bit  8    = Cursed
         *   bits 13:9 = Unreferenced
         *   bits 15:14= ChargeCount
         */
        unsigned short bf = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        j->type         = (unsigned char)(bf & 0x7Fu);
        j->doNotDiscard = (unsigned char)((bf >> 7) & 0x01u);
        j->cursed       = (unsigned char)((bf >> 8) & 0x01u);
        j->chargeCount  = (unsigned char)((bf >> 14) & 0x03u);
}

/* ---- Phase 9: decode Projectile (type 14, 8 bytes) ---- */
static void decode_projectile(const unsigned char* raw, struct DungeonProjectile_Compat* p) {
        p->next          = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        p->slot          = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        p->kineticEnergy = raw[4];
        p->attack        = raw[5];
        p->eventIndex    = (unsigned short)(raw[6] | ((unsigned short)raw[7] << 8));
}

/* ---- Phase 9: decode Explosion (type 15, 4 bytes) ---- */
static void decode_explosion(const unsigned char* raw, struct DungeonExplosion_Compat* e) {
        e->next = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
        /* Bytes 2-3 (MEDIA016):
         *   bits 6:0  = Type
         *   bit  7    = Centered
         *   bits 15:8 = Attack
         */
        unsigned short bf = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        e->type     = (unsigned char)(bf & 0x7Fu);
        e->centered = (unsigned char)((bf >> 7) & 0x01u);
        e->attack   = (unsigned char)((bf >> 8) & 0xFFu);
}

int F0504_DUNGEON_LoadThingData_Compat(
        const char* path,
        struct DungeonDatState_Compat* state,
        struct DungeonThings_Compat* things)
{
        FILE* file;
        long squareFirstThingsOffset;
        long thingDataOffset;
        int totalColumns = 0;
        int i;

        memset(things, 0, sizeof(*things));

        if (!state->loaded) return 0;

        file = fopen(path, "rb");
        if (!file) return 0;

        /* File layout: header → maps → cumtable → SFT → thingdata → rawmapdata → textdata → checksum(2)
         * Cumtable has totalColumns entries (NOT totalColumns+1).
         * SFT starts right after cumulative thing count table. */
        for (i = 0; i < (int)state->header.mapCount; i++) {
                totalColumns += state->maps[i].width;
        }

        squareFirstThingsOffset = DUNGEON_HEADER_SIZE +
                (long)state->header.mapCount * DUNGEON_MAP_DESC_SIZE +
                (long)totalColumns * 2;

        thingDataOffset = squareFirstThingsOffset +
                (long)state->header.squareFirstThingCount * 2;

        /* Text data is between SFT and thing data in the file layout:
         * header → maps → cumtable → SFT → TEXTDATA → thingdata → rawmapdata → checksum(2)
         */
        long textDataOffset = thingDataOffset;
        thingDataOffset += (long)state->header.textDataWordCount * 2;

        /* ---- Read SquareFirstThings ---- */
        things->squareFirstThingCount = (int)state->header.squareFirstThingCount;
        if (things->squareFirstThingCount > 0) {
                things->squareFirstThings = (unsigned short*)calloc(
                        things->squareFirstThingCount, sizeof(unsigned short));
                if (!things->squareFirstThings) goto fail;

                if (fseek(file, squareFirstThingsOffset, SEEK_SET) != 0) goto fail;
                for (i = 0; i < things->squareFirstThingCount; i++) {
                        unsigned char bytes[2];
                        if (fread(bytes, 1, 2, file) != 2) goto fail;
                        things->squareFirstThings[i] = (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
                }
        }

        /* ---- Read raw thing data for all 16 types ---- */
        {
                long offset = thingDataOffset;
                for (i = 0; i < 16; i++) {
                        things->thingCounts[i] = (int)state->header.thingCounts[i];
                        int byteCount = things->thingCounts[i] * (int)s_thingDataByteCount[i];
                        if (byteCount > 0) {
                                things->rawThingData[i] = (unsigned char*)malloc(byteCount);
                                if (!things->rawThingData[i]) goto fail;
                                if (fseek(file, offset, SEEK_SET) != 0) goto fail;
                                if ((int)fread(things->rawThingData[i], 1, byteCount, file) != byteCount) goto fail;
                        }
                        offset += byteCount;
                }
        }

        /* ---- Read text data (located between SFT and thing data) ---- */
        things->textDataWordCount = (int)state->header.textDataWordCount;
        if (things->textDataWordCount > 0) {
                things->textData = (unsigned short*)calloc(
                        things->textDataWordCount, sizeof(unsigned short));
                if (!things->textData) goto fail;
                if (fseek(file, textDataOffset, SEEK_SET) != 0) goto fail;
                for (i = 0; i < things->textDataWordCount; i++) {
                        unsigned char bytes[2];
                        if (fread(bytes, 1, 2, file) != 2) goto fail;
                        things->textData[i] = (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
                }
        }

        /* ---- Decode Door things (type 0) ---- */
        things->doorCount = things->thingCounts[THING_TYPE_DOOR];
        if (things->doorCount > 0 && things->rawThingData[THING_TYPE_DOOR]) {
                things->doors = (struct DungeonDoor_Compat*)calloc(
                        things->doorCount, sizeof(struct DungeonDoor_Compat));
                if (!things->doors) goto fail;
                for (i = 0; i < things->doorCount; i++) {
                        decode_door(things->rawThingData[THING_TYPE_DOOR] + i * 4, &things->doors[i]);
                }
        }

        /* ---- Decode TextString things (type 2) ---- */
        things->textStringCount = things->thingCounts[THING_TYPE_TEXTSTRING];
        if (things->textStringCount > 0 && things->rawThingData[THING_TYPE_TEXTSTRING]) {
                things->textStrings = (struct DungeonTextString_Compat*)calloc(
                        things->textStringCount, sizeof(struct DungeonTextString_Compat));
                if (!things->textStrings) goto fail;
                for (i = 0; i < things->textStringCount; i++) {
                        decode_textstring(things->rawThingData[THING_TYPE_TEXTSTRING] + i * 4, &things->textStrings[i]);
                }
        }

        /* ---- Decode Teleporter things (type 1) ---- */
        things->teleporterCount = things->thingCounts[THING_TYPE_TELEPORTER];
        if (things->teleporterCount > 0 && things->rawThingData[THING_TYPE_TELEPORTER]) {
                things->teleporters = (struct DungeonTeleporter_Compat*)calloc(
                        things->teleporterCount, sizeof(struct DungeonTeleporter_Compat));
                if (!things->teleporters) goto fail;
                for (i = 0; i < things->teleporterCount; i++) {
                        decode_teleporter(things->rawThingData[THING_TYPE_TELEPORTER] + i * 6, &things->teleporters[i]);
                }
        }

        /* ---- Decode Sensor things (type 3, "actuators") ---- */
        things->sensorCount = things->thingCounts[THING_TYPE_SENSOR];
        if (things->sensorCount > 0 && things->rawThingData[THING_TYPE_SENSOR]) {
                things->sensors = (struct DungeonSensor_Compat*)calloc(
                        things->sensorCount, sizeof(struct DungeonSensor_Compat));
                if (!things->sensors) goto fail;
                for (i = 0; i < things->sensorCount; i++) {
                        decode_sensor(things->rawThingData[THING_TYPE_SENSOR] + i * 8, &things->sensors[i]);
                }
        }

        /* ---- Phase 9: Decode Group things (type 4, monsters) ---- */
        things->groupCount = things->thingCounts[THING_TYPE_GROUP];
        if (things->groupCount > 0 && things->rawThingData[THING_TYPE_GROUP]) {
                things->groups = (struct DungeonGroup_Compat*)calloc(
                        things->groupCount, sizeof(struct DungeonGroup_Compat));
                if (!things->groups) goto fail;
                for (i = 0; i < things->groupCount; i++)
                        decode_group(things->rawThingData[THING_TYPE_GROUP] + i * 16, &things->groups[i]);
        }

        /* ---- Phase 9: Decode Weapon things (type 5) ---- */
        things->weaponCount = things->thingCounts[THING_TYPE_WEAPON];
        if (things->weaponCount > 0 && things->rawThingData[THING_TYPE_WEAPON]) {
                things->weapons = (struct DungeonWeapon_Compat*)calloc(
                        things->weaponCount, sizeof(struct DungeonWeapon_Compat));
                if (!things->weapons) goto fail;
                for (i = 0; i < things->weaponCount; i++)
                        decode_weapon(things->rawThingData[THING_TYPE_WEAPON] + i * 4, &things->weapons[i]);
        }

        /* ---- Phase 9: Decode Armour things (type 6) ---- */
        things->armourCount = things->thingCounts[THING_TYPE_ARMOUR];
        if (things->armourCount > 0 && things->rawThingData[THING_TYPE_ARMOUR]) {
                things->armours = (struct DungeonArmour_Compat*)calloc(
                        things->armourCount, sizeof(struct DungeonArmour_Compat));
                if (!things->armours) goto fail;
                for (i = 0; i < things->armourCount; i++)
                        decode_armour(things->rawThingData[THING_TYPE_ARMOUR] + i * 4, &things->armours[i]);
        }

        /* ---- Phase 9: Decode Scroll things (type 7) ---- */
        things->scrollCount = things->thingCounts[THING_TYPE_SCROLL];
        if (things->scrollCount > 0 && things->rawThingData[THING_TYPE_SCROLL]) {
                things->scrolls = (struct DungeonScroll_Compat*)calloc(
                        things->scrollCount, sizeof(struct DungeonScroll_Compat));
                if (!things->scrolls) goto fail;
                for (i = 0; i < things->scrollCount; i++)
                        decode_scroll(things->rawThingData[THING_TYPE_SCROLL] + i * 4, &things->scrolls[i]);
        }

        /* ---- Phase 9: Decode Potion things (type 8) ---- */
        things->potionCount = things->thingCounts[THING_TYPE_POTION];
        if (things->potionCount > 0 && things->rawThingData[THING_TYPE_POTION]) {
                things->potions = (struct DungeonPotion_Compat*)calloc(
                        things->potionCount, sizeof(struct DungeonPotion_Compat));
                if (!things->potions) goto fail;
                for (i = 0; i < things->potionCount; i++)
                        decode_potion(things->rawThingData[THING_TYPE_POTION] + i * 4, &things->potions[i]);
        }

        /* ---- Phase 9: Decode Container things (type 9) ---- */
        things->containerCount = things->thingCounts[THING_TYPE_CONTAINER];
        if (things->containerCount > 0 && things->rawThingData[THING_TYPE_CONTAINER]) {
                things->containers = (struct DungeonContainer_Compat*)calloc(
                        things->containerCount, sizeof(struct DungeonContainer_Compat));
                if (!things->containers) goto fail;
                for (i = 0; i < things->containerCount; i++)
                        decode_container(things->rawThingData[THING_TYPE_CONTAINER] + i * 8, &things->containers[i]);
        }

        /* ---- Phase 9: Decode Junk things (type 10) ---- */
        things->junkCount = things->thingCounts[THING_TYPE_JUNK];
        if (things->junkCount > 0 && things->rawThingData[THING_TYPE_JUNK]) {
                things->junks = (struct DungeonJunk_Compat*)calloc(
                        things->junkCount, sizeof(struct DungeonJunk_Compat));
                if (!things->junks) goto fail;
                for (i = 0; i < things->junkCount; i++)
                        decode_junk(things->rawThingData[THING_TYPE_JUNK] + i * 4, &things->junks[i]);
        }

        /* ---- Phase 9: Decode Projectile things (type 14) ---- */
        things->projectileCount = things->thingCounts[THING_TYPE_PROJECTILE];
        if (things->projectileCount > 0 && things->rawThingData[THING_TYPE_PROJECTILE]) {
                things->projectiles = (struct DungeonProjectile_Compat*)calloc(
                        things->projectileCount, sizeof(struct DungeonProjectile_Compat));
                if (!things->projectiles) goto fail;
                for (i = 0; i < things->projectileCount; i++)
                        decode_projectile(things->rawThingData[THING_TYPE_PROJECTILE] + i * 8, &things->projectiles[i]);
        }

        /* ---- Phase 9: Decode Explosion things (type 15) ---- */
        things->explosionCount = things->thingCounts[THING_TYPE_EXPLOSION];
        if (things->explosionCount > 0 && things->rawThingData[THING_TYPE_EXPLOSION]) {
                things->explosions = (struct DungeonExplosion_Compat*)calloc(
                        things->explosionCount, sizeof(struct DungeonExplosion_Compat));
                if (!things->explosions) goto fail;
                for (i = 0; i < things->explosionCount; i++)
                        decode_explosion(things->rawThingData[THING_TYPE_EXPLOSION] + i * 4, &things->explosions[i]);
        }

        things->loaded = 1;
        fclose(file);
        return 1;

fail:
        fclose(file);
        F0504_DUNGEON_FreeThingData_Compat(things);
        return 0;
}

void F0504_DUNGEON_FreeThingData_Compat(
        struct DungeonThings_Compat* things)
{
        int i;
        if (things->squareFirstThings) {
                free(things->squareFirstThings);
                things->squareFirstThings = NULL;
        }
        if (things->doors) {
                free(things->doors);
                things->doors = NULL;
        }
        if (things->textStrings) {
                free(things->textStrings);
                things->textStrings = NULL;
        }
        if (things->teleporters) {
                free(things->teleporters);
                things->teleporters = NULL;
        }
        if (things->sensors) {
                free(things->sensors);
                things->sensors = NULL;
        }
        if (things->groups) { free(things->groups); things->groups = NULL; }
        if (things->weapons) { free(things->weapons); things->weapons = NULL; }
        if (things->armours) { free(things->armours); things->armours = NULL; }
        if (things->scrolls) { free(things->scrolls); things->scrolls = NULL; }
        if (things->potions) { free(things->potions); things->potions = NULL; }
        if (things->containers) { free(things->containers); things->containers = NULL; }
        if (things->junks) { free(things->junks); things->junks = NULL; }
        if (things->projectiles) { free(things->projectiles); things->projectiles = NULL; }
        if (things->explosions) { free(things->explosions); things->explosions = NULL; }
        for (i = 0; i < 16; i++) {
                if (things->rawThingData[i]) {
                        free(things->rawThingData[i]);
                        things->rawThingData[i] = NULL;
                }
        }
        if (things->textData) {
                free(things->textData);
                things->textData = NULL;
        }
        things->loaded = 0;
}

/* ---- Text data decoding ---- */

/*
 * DM text encoding (from ReDMCSB DUNGEON.C / Christophe Fontanel):
 *   3 five-bit codes per 16-bit word (bit 15 unused):
 *     code[0] = (word >> 10) & 0x1F
 *     code[1] = (word >>  5) & 0x1F
 *     code[2] =  word        & 0x1F
 *
 *   0-25: 'A'-'Z', 26: ' ', 27: '.', 28: separator,
 *   29: escape (symbol), 30: escape (word), 31: end
 */

int F0507_DUNGEON_DecodeTextAtOffset_Compat(
        const unsigned short* textData,
        int                   textDataWordCount,
        int                   wordOffset,
        char*                 outBuf,
        int                   outBufSize)
{
        int wi = wordOffset;
        int codeIdx = 0;
        int pos = 0;
        int escape = 0;
        unsigned short w = 0;

        if (!textData || wordOffset < 0 || wordOffset >= textDataWordCount || !outBuf || outBufSize < 2) {
                if (outBuf && outBufSize > 0) outBuf[0] = '\0';
                return -1;
        }

        while (pos < outBufSize - 1 && wi < textDataWordCount) {
                int code;
                if (codeIdx == 0) {
                        w = textData[wi];
                        code = (w >> 10) & 0x1F;
                } else if (codeIdx == 1) {
                        code = (w >> 5) & 0x1F;
                } else {
                        code = w & 0x1F;
                }
                codeIdx++;
                if (codeIdx >= 3) {
                        codeIdx = 0;
                        wi++;
                }

                if (escape) {
                        /* Escape replacement: for v1 compat, emit placeholder */
                        if (escape == 30) {
                                /* Extended word strings: code indexes word table */
                                /* "THE ", "YOU ", etc. — emit placeholder */
                                static const char* s_wordEscape[] = {
                                        "?", "!", "THE ", "YOU ", "", "", "", "",
                                        "", "", "", "", "", "", "", "",
                                        "", "", "", "", "", "", "", "",
                                        "", "", "", "", "", "", "", ""
                                };
                                const char* rep = (code < 32) ? s_wordEscape[code] : "";
                                while (*rep && pos < outBufSize - 1) {
                                        outBuf[pos++] = *rep++;
                                }
                        } else if (escape == 29) {
                                /* Symbol escape: emit [SYM:N] placeholder */
                                outBuf[pos++] = '?'; /* placeholder for symbol */
                        }
                        escape = 0;
                } else if (code < 26) {
                        outBuf[pos++] = (char)('A' + code);
                } else if (code == 26) {
                        outBuf[pos++] = ' ';
                } else if (code == 27) {
                        outBuf[pos++] = '.';
                } else if (code == 28) {
                        outBuf[pos++] = '\n'; /* separator */
                } else if (code == 29 || code == 30) {
                        escape = code;
                } else { /* code == 31: end of text */
                        break;
                }
        }
        outBuf[pos] = '\0';
        return pos;
}

int F0506_DUNGEON_DecodeTextTable_Compat(
        const unsigned short* textData,
        int                   textDataWordCount,
        struct DungeonTextTable_Compat* table)
{
        int wi, codeIdx, strCount, strCap, pos, escape;
        unsigned short w;
        char buf[DUNGEON_TEXT_MAX_STRING_LEN];

        memset(table, 0, sizeof(*table));
        if (!textData || textDataWordCount <= 0) return 0;

        /* First pass: count strings (separated by code 31) */
        strCount = 0;
        wi = 0; codeIdx = 0; w = 0;
        while (wi < textDataWordCount) {
                int code;
                if (codeIdx == 0) { w = textData[wi]; code = (w >> 10) & 0x1F; }
                else if (codeIdx == 1) { code = (w >> 5) & 0x1F; }
                else { code = w & 0x1F; }
                codeIdx++;
                if (codeIdx >= 3) { codeIdx = 0; wi++; }
                if (code == 31) strCount++;
        }
        if (strCount == 0) return 0;

        table->strings = (char**)calloc(strCount, sizeof(char*));
        if (!table->strings) return 0;
        table->count = strCount;

        /* Second pass: decode each string */
        wi = 0; codeIdx = 0; w = 0; pos = 0; escape = 0;
        strCap = 0;
        while (wi < textDataWordCount && strCap < strCount) {
                int code;
                if (codeIdx == 0) { w = textData[wi]; code = (w >> 10) & 0x1F; }
                else if (codeIdx == 1) { code = (w >> 5) & 0x1F; }
                else { code = w & 0x1F; }
                codeIdx++;
                if (codeIdx >= 3) { codeIdx = 0; wi++; }

                if (escape) {
                        if (escape == 30 && pos < DUNGEON_TEXT_MAX_STRING_LEN - 5) {
                                static const char* s_we[] = {
                                        "?","!","THE ","YOU ","","","","",
                                        "","","","","","","","",
                                        "","","","","","","","",
                                        "","","","","","","",""
                                };
                                const char* rep = (code < 32) ? s_we[code] : "";
                                while (*rep && pos < DUNGEON_TEXT_MAX_STRING_LEN - 1)
                                        buf[pos++] = *rep++;
                        } else if (escape == 29 && pos < DUNGEON_TEXT_MAX_STRING_LEN - 1) {
                                buf[pos++] = '?';
                        }
                        escape = 0;
                } else if (code == 31) {
                        buf[pos] = '\0';
                        table->strings[strCap] = (char*)malloc(pos + 1);
                        if (table->strings[strCap]) {
                                memcpy(table->strings[strCap], buf, pos + 1);
                        }
                        strCap++;
                        pos = 0;
                } else if (code < 26) {
                        if (pos < DUNGEON_TEXT_MAX_STRING_LEN - 1) buf[pos++] = (char)('A' + code);
                } else if (code == 26) {
                        if (pos < DUNGEON_TEXT_MAX_STRING_LEN - 1) buf[pos++] = ' ';
                } else if (code == 27) {
                        if (pos < DUNGEON_TEXT_MAX_STRING_LEN - 1) buf[pos++] = '.';
                } else if (code == 28) {
                        if (pos < DUNGEON_TEXT_MAX_STRING_LEN - 1) buf[pos++] = '\n';
                } else if (code == 29 || code == 30) {
                        escape = code;
                }
        }

        return 1;
}

void F0506_DUNGEON_FreeTextTable_Compat(
        struct DungeonTextTable_Compat* table)
{
        int i;
        if (table->strings) {
                for (i = 0; i < table->count; i++) {
                        if (table->strings[i]) free(table->strings[i]);
                }
                free(table->strings);
                table->strings = NULL;
        }
        table->count = 0;
}
