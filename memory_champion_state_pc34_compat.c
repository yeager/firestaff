#include "memory_champion_state_pc34_compat.h"
#include <string.h>

/*
 * Champion + Party state implementation — pure data layer.
 * NO UI, NO render, NO strings, NO cheats.
 */

/* ---- Little-endian helpers ---- */
static void write_u16_le(unsigned char* buf, unsigned short v) {
    buf[0] = (unsigned char)(v & 0xFF);
    buf[1] = (unsigned char)((v >> 8) & 0xFF);
}

static unsigned short read_u16_le(const unsigned char* buf) {
    return (unsigned short)(buf[0] | (buf[1] << 8));
}

static void write_u32_le(unsigned char* buf, unsigned long v) {
    buf[0] = (unsigned char)(v & 0xFF);
    buf[1] = (unsigned char)((v >> 8) & 0xFF);
    buf[2] = (unsigned char)((v >> 16) & 0xFF);
    buf[3] = (unsigned char)((v >> 24) & 0xFF);
}

static unsigned long read_u32_le(const unsigned char* buf) {
    return (unsigned long)(buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
}

void F0600_CHAMPION_InitEmpty_Compat(struct ChampionState_Compat* champ) {
    memset(champ, 0, sizeof(*champ));
    champ->present = 0;
    champ->portraitIndex = -1;
    champ->direction = DIR_NORTH;
    /* Fill inventory with THING_NONE */
    {
        int i;
        for (i = 0; i < CHAMPION_SLOT_COUNT; i++) {
            champ->inventory[i] = THING_NONE;
        }
    }
}

static void pack_text_field(unsigned char* dst, int dstLen,
                            const char* src, int srcLen) {
    int i;
    for (i = 0; i < dstLen; ++i) {
        dst[i] = ' ';
    }
    for (i = 0; i < dstLen && i < srcLen; ++i) {
        unsigned char ch = (unsigned char)src[i];
        dst[i] = (ch >= 0x20 && ch <= 0x7e) ? ch : ' ';
    }
}

int F0606_CHAMPION_ParseMirrorTextIdentity_Compat(
    const char* mirrorText,
    struct ChampionState_Compat* champ)
{
    const char* nameStart;
    const char* nameEnd;
    const char* titleStart;
    const char* titleEnd;
    const char* sexStart;
    const char* statStart;
    const char* statEnd;
    const char* skillStart;
    const char* skillEnd;
    const char* inventoryStart;
    const char* inventoryEnd;

    if (!mirrorText || !champ) return 0;

    nameStart = mirrorText;
    nameEnd = strchr(nameStart, '|');
    if (!nameEnd || nameEnd == nameStart) return 0;
    titleStart = nameEnd + 1;
    titleEnd = strchr(titleStart, '|');
    if (!titleEnd) return 0;

    pack_text_field(champ->name, CHAMPION_NAME_LENGTH,
                    nameStart, (int)(nameEnd - nameStart));
    pack_text_field(champ->title, CHAMPION_TITLE_LENGTH,
                    titleStart, (int)(titleEnd - titleStart));

    sexStart = titleEnd;
    if (sexStart[0] == '|' && sexStart[1] == '|') {
        sexStart += 2;
        champ->sex = (unsigned char)sexStart[0];
        statStart = strchr(sexStart, '|');
        if (statStart) {
            ++statStart;
            statEnd = strchr(statStart, '|');
            if (statEnd) {
                pack_text_field(champ->mirrorStatsText, CHAMPION_MIRROR_FIELD_LENGTH,
                                statStart, (int)(statEnd - statStart));
                skillStart = statEnd + 1;
                skillEnd = strchr(skillStart, '|');
                if (!skillEnd) skillEnd = skillStart + strlen(skillStart);
                pack_text_field(champ->mirrorSkillsText, CHAMPION_MIRROR_FIELD_LENGTH,
                                skillStart, (int)(skillEnd - skillStart));
                inventoryStart = (*skillEnd == '|') ? skillEnd + 1 : skillEnd;
                inventoryEnd = strchr(inventoryStart, '|');
                if (!inventoryEnd) inventoryEnd = inventoryStart + strlen(inventoryStart);
                pack_text_field(champ->mirrorInventoryText, CHAMPION_MIRROR_INVENTORY_TEXT_LENGTH,
                                inventoryStart, (int)(inventoryEnd - inventoryStart));
            }
        }
    } else {
        champ->sex = 0;
    }
    return 1;
}

int F0601_CHAMPION_InitPartyFromDungeon_Compat(
    const struct DungeonDatState_Compat* dungeon,
    struct PartyState_Compat* party)
{
    int dir, py, px;
    int i;

    if (!dungeon || !dungeon->loaded || !party) return 0;

    memset(party, 0, sizeof(*party));

    /* Decode initial party location from header */
    F0501_DUNGEON_DecodePartyLocation_Compat(
        dungeon->header.initialPartyLocation, &dir, &py, &px);

    party->mapIndex = 0;  /* Party starts on map 0 */
    party->mapX = px;
    party->mapY = py;
    party->direction = dir;
    party->activeChampionIndex = -1;  /* No champion active yet */
    party->championCount = 0;

    /* Initialise all slots as empty */
    for (i = 0; i < CHAMPION_MAX_PARTY; i++) {
        F0600_CHAMPION_InitEmpty_Compat(&party->champions[i]);
    }

    return 1;
}

/*
 * Serialisation layout (little-endian):
 *
 * Champion (CHAMPION_SERIALIZED_SIZE = 256 bytes):
 *   [0]     present (u8)
 *   [1]     portraitIndex (s8)
 *   [2..9]  name[8]
 *   [10]    direction (u8)
 *   [11]    food (u8)
 *   [12]    water (u8)
 *   [13]    pad
 *   [14..25]  attributes[6] (u16 LE each)
 *   [26..33]  skillLevels[4] (u16 LE each)
 *   [34..49]  skillExperience[4] (u32 LE each)
 *   [50..55]  hp (current/max/shifted, u16 LE each)
 *   [56..61]  stamina
 *   [62..67]  mana
 *   [68..127] inventory[30] (u16 LE each)
 *   [128..129] load (u16 LE)
 *   [130..131] maxLoad (u16 LE)
 *   [132..133] wounds (u16 LE)
 *   [134..135] poisonDose (u16 LE)
 *   [136..155] title[20]
 *   [156] sex ('M'/'F'/0)
 *   [157..172] mirrorStatsText[16]
 *   [173..188] mirrorSkillsText[16]
 *   [189..220] mirrorInventoryText[32]
 *   [221..255] reserved (zero)
 */

int F0602_CHAMPION_Serialize_Compat(
    const struct ChampionState_Compat* champ,
    unsigned char* buf,
    int bufSize)
{
    int i, off;
    if (bufSize < CHAMPION_SERIALIZED_SIZE) return -1;
    memset(buf, 0, CHAMPION_SERIALIZED_SIZE);

    buf[0] = (unsigned char)champ->present;
    buf[1] = (unsigned char)(champ->portraitIndex & 0xFF);
    memcpy(&buf[2], champ->name, CHAMPION_NAME_LENGTH);
    buf[10] = champ->direction;
    buf[11] = champ->food;
    buf[12] = champ->water;

    off = 14;
    for (i = 0; i < CHAMPION_ATTR_COUNT; i++) {
        write_u16_le(&buf[off], champ->attributes[i]);
        off += 2;
    }
    for (i = 0; i < CHAMPION_SKILL_COUNT; i++) {
        write_u16_le(&buf[off], champ->skillLevels[i]);
        off += 2;
    }
    for (i = 0; i < CHAMPION_SKILL_COUNT; i++) {
        write_u32_le(&buf[off], champ->skillExperience[i]);
        off += 4;
    }

    /* hp */
    write_u16_le(&buf[50], champ->hp.current);
    write_u16_le(&buf[52], champ->hp.maximum);
    write_u16_le(&buf[54], champ->hp.shifted);
    /* stamina */
    write_u16_le(&buf[56], champ->stamina.current);
    write_u16_le(&buf[58], champ->stamina.maximum);
    write_u16_le(&buf[60], champ->stamina.shifted);
    /* mana */
    write_u16_le(&buf[62], champ->mana.current);
    write_u16_le(&buf[64], champ->mana.maximum);
    write_u16_le(&buf[66], champ->mana.shifted);

    off = 68;
    for (i = 0; i < CHAMPION_SLOT_COUNT; i++) {
        write_u16_le(&buf[off], champ->inventory[i]);
        off += 2;
    }

    write_u16_le(&buf[128], champ->load);
    write_u16_le(&buf[130], champ->maxLoad);
    write_u16_le(&buf[132], champ->wounds);
    write_u16_le(&buf[134], champ->poisonDose);
    memcpy(&buf[136], champ->title, CHAMPION_TITLE_LENGTH);
    buf[156] = champ->sex;
    memcpy(&buf[157], champ->mirrorStatsText, CHAMPION_MIRROR_FIELD_LENGTH);
    memcpy(&buf[173], champ->mirrorSkillsText, CHAMPION_MIRROR_FIELD_LENGTH);
    memcpy(&buf[189], champ->mirrorInventoryText, CHAMPION_MIRROR_INVENTORY_TEXT_LENGTH);

    return CHAMPION_SERIALIZED_SIZE;
}

int F0603_CHAMPION_Deserialize_Compat(
    struct ChampionState_Compat* champ,
    const unsigned char* buf,
    int bufSize)
{
    int i, off;
    if (bufSize < CHAMPION_SERIALIZED_SIZE) return -1;

    memset(champ, 0, sizeof(*champ));
    champ->present = buf[0];
    champ->portraitIndex = (signed char)buf[1];
    memcpy(champ->name, &buf[2], CHAMPION_NAME_LENGTH);
    champ->direction = buf[10];
    champ->food = buf[11];
    champ->water = buf[12];

    off = 14;
    for (i = 0; i < CHAMPION_ATTR_COUNT; i++) {
        champ->attributes[i] = read_u16_le(&buf[off]);
        off += 2;
    }
    for (i = 0; i < CHAMPION_SKILL_COUNT; i++) {
        champ->skillLevels[i] = read_u16_le(&buf[off]);
        off += 2;
    }
    for (i = 0; i < CHAMPION_SKILL_COUNT; i++) {
        champ->skillExperience[i] = read_u32_le(&buf[off]);
        off += 4;
    }

    champ->hp.current    = read_u16_le(&buf[50]);
    champ->hp.maximum    = read_u16_le(&buf[52]);
    champ->hp.shifted    = read_u16_le(&buf[54]);
    champ->stamina.current = read_u16_le(&buf[56]);
    champ->stamina.maximum = read_u16_le(&buf[58]);
    champ->stamina.shifted = read_u16_le(&buf[60]);
    champ->mana.current  = read_u16_le(&buf[62]);
    champ->mana.maximum  = read_u16_le(&buf[64]);
    champ->mana.shifted  = read_u16_le(&buf[66]);

    off = 68;
    for (i = 0; i < CHAMPION_SLOT_COUNT; i++) {
        champ->inventory[i] = read_u16_le(&buf[off]);
        off += 2;
    }

    champ->load      = read_u16_le(&buf[128]);
    champ->maxLoad   = read_u16_le(&buf[130]);
    champ->wounds    = read_u16_le(&buf[132]);
    champ->poisonDose = read_u16_le(&buf[134]);
    memcpy(champ->title, &buf[136], CHAMPION_TITLE_LENGTH);
    champ->sex = buf[156];
    memcpy(champ->mirrorStatsText, &buf[157], CHAMPION_MIRROR_FIELD_LENGTH);
    memcpy(champ->mirrorSkillsText, &buf[173], CHAMPION_MIRROR_FIELD_LENGTH);
    memcpy(champ->mirrorInventoryText, &buf[189], CHAMPION_MIRROR_INVENTORY_TEXT_LENGTH);

    return CHAMPION_SERIALIZED_SIZE;
}

int F0604_PARTY_Serialize_Compat(
    const struct PartyState_Compat* party,
    unsigned char* buf,
    int bufSize)
{
    int i, off;
    if (bufSize < PARTY_SERIALIZED_SIZE) return -1;
    memset(buf, 0, PARTY_SERIALIZED_SIZE);

    /* Party header: 32 bytes */
    write_u16_le(&buf[0], (unsigned short)party->championCount);
    write_u16_le(&buf[2], (unsigned short)party->mapIndex);
    write_u16_le(&buf[4], (unsigned short)party->mapX);
    write_u16_le(&buf[6], (unsigned short)party->mapY);
    write_u16_le(&buf[8], (unsigned short)party->direction);
    write_u16_le(&buf[10], (unsigned short)(party->activeChampionIndex & 0xFFFF));
    write_u16_le(&buf[12], party->eventFlags);
    write_u16_le(&buf[14], party->magicShieldTime);
    write_u16_le(&buf[16], party->fireShieldTime);
    /* bytes 18..31 reserved */

    off = 32;
    for (i = 0; i < CHAMPION_MAX_PARTY; i++) {
        int rc = F0602_CHAMPION_Serialize_Compat(&party->champions[i], &buf[off], CHAMPION_SERIALIZED_SIZE);
        if (rc < 0) return -1;
        off += CHAMPION_SERIALIZED_SIZE;
    }

    return off;
}

int F0605_PARTY_Deserialize_Compat(
    struct PartyState_Compat* party,
    const unsigned char* buf,
    int bufSize)
{
    int i, off;
    if (bufSize < PARTY_SERIALIZED_SIZE) return -1;

    memset(party, 0, sizeof(*party));
    party->championCount       = (int)read_u16_le(&buf[0]);
    party->mapIndex            = (int)read_u16_le(&buf[2]);
    party->mapX                = (int)read_u16_le(&buf[4]);
    party->mapY                = (int)read_u16_le(&buf[6]);
    party->direction           = (int)read_u16_le(&buf[8]);
    party->activeChampionIndex = (short)read_u16_le(&buf[10]);
    party->eventFlags          = read_u16_le(&buf[12]);
    party->magicShieldTime     = read_u16_le(&buf[14]);
    party->fireShieldTime      = read_u16_le(&buf[16]);

    off = 32;
    for (i = 0; i < CHAMPION_MAX_PARTY; i++) {
        int rc = F0603_CHAMPION_Deserialize_Compat(&party->champions[i], &buf[off], CHAMPION_SERIALIZED_SIZE);
        if (rc < 0) return -1;
        off += CHAMPION_SERIALIZED_SIZE;
    }

    return off;
}
