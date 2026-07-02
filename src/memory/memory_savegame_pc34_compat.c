/*
 * Save-game composite data layer — Phase 15 of M10.
 * See PHASE15_PLAN.md for the authoritative spec.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_savegame_pc34_compat.h"

/* -------- Platform assumptions (plan §3, R4) -------- */

_Static_assert(sizeof(int) == 4, "int32 assumption required");
_Static_assert(sizeof(uint32_t) == 4, "uint32 assumption required");

/* Re-assert Phase 10..14 size constants drive our budget. Any drift
 * in upstream serialised sizes will break the build here, not at
 * runtime. Plan §8 Risk R3 mitigation. */
_Static_assert(PARTY_SERIALIZED_V1_SIZE == 1056,
               "PARTY_SERIALIZED_V1_SIZE drifted");
_Static_assert(PARTY_SERIALIZED_SIZE == 2928,
               "PARTY_SERIALIZED_SIZE drifted");
_Static_assert(SENSOR_EFFECT_LIST_SERIALIZED_SIZE == 228,
               "SENSOR_EFFECT_LIST_SERIALIZED_SIZE drifted");
_Static_assert(TIMELINE_QUEUE_SERIALIZED_SIZE == 11272,
               "TIMELINE_QUEUE_SERIALIZED_SIZE drifted");
_Static_assert(COMBAT_RESULT_SERIALIZED_SIZE == 56,
               "COMBAT_RESULT_SERIALIZED_SIZE drifted");
_Static_assert(WEAPON_PROFILE_SERIALIZED_SIZE == 32,
               "WEAPON_PROFILE_SERIALIZED_SIZE drifted");
_Static_assert(MAGIC_STATE_SERIALIZED_SIZE == 84,
               "MAGIC_STATE_SERIALIZED_SIZE drifted");
_Static_assert(DUNGEON_MUTATION_LIST_SERIALIZED_SIZE == 49156,
               "DUNGEON_MUTATION_LIST_SERIALIZED_SIZE arithmetic broken");

/* -------- LE primitive readers/writers (local duplicates so
 * Phase 15 has no dependency on another subsystem's internal
 * helpers). -------- */

static void write_u32_le(unsigned char* p, uint32_t v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    p[2] = (unsigned char)((v >> 16) & 0xFFu);
    p[3] = (unsigned char)((v >> 24) & 0xFFu);
}

static uint32_t read_u32_le(const unsigned char* p) {
    return ((uint32_t)p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void write_i32_le(unsigned char* p, int v) {
    write_u32_le(p, (uint32_t)v);
}

static int read_i32_le(const unsigned char* p) {
    return (int)read_u32_le(p);
}

/* BUG-112 (v2.7.13): per-mutation-kind field-mask valid-bit range.
 * Returns the bitmask of bits that are meaningful for the given
 * kind. Used by both F0782 (serialize) and F0782b (deserialize) to
 * clamp stray reserved bits and to reject malformed entries. */
static unsigned int dungeon_mutation_field_mask_valid(int kind) {
    switch (kind) {
        case DUNGEON_MUTATION_KIND_INVALID:      return 0u;
        case DUNGEON_MUTATION_KIND_SQUARE_BYTE:  return DUNGEON_MUTATION_FIELD_SQUARE_VALID_MASK;
        case DUNGEON_MUTATION_KIND_DOOR_STATE:   return DUNGEON_MUTATION_FIELD_DOOR_VALID_MASK;
        case DUNGEON_MUTATION_KIND_SENSOR_TOG:   return DUNGEON_MUTATION_FIELD_SENSOR_VALID_MASK;
        case DUNGEON_MUTATION_KIND_GROUP_HP:     return DUNGEON_MUTATION_FIELD_GROUP_HP_VALID_MASK;
        case DUNGEON_MUTATION_KIND_THING_LINK:   return DUNGEON_MUTATION_FIELD_THING_VALID_MASK;
        case DUNGEON_MUTATION_KIND_FIELD_GENERIC: return DUNGEON_MUTATION_FIELD_GENERIC_VALID_MASK;
        default:                                 return 0u; /* unknown kind: mask everything off */
    }
}

void F0790_SAVEGAME_SetAudioMetadata_Compat(
    struct SaveGameHeader_Compat* hdr,
    uint32_t gameID,
    int musicOn)
{
    if (hdr == 0) return;
    write_u32_le(hdr->reserved + SAVEGAME_HEADER_RESERVED_GAME_ID_OFFSET,
                 gameID);
    hdr->reserved[SAVEGAME_HEADER_RESERVED_MUSIC_ON_OFFSET] =
        (unsigned char)(musicOn ? 1 : 0);
}

uint32_t F0791_SAVEGAME_GetGameID_Compat(
    const struct SaveGameHeader_Compat* hdr)
{
    if (hdr == 0) return 0u;
    return read_u32_le(hdr->reserved + SAVEGAME_HEADER_RESERVED_GAME_ID_OFFSET);
}

int F0792_SAVEGAME_GetMusicOn_Compat(
    const struct SaveGameHeader_Compat* hdr)
{
    if (hdr == 0) return 0;
    return hdr->reserved[SAVEGAME_HEADER_RESERVED_MUSIC_ON_OFFSET] ? 1 : 0;
}

/* ==========================================================
 *  Group A — Checksum / header helpers (F0770–F0772)
 * ========================================================== */

/*
 * CRC32 / IEEE 802.3 reflected polynomial 0xEDB88320.
 * Init 0xFFFFFFFF, final XOR 0xFFFFFFFF. Bit-by-bit (plan §4.3).
 * Empty input returns 0 (0xFFFFFFFF ^ 0xFFFFFFFF).
 * Defined behaviour: NULL buffer with len==0 returns 0 (no deref).
 */
uint32_t F0770_SAVEGAME_CRC32_Compat(const unsigned char* buf, size_t len) {
    uint32_t c = 0xFFFFFFFFu;
    size_t i;
    int k;
    if (buf == 0) {
        return len == 0 ? 0u : 0u; /* NULL+nonzero: treat as empty (defensive). */
    }
    for (i = 0; i < len; i++) {
        c ^= (uint32_t)buf[i];
        for (k = 0; k < 8; k++) {
            uint32_t mask = (uint32_t)(-(int32_t)(c & 1u));
            c = (c >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return c ^ 0xFFFFFFFFu;
}

int F0771_SAVEGAME_WriteHeader_Compat(
    struct SaveGameHeader_Compat* hdr,
    uint32_t totalSize, uint32_t bodyCRC)
{
    if (hdr == 0) return SAVEGAME_ERROR_NULL_ARG;
    memset(hdr, 0, sizeof(*hdr));
    hdr->magic[0] = 'R'; hdr->magic[1] = 'D'; hdr->magic[2] = 'M';
    hdr->magic[3] = 'C'; hdr->magic[4] = 'S'; hdr->magic[5] = 'B';
    hdr->magic[6] = '1'; hdr->magic[7] = '5';
    hdr->formatVersion  = SAVEGAME_FORMAT_VERSION;
    hdr->endianSentinel = SAVEGAME_ENDIAN_SENTINEL;
    hdr->totalFileSize  = totalSize;
    hdr->sectionCount   = SAVEGAME_SECTION_COUNT;
    hdr->bodyCRC32      = bodyCRC;
    /* reserved stays zero-filled by memset above; callers that need
     * ReDMCSB GameID/MusicOn metadata stamp it with F0790. */
    return SAVEGAME_OK;
}

int F0772_SAVEGAME_ValidateHeader_Compat(
    const struct SaveGameHeader_Compat* hdr,
    uint32_t actualBufferSize)
{
    static const unsigned char expected[8] = {
        'R','D','M','C','S','B','1','5'
    };
    if (hdr == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (memcmp(hdr->magic, expected, 8) != 0) {
        return SAVEGAME_ERROR_BAD_MAGIC;
    }
    if (hdr->formatVersion != SAVEGAME_FORMAT_VERSION) {
        return SAVEGAME_ERROR_BAD_VERSION;
    }
    if (hdr->endianSentinel != SAVEGAME_ENDIAN_SENTINEL) {
        return SAVEGAME_ERROR_BAD_ENDIAN;
    }
    if (hdr->sectionCount != SAVEGAME_SECTION_COUNT) {
        return SAVEGAME_ERROR_BAD_SECTION_COUNT;
    }
    if (hdr->totalFileSize != actualBufferSize) {
        return SAVEGAME_ERROR_BAD_SIZE;
    }
    return SAVEGAME_OK;
}

/* Raw header (de)serialisers — buffer <-> struct. Kept file-local. */

static void savegame_header_write(
    unsigned char* buf, const struct SaveGameHeader_Compat* hdr)
{
    memcpy(buf + 0, hdr->magic, 8);
    write_u32_le(buf + 8,  hdr->formatVersion);
    write_u32_le(buf + 12, hdr->endianSentinel);
    write_u32_le(buf + 16, hdr->totalFileSize);
    write_u32_le(buf + 20, hdr->sectionCount);
    write_u32_le(buf + 24, hdr->bodyCRC32);
    memcpy(buf + 28, hdr->reserved, 36);
}

static void savegame_header_read(
    const unsigned char* buf, struct SaveGameHeader_Compat* hdr)
{
    memcpy(hdr->magic, buf + 0, 8);
    hdr->formatVersion  = read_u32_le(buf + 8);
    hdr->endianSentinel = read_u32_le(buf + 12);
    hdr->totalFileSize  = read_u32_le(buf + 16);
    hdr->sectionCount   = read_u32_le(buf + 20);
    hdr->bodyCRC32      = read_u32_le(buf + 24);
    memcpy(hdr->reserved, buf + 28, 36);
}

/* ==========================================================
 *  Group C (MovementResult local pair — F0776a / F0776b)
 * ========================================================== */

int F0776a_SAVEGAME_MovementResultSerialize_Compat(
    const struct MovementResult_Compat* mv,
    unsigned char* buf, int bufSize)
{
    if (mv == 0 || buf == 0) return 0;
    if (bufSize < MOVEMENT_RESULT_SERIALIZED_SIZE) return 0;
    write_i32_le(buf +  0, mv->resultCode);
    write_i32_le(buf +  4, mv->newMapX);
    write_i32_le(buf +  8, mv->newMapY);
    write_i32_le(buf + 12, mv->newDirection);
    write_i32_le(buf + 16, mv->newMapIndex);
    return 1;
}

int F0776b_SAVEGAME_MovementResultDeserialize_Compat(
    struct MovementResult_Compat* mv,
    const unsigned char* buf, int bufSize)
{
    if (mv == 0 || buf == 0) return 0;
    if (bufSize < MOVEMENT_RESULT_SERIALIZED_SIZE) return 0;
    mv->resultCode   = read_i32_le(buf +  0);
    mv->newMapX      = read_i32_le(buf +  4);
    mv->newMapY      = read_i32_le(buf +  8);
    mv->newDirection = read_i32_le(buf + 12);
    mv->newMapIndex  = read_i32_le(buf + 16);
    return 1;
}

/* ==========================================================
 *  Group C — per-section wrappers (F0776–F0782 + mirrors)
 * ========================================================== */

int F0776_SAVEGAME_SerializeParty_Compat(
    const struct PartyState_Compat* party,
    unsigned char* buf, int bufSize, int* outBytes)
{
    int n;
    if (party == 0 || buf == 0 || outBytes == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < PARTY_SERIALIZED_SIZE) return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    n = F0604_PARTY_Serialize_Compat(party, buf, bufSize);
    if (n != PARTY_SERIALIZED_SIZE) return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    *outBytes = n;
    return SAVEGAME_OK;
}

int F0776_SAVEGAME_DeserializeParty_Compat(
    struct PartyState_Compat* party,
    const unsigned char* buf, int bufSize)
{
    int n;
    if (party == 0 || buf == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < PARTY_SERIALIZED_SIZE) return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    n = F0605_PARTY_Deserialize_Compat(party, buf, bufSize);
    if (n != PARTY_SERIALIZED_SIZE) return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    return SAVEGAME_OK;
}

int F0777_SAVEGAME_SerializeMovement_Compat(
    const struct MovementResult_Compat* mv,
    unsigned char* buf, int bufSize, int* outBytes)
{
    if (mv == 0 || buf == 0 || outBytes == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < MOVEMENT_RESULT_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    if (!F0776a_SAVEGAME_MovementResultSerialize_Compat(mv, buf, bufSize))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    *outBytes = MOVEMENT_RESULT_SERIALIZED_SIZE;
    return SAVEGAME_OK;
}

int F0777_SAVEGAME_DeserializeMovement_Compat(
    struct MovementResult_Compat* mv,
    const unsigned char* buf, int bufSize)
{
    if (mv == 0 || buf == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < MOVEMENT_RESULT_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    if (!F0776b_SAVEGAME_MovementResultDeserialize_Compat(mv, buf, bufSize))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    return SAVEGAME_OK;
}

int F0778_SAVEGAME_SerializeSensor_Compat(
    const struct SensorEffectList_Compat* s,
    unsigned char* buf, int bufSize, int* outBytes)
{
    if (s == 0 || buf == 0 || outBytes == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < SENSOR_EFFECT_LIST_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    if (!F0713_SENSOR_ListSerialize_Compat(s, buf, bufSize))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    *outBytes = SENSOR_EFFECT_LIST_SERIALIZED_SIZE;
    return SAVEGAME_OK;
}

int F0778_SAVEGAME_DeserializeSensor_Compat(
    struct SensorEffectList_Compat* s,
    const unsigned char* buf, int bufSize)
{
    if (s == 0 || buf == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < SENSOR_EFFECT_LIST_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    if (!F0714_SENSOR_ListDeserialize_Compat(s, buf, bufSize))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    return SAVEGAME_OK;
}

int F0779_SAVEGAME_SerializeTimeline_Compat(
    const struct TimelineQueue_Compat* q,
    unsigned char* buf, int bufSize, int* outBytes)
{
    if (q == 0 || buf == 0 || outBytes == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < TIMELINE_QUEUE_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    if (!F0727_TIMELINE_QueueSerialize_Compat(q, buf, bufSize))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    *outBytes = TIMELINE_QUEUE_SERIALIZED_SIZE;
    return SAVEGAME_OK;
}

int F0779_SAVEGAME_DeserializeTimeline_Compat(
    struct TimelineQueue_Compat* q,
    const unsigned char* buf, int bufSize)
{
    if (q == 0 || buf == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < TIMELINE_QUEUE_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    if (!F0728_TIMELINE_QueueDeserialize_Compat(q, buf, bufSize))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    return SAVEGAME_OK;
}

/* CombatScratch composition: result (56) + weapon (32) + rng (4) +
 * combatActive (4) + reserved (4) = 100 bytes. */
int F0780_SAVEGAME_SerializeCombat_Compat(
    const struct CombatScratch_Compat* c,
    unsigned char* buf, int bufSize, int* outBytes)
{
    if (c == 0 || buf == 0 || outBytes == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < COMBAT_SCRATCH_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    memset(buf, 0, COMBAT_SCRATCH_SERIALIZED_SIZE);
    if (!F0742_COMBAT_ResultSerialize_Compat(
            &c->lastResult, buf + 0, COMBAT_RESULT_SERIALIZED_SIZE))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    if (!F0747a_COMBAT_WeaponProfileSerialize_Compat(
            &c->lastWeapon,
            buf + COMBAT_RESULT_SERIALIZED_SIZE,
            WEAPON_PROFILE_SERIALIZED_SIZE))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    write_u32_le(buf + 88, c->rng.seed);
    write_i32_le(buf + 92, c->combatActive);
    write_i32_le(buf + 96, c->reserved);
    *outBytes = COMBAT_SCRATCH_SERIALIZED_SIZE;
    return SAVEGAME_OK;
}

int F0780b_SAVEGAME_DeserializeCombat_Compat(
    struct CombatScratch_Compat* c,
    const unsigned char* buf, int bufSize)
{
    if (c == 0 || buf == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < COMBAT_SCRATCH_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    memset(c, 0, sizeof(*c));
    if (!F0743_COMBAT_ResultDeserialize_Compat(
            &c->lastResult, buf + 0, COMBAT_RESULT_SERIALIZED_SIZE))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    if (!F0747b_COMBAT_WeaponProfileDeserialize_Compat(
            &c->lastWeapon,
            buf + COMBAT_RESULT_SERIALIZED_SIZE,
            WEAPON_PROFILE_SERIALIZED_SIZE))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    c->rng.seed     = read_u32_le(buf + 88);
    c->combatActive = read_i32_le(buf + 92);
    c->reserved     = read_i32_le(buf + 96);
    return SAVEGAME_OK;
}

int F0781_SAVEGAME_SerializeMagic_Compat(
    const struct MagicState_Compat* m,
    unsigned char* buf, int bufSize, int* outBytes)
{
    if (m == 0 || buf == 0 || outBytes == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < MAGIC_STATE_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    if (!F0768a_MAGIC_MagicStateSerialize_Compat(m, buf, bufSize))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    *outBytes = MAGIC_STATE_SERIALIZED_SIZE;
    return SAVEGAME_OK;
}

int F0781_SAVEGAME_DeserializeMagic_Compat(
    struct MagicState_Compat* m,
    const unsigned char* buf, int bufSize)
{
    if (m == 0 || buf == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < MAGIC_STATE_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    if (!F0768b_MAGIC_MagicStateDeserialize_Compat(m, buf, bufSize))
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    return SAVEGAME_OK;
}

/*
 * DungeonMutationList: 4-byte count LE + 1024 fixed 48-byte slots.
 * Unused slots zero-filled. Each slot is 12 int32 LE.
 *
 * BUG-112 (v2.7.13 fix): the field-mask bit layout is now defined per
 * mutation kind in include/memory_savegame_pc34_compat.h
 * (DUNGEON_MUTATION_FIELD_*). The mask identifies which sub-field of
 * the targeted record actually changed, so a replay engine can apply
 * the delta without dragging a full byte-blob diff through the save.
 *
 * ReDMCSB anchor: LOADSAVE.C F0433 (SaveGame, lines 1502-1707) and
 * F0435 (LoadGame, lines 2192-2660). The original PC 3.4 code does
 * NOT carry a per-field bitmask — F0433 writes L1348_s_GlobalData and
 * M516_CHAMPIONS[] as opaque byte blocks via F0007_MAIN_CopyBytes
 * (lines 1560-1564). The DungeonMutation field-mask is a Firestaff
 * abstraction layered on top of the same byte stream, so a phase-15
 * save still round-trips through the same SAVE_HEADER framing.
 */
int F0782_SAVEGAME_SerializeDungeonDelta_Compat(
    const struct DungeonMutationList_Compat* list,
    unsigned char* buf, int bufSize, int* outBytes)
{
    int i;
    if (list == 0 || buf == 0 || outBytes == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < DUNGEON_MUTATION_LIST_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    if (list->count < 0 || list->count > DUNGEON_MUTATION_LIST_MAX_COUNT)
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;

    memset(buf, 0, DUNGEON_MUTATION_LIST_SERIALIZED_SIZE);
    write_i32_le(buf, list->count);

    for (i = 0; i < DUNGEON_MUTATION_LIST_MAX_COUNT; i++) {
        unsigned char* slot = buf + 4 + (i * DUNGEON_MUTATION_SERIALIZED_SIZE);
        if (i < list->count) {
            const struct DungeonMutation_Compat* m = &list->entries[i];
            /* BUG-112: clamp fieldMask to the per-kind valid range so
             * reserved bits never leak into the save file. */
            unsigned int validMask = dungeon_mutation_field_mask_valid(m->kind);
            unsigned int clamped = ((unsigned int)m->fieldMask) & validMask;
            write_i32_le(slot +  0, m->kind);
            write_i32_le(slot +  4, m->mapIndex);
            write_i32_le(slot +  8, m->x);
            write_i32_le(slot + 12, m->y);
            write_i32_le(slot + 16, m->cell);
            write_i32_le(slot + 20, m->thingType);
            write_i32_le(slot + 24, m->thingIndex);
            write_u32_le(slot + 28, m->beforeValue);
            write_u32_le(slot + 32, m->afterValue);
            write_i32_le(slot + 36, (int)clamped);
            write_i32_le(slot + 40, m->reserved);
            write_i32_le(slot + 44, m->reserved1);
        }
        /* Unused slots stay zero-filled. */
    }
    *outBytes = DUNGEON_MUTATION_LIST_SERIALIZED_SIZE;
    return SAVEGAME_OK;
}

int F0782b_SAVEGAME_DeserializeDungeonDelta_Compat(
    struct DungeonMutationList_Compat* list,
    const unsigned char* buf, int bufSize)
{
    int i, count;
    if (list == 0 || buf == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < DUNGEON_MUTATION_LIST_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;

    memset(list, 0, sizeof(*list));
    count = read_i32_le(buf);
    if (count < 0 || count > DUNGEON_MUTATION_LIST_MAX_COUNT)
        return SAVEGAME_ERROR_SUBSYS_DESERIALIZE;
    list->count = count;

    for (i = 0; i < DUNGEON_MUTATION_LIST_MAX_COUNT; i++) {
        const unsigned char* slot = buf + 4 + (i * DUNGEON_MUTATION_SERIALIZED_SIZE);
        struct DungeonMutation_Compat* m = &list->entries[i];
        m->kind        = read_i32_le(slot +  0);
        m->mapIndex    = read_i32_le(slot +  4);
        m->x           = read_i32_le(slot +  8);
        m->y           = read_i32_le(slot + 12);
        m->cell        = read_i32_le(slot + 16);
        m->thingType   = read_i32_le(slot + 20);
        m->thingIndex  = read_i32_le(slot + 24);
        m->beforeValue = read_u32_le(slot + 28);
        m->afterValue  = read_u32_le(slot + 32);
        /* BUG-112: clamp fieldMask to the per-kind valid range so a
         * older / non-conformant save with stray reserved bits still
         * round-trips into a sane in-memory representation. */
        m->fieldMask   = (int)(((unsigned int)read_i32_le(slot + 36)) &
                               dungeon_mutation_field_mask_valid(m->kind));
        m->reserved    = read_i32_le(slot + 40);
        m->reserved1   = read_i32_le(slot + 44);
    }
    return SAVEGAME_OK;
}

/* ==========================================================
 *  Group D — Section table helpers (F0783 / F0784)
 * ========================================================== */

/* Compile-time-constant size vector, indexed by slot. */
static const uint32_t SAVEGAME_SECTION_SIZES[SAVEGAME_SECTION_COUNT] = {
    (uint32_t)PARTY_SERIALIZED_SIZE,                 /* 0 PARTY     */
    (uint32_t)MOVEMENT_RESULT_SERIALIZED_SIZE,       /* 1 MOVEMENT  */
    (uint32_t)SENSOR_EFFECT_LIST_SERIALIZED_SIZE,    /* 2 SENSOR    */
    (uint32_t)TIMELINE_QUEUE_SERIALIZED_SIZE,        /* 3 TIMELINE  */
    (uint32_t)COMBAT_SCRATCH_SERIALIZED_SIZE,        /* 4 COMBAT    */
    (uint32_t)MAGIC_STATE_SERIALIZED_SIZE,           /* 5 MAGIC     */
    (uint32_t)DUNGEON_MUTATION_LIST_SERIALIZED_SIZE  /* 6 DUNGEON_DELTA */
};

static const uint32_t SAVEGAME_SECTION_KINDS[SAVEGAME_SECTION_COUNT] = {
    SAVEGAME_SECTION_PARTY,
    SAVEGAME_SECTION_MOVEMENT,
    SAVEGAME_SECTION_SENSOR,
    SAVEGAME_SECTION_TIMELINE,
    SAVEGAME_SECTION_COMBAT,
    SAVEGAME_SECTION_MAGIC,
    SAVEGAME_SECTION_DUNGEON_DELTA
};

int F0783_SAVEGAME_ComputeSectionTable_Compat(
    const struct SaveGame_Compat* state,
    struct SaveGameSectionEntry_Compat outTable[SAVEGAME_SECTION_COUNT],
    uint32_t* outTotalSize)
{
    uint32_t off;
    int slot;
    (void)state; /* Sizes are compile-time constants; see plan §4.4. */
    if (outTable == 0 || outTotalSize == 0) return SAVEGAME_ERROR_NULL_ARG;

    off = (uint32_t)(SAVEGAME_HEADER_SERIALIZED_SIZE +
                     SAVEGAME_SECTION_COUNT * SAVEGAME_SECTION_ENTRY_SERIALIZED_SIZE);
    for (slot = 0; slot < SAVEGAME_SECTION_COUNT; slot++) {
        outTable[slot].kind     = SAVEGAME_SECTION_KINDS[slot];
        outTable[slot].offset   = off;
        outTable[slot].size     = SAVEGAME_SECTION_SIZES[slot];
        outTable[slot].reserved = 0;
        off += SAVEGAME_SECTION_SIZES[slot];
    }
    *outTotalSize = off;
    return SAVEGAME_OK;
}

int F0784_SAVEGAME_ValidateSectionTable_Compat(
    const struct SaveGameSectionEntry_Compat table[SAVEGAME_SECTION_COUNT],
    uint32_t totalFileSize)
{
    uint32_t expectedOffset;
    int slot;
    if (table == 0) return SAVEGAME_ERROR_NULL_ARG;

    /* First slot must start at header + section table. */
    expectedOffset = (uint32_t)(SAVEGAME_HEADER_SERIALIZED_SIZE +
                               SAVEGAME_SECTION_COUNT *
                               SAVEGAME_SECTION_ENTRY_SERIALIZED_SIZE);

    for (slot = 0; slot < SAVEGAME_SECTION_COUNT; slot++) {
        uint64_t endBoundary;
        if (table[slot].kind != SAVEGAME_SECTION_KINDS[slot])
            return SAVEGAME_ERROR_BAD_SECTION_KIND;
        if (table[slot].reserved != 0)
            return SAVEGAME_ERROR_BAD_SECTION_ORDER;
        /* Offset must equal the deterministic running offset. Any
         * other value would overlap or skip. */
        if (table[slot].offset != expectedOffset) {
            /* Classify: if < expected -> overlap with prior slot; if
             * > expected -> gap/order error. */
            if (table[slot].offset < expectedOffset)
                return SAVEGAME_ERROR_SECTION_OVERLAP;
            return SAVEGAME_ERROR_BAD_SECTION_ORDER;
        }
        if (table[slot].size != SAVEGAME_SECTION_SIZES[slot])
            return SAVEGAME_ERROR_BAD_SECTION_ORDER;
        endBoundary = (uint64_t)table[slot].offset +
                      (uint64_t)table[slot].size;
        if (endBoundary > (uint64_t)totalFileSize)
            return SAVEGAME_ERROR_SECTION_OVERFLOW;
        expectedOffset = (uint32_t)endBoundary;
    }
    if (expectedOffset != totalFileSize)
        return SAVEGAME_ERROR_BAD_SIZE;
    return SAVEGAME_OK;
}

/* ==========================================================
 *  Group B — Top-level save / load (F0773 / F0774 / F0775)
 * ========================================================== */

static int savegame_serialise_slot(
    int slot,
    const struct SaveGame_Compat* state,
    unsigned char* buf, int bufSize, int* outBytes)
{
    switch (slot) {
    case 0:
        if (state->party == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0776_SAVEGAME_SerializeParty_Compat(
            state->party, buf, bufSize, outBytes);
    case 1:
        if (state->lastMovement == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0777_SAVEGAME_SerializeMovement_Compat(
            state->lastMovement, buf, bufSize, outBytes);
    case 2:
        if (state->pendingSensorEffects == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0778_SAVEGAME_SerializeSensor_Compat(
            state->pendingSensorEffects, buf, bufSize, outBytes);
    case 3:
        if (state->timeline == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0779_SAVEGAME_SerializeTimeline_Compat(
            state->timeline, buf, bufSize, outBytes);
    case 4:
        if (state->combatScratch == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0780_SAVEGAME_SerializeCombat_Compat(
            state->combatScratch, buf, bufSize, outBytes);
    case 5:
        if (state->magic == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0781_SAVEGAME_SerializeMagic_Compat(
            state->magic, buf, bufSize, outBytes);
    case 6:
        if (state->mutations == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0782_SAVEGAME_SerializeDungeonDelta_Compat(
            state->mutations, buf, bufSize, outBytes);
    default:
        return SAVEGAME_ERROR_INTERNAL;
    }
}

static int savegame_deserialise_slot(
    int slot,
    const unsigned char* buf, int bufSize,
    struct SaveGame_Compat* outState)
{
    switch (slot) {
    case 0:
        if (outState->party == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0776_SAVEGAME_DeserializeParty_Compat(
            outState->party, buf, bufSize);
    case 1:
        if (outState->lastMovement == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0777_SAVEGAME_DeserializeMovement_Compat(
            outState->lastMovement, buf, bufSize);
    case 2:
        if (outState->pendingSensorEffects == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0778_SAVEGAME_DeserializeSensor_Compat(
            outState->pendingSensorEffects, buf, bufSize);
    case 3:
        if (outState->timeline == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0779_SAVEGAME_DeserializeTimeline_Compat(
            outState->timeline, buf, bufSize);
    case 4:
        if (outState->combatScratch == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0780b_SAVEGAME_DeserializeCombat_Compat(
            outState->combatScratch, buf, bufSize);
    case 5:
        if (outState->magic == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0781_SAVEGAME_DeserializeMagic_Compat(
            outState->magic, buf, bufSize);
    case 6:
        if (outState->mutations == 0) return SAVEGAME_ERROR_NULL_ARG;
        return F0782b_SAVEGAME_DeserializeDungeonDelta_Compat(
            outState->mutations, buf, bufSize);
    default:
        return SAVEGAME_ERROR_INTERNAL;
    }
}

int F0773_SAVEGAME_SaveToBuffer_Compat(
    const struct SaveGame_Compat* state,
    unsigned char* outBuf, int outBufSize,
    int* outBytesWritten)
{
    struct SaveGameSectionEntry_Compat table[SAVEGAME_SECTION_COUNT];
    struct SaveGameHeader_Compat hdr;
    uint32_t totalSize = 0;
    uint32_t bodyCRC;
    int slot;
    int err;

    if (state == 0 || outBuf == 0 || outBytesWritten == 0)
        return SAVEGAME_ERROR_NULL_ARG;

    err = F0783_SAVEGAME_ComputeSectionTable_Compat(state, table, &totalSize);
    if (err != SAVEGAME_OK) return err;
    if ((int)totalSize > outBufSize)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;

    /* Determinism in reserved regions (header.reserved, unused
     * section-table slot.reserved, per-subsystem padding). */
    memset(outBuf, 0, totalSize);

    /* Pass 1: write each section payload. */
    for (slot = 0; slot < SAVEGAME_SECTION_COUNT; slot++) {
        int bytesWritten = 0;
        unsigned char* dst = outBuf + table[slot].offset;
        int dstSize = (int)table[slot].size;
        err = savegame_serialise_slot(slot, state, dst, dstSize, &bytesWritten);
        if (err != SAVEGAME_OK) return err;
        if ((uint32_t)bytesWritten != table[slot].size)
            return SAVEGAME_ERROR_INTERNAL;
    }

    /* Pass 2: write section table at offset 64. */
    for (slot = 0; slot < SAVEGAME_SECTION_COUNT; slot++) {
        unsigned char* entry = outBuf +
            SAVEGAME_HEADER_SERIALIZED_SIZE +
            slot * SAVEGAME_SECTION_ENTRY_SERIALIZED_SIZE;
        write_u32_le(entry +  0, table[slot].kind);
        write_u32_le(entry +  4, table[slot].offset);
        write_u32_le(entry +  8, table[slot].size);
        write_u32_le(entry + 12, table[slot].reserved);
    }

    /* Pass 3: compute body CRC and stamp header. */
    bodyCRC = F0770_SAVEGAME_CRC32_Compat(
        outBuf + SAVEGAME_HEADER_SERIALIZED_SIZE,
        (size_t)(totalSize - SAVEGAME_HEADER_SERIALIZED_SIZE));
    err = F0771_SAVEGAME_WriteHeader_Compat(&hdr, totalSize, bodyCRC);
    if (err != SAVEGAME_OK) return err;
    memcpy(hdr.reserved, state->header.reserved, sizeof(hdr.reserved));
    savegame_header_write(outBuf, &hdr);

    *outBytesWritten = (int)totalSize;
    return SAVEGAME_OK;
}

int F0774_SAVEGAME_LoadFromBuffer_Compat(
    const unsigned char* buf, int bufSize,
    struct SaveGame_Compat* outState)
{
    struct SaveGameHeader_Compat hdr;
    struct SaveGameSectionEntry_Compat table[SAVEGAME_SECTION_COUNT];
    uint32_t bodyCRC;
    int slot;
    int err;

    if (buf == 0 || outState == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < SAVEGAME_HEADER_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;

    outState->lastFailingSection = -1;

    /* Phase 1: header */
    savegame_header_read(buf, &hdr);
    err = F0772_SAVEGAME_ValidateHeader_Compat(&hdr, (uint32_t)bufSize);
    if (err != SAVEGAME_OK) return err;

    /* Phase 2: body CRC */
    bodyCRC = F0770_SAVEGAME_CRC32_Compat(
        buf + SAVEGAME_HEADER_SERIALIZED_SIZE,
        (size_t)(hdr.totalFileSize - SAVEGAME_HEADER_SERIALIZED_SIZE));
    if (bodyCRC != hdr.bodyCRC32) return SAVEGAME_ERROR_BAD_CRC;

    /* Phase 3: section table */
    for (slot = 0; slot < SAVEGAME_SECTION_COUNT; slot++) {
        const unsigned char* entry = buf +
            SAVEGAME_HEADER_SERIALIZED_SIZE +
            slot * SAVEGAME_SECTION_ENTRY_SERIALIZED_SIZE;
        table[slot].kind     = read_u32_le(entry +  0);
        table[slot].offset   = read_u32_le(entry +  4);
        table[slot].size     = read_u32_le(entry +  8);
        table[slot].reserved = read_u32_le(entry + 12);
    }
    err = F0784_SAVEGAME_ValidateSectionTable_Compat(table, hdr.totalFileSize);
    if (err != SAVEGAME_OK) return err;

    /* Phase 4: delegate to each subsystem. */
    for (slot = 0; slot < SAVEGAME_SECTION_COUNT; slot++) {
        uint64_t endBoundary =
            (uint64_t)table[slot].offset + (uint64_t)table[slot].size;
        if (endBoundary > (uint64_t)hdr.totalFileSize)
            return SAVEGAME_ERROR_SECTION_OVERFLOW;
        err = savegame_deserialise_slot(
            slot,
            buf + table[slot].offset,
            (int)table[slot].size,
            outState);
        if (err != SAVEGAME_OK) {
            outState->lastFailingSection = slot;
            return err;
        }
    }

    /* Phase 5: stamp header into outState for round-trip parity. */
    outState->header = hdr;
    return SAVEGAME_OK;
}

int F0775_SAVEGAME_InitEmpty_Compat(struct SaveGame_Compat* state) {
    if (state == 0) return SAVEGAME_ERROR_NULL_ARG;
    memset(&state->header, 0, sizeof(state->header));
    state->lastFailingSection = -1;
    /* Subsystem pointers are caller-owned — do NOT touch them. */
    return SAVEGAME_OK;
}

/* ==========================================================
 *  Group E — Filesystem wrappers (F0785 / F0786).
 *  Only IO in Phase 15. Contract: fully read or fully write; no
 *  partial operations; no mmap; all buffers live in the process
 *  heap for the duration of one call.
 * ========================================================== */

int F0785_SAVEGAME_SaveToFile_Compat(
    const char* path,
    const struct SaveGame_Compat* state)
{
    struct SaveGameSectionEntry_Compat table[SAVEGAME_SECTION_COUNT];
    uint32_t totalSize = 0;
    unsigned char* buf;
    FILE* f;
    int written = 0;
    int err;
    size_t w;

    if (path == 0 || state == 0) return SAVEGAME_ERROR_NULL_ARG;
    err = F0783_SAVEGAME_ComputeSectionTable_Compat(state, table, &totalSize);
    if (err != SAVEGAME_OK) return err;
    if (totalSize == 0) return SAVEGAME_ERROR_INTERNAL;

    buf = (unsigned char*)malloc((size_t)totalSize);
    if (buf == 0) return SAVEGAME_ERROR_INTERNAL;

    err = F0773_SAVEGAME_SaveToBuffer_Compat(state, buf, (int)totalSize, &written);
    if (err != SAVEGAME_OK) { free(buf); return err; }

    f = fopen(path, "wb");
    if (f == 0) { free(buf); return SAVEGAME_ERROR_FILE_OPEN; }
    w = fwrite(buf, 1, (size_t)totalSize, f);
    fclose(f);
    free(buf);
    if (w != (size_t)totalSize) return SAVEGAME_ERROR_FILE_WRITE;
    return SAVEGAME_OK;
}

int F0786_SAVEGAME_LoadFromFile_Compat(
    const char* path,
    struct SaveGame_Compat* outState)
{
    FILE* f;
    long sz;
    unsigned char* buf;
    size_t r;
    int err;

    if (path == 0 || outState == 0) return SAVEGAME_ERROR_NULL_ARG;
    f = fopen(path, "rb");
    if (f == 0) return SAVEGAME_ERROR_FILE_OPEN;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return SAVEGAME_ERROR_FILE_READ; }
    sz = ftell(f);
    if (sz <= 0 || sz > SAVEGAME_MAX_FILE_SIZE) {
        fclose(f); return SAVEGAME_ERROR_FILE_SIZE;
    }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return SAVEGAME_ERROR_FILE_READ; }
    buf = (unsigned char*)malloc((size_t)sz);
    if (buf == 0) { fclose(f); return SAVEGAME_ERROR_INTERNAL; }
    r = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (r != (size_t)sz) { free(buf); return SAVEGAME_ERROR_FILE_READ; }

    err = F0774_SAVEGAME_LoadFromBuffer_Compat(buf, (int)sz, outState);
    free(buf);
    return err;
}

/* ==========================================================
 *  Group F — Diagnostics (F0787 / F0788 / F0789)
 * ========================================================== */

int F0787_SAVEGAME_Compare_Compat(
    const unsigned char* bufA, int sizeA,
    const unsigned char* bufB, int sizeB,
    int* outFirstDiffOffset)
{
    int i;
    int lim = sizeA < sizeB ? sizeA : sizeB;
    if (outFirstDiffOffset) *outFirstDiffOffset = -1;
    if (bufA == 0 || bufB == 0) return 0;
    if (sizeA != sizeB) {
        if (outFirstDiffOffset) *outFirstDiffOffset = lim;
        return 0;
    }
    for (i = 0; i < lim; i++) {
        if (bufA[i] != bufB[i]) {
            if (outFirstDiffOffset) *outFirstDiffOffset = i;
            return 0;
        }
    }
    return 1;
}

int F0788_SAVEGAME_InspectHeader_Compat(
    const unsigned char* buf, int bufSize,
    struct SaveGameHeader_Compat* outHdr)
{
    if (buf == 0 || outHdr == 0) return SAVEGAME_ERROR_NULL_ARG;
    if (bufSize < SAVEGAME_HEADER_SERIALIZED_SIZE)
        return SAVEGAME_ERROR_BUFFER_TOO_SMALL;
    savegame_header_read(buf, outHdr);
    /* Deliberately tolerant — no validation. */
    return SAVEGAME_OK;
}

const char* F0789_SAVEGAME_ErrorToString_Compat(int code) {
    switch (code) {
    case SAVEGAME_OK:                       return "ok";
    case SAVEGAME_ERROR_NULL_ARG:           return "null argument";
    case SAVEGAME_ERROR_BUFFER_TOO_SMALL:   return "buffer too small";
    case SAVEGAME_ERROR_BAD_MAGIC:          return "bad magic";
    case SAVEGAME_ERROR_BAD_VERSION:        return "bad version";
    case SAVEGAME_ERROR_BAD_ENDIAN:         return "bad endian sentinel";
    case SAVEGAME_ERROR_BAD_SIZE:           return "bad total file size";
    case SAVEGAME_ERROR_BAD_CRC:            return "bad CRC";
    case SAVEGAME_ERROR_BAD_SECTION_COUNT:  return "bad section count";
    case SAVEGAME_ERROR_BAD_SECTION_KIND:   return "bad section kind";
    case SAVEGAME_ERROR_BAD_SECTION_ORDER:  return "bad section order";
    case SAVEGAME_ERROR_SECTION_OVERFLOW:   return "section overflows file";
    case SAVEGAME_ERROR_SECTION_OVERLAP:    return "section overlaps another";
    case SAVEGAME_ERROR_SUBSYS_DESERIALIZE: return "subsystem deserialise failed";
    case SAVEGAME_ERROR_FILE_OPEN:          return "file open failed";
    case SAVEGAME_ERROR_FILE_READ:          return "file read failed";
    case SAVEGAME_ERROR_FILE_WRITE:         return "file write failed";
    case SAVEGAME_ERROR_FILE_SIZE:          return "file size invalid";
    case SAVEGAME_ERROR_INTERNAL:           return "internal error";
    default:                                 return "unknown";
    }
}

/* ================================================================
 *  F0417 SaveUtil port (minimal subset) — SAVEHEAD.C:44,97,104
 * ================================================================
 *
 * The original ReDMCSB F0417_SAVEUTIL_GetChecksumAndObfuscate
 * carries Noise[10] + Keys[16] + Checksums[16] + a 16-byte XOR
 * pass derived from the running Noise accumulator.  v1 ports a
 * minimal subset:
 *
 *   - 10 uint16_t Noise entries (deterministic from the
 *     exporter's RNG state).
 *   - 16 uint16_t per-section XOR keys derived from
 *     crc32(noise[0..9]).
 *   - 16 uint32_t per-section checksums (caller-supplied
 *     pre-obfuscation; the obfuscation does not modify them
 *     because the obfuscation is in the post-checksum space).
 *   - XOR pass: byte[i] ^= noise[0] ^ (noise[i % 10] << (i % 7))
 *
 * The full F0417 port with CPSC checksum derivation and the
 * group-of-16 word XOR is deferred — the minimal port is enough
 * to byte-validate the obfuscation cycle (importer echoes the
 * same Noise[] back to deobfuscate).
 */
int F0417_SAVEUTIL_Port_Hint_Compat(
    struct SaveGameHeader_Compat* hdr,
    const uint16_t noiseSeed[10])
{
    int i;
    uint32_t derivedKey;
    if (!hdr) return 0;
    if (noiseSeed) {
        for (i = 0; i < 10; ++i) {
            hdr->noise[i] = noiseSeed[i];
        }
    }
    /* F0417: noise[0..9] -> 16 section keys via simple FNV-style
     * fold.  The full port would derive each from a different
     * byte of the CPSC hash; v1 keeps the derivation simple so
     * the importer can reverse-engineer it. */
    derivedKey = 0x811C9DC5u;  /* FNV-1a basis */
    for (i = 0; i < 10; ++i) {
        derivedKey ^= (uint32_t)hdr->noise[i];
        derivedKey *= 0x01000193u; /* FNV prime */
    }
    for (i = 0; i < 16; ++i) {
        hdr->sectionKeys[i] = (uint16_t)(derivedKey ^ (uint32_t)i * 0x9E37u);
    }
    /* sectionChecksums[0..15] is caller-supplied: the F0417
     * original does not compute them; it XORs a 16-byte block
     * of the data into the per-section key accumulator.  v1
     * exposes the field so the caller can fill it. */
    return 1;
}

int F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat(
    const struct SaveGameHeader_Compat* hdr,
    unsigned char* buf,
    int bufLen)
{
    int i;
    uint16_t runningKey;
    if (!hdr || !buf || bufLen <= 0) return 0;
    /* F0417 XOR pass: noise[0] ^ (noise[i % 10] << (i % 7)).  The
     * original uses a more complex accumulator; v1 uses a simple
     * byte-index rolling key so the importer can reverse it. */
    runningKey = hdr->noise[0];
    for (i = 0; i < bufLen; ++i) {
        uint16_t k = (uint16_t)(runningKey ^ ((uint16_t)hdr->noise[i % 10] << (i % 7)));
        buf[i] = (unsigned char)(buf[i] ^ (unsigned char)(k & 0xFF));
    }
    return 1;
}
