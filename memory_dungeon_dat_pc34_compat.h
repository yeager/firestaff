#ifndef REDMCSB_MEMORY_DUNGEON_DAT_PC34_COMPAT_H
#define REDMCSB_MEMORY_DUNGEON_DAT_PC34_COMPAT_H

#include <stdio.h>

/*
 * DUNGEON.DAT header seam for ReDMCSB PC 3.4 (I34E).
 *
 * Reads the top-level DUNGEON_HEADER and MAP descriptors from an
 * uncompressed PC-format dungeon file.  Does NOT decode level tile data,
 * thing data, or text data — header + structure inventory only.
 *
 * On-disk format (little-endian, Watcom/Borland LSB-first bitfields):
 *   Offset 0:  DUNGEON_HEADER  (44 bytes)
 *   Offset 44: MAP[MapCount]   (16 bytes each)
 *   Then:      raw map data, thing arrays, square-first-things, text data.
 */

#define DUNGEON_HEADER_SIZE   44
#define DUNGEON_MAP_DESC_SIZE 16
#define DUNGEON_THING_TYPE_COUNT 16
#define DUNGEON_MAX_MAPS      32   /* Sanity cap */

/* Compressed-dungeon signature (save games). We reject this. */
#define DUNGEON_COMPRESSED_SIGNATURE 0x8104u

/* Known dungeon IDs (from DEFS.H) */
#define DUNGEON_ID_DM          10
#define DUNGEON_ID_CSB_PRISON  12
#define DUNGEON_ID_CSB_GAME    13
#define DUNGEON_ID_KID         50

struct DungeonHeader_Compat {
    unsigned short ornamentRandomSeed;
    unsigned short rawMapDataByteCount;
    unsigned char  mapCount;
    unsigned char  unreferenced;
    unsigned short textDataWordCount;
    unsigned short initialPartyLocation;
    unsigned short squareFirstThingCount;
    unsigned short thingCounts[DUNGEON_THING_TYPE_COUNT];
};

struct DungeonMapDesc_Compat {
    unsigned short rawMapDataByteOffset;
    unsigned short aUnreferenced;
    unsigned short bUnreferenced;
    unsigned char  offsetMapX;
    unsigned char  offsetMapY;
    /* Decoded from bitfield A (PC LSB-first packing) */
    unsigned char  level;
    unsigned char  width;   /* actual width (stored value + 1) */
    unsigned char  height;  /* actual height (stored value + 1) */
    /* Raw bitfields B, C, D (stored raw + decoded) */
    unsigned short rawBitfieldB;
    unsigned short rawBitfieldC;
    unsigned short rawBitfieldD;
    /* Decoded from bitfield B (ornament counts) */
    unsigned char  wallOrnamentCount;
    unsigned char  randomWallOrnamentCount;
    unsigned char  floorOrnamentCount;
    unsigned char  randomFloorOrnamentCount;
    /* Decoded from bitfield C */
    unsigned char  doorOrnamentCount;
    unsigned char  creatureTypeCount;
    unsigned char  difficulty;
    /* Decoded from bitfield D (graphic sets) */
    unsigned char  floorSet;
    unsigned char  wallSet;
    unsigned char  doorSet0;
    unsigned char  doorSet1;
};

/*
 * Square (tile) element types from DEFS.H.
 * M034_SQUARE_TYPE(square) = square >> 5
 */
#define DUNGEON_ELEMENT_WALL       0
#define DUNGEON_ELEMENT_CORRIDOR   1
#define DUNGEON_ELEMENT_PIT        2
#define DUNGEON_ELEMENT_STAIRS     3
#define DUNGEON_ELEMENT_DOOR       4
#define DUNGEON_ELEMENT_TELEPORTER 5
#define DUNGEON_ELEMENT_FAKEWALL   6
#define DUNGEON_ELEMENT_COUNT      7

/* Bit masks for square attribute byte (lower 5 bits) */
#define DUNGEON_SQUARE_MASK_TYPE       0xE0
#define DUNGEON_SQUARE_MASK_ATTRIBS    0x1F
#define DUNGEON_SQUARE_MASK_THING_LIST 0x10

struct DungeonMapTiles_Compat {
    unsigned char* squareData;  /* [width * height], column-major: [col*height + row] */
    int            squareCount; /* width * height */
};

struct DungeonDatState_Compat {
    struct DungeonHeader_Compat   header;
    struct DungeonMapDesc_Compat* maps;   /* [header.mapCount] */
    struct DungeonMapTiles_Compat* tiles;  /* [header.mapCount], NULL until tiles loaded */
    long                          fileSize;
    int                           loaded;
    int                           tilesLoaded;
};

/*
 * Load DUNGEON_HEADER + MAP descriptors from an uncompressed PC dungeon file.
 * Returns 1 on success, 0 on failure (sets state->loaded = 0).
 */
int F0500_DUNGEON_LoadDatHeader_Compat(
    const char* path,
    struct DungeonDatState_Compat* state);

/*
 * Free dynamically allocated map descriptor array.
 */
void F0500_DUNGEON_FreeDatHeader_Compat(
    struct DungeonDatState_Compat* state);

/*
 * Decode party location bitfield.
 * direction: bits 11-10, y: bits 9-5, x: bits 4-0
 */
void F0501_DUNGEON_DecodePartyLocation_Compat(
    unsigned short partyLocation,
    int* outDirection,
    int* outY,
    int* outX);

/*
 * Load tile (square) data for all maps from DUNGEON.DAT.
 * Must be called after F0500_DUNGEON_LoadDatHeader_Compat succeeds.
 * Returns 1 on success, 0 on failure.
 */
int F0502_DUNGEON_LoadTileData_Compat(
    const char* path,
    struct DungeonDatState_Compat* state);

/*
 * Free tile data.
 */
void F0502_DUNGEON_FreeTileData_Compat(
    struct DungeonDatState_Compat* state);

/*
 * Get element type name string for a square element type (0-6).
 */
const char* F0503_DUNGEON_GetElementName_Compat(int elementType);

/* ---- Thing data decoding (Phase 7) ---- */

/*
 * THING encoding (uint16):
 *   bits  9:0  = index into ThingData[type] array
 *   bits 13:10 = thing type/category (0-15)
 *   bits 15:14 = direction (used at runtime, not in thing data arrays)
 *
 * Special values:
 *   0xFFFF = THING_NONE   (unused slot)
 *   0xFFFE = THING_ENDOFLIST (end of linked list)
 */
#define THING_TYPE_DOOR        0
#define THING_TYPE_TELEPORTER  1
#define THING_TYPE_TEXTSTRING  2
#define THING_TYPE_SENSOR      3
#define THING_TYPE_GROUP       4
#define THING_TYPE_WEAPON      5
#define THING_TYPE_ARMOUR      6
#define THING_TYPE_SCROLL      7
#define THING_TYPE_POTION      8
#define THING_TYPE_CONTAINER   9
#define THING_TYPE_JUNK       10
#define THING_TYPE_PROJECTILE 14
#define THING_TYPE_EXPLOSION  15

#define THING_NONE       0xFFFFu
#define THING_ENDOFLIST  0xFFFEu

#define THING_GET_TYPE(t)  (((t) & 0x3C00u) >> 10)
#define THING_GET_INDEX(t) ((t) & 0x03FFu)

/* Per-type byte counts (matches G0235_auc_Graphic559_ThingDataByteCount) */
static const unsigned char s_thingDataByteCount[16] = {
    4,  /* 0  Door */
    6,  /* 1  Teleporter */
    4,  /* 2  TextString */
    8,  /* 3  Sensor */
    16, /* 4  Group */
    4,  /* 5  Weapon */
    4,  /* 6  Armour */
    4,  /* 7  Scroll */
    4,  /* 8  Potion */
    8,  /* 9  Container */
    4,  /* 10 Junk */
    0,  /* 11 Unused */
    0,  /* 12 Unused */
    0,  /* 13 Unused */
    8,  /* 14 Projectile */
    4   /* 15 Explosion */
};

static const char* const s_thingTypeNames[16] __attribute__((unused)) = {
    "Door", "Teleporter", "TextString", "Sensor",
    "Group", "Weapon", "Armour", "Scroll",
    "Potion", "Container", "Junk", "Unused11",
    "Unused12", "Unused13", "Projectile", "Explosion"
};

/* Decoded Door thing (type 0, 4 bytes on disk) */
struct DungeonDoor_Compat {
    unsigned short next;           /* THING: Next in linked list */
    unsigned char  type;           /* 1 bit: 0=set0, 1=set1 */
    unsigned char  ornamentOrdinal; /* 4 bits: 0-15 */
    unsigned char  vertical;       /* 1 bit */
    unsigned char  button;         /* 1 bit */
    unsigned char  magicDestructible; /* 1 bit */
    unsigned char  meleeDestructible; /* 1 bit */
};

/* Decoded TextString thing (type 2, 4 bytes on disk) */
struct DungeonTextString_Compat {
    unsigned short next;              /* THING: Next in linked list */
    unsigned char  visible;           /* 1 bit */
    unsigned short textDataWordOffset; /* 13 bits: offset into text data */
};

/* Decoded Teleporter thing (type 1, 6 bytes on disk) */
struct DungeonTeleporter_Compat {
    unsigned short next;              /* THING */
    unsigned char  targetMapX;        /* 5 bits */
    unsigned char  targetMapY;        /* 5 bits */
    unsigned char  rotation;          /* 2 bits */
    unsigned char  absoluteRotation;  /* 1 bit */
    unsigned char  scope;             /* 2 bits */
    unsigned char  audible;           /* 1 bit */
    unsigned char  targetMapIndex;    /* 8 bits */
};

/*
 * Decoded Sensor thing (type 3, 8 bytes on disk).
 * Sensors are the "actuators" of the dungeon: wall buttons, floor triggers,
 * pressure plates, projectile launchers, group generators, etc.
 *
 * The on-disk struct is a union (Remote vs Local) but the first 6 bytes
 * share identical layout; only bytes 6-7 differ in interpretation.
 * We decode both interpretations; the consumer picks based on sensorType.
 *
 * M039_TYPE(sensor) = Type_Data & 0x007F   (7-bit type)
 * M040_DATA(sensor) = Type_Data >> 7       (9-bit data)
 */
struct DungeonSensor_Compat {
    unsigned short next;             /* THING: Next in linked list */
    unsigned char  sensorType;       /* 7 bits: type code (0-127) */
    unsigned short sensorData;       /* 9 bits: context-dependent data */
    /* Common bitfield (bytes 4-5, PC LSB-first / MEDIA016): */
    unsigned char  onceOnly;         /* 1 bit */
    unsigned char  effect;           /* 2 bits */
    unsigned char  revertEffect;     /* 1 bit */
    unsigned char  audible;          /* 1 bit */
    unsigned char  value;            /* 4 bits */
    unsigned char  localEffect;      /* 1 bit */
    unsigned char  ornamentOrdinal;  /* 4 bits */
    /* Remote interpretation of bytes 6-7: */
    unsigned char  targetCell;       /* 2 bits (ignored for non-wall squares) */
    unsigned char  targetMapX;       /* 5 bits */
    unsigned char  targetMapY;       /* 5 bits */
    /* Local interpretation of bytes 6-7: */
    unsigned short localMultiple;    /* 12 bits (health mult, ticks, kinetic energy, etc.) */
};

/* Sensor type ranges (from DEFS.H) */
#define SENSOR_FLOOR_TYPE_MIN   0
#define SENSOR_FLOOR_TYPE_MAX   9
#define SENSOR_WALL_TYPE_MIN    0
#define SENSOR_WALL_TYPE_MAX   18
#define SENSOR_WALL_TYPE_PORTRAIT 127

/* ---- Monster Group (type 4, 16 bytes on disk) ---- */
struct DungeonGroup_Compat {
    unsigned short next;          /* THING: Next in linked list */
    unsigned short slot;          /* THING: Slot (possession chain for group) */
    unsigned char  creatureType;  /* 8 bits: creature type index (0-26 for DM) */
    unsigned char  cells;         /* 8 bits: cell positions for up to 4 creatures */
    unsigned short health[4];     /* HP for each creature slot */
    /* Bitfield word (bytes 14-15, MEDIA016 / PC LSB-first): */
    unsigned char  behavior;      /* 4 bits */
    unsigned char  count;         /* 2 bits: actual count = count + 1 */
    unsigned char  direction;     /* 2 bits */
    unsigned char  doNotDiscard;  /* 1 bit */
};

/* ---- Weapon (type 5, 4 bytes on disk) ---- */
struct DungeonWeapon_Compat {
    unsigned short next;          /* THING: Next in linked list */
    unsigned char  type;          /* 7 bits */
    unsigned char  doNotDiscard;  /* 1 bit */
    unsigned char  cursed;        /* 1 bit */
    unsigned char  poisoned;      /* 1 bit */
    unsigned char  chargeCount;   /* 4 bits */
    unsigned char  broken;        /* 1 bit */
    unsigned char  lit;           /* 1 bit (torches) */
};

/* ---- Armour (type 6, 4 bytes on disk) ---- */
struct DungeonArmour_Compat {
    unsigned short next;
    unsigned char  type;          /* 7 bits */
    unsigned char  doNotDiscard;  /* 1 bit */
    unsigned char  cursed;        /* 1 bit */
    unsigned char  chargeCount;   /* 4 bits */
    unsigned char  broken;        /* 1 bit */
};

/* ---- Scroll (type 7, 4 bytes on disk) ---- */
struct DungeonScroll_Compat {
    unsigned short next;
    unsigned short textStringThingIndex; /* 10 bits */
    unsigned char  closed;               /* 6 bits */
};

/* ---- Potion (type 8, 4 bytes on disk) ---- */
struct DungeonPotion_Compat {
    unsigned short next;
    unsigned char  power;         /* 8 bits */
    unsigned char  type;          /* 7 bits */
    unsigned char  doNotDiscard;  /* 1 bit */
};

/* ---- Container (type 9, 8 bytes on disk) ---- */
struct DungeonContainer_Compat {
    unsigned short next;
    unsigned short slot;          /* THING: contents chain */
    unsigned char  type;          /* 2 bits */
};

/* ---- Junk (type 10, 4 bytes on disk) ---- */
struct DungeonJunk_Compat {
    unsigned short next;
    unsigned char  type;          /* 7 bits */
    unsigned char  doNotDiscard;  /* 1 bit */
    unsigned char  cursed;        /* 1 bit */
    unsigned char  chargeCount;   /* 2 bits */
};

/* ---- Projectile (type 14, 8 bytes on disk) ---- */
struct DungeonProjectile_Compat {
    unsigned short next;
    unsigned short slot;          /* THING: projectile thing */
    unsigned char  kineticEnergy; /* 8 bits */
    unsigned char  attack;        /* 8 bits */
    unsigned short eventIndex;    /* 16 bits */
};

/* ---- Explosion (type 15, 4 bytes on disk) ---- */
struct DungeonExplosion_Compat {
    unsigned short next;
    unsigned char  type;          /* 7 bits */
    unsigned char  centered;      /* 1 bit */
    unsigned char  attack;        /* 8 bits */
};

/* Max known creature types in DM */
#define DUNGEON_CREATURE_TYPE_MAX 26

/* Max known weapon types */
#define DUNGEON_WEAPON_TYPE_MAX   45
/* Max known armour types */
#define DUNGEON_ARMOUR_TYPE_MAX   57
/* Max known junk types */
#define DUNGEON_JUNK_TYPE_MAX     52
/* Max known potion types */
#define DUNGEON_POTION_TYPE_MAX   20
/* Max known container types */
#define DUNGEON_CONTAINER_TYPE_MAX 3

/* All decoded thing data for the dungeon */
struct DungeonThings_Compat {
    /* SquareFirstThings array */
    unsigned short* squareFirstThings;
    int             squareFirstThingCount;

    /* Decoded things per type */
    struct DungeonDoor_Compat*       doors;
    int                              doorCount;
    struct DungeonTextString_Compat* textStrings;
    int                              textStringCount;
    struct DungeonTeleporter_Compat* teleporters;
    int                              teleporterCount;
    struct DungeonSensor_Compat*     sensors;
    int                              sensorCount;

    /* Phase 9: monsters and items */
    struct DungeonGroup_Compat*      groups;
    int                              groupCount;
    struct DungeonWeapon_Compat*     weapons;
    int                              weaponCount;
    struct DungeonArmour_Compat*     armours;
    int                              armourCount;
    struct DungeonScroll_Compat*     scrolls;
    int                              scrollCount;
    struct DungeonPotion_Compat*     potions;
    int                              potionCount;
    struct DungeonContainer_Compat*  containers;
    int                              containerCount;
    struct DungeonJunk_Compat*       junks;
    int                              junkCount;
    struct DungeonProjectile_Compat* projectiles;
    int                              projectileCount;
    struct DungeonExplosion_Compat*  explosions;
    int                              explosionCount;

    /* Raw thing data blobs for all 16 types (for future decoding) */
    unsigned char* rawThingData[16];
    int            thingCounts[16];

    /* Text data */
    unsigned short* textData;
    int             textDataWordCount;

    int loaded;
};

/*
 * Load SquareFirstThings, thing data (all types raw + Door/TextString/Teleporter decoded),
 * and text data from DUNGEON.DAT.
 * Requires header+tiles already loaded.
 * Returns 1 on success.
 */
int F0504_DUNGEON_LoadThingData_Compat(
    const char* path,
    struct DungeonDatState_Compat* state,
    struct DungeonThings_Compat* things);

/*
 * Free thing data.
 */
void F0504_DUNGEON_FreeThingData_Compat(
    struct DungeonThings_Compat* things);

/*
 * Get thing type name.
 */
const char* F0505_DUNGEON_GetThingTypeName_Compat(int thingType);

/* ---- Text data decoding (Phase 7 text-strings) ---- */

/*
 * DM text encoding (from ReDMCSB DUNGEON.C):
 *   Each 16-bit word stores 3 five-bit codes:
 *     code[0] = (word >> 10) & 0x1F
 *     code[1] = (word >>  5) & 0x1F
 *     code[2] =  word        & 0x1F
 *   Bit 15 of each word is unused.
 *
 *   Codes 0-25: 'A'-'Z'
 *   Code 26:    ' ' (space)
 *   Code 27:    '.' (period)
 *   Code 28:    separator (newline or 0x80 per context)
 *   Code 29:    escape → next code indexes symbol table
 *   Code 30:    escape → next code indexes word table ("THE ", "YOU ", etc.)
 *   Code 31:    end of text
 *
 * v2-design note: Strings are exposed by INDEX, not by content.
 * The table is language-dependent; index is the contract.
 */

#define DUNGEON_TEXT_MAX_STRING_LEN 128  /* Max decoded string length per entry */
#define DUNGEON_TEXT_CODE_END       31   /* End-of-text code */

struct DungeonTextTable_Compat {
    int    count;    /* Number of decoded strings */
    char** strings;  /* [count] null-terminated decoded strings (heap-allocated) */
};

/*
 * Decode ALL text strings from the textData section.
 * Walks the entire textData word array, splitting on code-31 (end).
 * Each string is stored as a heap-allocated null-terminated char array.
 * Returns 1 on success, 0 on failure.
 */
int F0506_DUNGEON_DecodeTextTable_Compat(
    const unsigned short* textData,
    int                   textDataWordCount,
    struct DungeonTextTable_Compat* table);

/*
 * Decode a single text string starting at the given word offset.
 * Writes to outBuf (max outBufSize chars including NUL).
 * Returns number of characters written (excluding NUL), or -1 on error.
 */
int F0507_DUNGEON_DecodeTextAtOffset_Compat(
    const unsigned short* textData,
    int                   textDataWordCount,
    int                   wordOffset,
    char*                 outBuf,
    int                   outBufSize);

/*
 * Free text table.
 */
void F0506_DUNGEON_FreeTextTable_Compat(
    struct DungeonTextTable_Compat* table);

#endif
