#ifndef REDMCSB_MEMORY_CHAMPION_STATE_PC34_COMPAT_H
#define REDMCSB_MEMORY_CHAMPION_STATE_PC34_COMPAT_H

/*
 * Champion + Party state data layer for ReDMCSB PC 3.4.
 *
 * Pure data structs and init functions — NO UI, NO render, NO strings,
 * NO cheats, NO config, NO platform-specific code.
 *
 * Source: CHAMPION.C, DEFS.H from the original Dungeon Master / CSB source.
 *
 * v2-design: All data accessed by INDEX/ID.  Party/champion state is a
 * serializable pure-data struct for save/load.  Language-dependent strings
 * are NOT stored here.
 */

#include "memory_dungeon_dat_pc34_compat.h"

/* ---- Direction constants (shared with movement) ---- */
#define DIR_NORTH 0
#define DIR_EAST  1
#define DIR_SOUTH 2
#define DIR_WEST  3
#define DIR_COUNT 4

/* ---- Champion slot limits ---- */
#define CHAMPION_MAX_PARTY   4
#define CHAMPION_NAME_LENGTH 8  /* Packed name, 8 chars max (no NUL in original) */
#define CHAMPION_TITLE_LENGTH 20 /* Packed title, 20 chars max (no NUL in original) */
#define CHAMPION_MIRROR_FIELD_LENGTH 16 /* Packed source mirror stat/skill text field */
#define CHAMPION_MIRROR_INVENTORY_TEXT_LENGTH 32 /* Packed source mirror inventory text */

/* ---- Attribute indices (CHAMPION.C ordering) ---- */
#define CHAMPION_ATTR_STRENGTH   0
#define CHAMPION_ATTR_DEXTERITY  1
#define CHAMPION_ATTR_WISDOM     2
#define CHAMPION_ATTR_VITALITY   3
#define CHAMPION_ATTR_ANTIMAGIC  4
#define CHAMPION_ATTR_ANTIFIRE   5
#define CHAMPION_ATTR_COUNT      6

/* ---- Skill indices (CHAMPION.C ordering) ---- */
#define CHAMPION_SKILL_FIGHTER   0
#define CHAMPION_SKILL_NINJA     1
#define CHAMPION_SKILL_PRIEST    2
#define CHAMPION_SKILL_WIZARD    3
#define CHAMPION_SKILL_COUNT     4

/* ---- Inventory slot indices (CHAMPION.C / DEFS.H) ---- */
#define CHAMPION_SLOT_HEAD        0
#define CHAMPION_SLOT_NECK        1
#define CHAMPION_SLOT_TORSO       2
#define CHAMPION_SLOT_LEGS        3
#define CHAMPION_SLOT_FEET        4
#define CHAMPION_SLOT_POUCH_1     5
#define CHAMPION_SLOT_POUCH_2     6
#define CHAMPION_SLOT_QUIVER_1    7
#define CHAMPION_SLOT_QUIVER_2    8
#define CHAMPION_SLOT_QUIVER_3    9
#define CHAMPION_SLOT_QUIVER_4   10
#define CHAMPION_SLOT_BACKPACK_1 11
#define CHAMPION_SLOT_BACKPACK_2 12
#define CHAMPION_SLOT_BACKPACK_3 13
#define CHAMPION_SLOT_BACKPACK_4 14
#define CHAMPION_SLOT_BACKPACK_5 15
#define CHAMPION_SLOT_BACKPACK_6 16
#define CHAMPION_SLOT_BACKPACK_7 17
#define CHAMPION_SLOT_BACKPACK_8 18
#define CHAMPION_SLOT_HAND_LEFT  19
#define CHAMPION_SLOT_HAND_RIGHT 20
#define CHAMPION_SLOT_ACTION_HAND 21  /* set equal to left or right */
#define CHAMPION_SLOT_COUNT      30   /* total slots including unused */

/* ---- Champion stat (current/max/shifted triple, per CHAMPION.C) ---- */
struct ChampionStat_Compat {
    unsigned short current;
    unsigned short maximum;
    unsigned short shifted;  /* max << 1 for internal precision */
};

/*
 * Champion state — pure data struct.
 * All fields accessible by index.  No pointers to UI/render/strings.
 *
 * Source fields mapped from CHAMPION.C Champion struct.
 * Fields marked [DUNGEON.DAT] come from the initial dungeon data.
 * Fields marked [RUNTIME] are zero-initialised and set during gameplay.
 */
struct ChampionState_Compat {
    /* Identity (portrait index is the canonical champion ID) */
    int            portraitIndex;           /* [DUNGEON.DAT] sensor data for mirror */
    unsigned char  name[CHAMPION_NAME_LENGTH]; /* [DUNGEON.DAT] packed 8-char name */
    unsigned char  title[CHAMPION_TITLE_LENGTH]; /* [DUNGEON.DAT] packed title */
    unsigned char  sex;                     /* [DUNGEON.DAT] champion sex byte ('M'/'F') */
    unsigned char  mirrorStatsText[CHAMPION_MIRROR_FIELD_LENGTH];  /* [DUNGEON.DAT] encoded stat text */
    unsigned char  mirrorSkillsText[CHAMPION_MIRROR_FIELD_LENGTH]; /* [DUNGEON.DAT] encoded skill text */
    unsigned char  mirrorInventoryText[CHAMPION_MIRROR_INVENTORY_TEXT_LENGTH]; /* [DUNGEON.DAT] encoded inventory text */
    int            present;                 /* 1 if this slot is occupied */

    /* Attributes: base values [DUNGEON.DAT], current [RUNTIME] */
    unsigned short attributes[CHAMPION_ATTR_COUNT];

    /* Skill levels [DUNGEON.DAT initial, RUNTIME modified] */
    unsigned short skillLevels[CHAMPION_SKILL_COUNT];
    unsigned long  skillExperience[CHAMPION_SKILL_COUNT]; /* [RUNTIME] */

    /* HP / Stamina / Mana */
    struct ChampionStat_Compat hp;
    struct ChampionStat_Compat stamina;
    struct ChampionStat_Compat mana;

    /* Inventory: each slot holds a THING value (THING_NONE if empty) */
    unsigned short inventory[CHAMPION_SLOT_COUNT];

    /* Load / encumbrance [RUNTIME] */
    unsigned short load;
    unsigned short maxLoad;

    /* Direction the champion faces within the party cell [RUNTIME] */
    unsigned char  direction;

    /* Status flags [RUNTIME] */
    unsigned short wounds;      /* bitfield: which body parts wounded */
    unsigned short poisonDose;  /* accumulated poison */
    unsigned char  food;        /* food level 0-255 */
    unsigned char  water;       /* water level 0-255 */
};

/*
 * Party state — the core runtime state of the player's party.
 *
 * Pure data, serializable.  No pointers to UI/render/config.
 */
struct PartyState_Compat {
    /* Champions (up to 4, some may be .present=0) */
    struct ChampionState_Compat champions[CHAMPION_MAX_PARTY];
    int                         championCount; /* 0-4, how many present */

    /* Party position (matches dungeon coordinate system) */
    int mapIndex;       /* current map level */
    int mapX;           /* current X position (0..width-1) */
    int mapY;           /* current Y position (0..height-1) */
    int direction;      /* DIR_NORTH..DIR_WEST */

    /* Active leader index (0..3 or -1 if none) */
    int activeChampionIndex;

    /* Event/status flags [RUNTIME] */
    unsigned short eventFlags;
    unsigned short magicShieldTime;  /* countdown ticks remaining */
    unsigned short fireShieldTime;   /* countdown ticks remaining */
};

/*
 * Initialise a zero-state champion (all fields zeroed, present=0).
 */
void F0600_CHAMPION_InitEmpty_Compat(struct ChampionState_Compat* champ);

/*
 * Initialise party state from DUNGEON.DAT header.
 * Sets initial position (mapX, mapY, direction) from initialPartyLocation.
 * All champion slots are empty (must be recruited via mirrors).
 *
 * Returns 1 on success, 0 on failure.
 */
int F0601_CHAMPION_InitPartyFromDungeon_Compat(
    const struct DungeonDatState_Compat* dungeon,
    struct PartyState_Compat* party);

/*
 * Serialise champion state to a flat byte buffer.
 * Returns number of bytes written, or -1 if buffer too small.
 * Buffer must be at least CHAMPION_SERIALIZED_SIZE bytes.
 */
#define CHAMPION_SERIALIZED_SIZE 256

int F0602_CHAMPION_Serialize_Compat(
    const struct ChampionState_Compat* champ,
    unsigned char* buf,
    int bufSize);

/*
 * Deserialise champion state from a flat byte buffer.
 * Returns number of bytes consumed, or -1 on error.
 */
int F0603_CHAMPION_Deserialize_Compat(
    struct ChampionState_Compat* champ,
    const unsigned char* buf,
    int bufSize);

/*
 * Serialise entire party state.
 * Returns bytes written, or -1 if buffer too small.
 */
#define PARTY_SERIALIZED_SIZE (32 + CHAMPION_MAX_PARTY * CHAMPION_SERIALIZED_SIZE)

int F0604_PARTY_Serialize_Compat(
    const struct PartyState_Compat* party,
    unsigned char* buf,
    int bufSize);

/*
 * Deserialise entire party state.
 * Returns bytes consumed, or -1 on error.
 */
int F0605_PARTY_Deserialize_Compat(
    struct PartyState_Compat* party,
    const unsigned char* buf,
    int bufSize);

/*
 * Parse a DUNGEON.DAT champion mirror text string into identity fields.
 * Source text format is NAME|TITLE||SEX|... (decoded separator = '|').
 * Only the raw packed Name[8], Title[20], sex byte, and encoded mirror
 * stat/skill/inventory fields are written here; gameplay stats remain owned by the
 * lifecycle/recruitment path.
 */
int F0606_CHAMPION_ParseMirrorTextIdentity_Compat(
    const char* mirrorText,
    struct ChampionState_Compat* champ);

/* Decode a DUNGEON.DAT TextString thing by index and parse it as a champion
 * mirror record. Returns 1 only when the text string exists, decodes, and
 * matches the source champion mirror shape. */
int F0607_CHAMPION_ParseMirrorTextString_Compat(
    const struct DungeonThings_Compat* things,
    int textStringIndex,
    struct ChampionState_Compat* champ);

int F0608_CHAMPION_CountMirrorTextStrings_Compat(
    const struct DungeonThings_Compat* things);

int F0609_CHAMPION_FindMirrorTextStringByName_Compat(
    const struct DungeonThings_Compat* things,
    const unsigned char packedName[CHAMPION_NAME_LENGTH]);

int F0610_PARTY_AddChampionFromMirrorTextString_Compat(
    const struct DungeonThings_Compat* things,
    int textStringIndex,
    struct PartyState_Compat* party);

int F0611_PARTY_AddChampionFromMirrorName_Compat(
    const struct DungeonThings_Compat* things,
    const unsigned char packedName[CHAMPION_NAME_LENGTH],
    struct PartyState_Compat* party);

#endif
