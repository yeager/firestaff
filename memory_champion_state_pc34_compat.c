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

static int mirror_text_is_separator(char ch) {
    return ch == '|' || ch == '\n';
}

static const char* mirror_text_find_separator(const char* s) {
    if (!s) return 0;
    while (*s) {
        if (mirror_text_is_separator(*s)) return s;
        ++s;
    }
    return 0;
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

    memset(champ->name, ' ', CHAMPION_NAME_LENGTH);
    memset(champ->title, ' ', CHAMPION_TITLE_LENGTH);
    champ->sex = 0;
    memset(champ->mirrorStatsText, ' ', CHAMPION_MIRROR_FIELD_LENGTH);
    memset(champ->mirrorSkillsText, ' ', CHAMPION_MIRROR_FIELD_LENGTH);
    memset(champ->mirrorInventoryText, ' ', CHAMPION_MIRROR_INVENTORY_TEXT_LENGTH);

    nameStart = mirrorText;
    nameEnd = mirror_text_find_separator(nameStart);
    if (!nameEnd || nameEnd == nameStart) return 0;
    titleStart = nameEnd + 1;
    titleEnd = mirror_text_find_separator(titleStart);
    if (!titleEnd) return 0;

    pack_text_field(champ->name, CHAMPION_NAME_LENGTH,
                    nameStart, (int)(nameEnd - nameStart));
    pack_text_field(champ->title, CHAMPION_TITLE_LENGTH,
                    titleStart, (int)(titleEnd - titleStart));

    sexStart = titleEnd;
    if (mirror_text_is_separator(sexStart[0]) && mirror_text_is_separator(sexStart[1])) {
        sexStart += 2;
        if (sexStart[0] != 'M' && sexStart[0] != 'F') return 0;
        champ->sex = (unsigned char)sexStart[0];
        statStart = mirror_text_find_separator(sexStart);
        if (statStart) {
            ++statStart;
            statEnd = mirror_text_find_separator(statStart);
            if (statEnd) {
                pack_text_field(champ->mirrorStatsText, CHAMPION_MIRROR_FIELD_LENGTH,
                                statStart, (int)(statEnd - statStart));
                skillStart = statEnd + 1;
                skillEnd = mirror_text_find_separator(skillStart);
                if (!skillEnd) skillEnd = skillStart + strlen(skillStart);
                pack_text_field(champ->mirrorSkillsText, CHAMPION_MIRROR_FIELD_LENGTH,
                                skillStart, (int)(skillEnd - skillStart));
                inventoryStart = mirror_text_is_separator(*skillEnd) ? skillEnd + 1 : skillEnd;
                inventoryEnd = mirror_text_find_separator(inventoryStart);
                if (!inventoryEnd) inventoryEnd = inventoryStart + strlen(inventoryStart);
                pack_text_field(champ->mirrorInventoryText, CHAMPION_MIRROR_INVENTORY_TEXT_LENGTH,
                                inventoryStart, (int)(inventoryEnd - inventoryStart));
            }
        }
    } else {
        champ->sex = 0;
        return 0;
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


int F0607_CHAMPION_ParseMirrorTextString_Compat(
    const struct DungeonThings_Compat* things,
    int textStringIndex,
    struct ChampionState_Compat* champ)
{
    char decoded[DUNGEON_TEXT_MAX_STRING_LEN];
    const struct DungeonTextString_Compat* ts;
    int len;

    if (!things || !champ) return 0;
    if (textStringIndex < 0 || textStringIndex >= things->textStringCount) return 0;
    if (!things->textStrings || !things->textData || things->textDataWordCount <= 0) return 0;

    ts = &things->textStrings[textStringIndex];
    len = F0507_DUNGEON_DecodeTextAtOffset_Compat(
        things->textData,
        things->textDataWordCount,
        ts->textDataWordOffset,
        decoded,
        (int)sizeof(decoded));
    if (len <= 0) return 0;
    return F0606_CHAMPION_ParseMirrorTextIdentity_Compat(decoded, champ);
}


int F0608_CHAMPION_CountMirrorTextStrings_Compat(
    const struct DungeonThings_Compat* things)
{
    int i;
    int count = 0;
    struct ChampionState_Compat parsed;
    if (!things) return 0;
    for (i = 0; i < things->textStringCount; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&parsed);
        if (F0607_CHAMPION_ParseMirrorTextString_Compat(things, i, &parsed)) {
            ++count;
        }
    }
    return count;
}

int F0609_CHAMPION_FindMirrorTextStringByName_Compat(
    const struct DungeonThings_Compat* things,
    const unsigned char packedName[CHAMPION_NAME_LENGTH])
{
    int i;
    struct ChampionState_Compat parsed;
    if (!things || !packedName) return -1;
    for (i = 0; i < things->textStringCount; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&parsed);
        if (F0607_CHAMPION_ParseMirrorTextString_Compat(things, i, &parsed) &&
            memcmp(parsed.name, packedName, CHAMPION_NAME_LENGTH) == 0) {
            return i;
        }
    }
    return -1;
}

int F0626_PARTY_ContainsChampionName_Compat(const struct PartyState_Compat* party,
                                        const unsigned char packedName[CHAMPION_NAME_LENGTH]) {
    int i;
    if (!party || !packedName) return 0;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        if (party->champions[i].present &&
            memcmp(party->champions[i].name, packedName, CHAMPION_NAME_LENGTH) == 0) {
            return 1;
        }
    }
    return 0;
}

int F0610_PARTY_AddChampionFromMirrorTextString_Compat(
    const struct DungeonThings_Compat* things,
    int textStringIndex,
    struct PartyState_Compat* party)
{
    struct ChampionState_Compat parsed;
    int slot;
    if (!things || !party) return 0;
    if (party->championCount >= CHAMPION_MAX_PARTY) return 0;

    F0600_CHAMPION_InitEmpty_Compat(&parsed);
    if (!F0607_CHAMPION_ParseMirrorTextString_Compat(things, textStringIndex, &parsed)) return 0;
    if (F0626_PARTY_ContainsChampionName_Compat(party, parsed.name)) return 0;

    slot = F0624_PARTY_GetNextFreeChampionSlot_Compat(party);
    if (slot < 0 || slot >= CHAMPION_MAX_PARTY) return 0;
    F0600_CHAMPION_InitEmpty_Compat(&party->champions[slot]);
    memcpy(party->champions[slot].name, parsed.name, CHAMPION_NAME_LENGTH);
    memcpy(party->champions[slot].title, parsed.title, CHAMPION_TITLE_LENGTH);
    party->champions[slot].sex = parsed.sex;
    memcpy(party->champions[slot].mirrorStatsText, parsed.mirrorStatsText, CHAMPION_MIRROR_FIELD_LENGTH);
    memcpy(party->champions[slot].mirrorSkillsText, parsed.mirrorSkillsText, CHAMPION_MIRROR_FIELD_LENGTH);
    memcpy(party->champions[slot].mirrorInventoryText, parsed.mirrorInventoryText, CHAMPION_MIRROR_INVENTORY_TEXT_LENGTH);
    party->champions[slot].present = 1;
    party->champions[slot].portraitIndex = textStringIndex;
    party->champions[slot].direction = (unsigned char)party->direction;
    party->championCount = F0638_PARTY_CountOccupiedChampionSlots_Compat(party);
    if (party->activeChampionIndex < 0) {
        party->activeChampionIndex = slot;
    }
    return 1;
}

int F0611_PARTY_AddChampionFromMirrorName_Compat(
    const struct DungeonThings_Compat* things,
    const unsigned char packedName[CHAMPION_NAME_LENGTH],
    struct PartyState_Compat* party)
{
    int idx = F0609_CHAMPION_FindMirrorTextStringByName_Compat(things, packedName);
    if (idx < 0) return 0;
    return F0610_PARTY_AddChampionFromMirrorTextString_Compat(things, idx, party);
}


int F0612_CHAMPION_CollectMirrorTextStrings_Compat(
    const struct DungeonThings_Compat* things,
    struct ChampionState_Compat* outChampions,
    int* outTextStringIndices,
    int capacity)
{
    int i;
    int count = 0;
    struct ChampionState_Compat parsed;
    if (!things || capacity < 0) return 0;
    for (i = 0; i < things->textStringCount; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&parsed);
        if (F0607_CHAMPION_ParseMirrorTextString_Compat(things, i, &parsed)) {
            if (count < capacity) {
                if (outChampions) memcpy(&outChampions[count], &parsed, sizeof(parsed));
                if (outTextStringIndices) outTextStringIndices[count] = i;
            }
            ++count;
        }
    }
    return count;
}

int F0613_CHAMPION_GetMirrorTextStringIndexByOrdinal_Compat(
    const struct DungeonThings_Compat* things,
    int mirrorOrdinal)
{
    int i;
    int count = 0;
    struct ChampionState_Compat parsed;
    if (!things || mirrorOrdinal < 0) return -1;
    for (i = 0; i < things->textStringCount; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&parsed);
        if (F0607_CHAMPION_ParseMirrorTextString_Compat(things, i, &parsed)) {
            if (count == mirrorOrdinal) return i;
            ++count;
        }
    }
    return -1;
}

int F0614_PARTY_AddChampionFromMirrorOrdinal_Compat(
    const struct DungeonThings_Compat* things,
    int mirrorOrdinal,
    struct PartyState_Compat* party)
{
    int idx = F0613_CHAMPION_GetMirrorTextStringIndexByOrdinal_Compat(things, mirrorOrdinal);
    if (idx < 0) return 0;
    return F0610_PARTY_AddChampionFromMirrorTextString_Compat(things, idx, party);
}

int F0615_CHAMPION_FindMirrorTextStringByTitle_Compat(
    const struct DungeonThings_Compat* things,
    const unsigned char packedTitle[CHAMPION_TITLE_LENGTH])
{
    int i;
    struct ChampionState_Compat parsed;
    if (!things || !packedTitle) return -1;
    for (i = 0; i < things->textStringCount; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&parsed);
        if (F0607_CHAMPION_ParseMirrorTextString_Compat(things, i, &parsed) &&
            memcmp(parsed.title, packedTitle, CHAMPION_TITLE_LENGTH) == 0) {
            return i;
        }
    }
    return -1;
}

int F0616_CHAMPION_CountMirrorTextStringsBySex_Compat(
    const struct DungeonThings_Compat* things,
    unsigned char sex)
{
    int i;
    int count = 0;
    struct ChampionState_Compat parsed;
    if (!things || (sex != 'M' && sex != 'F')) return 0;
    for (i = 0; i < things->textStringCount; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&parsed);
        if (F0607_CHAMPION_ParseMirrorTextString_Compat(things, i, &parsed) && parsed.sex == sex) {
            ++count;
        }
    }
    return count;
}

int F0617_CHAMPION_HasMirrorTextStringByName_Compat(
    const struct DungeonThings_Compat* things,
    const unsigned char packedName[CHAMPION_NAME_LENGTH])
{
    return F0609_CHAMPION_FindMirrorTextStringByName_Compat(things, packedName) >= 0;
}


int F0618_CHAMPION_PackName_Compat(
    const char* name,
    unsigned char outName[CHAMPION_NAME_LENGTH])
{
    if (!name || !outName) return 0;
    pack_text_field(outName, CHAMPION_NAME_LENGTH, name, (int)strlen(name));
    return 1;
}

int F0619_CHAMPION_PackTitle_Compat(
    const char* title,
    unsigned char outTitle[CHAMPION_TITLE_LENGTH])
{
    if (!title || !outTitle) return 0;
    pack_text_field(outTitle, CHAMPION_TITLE_LENGTH, title, (int)strlen(title));
    return 1;
}

int F0620_CHAMPION_FindMirrorTextStringByNameString_Compat(
    const struct DungeonThings_Compat* things,
    const char* name)
{
    unsigned char packed[CHAMPION_NAME_LENGTH];
    if (!F0618_CHAMPION_PackName_Compat(name, packed)) return -1;
    return F0609_CHAMPION_FindMirrorTextStringByName_Compat(things, packed);
}

int F0621_PARTY_AddChampionFromMirrorNameString_Compat(
    const struct DungeonThings_Compat* things,
    const char* name,
    struct PartyState_Compat* party)
{
    unsigned char packed[CHAMPION_NAME_LENGTH];
    if (!F0618_CHAMPION_PackName_Compat(name, packed)) return 0;
    return F0611_PARTY_AddChampionFromMirrorName_Compat(things, packed, party);
}

int F0622_CHAMPION_GetMirrorOrdinalByName_Compat(
    const struct DungeonThings_Compat* things,
    const unsigned char packedName[CHAMPION_NAME_LENGTH])
{
    int i;
    int count = 0;
    struct ChampionState_Compat parsed;
    if (!things || !packedName) return -1;
    for (i = 0; i < things->textStringCount; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&parsed);
        if (F0607_CHAMPION_ParseMirrorTextString_Compat(things, i, &parsed)) {
            if (memcmp(parsed.name, packedName, CHAMPION_NAME_LENGTH) == 0) return count;
            ++count;
        }
    }
    return -1;
}

int F0623_CHAMPION_GetMirrorOrdinalByTitle_Compat(
    const struct DungeonThings_Compat* things,
    const unsigned char packedTitle[CHAMPION_TITLE_LENGTH])
{
    int i;
    int count = 0;
    struct ChampionState_Compat parsed;
    if (!things || !packedTitle) return -1;
    for (i = 0; i < things->textStringCount; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&parsed);
        if (F0607_CHAMPION_ParseMirrorTextString_Compat(things, i, &parsed)) {
            if (memcmp(parsed.title, packedTitle, CHAMPION_TITLE_LENGTH) == 0) return count;
            ++count;
        }
    }
    return -1;
}

int F0624_PARTY_GetNextFreeChampionSlot_Compat(
    const struct PartyState_Compat* party)
{
    int i;
    if (!party) return -1;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        if (!party->champions[i].present) return i;
    }
    return -1;
}

int F0625_PARTY_IsFull_Compat(const struct PartyState_Compat* party)
{
    return F0624_PARTY_GetNextFreeChampionSlot_Compat(party) < 0;
}


int F0627_CHAMPION_PackedTrimLength_Compat(
    const unsigned char* packed,
    int packedLen)
{
    int end;
    if (!packed || packedLen <= 0) return 0;
    end = packedLen;
    while (end > 0 && (packed[end - 1] == ' ' || packed[end - 1] == 0)) --end;
    return end;
}

static int unpack_packed_text(const unsigned char* packed, int packedLen,
                              char* out, int outSize) {
    int i;
    int len;
    if (!packed || !out || outSize <= 0) return 0;
    len = F0627_CHAMPION_PackedTrimLength_Compat(packed, packedLen);
    if (len >= outSize) len = outSize - 1;
    for (i = 0; i < len; ++i) {
        unsigned char ch = packed[i];
        out[i] = (ch >= 0x20 && ch <= 0x7e) ? (char)ch : ' ';
    }
    out[len] = '\0';
    return len;
}

int F0628_CHAMPION_UnpackName_Compat(
    const struct ChampionState_Compat* champ,
    char* outName,
    int outSize)
{
    if (!champ) return 0;
    return unpack_packed_text(champ->name, CHAMPION_NAME_LENGTH, outName, outSize);
}

int F0629_CHAMPION_UnpackTitle_Compat(
    const struct ChampionState_Compat* champ,
    char* outTitle,
    int outSize)
{
    if (!champ) return 0;
    return unpack_packed_text(champ->title, CHAMPION_TITLE_LENGTH, outTitle, outSize);
}

int F0630_CHAMPION_MirrorStatsTextLength_Compat(
    const struct ChampionState_Compat* champ)
{
    if (!champ) return 0;
    return F0627_CHAMPION_PackedTrimLength_Compat(champ->mirrorStatsText, CHAMPION_MIRROR_FIELD_LENGTH);
}

int F0631_CHAMPION_MirrorSkillsTextLength_Compat(
    const struct ChampionState_Compat* champ)
{
    if (!champ) return 0;
    return F0627_CHAMPION_PackedTrimLength_Compat(champ->mirrorSkillsText, CHAMPION_MIRROR_FIELD_LENGTH);
}

int F0632_CHAMPION_MirrorInventoryTextLength_Compat(
    const struct ChampionState_Compat* champ)
{
    if (!champ) return 0;
    return F0627_CHAMPION_PackedTrimLength_Compat(champ->mirrorInventoryText, CHAMPION_MIRROR_INVENTORY_TEXT_LENGTH);
}

int F0633_CHAMPION_IsEncodedMirrorField_Compat(
    const unsigned char* packed,
    int packedLen)
{
    int i;
    int len = F0627_CHAMPION_PackedTrimLength_Compat(packed, packedLen);
    if (!packed || len <= 0) return 0;
    for (i = 0; i < len; ++i) {
        if (packed[i] < 'A' || packed[i] > 'Z') return 0;
    }
    return 1;
}

int F0634_CHAMPION_HasValidEncodedMirrorFields_Compat(
    const struct ChampionState_Compat* champ)
{
    if (!champ) return 0;
    return F0633_CHAMPION_IsEncodedMirrorField_Compat(champ->mirrorStatsText, CHAMPION_MIRROR_FIELD_LENGTH) &&
           F0633_CHAMPION_IsEncodedMirrorField_Compat(champ->mirrorSkillsText, CHAMPION_MIRROR_FIELD_LENGTH) &&
           F0633_CHAMPION_IsEncodedMirrorField_Compat(champ->mirrorInventoryText, CHAMPION_MIRROR_INVENTORY_TEXT_LENGTH);
}

int F0635_CHAMPION_GetMirrorNameByOrdinal_Compat(
    const struct DungeonThings_Compat* things,
    int mirrorOrdinal,
    char* outName,
    int outSize)
{
    int idx;
    struct ChampionState_Compat parsed;
    if (!outName || outSize <= 0) return 0;
    outName[0] = '\0';
    idx = F0613_CHAMPION_GetMirrorTextStringIndexByOrdinal_Compat(things, mirrorOrdinal);
    if (idx < 0) return 0;
    F0600_CHAMPION_InitEmpty_Compat(&parsed);
    if (!F0607_CHAMPION_ParseMirrorTextString_Compat(things, idx, &parsed)) return 0;
    return F0628_CHAMPION_UnpackName_Compat(&parsed, outName, outSize);
}

int F0636_CHAMPION_GetMirrorTitleByOrdinal_Compat(
    const struct DungeonThings_Compat* things,
    int mirrorOrdinal,
    char* outTitle,
    int outSize)
{
    int idx;
    struct ChampionState_Compat parsed;
    if (!outTitle || outSize <= 0) return 0;
    outTitle[0] = '\0';
    idx = F0613_CHAMPION_GetMirrorTextStringIndexByOrdinal_Compat(things, mirrorOrdinal);
    if (idx < 0) return 0;
    F0600_CHAMPION_InitEmpty_Compat(&parsed);
    if (!F0607_CHAMPION_ParseMirrorTextString_Compat(things, idx, &parsed)) return 0;
    return F0629_CHAMPION_UnpackTitle_Compat(&parsed, outTitle, outSize);
}

int F0637_CHAMPION_FindMirrorTextStringByTitleString_Compat(
    const struct DungeonThings_Compat* things,
    const char* title)
{
    unsigned char packed[CHAMPION_TITLE_LENGTH];
    if (!F0619_CHAMPION_PackTitle_Compat(title, packed)) return -1;
    return F0615_CHAMPION_FindMirrorTextStringByTitle_Compat(things, packed);
}

int F0638_PARTY_CountOccupiedChampionSlots_Compat(
    const struct PartyState_Compat* party)
{
    int i;
    int count = 0;
    if (!party) return 0;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) if (party->champions[i].present) ++count;
    return count;
}

int F0639_PARTY_IsChampionSlotOccupied_Compat(
    const struct PartyState_Compat* party,
    int slot)
{
    if (!party || slot < 0 || slot >= CHAMPION_MAX_PARTY) return 0;
    return party->champions[slot].present ? 1 : 0;
}

int F0640_PARTY_RecountChampionSlots_Compat(
    const struct PartyState_Compat* party)
{
    return F0638_PARTY_CountOccupiedChampionSlots_Compat(party);
}

int F0641_PARTY_HasActiveChampion_Compat(
    const struct PartyState_Compat* party)
{
    if (!party) return 0;
    return party->activeChampionIndex >= 0 &&
           party->activeChampionIndex < CHAMPION_MAX_PARTY &&
           party->champions[party->activeChampionIndex].present;
}

int F0642_PARTY_SetActiveChampionIfPresent_Compat(
    struct PartyState_Compat* party,
    int slot)
{
    if (!party || slot < 0 || slot >= CHAMPION_MAX_PARTY) return 0;
    if (!party->champions[slot].present) return 0;
    party->activeChampionIndex = slot;
    return 1;
}

int F0643_PARTY_ClearChampionSlot_Compat(
    struct PartyState_Compat* party,
    int slot)
{
    if (!party || slot < 0 || slot >= CHAMPION_MAX_PARTY) return 0;
    if (!party->champions[slot].present) return 0;
    F0600_CHAMPION_InitEmpty_Compat(&party->champions[slot]);
    party->championCount = F0638_PARTY_CountOccupiedChampionSlots_Compat(party);
    if (party->activeChampionIndex == slot) party->activeChampionIndex = -1;
    return 1;
}

int F0644_PARTY_GetChampionSlotByName_Compat(
    const struct PartyState_Compat* party,
    const unsigned char packedName[CHAMPION_NAME_LENGTH])
{
    int i;
    if (!party || !packedName) return -1;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        if (party->champions[i].present &&
            memcmp(party->champions[i].name, packedName, CHAMPION_NAME_LENGTH) == 0) return i;
    }
    return -1;
}

int F0645_PARTY_GetChampionSlotByNameString_Compat(
    const struct PartyState_Compat* party,
    const char* name)
{
    unsigned char packed[CHAMPION_NAME_LENGTH];
    if (!F0618_CHAMPION_PackName_Compat(name, packed)) return -1;
    return F0644_PARTY_GetChampionSlotByName_Compat(party, packed);
}

int F0646_PARTY_AddChampionFromMirrorOrdinalIfAbsent_Compat(
    const struct DungeonThings_Compat* things,
    int mirrorOrdinal,
    struct PartyState_Compat* party)
{
    int idx;
    struct ChampionState_Compat parsed;
    if (!things || !party) return 0;
    idx = F0613_CHAMPION_GetMirrorTextStringIndexByOrdinal_Compat(things, mirrorOrdinal);
    if (idx < 0) return 0;
    F0600_CHAMPION_InitEmpty_Compat(&parsed);
    if (!F0607_CHAMPION_ParseMirrorTextString_Compat(things, idx, &parsed)) return 0;
    if (F0626_PARTY_ContainsChampionName_Compat(party, parsed.name)) return 1;
    return F0610_PARTY_AddChampionFromMirrorTextString_Compat(things, idx, party);
}

int F0647_PARTY_AddChampionFromMirrorNameStringIfAbsent_Compat(
    const struct DungeonThings_Compat* things,
    const char* name,
    struct PartyState_Compat* party)
{
    int ord = F0648_CHAMPION_GetMirrorOrdinalByNameString_Compat(things, name);
    if (ord < 0) return 0;
    return F0646_PARTY_AddChampionFromMirrorOrdinalIfAbsent_Compat(things, ord, party);
}

int F0648_CHAMPION_GetMirrorOrdinalByNameString_Compat(
    const struct DungeonThings_Compat* things,
    const char* name)
{
    unsigned char packed[CHAMPION_NAME_LENGTH];
    if (!F0618_CHAMPION_PackName_Compat(name, packed)) return -1;
    return F0622_CHAMPION_GetMirrorOrdinalByName_Compat(things, packed);
}

int F0649_CHAMPION_GetMirrorOrdinalByTitleString_Compat(
    const struct DungeonThings_Compat* things,
    const char* title)
{
    unsigned char packed[CHAMPION_TITLE_LENGTH];
    if (!F0619_CHAMPION_PackTitle_Compat(title, packed)) return -1;
    return F0623_CHAMPION_GetMirrorOrdinalByTitle_Compat(things, packed);
}

int F0650_CHAMPION_GetMirrorTextStringIndexByNameString_Compat(
    const struct DungeonThings_Compat* things,
    const char* name)
{
    return F0620_CHAMPION_FindMirrorTextStringByNameString_Compat(things, name);
}

int F0651_CHAMPION_GetMirrorTextStringIndexByTitleString_Compat(
    const struct DungeonThings_Compat* things,
    const char* title)
{
    return F0637_CHAMPION_FindMirrorTextStringByTitleString_Compat(things, title);
}


static int catalog_find_record_index_by_ordinal(const struct ChampionMirrorCatalog_Compat* catalog,
                                                int mirrorOrdinal) {
    int i;
    if (!catalog || mirrorOrdinal < 0) return -1;
    for (i = 0; i < catalog->count; ++i) {
        if (catalog->records[i].mirrorOrdinal == mirrorOrdinal) return i;
    }
    return -1;
}

static int catalog_recruit_record(const struct ChampionMirrorRecord_Compat* record,
                                  struct PartyState_Compat* party,
                                  int idempotent) {
    int slot;
    if (!record || !party || !F0669_CHAMPION_MirrorCatalogRecordValid_Compat(record)) return 0;
    if (F0626_PARTY_ContainsChampionName_Compat(party, record->champion.name)) return idempotent ? 1 : 0;
    if (F0625_PARTY_IsFull_Compat(party)) return 0;
    slot = F0624_PARTY_GetNextFreeChampionSlot_Compat(party);
    if (slot < 0 || slot >= CHAMPION_MAX_PARTY) return 0;
    F0600_CHAMPION_InitEmpty_Compat(&party->champions[slot]);
    memcpy(&party->champions[slot], &record->champion, sizeof(record->champion));
    party->champions[slot].present = 1;
    party->champions[slot].portraitIndex = record->textStringIndex;
    party->champions[slot].direction = (unsigned char)party->direction;
    party->championCount = F0638_PARTY_CountOccupiedChampionSlots_Compat(party);
    if (party->activeChampionIndex < 0) party->activeChampionIndex = slot;
    return 1;
}

int F0652_CHAMPION_BuildMirrorCatalog_Compat(
    const struct DungeonThings_Compat* things,
    struct ChampionMirrorCatalog_Compat* catalog)
{
    int i;
    int ordinal = 0;
    struct ChampionState_Compat parsed;
    if (!things || !catalog) return 0;
    memset(catalog, 0, sizeof(*catalog));
    for (i = 0; i < things->textStringCount; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&parsed);
        if (!F0607_CHAMPION_ParseMirrorTextString_Compat(things, i, &parsed)) continue;
        if (catalog->count < CHAMPION_MIRROR_CATALOG_MAX) {
            struct ChampionMirrorRecord_Compat* r = &catalog->records[catalog->count];
            r->textStringIndex = i;
            r->mirrorOrdinal = ordinal;
            memcpy(&r->champion, &parsed, sizeof(parsed));
            F0628_CHAMPION_UnpackName_Compat(&parsed, r->nameText, CHAMPION_NAME_TEXT_CAPACITY);
            F0629_CHAMPION_UnpackTitle_Compat(&parsed, r->titleText, CHAMPION_TITLE_TEXT_CAPACITY);
            catalog->count++;
        }
        ordinal++;
    }
    return catalog->count;
}

int F0653_CHAMPION_MirrorCatalogFindByOrdinal_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    int mirrorOrdinal)
{ return catalog_find_record_index_by_ordinal(catalog, mirrorOrdinal); }

int F0654_CHAMPION_MirrorCatalogFindByName_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    const char* name)
{
    int i;
    if (!catalog || !name) return -1;
    for (i = 0; i < catalog->count; ++i) if (strcmp(catalog->records[i].nameText, name) == 0) return i;
    return -1;
}

int F0655_CHAMPION_MirrorCatalogFindByTitle_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    const char* title)
{
    int i;
    if (!catalog || !title) return -1;
    for (i = 0; i < catalog->count; ++i) if (strcmp(catalog->records[i].titleText, title) == 0) return i;
    return -1;
}

int F0656_CHAMPION_MirrorCatalogCountBySex_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    unsigned char sex)
{
    int i, count = 0;
    if (!catalog || (sex != 'M' && sex != 'F')) return 0;
    for (i = 0; i < catalog->count; ++i) if (catalog->records[i].champion.sex == sex) ++count;
    return count;
}

int F0657_CHAMPION_MirrorCatalogCountTitled_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog)
{
    int i, count = 0;
    if (!catalog) return 0;
    for (i = 0; i < catalog->count; ++i) if (catalog->records[i].titleText[0]) ++count;
    return count;
}

int F0658_CHAMPION_MirrorCatalogCountUntitled_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog)
{ return catalog ? catalog->count - F0657_CHAMPION_MirrorCatalogCountTitled_Compat(catalog) : 0; }

int F0659_CHAMPION_MirrorCatalogNamesUnique_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog)
{
    int i, j;
    if (!catalog) return 0;
    for (i = 0; i < catalog->count; ++i) for (j = i + 1; j < catalog->count; ++j)
        if (memcmp(catalog->records[i].champion.name, catalog->records[j].champion.name, CHAMPION_NAME_LENGTH) == 0) return 0;
    return 1;
}

int F0660_CHAMPION_MirrorCatalogGetName_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    int mirrorOrdinal,
    char* outName,
    int outSize)
{
    int idx;
    if (!outName || outSize <= 0) return 0;
    outName[0] = '\0';
    idx = catalog_find_record_index_by_ordinal(catalog, mirrorOrdinal);
    if (idx < 0) return 0;
    snprintf(outName, (size_t)outSize, "%s", catalog->records[idx].nameText);
    return (int)strlen(outName);
}

int F0661_CHAMPION_MirrorCatalogGetTitle_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    int mirrorOrdinal,
    char* outTitle,
    int outSize)
{
    int idx;
    if (!outTitle || outSize <= 0) return 0;
    outTitle[0] = '\0';
    idx = catalog_find_record_index_by_ordinal(catalog, mirrorOrdinal);
    if (idx < 0) return 0;
    snprintf(outTitle, (size_t)outSize, "%s", catalog->records[idx].titleText);
    return (int)strlen(outTitle);
}

int F0662_CHAMPION_MirrorCatalogGetTextStringIndex_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    int mirrorOrdinal)
{
    int idx = catalog_find_record_index_by_ordinal(catalog, mirrorOrdinal);
    return idx >= 0 ? catalog->records[idx].textStringIndex : -1;
}

int F0663_CHAMPION_MirrorCatalogHasName_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    const char* name)
{ return F0654_CHAMPION_MirrorCatalogFindByName_Compat(catalog, name) >= 0; }

int F0664_CHAMPION_MirrorCatalogHasTitle_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    const char* title)
{ return F0655_CHAMPION_MirrorCatalogFindByTitle_Compat(catalog, title) >= 0; }

int F0665_CHAMPION_MirrorCatalogFirstOrdinalBySex_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    unsigned char sex)
{
    int i;
    if (!catalog) return -1;
    for (i = 0; i < catalog->count; ++i) if (catalog->records[i].champion.sex == sex) return catalog->records[i].mirrorOrdinal;
    return -1;
}

int F0666_CHAMPION_MirrorCatalogLastOrdinalBySex_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    unsigned char sex)
{
    int i;
    if (!catalog) return -1;
    for (i = catalog->count - 1; i >= 0; --i) if (catalog->records[i].champion.sex == sex) return catalog->records[i].mirrorOrdinal;
    return -1;
}

int F0667_CHAMPION_MirrorCatalogOrdinalAfter_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    int mirrorOrdinal)
{
    int idx = catalog_find_record_index_by_ordinal(catalog, mirrorOrdinal);
    if (!catalog || idx < 0 || idx + 1 >= catalog->count) return -1;
    return catalog->records[idx + 1].mirrorOrdinal;
}

int F0668_CHAMPION_MirrorCatalogOrdinalBefore_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    int mirrorOrdinal)
{
    int idx = catalog_find_record_index_by_ordinal(catalog, mirrorOrdinal);
    if (!catalog || idx <= 0) return -1;
    return catalog->records[idx - 1].mirrorOrdinal;
}

int F0669_CHAMPION_MirrorCatalogRecordValid_Compat(
    const struct ChampionMirrorRecord_Compat* record)
{
    if (!record) return 0;
    return record->mirrorOrdinal >= 0 && record->textStringIndex >= 0 && record->nameText[0] != '\0' &&
           (record->champion.sex == 'M' || record->champion.sex == 'F') &&
           F0634_CHAMPION_HasValidEncodedMirrorFields_Compat(&record->champion);
}

int F0670_CHAMPION_MirrorCatalogAllRecordsValid_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog)
{
    int i;
    if (!catalog || catalog->count <= 0) return 0;
    for (i = 0; i < catalog->count; ++i) if (!F0669_CHAMPION_MirrorCatalogRecordValid_Compat(&catalog->records[i])) return 0;
    return 1;
}

int F0671_CHAMPION_MirrorCatalogRecruitOrdinal_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    int mirrorOrdinal,
    struct PartyState_Compat* party)
{
    int idx = catalog_find_record_index_by_ordinal(catalog, mirrorOrdinal);
    return idx >= 0 ? catalog_recruit_record(&catalog->records[idx], party, 0) : 0;
}

int F0672_CHAMPION_MirrorCatalogRecruitName_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    const char* name,
    struct PartyState_Compat* party)
{
    int idx = F0654_CHAMPION_MirrorCatalogFindByName_Compat(catalog, name);
    return idx >= 0 ? catalog_recruit_record(&catalog->records[idx], party, 0) : 0;
}

int F0673_CHAMPION_MirrorCatalogRecruitOrdinalIfAbsent_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    int mirrorOrdinal,
    struct PartyState_Compat* party)
{
    int idx = catalog_find_record_index_by_ordinal(catalog, mirrorOrdinal);
    return idx >= 0 ? catalog_recruit_record(&catalog->records[idx], party, 1) : 0;
}

int F0674_CHAMPION_MirrorCatalogRecruitNameIfAbsent_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    const char* name,
    struct PartyState_Compat* party)
{
    int idx = F0654_CHAMPION_MirrorCatalogFindByName_Compat(catalog, name);
    return idx >= 0 ? catalog_recruit_record(&catalog->records[idx], party, 1) : 0;
}

int F0675_CHAMPION_MirrorCatalogCopyRecord_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    int mirrorOrdinal,
    struct ChampionState_Compat* outChampion)
{
    int idx = catalog_find_record_index_by_ordinal(catalog, mirrorOrdinal);
    if (idx < 0 || !outChampion) return 0;
    memcpy(outChampion, &catalog->records[idx].champion, sizeof(*outChampion));
    return 1;
}

int F0676_CHAMPION_MirrorCatalogGetOrdinalForTextStringIndex_Compat(
    const struct ChampionMirrorCatalog_Compat* catalog,
    int textStringIndex)
{
    int i;
    if (!catalog) return -1;
    for (i = 0; i < catalog->count; ++i) if (catalog->records[i].textStringIndex == textStringIndex) return catalog->records[i].mirrorOrdinal;
    return -1;
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
