/*
 * Phase 20 — Tick orchestrator & deterministic harness.
 *
 * Implementation of memory_tick_orchestrator_pc34_compat.h.
 *
 * See header for documented plan deviations (D1..D5).
 */

#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "memory_door_action_pc34_compat.h"  /* Pass 38 — door animation stepper */
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_champion_needs_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_combat_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "dm1_v1_spell_casting_pc34_compat.h"
#include "firestaff/dm1/v1/G0492_pc34_compat.h"
#include "firestaff/dm1/v1/G0493_pc34_compat.h"
#include "dm1_v1_sound_pc34_compat.h"        /* DM1_SND_BUZZ for C006 generator audio */

enum {
    ORCH_CREATURE_BLACK_FLAME_PC34 = 11,
    ORCH_SOUND_WOODEN_THUD_PC34 = 4,
    ORCH_POTION_EMPTY_FLASK_PC34 = 20,
    ORCH_JUNK_ZOKATHRA_PC34 = 51,
    ORCH_WEAPON_TORCH_PC34 = 2,
    ORCH_TORCH_DECAY_TICK_MASK_PC34 = 511,
    ORCH_BLACK_FLAME_MAX_HEALTH_PC34 = 1000
};

static const int s_orch_light_power_to_amount_pc34[16] = {
    0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100
};

static const int s_orch_palette_index_to_light_amount_pc34[6] = {
    99, 75, 50, 25, 1, 0
};

static const unsigned char s_orch_thing_data_byte_count[16] = {
    /* DUNGEON.DAT raw Thing strides; keep aligned with
     * memory_dungeon_dat_pc34_compat.c decode_* offsets and native export. */
    4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
};

static unsigned short orch_make_thing_ref_compat(int type, int index);
static int orch_square_first_thing_list_index_compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY);

/* ================================================================
 *  Local LE helpers
 * ================================================================ */

static void w_u32(unsigned char* p, uint32_t v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    p[2] = (unsigned char)((v >> 16) & 0xFFu);
    p[3] = (unsigned char)((v >> 24) & 0xFFu);
}
static uint32_t r_u32(const unsigned char* p) {
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}
static void w_i32(unsigned char* p, int32_t v) { w_u32(p, (uint32_t)v); }
static int32_t r_i32(const unsigned char* p) { return (int32_t)r_u32(p); }
static void w_u16(unsigned char* p, uint16_t v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
}
static uint16_t r_u16(const unsigned char* p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

/* CRC32 (reuse of Phase 15 poly). */
static uint32_t crc32_ieee(const unsigned char* buf, size_t len) {
    return F0770_SAVEGAME_CRC32_Compat(buf, len);
}

/* Compute fingerprint of a DUNGEON.DAT file on disk. 0 on failure. */
static uint32_t dungeon_file_fingerprint(const char* path) {
    FILE* f = fopen(path, "rb");
    unsigned char buf[4096];
    uint32_t c = 0xFFFFFFFFu;
    size_t got;
    int k;
    size_t i;
    if (!f) return 0u;
    while ((got = fread(buf, 1, sizeof buf, f)) > 0) {
        for (i = 0; i < got; i++) {
            c ^= (uint32_t)buf[i];
            for (k = 0; k < 8; k++) {
                uint32_t mask = (uint32_t)(-(int32_t)(c & 1u));
                c = (c >> 1) ^ (0xEDB88320u & mask);
            }
        }
    }
    fclose(f);
    return c ^ 0xFFFFFFFFu;
}

static void set_party_direction_redmcsb_compat(struct PartyState_Compat* party, int newDirection) {
    int oldDirection;
    int delta;
    int i;
    if (!party) return;
    newDirection &= 3;
    oldDirection = party->direction & 3;
    if (newDirection == oldDirection) {
        party->direction = newDirection;
        return;
    }
    /* ReDMCSB CHAMPION.C:117-130, F0284_CHAMPION_SetPartyDirection:
     * if direction changes, delta = new - old normalized to [0..3], then
     * every party champion Cell and Direction are rotated by delta before
     * G0308_i_PartyDirection is updated. Compat currently stores champion
     * Direction (not Cell), so keep it source-aligned here. */
    delta = newDirection - oldDirection;
    if (delta < 0) delta += 4;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        if (party->champions[i].present) {
            party->champions[i].cell = (unsigned char)((party->champions[i].cell + delta) & 3);
            party->champions[i].direction = (unsigned char)((party->champions[i].direction + delta) & 3);
        }
    }
    party->direction = newDirection;
}

/* Emit a single emission into the result, ignoring overflow. */
static void emit(struct TickResult_Compat* r, uint8_t kind,
                 int32_t a, int32_t b, int32_t c, int32_t d) {
    struct TickEmission_Compat* e;
    if (!r) return;
    if (r->emissionCount >= TICK_EMISSION_CAPACITY) return;
    e = &r->emissions[r->emissionCount++];
    memset(e, 0, sizeof(*e));
    e->kind = kind;
    e->payloadSize = 16;
    e->payload[0] = a;
    e->payload[1] = b;
    e->payload[2] = c;
    e->payload[3] = d;
}

struct OrchTeleporterBuzz_Compat {
    int mapIndex;
    int mapX;
    int mapY;
};

struct OrchTeleporterBuzzList_Compat {
    int count;
    struct OrchTeleporterBuzz_Compat items[TICK_EMISSION_CAPACITY];
};

static void orch_teleporter_buzz_list_init_compat(
    struct OrchTeleporterBuzzList_Compat* list)
{
    if (list) memset(list, 0, sizeof(*list));
}

static void orch_record_teleporter_buzz_compat(
    struct OrchTeleporterBuzzList_Compat* list,
    int mapIndex,
    int mapX,
    int mapY)
{
    struct OrchTeleporterBuzz_Compat* item;
    if (!list || list->count >= TICK_EMISSION_CAPACITY) return;
    item = &list->items[list->count++];
    item->mapIndex = mapIndex;
    item->mapX = mapX;
    item->mapY = mapY;
}

static void orch_emit_teleporter_buzzes_compat(
    struct TickResult_Compat* result,
    const struct OrchTeleporterBuzzList_Compat* list)
{
    int i;
    if (!result || !list) return;
    for (i = 0; i < list->count; ++i) {
        emit(result, EMIT_SOUND_REQUEST, DM1_SND_BUZZ,
             list->items[i].mapX, list->items[i].mapY,
             list->items[i].mapIndex);
    }
}

/* ================================================================
 *  Small-struct serialisers
 * ================================================================ */

int F0897a_TickInput_Serialize_Compat(
    const struct TickInput_Compat* in,
    unsigned char* outBuf, int outBufSize)
{
    if (!in || !outBuf || outBufSize < TICK_INPUT_SERIALIZED_SIZE) return 0;
    memset(outBuf, 0, TICK_INPUT_SERIALIZED_SIZE);
    w_u32(outBuf + 0, in->tick);
    outBuf[4] = in->command;
    outBuf[5] = in->commandArg1;
    outBuf[6] = in->commandArg2;
    outBuf[7] = in->reserved;
    w_u32(outBuf + 8, in->forcedRngAdvance);
    w_u32(outBuf + 12, in->reserved2);
    return 1;
}
int F0897a_TickInput_Deserialize_Compat(
    struct TickInput_Compat* out,
    const unsigned char* buf, int bufSize)
{
    if (!out || !buf || bufSize < TICK_INPUT_SERIALIZED_SIZE) return 0;
    memset(out, 0, sizeof(*out));
    out->tick = r_u32(buf + 0);
    out->command = buf[4];
    out->commandArg1 = buf[5];
    out->commandArg2 = buf[6];
    out->reserved = buf[7];
    out->forcedRngAdvance = r_u32(buf + 8);
    out->reserved2 = r_u32(buf + 12);
    return 1;
}

int F0897b_TickEmission_Serialize_Compat(
    const struct TickEmission_Compat* in,
    unsigned char* outBuf, int outBufSize)
{
    int i;
    if (!in || !outBuf || outBufSize < TICK_EMISSION_SERIALIZED_SIZE) return 0;
    memset(outBuf, 0, TICK_EMISSION_SERIALIZED_SIZE);
    outBuf[0] = in->kind;
    outBuf[1] = in->reserved;
    w_u16(outBuf + 2, in->payloadSize);
    for (i = 0; i < 4; i++) w_i32(outBuf + 4 + 4*i, in->payload[i]);
    return 1;
}
int F0897b_TickEmission_Deserialize_Compat(
    struct TickEmission_Compat* out,
    const unsigned char* buf, int bufSize)
{
    int i;
    if (!out || !buf || bufSize < TICK_EMISSION_SERIALIZED_SIZE) return 0;
    memset(out, 0, sizeof(*out));
    out->kind = buf[0];
    out->reserved = buf[1];
    out->payloadSize = r_u16(buf + 2);
    for (i = 0; i < 4; i++) out->payload[i] = r_i32(buf + 4 + 4*i);
    return 1;
}

int F0897c_TickStreamRecord_Serialize_Compat(
    const struct TickStreamRecord_Compat* in,
    unsigned char* outBuf, int outBufSize)
{
    if (!in || !outBuf || outBufSize < TICK_STREAM_RECORD_SERIALIZED_SIZE) return 0;
    memset(outBuf, 0, TICK_STREAM_RECORD_SERIALIZED_SIZE);
    if (!F0897a_TickInput_Serialize_Compat(&in->input, outBuf, TICK_INPUT_SERIALIZED_SIZE))
        return 0;
    w_u32(outBuf + 16, in->worldHashPost);
    w_u16(outBuf + 20, in->emissionCount);
    w_u16(outBuf + 22, in->reserved);
    return 1;
}
int F0897c_TickStreamRecord_Deserialize_Compat(
    struct TickStreamRecord_Compat* out,
    const unsigned char* buf, int bufSize)
{
    if (!out || !buf || bufSize < TICK_STREAM_RECORD_SERIALIZED_SIZE) return 0;
    memset(out, 0, sizeof(*out));
    if (!F0897a_TickInput_Deserialize_Compat(&out->input, buf, TICK_INPUT_SERIALIZED_SIZE))
        return 0;
    out->worldHashPost = r_u32(buf + 16);
    out->emissionCount = r_u16(buf + 20);
    out->reserved = r_u16(buf + 22);
    return 1;
}

int F0897d_GameConfig_Serialize_Compat(
    const struct GameConfig_Compat* in,
    unsigned char* outBuf, int outBufSize)
{
    if (!in || !outBuf || outBufSize < GAME_CONFIG_SERIALIZED_SIZE) return 0;
    memset(outBuf, 0, GAME_CONFIG_SERIALIZED_SIZE);
    memcpy(outBuf + 0, in->dungeonPath, 48);
    /* NUL-terminate defensively even though caller should provide it */
    outBuf[47] = (unsigned char)(in->dungeonPath[47] == 0 ? 0 : in->dungeonPath[47]);
    w_u32(outBuf + 48, in->startingSeed);
    w_u32(outBuf + 52, in->flags);
    w_u32(outBuf + 56, in->reserved[0]);
    w_u32(outBuf + 60, in->reserved[1]);
    return 1;
}
int F0897d_GameConfig_Deserialize_Compat(
    struct GameConfig_Compat* out,
    const unsigned char* buf, int bufSize)
{
    if (!out || !buf || bufSize < GAME_CONFIG_SERIALIZED_SIZE) return 0;
    memset(out, 0, sizeof(*out));
    memcpy(out->dungeonPath, buf + 0, 48);
    out->dungeonPath[47] = 0; /* ensure NUL term */
    out->startingSeed = r_u32(buf + 48);
    out->flags = r_u32(buf + 52);
    out->reserved[0] = r_u32(buf + 56);
    out->reserved[1] = r_u32(buf + 60);
    return 1;
}

/* ================================================================
 *  Group F — Serialise / Deserialise GameWorld (F0897-F0899)
 *
 *  Section framing: [tag u32][size u32][payload bytes].
 *  See plan §2.1 for serialisation order.
 * ================================================================ */

#define SEC_TAG_ORCH_SCALARS      0x20000001u
#define SEC_TAG_DUNGEON_FP        0x20000002u
#define SEC_TAG_PARTY             0x20000010u
#define SEC_TAG_TIMELINE          0x20000011u
#define SEC_TAG_COMBAT_RESULT     0x20000012u
#define SEC_TAG_MAGIC             0x20000013u
#define SEC_TAG_DUNGEON_MUTATIONS 0x20000014u
#define SEC_TAG_CREATURE_AI       0x20000015u
#define SEC_TAG_PROJECTILES       0x20000016u
#define SEC_TAG_EXPLOSIONS        0x20000017u
#define SEC_TAG_LIFECYCLE         0x20000018u
#define SEC_TAG_SENSOR_PENDING    0x20000019u
#define SEC_TAG_SAVE_HEADER       0x2000001Au

#define ORCH_SCALARS_PAYLOAD_SIZE 52  /* 13 × int32: gameTick, partyDead,
                                         gameWon, partyMapIndex,
                                         newPartyMapIndex, masterRng.seed,
                                         partyIsResting, freezeLifeTicks,
                                         disabledMovementTicks,
                                         projectileDisabledMovementTicks,
                                         lastProjectileDisabledMovementDirection,
                                         creatureAICount, reserved */

/* Subsystem serialised sizes (predicted). */
static int party_size(void)     { return PARTY_SERIALIZED_SIZE; }
static int timeline_size(void)  { return TIMELINE_QUEUE_SERIALIZED_SIZE; }
static int combat_res_size(void){ return COMBAT_RESULT_SERIALIZED_SIZE; }
static int magic_size(void)     { return MAGIC_STATE_SERIALIZED_SIZE; }
static int mutations_size(void) { return DUNGEON_MUTATION_LIST_SERIALIZED_SIZE; }
static int ai_size(void)        { return CREATURE_AI_STATE_SERIALIZED_SIZE; }
static int proj_list_size(void) { return PROJECTILE_LIST_SERIALIZED_SIZE; }
static int expl_list_size(void) { return EXPLOSION_LIST_SERIALIZED_SIZE; }
static int lifecycle_size(void) { return LIFECYCLE_STATE_SERIALIZED_SIZE; }
static int sensor_size(void)    { return SENSOR_EFFECT_LIST_SERIALIZED_SIZE; }
static int save_hdr_size(void)  { return SAVEGAME_HEADER_SERIALIZED_SIZE; }

int F0899_WORLD_SerializedSize_Compat(const struct GameWorld_Compat* world) {
    int total = 0;
    int aiPayload;
    int ai_count;
    if (!world) return 0;
    ai_count = world->creatureAICount;
    if (ai_count < 0) ai_count = 0;
    if (ai_count > GAMEWORLD_CREATURE_AI_CAPACITY)
        ai_count = GAMEWORLD_CREATURE_AI_CAPACITY;
    aiPayload = 4 + (ai_count * ai_size()); /* int32 count + items */

    /* Each section: 8-byte header + payload. */
    total += 8 + ORCH_SCALARS_PAYLOAD_SIZE;
    total += 8 + 4; /* dungeon fingerprint (uint32) */
    total += 8 + party_size();
    total += 8 + timeline_size();
    total += 8 + combat_res_size();
    total += 8 + magic_size();
    total += 8 + mutations_size();
    total += 8 + aiPayload;
    total += 8 + proj_list_size();
    total += 8 + expl_list_size();
    total += 8 + lifecycle_size();
    total += 8 + sensor_size();
    total += 8 + save_hdr_size();
    return total;
}

static int write_section(unsigned char* out, int outSize, int* off,
                         uint32_t tag, int payloadSize,
                         int (*writer)(const void*, unsigned char*, int),
                         const void* source)
{
    if (*off + 8 + payloadSize > outSize) return 0;
    w_u32(out + *off, tag);
    w_u32(out + *off + 4, (uint32_t)payloadSize);
    *off += 8;
    if (writer) {
        if (!writer(source, out + *off, payloadSize)) return 0;
    } else if (source) {
        memcpy(out + *off, source, (size_t)payloadSize);
    } else {
        memset(out + *off, 0, (size_t)payloadSize);
    }
    *off += payloadSize;
    return 1;
}

/* Adapters that return 1/0 and call the real subsystem serialisers. */
static int ad_party(const void* src, unsigned char* buf, int sz) {
    int rc = F0604_PARTY_Serialize_Compat((const struct PartyState_Compat*)src, buf, sz);
    return rc > 0 ? 1 : 0;
}
static int ad_timeline(const void* src, unsigned char* buf, int sz) {
    return F0727_TIMELINE_QueueSerialize_Compat((const struct TimelineQueue_Compat*)src, buf, sz);
}
static int ad_combat_res(const void* src, unsigned char* buf, int sz) {
    return F0742_COMBAT_ResultSerialize_Compat((const struct CombatResult_Compat*)src, buf, sz);
}
static int ad_magic(const void* src, unsigned char* buf, int sz) {
    return F0768a_MAGIC_MagicStateSerialize_Compat((const struct MagicState_Compat*)src, buf, sz);
}
static int ad_mutations(const void* src, unsigned char* buf, int sz) {
    int outBytes = 0;
    return F0782_SAVEGAME_SerializeDungeonDelta_Compat(
        (const struct DungeonMutationList_Compat*)src, buf, sz, &outBytes) == SAVEGAME_OK;
}
static int ad_proj_list(const void* src, unsigned char* buf, int sz) {
    return F0829_PROJECTILE_ListSerialize_Compat((const struct ProjectileList_Compat*)src, buf, sz);
}
static int ad_expl_list(const void* src, unsigned char* buf, int sz) {
    return F0829_EXPLOSION_ListSerialize_Compat((const struct ExplosionList_Compat*)src, buf, sz);
}
static int ad_lifecycle(const void* src, unsigned char* buf, int sz) {
    return F0857_LIFECYCLE_Serialize_Compat((const struct LifecycleState_Compat*)src, buf, sz);
}
static int ad_sensor(const void* src, unsigned char* buf, int sz) {
    return F0713_SENSOR_ListSerialize_Compat((const struct SensorEffectList_Compat*)src, buf, sz);
}

int F0897_WORLD_Serialize_Compat(
    const struct GameWorld_Compat* world,
    unsigned char* outBuf,
    int outBufSize,
    int* outBytesWritten)
{
    int off = 0;
    int ai_count, i;
    int aiPayloadSize;
    unsigned char scalars[ORCH_SCALARS_PAYLOAD_SIZE];
    unsigned char fp[4];

    if (!world || !outBuf) return 0;
    if (outBufSize < F0899_WORLD_SerializedSize_Compat(world)) return 0;

    ai_count = world->creatureAICount;
    if (ai_count < 0) ai_count = 0;
    if (ai_count > GAMEWORLD_CREATURE_AI_CAPACITY) ai_count = GAMEWORLD_CREATURE_AI_CAPACITY;
    aiPayloadSize = 4 + ai_count * ai_size();

    /* 1. Orchestrator scalars */
    memset(scalars, 0, sizeof(scalars));
    w_u32(scalars + 0, world->gameTick);
    w_i32(scalars + 4, world->partyDead);
    w_i32(scalars + 8, world->gameWon);
    w_i32(scalars + 12, world->partyMapIndex);
    w_i32(scalars + 16, world->newPartyMapIndex);
    w_u32(scalars + 20, world->masterRng.seed);
    w_i32(scalars + 24, world->partyIsResting);
    w_i32(scalars + 28, world->freezeLifeTicks);
    w_i32(scalars + 32, world->disabledMovementTicks);
    w_i32(scalars + 36, world->projectileDisabledMovementTicks);
    w_i32(scalars + 40, world->lastProjectileDisabledMovementDirection);
    w_i32(scalars + 44, ai_count);
    w_i32(scalars + 48, 0);

    w_u32(outBuf + off, SEC_TAG_ORCH_SCALARS); off += 4;
    w_u32(outBuf + off, ORCH_SCALARS_PAYLOAD_SIZE); off += 4;
    memcpy(outBuf + off, scalars, ORCH_SCALARS_PAYLOAD_SIZE);
    off += ORCH_SCALARS_PAYLOAD_SIZE;

    /* 2. Dungeon fingerprint */
    w_u32(fp, world->dungeonFingerprint);
    w_u32(outBuf + off, SEC_TAG_DUNGEON_FP); off += 4;
    w_u32(outBuf + off, 4); off += 4;
    memcpy(outBuf + off, fp, 4); off += 4;

    /* 3. Party */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_PARTY,
                       party_size(), ad_party, &world->party)) return 0;

    /* 4. Timeline */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_TIMELINE,
                       timeline_size(), ad_timeline, &world->timeline)) return 0;

    /* 5. Combat result */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_COMBAT_RESULT,
                       combat_res_size(), ad_combat_res, &world->pendingCombat)) return 0;

    /* 6. Magic */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_MAGIC,
                       magic_size(), ad_magic, &world->magic)) return 0;

    /* 7. Dungeon mutations */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_DUNGEON_MUTATIONS,
                       mutations_size(), ad_mutations, &world->dungeonMutations)) return 0;

    /* 8. Creature AI list */
    w_u32(outBuf + off, SEC_TAG_CREATURE_AI); off += 4;
    w_u32(outBuf + off, (uint32_t)aiPayloadSize); off += 4;
    w_i32(outBuf + off, ai_count); off += 4;
    for (i = 0; i < ai_count; i++) {
        if (off + ai_size() > outBufSize) return 0;
        if (!F0805_CREATURE_AIStateSerialize_Compat(&world->creatureAI[i],
                                                    outBuf + off, ai_size())) return 0;
        off += ai_size();
    }

    /* 9. Projectile list */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_PROJECTILES,
                       proj_list_size(), ad_proj_list, &world->projectiles)) return 0;

    /* 10. Explosion list */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_EXPLOSIONS,
                       expl_list_size(), ad_expl_list, &world->explosions)) return 0;

    /* 11. Lifecycle */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_LIFECYCLE,
                       lifecycle_size(), ad_lifecycle, &world->lifecycle)) return 0;

    /* 12. Pending sensor effects */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_SENSOR_PENDING,
                       sensor_size(), ad_sensor, &world->pendingSensorEffects)) return 0;

    /* 13. Save header — raw write of the 64-byte SaveGameHeader_Compat */
    if (off + 8 + save_hdr_size() > outBufSize) return 0;
    w_u32(outBuf + off, SEC_TAG_SAVE_HEADER); off += 4;
    w_u32(outBuf + off, (uint32_t)save_hdr_size()); off += 4;
    {
        const struct SaveGameHeader_Compat* h = &world->saveHeader;
        unsigned char* p = outBuf + off;
        memset(p, 0, save_hdr_size());
        memcpy(p + 0, h->magic, 8);
        w_u32(p + 8, h->formatVersion);
        w_u32(p + 12, h->endianSentinel);
        w_u32(p + 16, h->totalFileSize);
        w_u32(p + 20, h->sectionCount);
        w_u32(p + 24, h->bodyCRC32);
        memcpy(p + 28, h->reserved, 36);
    }
    off += save_hdr_size();

    if (outBytesWritten) *outBytesWritten = off;
    return 1;
}

static int read_section_hdr(const unsigned char* buf, int bufSize, int* off,
                            uint32_t expectedTag, uint32_t* outSize)
{
    uint32_t tag, size;
    if (*off + 8 > bufSize) return 0;
    tag = r_u32(buf + *off);
    size = r_u32(buf + *off + 4);
    if (tag != expectedTag) return 0;
    if (*off + 8 + (int)size > bufSize) return 0;
    *off += 8;
    *outSize = size;
    return 1;
}

int F0898_WORLD_Deserialize_Compat(
    struct GameWorld_Compat* world,
    const unsigned char* buf,
    int bufSize,
    int* outBytesRead)
{
    int off = 0;
    uint32_t sz;
    int i, ai_count;
    const unsigned char* p;

    if (!world || !buf) return 0;

    /* Preserve pointer fields that aren't serialised. */
    struct DungeonDatState_Compat* keep_dungeon = world->dungeon;
    struct DungeonThings_Compat*   keep_things  = world->things;
    int keep_owns = world->ownsDungeon;

    /* 1. Scalars */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_ORCH_SCALARS, &sz)) return 0;
    if (sz != ORCH_SCALARS_PAYLOAD_SIZE) return 0;
    world->gameTick = r_u32(buf + off + 0);
    world->partyDead = r_i32(buf + off + 4);
    world->gameWon   = r_i32(buf + off + 8);
    world->partyMapIndex = r_i32(buf + off + 12);
    world->newPartyMapIndex = r_i32(buf + off + 16);
    world->masterRng.seed = r_u32(buf + off + 20);
    world->partyIsResting = r_i32(buf + off + 24);
    world->freezeLifeTicks = r_i32(buf + off + 28);
    world->disabledMovementTicks = r_i32(buf + off + 32);
    world->projectileDisabledMovementTicks = r_i32(buf + off + 36);
    world->lastProjectileDisabledMovementDirection = r_i32(buf + off + 40);
    ai_count = r_i32(buf + off + 44);
    off += ORCH_SCALARS_PAYLOAD_SIZE;

    /* 2. Fingerprint */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_DUNGEON_FP, &sz)) return 0;
    if (sz != 4) return 0;
    world->dungeonFingerprint = r_u32(buf + off);
    off += 4;

    /* 3. Party */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_PARTY, &sz)) return 0;
    if ((int)sz != party_size() && (int)sz != PARTY_SERIALIZED_V1_SIZE)
        return 0;
    if (F0605_PARTY_Deserialize_Compat(&world->party, buf + off, sz) <= 0) return 0;
    off += sz;

    /* 4. Timeline */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_TIMELINE, &sz)) return 0;
    if ((int)sz != timeline_size()) return 0;
    if (!F0728_TIMELINE_QueueDeserialize_Compat(&world->timeline, buf + off, sz)) return 0;
    off += sz;

    /* 5. Combat result */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_COMBAT_RESULT, &sz)) return 0;
    if ((int)sz != combat_res_size()) return 0;
    if (!F0743_COMBAT_ResultDeserialize_Compat(&world->pendingCombat, buf + off, sz)) return 0;
    off += sz;

    /* 6. Magic */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_MAGIC, &sz)) return 0;
    if ((int)sz != magic_size()) return 0;
    if (!F0768b_MAGIC_MagicStateDeserialize_Compat(&world->magic, buf + off, sz)) return 0;
    off += sz;

    /* 7. Dungeon mutations */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_DUNGEON_MUTATIONS, &sz)) return 0;
    if ((int)sz != mutations_size()) return 0;
    if (F0782b_SAVEGAME_DeserializeDungeonDelta_Compat(
            &world->dungeonMutations, buf + off, sz) != SAVEGAME_OK) return 0;
    off += sz;

    /* 8. Creature AI list */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_CREATURE_AI, &sz)) return 0;
    {
        int count_read = r_i32(buf + off);
        if (count_read < 0 || count_read > GAMEWORLD_CREATURE_AI_CAPACITY) return 0;
        if ((int)sz != 4 + count_read * ai_size()) return 0;
        if (count_read != ai_count) return 0;
        off += 4;
        for (i = 0; i < count_read; i++) {
            if (!F0806_CREATURE_AIStateDeserialize_Compat(
                    &world->creatureAI[i], buf + off, ai_size())) return 0;
            off += ai_size();
        }
        /* Zero tail */
        for (i = count_read; i < GAMEWORLD_CREATURE_AI_CAPACITY; i++) {
            memset(&world->creatureAI[i], 0, sizeof(world->creatureAI[i]));
        }
        world->creatureAICount = count_read;
    }

    /* 9. Projectile list */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_PROJECTILES, &sz)) return 0;
    if ((int)sz != proj_list_size()) return 0;
    if (!F0829_PROJECTILE_ListDeserialize_Compat(&world->projectiles, buf + off, sz)) return 0;
    off += sz;

    /* 10. Explosion list */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_EXPLOSIONS, &sz)) return 0;
    if ((int)sz != expl_list_size()) return 0;
    if (!F0829_EXPLOSION_ListDeserialize_Compat(&world->explosions, buf + off, sz)) return 0;
    off += sz;

    /* 11. Lifecycle */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_LIFECYCLE, &sz)) return 0;
    if ((int)sz != lifecycle_size()) return 0;
    if (!F0858_LIFECYCLE_Deserialize_Compat(&world->lifecycle, buf + off, sz)) return 0;
    off += sz;

    /* 12. Sensor pending */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_SENSOR_PENDING, &sz)) return 0;
    if ((int)sz != sensor_size()) return 0;
    if (!F0714_SENSOR_ListDeserialize_Compat(&world->pendingSensorEffects, buf + off, sz)) return 0;
    off += sz;

    /* 13. Save header */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_SAVE_HEADER, &sz)) return 0;
    if ((int)sz != save_hdr_size()) return 0;
    p = buf + off;
    memcpy(world->saveHeader.magic, p + 0, 8);
    world->saveHeader.formatVersion  = r_u32(p + 8);
    world->saveHeader.endianSentinel = r_u32(p + 12);
    world->saveHeader.totalFileSize  = r_u32(p + 16);
    world->saveHeader.sectionCount   = r_u32(p + 20);
    world->saveHeader.bodyCRC32      = r_u32(p + 24);
    memcpy(world->saveHeader.reserved, p + 28, 36);
    off += sz;

    /* Restore non-serialised pointer fields. */
    world->dungeon = keep_dungeon;
    world->things  = keep_things;
    world->ownsDungeon = keep_owns;

    if (outBytesRead) *outBytesRead = off;
    return 1;
}

/* ================================================================
 *  Group D — World hash + determinism (F0891-F0893)
 * ================================================================ */

int F0891_ORCH_WorldHash_Compat(
    const struct GameWorld_Compat* world,
    uint32_t* outHash)
{
    int bufSize, written;
    unsigned char* buf;
    uint32_t h;
    if (!world || !outHash) return 0;
    bufSize = F0899_WORLD_SerializedSize_Compat(world);
    if (bufSize <= 0) return 0;
    buf = (unsigned char*)malloc((size_t)bufSize);
    if (!buf) return 0;
    if (!F0897_WORLD_Serialize_Compat(world, buf, bufSize, &written)) {
        free(buf); return 0;
    }
    h = crc32_ieee(buf, (size_t)written);
    free(buf);
    *outHash = h;
    return 1;
}

int F0892_ORCH_VerifyDeterminism_Compat(
    const struct GameWorld_Compat* initialWorld,
    const struct TickInput_Compat* inputs,
    int tickCount)
{
    struct GameWorld_Compat a, b;
    uint32_t hashA = 0, hashB = 0;
    int ok;
    int i;
    int rca, rcb;
    struct TickResult_Compat resultA, resultB;

    if (!initialWorld || !inputs || tickCount < 0) return 0;

    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    if (!F0880b_WORLD_Clone_Compat(initialWorld, &a)) return 0;
    if (!F0880b_WORLD_Clone_Compat(initialWorld, &b)) {
        F0883_WORLD_Free_Compat(&a);
        return 0;
    }

    ok = 1;
    for (i = 0; i < tickCount; i++) {
        rca = F0884_ORCH_AdvanceOneTick_Compat(&a, &inputs[i], &resultA);
        rcb = F0884_ORCH_AdvanceOneTick_Compat(&b, &inputs[i], &resultB);
        if (rca != rcb) { ok = 0; break; }
        if (resultA.worldHashPost != resultB.worldHashPost) { ok = 0; break; }
        if (rca != ORCH_OK) break; /* party dead or won — short-circuit */
    }

    if (ok) {
        if (!F0891_ORCH_WorldHash_Compat(&a, &hashA)) ok = 0;
        if (!F0891_ORCH_WorldHash_Compat(&b, &hashB)) ok = 0;
        if (hashA != hashB) ok = 0;
    }

    F0883_WORLD_Free_Compat(&a);
    F0883_WORLD_Free_Compat(&b);
    return ok;
}

int F0893_ORCH_VerifyResumeEquivalence_Compat(
    const struct GameWorld_Compat* initialWorld,
    const struct TickInput_Compat* inputs,
    int tickCount,
    int resumeAtTick)
{
    struct GameWorld_Compat straight, part, resumed;
    uint32_t hashA = 0, hashC = 0;
    int ok = 1, rc;
    int size, written;
    unsigned char* blob = NULL;

    if (!initialWorld || !inputs || tickCount < 0) return 0;
    if (resumeAtTick < 0 || resumeAtTick > tickCount) return 0;

    memset(&straight, 0, sizeof(straight));
    memset(&part,     0, sizeof(part));
    memset(&resumed,  0, sizeof(resumed));

    if (!F0880b_WORLD_Clone_Compat(initialWorld, &straight)) return 0;
    if (!F0880b_WORLD_Clone_Compat(initialWorld, &part)) {
        F0883_WORLD_Free_Compat(&straight); return 0;
    }

    /* Run straight through. */
    rc = F0885_ORCH_RunNTicks_Compat(&straight, inputs, tickCount, NULL, &hashA);
    if (rc < 0) { ok = 0; goto cleanup; }

    /* Run first K ticks on part, then serialise + deserialise + continue. */
    rc = F0885_ORCH_RunNTicks_Compat(&part, inputs, resumeAtTick, NULL, NULL);
    if (rc < 0) { ok = 0; goto cleanup; }

    size = F0899_WORLD_SerializedSize_Compat(&part);
    if (size <= 0) { ok = 0; goto cleanup; }
    blob = (unsigned char*)malloc((size_t)size);
    if (!blob) { ok = 0; goto cleanup; }
    if (!F0897_WORLD_Serialize_Compat(&part, blob, size, &written)) { ok = 0; goto cleanup; }

    /* Deserialise into a fresh world — reuse the same dungeon pointer. */
    memset(&resumed, 0, sizeof(resumed));
    resumed.dungeon = part.dungeon;
    resumed.things = part.things;
    resumed.ownsDungeon = 0; /* never own when transplanted */
    if (!F0898_WORLD_Deserialize_Compat(&resumed, blob, size, NULL)) { ok = 0; goto cleanup; }

    rc = F0885_ORCH_RunNTicks_Compat(&resumed, inputs + resumeAtTick,
                                     tickCount - resumeAtTick, NULL, &hashC);
    if (rc < 0) { ok = 0; goto cleanup; }

    if (hashA != hashC) ok = 0;

cleanup:
    if (blob) free(blob);
    F0883_WORLD_Free_Compat(&straight);
    F0883_WORLD_Free_Compat(&part);
    /* resumed shares dungeon pointers; ownsDungeon=0 so free is safe */
    F0883_WORLD_Free_Compat(&resumed);
    return ok;
}

/* ================================================================
 *  Group A — Construct / Destruct / Clone
 * ================================================================ */

struct GameWorld_Compat* F0880_WORLD_AllocDefault_Compat(void) {
    struct GameWorld_Compat* w = (struct GameWorld_Compat*)calloc(1, sizeof(*w));
    return w;
}

static void init_save_header(struct GameWorld_Compat* w) {
    memset(&w->saveHeader, 0, sizeof(w->saveHeader));
    w->saveHeader.magic[0] = 'R'; w->saveHeader.magic[1] = 'D';
    w->saveHeader.magic[2] = 'M'; w->saveHeader.magic[3] = 'C';
    w->saveHeader.magic[4] = 'S'; w->saveHeader.magic[5] = 'B';
    w->saveHeader.magic[6] = '2'; w->saveHeader.magic[7] = '0';
    w->saveHeader.formatVersion = 1;
    w->saveHeader.endianSentinel = SAVEGAME_ENDIAN_SENTINEL;
    w->saveHeader.sectionCount = SAVEGAME_SECTION_COUNT;
}

int F0881_WORLD_InitDefault_Compat(struct GameWorld_Compat* world, uint32_t seed) {
    int i;
    if (!world) return 0;
    /* Preserve any dungeon pointers the caller set up before init. */
    {
        struct DungeonDatState_Compat* keep_dungeon = world->dungeon;
        struct DungeonThings_Compat* keep_things = world->things;
        int keep_owns = world->ownsDungeon;
        uint32_t keep_fp = world->dungeonFingerprint;
        memset(world, 0, sizeof(*world));
        world->dungeon = keep_dungeon;
        world->things = keep_things;
        world->ownsDungeon = keep_owns;
        world->dungeonFingerprint = keep_fp;
    }

    world->gameTick = 0;
    world->partyDead = 0;
    world->gameWon = 0;
    world->partyMapIndex = 0;
    world->newPartyMapIndex = -1;
    world->partyIsResting = 0;
    world->freezeLifeTicks = 0;
    world->disabledMovementTicks = 0;
    world->projectileDisabledMovementTicks = 0;
    world->lastProjectileDisabledMovementDirection = 0;

    F0720_TIMELINE_Init_Compat(&world->timeline, 0);
    F0730_COMBAT_RngInit_Compat(&world->masterRng, seed ? seed : 1u);
    memset(&world->party, 0, sizeof(world->party));
    for (i = 0; i < CHAMPION_MAX_PARTY; i++)
        F0600_CHAMPION_InitEmpty_Compat(&world->party.champions[i]);
    world->party.activeChampionIndex = -1;

    memset(&world->pendingCombat, 0, sizeof(world->pendingCombat));
    memset(&world->magic, 0, sizeof(world->magic));
    memset(&world->dungeonMutations, 0, sizeof(world->dungeonMutations));
    memset(&world->pendingSensorEffects, 0, sizeof(world->pendingSensorEffects));
    memset(&world->projectiles, 0, sizeof(world->projectiles));
    memset(&world->explosions, 0, sizeof(world->explosions));
    for (i = 0; i < GAMEWORLD_CREATURE_AI_CAPACITY; i++)
        memset(&world->creatureAI[i], 0, sizeof(world->creatureAI[i]));
    world->creatureAICount = 0;
    memset(&world->lifecycle, 0, sizeof(world->lifecycle));
    F0859_LIFECYCLE_Init_Compat(&world->lifecycle, &world->party);

    init_save_header(world);
    return 1;
}

int F0882_WORLD_InitFromDungeonDat_Compat(
    const char* dungeonPath,
    uint32_t seed,
    struct GameWorld_Compat* outWorld)
{
    struct DungeonDatState_Compat* dungeon = NULL;
    struct DungeonThings_Compat* things = NULL;
    struct TimelineEvent_Compat ev;
    int direction = 0, py = 0, px = 0;

    if (!dungeonPath || !outWorld) return 0;

    dungeon = (struct DungeonDatState_Compat*)calloc(1, sizeof(*dungeon));
    things = (struct DungeonThings_Compat*)calloc(1, sizeof(*things));
    if (!dungeon || !things) goto fail;

    if (!F0500_DUNGEON_LoadDatHeader_Compat(dungeonPath, dungeon)) goto fail;
    { const char* lp = dungeon->decompressedPath[0] ? dungeon->decompressedPath : dungeonPath; if (!F0502_DUNGEON_LoadTileData_Compat(lp, dungeon)) goto fail;
    if (!F0504_DUNGEON_LoadThingData_Compat(lp, dungeon, things)) goto fail; }

    /* DUN-05 (audit, v2.7.x): surface BUG0_08 overfill divergence
     * by emitting a one-shot warning if the dungeon has more
     * thing-bearing squares than the SFT buffer can hold. Defensive
     * behaviour is preserved; the warning makes the divergence
     * observable. Ref: ReDMCSB DUNGEON.C:F0163_DUNGEON_LinkThingToList. */
    (void)F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat(dungeon, things);

    /* Set up the world to own these. */
    memset(outWorld, 0, sizeof(*outWorld));
    outWorld->dungeon = dungeon;
    outWorld->things = things;
    outWorld->ownsDungeon = 1;
    outWorld->dungeonFingerprint = dungeon_file_fingerprint(dungeonPath);
    if (!F0881_WORLD_InitDefault_Compat(outWorld, seed)) goto fail;

    /* Initial party location from dungeon header. */
    F0501_DUNGEON_DecodePartyLocation_Compat(
        dungeon->header.initialPartyLocation, &direction, &py, &px);
    outWorld->party.mapIndex = 0;
    outWorld->party.mapX = px;
    outWorld->party.mapY = py;
    set_party_direction_redmcsb_compat(&outWorld->party, direction);
    outWorld->partyMapIndex = 0;

    /* Schedule an initial watchdog / generator-placeholder event at tick 1
     * so the timeline is non-empty at init (invariant C14). */
    memset(&ev, 0, sizeof(ev));
    ev.kind = TIMELINE_EVENT_WATCHDOG;
    ev.fireAtTick = 1;
    ev.mapIndex = 0;
    F0721_TIMELINE_Schedule_Compat(&outWorld->timeline, &ev);

    return 1;

fail:
    if (dungeon) {
        F0502_DUNGEON_FreeTileData_Compat(dungeon);
        F0500_DUNGEON_FreeDatHeader_Compat(dungeon);
        free(dungeon);
    }
    if (things) {
        F0504_DUNGEON_FreeThingData_Compat(things);
        free(things);
    }
    return 0;
}

void F0883_WORLD_Free_Compat(struct GameWorld_Compat* world) {
    if (!world) return;
    if (world->ownsDungeon) {
        if (world->things) {
            F0504_DUNGEON_FreeThingData_Compat(world->things);
            free(world->things);
        }
        if (world->dungeon) {
            F0502_DUNGEON_FreeTileData_Compat(world->dungeon);
            F0500_DUNGEON_FreeDatHeader_Compat(world->dungeon);
            free(world->dungeon);
        }
    }
    world->dungeon = NULL;
    world->things = NULL;
    world->ownsDungeon = 0;
}

int F0880b_WORLD_Clone_Compat(
    const struct GameWorld_Compat* src,
    struct GameWorld_Compat* dst)
{
    if (!src || !dst) return 0;
    /* Structural copy: pointer fields share ownership with src (dst
     * must set ownsDungeon=0 so we don't double-free). */
    memcpy(dst, src, sizeof(*src));
    dst->ownsDungeon = 0;
    return 1;
}

/* ================================================================
 *  Group B — Tick Orchestrator
 * ================================================================ */

/* Translate a player command to a movement action (0..5) and/or
 * party-direction change. Returns movement-action or -1 if not a
 * movement command. */

static const unsigned short s_dm1_i34_creature_attributes[27] = {
    0x0482, 0x0480, 0x4510, 0x04B4, 0x0701, 0x0581, 0x070C,
    0x0300, 0x5864, 0x0282, 0x1480, 0x18C6, 0x1280, 0x14A2,
    0x05B8, 0x0381, 0x0680, 0x04A0, 0x0280, 0x4060, 0x10DE,
    0x0082, 0x1480, 0x78AA, 0x068A, 0x78AA, 0x78AA
};

static unsigned short orch_next_thing_compat(
    const struct DungeonThings_Compat* things,
    unsigned short thing)
{
    int type;
    int index;
    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) return THING_NONE;
    type = THING_GET_TYPE(thing);
    index = THING_GET_INDEX(thing);
    if (index < 0) return THING_NONE;
    switch (type) {
    case THING_TYPE_DOOR:
        return (index < things->doorCount) ? things->doors[index].next : THING_NONE;
    case THING_TYPE_TELEPORTER:
        return (index < things->teleporterCount) ? things->teleporters[index].next : THING_NONE;
    case THING_TYPE_TEXTSTRING:
        return (index < things->textStringCount) ? things->textStrings[index].next : THING_NONE;
    case THING_TYPE_SENSOR:
        return (index < things->sensorCount) ? things->sensors[index].next : THING_NONE;
    case THING_TYPE_GROUP:
        return (index < things->groupCount) ? things->groups[index].next : THING_NONE;
    case THING_TYPE_WEAPON:
        return (index < things->weaponCount) ? things->weapons[index].next : THING_NONE;
    case THING_TYPE_ARMOUR:
        return (index < things->armourCount) ? things->armours[index].next : THING_NONE;
    case THING_TYPE_SCROLL:
        return (index < things->scrollCount) ? things->scrolls[index].next : THING_NONE;
    case THING_TYPE_POTION:
        return (index < things->potionCount) ? things->potions[index].next : THING_NONE;
    case THING_TYPE_CONTAINER:
        return (index < things->containerCount) ? things->containers[index].next : THING_NONE;
    case THING_TYPE_JUNK:
        return (index < things->junkCount) ? things->junks[index].next : THING_NONE;
    case THING_TYPE_PROJECTILE:
        return (index < things->projectileCount) ? things->projectiles[index].next : THING_NONE;
    case THING_TYPE_EXPLOSION:
        return (index < things->explosionCount) ? things->explosions[index].next : THING_NONE;
    default:
        return THING_NONE;
    }
}

static void orch_set_thing_next_compat(
    struct DungeonThings_Compat* things,
    unsigned short thing,
    unsigned short next)
{
    int type;
    int index;
    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) return;
    type = THING_GET_TYPE(thing);
    index = THING_GET_INDEX(thing);
    if (index < 0) return;
    switch (type) {
    case THING_TYPE_DOOR:
        if (things->doors && index < things->doorCount) things->doors[index].next = next;
        break;
    case THING_TYPE_TELEPORTER:
        if (things->teleporters && index < things->teleporterCount) things->teleporters[index].next = next;
        break;
    case THING_TYPE_TEXTSTRING:
        if (things->textStrings && index < things->textStringCount) things->textStrings[index].next = next;
        break;
    case THING_TYPE_SENSOR:
        if (things->sensors && index < things->sensorCount) things->sensors[index].next = next;
        break;
    case THING_TYPE_GROUP:
        if (things->groups && index < things->groupCount) things->groups[index].next = next;
        break;
    case THING_TYPE_WEAPON:
        if (things->weapons && index < things->weaponCount) things->weapons[index].next = next;
        break;
    case THING_TYPE_ARMOUR:
        if (things->armours && index < things->armourCount) things->armours[index].next = next;
        break;
    case THING_TYPE_SCROLL:
        if (things->scrolls && index < things->scrollCount) things->scrolls[index].next = next;
        break;
    case THING_TYPE_POTION:
        if (things->potions && index < things->potionCount) things->potions[index].next = next;
        break;
    case THING_TYPE_CONTAINER:
        if (things->containers && index < things->containerCount) things->containers[index].next = next;
        break;
    case THING_TYPE_JUNK:
        if (things->junks && index < things->junkCount) things->junks[index].next = next;
        break;
    case THING_TYPE_PROJECTILE:
        if (things->projectiles && index < things->projectileCount) things->projectiles[index].next = next;
        break;
    case THING_TYPE_EXPLOSION:
        if (things->explosions && index < things->explosionCount) things->explosions[index].next = next;
        break;
    default:
        break;
    }
}

static void orch_write_raw_next_compat(
    struct DungeonThings_Compat* things,
    unsigned short thing)
{
    int type;
    int index;
    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) return;
    type = THING_GET_TYPE(thing);
    index = THING_GET_INDEX(thing);
    if (type < 0 || type >= 16 || index < 0 ||
        index >= things->thingCounts[type] || !things->rawThingData[type] ||
        s_orch_thing_data_byte_count[type] < 2) {
        return;
    }
    w_u16(things->rawThingData[type] +
          (index * s_orch_thing_data_byte_count[type]),
          orch_next_thing_compat(things, thing));
}

static void orch_write_raw_group_compat(
    struct DungeonThings_Compat* things,
    int groupIndex)
{
    struct DungeonGroup_Compat* group;
    unsigned char* raw;
    uint16_t bitfield;
    if (!things || !things->groups || groupIndex < 0 ||
        groupIndex >= things->groupCount ||
        groupIndex >= things->thingCounts[THING_TYPE_GROUP] ||
        !things->rawThingData[THING_TYPE_GROUP]) {
        return;
    }
    group = &things->groups[groupIndex];
    raw = things->rawThingData[THING_TYPE_GROUP] + (groupIndex * 16);
    bitfield = (uint16_t)(raw[14] | ((uint16_t)raw[15] << 8));
    bitfield = (uint16_t)((bitfield & 0xf890u) |
                          ((uint16_t)(group->behavior & 0x0Fu)) |
                          ((uint16_t)(group->count & 0x03u) << 5) |
                          ((uint16_t)(group->direction & 0x03u) << 8) |
                          ((uint16_t)(group->doNotDiscard & 0x01u) << 10));
    w_u16(raw + 0, group->next);
    w_u16(raw + 2, group->slot);
    raw[4] = group->creatureType;
    raw[5] = group->cells;
    w_u16(raw + 6, group->health[0]);
    w_u16(raw + 8, group->health[1]);
    w_u16(raw + 10, group->health[2]);
    w_u16(raw + 12, group->health[3]);
    w_u16(raw + 14, bitfield);
}

static void orch_write_raw_weapon_compat(
    struct DungeonThings_Compat* things,
    int weaponIndex)
{
    struct DungeonWeapon_Compat* weapon;
    unsigned char* raw;
    uint16_t bitfield;
    if (!things || !things->weapons || weaponIndex < 0 ||
        weaponIndex >= things->weaponCount ||
        weaponIndex >= things->thingCounts[THING_TYPE_WEAPON] ||
        !things->rawThingData[THING_TYPE_WEAPON]) {
        return;
    }
    weapon = &things->weapons[weaponIndex];
    raw = things->rawThingData[THING_TYPE_WEAPON] + (weaponIndex * 4);
    bitfield = (uint16_t)(((uint16_t)(weapon->type & 0x7Fu)) |
                          ((uint16_t)(weapon->doNotDiscard & 0x01u) << 7) |
                          ((uint16_t)(weapon->cursed & 0x01u) << 8) |
                          ((uint16_t)(weapon->poisoned & 0x01u) << 9) |
                          ((uint16_t)(weapon->chargeCount & 0x0Fu) << 10) |
                          ((uint16_t)(weapon->broken & 0x01u) << 14) |
                          ((uint16_t)(weapon->lit & 0x01u) << 15));
    w_u16(raw + 0, weapon->next);
    w_u16(raw + 2, bitfield);
}

static int orch_get_lit_torch_weapon_index_compat(
    const struct DungeonThings_Compat* things,
    unsigned short thing)
{
    int weaponIndex;
    const struct DungeonWeapon_Compat* weapon;
    if (!things || !things->weapons ||
        thing == THING_NONE || thing == THING_ENDOFLIST ||
        THING_GET_TYPE(thing) != THING_TYPE_WEAPON) {
        return -1;
    }
    weaponIndex = (int)THING_GET_INDEX(thing);
    if (weaponIndex < 0 || weaponIndex >= things->weaponCount) return -1;
    weapon = &things->weapons[weaponIndex];
    if (weapon->type != ORCH_WEAPON_TORCH_PC34 || !weapon->lit) return -1;
    return weaponIndex;
}

static int orch_decrease_torches_light_power_f0338_compat(
    struct GameWorld_Compat* world)
{
    int championIndex;
    int changed = 0;
    static const int handSlots[2] = {
        CHAMPION_SLOT_ACTION_HAND,
        CHAMPION_SLOT_HAND_LEFT
    };
    if (!world || !world->things) return 0;
    for (championIndex = 0;
         championIndex < world->party.championCount &&
             championIndex < CHAMPION_MAX_PARTY;
         championIndex++) {
        struct ChampionState_Compat* champion =
            &world->party.champions[championIndex];
        int slotOrdinal;
        for (slotOrdinal = 0; slotOrdinal < 2; slotOrdinal++) {
            int weaponIndex = orch_get_lit_torch_weapon_index_compat(
                world->things, champion->inventory[handSlots[slotOrdinal]]);
            struct DungeonWeapon_Compat* weapon;
            if (weaponIndex < 0) continue;
            weapon = &world->things->weapons[weaponIndex];
            if (weapon->chargeCount <= 0) continue;
            weapon->chargeCount--;
            if (weapon->chargeCount == 0) {
                weapon->doNotDiscard = 0;
            }
            orch_write_raw_weapon_compat(world->things, weaponIndex);
            changed++;
        }
    }
    return changed;
}

int F0890b_ORCH_ComputeDungeonViewLight_Compat(
    const struct GameWorld_Compat* world,
    struct DungeonViewLight_Compat* outLight)
{
    int i, j;
    int multiplier = 6;
    int totalLight = 0;
    if (!outLight) return 0;
    memset(outLight, 0, sizeof(*outLight));
    outLight->paletteIndex = 5;
    if (!world) return 0;

    if (world->dungeon &&
        world->party.mapIndex >= 0 &&
        world->party.mapIndex < (int)world->dungeon->header.mapCount &&
        world->dungeon->maps[world->party.mapIndex].difficulty == 0) {
        /* ReDMCSB: PANEL.C F0337 lines 367-372 forces maps with
         * Difficulty == 0 to the brightest dungeon-view palette. */
        outLight->paletteIndex = 0;
        outLight->refreshPaletteRequested = 1;
        outLight->forcedBrightMap = 1;
        return 1;
    }

    /* ReDMCSB: PANEL.C F0337 lines 373-386 inspects two hand slots for
     * all four champion records, even when fewer party members are live. */
    for (i = 0; i < CHAMPION_MAX_PARTY; i++) {
        static const int handSlots[2] = {
            CHAMPION_SLOT_ACTION_HAND,
            CHAMPION_SLOT_HAND_LEFT
        };
        const struct ChampionState_Compat* champion =
            &world->party.champions[i];
        for (j = 0; j < 2; j++) {
            int slotOrdinal = (i * 2) + j;
            int weaponIndex = orch_get_lit_torch_weapon_index_compat(
                world->things, champion->inventory[handSlots[j]]);
            if (weaponIndex >= 0) {
                int power = world->things->weapons[weaponIndex].chargeCount;
                if (power < 0) power = 0;
                if (power > 15) power = 15;
                outLight->torchLightPower[slotOrdinal] = power;
                if (power > 0) outLight->litTorchCount++;
            }
        }
    }

    /* ReDMCSB: PANEL.C F0337 lines 388-404 selection-sorts only the
     * first four entries; the fifth summed torch may be any lower slot. */
    for (i = 0; i < 4; i++) {
        for (j = i + 1; j < 8; j++) {
            if (outLight->torchLightPower[j] > outLight->torchLightPower[i]) {
                int tmp = outLight->torchLightPower[j];
                outLight->torchLightPower[j] = outLight->torchLightPower[i];
                outLight->torchLightPower[i] = tmp;
            }
        }
    }

    for (i = 0; i < 5; i++) {
        int power = outLight->torchLightPower[i];
        if (power > 0) {
            totalLight += (s_orch_light_power_to_amount_pc34[power] << multiplier) >> 6;
            if (multiplier > 0) multiplier--;
        }
    }
    totalLight += world->magic.magicalLightAmount;
    outLight->totalLightAmount = totalLight;

    if (totalLight > 0) {
        int paletteIndex = 0;
        while (paletteIndex < 5 &&
               s_orch_palette_index_to_light_amount_pc34[paletteIndex] > totalLight) {
            paletteIndex++;
        }
        outLight->paletteIndex = paletteIndex;
    } else {
        outLight->paletteIndex = 5;
    }
    outLight->refreshPaletteRequested = 1;
    return 1;
}

static void orch_write_raw_armour_compat(
    struct DungeonThings_Compat* things,
    int armourIndex)
{
    struct DungeonArmour_Compat* armour;
    unsigned char* raw;
    uint16_t bitfield;
    if (!things || !things->armours || armourIndex < 0 ||
        armourIndex >= things->armourCount ||
        armourIndex >= things->thingCounts[THING_TYPE_ARMOUR] ||
        !things->rawThingData[THING_TYPE_ARMOUR]) {
        return;
    }
    armour = &things->armours[armourIndex];
    raw = things->rawThingData[THING_TYPE_ARMOUR] + (armourIndex * 4);
    bitfield = (uint16_t)(((uint16_t)(armour->type & 0x7Fu)) |
                          ((uint16_t)(armour->doNotDiscard & 0x01u) << 7) |
                          ((uint16_t)(armour->cursed & 0x01u) << 8) |
                          ((uint16_t)(armour->chargeCount & 0x0Fu) << 9) |
                          ((uint16_t)(armour->broken & 0x01u) << 13));
    w_u16(raw + 0, armour->next);
    w_u16(raw + 2, bitfield);
}

static void orch_write_raw_junk_compat(
    struct DungeonThings_Compat* things,
    int junkIndex)
{
    struct DungeonJunk_Compat* junk;
    unsigned char* raw;
    uint16_t bitfield;
    if (!things || !things->junks || junkIndex < 0 ||
        junkIndex >= things->junkCount ||
        junkIndex >= things->thingCounts[THING_TYPE_JUNK] ||
        !things->rawThingData[THING_TYPE_JUNK]) {
        return;
    }
    junk = &things->junks[junkIndex];
    raw = things->rawThingData[THING_TYPE_JUNK] + (junkIndex * 4);
    bitfield = (uint16_t)(((uint16_t)(junk->type & 0x7Fu)) |
                          ((uint16_t)(junk->doNotDiscard & 0x01u) << 7) |
                          ((uint16_t)(junk->cursed & 0x01u) << 8) |
                          ((uint16_t)(junk->chargeCount & 0x03u) << 14));
    w_u16(raw + 0, junk->next);
    w_u16(raw + 2, bitfield);
}

static int orch_thing_is_in_champion_inventory_compat(
    const struct GameWorld_Compat* world,
    unsigned short thing)
{
    int championIndex;
    int slotIndex;
    if (!world || thing == THING_NONE || thing == THING_ENDOFLIST) return 0;
    for (championIndex = 0; championIndex < CHAMPION_MAX_PARTY; ++championIndex) {
        const struct ChampionState_Compat* champion =
            &world->party.champions[championIndex];
        if (!champion->present) continue;
        for (slotIndex = 0; slotIndex < CHAMPION_SLOT_COUNT; ++slotIndex) {
            if (champion->inventory[slotIndex] == thing) return 1;
        }
    }
    return 0;
}

static int orch_thing_is_in_square_lists_compat(
    const struct DungeonThings_Compat* things,
    unsigned short needle)
{
    int listIndex;
    if (!things || !things->squareFirstThings ||
        needle == THING_NONE || needle == THING_ENDOFLIST) {
        return 0;
    }
    for (listIndex = 0; listIndex < things->squareFirstThingCount; ++listIndex) {
        unsigned short thing = things->squareFirstThings[listIndex];
        int safety = 0;
        while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
            if (thing == needle) return 1;
            thing = orch_next_thing_compat(things, thing);
        }
    }
    return 0;
}

static int orch_find_unused_junk_slot_f0166_compat(
    struct GameWorld_Compat* world,
    unsigned short* outThing)
{
    int i;
    if (!world || !world->things || !world->things->junks ||
        world->things->junkCount <= 0 || !outThing) {
        return 0;
    }
    for (i = 0; i < world->things->junkCount; ++i) {
        unsigned short thing = orch_make_thing_ref_compat(THING_TYPE_JUNK, i);
        unsigned short next = world->things->junks[i].next;
        if (next != THING_NONE && next != THING_ENDOFLIST) continue;
        if (orch_thing_is_in_champion_inventory_compat(world, thing)) continue;
        if (orch_thing_is_in_square_lists_compat(world->things, thing)) continue;
        *outThing = thing;
        return 1;
    }
    return 0;
}

static int orch_cmd_cast_spell_empty_flask_slot_compat(
    const struct TickInput_Compat* input)
{
    if (!input ||
        (input->reserved2 & CMD_CAST_SPELL_RESERVED2_HAS_EMPTY_FLASK) == 0u) {
        return -1;
    }
    return (int)((input->reserved2 &
                  CMD_CAST_SPELL_RESERVED2_EMPTY_FLASK_SLOT_MASK) >>
                 CMD_CAST_SPELL_RESERVED2_EMPTY_FLASK_SLOT_SHIFT);
}

static int orch_cmd_cast_spell_xp_compat(
    const struct TickInput_Compat* input,
    const struct SpellDefinition_Compat* spell,
    int powerOrdinal,
    struct RngState_Compat* rng)
{
    if (input &&
        (input->reserved2 & CMD_CAST_SPELL_RESERVED2_HAS_SPELL_XP) != 0u) {
        return (int)((input->reserved2 &
                      CMD_CAST_SPELL_RESERVED2_SPELL_XP_MASK) >>
                     CMD_CAST_SPELL_RESERVED2_SPELL_XP_SHIFT);
    }
    if (!spell) {
        return powerOrdinal > 0 ? powerOrdinal : 1;
    }
    /* ReDMCSB: MENU.C F0412 line 1826 draws RANDOM(8) before
     * computing L1273_ui_Experience. Direct M10 callers without the
     * M11 prevalidation handoff draw it here. */
    return (int)dm1_spell_experience(
        powerOrdinal,
        spell->baseRequiredSkillLevel,
        F0732_COMBAT_RngRandom_Compat(rng, 8));
}

static int orch_cmd_cast_spell_mutate_empty_flask_f0411_compat(
    struct GameWorld_Compat* world,
    int championIndex,
    int slotIndex,
    int potionType,
    int potionPower)
{
    struct ChampionState_Compat* champion;
    struct DungeonPotion_Compat* potion;
    unsigned short thing;
    int potionIndex;

    if (!world || !world->things || !world->things->potions) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    if (slotIndex < 0 || slotIndex >= CHAMPION_SLOT_COUNT) return 0;
    champion = &world->party.champions[championIndex];
    if (!champion->present) return 0;

    thing = champion->inventory[slotIndex];
    if (thing == THING_NONE || THING_GET_TYPE(thing) != THING_TYPE_POTION) {
        return 0;
    }
    potionIndex = (int)THING_GET_INDEX(thing);
    if (potionIndex < 0 || potionIndex >= world->things->potionCount) {
        return 0;
    }
    potion = &world->things->potions[potionIndex];
    if ((int)potion->type != ORCH_POTION_EMPTY_FLASK_PC34) {
        return 0;
    }

    /* ReDMCSB MENU.C F0411/F0412 lines 1721-1749 and 1845-1855:
     * potion spells mutate the empty-flask POTION object in hand. */
    potion->type = (unsigned char)(potionType & 0x7F);
    potion->power = (unsigned char)(potionPower & 0xFF);
    if (world->things->rawThingData[THING_TYPE_POTION] &&
        potionIndex < world->things->thingCounts[THING_TYPE_POTION]) {
        unsigned char* raw =
            world->things->rawThingData[THING_TYPE_POTION] + (potionIndex * 4);
        raw[2] = potion->power;
        raw[3] = (unsigned char)((potion->type & 0x7Fu) |
                                 (potion->doNotDiscard ? 0x80u : 0u));
    }
    return 1;
}

static int orch_link_thing_to_party_square_compat(
    struct GameWorld_Compat* world,
    unsigned short thing)
{
    int sftIndex;
    unsigned short current;
    int safety = 0;
    if (!world || !world->dungeon || !world->things ||
        !world->things->squareFirstThings) {
        return 0;
    }
    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, world->party.mapIndex, world->party.mapX,
        world->party.mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) {
        return 0;
    }
    current = world->things->squareFirstThings[sftIndex];
    if (current == THING_NONE || current == THING_ENDOFLIST) {
        world->things->squareFirstThings[sftIndex] = thing;
        orch_set_thing_next_compat(world->things, thing, THING_ENDOFLIST);
        orch_write_raw_next_compat(world->things, thing);
        return 1;
    }
    while (current != THING_NONE && current != THING_ENDOFLIST && safety++ < 64) {
        unsigned short next = orch_next_thing_compat(world->things, current);
        if (next == THING_NONE || next == THING_ENDOFLIST) {
            orch_set_thing_next_compat(world->things, current, thing);
            orch_set_thing_next_compat(world->things, thing, THING_ENDOFLIST);
            orch_write_raw_next_compat(world->things, current);
            orch_write_raw_next_compat(world->things, thing);
            return 1;
        }
        current = next;
    }
    return 0;
}

static int orch_cmd_cast_spell_materialize_zokathra_f0412_compat(
    struct GameWorld_Compat* world,
    int championIndex)
{
    struct ChampionState_Compat* champion;
    struct DungeonJunk_Compat* junk;
    unsigned short thing;
    int junkIndex;
    int slotIndex = -1;

    if (!world || !world->things || !world->things->junks) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champion = &world->party.champions[championIndex];
    if (!champion->present) return 0;
    if (!orch_find_unused_junk_slot_f0166_compat(world, &thing)) return 0;

    junkIndex = (int)THING_GET_INDEX(thing);
    if (junkIndex < 0 || junkIndex >= world->things->junkCount) return 0;
    junk = &world->things->junks[junkIndex];

    /* ReDMCSB MENU.C F0412 lines 1994-2025: Zokathra allocates an
     * unused JUNK thing, sets Type=C51_JUNK_ZOKATHRA, then tries ready
     * hand, action hand, and finally moves the object to the party square. */
    junk->next = THING_ENDOFLIST;
    junk->type = ORCH_JUNK_ZOKATHRA_PC34;
    junk->doNotDiscard = 0;
    junk->cursed = 0;
    junk->chargeCount = 0;
    orch_write_raw_junk_compat(world->things, junkIndex);

    if (champion->inventory[CHAMPION_SLOT_HAND_LEFT] == THING_NONE) {
        slotIndex = CHAMPION_SLOT_HAND_LEFT;
    } else if (champion->inventory[CHAMPION_SLOT_ACTION_HAND] == THING_NONE) {
        slotIndex = CHAMPION_SLOT_ACTION_HAND;
    }

    if (slotIndex >= 0) {
        champion->inventory[slotIndex] = thing;
        return 1;
    }

    return orch_link_thing_to_party_square_compat(world, thing);
}

static int orch_normalize_status_timeout_aux0_pc34_compat(int aux0)
{
    switch (aux0) {
        case TIMELINE_AUX_THIEVES_EYE:
            return LIFECYCLE_STATUS_THIEVES_EYE;
        case TIMELINE_AUX_INVISIBILITY:
            return LIFECYCLE_STATUS_INVISIBILITY;
        case TIMELINE_AUX_PARTY_SHIELD:
            return LIFECYCLE_STATUS_PARTY_SHIELD;
        case TIMELINE_AUX_FIRESHIELD:
            return LIFECYCLE_STATUS_FIRE_SHIELD;
        case TIMELINE_AUX_FOOTPRINTS:
            return LIFECYCLE_STATUS_FOOTPRINTS;
        case TIMELINE_AUX_SPELL_SHIELD:
            return LIFECYCLE_STATUS_SPELL_SHIELD;
        default:
            return aux0;
    }
}

static int orch_status_timeout_defense_pc34_compat(
    const struct TimelineEvent_Compat* ev,
    int normalizedStatus)
{
    if (!ev) return 0;
    if (normalizedStatus == LIFECYCLE_STATUS_PARTY_SHIELD &&
        ev->aux0 == TIMELINE_AUX_PARTY_SHIELD) {
        return ev->aux4;
    }
    return ev->aux1;
}

int F0888_ORCH_GetChampionF0303SkillLevel_Compat(
    const struct GameWorld_Compat* world,
    int championIndex,
    int skillIndex)
{
    const struct ChampionState_Compat* champion;
    const struct ChampionLifecycleState_Compat* lifecycleChampion;
    DM1_ChampionSkillState skillState;
    DM1_SkillLevelQuery query;
    int partyIsResting;
    int i;

    if (!world) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    if (skillIndex < 0 || skillIndex >= DM1_TOTAL_SKILL_COUNT) return 0;

    champion = &world->party.champions[championIndex];
    if (!champion->present) return 0;

    lifecycleChampion = &world->lifecycle.champions[championIndex];
    memset(&skillState, 0, sizeof(skillState));
    for (i = 0; i < DM1_TOTAL_SKILL_COUNT && i < LIFECYCLE_SKILL_COUNT; ++i) {
        skillState.skills[i].experience = lifecycleChampion->skills20[i].experience;
        skillState.skills[i].temporaryExperience =
            lifecycleChampion->skills20[i].temporaryExperience;
    }

    partyIsResting = world->partyIsResting || world->lifecycle.rest.isResting;
    if (!dm1_skill_build_query_from_champion_inventory(
            champion, world->things, partyIsResting, &query)) {
        return F0848_LIFECYCLE_ComputeSkillLevel_Compat(
            lifecycleChampion, skillIndex, 0);
    }

    /* ReDMCSB: COMMAND/MENU spell/action execution queries live levels via
     * CHAMPION.C F0303, which includes temporary XP, resting and equipped
     * action-hand/neck object modifiers. */
    return dm1_skill_get_level_ex(&skillState, skillIndex, 0, &query);
}

int F0888_ORCH_GetChampionF0312SkillBonus_Compat(
    const struct GameWorld_Compat* world,
    int championIndex,
    int weaponClass)
{
    int swingLevel;
    int throwLevel;
    int shootLevel;
    if (!world) return 0;
    /* ReDMCSB CHAMPION.C F0312 lines 1285-1296 asks F0303 for the
     * weapon-class-specific Swing/Throw/Shoot levels before adding
     * skillLevel << 1 to strength.  Keep the class rule in the DM1 combat
     * helper and make the orchestrator own the live F0303 lookup. */
    swingLevel = F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        world, championIndex, DM1_SKILL_IDX_SWING);
    throwLevel = F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        world, championIndex, DM1_SKILL_IDX_THROW);
    shootLevel = F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        world, championIndex, DM1_SKILL_IDX_SHOOT);
    return dm1_champion_f0312_skill_level_bonus_pc34(
        weaponClass, swingLevel, throwLevel, shootLevel);
}

int F0888_ORCH_GetChampionActionHandWeaponClass_Compat(
    const struct GameWorld_Compat* world,
    int championIndex)
{
    DM1_WeaponInfo info;
    if (F0888_ORCH_GetChampionActionHandWeaponInfo_Compat(
            world, championIndex, &info) <= 0) {
        return -1;
    }
    return info.weaponClass;
}

int F0888_ORCH_GetChampionActionHandWeaponInfo_Compat(
    const struct GameWorld_Compat* world,
    int championIndex,
    DM1_WeaponInfo* outInfo)
{
    const struct ChampionState_Compat* champion;
    unsigned short thing;
    int index;

    if (outInfo) {
        memset(outInfo, 0, sizeof(*outInfo));
        outInfo->weaponClass = -1;
    }
    if (!outInfo || !world || !world->things) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champion = &world->party.champions[championIndex];
    if (!champion->present) return 0;

    thing = champion->inventory[CHAMPION_SLOT_ACTION_HAND];
    if (thing == THING_NONE || thing == THING_ENDOFLIST) return 0;
    if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON) return 0;
    index = (int)THING_GET_INDEX(thing);
    if (index < 0 || index >= world->things->weaponCount || !world->things->weapons) {
        return 0;
    }

    /* ReDMCSB: CHAMPION.C F0312 line 1282 asks F0158_DUNGEON_GetWeaponInfo
     * for the live action-hand WEAPON.Type, then consumes WEAPON_INFO fields. */
    return dm1_weapon_info_pc34(world->things->weapons[index].type, outInfo) > 0;
}

int F0888_ORCH_GetCreatureSnapshot_Compat(
    const struct GameWorld_Compat* world,
    int groupIndex,
    int creatureIndex,
    int doubledMapDifficulty,
    struct CombatantCreatureSnapshot_Compat* outSnapshot)
{
    const struct DungeonGroup_Compat* group;
    const struct CreatureBehaviorProfile_Compat* profile;

    if (outSnapshot) {
        memset(outSnapshot, 0, sizeof(*outSnapshot));
        outSnapshot->creatureType = -1;
        outSnapshot->creatureIndex = -1;
    }
    if (!outSnapshot || !world || !world->things || !world->things->groups) return 0;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;
    if (creatureIndex < 0 || creatureIndex > 3) return 0;

    group = &world->things->groups[groupIndex];
    if (creatureIndex > (int)group->count) return 0;
    profile = CREATURE_GetProfile_Compat(group->creatureType);
    if (!profile) return 0;

    /* ReDMCSB: live group health comes from the active DUNGEON.C group slot,
     * while immutable creature stats come from the G0243-style creature
     * profile table already mirrored by memory_creature_ai_pc34_compat. */
    outSnapshot->creatureType = group->creatureType;
    outSnapshot->attack = profile->baseAttack;
    outSnapshot->defense = profile->baseDefense;
    outSnapshot->dexterity = profile->dexterity;
    outSnapshot->baseHealth = profile->baseHealth;
    outSnapshot->poisonAttack = profile->poisonAttack;
    outSnapshot->attackType = profile->attackType;
    outSnapshot->attributes = profile->attributes;
    outSnapshot->woundProbabilities = profile->woundProbabilities;
    outSnapshot->properties = profile->properties;
    outSnapshot->doubledMapDifficulty = doubledMapDifficulty;
    outSnapshot->creatureIndex = creatureIndex;
    outSnapshot->healthBefore = group->health[creatureIndex];
    outSnapshot->isCandidateInvulnerable =
        world->candidateAttackInvulnerableEnabled &&
        world->candidateAttackInvulnerableGroupIndex == groupIndex &&
        world->candidateAttackInvulnerableCreatureIndex == creatureIndex;
    return 1;
}

static int orch_f0312_stamina_adjusted_value_compat(
    const struct ChampionState_Compat* champion,
    int value)
{
    int currentStamina;
    int halfMaximumStamina;
    int halfValue;

    if (!champion) return value;
    currentStamina = (int)champion->stamina.current;
    halfMaximumStamina = (int)champion->stamina.maximum >> 1;
    if (halfMaximumStamina > 0 && currentStamina < halfMaximumStamina) {
        /* ReDMCSB CHAMPION.C F0306 lines 1094-1095: the first operand
         * halves P0641 before the second operand reuses that halved value. */
        halfValue = value >> 1;
        value = halfValue + (int)(((long)halfValue * (long)currentStamina) /
                                  (long)halfMaximumStamina);
    }
    return value;
}

struct OrchArmourInfoPc34 {
    unsigned char weight;
    unsigned char defense;
    unsigned char attributes;
};

static const struct OrchArmourInfoPc34 s_orch_dm1_armour_info_pc34[58] = {
    /* ReDMCSB DUNGEON.C G0239 lines 309-369: { Weight, Defense,
     * Attributes, Unreferenced }.  F0313 shield defense also asks F0312
     * for hand strength, and F0312 depends on held-object weight. */
    {   3,   5, 0x01 }, {   4,  10, 0x01 }, {   3,   4, 0x01 }, {   6,   5, 0x02 },
    {  16,  25, 0x04 }, {   4,   5, 0x00 }, {   4,   5, 0x00 }, {   3,   7, 0x01 },
    {   3,   7, 0x01 }, {   4,   6, 0x01 }, {   2,   4, 0x00 }, {   4,   5, 0x01 },
    {   5,   7, 0x01 }, {   3,  11, 0x02 }, {   3,  13, 0x02 }, {   4,  13, 0x02 },
    {   6,  17, 0x03 }, {   8,  20, 0x03 }, {  14,  20, 0x03 }, {   6,  12, 0x02 },
    {   5,   9, 0x01 }, {   5,   8, 0x01 }, {   5,   9, 0x01 }, {   4,   1, 0x04 },
    {   6,   5, 0x04 }, {  11,  12, 0x05 }, {  14,  17, 0x05 }, {  15,  20, 0x05 },
    {  11,  22, 0x85 }, {  10,  16, 0x82 }, {  14,  20, 0x83 }, {  21,  35, 0x84 },
    {  65,  35, 0x05 }, {  53,  35, 0x05 }, {  52,  70, 0x07 }, {  41,  55, 0x07 },
    {  16,  25, 0x06 }, {  16,  30, 0x06 }, {  19,  40, 0x07 }, { 120,  65, 0x04 },
    {  80,  56, 0x04 }, {  28,  37, 0x05 }, {  34,  56, 0x84 }, {  17,  62, 0x05 },
    { 108, 125, 0x04 }, {  72,  90, 0x04 }, {  24,  50, 0x05 }, {  30,  85, 0x84 },
    {  35,  76, 0x04 }, { 141, 160, 0x04 }, {  90, 101, 0x04 }, {  31,  60, 0x05 },
    {  40, 100, 0x84 }, {  14,  54, 0x06 }, {  57,  60, 0x07 }, {  81,  88, 0x04 },
    {   3,  16, 0x02 }, {   2,   3, 0x03 }
};

static int orch_dm1_armour_defense_f0143_compat(int armourType,
                                                 int useSharpDefense,
                                                 int* outDefense,
                                                 int* outIsShield,
                                                 int* outWeight)
{
    int defense;
    int attributes;
    if (!outDefense) return 0;
    *outDefense = 0;
    if (outIsShield) *outIsShield = 0;
    if (outWeight) *outWeight = 0;
    if (armourType < 0 ||
        armourType >= (int)(sizeof(s_orch_dm1_armour_info_pc34) /
                            sizeof(s_orch_dm1_armour_info_pc34[0]))) {
        return 0;
    }
    defense = (int)s_orch_dm1_armour_info_pc34[armourType].defense;
    attributes = (int)s_orch_dm1_armour_info_pc34[armourType].attributes;
    if (useSharpDefense) {
        /* ReDMCSB DUNGEON.C F0143 lines 1240-1244:
         * F0030_MAIN_GetScaledProduct(Defense, 3, sharp + 4). */
        defense = (defense * ((attributes & 0x07) + 4)) >> 3;
    }
    *outDefense = defense;
    if (outIsShield) *outIsShield = (attributes & 0x80) ? 1 : 0;
    if (outWeight) *outWeight = (int)s_orch_dm1_armour_info_pc34[armourType].weight;
    return 1;
}

static int orch_defender_inventory_slot_for_wound_index_compat(int woundIndex)
{
    switch (woundIndex) {
    case 0: return CHAMPION_SLOT_HAND_LEFT;    /* COMBAT_WOUND_READY_HAND */
    case 1: return CHAMPION_SLOT_HEAD;         /* COMBAT_WOUND_HEAD */
    case 2: return CHAMPION_SLOT_TORSO;        /* COMBAT_WOUND_TORSO */
    case 3: return CHAMPION_SLOT_ACTION_HAND;  /* COMBAT_WOUND_ACTION_HAND */
    case 4: return CHAMPION_SLOT_LEGS;         /* COMBAT_WOUND_LEGS */
    case 5: return CHAMPION_SLOT_FEET;         /* COMBAT_WOUND_FEET */
    default: return -1;
    }
}

static int orch_defender_armour_defense_for_thing_compat(
    const struct GameWorld_Compat* world,
    unsigned short thing,
    int useSharpDefense,
    int* outDefense,
    int* outIsShield,
    int* outWeight)
{
    int thingIndex;
    int armourType;

    if (outDefense) *outDefense = 0;
    if (outIsShield) *outIsShield = 0;
    if (outWeight) *outWeight = 0;
    if (!world || !world->things || !world->things->armours ||
        !outDefense) {
        return 0;
    }
    if (thing == THING_NONE || thing == THING_ENDOFLIST ||
        THING_GET_TYPE(thing) != THING_TYPE_ARMOUR) {
        return 0;
    }
    thingIndex = (int)THING_GET_INDEX(thing);
    if (thingIndex < 0 || thingIndex >= world->things->armourCount) {
        return 0;
    }
    armourType = (int)world->things->armours[thingIndex].type;
    return orch_dm1_armour_defense_f0143_compat(
        armourType, useSharpDefense, outDefense, outIsShield, outWeight);
}

static int orch_f0312_hand_strength_baseline_compat(
    const struct ChampionState_Compat* champion,
    int handWoundIndex,
    int objectWeight)
{
    int strength;
    int maxLoad;
    int oneSixteenthMaximumLoad;
    int loadThreshold;

    if (!champion) return 0;

    /* ReDMCSB CHAMPION.C F0312 lines 1264-1306 starts with RANDOM(16)
     * plus current strength, adjusts for held-object weight, stamina, and
     * hand wounds, then returns bounded strength >> 1.  M10 snapshots are
     * deterministic, so this caches the same non-random baseline only. */
    strength = (int)champion->attributes[CHAMPION_ATTR_STRENGTH];
    maxLoad = (int)champion->maxLoad;
    if (maxLoad <= 0) {
        maxLoad = (strength << 3) + 100;
    }
    oneSixteenthMaximumLoad = maxLoad >> 4;
    if (objectWeight <= oneSixteenthMaximumLoad) {
        strength += objectWeight - 12;
    } else {
        loadThreshold =
            oneSixteenthMaximumLoad + ((oneSixteenthMaximumLoad - 12) >> 1);
        if (objectWeight <= loadThreshold) {
            strength += (objectWeight - oneSixteenthMaximumLoad) >> 1;
        } else {
            strength -= (objectWeight - loadThreshold) << 1;
        }
    }
    strength = orch_f0312_stamina_adjusted_value_compat(champion, strength);
    if ((champion->wounds & (1u << handWoundIndex)) != 0) {
        strength >>= 1;
    }
    strength >>= 1;
    if (strength < 0) return 0;
    if (strength > 100) return 100;
    return strength;
}

static void orch_fill_defender_wound_defense_baseline_compat(
    const struct GameWorld_Compat* world,
    const struct ChampionState_Compat* champion,
    int useSharpDefense,
    struct CombatantChampionSnapshot_Compat* outChampion)
{
    static const int s_woundDefenseFactor[6] = { 5, 5, 4, 6, 3, 1 };
    int woundIndex;

    if (!world || !champion || !outChampion) return;

    for (woundIndex = 0; woundIndex < 6; ++woundIndex) {
        int inventorySlot;
        int bodyDefense = 0;
        int ignoredShield = 0;
        int baseline = 0;

        /* ReDMCSB CHAMPION.C F0313 lines 1336-1346: shields in
         * ready/action hands contribute F0312 hand strength plus armour
         * defense, weighted by the target wound slot. */
        {
            static const int s_handSlots[2] = {
                CHAMPION_SLOT_HAND_LEFT,
                CHAMPION_SLOT_ACTION_HAND
            };
            static const int s_handWoundIndexes[2] = { 0, 3 };
            int hand;
            for (hand = 0; hand < 2; ++hand) {
                int shieldDefense = 0;
                int isShield = 0;
                int shieldWeight = 0;
                if (orch_defender_armour_defense_for_thing_compat(
                        world, champion->inventory[s_handSlots[hand]],
                        useSharpDefense,
                        &shieldDefense, &isShield, &shieldWeight) &&
                    isShield) {
                    int handStrength =
                        orch_f0312_hand_strength_baseline_compat(
                            champion, s_handWoundIndexes[hand], shieldWeight);
                    baseline +=
                        ((handStrength + shieldDefense) *
                         s_woundDefenseFactor[woundIndex]) >>
                        ((s_handWoundIndexes[hand] == woundIndex) ? 4 : 5);
                }
            }
        }

        inventorySlot =
            orch_defender_inventory_slot_for_wound_index_compat(woundIndex);
        if (inventorySlot >= 0 && inventorySlot < CHAMPION_SLOT_COUNT &&
            woundIndex != 0 && woundIndex != 3 &&
            orch_defender_armour_defense_for_thing_compat(
                world, champion->inventory[inventorySlot], useSharpDefense,
                &bodyDefense, &ignoredShield, NULL)) {
            /* ReDMCSB CHAMPION.C F0313 lines 1355-1361 adds body-slot
             * armour defense for wound slots past the two hand slots. */
            baseline += bodyDefense;
        }
        outChampion->woundDefense[woundIndex] = baseline;
    }
}

static int orch_cmd_attack_f0312_strength_action_hand_compat(
    struct GameWorld_Compat* world,
    const struct ChampionState_Compat* champion,
    int championIndex,
    const DM1_WeaponInfo* weaponInfo,
    int hasActionHandWeapon)
{
    int strength;
    int objectWeight;
    int oneSixteenthMaximumLoad;
    int loadThreshold;
    int maxLoad;

    if (!world || !champion || !weaponInfo) return 0;

    /* ReDMCSB CHAMPION.C F0312 lines 1264-1299: action-hand melee strength
     * starts with RANDOM(16)+Strength, applies object-weight/load pressure,
     * then weapon strength, class skill bonus, stamina adjustment, wound
     * penalty and final bounded (strength >> 1). */
    strength = F0732_COMBAT_RngRandom_Compat(&world->masterRng, 16) +
        (int)champion->attributes[CHAMPION_ATTR_STRENGTH];
    objectWeight = hasActionHandWeapon ? weaponInfo->weight : 0;
    maxLoad = (int)champion->maxLoad;
    if (maxLoad <= 0) {
        maxLoad = ((int)champion->attributes[CHAMPION_ATTR_STRENGTH] << 3) + 100;
    }
    oneSixteenthMaximumLoad = maxLoad >> 4;
    if (objectWeight <= oneSixteenthMaximumLoad) {
        strength += objectWeight - 12;
    } else {
        loadThreshold = oneSixteenthMaximumLoad +
            ((oneSixteenthMaximumLoad - 12) >> 1);
        if (objectWeight <= loadThreshold) {
            strength += (objectWeight - oneSixteenthMaximumLoad) >> 1;
        } else {
            strength -= (objectWeight - loadThreshold) << 1;
        }
    }

    if (hasActionHandWeapon) {
        strength += weaponInfo->strength;
        strength += F0888_ORCH_GetChampionF0312SkillBonus_Compat(
            world, championIndex, weaponInfo->weaponClass) << 1;
    }

    strength = orch_f0312_stamina_adjusted_value_compat(champion, strength);
    if ((champion->wounds & COMBAT_WOUND_ACTION_HAND) != 0) {
        strength >>= 1;
    }
    strength >>= 1;
    if (strength < 0) strength = 0;
    if (strength > 100) strength = 100;
    return strength;
}

static int orch_build_cmd_attack_champion_snapshot_compat(
    struct GameWorld_Compat* world,
    int championIndex,
    const DM1_WeaponInfo* weaponInfo,
    int weaponType,
    int hasActionHandWeapon,
    int actionSkillIndex,
    struct CombatantChampionSnapshot_Compat* outChampion)
{
    const struct ChampionState_Compat* champion;

    if (!outChampion || !world || !weaponInfo) return 0;
    memset(outChampion, 0, sizeof(*outChampion));
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champion = &world->party.champions[championIndex];
    if (!champion->present || champion->hp.current == 0) return 0;

    if (actionSkillIndex < 0 || actionSkillIndex >= DM1_TOTAL_SKILL_COUNT) {
        if (weaponInfo->weaponClass >= DM1_WEAPON_CLASS_FIRST_BOW &&
            weaponInfo->weaponClass < DM1_WEAPON_CLASS_FIRST_MAGIC_WEAPON) {
            actionSkillIndex = DM1_SKILL_IDX_SHOOT;
        } else if (weaponInfo->weaponClass == 0) {
            actionSkillIndex = DM1_SKILL_IDX_SWING;
        } else {
            actionSkillIndex = DM1_SKILL_IDX_THROW;
        }
    }

    outChampion->championIndex = championIndex;
    outChampion->currentHealth = champion->hp.current;
    outChampion->dexterity = champion->attributes[CHAMPION_ATTR_DEXTERITY];
    outChampion->strengthActionHand =
        orch_cmd_attack_f0312_strength_action_hand_compat(
            world, champion, championIndex, weaponInfo, hasActionHandWeapon);
    outChampion->skillLevelParry = F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        world, championIndex, DM1_SKILL_IDX_PARRY);
    outChampion->skillLevelAction = F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        world, championIndex, actionSkillIndex);
    outChampion->statisticVitality = champion->attributes[CHAMPION_ATTR_VITALITY];
    outChampion->statisticAntifire = champion->attributes[CHAMPION_ATTR_ANTIFIRE];
    outChampion->statisticAntimagic = champion->attributes[CHAMPION_ATTR_ANTIMAGIC];
    outChampion->statisticWisdom = champion->attributes[CHAMPION_ATTR_WISDOM];
    outChampion->statisticLuck = (int)world->lifecycle.champions[championIndex]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT];
    outChampion->statisticLuckMax = (int)world->lifecycle.champions[championIndex]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_MAXIMUM];
    outChampion->statisticLuckMin = (int)world->lifecycle.champions[championIndex]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_MINIMUM];
    outChampion->actionHandIcon = weaponType;
    outChampion->wounds = champion->wounds;
    outChampion->isResting = world->partyIsResting || world->lifecycle.rest.isResting;
    outChampion->partyShieldDefense = champion->actionDefense;
    return 1;
}

static void orch_writeback_cmd_attack_luck_compat(
    struct GameWorld_Compat* world,
    int championIndex,
    const struct CombatantChampionSnapshot_Compat* championSnapshot)
{
    int luck;
    if (!world || !championSnapshot) return;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return;
    luck = championSnapshot->statisticLuck;
    if (luck < 0) luck = 0;
    if (luck > 255) luck = 255;
    world->lifecycle.champions[championIndex]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT] =
            (uint8_t)luck;
}

static int orch_build_defender_champion_snapshot_compat(
    const struct GameWorld_Compat* world,
    int championIndex,
    int attackType,
    struct CombatantChampionSnapshot_Compat* outChampion)
{
    const struct ChampionState_Compat* champion;

    if (!world || !outChampion) return 0;
    memset(outChampion, 0, sizeof(*outChampion));
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champion = &world->party.champions[championIndex];
    if (!champion->present || champion->hp.current == 0) return 0;

    outChampion->championIndex = championIndex;
    outChampion->currentHealth = champion->hp.current;
    outChampion->dexterity = champion->attributes[CHAMPION_ATTR_DEXTERITY];
    outChampion->statisticVitality = champion->attributes[CHAMPION_ATTR_VITALITY];
    outChampion->statisticAntifire = champion->attributes[CHAMPION_ATTR_ANTIFIRE];
    outChampion->statisticAntimagic = champion->attributes[CHAMPION_ATTR_ANTIMAGIC];
    outChampion->statisticWisdom = champion->attributes[CHAMPION_ATTR_WISDOM];
    outChampion->wounds = champion->wounds;
    orch_fill_defender_wound_defense_baseline_compat(
        world, champion, attackType == COMBAT_ATTACK_SHARP, outChampion);
    outChampion->isResting =
        world->partyIsResting || world->lifecycle.rest.isResting;

    /* ReDMCSB CHAMPION.C F0321 lines 1878-1888 subtracts the
     * attack-specific party spell/fire shield before the common
     * F0313 body-defense scale.  For physical/lightning projectile
     * paths, fold the champion action defense and party shield into
     * the deterministic F0313 snapshot field. */
    if (attackType == COMBAT_ATTACK_MAGIC) {
        outChampion->partyShieldDefense = world->magic.spellShieldDefense;
    } else if (attackType == COMBAT_ATTACK_FIRE) {
        outChampion->partyShieldDefense = world->magic.fireShieldDefense;
    } else {
        outChampion->partyShieldDefense =
            champion->actionDefense +
            world->lifecycle.champions[championIndex].shieldDefense +
            world->magic.partyShieldDefense;
    }
    return 1;
}

static void orch_build_cmd_attack_weapon_profile_compat(
    const DM1_WeaponInfo* weaponInfo,
    int weaponType,
    int actionIndex,
    int actionSkillIndex,
    struct WeaponProfile_Compat* outWeapon)
{
    int hitProbability = dm1_v1_graphic560_action_hit_probability_get_pc34(
        actionIndex);
    int damageFactor = dm1_v1_graphic560_action_damage_factor_get_pc34(
        actionIndex);

    if (hitProbability < 0 || damageFactor < 0) {
        actionIndex = CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;
        hitProbability = dm1_v1_graphic560_action_hit_probability_get_pc34(
            actionIndex);
        damageFactor = dm1_v1_graphic560_action_damage_factor_get_pc34(
            actionIndex);
    }

    memset(outWeapon, 0, sizeof(*outWeapon));
    outWeapon->weaponType = weaponType;
    outWeapon->weaponClass = weaponInfo->weaponClass;
    outWeapon->weaponStrength = weaponInfo->strength;
    outWeapon->kineticEnergy = weaponInfo->kineticEnergy;
    outWeapon->hitProbability = hitProbability;
    if (weaponType == COMBAT_ICON_VORPAL_BLADE ||
        actionIndex == DM1_ACTION_DISRUPT) {
        outWeapon->hitProbability |= 0x8000;
    }
    outWeapon->damageFactor = damageFactor;
    outWeapon->skillIndex = actionSkillIndex;
    outWeapon->attributes = weaponInfo->attributes;
}

static int orch_cmd_attack_action_index_compat(const struct TickInput_Compat* input)
{
    int actionIndex;
    if (!input) return CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;
    if ((input->reserved2 & CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID) == 0u) {
        return CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;
    }
    actionIndex = (int)(input->reserved2 & CMD_ATTACK_RESERVED2_ACTION_INDEX_MASK);
    if (dm1_v1_graphic560_action_damage_factor_get_pc34(actionIndex) < 0 ||
        dm1_v1_graphic560_action_hit_probability_get_pc34(actionIndex) < 0) {
        return CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;
    }
    return actionIndex;
}

static int orch_cmd_attack_has_live_action_index_compat(
    const struct TickInput_Compat* input)
{
    return input &&
        ((input->reserved2 & CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID) != 0u);
}

static int orch_cmd_attack_has_live_group_table_compat(
    const struct GameWorld_Compat* world)
{
    return world && world->things && world->things->groups &&
        world->things->groupCount > 0;
}

static int orch_cmd_attack_has_legacy_marker_compat(
    const struct TickInput_Compat* input)
{
    return input &&
        ((input->reserved2 & CMD_ATTACK_RESERVED2_LEGACY_MARKER_VALID) != 0u);
}

static int orch_cmd_attack_target_direction_compat(
    const struct GameWorld_Compat* world,
    const struct TickInput_Compat* input)
{
    int direction = world ? (world->party.direction & 3) : 0;
    if (input &&
        ((input->reserved2 & CMD_ATTACK_RESERVED2_TARGET_DIRECTION_VALID) != 0u)) {
        direction = (int)((input->reserved2 &
                           CMD_ATTACK_RESERVED2_TARGET_DIRECTION_MASK) >>
                          CMD_ATTACK_RESERVED2_TARGET_DIRECTION_SHIFT) & 3;
    }
    return direction;
}

static int orch_cmd_attack_action_hand_is_empty_compat(
    const struct GameWorld_Compat* world,
    int championIndex)
{
    unsigned short thing;
    if (!world) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    if (!world->party.champions[championIndex].present) return 0;
    thing = world->party.champions[championIndex]
        .inventory[CHAMPION_SLOT_ACTION_HAND];
    return thing == THING_NONE || thing == THING_ENDOFLIST;
}

static void orch_cmd_attack_empty_hand_weapon_info_compat(
    DM1_WeaponInfo* outInfo)
{
    if (!outInfo) return;
    memset(outInfo, 0, sizeof(*outInfo));
    outInfo->weaponClass = 255;
}

static int orch_cmd_attack_action_skill_index_compat(int actionIndex)
{
    DM1_ActionXpRoute route;
    if (!dm1_v1_action_xp_route(actionIndex, &route) || !route.valid) {
        if (!dm1_v1_action_xp_route(
                CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34, &route) ||
            !route.valid) {
            return -1;
        }
    }
    return route.skillIndex;
}

static int orch_cmd_attack_map_difficulty_compat(
    const struct GameWorld_Compat* world)
{
    int mapIndex;
    if (!world || !world->dungeon || !world->dungeon->maps) return 0;

    mapIndex = world->party.mapIndex;
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        mapIndex = world->partyMapIndex;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }

    return (int)world->dungeon->maps[mapIndex].difficulty;
}

static int orch_cmd_attack_doubled_map_difficulty_compat(
    const struct GameWorld_Compat* world)
{
    int difficulty = orch_cmd_attack_map_difficulty_compat(world);
    if (difficulty <= 0) return 0;

    /* ReDMCSB: PROJEXPL.C F0231 lines 1477-1491 builds
     * L0567_i_DoubledMapDifficulty as CurrentMap.Difficulty << 1 before
     * applying creature dexterity/defense in champion melee resolution. */
    return difficulty << 1;
}

static int orch_cmd_attack_creature_experience_compat(
    const struct CombatantCreatureSnapshot_Compat* creature)
{
    if (!creature) return 0;
    return (creature->properties >> 8) & 0x000F;
}

static int orch_unlink_thing_from_square_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned short thingToUnlink);

static void orch_remove_active_group_state_compat(
    struct GameWorld_Compat* world,
    int groupIndex);

static unsigned short orch_make_thing_ref_compat(int type, int index);

static int orch_group_creature_cell_compat(
    const struct DungeonGroup_Compat* group,
    int creatureIndex);
static int orch_ai_state_to_dm1_behavior_compat(int stateKind);

static void orch_cmd_attack_apply_f0231_side_effects_compat(
    struct GameWorld_Compat* world,
    int championIndex,
    int actionSkillIndex,
    const struct CombatantCreatureSnapshot_Compat* creature,
    int damageApplied)
{
    struct ChampionState_Compat* champion;
    int staminaCost;
    int pendingDamage;
    int16_t currentStamina;

    if (!world) return;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return;
    champion = &world->party.champions[championIndex];
    if (!champion->present || champion->hp.current <= 0) return;

    if (damageApplied > 0) {
        int creatureExperience = orch_cmd_attack_creature_experience_compat(creature);
        int experience = ((damageApplied * creatureExperience) >> 4) + 3;
        if (actionSkillIndex >= 0) {
            /* ReDMCSB: PROJEXPL.C F0231 lines ~1512-1514 awards
             * F0304 skill XP after damage, using M058_EXPERIENCE(Properties).
             * F0304 owns map difficulty and recent-combat modifiers. */
            (void)F0849_LIFECYCLE_AddSkillExperience_Compat(
                &world->lifecycle.champions[championIndex],
                actionSkillIndex,
                experience,
                orch_cmd_attack_map_difficulty_compat(world),
                world->gameTick,
                world->lifecycle.lastCreatureAttackTime,
                0,
                0);
        }
        staminaCost = F0732_COMBAT_RngRandom_Compat(&world->masterRng, 4) + 4;
    } else {
        staminaCost = F0732_COMBAT_RngRandom_Compat(&world->masterRng, 2) + 2;
    }

    currentStamina = (int16_t)champion->stamina.current;
    pendingDamage = dm1_needs_decrement_stamina(
        &currentStamina,
        champion->stamina.maximum,
        (int16_t)staminaCost);
    champion->stamina.current = (unsigned short)currentStamina;
    if (pendingDamage > 0) {
        int hp = (int)champion->hp.current - pendingDamage;
        champion->hp.current = (int16_t)((hp > 0) ? hp : 0);
    }
}

static void orch_cmd_attack_target_square_compat(
    const struct GameWorld_Compat* world,
    int direction,
    int* outMapIndex,
    int* outMapX,
    int* outMapY)
{
    int mapIndex = 0;
    int mapX = 0;
    int mapY = 0;
    if (world) {
        mapIndex = world->party.mapIndex;
        mapX = world->party.mapX;
        mapY = world->party.mapY;
        switch (direction & 3) {
            case DIR_NORTH: mapY--; break;
            case DIR_EAST:  mapX++; break;
            case DIR_SOUTH: mapY++; break;
            case DIR_WEST:  mapX--; break;
        }
    }
    if (outMapIndex) *outMapIndex = mapIndex;
    if (outMapX) *outMapX = mapX;
    if (outMapY) *outMapY = mapY;
}

static void orch_cmd_attack_schedule_f0231_reaction_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    const struct CombatantCreatureSnapshot_Compat* creature,
    int targetDirection,
    int outcome)
{
    struct TimelineEvent_Compat reaction;
    int mapIndex;
    int mapX;
    int mapY;

    if (!world || groupIndex < 0) return;
    if (outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES) return;

    orch_cmd_attack_target_square_compat(
        world, targetDirection, &mapIndex, &mapX, &mapY);
    memset(&reaction, 0, sizeof(reaction));
    reaction.kind = TIMELINE_EVENT_CREATURE_REACTION;
    reaction.fireAtTick = world->gameTick + 1u;
    reaction.mapIndex = mapIndex;
    reaction.mapX = mapX;
    reaction.mapY = mapY;
    reaction.aux0 = groupIndex;
    reaction.aux1 = creature ? creature->creatureType : -1;
    reaction.aux2 = DM1_EVENT_REACTION_PARTY_IS_ADJACENT;
    (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &reaction);
}

static void orch_cmd_attack_apply_group_kill_side_effects_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    int targetDirection,
    int outcome)
{
    int mapIndex;
    int mapX;
    int mapY;

    if (!world || !world->things || groupIndex < 0) return;
    if (outcome != COMBAT_OUTCOME_KILLED_ALL_CREATURES) return;

    orch_cmd_attack_target_square_compat(
        world, targetDirection, &mapIndex, &mapX, &mapY);
    (void)orch_unlink_thing_from_square_compat(
        world, mapIndex, mapX, mapY,
        orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex));
    if (groupIndex < world->things->groupCount && world->things->groups) {
        world->things->groups[groupIndex].next = THING_NONE;
    }
    orch_remove_active_group_state_compat(world, groupIndex);
}

static int orch_cmd_attack_f0190_smoke_attack_compat(
    const struct CombatantCreatureSnapshot_Compat* creature)
{
    int size;
    if (!creature) return 110;
    size = creature->attributes & DM1_ATTR_SIZE_MASK;
    if (size == DM1_SIZE_FULL_SQUARE) return 255;
    if (size == DM1_SIZE_HALF_SQUARE) return 190;
    return 110;
}

static void orch_cmd_attack_create_f0190_death_smoke_compat(
    struct GameWorld_Compat* world,
    const struct CombatantCreatureSnapshot_Compat* creature,
    int killedCell,
    int targetDirection,
    int outcome)
{
    struct ExplosionCreateInput_Compat create;
    struct TimelineEvent_Compat advance;
    int slotIndex = -1;
    int mapIndex;
    int mapX;
    int mapY;

    if (!world) return;
    if (outcome != COMBAT_OUTCOME_KILLED_SOME_CREATURES &&
        outcome != COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
        return;
    }

    orch_cmd_attack_target_square_compat(
        world, targetDirection, &mapIndex, &mapX, &mapY);
    memset(&create, 0, sizeof(create));
    create.explosionType = C040_EXPLOSION_SMOKE;
    create.attack = orch_cmd_attack_f0190_smoke_attack_compat(creature);
    create.mapIndex = mapIndex;
    create.mapX = mapX;
    create.mapY = mapY;
    create.cell = (killedCell == EXPLOSION_CELL_CENTERED)
        ? EXPLOSION_CELL_CENTERED : (killedCell & 3);
    create.centered = (create.cell == EXPLOSION_CELL_CENTERED) ? 1 : 0;
    create.currentTick = (int)world->gameTick;
    create.ownerKind = PROJECTILE_OWNER_CHAMPION;
    create.ownerIndex = -1;
    create.creatorProjectileSlot = -1;

    /* ReDMCSB: GROUP.C F0190 lines ~897-909 creates
     * C0xFFA8_THING_EXPLOSION_SMOKE with attack 110/190/255 from the
     * killed creature size, then PROJEXPL.C F0213 schedules its advance. */
    if (F0821_EXPLOSION_Create_Compat(
            &create, &world->explosions, &slotIndex, &advance)) {
        (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &advance);
    }
}

static void orch_create_f0190_death_smoke_at_square_compat(
    struct GameWorld_Compat* world,
    int creatureType,
    int killedCell,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct CreatureBehaviorProfile_Compat* profile;
    struct CombatantCreatureSnapshot_Compat creature;
    struct ExplosionCreateInput_Compat create;
    struct TimelineEvent_Compat advance;
    int slotIndex = -1;

    if (!world) return;
    profile = CREATURE_GetProfile_Compat(creatureType);
    memset(&creature, 0, sizeof(creature));
    creature.creatureType = creatureType;
    creature.attributes = profile ? profile->attributes : 0;

    memset(&create, 0, sizeof(create));
    create.explosionType = C040_EXPLOSION_SMOKE;
    create.attack = orch_cmd_attack_f0190_smoke_attack_compat(&creature);
    create.mapIndex = mapIndex;
    create.mapX = mapX;
    create.mapY = mapY;
    create.cell = (killedCell == EXPLOSION_CELL_CENTERED)
        ? EXPLOSION_CELL_CENTERED : (killedCell & 3);
    create.centered = (create.cell == EXPLOSION_CELL_CENTERED) ? 1 : 0;
    create.currentTick = (int)world->gameTick;
    create.ownerKind = PROJECTILE_OWNER_CHAMPION;
    create.ownerIndex = -1;
    create.creatorProjectileSlot = -1;

    if (F0821_EXPLOSION_Create_Compat(
            &create, &world->explosions, &slotIndex, &advance)) {
        (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &advance);
    }
}

static int orch_square_first_thing_list_index_compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY);

static int orch_cmd_attack_find_group_on_square_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    int* outGroupIndex)
{
    int sftIndex;
    unsigned short thing;
    int safety = 0;

    if (outGroupIndex) *outGroupIndex = -1;
    if (!world || !world->dungeon || !world->things ||
        !world->things->squareFirstThings) {
        return 0;
    }

    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) {
        return 0;
    }

    /* ReDMCSB: GROUP.C F0177 lines 123-147 starts from the group thing
     * on the target map square before selecting the creature ordinal. */
    thing = world->things->squareFirstThings[sftIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        if (type == THING_TYPE_GROUP &&
            index >= 0 && index < world->things->groupCount) {
            if (outGroupIndex) *outGroupIndex = index;
            return 1;
        }
        thing = orch_next_thing_compat(world->things, thing);
    }
    return 0;
}

static int orch_cmd_attack_first_living_creature_compat(
    const struct DungeonGroup_Compat* group)
{
    int i;
    if (!group) return -1;
    for (i = 0; i <= (int)group->count && i < 4; ++i) {
        if (group->health[i] > 0) return i;
    }
    return -1;
}

static int orch_cmd_attack_f0177_creature_slot_compat(
    const struct GameWorld_Compat* world,
    int championIndex,
    int groupIndex,
    int targetDirection)
{
    const struct DungeonGroup_Compat* group;
    DM1_CreatureGroup combatGroup;
    int i;

    if (!world || !world->things || !world->things->groups) return -1;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return -1;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return -1;

    group = &world->things->groups[groupIndex];
    if (group->cells == 0xFFu) {
        return orch_cmd_attack_first_living_creature_compat(group);
    }

    dm1_combat_init_group(&combatGroup);
    combatGroup.count = (int)group->count;
    if (combatGroup.count < 0) combatGroup.count = 0;
    if (combatGroup.count >= DM1_MAX_CREATURES_IN_GROUP) {
        combatGroup.count = DM1_MAX_CREATURES_IN_GROUP - 1;
    }
    for (i = 0; i <= combatGroup.count; ++i) {
        combatGroup.creatures[i].health = (int)group->health[i];
        combatGroup.creatures[i].cell = (int)((group->cells >> (i << 1)) & 0x03u);
        combatGroup.creatures[i].direction = (int)group->direction;
    }

    /* ReDMCSB: MENU.C F0402 lines 1024-1026 calls GROUP.C F0177 with the
     * champion Cell; F0177 then calls PROJEXPL.C F0229 for ordered cells. */
    return dm1_get_melee_target(
        &combatGroup,
        (int)(world->party.champions[championIndex].cell & 3),
        targetDirection,
        (int)group->direction);
}

static int orch_cmd_attack_champion_reach_blocked_f0407_compat(
    const struct GameWorld_Compat* world,
    int championIndex,
    int targetDirection)
{
    const struct ChampionState_Compat* champion;
    int relativeCell;
    int blockingCell = -1;
    int i;

    if (!world) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champion = &world->party.champions[championIndex];
    if (!champion->present || champion->hp.current == 0) return 0;

    relativeCell = ((int)(champion->cell & 3) + 4 - (targetDirection & 3)) & 3;
    if (relativeCell == 2) {
        blockingCell = ((int)(champion->cell & 3) + 3) & 3;
    } else if (relativeCell == 3) {
        blockingCell = ((int)(champion->cell & 3) + 1) & 3;
    } else {
        return 0;
    }

    /* ReDMCSB: MENU.C F0407 lines 1032-1041 rejects a back-row melee
     * action when another champion occupies the source front cell. */
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        const struct ChampionState_Compat* other = &world->party.champions[i];
        if (i == championIndex) continue;
        if (other->present && other->hp.current > 0 &&
            ((int)(other->cell & 3) == blockingCell)) {
            return 1;
        }
    }
    return 0;
}

static int orch_cmd_attack_disrupt_material_blocked_f0407_compat(
    const struct GameWorld_Compat* world,
    int actionIndex,
    int groupIndex)
{
    const struct CreatureBehaviorProfile_Compat* profile;
    int creatureType;

    if (!world || !world->things) return 0;
    if (actionIndex != DM1_ACTION_DISRUPT) return 0;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;

    creatureType = world->things->groups[groupIndex].creatureType;
    profile = CREATURE_GetProfile_Compat(creatureType);
    if (!profile) return 0;

    /* ReDMCSB: MENU.C F0407 lines 1042-1043 rejects DISRUPT before F0231
     * unless the target creature carries MASK0x0040_NON_MATERIAL. */
    return (profile->attributes & CREATURE_ATTR_MASK_NON_MATERIAL) == 0;
}

static int orch_cmd_attack_action_can_hit_door_f0407_compat(int actionIndex)
{
    switch (actionIndex) {
    case DM1_ACTION_CHOP:
    case DM1_ACTION_KICK:
    case DM1_ACTION_SWING:
    case DM1_ACTION_HACK:
    case DM1_ACTION_BERZERK:
    case DM1_ACTION_BASH:
        return 1;
    default:
        return 0;
    }
}

static int orch_cmd_attack_resolve_target_compat(
    const struct GameWorld_Compat* world,
    const struct TickInput_Compat* input,
    int* outGroupIndex,
    int* outCreatureIndex)
{
    int groupIndex;
    int creatureIndex;

    if (outGroupIndex) *outGroupIndex = -1;
    if (outCreatureIndex) *outCreatureIndex = -1;
    if (!world || !input || !world->things) return 0;

    groupIndex = (int)input->commandArg2;
    creatureIndex = (int)input->reserved;
    if (input->commandArg2 == CMD_ATTACK_TARGET_AUTO_GROUP_PC34) {
        int dx = 0;
        int dy = 0;
        int targetMapX;
        int targetMapY;
        int targetDirection = orch_cmd_attack_target_direction_compat(world, input);
        F0701_MOVEMENT_GetStepDelta_Compat(
            targetDirection, MOVE_FORWARD, &dx, &dy);
        targetMapX = world->party.mapX + dx;
        targetMapY = world->party.mapY + dy;
        if (!orch_cmd_attack_find_group_on_square_compat(
                world, world->party.mapIndex, targetMapX, targetMapY,
                &groupIndex)) {
            return 0;
        }
    }

    if (groupIndex < 0 || groupIndex >= world->things->groupCount ||
        !world->things->groups) {
        return 0;
    }
    if (input->reserved == CMD_ATTACK_CREATURE_AUTO_PC34) {
        creatureIndex = orch_cmd_attack_f0177_creature_slot_compat(
            world, (int)input->commandArg1, groupIndex,
            orch_cmd_attack_target_direction_compat(world, input));
        if (creatureIndex < 0 ||
            creatureIndex > (int)world->things->groups[groupIndex].count) {
            creatureIndex = orch_cmd_attack_first_living_creature_compat(
                &world->things->groups[groupIndex]);
        }
    } else if (
        creatureIndex < 0 || creatureIndex > (int)world->things->groups[groupIndex].count) {
        return 0;
    }
    if (creatureIndex < 0) return 0;

    if (outGroupIndex) *outGroupIndex = groupIndex;
    if (outCreatureIndex) *outCreatureIndex = creatureIndex;
    return 1;
}

static int orch_square_first_thing_list_index_compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat* map;
    int sftIndex = 0;
    int m;
    int squareIndex;
    if (!dungeon || !dungeon->tilesLoaded || !dungeon->tiles || !dungeon->maps) return -1;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return -1;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) return -1;
    for (m = 0; m < mapIndex; ++m) {
        int i;
        int count = dungeon->maps[m].width * dungeon->maps[m].height;
        if (!dungeon->tiles[m].squareData) return -1;
        for (i = 0; i < count; ++i) {
            if (dungeon->tiles[m].squareData[i] & DUNGEON_SQUARE_MASK_THING_LIST) ++sftIndex;
        }
    }
    squareIndex = mapX * map->height + mapY;
    if (!dungeon->tiles[mapIndex].squareData) return -1;
    {
        int i;
        for (i = 0; i < squareIndex; ++i) {
            if (dungeon->tiles[mapIndex].squareData[i] & DUNGEON_SQUARE_MASK_THING_LIST) ++sftIndex;
        }
    }
    return sftIndex;
}

static void orch_projectile_step_compat(int direction, int* dx, int* dy)
{
    if (!dx || !dy) return;
    switch (direction & 3) {
        case 0: *dx = 0;  *dy = -1; break;
        case 1: *dx = 1;  *dy = 0;  break;
        case 2: *dx = 0;  *dy = 1;  break;
        case 3: *dx = -1; *dy = 0;  break;
        default: *dx = 0; *dy = 0;  break;
    }
}

static int orch_read_square_byte_compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned char* outSquare)
{
    const struct DungeonMapDesc_Compat* map;
    const struct DungeonMapTiles_Compat* tiles;
    if (outSquare) *outSquare = 0;
    if (!dungeon || !dungeon->tilesLoaded || !dungeon->maps || !dungeon->tiles) return 0;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return 0;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= (int)map->width ||
        mapY < 0 || mapY >= (int)map->height) return 0;
    tiles = &dungeon->tiles[mapIndex];
    if (!tiles->squareData) return 0;
    if (outSquare) {
        *outSquare = tiles->squareData[(mapX * (int)map->height) + mapY];
    }
    return 1;
}

static int orch_cmd_attack_find_door_on_square_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    int* outDoorIndex)
{
    int sftIndex;
    unsigned short thing;
    int safety = 0;

    if (outDoorIndex) *outDoorIndex = -1;
    if (!world || !world->dungeon || !world->things ||
        !world->things->squareFirstThings || !world->things->doors) {
        return 0;
    }

    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) {
        return 0;
    }

    thing = world->things->squareFirstThings[sftIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        if (type == THING_TYPE_DOOR &&
            index >= 0 && index < world->things->doorCount) {
            if (outDoorIndex) *outDoorIndex = index;
            return 1;
        }
        thing = orch_next_thing_compat(world->things, thing);
    }
    return 0;
}

static int orch_cmd_attack_door_defense_pc34_compat(
    const struct GameWorld_Compat* world,
    const struct DungeonDoor_Compat* door)
{
    int doorSet;
    int doorInfoIndex;
    static const unsigned char s_i34_door_defense[4] = {
        110, 42, 230, 255
    };

    if (!door || !world || !world->dungeon || !world->dungeon->maps) {
        return 255;
    }
    if (world->party.mapIndex < 0 ||
        world->party.mapIndex >= (int)world->dungeon->header.mapCount) {
        return 255;
    }
    doorSet = door->type ? world->dungeon->maps[world->party.mapIndex].doorSet1
                         : world->dungeon->maps[world->party.mapIndex].doorSet0;
    doorInfoIndex = doorSet & 3;
    /* ReDMCSB DUNGEON.C G0254_as_Graphic559_DoorInfo lines 560-566:
     * portcullis=110, wooden=42, iron=230, Ra=255.  F0174 copies the
     * active map's two door sets into G0275 before F0232 compares Defense. */
    return (int)s_i34_door_defense[doorInfoIndex];
}

static int orch_set_door_state_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    int doorState)
{
    const struct DungeonMapDesc_Compat* map;
    struct DungeonMapTiles_Compat* tiles;
    int squareIndex;
    int squareByte;

    if (!world || !world->dungeon || !world->dungeon->maps ||
        !world->dungeon->tiles) {
        return 0;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= (int)map->width ||
        mapY < 0 || mapY >= (int)map->height) {
        return 0;
    }
    tiles = &world->dungeon->tiles[mapIndex];
    if (!tiles->squareData) return 0;

    squareIndex = (mapX * (int)map->height) + mapY;
    squareByte = (int)tiles->squareData[squareIndex];
    if (((squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5) !=
        DUNGEON_ELEMENT_DOOR) {
        return 0;
    }
    tiles->squareData[squareIndex] =
        (unsigned char)((squareByte & ~0x07) | (doorState & 0x07));
    return 1;
}

static int orch_cmd_attack_f0407_closed_door_compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* input,
    const DM1_WeaponInfo* weaponInfo,
    int hasActionHandWeapon,
    int actionIndex,
    struct TickResult_Compat* result)
{
    const struct DungeonMapDesc_Compat* map;
    struct DungeonMapTiles_Compat* tiles;
    struct DungeonDoor_Compat* door;
    const struct ChampionState_Compat* champion;
    int mapIndex;
    int mapX;
    int mapY;
    int squareIndex;
    int squareByte;
    int doorIndex = -1;
    int attack;
    int targetDirection;

    if (!world || !input || !weaponInfo || !world->dungeon ||
        !world->dungeon->tiles || !world->dungeon->maps) {
        return 0;
    }
    if (!orch_cmd_attack_action_can_hit_door_f0407_compat(actionIndex)) {
        return 0;
    }

    targetDirection = orch_cmd_attack_target_direction_compat(world, input);
    orch_cmd_attack_target_square_compat(
        world, targetDirection, &mapIndex, &mapX, &mapY);
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= (int)map->width ||
        mapY < 0 || mapY >= (int)map->height) {
        return 0;
    }
    tiles = &world->dungeon->tiles[mapIndex];
    if (!tiles->squareData) return 0;

    squareIndex = (mapX * (int)map->height) + mapY;
    squareByte = (int)tiles->squareData[squareIndex];
    if (((squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5) !=
        DUNGEON_ELEMENT_DOOR) {
        return 0;
    }
    if ((squareByte & 0x07) != 4) {
        return 0;
    }
    if (!orch_cmd_attack_find_door_on_square_compat(
            world, mapIndex, mapX, mapY, &doorIndex)) {
        return 1;
    }
    door = &world->things->doors[doorIndex];
    if ((int)input->commandArg1 < 0 ||
        (int)input->commandArg1 >= CHAMPION_MAX_PARTY) {
        return 1;
    }
    champion = &world->party.champions[(int)input->commandArg1];
    if (!champion->present || champion->hp.current == 0) {
        return 1;
    }

    /* ReDMCSB MENU.C F0407 lines 1268-1275 handles closed-door melee
     * before F0402/F0231 creature melee.  It calls F0312(action hand) and
     * GROUP.C/PROJEXPL.C F0232, then requests C04 wooden-thud one tick
     * later even when F0232 does not destroy the door. */
    {
        struct TimelineEvent_Compat thud;
        memset(&thud, 0, sizeof(thud));
        thud.kind = TIMELINE_EVENT_PLAY_SOUND;
        thud.fireAtTick = world->gameTick + 1u;
        thud.mapIndex = mapIndex;
        thud.mapX = mapX;
        thud.mapY = mapY;
        thud.aux0 = ORCH_SOUND_WOODEN_THUD_PC34;
        (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &thud);
    }

    if (!door->meleeDestructible) {
        return 1;
    }

    /* ReDMCSB PROJEXPL.C F0232 lines 1569-1593 destroys only a closed
     * melee-destructible door whose attack reaches the active door-set
     * defense. */
    attack = orch_cmd_attack_f0312_strength_action_hand_compat(
        world, champion, (int)input->commandArg1, weaponInfo,
        hasActionHandWeapon);
    if (attack >= orch_cmd_attack_door_defense_pc34_compat(world, door)) {
        struct TimelineEvent_Compat destruction;
        memset(&destruction, 0, sizeof(destruction));
        destruction.kind = TIMELINE_EVENT_DOOR_DESTRUCTION;
        destruction.fireAtTick = world->gameTick + 2u;
        destruction.mapIndex = mapIndex;
        destruction.mapX = mapX;
        destruction.mapY = mapY;
        destruction.aux0 = 5;
        (void)F0721_TIMELINE_Schedule_Compat(
            &world->timeline, &destruction);
    }
    return 1;
}

static int orch_reenable_generator_sensor_on_square_compat(
    const struct DungeonDatState_Compat* dungeon,
    struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY)
{
    int sftIndex;
    unsigned short thing;
    int safety = 0;

    if (!dungeon || !things || !things->loaded || !things->squareFirstThings) return 0;
    sftIndex = orch_square_first_thing_list_index_compat(dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= things->squareFirstThingCount) return 0;

    /* ReDMCSB TIMELINE.C:1009-1027, F0246: event C65 scans the
     * target square's thing list and changes the first disabled sensor
     * back to C006_SENSOR_FLOOR_GROUP_GENERATOR.  The full C05 trigger
     * and F0185 insertion path remain separate; this helper only owns
     * the source-locked C65 re-enable mutation. */
    thing = things->squareFirstThings[sftIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        if (type == THING_TYPE_SENSOR && index >= 0 && index < things->sensorCount) {
            struct DungeonSensor_Compat* sensor = &things->sensors[index];
            if (sensor->sensorType == RUNTIME_SENSOR_TYPE_DISABLED) {
                sensor->sensorType = RUNTIME_SENSOR_TYPE_FLOOR_GROUP_GENERATOR;
                return 1;
            }
        }
        thing = orch_next_thing_compat(things, thing);
    }
    return 0;
}

static int orch_find_generator_sensor_on_square_compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    int* outSensorIndex)
{
    int sftIndex;
    unsigned short thing;
    int safety = 0;

    if (outSensorIndex) *outSensorIndex = -1;
    if (!dungeon || !things || !things->loaded || !things->squareFirstThings) return 0;
    sftIndex = orch_square_first_thing_list_index_compat(dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= things->squareFirstThingCount) return 0;

    thing = things->squareFirstThings[sftIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        if (type == THING_TYPE_SENSOR && index >= 0 && index < things->sensorCount) {
            const struct DungeonSensor_Compat* sensor = &things->sensors[index];
            if (sensor->sensorType == RUNTIME_SENSOR_TYPE_FLOOR_GROUP_GENERATOR) {
                if (outSensorIndex) *outSensorIndex = index;
                return 1;
            }
        }
        thing = orch_next_thing_compat(things, thing);
    }
    return 0;
}

static unsigned short orch_make_thing_ref_compat(int type, int index) {
    return (unsigned short)(((type & 0x0F) << 10) | (index & 0x03FF));
}

static int orch_find_unused_group_slot_compat(
    const struct DungeonThings_Compat* things)
{
    int i;
    if (!things || !things->groups || things->groupCount <= 0) return -1;

    /* ReDMCSB DUNGEON.C:F0166:2077-2137 scans the fixed thing-data
     * array from index 0 and takes the first entry whose Next word is
     * C0xFFFF_THING_NONE.  GROUP.C:F0185:512-521 calls that allocator
     * before initializing a generated group, so C006 generation must
     * reuse an unused source slot instead of growing the group array. */
    for (i = 0; i < things->groupCount; ++i) {
        if (things->groups[i].next == THING_NONE) return i;
    }
    return -1;
}

static int orch_square_has_group_or_party_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY)
{
    int sftIndex;
    unsigned short thing;
    int safety = 0;

    if (!world || !world->dungeon || !world->things) return 0;
    if (world->partyMapIndex == mapIndex &&
        world->party.mapX == mapX && world->party.mapY == mapY) {
        return 1;
    }
    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;
    thing = world->things->squareFirstThings[sftIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        if (type == THING_TYPE_GROUP && index >= 0 && index < world->things->groupCount) {
            return 1;
        }
        thing = orch_next_thing_compat(world->things, thing);
    }
    return 0;
}

static int orch_find_teleporter_on_square_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    struct DungeonTeleporter_Compat* outTeleporter)
{
    int sftIndex;
    unsigned short thing;
    int safety = 0;

    if (!world || !world->dungeon || !world->things || !outTeleporter) return 0;
    if (!world->things->teleporters || world->things->teleporterCount <= 0) return 0;
    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;

    thing = world->things->squareFirstThings[sftIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        if (type == THING_TYPE_TELEPORTER &&
            index >= 0 && index < world->things->teleporterCount) {
            *outTeleporter = world->things->teleporters[index];
            return 1;
        }
        thing = orch_next_thing_compat(world->things, thing);
    }
    return 0;
}

static int orch_resolve_group_f0267_teleporter_destination_compat(
    const struct GameWorld_Compat* world,
    int* inOutMapIndex,
    int* inOutMapX,
    int* inOutMapY,
    struct OrchTeleporterBuzzList_Compat* outTeleporterBuzzes)
{
    int remaining;

    orch_teleporter_buzz_list_init_compat(outTeleporterBuzzes);
    if (!world || !world->dungeon || !inOutMapIndex || !inOutMapX || !inOutMapY) return 0;

    /* ReDMCSB MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE source lock:
     * - lines 453-454 choose MASK0x0001_SCOPE_CREATURES for group moves.
     * - lines 469-472 bound chained teleporter/pit moves (PC34 branch: 100).
     * - lines 474-492 require an open teleporter, creature scope, and then
     *   switch to TargetMapIndex/TargetMapX/TargetMapY.
     * - lines 520-524 request M560 at the target on each audible group teleporter hop.
     * This helper intentionally owns only the C006/F0185 and event60/61
     * group-teleporter destination subcase. GROUP.C:F0185:543 and
     * TIMELINE.C:F0252:1534 pass CM1_MAPX_NOT_ON_A_SQUARE, so the
     * MOVESENS.C:F0267:432-435 projectile-impact precheck is not entered
     * for generated/deferred insertion; group rotation remains outside it. */
    for (remaining = 100; remaining > 0; --remaining) {
        const struct DungeonMapDesc_Compat* map;
        unsigned char squareByte;
        int squareIndex;
        int squareType;
        struct DungeonTeleporter_Compat tp;
        int targetMapIndex;
        int destinationIsTeleporterTarget;

        if (*inOutMapIndex < 0 || *inOutMapIndex >= (int)world->dungeon->header.mapCount) break;
        map = &world->dungeon->maps[*inOutMapIndex];
        if (*inOutMapX < 0 || *inOutMapX >= map->width ||
            *inOutMapY < 0 || *inOutMapY >= map->height) break;
        if (!world->dungeon->tiles || !world->dungeon->tiles[*inOutMapIndex].squareData) break;

        squareIndex = (*inOutMapX * map->height) + *inOutMapY;
        squareByte = world->dungeon->tiles[*inOutMapIndex].squareData[squareIndex];
        squareType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
        if (squareType != DUNGEON_ELEMENT_TELEPORTER || !(squareByte & 0x08)) break;
        if (!orch_find_teleporter_on_square_compat(
                world, *inOutMapIndex, *inOutMapX, *inOutMapY, &tp)) break;
        if (!(tp.scope & 0x01)) break;

        destinationIsTeleporterTarget =
            (*inOutMapX == (int)tp.targetMapX &&
             *inOutMapY == (int)tp.targetMapY &&
             *inOutMapIndex == (int)tp.targetMapIndex);

        targetMapIndex = (int)tp.targetMapIndex;
        if (targetMapIndex < 0 || targetMapIndex >= (int)world->dungeon->header.mapCount) break;
        *inOutMapIndex = targetMapIndex;
        *inOutMapX = (int)tp.targetMapX;
        *inOutMapY = (int)tp.targetMapY;
        if (tp.audible) {
            orch_record_teleporter_buzz_compat(
                outTeleporterBuzzes, *inOutMapIndex, *inOutMapX, *inOutMapY);
        }
        if (destinationIsTeleporterTarget) break;
    }
    return 1;
}


static int orch_group_level_change_location_compat(
    const struct DungeonDatState_Compat* dungeon,
    int sourceMapIndex,
    int levelDelta,
    int* mapX,
    int* mapY)
{
    const struct DungeonMapDesc_Compat* sourceMap;
    int globalX;
    int globalY;
    int targetLevel;
    int i;

    if (!dungeon || !dungeon->maps || !mapX || !mapY ||
        sourceMapIndex < 0 || sourceMapIndex >= (int)dungeon->header.mapCount) {
        return -1;
    }

    sourceMap = &dungeon->maps[sourceMapIndex];
    globalX = (int)sourceMap->offsetMapX + *mapX;
    globalY = (int)sourceMap->offsetMapY + *mapY;
    targetLevel = (int)sourceMap->level + levelDelta;

    for (i = 0; i < (int)dungeon->header.mapCount; ++i) {
        const struct DungeonMapDesc_Compat* targetMap = &dungeon->maps[i];
        if ((int)targetMap->level == targetLevel &&
            globalX >= (int)targetMap->offsetMapX &&
            globalX < (int)targetMap->offsetMapX + (int)targetMap->width &&
            globalY >= (int)targetMap->offsetMapY &&
            globalY < (int)targetMap->offsetMapY + (int)targetMap->height) {
            *mapX = globalX - (int)targetMap->offsetMapX;
            *mapY = globalY - (int)targetMap->offsetMapY;
            return i;
        }
    }
    return -1;
}

static unsigned short orch_thing_with_cell_compat(unsigned short thing, int cell)
{
    return (unsigned short)((thing & 0x3FFFu) | (unsigned short)((cell & 0x03) << 14));
}

static int orch_set_next_thing_compat(
    struct DungeonThings_Compat* things,
    unsigned short thing,
    unsigned short nextThing)
{
    int type = THING_GET_TYPE(thing);
    int index = THING_GET_INDEX(thing);

    if (!things || index < 0) return 0;
    switch (type) {
        case THING_TYPE_DOOR:
            if (index >= things->doorCount || !things->doors) return 0;
            things->doors[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_TELEPORTER:
            if (index >= things->teleporterCount || !things->teleporters) return 0;
            things->teleporters[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_TEXTSTRING:
            if (index >= things->textStringCount || !things->textStrings) return 0;
            things->textStrings[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_SENSOR:
            if (index >= things->sensorCount || !things->sensors) return 0;
            things->sensors[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_GROUP:
            if (index >= things->groupCount || !things->groups) return 0;
            things->groups[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_WEAPON:
            if (index >= things->weaponCount || !things->weapons) return 0;
            things->weapons[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_ARMOUR:
            if (index >= things->armourCount || !things->armours) return 0;
            things->armours[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_SCROLL:
            if (index >= things->scrollCount || !things->scrolls) return 0;
            things->scrolls[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_POTION:
            if (index >= things->potionCount || !things->potions) return 0;
            things->potions[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_CONTAINER:
            if (index >= things->containerCount || !things->containers) return 0;
            things->containers[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_JUNK:
            if (index >= things->junkCount || !things->junks) return 0;
            things->junks[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_PROJECTILE:
            if (index >= things->projectileCount || !things->projectiles) return 0;
            things->projectiles[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        case THING_TYPE_EXPLOSION:
            if (index >= things->explosionCount || !things->explosions) return 0;
            things->explosions[index].next = nextThing;
            orch_write_raw_next_compat(things, thing);
            return 1;
        default:
            return 0;
    }
}


static int orch_unlink_thing_from_square_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned short thingToUnlink)
{
    int sftIndex;
    unsigned short thing;
    unsigned short previous = THING_NONE;
    unsigned short target = (unsigned short)(thingToUnlink & 0x3FFFu);
    int safety = 0;

    if (!world || !world->dungeon || !world->things) return 0;
    if (thingToUnlink == THING_NONE || thingToUnlink == THING_ENDOFLIST) return 0;
    sftIndex = orch_square_first_thing_list_index_compat(world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;

    thing = world->things->squareFirstThings[sftIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        unsigned short nextThing = orch_next_thing_compat(world->things, thing);
        if ((unsigned short)(thing & 0x3FFFu) == target) {
            if (previous == THING_NONE) {
                world->things->squareFirstThings[sftIndex] = nextThing;
            } else if (!orch_set_next_thing_compat(world->things, previous, nextThing)) {
                return 0;
            }
            (void)orch_set_next_thing_compat(world->things, thingToUnlink, THING_ENDOFLIST);
            return 1;
        }
        previous = thing;
        thing = nextThing;
    }
    return 0;
}

static int orch_delete_projectile_move_events_compat(
    struct GameWorld_Compat* world,
    int projectileIndex)
{
    int i;
    int writeIndex = 0;
    int deleted = 0;
    int oldCount;

    if (!world || projectileIndex < 0) return 0;
    oldCount = world->timeline.count;
    for (i = 0; i < oldCount; ++i) {
        const struct TimelineEvent_Compat* event = &world->timeline.events[i];
        if (event->kind == TIMELINE_EVENT_PROJECTILE_MOVE &&
            event->aux0 == projectileIndex) {
            deleted = 1;
            continue;
        }
        if (writeIndex != i) {
            world->timeline.events[writeIndex] = world->timeline.events[i];
        }
        ++writeIndex;
    }
    while (writeIndex < oldCount) {
        memset(&world->timeline.events[writeIndex], 0, sizeof(world->timeline.events[writeIndex]));
        ++writeIndex;
    }
    if (deleted) world->timeline.count = writeIndex;
    return deleted;
}

static int orch_projectile_landing_cell_f0219_compat(
    const struct ProjectileInstance_Compat* projectile)
{
    if (!projectile) return -1;
    if ((projectile->direction & 1) == (projectile->cell & 1)) {
        return (projectile->cell - 1) & 3;
    }
    return (projectile->cell + 1) & 3;
}

static int orch_find_projectile_collision_peer_compat(
    const struct GameWorld_Compat* world,
    const struct ProjectileInstance_Compat* projectile,
    int projectileIndex,
    const struct CellContentDigest_Compat* digest)
{
    int i;
    int landingCell;

    if (!world || !projectile || !digest) return -1;
    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat* other =
            &world->projectiles.entries[i];
        if (i == projectileIndex || other->slotIndex < 0) continue;
        if (other->mapIndex == projectile->mapIndex &&
            other->mapX == projectile->mapX &&
            other->mapY == projectile->mapY &&
            other->cell == projectile->cell) {
            return i;
        }
    }

    landingCell = orch_projectile_landing_cell_f0219_compat(projectile);
    if (landingCell < 0) return -1;
    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat* other =
            &world->projectiles.entries[i];
        if (i == projectileIndex || other->slotIndex < 0) continue;
        if (other->mapIndex == digest->destMapIndex &&
            other->mapX == digest->destMapX &&
            other->mapY == digest->destMapY &&
            other->cell == landingCell) {
            return i;
        }
    }
    return -1;
}

static int orch_find_active_group_state_index_compat(
    const struct GameWorld_Compat* world,
    int groupIndex)
{
    int i;
    if (!world || groupIndex < 0) return -1;
    for (i = 0; i < world->creatureAICount; ++i) {
        if (world->creatureAI[i].reserved0 == groupIndex) return i;
    }
    return -1;
}

static void orch_remove_active_group_state_compat(
    struct GameWorld_Compat* world,
    int groupIndex)
{
    int i;
    int writeIndex = 0;
    int oldCount;
    int retainedCount;
    if (!world || groupIndex < 0) return;
    oldCount = world->creatureAICount;
    for (i = 0; i < oldCount; ++i) {
        if (world->creatureAI[i].reserved0 == groupIndex) continue;
        if (writeIndex != i) world->creatureAI[writeIndex] = world->creatureAI[i];
        ++writeIndex;
    }
    retainedCount = writeIndex;
    while (writeIndex < oldCount) {
        memset(&world->creatureAI[writeIndex], 0, sizeof(world->creatureAI[writeIndex]));
        ++writeIndex;
    }
    world->creatureAICount = retainedCount;
}

static int orch_group_creature_cell_compat(
    const struct DungeonGroup_Compat* group,
    int creatureIndex)
{
    if (!group) return 0xFF;
    if (group->cells == 0xFFu) return 0xFF;
    return (group->cells >> (creatureIndex << 1)) & 0x03;
}

static int orch_group_set_creature_cell_compat(
    int cells,
    int creatureIndex,
    int cell)
{
    int shift = creatureIndex << 1;
    return (cells & ~(0x03 << shift)) | ((cell & 0x03) << shift);
}

static void orch_build_group_projectile_impact_cells_compat(
    const struct DungeonGroup_Compat* group,
    unsigned char ordinalInCell[4])
{
    int i;
    if (!ordinalInCell) return;
    memset(ordinalInCell, 0, 4);
    if (!group) return;
    for (i = 0; i <= (int)group->count && i < 4; ++i) {
        int cell;
        if (group->health[i] == 0) continue;
        if (group->cells == 0xFFu) {
            ordinalInCell[0] = ordinalInCell[1] =
                ordinalInCell[2] = ordinalInCell[3] = (unsigned char)(i + 1);
            return;
        }
        cell = orch_group_creature_cell_compat(group, i);
        if (cell >= 0 && cell < 4) ordinalInCell[cell] = (unsigned char)(i + 1);
    }
}

static int orch_maybe_attach_projectile_weapon_to_group_slot_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const struct ProjectileInstance_Compat* projectile,
    int damageOutcome);

int F0890a_ORCH_ApplyProjectileCreatureImpact_Compat(
    struct DungeonGroup_Compat* group,
    int creatureIndex,
    const struct ProjectileInstance_Compat* projectile)
{
    const struct CreatureBehaviorProfile_Compat* profile;
    int impactAttack;
    int defense;
    int damage;
    int i;

    if (!group || !projectile || creatureIndex < 0 || creatureIndex > 3) return 0;
    profile = CREATURE_GetProfile_Compat(group->creatureType);
    if (profile && (profile->attributes & CREATURE_ATTR_MASK_ARCHENEMY)) return 0;

    impactAttack = projectile->attack ? projectile->attack : projectile->kineticEnergy;

    /* ReDMCSB PROJEXPL.C:F0217 heals Black Flame on fireball impact
     * and then skips the normal damage branch. */
    if (projectile->projectileSubtype == PROJECTILE_SUBTYPE_FIREBALL &&
        group->creatureType == ORCH_CREATURE_BLACK_FLAME_PC34) {
        int healed = (int)group->health[creatureIndex] + impactAttack;
        if (healed > ORCH_BLACK_FLAME_MAX_HEALTH_PC34) {
            healed = ORCH_BLACK_FLAME_MAX_HEALTH_PC34;
        }
        group->health[creatureIndex] = (unsigned short)healed;
        return 0;
    }

    /* PROJEXPL.C:F0217 scales projectile impact attack by creature defense
     * before calling GROUP.C:F0190.  The compat projectile record already
     * stores the resolved impact attack/energy, so this keeps the same
     * source-shaped branch without recreating every associated-object case. */
    defense = (profile && profile->baseDefense > 0) ? profile->baseDefense : 64;
    damage = (impactAttack << 6) / defense;
    if (damage <= 0) return 0;

    if (group->health[creatureIndex] > (unsigned int)damage) {
        group->health[creatureIndex] = (unsigned short)(group->health[creatureIndex] - damage);
        return 0;
    }

    group->health[creatureIndex] = 0;
    if (group->count == 0) return 2;

    for (i = creatureIndex; i < (int)group->count && i < 3; ++i) {
        group->health[i] = group->health[i + 1];
        if (group->cells != 0xFFu) {
            group->cells = (unsigned char)orch_group_set_creature_cell_compat(
                group->cells, i, orch_group_creature_cell_compat(group, i + 1));
        }
    }
    group->health[group->count] = 0;
    if (group->cells != 0xFFu) group->cells = (unsigned char)(group->cells & 0x3Fu);
    group->count--;
    return 1;
}

static void orch_build_precheck_projectile_instance_compat(
    const struct DungeonProjectile_Compat* projectile,
    int projectileIndex,
    struct ProjectileInstance_Compat* out)
{
    if (!out) return;
    memset(out, 0, sizeof(*out));
    out->slotIndex = projectileIndex;
    if (!projectile) return;
    out->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    out->projectileSubtype = projectile->slot & 0x00FFu;
    out->kineticEnergy = projectile->kineticEnergy;
    out->attack = projectile->attack;
    out->reserved1 = projectile->slot;
}

static int orch_process_group_projectile_impacts_on_square_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    int mapIndex,
    int mapX,
    int mapY,
    const unsigned char ordinalInCell[4],
    int* outKilledGroup)
{
    int sftIndex;
    int restart;

    if (outKilledGroup) *outKilledGroup = 0;
    if (!world || !group || !ordinalInCell || !world->dungeon || !world->things) return 0;
    sftIndex = orch_square_first_thing_list_index_compat(world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 1;

    do {
        unsigned short thing = world->things->squareFirstThings[sftIndex];
        int safety = 0;
        restart = 0;
        while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
            int type = THING_GET_TYPE(thing);
            int index = THING_GET_INDEX(thing);
            int cell = THING_GET_CELL(thing);
            if (type == THING_TYPE_PROJECTILE && index >= 0 &&
                index < world->things->projectileCount && world->things->projectiles &&
                ordinalInCell[cell]) {
                struct DungeonProjectile_Compat* projectile = &world->things->projectiles[index];
                struct ProjectileInstance_Compat compatProjectile;
                int creatureIndex = (int)ordinalInCell[cell] - 1;
                int outcome;
                int combatOutcome;

                orch_build_precheck_projectile_instance_compat(
                    projectile, index, &compatProjectile);
                /* MOVESENS.C:F0266:292-301 calls F0217 on matching projectile
                 * cells, then F0214 deletes the projectile event and F0217/
                 * PROJEXPL.C:607-608 unlinks/deletes the projectile thing.
                 * The compat timeline stores projectile slot in aux0. */
                (void)orch_delete_projectile_move_events_compat(world, index);
                (void)orch_unlink_thing_from_square_compat(world, mapIndex, mapX, mapY, thing);
                projectile->next = THING_NONE;
                projectile->eventIndex = 0xFFFFu;
                outcome = F0890a_ORCH_ApplyProjectileCreatureImpact_Compat(
                    group, creatureIndex, &compatProjectile);
                combatOutcome = (outcome == 2) ? COMBAT_OUTCOME_KILLED_ALL_CREATURES :
                    ((outcome == 1) ? COMBAT_OUTCOME_KILLED_SOME_CREATURES :
                                      COMBAT_OUTCOME_KILLED_NO_CREATURES);
                /* ReDMCSB PROJEXPL.C:F0217 lines 540-553 can pass
                 * GROUP.Slot to F0215 so F0266 source/destination
                 * projectile prechecks preserve thrown sharp weapons as
                 * group possessions when no creature is killed. */
                (void)orch_maybe_attach_projectile_weapon_to_group_slot_compat(
                    world, group, &compatProjectile, combatOutcome);
                if (outcome == 2) {
                    if (outKilledGroup) *outKilledGroup = 1;
                    return 1;
                }
                restart = 1;
                break;
            }
            thing = orch_next_thing_compat(world->things, thing);
        }
    } while (restart);
    return 1;
}

static int orch_apply_f0266_group_projectile_precheck_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    int sourceMapIndex,
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY,
    int* outKilledGroup)
{
    struct DungeonGroup_Compat* group;
    unsigned char sourceOrdinalInCell[4];
    unsigned char destinationOrdinalInCell[4];
    unsigned char intermediaryOrdinalInCell[4];
    int checkDestination;

    if (outKilledGroup) *outKilledGroup = 0;
    if (!world || !world->things || groupIndex < 0 ||
        groupIndex >= world->things->groupCount || !world->things->groups) return 0;
    if (sourceMapX < 0) return 1;
    if (sourceMapIndex != world->partyMapIndex) return 1;

    /* MOVESENS.C:F0267:432-435 enters F0266 only for party moves or
     * groups moving on the party map from a real source square.  Non-square
     * C006/F0185/event60 insertion remains the no-impact preservation path. */
    group = &world->things->groups[groupIndex];
    orch_build_group_projectile_impact_cells_compat(group, sourceOrdinalInCell);
    checkDestination = F0709_MOVEMENT_BuildIntermediaryProjectileImpactCells_Compat(
        sourceMapX, sourceMapY, destinationMapX, destinationMapY,
        sourceOrdinalInCell, destinationOrdinalInCell, intermediaryOrdinalInCell);
    if (!orch_process_group_projectile_impacts_on_square_compat(
            world, group, sourceMapIndex, sourceMapX, sourceMapY,
            sourceOrdinalInCell, outKilledGroup)) {
        return 0;
    }
    if (outKilledGroup && *outKilledGroup) return 1;
    if (checkDestination) {
        if (!orch_process_group_projectile_impacts_on_square_compat(
                world, group, sourceMapIndex, destinationMapX, destinationMapY,
                intermediaryOrdinalInCell, outKilledGroup)) {
            return 0;
        }
    }
    return 1;
}

static int orch_build_projectile_digest_compat(
    const struct GameWorld_Compat* world,
    const struct ProjectileInstance_Compat* projectile,
    int projectileIndex,
    struct CellContentDigest_Compat* out)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char sourceSquare = 0;
    unsigned char destSquare = 0;
    int dx = 0;
    int dy = 0;
    int destX;
    int destY;
    int i;

    if (!world || !projectile || !out || !world->dungeon) return 0;
    if (projectile->mapIndex < 0 ||
        projectile->mapIndex >= (int)world->dungeon->header.mapCount) return 0;
    if (!orch_read_square_byte_compat(world->dungeon, projectile->mapIndex,
                                      projectile->mapX, projectile->mapY,
                                      &sourceSquare)) {
        return 0;
    }

    map = &world->dungeon->maps[projectile->mapIndex];
    orch_projectile_step_compat(projectile->direction, &dx, &dy);
    destX = projectile->mapX + dx;
    destY = projectile->mapY + dy;

    memset(out, 0, sizeof(*out));
    out->sourceMapIndex = projectile->mapIndex;
    out->sourceMapX = projectile->mapX;
    out->sourceMapY = projectile->mapY;
    out->sourceSquareType = (sourceSquare & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    out->destTeleporterNewDirection = -1;

    for (i = 0; i < world->projectiles.count &&
                i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat* other =
            &world->projectiles.entries[i];
        if (i == projectileIndex || other->slotIndex < 0) continue;
        if (other->mapIndex == projectile->mapIndex &&
            other->mapX == projectile->mapX &&
            other->mapY == projectile->mapY &&
            other->cell == projectile->cell) {
            /* ReDMCSB PROJEXPL.C F0219/F0217 carries M011_CELL through
             * projectile impact checks; another projectile on the same
             * dungeon square but in a different cell is not the current
             * cell impact. */
            out->sourceHasOtherProjectile = 1;
            break;
        }
    }

    out->destMapIndex = projectile->mapIndex;
    out->destMapX = destX;
    out->destMapY = destY;
    if (destX < 0 || destY < 0 ||
        destX >= (int)map->width || destY >= (int)map->height ||
        !orch_read_square_byte_compat(world->dungeon, projectile->mapIndex,
                                      destX, destY, &destSquare)) {
        out->destIsMapBoundary = 1;
        out->destSquareType = PROJECTILE_ELEMENT_WALL;
        out->destDoorState = PROJECTILE_DOOR_STATE_NONE;
        return 1;
    }

    out->destSquareType = (destSquare & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    if (out->destSquareType == PROJECTILE_ELEMENT_DOOR) {
        int doorState = destSquare & 0x07;
        if (doorState == 0) {
            out->destDoorState = PROJECTILE_DOOR_STATE_OPEN;
        } else if (doorState <= 4) {
            out->destDoorState = doorState;
        } else if (doorState == 5) {
            out->destDoorState = PROJECTILE_DOOR_STATE_DESTROYED;
        } else {
            out->destDoorState = PROJECTILE_DOOR_STATE_NONE;
        }
        {
            int doorIndex = -1;
            if (orch_cmd_attack_find_door_on_square_compat(
                    world, projectile->mapIndex, destX, destY, &doorIndex) &&
                world->things && world->things->doors &&
                doorIndex >= 0 && doorIndex < world->things->doorCount) {
                /* ReDMCSB PROJEXPL.C:F0217 lines 485-488 only lets an
                 * Open Door projectile schedule C10/C02 when Door->Button
                 * is set on the impacted door thing. */
                out->destDoorHasButton =
                    world->things->doors[doorIndex].button ? 1 : 0;
                {
                    const struct DungeonDoor_Compat* door =
                        &world->things->doors[doorIndex];
                    int doorSet = door->type ? map->doorSet1 : map->doorSet0;
                    int doorInfoIndex = doorSet & 3;
                    /* ReDMCSB DUNGEON.C:G0254 lines 560-565: only the
                     * portcullis DoorInfo row (index 0) has
                     * MASK0x0002_PROJECTILES_CAN_PASS_THROUGH. */
                    out->destDoorAllowsProjectilePassThrough =
                        (doorInfoIndex == 0) ? 1 : 0;
                }
            }
        }
    } else {
        out->destDoorState = PROJECTILE_DOOR_STATE_NONE;
    }

    if (world->party.mapIndex == projectile->mapIndex &&
        world->party.mapX == destX && world->party.mapY == destY) {
        out->destHasChampion = 1;
        out->destPartyDirection = world->party.direction & 3;
        out->destChampionCellMask = 0x0F;
    }

    for (i = 0; i < world->creatureAICount &&
                i < GAMEWORLD_CREATURE_AI_CAPACITY; ++i) {
        const struct CreatureAIState_Compat* ai = &world->creatureAI[i];
        if (ai->groupMapIndex == projectile->mapIndex &&
            ai->groupMapX == destX && ai->groupMapY == destY) {
            const struct CreatureBehaviorProfile_Compat* profile =
                CREATURE_GetProfile_Compat(ai->creatureType);
            out->destHasCreatureGroup = 1;
            out->destCreatureType = ai->creatureType;
            out->destCreatureCellMask = 0x0F;
            out->destCreatureIsNonMaterial =
                profile &&
                ((profile->attributes & CREATURE_ATTR_MASK_NON_MATERIAL) != 0);
            break;
        }
    }

    for (i = 0; i < world->projectiles.count &&
                i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat* other =
            &world->projectiles.entries[i];
        if (i == projectileIndex || other->slotIndex < 0) continue;
        if (other->mapIndex == projectile->mapIndex &&
            other->mapX == destX && other->mapY == destY) {
            int newCell;
            if ((projectile->direction & 1) == (projectile->cell & 1)) {
                newCell = (projectile->cell - 1) & 3;
            } else {
                newCell = (projectile->cell + 1) & 3;
            }
            if (other->cell == newCell) {
                /* ReDMCSB F0219 lines 721-725 applies the parity cell step
                 * before relinking the projectile.  Only a projectile in that
                 * landing cell is a same-cell projectile collision. */
                out->destHasOtherProjectile = 1;
                break;
            }
        }
    }
    return 1;
}

static int orch_apply_projectile_champion_action_compat(
    struct GameWorld_Compat* world,
    const struct CombatAction_Compat* action,
    const struct ProjectileInstance_Compat* projectile,
    struct TickResult_Compat* result);
static int orch_apply_projectile_group_action_compat(
    struct GameWorld_Compat* world,
    const struct CombatAction_Compat* action,
    const struct ProjectileInstance_Compat* projectile,
    struct TickResult_Compat* result);
static int orch_maybe_attach_projectile_weapon_to_group_slot_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const struct ProjectileInstance_Compat* projectile,
    int damageOutcome);
static void orch_schedule_projectile_hit_group_reaction_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    const struct DungeonGroup_Compat* group,
    const struct CombatAction_Compat* action);
static int orch_drop_group_fixed_possessions_compat(
    struct GameWorld_Compat* world,
    const struct DungeonGroup_Compat* group,
    int mapIndex,
    int mapX,
    int mapY);
static int orch_drop_creature_fixed_possessions_compat(
    struct GameWorld_Compat* world,
    int creatureType,
    int cell,
    int mapIndex,
    int mapX,
    int mapY);
static int orch_drop_group_slot_possessions_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    int mapIndex,
    int mapX,
    int mapY);
static int orch_link_thing_to_square_tail_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned short thing);
static void orch_cmd_attack_cleanup_f0190_creature_events_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    int killedCreatureIndex);
static int orch_cmd_attack_apply_f0190_fear_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const struct CombatantCreatureSnapshot_Compat* creature,
    int groupIndex,
    int originalGroupCount,
    int mapIndex,
    int mapX,
    int mapY);

static int orch_materialize_projectile_associated_thing_compat(
    struct GameWorld_Compat* world,
    const struct ProjectileInstance_Compat* projectile,
    int associatedThingMovedToGroup)
{
    unsigned short associatedThing;
    unsigned short droppedThing;

    if (!world || !projectile || associatedThingMovedToGroup) return 1;
    if (!world->things || !world->dungeon) return 1;
    if ((projectile->flags & PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT) != 0) {
        return 1;
    }
    associatedThing = (unsigned short)projectile->reserved1;
    if (associatedThing == THING_NONE || associatedThing == THING_ENDOFLIST) {
        return 1;
    }
    if (THING_GET_TYPE(associatedThing) == THING_TYPE_EXPLOSION) {
        return 1;
    }

    /* ReDMCSB PROJEXPL.C:F0215 lines 248-259 moves Projectile.Slot to
     * the projectile map square when F0217 does not pass a GROUP.Slot
     * pointer for KEEP_THROWN_SHARP_WEAPONS.  F0219 lines 717-725 call
     * wall impacts before committing the destination move, so use the
     * projectile's stored square and cell here. */
    droppedThing = orch_thing_with_cell_compat(
        associatedThing, projectile->cell & 3);
    return orch_link_thing_to_square_tail_compat(
        world, projectile->mapIndex, projectile->mapX,
        projectile->mapY, droppedThing);
}

static int orch_projectile_associated_icon_index_compat(
    const struct DungeonThings_Compat* things,
    unsigned short thing)
{
    int type;
    int index;
    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return -1;
    }
    type = (int)THING_GET_TYPE(thing);
    index = (int)THING_GET_INDEX(thing);
    if (index < 0) return -1;

    /* ReDMCSB PROJEXPL.C:F0217 lines 496-501 calls
     * F0033_OBJECT_GetIconIndex for the PC 3.4 key-through-door fix.  The
     * orchestrator only needs the decoded icon/type value before F0811; the
     * projectile save blob keeps using reserved0 as transient scratch. */
    if (type == THING_TYPE_WEAPON) {
        if (!things->weapons || index >= things->weaponCount) return -1;
        return (int)things->weapons[index].type;
    }
    if (type == THING_TYPE_JUNK) {
        if (!things->junks || index >= things->junkCount) return -1;
        return (int)things->junks[index].type;
    }
    return -1;
}

static int orch_projectile_object_info_index_compat(
    const struct DungeonThings_Compat* things,
    unsigned short thing)
{
    int type;
    int index;
    int subtype;
    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) return -1;
    type = (int)THING_GET_TYPE(thing);
    index = (int)THING_GET_INDEX(thing);
    if (index < 0) return -1;

    /* ReDMCSB DUNGEON.C:F0141 maps thing type/subtype to
     * G0237_as_Graphic559_ObjectInfo before F0217 reads AllowedSlots. */
    switch (type) {
    case THING_TYPE_SCROLL:
        return 0;
    case THING_TYPE_CONTAINER:
        if (!things->containers || index >= things->containerCount) return -1;
        subtype = things->containers[index].type;
        if (subtype < 0 || subtype > 0) subtype = 0;
        return 1 + subtype;
    case THING_TYPE_POTION:
        if (!things->potions || index >= things->potionCount) return -1;
        subtype = things->potions[index].type;
        if (subtype < 0 || subtype > 20) subtype = 0;
        return 2 + subtype;
    case THING_TYPE_WEAPON:
        if (!things->weapons || index >= things->weaponCount) return -1;
        subtype = things->weapons[index].type;
        if (subtype < 0 || subtype > 45) subtype = 0;
        return 23 + subtype;
    case THING_TYPE_ARMOUR:
        if (!things->armours || index >= things->armourCount) return -1;
        subtype = things->armours[index].type;
        if (subtype < 0 || subtype > 57) subtype = 0;
        return 69 + subtype;
    case THING_TYPE_JUNK:
        if (!things->junks || index >= things->junkCount) return -1;
        subtype = things->junks[index].type;
        if (subtype < 0 || subtype > 52) subtype = 0;
        return 127 + subtype;
    default:
        return -1;
    }
}

static int orch_projectile_associated_allowed_slots_compat(
    const struct DungeonThings_Compat* things,
    unsigned short thing)
{
    static const unsigned short s_object_info_allowed_slots[180] = {
        0x0500, 0x0200, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500,
        0x0501, 0x0501, 0x0501, 0x0501, 0x0501, 0x0501, 0x0501, 0x0501,
        0x0501, 0x0501, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500,
        0x0500, 0x0400, 0x0400, 0x0040, 0x0040, 0x0040, 0x0040, 0x05C0,
        0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040,
        0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0440, 0x0040, 0x0040,
        0x0040, 0x0040, 0x05C0, 0x05C0, 0x0440, 0x05C0, 0x05C0, 0x05C0,
        0x0040, 0x0040, 0x0540, 0x0540, 0x0040, 0x0040, 0x0040, 0x0040,
        0x0440, 0x0040, 0x0440, 0x0040, 0x0040, 0x040C, 0x040C, 0x0410,
        0x0420, 0x0420, 0x0408, 0x0410, 0x0408, 0x0410, 0x0408, 0x0408,
        0x0410, 0x0410, 0x0408, 0x0410, 0x0420, 0x0408, 0x0410, 0x0420,
        0x0410, 0x0408, 0x0408, 0x0410, 0x0402, 0x0402, 0x0402, 0x0402,
        0x0402, 0x0400, 0x0200, 0x0200, 0x0200, 0x0408, 0x0410, 0x0408,
        0x0410, 0x0402, 0x0420, 0x0402, 0x0008, 0x0010, 0x0420, 0x0200,
        0x0402, 0x0008, 0x0010, 0x0420, 0x0200, 0x0402, 0x0008, 0x0010,
        0x0420, 0x0200, 0x0402, 0x0408, 0x0010, 0x0420, 0x0408, 0x0500,
        0x0501, 0x0504, 0x0504, 0x0500, 0x0400, 0x0500, 0x0500, 0x0500,
        0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500,
        0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500,
        0x0200, 0x0500, 0x0500, 0x0500, 0x0501, 0x0501, 0x0501, 0x0501,
        0x0401, 0x0401, 0x0501, 0x0501, 0x0504, 0x0504, 0x0504, 0x0504,
        0x0504, 0x0500, 0x0500, 0x0500, 0x0400, 0x0500, 0x0500, 0x0504,
        0x0500, 0x0500, 0x0000, 0x0400
    };
    int objectInfoIndex = orch_projectile_object_info_index_compat(things, thing);
    if (objectInfoIndex < 0 || objectInfoIndex >= 180) return 0;
    return (int)s_object_info_allowed_slots[objectInfoIndex];
}

static int orch_materialize_projectile_tick_explosion_compat(
    struct GameWorld_Compat* world,
    const struct ProjectileTickResult_Compat* tickResult)
{
    struct ExplosionCreateInput_Compat explosionIn;
    struct TimelineEvent_Compat firstExplosionAdvance;
    int explosionSlot = -1;

    if (!world || !tickResult || !tickResult->emittedExplosion) return 0;

    memset(&explosionIn, 0, sizeof(explosionIn));
    explosionIn.explosionType =
        tickResult->outExplosion.explosionType;
    explosionIn.attack = tickResult->outExplosion.attack;
    explosionIn.mapIndex = tickResult->outExplosion.mapIndex;
    explosionIn.mapX = tickResult->outExplosion.mapX;
    explosionIn.mapY = tickResult->outExplosion.mapY;
    explosionIn.cell = tickResult->outExplosion.cell;
    explosionIn.centered = tickResult->outExplosion.centered;
    explosionIn.poisonAttack = tickResult->outExplosion.poisonAttack;
    explosionIn.currentTick = (int)world->gameTick;
    explosionIn.ownerKind = tickResult->outExplosion.ownerKind;
    explosionIn.ownerIndex = tickResult->outExplosion.ownerIndex;
    explosionIn.creatorProjectileSlot =
        tickResult->outExplosion.creatorProjectileSlot;
    if (F0821_EXPLOSION_Create_Compat(
            &explosionIn, &world->explosions, &explosionSlot,
            &firstExplosionAdvance)) {
        (void)F0721_TIMELINE_Schedule_Compat(
            &world->timeline, &firstExplosionAdvance);
        return 1;
    }
    return 0;
}

static int orch_handle_projectile_move_event_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* event,
    struct TickResult_Compat* result)
{
    struct ProjectileInstance_Compat* projectile;
    struct ProjectileInstance_Compat projectileForAdvance;
    struct ProjectileInstance_Compat newState;
    struct ProjectileTickResult_Compat tickResult;
    struct CellContentDigest_Compat digest;
    int projectileIndex;
    int associatedThingMovedToGroup = 0;

    if (!world || !event) return 0;
    projectileIndex = event->aux0;
    if (projectileIndex < 0 || projectileIndex >= PROJECTILE_LIST_CAPACITY) return 0;
    projectile = &world->projectiles.entries[projectileIndex];
    if (projectile->slotIndex < 0 || projectile->reserved3 == 0) return 1;
    if (!orch_build_projectile_digest_compat(
            world, projectile, projectileIndex, &digest)) {
        return 0;
    }
    projectileForAdvance = *projectile;
    projectileForAdvance.reserved0 =
        orch_projectile_associated_icon_index_compat(
            world->things, (unsigned short)projectile->reserved1);
    projectileForAdvance.reserved2 =
        orch_projectile_associated_allowed_slots_compat(
            world->things, (unsigned short)projectile->reserved1);

    if (!F0811_PROJECTILE_Advance_Compat(
            &projectileForAdvance, &digest, world->gameTick, &world->masterRng,
            &newState, &tickResult)) {
        F0813_PROJECTILE_Despawn_Compat(&world->projectiles, projectileIndex);
        return 1;
    }

    if (tickResult.despawn) {
        if (tickResult.emittedCombatAction &&
            tickResult.outAction.kind == COMBAT_ACTION_APPLY_DAMAGE_CHAMPION) {
            (void)orch_apply_projectile_champion_action_compat(
                world, &tickResult.outAction, projectile, result);
        }
        if (tickResult.emittedCombatAction &&
            tickResult.outAction.kind == COMBAT_ACTION_APPLY_DAMAGE_GROUP) {
            int groupApplyResult = orch_apply_projectile_group_action_compat(
                world, &tickResult.outAction, projectile, result);
            if (groupApplyResult == 2) {
                associatedThingMovedToGroup = 1;
            }
        }
        if (tickResult.emittedExplosion) {
            (void)orch_materialize_projectile_tick_explosion_compat(
                world, &tickResult);
        }
        if (tickResult.emittedDoorDestructionEvent ||
            tickResult.emittedDoorToggleEvent) {
            /* ReDMCSB PROJEXPL.C:F0217 lines 506-508 calls F0232 to
             * schedule C02 door destruction from projectile impact. */
            (void)F0721_TIMELINE_Schedule_Compat(
                    &world->timeline, &tickResult.outNextTick);
        }
        if (tickResult.resultKind == PROJECTILE_RESULT_HIT_WALL &&
            tickResult.emittedSoundRequest) {
            /* ReDMCSB PROJEXPL.C:F0217 lines 587-600 requests the
             * non-explosion impact thud at the projectile source square.
             * Metallic thud is sound index 0, so resultKind/explosion is
             * the validity gate rather than the sound value. */
            emit(result, EMIT_SOUND_REQUEST, tickResult.emittedSoundCode,
                 projectile->mapX, projectile->mapY, projectile->mapIndex);
        }
        if (tickResult.resultKind == PROJECTILE_RESULT_HIT_DOOR &&
            tickResult.emittedSoundRequest) {
            /* ReDMCSB PROJEXPL.C:F0217 lines 485-508 handle Open Door and
             * door attack side effects, then lines 587-600 request the
             * non-explosion impact thud before deleting the projectile. */
            emit(result, EMIT_SOUND_REQUEST, tickResult.emittedSoundCode,
                 projectile->mapX, projectile->mapY, projectile->mapIndex);
        }
        if (tickResult.resultKind == PROJECTILE_RESULT_HIT_OTHER_PROJECTILE &&
            tickResult.emittedSoundRequest) {
            /* ReDMCSB PROJEXPL.C:F0218 lines 621-638 dispatches F0217 for
             * projectile interactions; F0217 lines 587-600 then requests the
             * ordinary non-explosion impact sound at the projectile square. */
            emit(result, EMIT_SOUND_REQUEST, tickResult.emittedSoundCode,
                 projectile->mapX, projectile->mapY, projectile->mapIndex);
        }
        if (tickResult.resultKind == PROJECTILE_RESULT_HIT_OTHER_PROJECTILE) {
            int otherIndex = orch_find_projectile_collision_peer_compat(
                world, projectile, projectileIndex, &digest);
            if (otherIndex >= 0) {
                struct ProjectileInstance_Compat* other =
                    &world->projectiles.entries[otherIndex];
                struct CellContentDigest_Compat peerDigest = digest;
                struct ProjectileTickResult_Compat peerImpact;
                /* Firestaff Phase17 v1 intentionally makes projectile-vs-
                 * projectile collision order-independent: both projectiles
                 * despawn. ReDMCSB F0218/F0217 is the source-locked impact
                 * delete model for projectile interactions and explicitly
                 * deletes the impacted projectile event after F0217. */
                peerDigest.destMapIndex = other->mapIndex;
                peerDigest.destMapX = other->mapX;
                peerDigest.destMapY = other->mapY;
                peerDigest.destSquareType = digest.sourceSquareType;
                memset(&peerImpact, 0, sizeof(peerImpact));
                if (F0820_PROJECTILE_ResolveCollision_Compat(
                        other, &peerDigest,
                        PROJECTILE_RESULT_HIT_OTHER_PROJECTILE,
                        world->gameTick, &world->masterRng, &peerImpact)) {
                    (void)orch_materialize_projectile_tick_explosion_compat(
                        world, &peerImpact);
                    if (peerImpact.emittedSoundRequest) {
                        emit(result, EMIT_SOUND_REQUEST,
                             peerImpact.emittedSoundCode,
                             other->mapX, other->mapY, other->mapIndex);
                    }
                }
                (void)orch_materialize_projectile_associated_thing_compat(
                    world, other, 0);
                (void)orch_delete_projectile_move_events_compat(
                    world, otherIndex);
                (void)F0813_PROJECTILE_Despawn_Compat(
                    &world->projectiles, otherIndex);
            }
        }
        (void)orch_materialize_projectile_associated_thing_compat(
            world, projectile, associatedThingMovedToGroup);
        F0813_PROJECTILE_Despawn_Compat(&world->projectiles, projectileIndex);
        return 1;
    }

    *projectile = newState;
    projectile->scheduledAtTick = (int)tickResult.outNextTick.fireAtTick;
    (void)F0721_TIMELINE_Schedule_Compat(&world->timeline,
                                         &tickResult.outNextTick);
    return 1;
}

static int orch_apply_projectile_champion_action_compat(
    struct GameWorld_Compat* world,
    const struct CombatAction_Compat* action,
    const struct ProjectileInstance_Compat* projectile,
    struct TickResult_Compat* result)
{
    struct CombatResult_Compat damage;
    struct CombatantChampionSnapshot_Compat defender;
    struct ChampionState_Compat* champion;
    int championIndex;
    int scaledAttack = 0;
    int killed = 0;

    if (!world || !action) return 0;
    if (action->kind != COMBAT_ACTION_APPLY_DAMAGE_CHAMPION) return 0;
    if (action->rawAttackValue <= 0) return 0;
    championIndex = action->defenderSlotOrCreatureIndex;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champion = &world->party.champions[championIndex];
    if (!champion->present || champion->hp.current == 0) return 0;

    memset(&damage, 0, sizeof(damage));
    if (!orch_build_defender_champion_snapshot_compat(
            world, championIndex, action->attackTypeCode, &defender) ||
        !F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(
            action->attackTypeCode,
            action->rawAttackValue,
            action->allowedWounds,
            &defender,
            &world->masterRng,
            &scaledAttack,
            NULL)) {
        return 0;
    }
    if (scaledAttack <= 0) {
        return 1;
    }
    damage.damageApplied = scaledAttack;
    damage.woundMaskAdded = action->allowedWounds;

    /* ReDMCSB PROJEXPL.C:F0217 lines 513-558 computes champion
     * projectile impact damage, adds HEAD|TORSO pending wounds through
     * F0321, then deletes the projectile. */
    if (!F0737_COMBAT_ApplyDamageToChampion_Compat(
            &damage, champion, &killed)) {
        return 0;
    }
    if (killed) {
        emit(result, EMIT_CHAMPION_DOWN, championIndex, 0, 0, 0);
    }
    /* ReDMCSB PROJEXPL.C:F0217 lines 557-558 gates projectile poison
     * through F0322 only after champion damage was applied, poison attack
     * exists, and RANDOM(2) passes.  CHAMPION.C:F0322 lines 1949-1960
     * applies immediate max(1, attack >> 6) damage and schedules C75. */
    if (!killed && projectile && projectile->poisonAttack > 0 &&
        F0732_COMBAT_RngRandom_Compat(&world->masterRng, 2) != 0) {
        int poisonDamage = projectile->poisonAttack >> 6;
        int nextAttack;
        unsigned int dose;
        struct TimelineEvent_Compat poisonEvent;

        if (poisonDamage < 1) poisonDamage = 1;
        if (poisonDamage > (int)champion->hp.current) {
            poisonDamage = (int)champion->hp.current;
        }
        champion->hp.current =
            (unsigned short)((int)champion->hp.current - poisonDamage);

        dose = (unsigned int)champion->poisonDose +
               (unsigned int)projectile->poisonAttack;
        if (dose > 0xFFFFu) dose = 0xFFFFu;
        champion->poisonDose = (unsigned short)dose;

        nextAttack = projectile->poisonAttack - 1;
        if (nextAttack > 0) {
            memset(&poisonEvent, 0, sizeof(poisonEvent));
            poisonEvent.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            poisonEvent.fireAtTick = world->gameTick +
                (uint32_t)LIFECYCLE_POISON_RESCHEDULE_TICKS;
            poisonEvent.mapIndex = world->partyMapIndex;
            poisonEvent.mapX = world->party.mapX;
            poisonEvent.mapY = world->party.mapY;
            poisonEvent.aux0 = LIFECYCLE_STATUS_POISON;
            poisonEvent.aux1 = nextAttack;
            poisonEvent.aux4 = championIndex;
            (void)F0721_TIMELINE_Schedule_Compat(
                &world->timeline, &poisonEvent);
            if (world->lifecycle.champions[championIndex].poisonEventCount <
                255) {
                world->lifecycle.champions[championIndex].poisonEventCount++;
            }
        }
        if (champion->hp.current == 0) {
            emit(result, EMIT_CHAMPION_DOWN, championIndex, 0, 0, 0);
        }
    }
    return 1;
}

static int orch_find_group_creature_index_for_cell_compat(
    const struct DungeonGroup_Compat* group,
    int targetCell)
{
    int i;

    if (!group) return -1;
    if (group->cells == 0xFFu) {
        return group->health[0] ? 0 : -1;
    }
    for (i = 0; i <= (int)group->count && i < 4; ++i) {
        if (group->health[i] == 0) continue;
        if (orch_group_creature_cell_compat(group, i) == (targetCell & 3)) {
            return i;
        }
    }
    return -1;
}

static int orch_projectile_weapon_type_is_kept_sharp_compat(int weaponType)
{
    return weaponType == 8  ||  /* C08_WEAPON_DAGGER */
           weaponType == 27 ||  /* C27_WEAPON_ARROW */
           weaponType == 28 ||  /* C28_WEAPON_SLAYER */
           weaponType == 31 ||  /* C31_WEAPON_POISON_DART */
           weaponType == 32;    /* C32_WEAPON_THROWING_STAR */
}

static int orch_maybe_attach_projectile_weapon_to_group_slot_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const struct ProjectileInstance_Compat* projectile,
    int damageOutcome)
{
    const struct CreatureBehaviorProfile_Compat* profile;
    unsigned short associatedThing;
    int weaponIndex;
    int weaponType;

    if (!world || !group || !projectile || !world->things) return 0;
    if (damageOutcome != COMBAT_OUTCOME_KILLED_NO_CREATURES) return 0;
    if ((projectile->flags & PROJECTILE_FLAG_CREATES_EXPLOSION) != 0) return 0;

    associatedThing = (unsigned short)projectile->reserved1;
    if (associatedThing == THING_NONE || associatedThing == THING_ENDOFLIST) return 0;
    if (THING_GET_TYPE(associatedThing) != THING_TYPE_WEAPON) return 0;

    profile = CREATURE_GetProfile_Compat((int)group->creatureType);
    if (!profile ||
        ((profile->attributes & CREATURE_ATTR_MASK_KEEP_THROWN_SHARP_WEAPONS) == 0)) {
        return 0;
    }

    weaponIndex = THING_GET_INDEX(associatedThing);
    if (!world->things->weapons ||
        weaponIndex < 0 ||
        weaponIndex >= world->things->weaponCount) {
        return 0;
    }
    weaponType = (int)world->things->weapons[weaponIndex].type;
    if (!orch_projectile_weapon_type_is_kept_sharp_compat(weaponType)) {
        return 0;
    }

    /* ReDMCSB PROJEXPL.C:F0217 lines 540-553 selects GROUP.Slot as the
     * projectile-delete target for non-exploding sharp weapon projectiles
     * that survive impact against KEEP_THROWN_SHARP_WEAPONS creatures. */
    if (!orch_set_next_thing_compat(world->things, associatedThing, group->slot)) {
        return 0;
    }
    group->slot = associatedThing;
    return 1;
}

static int orch_apply_projectile_group_action_compat(
    struct GameWorld_Compat* world,
    const struct CombatAction_Compat* action,
    const struct ProjectileInstance_Compat* projectile,
    struct TickResult_Compat* result)
{
    struct DungeonGroup_Compat* group;
    struct CombatResult_Compat damage;
    struct CombatantCreatureSnapshot_Compat creatureSnapshot;
    int groupIndex = -1;
    int creatureIndex;
    int killedCell;
    int originalCreatureType;
    int originalGroupCount;
    int creatureSnapshotReady;
    int outcome = COMBAT_OUTCOME_KILLED_NO_CREATURES;
    int associatedThingMovedToGroup = 0;

    if (!world || !action || !world->things || !world->things->groups) return 0;
    if (action->kind != COMBAT_ACTION_APPLY_DAMAGE_GROUP) return 0;
    if (action->rawAttackValue <= 0) return 0;
    if (!orch_cmd_attack_find_group_on_square_compat(
            world, action->targetMapIndex, action->targetMapX,
            action->targetMapY, &groupIndex)) {
        return 0;
    }
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;
    group = &world->things->groups[groupIndex];
    creatureIndex = orch_find_group_creature_index_for_cell_compat(
        group, action->targetCell);
    if (creatureIndex < 0) return 0;
    killedCell = orch_group_creature_cell_compat(group, creatureIndex);
    originalCreatureType = (int)group->creatureType;
    originalGroupCount = (int)group->count;
    creatureSnapshotReady = F0888_ORCH_GetCreatureSnapshot_Compat(
        world, groupIndex, creatureIndex, 0, &creatureSnapshot);

    if (projectile &&
        projectile->projectileSubtype == PROJECTILE_SUBTYPE_FIREBALL &&
        group->creatureType == ORCH_CREATURE_BLACK_FLAME_PC34) {
        int healed;
        /* ReDMCSB PROJEXPL.C:F0217 lines 527-531 heals Black Flame on
         * Fireball impact up to 1000 HP and jumps to T0217044, so no
         * normal F0190 damage, C30 reaction, explosion, or thud follows. */
        healed = (int)group->health[creatureIndex] + action->rawAttackValue;
        if (healed > ORCH_BLACK_FLAME_MAX_HEALTH_PC34) {
            healed = ORCH_BLACK_FLAME_MAX_HEALTH_PC34;
        }
        group->health[creatureIndex] = (unsigned short)healed;
        orch_write_raw_group_compat(world->things, groupIndex);
        return 1;
    }

    memset(&damage, 0, sizeof(damage));
    damage.damageApplied = action->rawAttackValue;

    /* ReDMCSB PROJEXPL.C:F0217 lines 515-537 resolves a concrete
     * creature ordinal in the impact cell, scales attack by creature
     * defense, adds F0192 poison adjustment, and applies the result
     * through GROUP.C F0190.  F0811 has already produced the bounded
     * scaled action payload, so the dispatcher only mutates the live
     * group through F0738. */
    if (!F0738_COMBAT_ApplyDamageToGroup_Compat(
            &damage, group, creatureIndex, &outcome)) {
        return 0;
    }
    if (outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
        /* ReDMCSB PROJEXPL.C:F0217 lines 535-539 receives the F0190
         * damage outcome; GROUP.C:F0190 lines 907-917 creates death
         * smoke, and all-kill removes the group from the square. */
        orch_create_f0190_death_smoke_at_square_compat(
            world, originalCreatureType, killedCell,
            action->targetMapIndex, action->targetMapX, action->targetMapY);
        (void)orch_drop_group_fixed_possessions_compat(
            world, group, action->targetMapIndex,
            action->targetMapX, action->targetMapY);
        (void)orch_drop_group_slot_possessions_compat(
            world, group, action->targetMapIndex,
            action->targetMapX, action->targetMapY);
        (void)orch_unlink_thing_from_square_compat(
            world, action->targetMapIndex, action->targetMapX,
            action->targetMapY,
            orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex));
        group->next = THING_NONE;
        orch_remove_active_group_state_compat(world, groupIndex);
        emit(result, EMIT_KILL_NOTIFY, groupIndex, creatureIndex,
             outcome, originalCreatureType);
    } else {
        if (outcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES) {
            /* ReDMCSB GROUP.C:F0190 lines 842-917 drops fixed possessions
             * for the killed member, cleans per-creature events, may frighten
             * the surviving group, compacts the group, and creates death smoke.
             * F0738 already compacted the live group; the orchestrator owns the
             * surrounding map/timeline side effects. */
            if (creatureSnapshotReady &&
                (creatureSnapshot.attributes & DM1_ATTR_DROP_FIXED_POSS)) {
                (void)orch_drop_creature_fixed_possessions_compat(
                    world, originalCreatureType, killedCell,
                    action->targetMapIndex, action->targetMapX,
                    action->targetMapY);
            }
            if (creatureSnapshotReady && group->behavior == DM1_BEHAVIOR_ATTACK) {
                orch_cmd_attack_cleanup_f0190_creature_events_compat(
                    world, action->targetMapIndex, action->targetMapX,
                    action->targetMapY, creatureIndex);
                (void)orch_cmd_attack_apply_f0190_fear_compat(
                    world, group, &creatureSnapshot, groupIndex,
                    originalGroupCount, action->targetMapIndex,
                    action->targetMapX, action->targetMapY);
            }
            orch_create_f0190_death_smoke_at_square_compat(
                world, originalCreatureType, killedCell,
                action->targetMapIndex, action->targetMapX, action->targetMapY);
            emit(result, EMIT_KILL_NOTIFY, groupIndex, creatureIndex,
                 outcome, originalCreatureType);
        } else {
            if (orch_maybe_attach_projectile_weapon_to_group_slot_compat(
                    world, group, projectile, outcome)) {
                associatedThingMovedToGroup = 1;
            }
        }
        orch_schedule_projectile_hit_group_reaction_compat(
            world, groupIndex, group, action);
    }
    /* ReDMCSB GROUP.C:F0190 lines 892-917 mutates the live group record
     * after projectile damage: surviving groups carry compacted HP/cells and
     * all-kill groups are unlinked.  Mirror those decoded changes into the
     * raw DUNGEON.DAT record used by save/export and later raw inspections. */
    orch_write_raw_group_compat(world->things, groupIndex);
    return associatedThingMovedToGroup ? 2 : 1;
}

static void orch_schedule_projectile_hit_group_reaction_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    const struct DungeonGroup_Compat* group,
    const struct CombatAction_Compat* action)
{
    int activeIndex;
    int ticksSinceLastMove = 0;
    const struct CreatureBehaviorProfile_Compat* profile;
    struct DM1GroupBehaviorContext_Compat ctx;
    struct DM1ActiveGroup_Compat activeGroup;
    struct DM1BehaviorResult_Compat behavior;
    struct TimelineEvent_Compat reaction;

    if (!world || !group || !action || groupIndex < 0) return;

    activeIndex = orch_find_active_group_state_index_compat(world, groupIndex);
    profile = CREATURE_GetProfile_Compat((int)group->creatureType);

    memset(&ctx, 0, sizeof(ctx));
    memset(&activeGroup, 0, sizeof(activeGroup));
    memset(&behavior, 0, sizeof(behavior));

    ctx.currentMapIndex = action->targetMapIndex;
    ctx.currentGroupMapX = action->targetMapX;
    ctx.currentGroupMapY = action->targetMapY;
    ctx.partyMapIndex = world->party.mapIndex;
    ctx.partyMapX = world->party.mapX;
    ctx.partyMapY = world->party.mapY;
    ctx.partyChampionCount = world->party.championCount;
    ctx.creatureType = (int)group->creatureType;
    ctx.groupBehavior = (activeIndex >= 0)
        ? orch_ai_state_to_dm1_behavior_compat(world->creatureAI[activeIndex].stateKind)
        : (int)group->behavior;
    ctx.creatureCount = group->count;
    ctx.movementTicks = profile ? profile->movementTicks : 1;
    if (ctx.movementTicks < 1) ctx.movementTicks = 1;
    if (activeIndex >= 0 && world->creatureAI[activeIndex].lastSeenPartyTick >= 0) {
        ticksSinceLastMove = (int)world->gameTick -
            world->creatureAI[activeIndex].lastSeenPartyTick;
        if (ticksSinceLastMove < 0) ticksSinceLastMove = 0;
    }
    ctx.ticksSinceLastMove = ticksSinceLastMove;
    ctx.currentTickLow = (int)world->gameTick;
    ctx.eventType = DM1_CM2_REACTION_HIT_BY_PROJECTILE;
    ctx.eventTicks = (int)world->gameTick;

    activeGroup.groupThingIndex = groupIndex;
    activeGroup.cells = group->cells;
    activeGroup.directions = group->direction;
    activeGroup.lastMoveTime = (activeIndex >= 0)
        ? world->creatureAI[activeIndex].lastSeenPartyTick : 0;
    activeGroup.priorMapX = action->targetMapX;
    activeGroup.priorMapY = action->targetMapY;

    /* ReDMCSB PROJEXPL.C:F0217 lines 536-537 schedules
     * F0209 CM2_EVENT_CREATE_REACTION_EVENT_30_HIT_BY_PROJECTILE
     * after non-all-kill projectile creature damage.  Let the bounded
     * F0209 port compute the concrete C30 event and source delay. */
    if (!F0810_DM1_GROUP_DispatchBehavior_Compat(
            &ctx, &activeGroup, &world->masterRng, &behavior)) {
        return;
    }
    if (behavior.nextEventDelayTicks <= 0 || behavior.nextEventType <= 0) {
        return;
    }

    memset(&reaction, 0, sizeof(reaction));
    reaction.kind = TIMELINE_EVENT_CREATURE_REACTION;
    reaction.fireAtTick = world->gameTick +
        (uint32_t)behavior.nextEventDelayTicks;
    reaction.mapIndex = action->targetMapIndex;
    reaction.mapX = action->targetMapX;
    reaction.mapY = action->targetMapY;
    reaction.aux0 = groupIndex;
    reaction.aux1 = (int)group->creatureType;
    reaction.aux2 = behavior.nextEventType;
    (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &reaction);
}

static int orch_build_explosion_digest_compat(
    const struct GameWorld_Compat* world,
    const struct ExplosionInstance_Compat* explosion,
    struct CellContentDigest_Compat* out)
{
    unsigned char square = 0;
    int hasSquare;
    int i;

    if (!world || !explosion || !out) return 0;
    memset(out, 0, sizeof(*out));
    out->sourceMapIndex = explosion->mapIndex;
    out->sourceMapX = explosion->mapX;
    out->sourceMapY = explosion->mapY;
    out->destMapIndex = explosion->mapIndex;
    out->destMapX = explosion->mapX;
    out->destMapY = explosion->mapY;
    out->destTeleporterNewDirection = -1;

    hasSquare = orch_read_square_byte_compat(
        world->dungeon, explosion->mapIndex, explosion->mapX,
        explosion->mapY, &square);
    if (!hasSquare) {
        out->sourceSquareType = PROJECTILE_ELEMENT_WALL;
        out->destSquareType = PROJECTILE_ELEMENT_WALL;
        out->destDoorState = PROJECTILE_DOOR_STATE_NONE;
        out->destIsMapBoundary = 1;
        return 1;
    }

    out->sourceSquareType = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    out->destSquareType = out->sourceSquareType;
    if (out->destSquareType == PROJECTILE_ELEMENT_DOOR) {
        int doorState = square & 0x07;
        if (doorState == 0) {
            out->destDoorState = PROJECTILE_DOOR_STATE_OPEN;
        } else if (doorState <= 4) {
            out->destDoorState = doorState;
        } else if (doorState == 5) {
            out->destDoorState = PROJECTILE_DOOR_STATE_DESTROYED;
        } else {
            out->destDoorState = PROJECTILE_DOOR_STATE_NONE;
        }
    } else {
        out->destDoorState = PROJECTILE_DOOR_STATE_NONE;
    }

    if (world->party.mapIndex == explosion->mapIndex &&
        world->party.mapX == explosion->mapX &&
        world->party.mapY == explosion->mapY) {
        out->destHasChampion = 1;
        out->destPartyDirection = world->party.direction & 3;
        out->destChampionCellMask = 0x0F;
    }

    for (i = 0; i < world->creatureAICount &&
                i < GAMEWORLD_CREATURE_AI_CAPACITY; ++i) {
        const struct CreatureAIState_Compat* ai = &world->creatureAI[i];
        if (ai->groupMapIndex == explosion->mapIndex &&
            ai->groupMapX == explosion->mapX &&
            ai->groupMapY == explosion->mapY) {
            const struct CreatureBehaviorProfile_Compat* profile =
                CREATURE_GetProfile_Compat(ai->creatureType);
            out->destHasCreatureGroup = 1;
            out->destCreatureType = ai->creatureType;
            out->destCreatureCellMask = 0x0F;
            out->destCreatureIsNonMaterial =
                profile &&
                ((profile->attributes & CREATURE_ATTR_MASK_NON_MATERIAL) != 0);
            break;
        }
    }
    return 1;
}

static int orch_apply_explosion_group_action_compat(
    struct GameWorld_Compat* world,
    const struct CombatAction_Compat* action);

static int orch_apply_explosion_party_action_compat(
    struct GameWorld_Compat* world,
    const struct CombatAction_Compat* action,
    struct TickResult_Compat* result);

static int orch_handle_explosion_advance_event_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* event,
    struct TickResult_Compat* result)
{
    struct ExplosionInstance_Compat* explosion;
    struct ExplosionInstance_Compat newState;
    struct ExplosionTickResult_Compat tickResult;
    struct CellContentDigest_Compat digest;
    int explosionSlot;

    if (!world || !event) return 0;
    explosionSlot = event->aux0;
    if (explosionSlot < 0 || explosionSlot >= EXPLOSION_LIST_CAPACITY) return 0;
    explosion = &world->explosions.entries[explosionSlot];
    if (explosion->slotIndex < 0 || explosion->reserved0 == 0) return 1;
    if (!orch_build_explosion_digest_compat(world, explosion, &digest)) {
        return 0;
    }
    if (!F0822_EXPLOSION_Advance_Compat(
            explosion, &digest, world->gameTick, &world->masterRng,
            &newState, &tickResult)) {
        F0824_EXPLOSION_Despawn_Compat(&world->explosions, explosionSlot);
        return 1;
    }

    if (tickResult.emittedCombatActionPartyCount > 0) {
        (void)orch_apply_explosion_party_action_compat(
            world, &tickResult.outActionParty, result);
    }
    if (tickResult.emittedCombatActionGroupCount > 0) {
        (void)orch_apply_explosion_group_action_compat(
            world, &tickResult.outActionGroup);
    }

    if (tickResult.emittedDoorDestructionEvent) {
        (void)F0721_TIMELINE_Schedule_Compat(
            &world->timeline, &tickResult.outNextTick);
    }

    if (tickResult.despawn) {
        F0824_EXPLOSION_Despawn_Compat(&world->explosions, explosionSlot);
        return 1;
    }

    *explosion = newState;
    if (tickResult.outNextTick.kind != TIMELINE_EVENT_INVALID &&
        tickResult.outNextTick.kind != 0) {
        explosion->scheduledAtTick = (int)tickResult.outNextTick.fireAtTick;
        (void)F0721_TIMELINE_Schedule_Compat(
            &world->timeline, &tickResult.outNextTick);
    }
    return 1;
}

static int orch_apply_explosion_party_action_compat(
    struct GameWorld_Compat* world,
    const struct CombatAction_Compat* action,
    struct TickResult_Compat* result)
{
    int i;
    int applied = 0;
    int randomWindow;
    int baseAttack;

    if (!world || !action) return 0;
    if (action->rawAttackValue <= 0) return 0;
    randomWindow = (action->rawAttackValue >> 3) + 1;
    baseAttack = action->rawAttackValue - randomWindow;
    randomWindow <<= 1;

    /* ReDMCSB PROJEXPL.C:F0213 line 173 and F0220 line 861 route
     * party-square fireball/lightning and poison-cloud explosions through
     * CHAMPION.C:F0324.  F0324 randomizes attack per champion by +/- 1/8
     * and then calls F0321, which applies fire/magic shields, defense, and
     * wound handling before reporting nonzero damage. */
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        struct CombatResult_Compat damage;
        struct CombatantChampionSnapshot_Compat defender;
        int killed = 0;
        int randomizedAttack;
        int scaledAttack = 0;
        struct ChampionState_Compat* champion = &world->party.champions[i];
        if (!champion->present || champion->hp.current == 0) continue;

        randomizedAttack = baseAttack +
                           F0732_COMBAT_RngRandom_Compat(&world->masterRng,
                                                         randomWindow);
        if (randomizedAttack < 1) randomizedAttack = 1;
        if (!orch_build_defender_champion_snapshot_compat(
                world, i, action->attackTypeCode, &defender) ||
            !F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(
                action->attackTypeCode, randomizedAttack,
                action->allowedWounds, &defender, &world->masterRng,
                &scaledAttack, NULL) ||
            scaledAttack <= 0) {
            continue;
        }

        memset(&damage, 0, sizeof(damage));
        damage.damageApplied = scaledAttack;
        damage.woundMaskAdded = action->allowedWounds;
        if (F0737_COMBAT_ApplyDamageToChampion_Compat(
                &damage, champion, &killed)) {
            applied++;
            if (killed) emit(result, EMIT_CHAMPION_DOWN, i, 0, 0, 0);
        }
    }
    return applied > 0;
}

static int orch_apply_explosion_group_action_compat(
    struct GameWorld_Compat* world,
    const struct CombatAction_Compat* action)
{
    struct DungeonGroup_Compat* group;
    struct CombatResult_Compat damage;
    int groupIndex = -1;
    int creatureIndex = 0;
    int outcome = COMBAT_OUTCOME_KILLED_NO_CREATURES;

    if (!world || !action || !world->things || !world->things->groups) return 0;
    if (!orch_cmd_attack_find_group_on_square_compat(
            world, action->targetMapIndex, action->targetMapX,
            action->targetMapY, &groupIndex)) {
        return 0;
    }
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;
    group = &world->things->groups[groupIndex];

    memset(&damage, 0, sizeof(damage));
    damage.damageApplied = action->rawAttackValue;
    if (damage.damageApplied <= 0) return 0;

    while (creatureIndex <= (int)group->count && creatureIndex < 4) {
        if (group->health[creatureIndex] == 0) {
            ++creatureIndex;
            continue;
        }
        (void)F0738_COMBAT_ApplyDamageToGroup_Compat(
            &damage, group, creatureIndex, &outcome);
        if (outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES) break;
        if (outcome == COMBAT_OUTCOME_KILLED_NO_CREATURES) ++creatureIndex;
    }
    return 1;
}


static unsigned short orch_allocate_fixed_possession_thing_compat(
    struct DungeonThings_Compat* things,
    const struct DM1FixedPossessionDrop_Compat* drop)
{
    int i;

    if (!things || !drop) return THING_NONE;
    switch (drop->thingType) {
        case DM1_DROP_THING_TYPE_WEAPON:
            if (!things->weapons) return THING_NONE;
            for (i = 0; i < things->weaponCount; ++i) {
                if (things->weapons[i].next == THING_NONE) {
                    memset(&things->weapons[i], 0, sizeof(things->weapons[i]));
                    things->weapons[i].next = THING_ENDOFLIST;
                    things->weapons[i].type = (unsigned char)(drop->itemType & 0x7F);
                    things->weapons[i].cursed = (unsigned char)(drop->cursed ? 1 : 0);
                    /* ReDMCSB GROUP.C:F0190 lines 831-847 drops fixed
                     * possessions through the DUNGEON.C:F0163 thing-list
                     * path.  Keep the decoded slot and the raw DUNGEON.DAT
                     * record in step when a free object slot is reused. */
                    orch_write_raw_weapon_compat(things, i);
                    return orch_thing_with_cell_compat(
                        orch_make_thing_ref_compat(THING_TYPE_WEAPON, i),
                        drop->cell);
                }
            }
            return THING_NONE;
        case DM1_DROP_THING_TYPE_ARMOUR:
            if (!things->armours) return THING_NONE;
            for (i = 0; i < things->armourCount; ++i) {
                if (things->armours[i].next == THING_NONE) {
                    memset(&things->armours[i], 0, sizeof(things->armours[i]));
                    things->armours[i].next = THING_ENDOFLIST;
                    things->armours[i].type = (unsigned char)(drop->itemType & 0x7F);
                    things->armours[i].cursed = (unsigned char)(drop->cursed ? 1 : 0);
                    /* ReDMCSB GROUP.C:F0190 lines 831-847 drops fixed
                     * possessions through the DUNGEON.C:F0163 thing-list
                     * path.  Keep the decoded slot and the raw DUNGEON.DAT
                     * record in step when a free object slot is reused. */
                    orch_write_raw_armour_compat(things, i);
                    return orch_thing_with_cell_compat(
                        orch_make_thing_ref_compat(THING_TYPE_ARMOUR, i),
                        drop->cell);
                }
            }
            return THING_NONE;
        case DM1_DROP_THING_TYPE_JUNK:
            if (!things->junks) return THING_NONE;
            for (i = 0; i < things->junkCount; ++i) {
                if (things->junks[i].next == THING_NONE) {
                    memset(&things->junks[i], 0, sizeof(things->junks[i]));
                    things->junks[i].next = THING_ENDOFLIST;
                    things->junks[i].type = (unsigned char)(drop->itemType & 0x7F);
                    things->junks[i].cursed = (unsigned char)(drop->cursed ? 1 : 0);
                    /* ReDMCSB GROUP.C:F0190 lines 831-847 drops fixed
                     * possessions through the DUNGEON.C:F0163 thing-list
                     * path.  Keep the decoded slot and the raw DUNGEON.DAT
                     * record in step when a free object slot is reused. */
                    orch_write_raw_junk_compat(things, i);
                    return orch_thing_with_cell_compat(
                        orch_make_thing_ref_compat(THING_TYPE_JUNK, i),
                        drop->cell);
                }
            }
            return THING_NONE;
        default:
            return THING_NONE;
    }
}

static int orch_link_thing_to_square_tail_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned short thing)
{
    int sftIndex;
    unsigned short current;
    int safety = 0;

    if (!world || !world->dungeon || !world->things) return 0;
    if (thing == THING_NONE || thing == THING_ENDOFLIST) return 0;
    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;

    if (!orch_set_next_thing_compat(world->things, thing, THING_ENDOFLIST)) {
        return 0;
    }
    current = world->things->squareFirstThings[sftIndex];
    if (current == THING_NONE || current == THING_ENDOFLIST) {
        world->things->squareFirstThings[sftIndex] = thing;
        return 1;
    }
    while (current != THING_NONE && current != THING_ENDOFLIST && safety++ < 64) {
        unsigned short next = orch_next_thing_compat(world->things, current);
        if (next == THING_NONE || next == THING_ENDOFLIST) {
            return orch_set_next_thing_compat(world->things, current, thing);
        }
        current = next;
    }
    return 0;
}

static int orch_drop_creature_fixed_possessions_compat(
    struct GameWorld_Compat* world,
    int creatureType,
    int sourceCell,
    int mapIndex,
    int mapX,
    int mapY)
{
    struct DM1FixedPossessionDrop_Compat drops[DM1_MAX_FIXED_POSSESSION_DROPS];
    int dropCount = 0;
    int weaponDropped = 0;
    int droppedAny = 0;
    int i;

    if (!world || !world->things) return 0;
    if (!F0824_DM1_GROUP_ResolveFixedPossessionDrops_Compat(
            creatureType, sourceCell, &world->masterRng, drops,
            DM1_MAX_FIXED_POSSESSION_DROPS, &dropCount, &weaponDropped)) {
        return 0;
    }

    for (i = 0; i < dropCount; ++i) {
        unsigned short thing =
            orch_allocate_fixed_possession_thing_compat(world->things, &drops[i]);
        if (thing == THING_NONE) {
            continue;
        }
        if (orch_link_thing_to_square_tail_compat(world, mapIndex, mapX, mapY, thing)) {
            droppedAny = 1;
        } else {
            (void)orch_set_next_thing_compat(world->things, thing, THING_NONE);
        }
    }
    return droppedAny;
}

static int orch_drop_moving_fixed_possessions_compat(
    struct GameWorld_Compat* world,
    int creatureType,
    const unsigned char* cells,
    int cellCount,
    int mapIndex,
    int mapX,
    int mapY)
{
    int i;
    int droppedAny = 0;

    if (!cells || cellCount <= 0) return 1;
    /* ReDMCSB GROUP.C:F0187:670-672 consumes the moving fixed-possession
     * cell accumulator as a stack, passing C02_MODE_PLAY_ONE_TICK_LATER. */
    for (i = cellCount - 1; i >= 0; --i) {
        droppedAny |= orch_drop_creature_fixed_possessions_compat(
            world, creatureType, cells[i], mapIndex, mapX, mapY);
    }
    return droppedAny || cellCount == 0;
}

static int orch_drop_group_fixed_possessions_compat(
    struct GameWorld_Compat* world,
    const struct DungeonGroup_Compat* group,
    int mapIndex,
    int mapX,
    int mapY)
{
    int creatureIndex;
    int groupCells;

    if (!world || !group) return 0;
    creatureIndex = group->count;
    if (creatureIndex < 0) creatureIndex = 0;
    if (creatureIndex > 3) creatureIndex = 3;
    groupCells = group->cells;

    /* ReDMCSB GROUP.C:F0188:716-721 drops fixed possessions for each
     * creature still represented by Count/cells before dropping Slot. */
    do {
        int cell = (groupCells == 0xFF)
            ? DM1_SINGLE_CENTERED_CREATURE_CELL
            : orch_group_creature_cell_compat(group, creatureIndex);
        (void)orch_drop_creature_fixed_possessions_compat(
            world, group->creatureType, cell, mapIndex, mapX, mapY);
    } while (creatureIndex--);
    return 1;
}

static int orch_drop_group_slot_possessions_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    int mapIndex,
    int mapX,
    int mapY);

static int orch_drop_group_f0267_rejection_possessions_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const unsigned char* movingFixedDropCells,
    int movingFixedDropCellCount,
    int mapIndex,
    int mapX,
    int mapY)
{
    if (!world || !group) return 0;
    (void)orch_drop_moving_fixed_possessions_compat(
        world, group->creatureType, movingFixedDropCells,
        movingFixedDropCellCount, mapIndex, mapX, mapY);
    (void)orch_drop_group_fixed_possessions_compat(
        world, group, mapIndex, mapX, mapY);
    return orch_drop_group_slot_possessions_compat(world, group, mapIndex, mapX, mapY);
}

static int orch_damage_group_by_pit_fall_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    unsigned char movingFixedDropCells[4],
    int* movingFixedDropCellCount)
{
    int originalCount;
    int creatureIndex;
    int killedAny = 0;

    if (!world || !group) return 0;
    if (movingFixedDropCellCount) *movingFixedDropCellCount = 0;
    originalCount = group->count;
    if (originalCount < 0) originalCount = 0;
    if (originalCount > 3) originalCount = 3;

    /* ReDMCSB GROUP.C:F0191:958-968 resets the moving fixed-drop cell
     * accumulator, then damages each creature from Count down to 0 with
     * attack 20 adjusted by random(6). GROUP.C:F0190:831-847 records the
     * killed creature cell for moving fixed-possession drops instead of
     * dropping them on the source map; F0190:892-905 then compacts HP/cells. */
    for (creatureIndex = originalCount; creatureIndex >= 0; --creatureIndex) {
        int damage = 17 + F0732_COMBAT_RngRandom_Compat(&world->masterRng, 6);
        if (creatureIndex > group->count) continue;
        if ((int)group->health[creatureIndex] <= damage) {
            int currentCount = group->count;
            int shiftIndex;
            int killedCell = orch_group_creature_cell_compat(group, creatureIndex);
            killedAny = 1;
            if (currentCount <= 0) {
                group->health[0] = 0;
                return 2;
            }
            if (movingFixedDropCells && movingFixedDropCellCount &&
                *movingFixedDropCellCount < 4) {
                movingFixedDropCells[(*movingFixedDropCellCount)++] =
                    (unsigned char)(killedCell & 0xFF);
            }
            for (shiftIndex = creatureIndex; shiftIndex < currentCount; ++shiftIndex) {
                group->health[shiftIndex] = group->health[shiftIndex + 1];
                if (group->cells != 0xFFu) {
                    int nextCell = orch_group_creature_cell_compat(group, shiftIndex + 1);
                    group->cells = (unsigned char)orch_group_set_creature_cell_compat(
                        group->cells, shiftIndex, nextCell);
                }
            }
            group->health[currentCount] = 0;
            if (group->cells != 0xFFu) {
                group->cells = (unsigned char)(group->cells & 0x3Fu);
            }
            group->count = (unsigned char)(currentCount - 1);
        } else {
            group->health[creatureIndex] = (unsigned short)(group->health[creatureIndex] - damage);
        }
    }
    return killedAny ? 1 : 0;
}

static int orch_drop_group_slot_possessions_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    int mapIndex,
    int mapX,
    int mapY)
{
    int sftIndex;
    unsigned short thing;
    int safety = 0;

    if (!world || !group || !world->dungeon || !world->things) return 0;
    thing = group->slot;
    if (thing == THING_NONE || thing == THING_ENDOFLIST) return 1;

    sftIndex = orch_square_first_thing_list_index_compat(world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;

    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        unsigned short nextThing = orch_next_thing_compat(world->things, thing);
        unsigned short droppedThing = orch_thing_with_cell_compat(
            thing, F0732_COMBAT_RngRandom_Compat(&world->masterRng, 4));
        if (!orch_link_thing_to_square_tail_compat(
                world, mapIndex, mapX, mapY, droppedThing)) {
            return 0;
        }
        thing = nextThing;
    }
    group->slot = THING_ENDOFLIST;
    return safety < 64;
}

static void orch_cmd_attack_apply_f0190_possession_drops_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const struct CombatantCreatureSnapshot_Compat* creature,
    int killedCell,
    int targetDirection,
    int outcome)
{
    int mapIndex;
    int mapX;
    int mapY;

    if (!world || !group || !creature) return;
    if (outcome != COMBAT_OUTCOME_KILLED_SOME_CREATURES &&
        outcome != COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
        return;
    }

    orch_cmd_attack_target_square_compat(
        world, targetDirection, &mapIndex, &mapX, &mapY);
    if (outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
        /* ReDMCSB: GROUP.C F0190 lines ~824-829 calls F0188 before
         * F0189 for a not-moving single-creature group. F0188 drops
         * fixed possessions first, then the group's Slot chain. */
        (void)orch_drop_group_fixed_possessions_compat(
            world, group, mapIndex, mapX, mapY);
        (void)orch_drop_group_slot_possessions_compat(
            world, group, mapIndex, mapX, mapY);
        return;
    }

    if (creature->attributes & DM1_ATTR_DROP_FIXED_POSS) {
        /* ReDMCSB: GROUP.C F0190 lines ~831-847 drops the killed
         * creature's fixed possessions immediately when the damaged group
         * is not moving; moving groups record the cell for F0187 instead. */
        (void)orch_drop_creature_fixed_possessions_compat(
            world, creature->creatureType, killedCell, mapIndex, mapX, mapY);
    }
}

static int orch_cmd_attack_f0190_event_creature_index_compat(int eventType)
{
    if (eventType >= DM1_EVENT_UPDATE_ASPECT_CREATURE_0 &&
        eventType < DM1_EVENT_UPDATE_BEHAVIOR_GROUP) {
        return eventType - DM1_EVENT_UPDATE_ASPECT_CREATURE_0;
    }
    if (eventType >= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
        eventType <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
        return eventType - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    }
    return -1;
}

static void orch_cmd_attack_cleanup_f0190_creature_events_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    int killedCreatureIndex)
{
    int readIndex;
    int writeIndex = 0;
    int oldCount;
    int compactedCount;

    if (!world) return;
    if (killedCreatureIndex < 0 || killedCreatureIndex > 3) return;
    oldCount = world->timeline.count;

    /* ReDMCSB: GROUP.C F0190 lines ~848-872 deletes pending C33-C36 /
     * C38-C41 events for the killed creature and decrements the event type
     * for creatures that shift down during group compaction. */
    for (readIndex = 0; readIndex < oldCount; ++readIndex) {
        struct TimelineEvent_Compat ev = world->timeline.events[readIndex];
        if (ev.kind == TIMELINE_EVENT_CREATURE_REACTION &&
            ev.mapIndex == mapIndex && ev.mapX == mapX && ev.mapY == mapY) {
            int eventCreatureIndex =
                orch_cmd_attack_f0190_event_creature_index_compat(ev.aux2);
            if (eventCreatureIndex == killedCreatureIndex) {
                continue;
            }
            if (eventCreatureIndex > killedCreatureIndex) {
                ev.aux2--;
            }
        }
        world->timeline.events[writeIndex++] = ev;
    }
    compactedCount = writeIndex;
    while (writeIndex < oldCount) {
        memset(&world->timeline.events[writeIndex], 0,
               sizeof(world->timeline.events[writeIndex]));
        ++writeIndex;
    }
    world->timeline.count = compactedCount;
}

static int orch_cmd_attack_apply_f0190_fear_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const struct CombatantCreatureSnapshot_Compat* creature,
    int groupIndex,
    int originalGroupCount,
    int mapIndex,
    int mapX,
    int mapY)
{
    struct DM1GroupBehaviorContext_Compat ctx;
    int activeIndex;
    int shouldFlee = 0;
    int fleeDelay = 0;

    if (!world || !group || !creature) return 0;
    if (group->behavior != DM1_BEHAVIOR_ATTACK) return 0;
    if (mapIndex != world->partyMapIndex) return 0;

    memset(&ctx, 0, sizeof(ctx));
    ctx.currentMapIndex = mapIndex;
    ctx.currentGroupMapX = mapX;
    ctx.currentGroupMapY = mapY;
    ctx.partyMapIndex = world->party.mapIndex;
    ctx.partyMapX = world->party.mapX;
    ctx.partyMapY = world->party.mapY;
    ctx.creatureType = creature->creatureType;
    ctx.creatureInfo.properties = creature->properties;
    ctx.groupBehavior = group->behavior;
    ctx.creatureCount = originalGroupCount;

    if (!F0821_DM1_GROUP_ShouldFrighten_Compat(
            &ctx, originalGroupCount, &world->masterRng,
            &shouldFlee, &fleeDelay)) {
        return 0;
    }
    if (!shouldFlee) return 0;

    group->behavior = DM1_BEHAVIOR_FLEE;
    activeIndex = orch_find_active_group_state_index_compat(world, groupIndex);
    if (activeIndex >= 0) {
        world->creatureAI[activeIndex].stateKind = AI_STATE_FLEE;
        world->creatureAI[activeIndex].fearCounter = fleeDelay;
    }
    return 1;
}

static int orch_cmd_attack_apply_f0190_killed_some_state_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const struct CombatantCreatureSnapshot_Compat* creature,
    int groupIndex,
    int killedCreatureIndex,
    int originalGroupCount,
    int targetDirection,
    int outcome)
{
    int mapIndex;
    int mapX;
    int mapY;

    if (!world || !group || !creature) return 0;
    if (outcome != COMBAT_OUTCOME_KILLED_SOME_CREATURES) return 0;

    orch_cmd_attack_target_square_compat(
        world, targetDirection, &mapIndex, &mapX, &mapY);
    if (group->behavior == DM1_BEHAVIOR_ATTACK) {
        orch_cmd_attack_cleanup_f0190_creature_events_compat(
            world, mapIndex, mapX, mapY, killedCreatureIndex);
        return orch_cmd_attack_apply_f0190_fear_compat(
            world, group, creature, groupIndex, originalGroupCount,
            mapIndex, mapX, mapY);
    }
    return 0;
}

static int orch_resolve_group_f0267_pit_destination_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    int* inOutMapIndex,
    int* inOutMapX,
    int* inOutMapY,
    int* outFallKilledGroup,
    unsigned char movingFixedDropCells[4],
    int* movingFixedDropCellCount)
{
    int remaining;

    if (outFallKilledGroup) *outFallKilledGroup = 0;
    if (movingFixedDropCellCount) *movingFixedDropCellCount = 0;
    if (!world || !world->dungeon || !group || !inOutMapIndex || !inOutMapX || !inOutMapY) return 0;

    /* ReDMCSB MOVESENS.C:F0267:538-574 follows open, non-imaginary pits
     * for non-levitating groups through F0154 level-change coordinates.
     * Lines 608-624 damage the group by attack 20 at each fall and stop the
     * chain if all creatures die; lines 656-663 then drop possessions at the
     * final destination and prevent insertion. */
    for (remaining = 100; remaining > 0; --remaining) {
        const struct DungeonMapDesc_Compat* map;
        unsigned char squareByte;
        int squareType;
        int targetMapIndex;

        if (*inOutMapIndex < 0 || *inOutMapIndex >= (int)world->dungeon->header.mapCount) break;
        map = &world->dungeon->maps[*inOutMapIndex];
        if (*inOutMapX < 0 || *inOutMapX >= map->width ||
            *inOutMapY < 0 || *inOutMapY >= map->height) break;
        if (!world->dungeon->tiles || !world->dungeon->tiles[*inOutMapIndex].squareData) break;

        squareByte = world->dungeon->tiles[*inOutMapIndex].squareData[*inOutMapX * map->height + *inOutMapY];
        squareType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
        if (squareType != DUNGEON_ELEMENT_PIT || !(squareByte & 0x08) || (squareByte & 0x01)) break;

        targetMapIndex = orch_group_level_change_location_compat(
            world->dungeon, *inOutMapIndex, 1, inOutMapX, inOutMapY);
        if (targetMapIndex < 0 || targetMapIndex >= (int)world->dungeon->header.mapCount) break;
        *inOutMapIndex = targetMapIndex;

        {
            unsigned char fallDropCells[4];
            int fallDropCellCount = 0;
            int fallOutcome = orch_damage_group_by_pit_fall_compat(
                world, group, fallDropCells, &fallDropCellCount);
            int i;
            for (i = 0; movingFixedDropCells && movingFixedDropCellCount &&
                        i < fallDropCellCount && *movingFixedDropCellCount < 4; ++i) {
                movingFixedDropCells[(*movingFixedDropCellCount)++] = fallDropCells[i];
            }
            if (fallOutcome == 2) {
                if (outFallKilledGroup) *outFallKilledGroup = 1;
                return 1;
            }
        }
    }
    return 1;
}

static int orch_is_group_creature_allowed_on_map_compat(
    const struct GameWorld_Compat* world,
    const struct DungeonGroup_Compat* group,
    int mapIndex)
{
    const struct DungeonMapDesc_Compat* map;
    int i;

    if (!world || !world->dungeon || !group || !world->dungeon->maps) return 0;
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) return 0;
    map = &world->dungeon->maps[mapIndex];

    /* ReDMCSB DUNGEON.C:F0139:1050-1079 reads the group creature type and
     * scans the target map metadata creature list. MOVESENS.C:F0267:656-663
     * performs this check after teleporter/pit resolution even when the source
     * map X sentinel is CM1_MAPX_NOT_ON_A_SQUARE. */
    for (i = 0; i < (int)map->creatureTypeCount && i < 16; ++i) {
        if ((int)map->allowedCreatureTypes[i] == (int)group->creatureType) {
            return 1;
        }
    }
    return 0;
}

static int orch_is_lord_chaos_allowed_square_compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char squareByte;
    int squareType;

    if (!dungeon || !dungeon->tilesLoaded || !dungeon->maps || !dungeon->tiles) return 0;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return 0;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) return 0;
    if (!dungeon->tiles[mapIndex].squareData) return 0;

    /* ReDMCSB GROUP.C:F0223:162-170 allows Lord Chaos on raw square
     * types corridor, teleporter, pit, and door during event60/61 retry. */
    squareByte = dungeon->tiles[mapIndex].squareData[mapX * map->height + mapY];
    squareType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    return squareType == DUNGEON_ELEMENT_CORRIDOR ||
           squareType == DUNGEON_ELEMENT_TELEPORTER ||
           squareType == DUNGEON_ELEMENT_PIT ||
           squareType == DUNGEON_ELEMENT_DOOR;
}

static int orch_try_lord_chaos_random_adjacent_retry_compat(
    struct GameWorld_Compat* world,
    const struct DungeonGroup_Compat* group,
    const struct TimelineEvent_Compat* ev,
    int* outMapX,
    int* outMapY)
{
    int candidateX;
    int candidateY;

    if (!world || !group || !ev || !outMapX || !outMapY) return 0;
    if (group->creatureType != 23) return 0; /* C23_CREATURE_LORD_CHAOS */
    if (F0732_COMBAT_RngRandom_Compat(&world->masterRng, 4) != 0) return 0;

    candidateX = ev->mapX;
    candidateY = ev->mapY;
    switch (F0732_COMBAT_RngRandom_Compat(&world->masterRng, 4)) {
        case 0: candidateX--; break;
        case 1: candidateX++; break;
        case 2: candidateY--; break;
        case 3: candidateY++; break;
    }
    if (!orch_is_lord_chaos_allowed_square_compat(
            world->dungeon, ev->mapIndex, candidateX, candidateY)) {
        return 0;
    }

    *outMapX = candidateX;
    *outMapY = candidateY;
    return 1;
}

static int orch_add_generated_group_active_state_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    const struct DungeonGroup_Compat* group,
    int mapIndex,
    int mapX,
    int mapY)
{
    struct CreatureAIState_Compat* ai;
    if (!world || !group) return 0;
    if (mapIndex != world->partyMapIndex) return 1;
    if (world->creatureAICount < 0 ||
        world->creatureAICount >= GAMEWORLD_CREATURE_AI_CAPACITY) {
        return 0;
    }

    /* ReDMCSB GROUP.C:414-447/F0183 creates ACTIVE_GROUP state for a
     * group that arrives on the party map.  Phase 20 stores the closest
     * persistent active-group analogue in creatureAI[]. */
    ai = &world->creatureAI[world->creatureAICount++];
    memset(ai, 0, sizeof(*ai));
    ai->stateKind = AI_STATE_WANDER;
    ai->creatureType = group->creatureType;
    ai->groupMapIndex = mapIndex;
    ai->groupMapX = mapX;
    ai->groupMapY = mapY;
    ai->groupCells = group->cells;
    ai->groupDirection = group->direction;
    ai->targetChampionIndex = -1;
    ai->lastSeenPartyMapX = -1;
    ai->lastSeenPartyMapY = -1;
    ai->lastSeenPartyTick = -1;
    ai->reserved0 = groupIndex;
    return 1;
}

static void orch_schedule_generated_group_wandering_event_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    const struct DungeonGroup_Compat* group,
    int mapIndex,
    int mapX,
    int mapY)
{
    struct TimelineEvent_Compat wander;
    if (!world || !group) return;

    /* ReDMCSB GROUP.C:311-338/F0180: newly placed groups start
     * wandering by scheduling C37 for game time +1 on their square. */
    memset(&wander, 0, sizeof(wander));
    wander.kind = TIMELINE_EVENT_CREATURE_TICK;
    wander.fireAtTick = world->gameTick + 1u;
    wander.mapIndex = mapIndex;
    wander.mapX = mapX;
    wander.mapY = mapY;
    wander.aux0 = groupIndex;
    wander.aux1 = group->creatureType;
    wander.aux2 = AI_STATE_WANDER;
    (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &wander);
}

static int orch_link_existing_group_to_square_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    int mapIndex,
    int mapX,
    int mapY)
{
    struct DungeonGroup_Compat* group;
    int sftIndex;

    if (!world || !world->dungeon || !world->things) return 0;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;

    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;

    group = &world->things->groups[groupIndex];
    group->next = world->things->squareFirstThings[sftIndex];
    world->things->squareFirstThings[sftIndex] =
        orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex);

    (void)orch_add_generated_group_active_state_compat(
        world, groupIndex, group, mapIndex, mapX, mapY);
    orch_schedule_generated_group_wandering_event_compat(
        world, groupIndex, group, mapIndex, mapX, mapY);
    return 1;
}

static int orch_schedule_deferred_group_move_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    int groupIndex,
    int audible)
{
    struct TimelineEvent_Compat deferred;

    if (!world || !ev) return 0;

    /* ReDMCSB MOVESENS.C:F0265:169-192 creates C60/C61 at
     * G0313_ul_GameTime + 5 with destination map/x/y and the group thing
     * slot; MOVESENS.C:F0267:830-844 calls it when insertion is blocked
     * by the party or another group. */
    memset(&deferred, 0, sizeof(deferred));
    deferred.kind = audible
        ? TIMELINE_EVENT_MOVE_GROUP_AUDIBLE
        : TIMELINE_EVENT_MOVE_GROUP_SILENT;
    deferred.fireAtTick = world->gameTick + 5u;
    deferred.mapIndex = ev->mapIndex;
    deferred.mapX = ev->mapX;
    deferred.mapY = ev->mapY;
    deferred.aux0 = groupIndex;
    return F0721_TIMELINE_Schedule_Compat(&world->timeline, &deferred);
}

static int orch_materialize_generated_group_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    const struct GeneratorResult_Compat* generator,
    int* outGroupIndex,
    struct OrchTeleporterBuzzList_Compat* outTeleporterBuzzes)
{
    struct DungeonGroup_Compat* group;
    int sftIndex;
    int groupIndex;
    int i;

    if (outGroupIndex) *outGroupIndex = -1;
    orch_teleporter_buzz_list_init_compat(outTeleporterBuzzes);
    if (!world || !ev || !generator || !world->dungeon || !world->things) return 0;
    if (!generator->spawned) return 0;

    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, ev->mapIndex, ev->mapX, ev->mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;

    groupIndex = orch_find_unused_group_slot_compat(world->things);
    if (groupIndex < 0) return 0;
    group = &world->things->groups[groupIndex];
    memset(group, 0, sizeof(*group));

    group->next = THING_ENDOFLIST;
    group->slot = THING_ENDOFLIST;
    group->creatureType = (unsigned char)(generator->spawnedCreatureType & 0xFF);
    group->cells = (unsigned char)(generator->spawnedGroupCells & 0xFF);
    for (i = 0; i < 4; ++i) {
        int hp = generator->spawnedGroupHealth[i];
        if (hp < 0) hp = 0;
        if (hp > 0xFFFF) hp = 0xFFFF;
        group->health[i] = (unsigned short)hp;
    }
    group->behavior = 0; /* C0_BEHAVIOR_WANDER */
    group->count = (unsigned char)(generator->spawnedCreatureCount & 0x03);
    group->direction = (unsigned char)(generator->spawnedDirection & 0x03);
    group->doNotDiscard = 0;

    /* ReDMCSB GROUP.C:F0185:543-545 keeps the initialized group slot
     * referenced by the deferred move event instead of linking it when
     * F0267 reports a blocked destination. */
    {
        int destMapIndex = ev->mapIndex;
        int destMapX = ev->mapX;
        int destMapY = ev->mapY;
        int fallKilledGroup = 0;
        unsigned char movingFixedDropCells[4];
        int movingFixedDropCellCount = 0;
        struct TimelineEvent_Compat resolvedEvent = *ev;

        (void)orch_resolve_group_f0267_teleporter_destination_compat(
            world, &destMapIndex, &destMapX, &destMapY,
            outTeleporterBuzzes);
        if (!orch_resolve_group_f0267_pit_destination_compat(
                world, group, &destMapIndex, &destMapX, &destMapY,
                &fallKilledGroup, movingFixedDropCells,
                &movingFixedDropCellCount)) {
            return 0;
        }
        resolvedEvent.mapIndex = destMapIndex;
        resolvedEvent.mapX = destMapX;
        resolvedEvent.mapY = destMapY;
        if (fallKilledGroup) {
            if (!orch_drop_group_f0267_rejection_possessions_compat(
                    world, group, movingFixedDropCells, movingFixedDropCellCount,
                    destMapIndex, destMapX, destMapY)) {
                return 0;
            }
            if (outGroupIndex) *outGroupIndex = groupIndex;
            return 0;
        }
        (void)orch_drop_moving_fixed_possessions_compat(
            world, group->creatureType, movingFixedDropCells,
            movingFixedDropCellCount, destMapIndex, destMapX, destMapY);
        if (!orch_is_group_creature_allowed_on_map_compat(world, group, destMapIndex)) {
            if (!orch_drop_group_f0267_rejection_possessions_compat(
                    world, group, NULL, 0, destMapIndex, destMapX, destMapY)) {
                return 0;
            }
            if (outGroupIndex) *outGroupIndex = groupIndex;
            return 0;
        }

        if (orch_square_has_group_or_party_compat(world, destMapIndex, destMapX, destMapY)) {
            if (!orch_schedule_deferred_group_move_compat(world, &resolvedEvent, groupIndex, 0)) {
                return 0;
            }
            if (outGroupIndex) *outGroupIndex = groupIndex;
            return 0;
        }

        if (!orch_link_existing_group_to_square_compat(
                world, groupIndex, destMapIndex, destMapX, destMapY)) {
            return 0;
        }
        if (outGroupIndex) *outGroupIndex = groupIndex;
        return 1;
    }
}

static int orch_handle_deferred_group_move_event_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result)
{
    int groupIndex;
    int targetMapX;
    int targetMapY;
    struct DungeonGroup_Compat* group;
    struct TimelineEvent_Compat retry;
    struct OrchTeleporterBuzzList_Compat teleporterBuzzes;

    if (!world || !ev || !result || !world->things) return 0;
    groupIndex = ev->aux0;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;
    group = &world->things->groups[groupIndex];
    retry = *ev;
    targetMapX = ev->mapX;
    targetMapY = ev->mapY;

    /* ReDMCSB TIMELINE.C:F0252:1527-1567 inserts the deferred group
     * only when the destination is clear.  If C23 Lord Chaos is blocked,
     * lines 1536-1555 give one 1/4-chance random adjacent insertion
     * attempt before the lines 1565-1567 retry at +5 ticks.  When clear,
     * line 1534 re-enters MOVESENS.C:F0267 with a non-square source, which
     * also skips MOVESENS.C:F0266 projectile impact/removal via the
     * SourceMapX >= 0 guard at F0267:432-435.  The teleporter destination
     * helper above covers the narrow C006 group teleporter/cross-map subcase
     * before the final insertion. */
    (void)orch_resolve_group_f0267_teleporter_destination_compat(
        world, &retry.mapIndex, &targetMapX, &targetMapY,
        &teleporterBuzzes);
    {
        int fallKilledGroup = 0;
        unsigned char movingFixedDropCells[4];
        int movingFixedDropCellCount = 0;
        if (!orch_resolve_group_f0267_pit_destination_compat(
                world, group, &retry.mapIndex, &targetMapX, &targetMapY,
                &fallKilledGroup, movingFixedDropCells,
                &movingFixedDropCellCount)) {
            return 0;
        }
        if (fallKilledGroup) {
            if (ev->kind == TIMELINE_EVENT_MOVE_GROUP_AUDIBLE) {
                emit(result, EMIT_SOUND_REQUEST, DM1_SND_BUZZ,
                     targetMapX, targetMapY, retry.mapIndex);
            }
            orch_emit_teleporter_buzzes_compat(result, &teleporterBuzzes);
            return orch_drop_group_f0267_rejection_possessions_compat(
                world, group, movingFixedDropCells, movingFixedDropCellCount,
                retry.mapIndex, targetMapX, targetMapY);
        }
        (void)orch_drop_moving_fixed_possessions_compat(
            world, group->creatureType, movingFixedDropCells,
            movingFixedDropCellCount, retry.mapIndex, targetMapX, targetMapY);
        if (!orch_is_group_creature_allowed_on_map_compat(world, group, retry.mapIndex)) {
            if (ev->kind == TIMELINE_EVENT_MOVE_GROUP_AUDIBLE) {
                emit(result, EMIT_SOUND_REQUEST, DM1_SND_BUZZ,
                     targetMapX, targetMapY, retry.mapIndex);
            }
            orch_emit_teleporter_buzzes_compat(result, &teleporterBuzzes);
            return orch_drop_group_f0267_rejection_possessions_compat(
                world, group, NULL, 0, retry.mapIndex, targetMapX, targetMapY);
        }
    }

    if (orch_square_has_group_or_party_compat(world, retry.mapIndex, targetMapX, targetMapY)) {
        if (orch_try_lord_chaos_random_adjacent_retry_compat(
                world, group, ev, &targetMapX, &targetMapY) &&
            !orch_square_has_group_or_party_compat(
                world, ev->mapIndex, targetMapX, targetMapY)) {
            if (ev->kind == TIMELINE_EVENT_MOVE_GROUP_AUDIBLE) {
                emit(result, EMIT_SOUND_REQUEST, DM1_SND_BUZZ,
                     targetMapX, targetMapY, ev->mapIndex);
            }
            return orch_link_existing_group_to_square_compat(
                world, groupIndex, ev->mapIndex, targetMapX, targetMapY);
        }

        retry.fireAtTick = ev->fireAtTick + 5u;
        retry.mapX = targetMapX;
        retry.mapY = targetMapY;
        return F0721_TIMELINE_Schedule_Compat(&world->timeline, &retry);
    }

    if (ev->kind == TIMELINE_EVENT_MOVE_GROUP_AUDIBLE) {
        emit(result, EMIT_SOUND_REQUEST, DM1_SND_BUZZ,
             targetMapX, targetMapY, retry.mapIndex);
    }
    orch_emit_teleporter_buzzes_compat(result, &teleporterBuzzes);
    return orch_link_existing_group_to_square_compat(
        world, groupIndex, retry.mapIndex, targetMapX, targetMapY);
}


static int orch_link_existing_group_to_square_head_only_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    int mapIndex,
    int mapX,
    int mapY)
{
    struct DungeonGroup_Compat* group;
    int sftIndex;

    if (!world || !world->dungeon || !world->things) return 0;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;
    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;

    group = &world->things->groups[groupIndex];
    group->next = world->things->squareFirstThings[sftIndex];
    world->things->squareFirstThings[sftIndex] =
        orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex);
    return 1;
}

static int orch_handle_creature_tick_group_move_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result)
{
    int groupIndex;
    int activeIndex;
    int direction;
    int destMapX;
    int destMapY;
    int killedByProjectile = 0;
    struct DungeonGroup_Compat* group;
    struct TimelineEvent_Compat nextEvent;

    (void)result;
    if (!world || !ev || !world->things || !world->dungeon) return 0;
    groupIndex = ev->aux0;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount || !world->things->groups) return 0;
    activeIndex = orch_find_active_group_state_index_compat(world, groupIndex);
    if (activeIndex < 0) return 1;
    group = &world->things->groups[groupIndex];
    direction = world->creatureAI[activeIndex].groupDirection & 3;
    destMapX = ev->mapX;
    destMapY = ev->mapY;
    switch (direction) {
        case DIR_NORTH: destMapY--; break;
        case DIR_EAST:  destMapX++; break;
        case DIR_SOUTH: destMapY++; break;
        case DIR_WEST:  destMapX--; break;
    }

    if (!F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            world->dungeon, ev->mapIndex, destMapX, destMapY,
            MOVEMENT_PASS_CTX_CREATURE) ||
        orch_square_has_group_or_party_compat(world, ev->mapIndex, destMapX, destMapY)) {
        nextEvent = *ev;
        nextEvent.fireAtTick = world->gameTick + 1u;
        return F0721_TIMELINE_Schedule_Compat(&world->timeline, &nextEvent);
    }

    if (!orch_apply_f0266_group_projectile_precheck_compat(
            world, groupIndex, ev->mapIndex, ev->mapX, ev->mapY,
            destMapX, destMapY, &killedByProjectile)) {
        return 0;
    }
    if (killedByProjectile) {
        (void)orch_unlink_thing_from_square_compat(
            world, ev->mapIndex, ev->mapX, ev->mapY,
            orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex));
        group->next = THING_NONE;
        orch_remove_active_group_state_compat(world, groupIndex);
        return 1;
    }

    if (!orch_unlink_thing_from_square_compat(
            world, ev->mapIndex, ev->mapX, ev->mapY,
            orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex))) {
        return 0;
    }
    group->direction = (unsigned char)direction;
    if (!orch_link_existing_group_to_square_head_only_compat(
            world, groupIndex, ev->mapIndex, destMapX, destMapY)) {
        return 0;
    }

    world->creatureAI[activeIndex].groupMapIndex = ev->mapIndex;
    world->creatureAI[activeIndex].groupMapX = destMapX;
    world->creatureAI[activeIndex].groupMapY = destMapY;
    world->creatureAI[activeIndex].groupCells = group->cells;

    nextEvent = *ev;
    nextEvent.fireAtTick = world->gameTick + 1u;
    nextEvent.mapX = destMapX;
    nextEvent.mapY = destMapY;
    return F0721_TIMELINE_Schedule_Compat(&world->timeline, &nextEvent);
}

static int orch_ai_state_to_dm1_behavior_compat(int stateKind)
{
    switch (stateKind) {
        case AI_STATE_ATTACK: return DM1_BEHAVIOR_ATTACK;
        case AI_STATE_APPROACH: return DM1_BEHAVIOR_APPROACH;
        case AI_STATE_FLEE: return DM1_BEHAVIOR_FLEE;
        case AI_STATE_WANDER:
        case AI_STATE_IDLE:
        default:
            return DM1_BEHAVIOR_WANDER;
    }
}

static int orch_dm1_behavior_to_ai_state_compat(int behavior)
{
    switch (behavior) {
        case DM1_BEHAVIOR_ATTACK: return AI_STATE_ATTACK;
        case DM1_BEHAVIOR_APPROACH: return AI_STATE_APPROACH;
        case DM1_BEHAVIOR_FLEE: return AI_STATE_FLEE;
        case DM1_BEHAVIOR_WANDER:
        default:
            return AI_STATE_WANDER;
    }
}

static int orch_handle_creature_reaction_event_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result)
{
    int groupIndex;
    int activeIndex;
    struct DungeonGroup_Compat* group;
    struct CreatureAIState_Compat* ai;
    struct DM1GroupBehaviorContext_Compat ctx;
    struct DM1ActiveGroup_Compat activeGroup;
    struct DM1BehaviorResult_Compat behavior;
    struct TimelineEvent_Compat next;

    (void)result;
    if (!world || !ev || !world->things || !world->things->groups) return 0;
    groupIndex = ev->aux0;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;
    group = &world->things->groups[groupIndex];
    if (group->next == THING_NONE) return 1;

    activeIndex = orch_find_active_group_state_index_compat(world, groupIndex);
    if (activeIndex < 0) {
        (void)orch_add_generated_group_active_state_compat(
            world, groupIndex, group, ev->mapIndex, ev->mapX, ev->mapY);
        activeIndex = orch_find_active_group_state_index_compat(world, groupIndex);
    }
    if (activeIndex < 0) return 1;

    ai = &world->creatureAI[activeIndex];
    memset(&ctx, 0, sizeof(ctx));
    memset(&activeGroup, 0, sizeof(activeGroup));
    memset(&behavior, 0, sizeof(behavior));

    ctx.currentGroupMapX = ev->mapX;
    ctx.currentGroupMapY = ev->mapY;
    ctx.partyMapX = world->party.mapX;
    ctx.partyMapY = world->party.mapY;
    ctx.partyMapIndex = world->party.mapIndex;
    ctx.currentMapIndex = ev->mapIndex;
    ctx.partyChampionCount = world->party.championCount;
    ctx.creatureType = group->creatureType;
    ctx.groupBehavior = orch_ai_state_to_dm1_behavior_compat(ai->stateKind);
    ctx.creatureCount = group->count;
    ctx.movementTicks = 1;
    ctx.currentTickLow = (int)world->gameTick;
    ctx.eventType = ev->aux2;
    ctx.eventTicks = (int)ev->fireAtTick;

    activeGroup.groupThingIndex = groupIndex;
    activeGroup.cells = group->cells;
    activeGroup.directions = group->direction;
    activeGroup.lastMoveTime = ai->lastSeenPartyTick;
    activeGroup.targetMapX = ai->lastSeenPartyMapX;
    activeGroup.targetMapY = ai->lastSeenPartyMapY;
    activeGroup.priorMapX = ai->groupMapX;
    activeGroup.priorMapY = ai->groupMapY;

    /* ReDMCSB: PROJEXPL.C F0231 calls GROUP.C F0209 with
     * CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT unless the
     * whole group died.  The scheduler has already converted CM1 into the
     * concrete C31 reaction event; this dispatch applies F0209's C31 branch
     * to the active-group analogue. */
    if (!F0810_DM1_GROUP_DispatchBehavior_Compat(
            &ctx, &activeGroup, &world->masterRng, &behavior)) {
        return 0;
    }

    ai->stateKind = orch_dm1_behavior_to_ai_state_compat(behavior.newBehavior);
    ai->groupMapIndex = ev->mapIndex;
    ai->groupMapX = ev->mapX;
    ai->groupMapY = ev->mapY;
    ai->groupCells = group->cells;
    ai->lastSeenPartyMapX = activeGroup.targetMapX;
    ai->lastSeenPartyMapY = activeGroup.targetMapY;
    ai->lastSeenPartyTick = (int)world->gameTick;
    group->behavior = (unsigned char)(behavior.newBehavior & 0xFF);

    if (behavior.nextEventDelayTicks > 0 && behavior.nextEventType > 0) {
        memset(&next, 0, sizeof(next));
        next.kind = TIMELINE_EVENT_CREATURE_REACTION;
        next.fireAtTick = world->gameTick + (uint32_t)behavior.nextEventDelayTicks;
        next.mapIndex = ev->mapIndex;
        next.mapX = ev->mapX;
        next.mapY = ev->mapY;
        next.aux0 = groupIndex;
        next.aux1 = group->creatureType;
        next.aux2 = behavior.nextEventType;
        (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &next);
    }
    return 1;
}

static int orch_handle_group_generator_trigger_runtime_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result)
{
    int sensorIndex = -1;
    const struct DungeonSensor_Compat* sensor;
    struct GeneratorContext_Compat ctx;
    struct GeneratorResult_Compat generator;
    struct OrchTeleporterBuzzList_Compat teleporterBuzzes;

    if (!world || !ev || !result || !world->dungeon || !world->things) return 0;
    if (!orch_find_generator_sensor_on_square_compat(
            world->dungeon, world->things, ev->mapIndex, ev->mapX, ev->mapY,
            &sensorIndex)) {
        return 0;
    }
    if (sensorIndex < 0 || sensorIndex >= world->things->sensorCount) return 0;

    sensor = &world->things->sensors[sensorIndex];
    memset(&ctx, 0, sizeof(ctx));
    ctx.sensorIndex = sensorIndex;
    ctx.mapIndex = ev->mapIndex;
    ctx.mapX = ev->mapX;
    ctx.mapY = ev->mapY;
    ctx.creatureType = (int)sensor->sensorData;
    ctx.creatureCountRaw = (int)sensor->value;
    ctx.randomizeCount = (sensor->value & 0x08u) ? 1 : 0;
    ctx.healthMultiplier = (int)(sensor->localMultiple & 0x000Fu);
    ctx.ticksRaw = (int)((sensor->localMultiple >> 4) & 0x00FFu);
    ctx.onceOnly = sensor->onceOnly ? 1 : 0;
    ctx.audible = sensor->audible ? 1 : 0;
    ctx.mapDifficulty = (world->dungeon->maps && ev->mapIndex >= 0 &&
                         ev->mapIndex < (int)world->dungeon->header.mapCount)
        ? (int)world->dungeon->maps[ev->mapIndex].difficulty
        : 1;
    ctx.isOnPartyMap = (ev->mapIndex == world->partyMapIndex) ? 1 : 0;
    ctx.currentActiveGroupCount = world->creatureAICount;
    ctx.maxActiveGroupCount = 60;

    if (!F0860_RUNTIME_HandleGroupGenerator_Compat(
            &ctx, &world->masterRng, world->gameTick, &generator)) {
        return 0;
    }

    if (generator.sensorDisabled) {
        world->things->sensors[sensorIndex].sensorType = RUNTIME_SENSOR_TYPE_DISABLED;
    }
    if (generator.reEnableScheduled) {
        (void)F0721_TIMELINE_Schedule_Compat(
            &world->timeline, &generator.reEnableEvent);
    }

    {
        int placed = orch_materialize_generated_group_compat(
            world, ev, &generator, 0, &teleporterBuzzes);
        orch_emit_teleporter_buzzes_compat(result, &teleporterBuzzes);
        if (placed) {
            /* ReDMCSB GROUP.C:543-547/F0185: successful placement requests
             * M560_SOUND_BUZZ independently of the sensor's Audible flag. */
            emit(result, EMIT_SOUND_REQUEST, DM1_SND_BUZZ,
                 ev->mapX, ev->mapY, ev->mapIndex);
        }
    }
    if (generator.soundRequested) {
        /* ReDMCSB TIMELINE.C:975-977/F0245: sensor Audible requests a
         * second prioritized M560_SOUND_BUZZ after generation. */
        emit(result, EMIT_SOUND_REQUEST, DM1_SND_BUZZ, ev->mapX, ev->mapY, ev->mapIndex);
    }
    return 1;
}

static int orch_find_material_group_on_square_compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    int* outGroupIndex,
    int* outCreatureHeight)
{
    int sftIndex;
    unsigned short thing;
    int safety = 0;
    if (outGroupIndex) *outGroupIndex = -1;
    if (outCreatureHeight) *outCreatureHeight = 0;
    if (!dungeon || !things || !things->loaded || !things->squareFirstThings) return 0;
    sftIndex = orch_square_first_thing_list_index_compat(dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= things->squareFirstThingCount) return 0;
    thing = things->squareFirstThings[sftIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        if (type == THING_TYPE_GROUP && index >= 0 && index < things->groupCount) {
            const struct DungeonGroup_Compat* group = &things->groups[index];
            int creatureType = group->creatureType;
            unsigned short attributes = 0;
            if (creatureType >= 0 && creatureType < 27) {
                attributes = s_dm1_i34_creature_attributes[creatureType];
            }
            if ((attributes & CREATURE_ATTR_MASK_NON_MATERIAL) == 0) {
                if (outGroupIndex) *outGroupIndex = index;
                if (outCreatureHeight) *outCreatureHeight = (int)((attributes >> 7) & 0x0003u);
                return 1;
            }
        }
        thing = orch_next_thing_compat(things, thing);
    }
    return 0;
}

static int orch_damage_group_all_creatures_compat(
    struct DungeonGroup_Compat* group,
    int damage)
{
    int i;
    int damaged = 0;
    if (!group || damage <= 0) return 0;
    for (i = 0; i <= (int)group->count && i < 4; ++i) {
        if (group->health[i] > 0) {
            group->health[i] = (group->health[i] > (unsigned int)damage)
                ? (unsigned short)(group->health[i] - damage)
                : 0;
            damaged++;
        }
    }
    return damaged;
}

static int cmd_to_move_action(uint8_t cmd, int partyDirection, int* outSetDir) {
    *outSetDir = -1;
    switch (cmd) {
        case CMD_MOVE_NORTH: return (partyDirection == DIR_NORTH) ? MOVE_FORWARD :
                                    (partyDirection == DIR_SOUTH) ? MOVE_BACKWARD :
                                    (partyDirection == DIR_EAST)  ? MOVE_LEFT : MOVE_RIGHT;
        case CMD_MOVE_EAST:  return (partyDirection == DIR_EAST)  ? MOVE_FORWARD :
                                    (partyDirection == DIR_WEST)  ? MOVE_BACKWARD :
                                    (partyDirection == DIR_NORTH) ? MOVE_RIGHT : MOVE_LEFT;
        case CMD_MOVE_SOUTH: return (partyDirection == DIR_SOUTH) ? MOVE_FORWARD :
                                    (partyDirection == DIR_NORTH) ? MOVE_BACKWARD :
                                    (partyDirection == DIR_WEST)  ? MOVE_RIGHT : MOVE_LEFT;
        case CMD_MOVE_WEST:  return (partyDirection == DIR_WEST)  ? MOVE_FORWARD :
                                    (partyDirection == DIR_EAST)  ? MOVE_BACKWARD :
                                    (partyDirection == DIR_SOUTH) ? MOVE_RIGHT : MOVE_LEFT;
        case CMD_TURN_LEFT:  return MOVE_TURN_LEFT;
        case CMD_TURN_RIGHT: return MOVE_TURN_RIGHT;
        default: return -1;
    }
}

static int movement_action_absolute_direction(int partyDirection, int moveAction) {
    partyDirection &= 3;
    switch (moveAction) {
        case MOVE_FORWARD:  return partyDirection;
        case MOVE_RIGHT:    return (partyDirection + 1) & 3;
        case MOVE_BACKWARD: return (partyDirection + 2) & 3;
        case MOVE_LEFT:     return (partyDirection + 3) & 3;
        default:            return -1;
    }
}

static int redmcsb_party_move_cooldown_ticks_compat(
    const struct PartyState_Compat* party)
{
    int i;
    int ticks = 1;

    if (!party) return ticks;

    /*
     * ReDMCSB source-lock: CLIKMENU.C:330-346 starts AL1115_ui_Ticks
     * at 1, then for each living party champion takes the max of
     * F0310_CHAMPION_GetMovementTicks before assigning
     * G0310_i_DisabledMovementTicks and clearing
     * G0311_i_ProjectileDisabledMovementTicks.  F0310 is ported as
     * F0841_LIFECYCLE_ComputeMoveTicks_Compat (CHAMPION.C:1180-1215).
     */
    for (i = 0; i < party->championCount && i < CHAMPION_MAX_PARTY; ++i) {
        const struct ChampionState_Compat* c = &party->champions[i];
        uint16_t championTicks;
        if (!c->present || c->hp.current == 0) continue;
        championTicks = F0841_LIFECYCLE_ComputeMoveTicks_Compat(
            c->load, c->maxLoad, c->wounds, LIFECYCLE_ICON_NONE);
        if ((int)championTicks > ticks) ticks = (int)championTicks;
    }
    return ticks;
}

static int movement_command_disabled_redmcsb_compat(
    const struct GameWorld_Compat* world,
    int moveAction)
{
    int absoluteDirection;
    if (!world) return 0;
    absoluteDirection = movement_action_absolute_direction(world->party.direction, moveAction);
    if (absoluteDirection < 0) return 0;
    /*
     * ReDMCSB source-lock: COMMAND.C:2095-2100 / 2104-2110 checks only
     * C003..C006 movement commands before dispatch.  A non-zero
     * G0310_i_DisabledMovementTicks suppresses all movement commands; a
     * non-zero G0311_i_ProjectileDisabledMovementTicks suppresses only the
     * movement whose absolute direction equals
     * G0312_i_LastProjectileDisabledMovementDirection.  GAMELOOP.C:150-155
     * decrements both cooldowns once per game tick.
     */
    if (world->disabledMovementTicks > 0) return 1;
    return world->projectileDisabledMovementTicks > 0 &&
           ((world->lastProjectileDisabledMovementDirection & 3) == absoluteDirection);
}

int F0888_ORCH_ApplyPlayerInput_Compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* input,
    struct TickResult_Compat* result)
{
    if (!world || !input) return 0;
    if (input->command == CMD_NONE) return 0;

    switch (input->command) {
    case CMD_MOVE_NORTH:
    case CMD_MOVE_EAST:
    case CMD_MOVE_SOUTH:
    case CMD_MOVE_WEST:
    case CMD_TURN_LEFT:
    case CMD_TURN_RIGHT: {
        int ignore;
        int mv = cmd_to_move_action(input->command, world->party.direction, &ignore);
        if (mv < 0) return 0;
        if (movement_command_disabled_redmcsb_compat(world, mv)) return 0;
        if (!world->dungeon) {
            /* no dungeon: succeed deterministically (unit-test path) */
            if (mv == MOVE_TURN_LEFT || mv == MOVE_TURN_RIGHT) {
                set_party_direction_redmcsb_compat(&world->party,
                    F0700_MOVEMENT_TurnDirection_Compat(world->party.direction, mv == MOVE_TURN_RIGHT));
            }
            return 1;
        } else {
            struct MovementResult_Compat mr;
            memset(&mr, 0, sizeof(mr));
            if (F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(
                    world->dungeon, world->things, &world->party, mv)) {
                /* ReDMCSB source-lock: CLIKMENU.C:291-318 checks
                 * F0175_GROUP_GetThing after wall/door/fake-wall legality
                 * and before F0267_MOVE_GetMoveResult_CPSCE.  A group
                 * collision discards queued input and returns without moving
                 * or setting G0310_i_DisabledMovementTicks.  This tick
                 * orchestrator has one command at a time, so the equivalent
                 * observable is: reject the command, leave party/cooldowns
                 * unchanged, and emit nothing. */
                return 0;
            }
            F0702_MOVEMENT_TryMove_Compat(world->dungeon, &world->party, mv, &mr);
            if (mr.resultCode == MOVE_TURN_ONLY) {
                /* ReDMCSB source-lock: COMMAND.C:2150-2152 routes
                 * C001/C002 turn commands to F0365_COMMAND_ProcessTypes1To2_TurnParty;
                 * CLIKMENU.C:171-173 processes walk-off/walk-on sensors around
                 * F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(...)).
                 * A turn therefore mutates G0308_i_PartyDirection without
                 * requiring map-coordinate movement. */
                set_party_direction_redmcsb_compat(&world->party, mr.newDirection);
                emit(result, EMIT_PARTY_MOVED,
                     world->party.mapX, world->party.mapY,
                     world->party.direction, world->party.mapIndex);
                return 1;
            }
            if (mr.resultCode == MOVE_OK) {
                struct PartyState_Compat movedParty = world->party;
                struct PostMoveResolution_Compat postMove;
                int oldMapX = world->party.mapX;
                int oldMapY = world->party.mapY;
                int oldMapIndex = world->party.mapIndex;
                int i;

                memset(&postMove, 0, sizeof(postMove));
                movedParty.mapX = mr.newMapX;
                movedParty.mapY = mr.newMapY;
                movedParty.direction = mr.newDirection;
                movedParty.mapIndex = mr.newMapIndex;
                (void)F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat(
                    world->dungeon,
                    world->things,
                    &movedParty,
                    world->gameTick,
                    &postMove);

                world->party.mapX = postMove.finalMapX;
                world->party.mapY = postMove.finalMapY;
                set_party_direction_redmcsb_compat(&world->party, postMove.finalDirection);
                world->party.mapIndex = postMove.finalMapIndex;
                for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
                    if (postMove.championFallDamage[i] > 0 &&
                        world->party.champions[i].present &&
                        world->party.champions[i].hp.current > 0) {
                        int hp = (int)world->party.champions[i].hp.current - postMove.championFallDamage[i];
                        world->party.champions[i].hp.current = (int16_t)((hp > 0) ? hp : 0);
                    }
                }
                if (postMove.pitCount > 0) {
                    emit(result, EMIT_PARTY_FELL,
                         world->party.mapIndex, world->party.mapX,
                         world->party.mapY, postMove.pitCount);
                }
                if (postMove.teleporterCount > 0) {
                    emit(result, EMIT_PARTY_TELEPORTED,
                         world->party.mapIndex, world->party.mapX,
                         world->party.mapY, postMove.teleporterCount);
                }
                if (postMove.teleporterAudibleCount > 0) {
                    emit(result, EMIT_SOUND_REQUEST, DM1_SND_BUZZ,
                         world->party.mapX, world->party.mapY,
                         world->party.mapIndex);
                }
                world->disabledMovementTicks = redmcsb_party_move_cooldown_ticks_compat(&world->party);
                world->projectileDisabledMovementTicks = 0;
                emit(result, EMIT_PARTY_MOVED,
                     world->party.mapX, world->party.mapY,
                     world->party.direction, world->party.mapIndex);
                /*
                 * Pass 37 — sensor enter/leave runtime wiring.
                 *
                 * Run F0718_SENSOR_ProcessPartyEnterLeave_Compat for the
                 * pre-move square (WALK_OFF) and the post-resolve final
                 * square (WALK_ON) and surface each produced effect as a
                 * distinct EMIT_SENSOR_EFFECT emission.  v1 scope (per
                 * V1_BLOCKERS.md #1):  bounded to the teleport + text
                 * effects F0710 already models; other sensor types flow
                 * through as SENSOR_EFFECT_UNSUPPORTED markers and are
                 * emitted as such so downstream probes can observe them
                 * without side effects on world state.
                 *
                 * The emitted payload is:
                 *   payload[0] = SensorEffect.kind   (SENSOR_EFFECT_*)
                 *   payload[1] = SensorEffect.sensorType
                 *   payload[2] = triggerEvent        (SENSOR_EVENT_*)
                 *   payload[3] = textIndex OR destMapIndex
                 *
                 * This is an EMISSION only; world mutation for teleport
                 * effects is deferred to a dedicated pass (the v1 path
                 * already covers tile-type teleporters through F0704).
                 */
                if (world->things) {
                    struct SensorEffectList_Compat walkOff;
                    struct SensorEffectList_Compat walkOn;
                    int s;

                    memset(&walkOff, 0, sizeof(walkOff));
                    memset(&walkOn, 0, sizeof(walkOn));

                    (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
                        world->dungeon, world->things,
                        oldMapIndex, oldMapX, oldMapY,
                        SENSOR_EVENT_WALK_OFF, &walkOff);
                    for (s = 0; s < walkOff.count; ++s) {
                        const struct SensorEffect_Compat* ef = &walkOff.effects[s];
                        int32_t p3 = (ef->kind == SENSOR_EFFECT_TELEPORT)
                            ? ef->destMapIndex
                            : ef->textIndex;
                        emit(result, EMIT_SENSOR_EFFECT,
                             ef->kind, ef->sensorType,
                             SENSOR_EVENT_WALK_OFF, p3);
                    }

                    (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
                        world->dungeon, world->things,
                        world->party.mapIndex,
                        world->party.mapX, world->party.mapY,
                        SENSOR_EVENT_WALK_ON, &walkOn);
                    for (s = 0; s < walkOn.count; ++s) {
                        const struct SensorEffect_Compat* ef = &walkOn.effects[s];
                        int32_t p3 = (ef->kind == SENSOR_EFFECT_TELEPORT)
                            ? ef->destMapIndex
                            : ef->textIndex;
                        emit(result, EMIT_SENSOR_EFFECT,
                             ef->kind, ef->sensorType,
                             SENSOR_EVENT_WALK_ON, p3);
                    }
                }
            }
            return 1;
        }
    }
    case CMD_ATTACK: {
        /* commandArg1 = champion index.  With live target data:
         * commandArg2 = group index or CMD_ATTACK_TARGET_AUTO_GROUP_PC34,
         * reserved = creature slot or CMD_ATTACK_CREATURE_AUTO_PC34.  If the
         * target snapshot is absent, the older marker-only fallback is kept
         * only for explicit legacy-marker callers without live group-table
         * data. */
        DM1_WeaponInfo weaponInfo;
        int weaponClass;
        int hasWeaponInfo = F0888_ORCH_GetChampionActionHandWeaponInfo_Compat(
            world, (int)input->commandArg1, &weaponInfo) > 0;
        int emptyHandLiveAction =
            !hasWeaponInfo &&
            orch_cmd_attack_has_live_action_index_compat(input) &&
            orch_cmd_attack_action_hand_is_empty_compat(
                world, (int)input->commandArg1);
        if (emptyHandLiveAction) {
            /* ReDMCSB MENU.C F0389 lines 717-718 opens action set 2
             * (PUNCH/KICK/WAR CRY) when the action hand is empty.  F0231
             * still uses F0312 champion strength for C01_SLOT_ACTION_HAND,
             * but there is no WEAPON_INFO strength/class addition. */
            orch_cmd_attack_empty_hand_weapon_info_compat(&weaponInfo);
        }
        if (hasWeaponInfo || emptyHandLiveAction) {
            struct CombatantChampionSnapshot_Compat championSnapshot;
            struct CombatantCreatureSnapshot_Compat creatureSnapshot;
            struct WeaponProfile_Compat weaponProfile;
            struct CombatResult_Compat combatResult;
            int groupIndex = -1;
            int creatureIndex = -1;
            int applyOutcome = COMBAT_OUTCOME_INVALID;
            int weaponType = -1;
            int actionIndex = orch_cmd_attack_action_index_compat(input);
            int actionSkillIndex =
                orch_cmd_attack_action_skill_index_compat(actionIndex);
            int killedCell = EXPLOSION_CELL_CENTERED;
            int originalGroupCount = -1;
            int fearTriggered = 0;
            int targetDirection =
                orch_cmd_attack_target_direction_compat(world, input);
            int targetResolved;
            int championSnapshotReady = 0;
            int creatureSnapshotReady = 0;
            if (hasWeaponInfo) {
                int weaponThingIndex = THING_GET_INDEX(
                    world->party.champions[(int)input->commandArg1]
                        .inventory[CHAMPION_SLOT_ACTION_HAND]);
                weaponType = world->things->weapons[weaponThingIndex].type;
            }
            weaponClass = weaponInfo.weaponClass;
            if (orch_cmd_attack_f0407_closed_door_compat(
                    world, input, &weaponInfo, hasWeaponInfo, actionIndex,
                    result)) {
                return 1;
            }
            targetResolved = orch_cmd_attack_resolve_target_compat(
                world, input, &groupIndex, &creatureIndex);
            if (!targetResolved &&
                input->commandArg2 == CMD_ATTACK_TARGET_AUTO_GROUP_PC34) {
                /* ReDMCSB MENU.C F0402 lines 1021-1057 returns false before
                 * F0231 when no melee target creature ordinal exists.  Keep
                 * legacy direct group-index callers on the marker fallback,
                 * but do not synthesize random damage for live auto-target
                 * attacks against an empty front square. */
                return 1;
            }
            if (!targetResolved &&
                orch_cmd_attack_has_live_action_index_compat(input)) {
                /* ReDMCSB MENU.C F0402 lines 1021-1057 only reaches F0231
                 * after a concrete G0517 action-target group and melee
                 * creature ordinal exist.  A direct compatibility call that
                 * already carries an F0407 action index is live melee data,
                 * not the older weapon-class marker path. */
                return 1;
            }
            if (!targetResolved &&
                orch_cmd_attack_has_live_group_table_compat(world)) {
                /* Direct group-index callers against a live THING group table
                 * are runtime melee data too: if the group/creature cannot be
                 * resolved, mirror F0402's early false return instead of
                 * reinterpreting commandArg2 as the legacy weapon-class marker. */
                return 1;
            }
            if (targetResolved &&
                orch_cmd_attack_champion_reach_blocked_f0407_compat(
                    world, (int)input->commandArg1, targetDirection)) {
                emit(result, EMIT_DAMAGE_DEALT,
                     input->commandArg1, groupIndex, 0, COMBAT_OUTCOME_INVALID);
                return 1;
            }
            if (targetResolved &&
                orch_cmd_attack_disrupt_material_blocked_f0407_compat(
                    world, actionIndex, groupIndex)) {
                emit(result, EMIT_DAMAGE_DEALT,
                     input->commandArg1, groupIndex, 0, COMBAT_OUTCOME_INVALID);
                return 1;
            }
            if (targetResolved) {
                creatureSnapshotReady =
                    F0888_ORCH_GetCreatureSnapshot_Compat(
                        world, groupIndex, creatureIndex,
                        orch_cmd_attack_doubled_map_difficulty_compat(world),
                        &creatureSnapshot);
                if (creatureSnapshotReady &&
                    creatureSnapshot.isCandidateInvulnerable) {
                    /* Candidate-panel attacks must remain true NO_ACTION:
                     * reject before F0312 consumes RANDOM(16). */
                    return 1;
                }
                championSnapshotReady =
                    orch_build_cmd_attack_champion_snapshot_compat(
                        world, (int)input->commandArg1, &weaponInfo, weaponType,
                        hasWeaponInfo, actionSkillIndex, &championSnapshot);
            }
            if (targetResolved && (!championSnapshotReady || !creatureSnapshotReady)) {
                /* ReDMCSB MENU.C F0402 enters F0231 only after a concrete
                 * target creature ordinal exists, and PROJEXPL.C F0231
                 * returns before damage when the champion is invalid/dead.
                 * Do not let a resolved live target fall through to the
                 * compatibility marker path when either live snapshot is
                 * unavailable. */
                return 1;
            }
            if (targetResolved && championSnapshotReady && creatureSnapshotReady) {
                orch_build_cmd_attack_weapon_profile_compat(
                    &weaponInfo, weaponType, actionIndex, actionSkillIndex,
                    &weaponProfile);
                if (F0735_COMBAT_ResolveChampionMelee_Compat(
                        &championSnapshot, &weaponProfile, &creatureSnapshot,
                        &world->masterRng, &combatResult)) {
                    if (combatResult.outcome == COMBAT_OUTCOME_NO_ACTION) {
                        /* ReDMCSB CLIKCHAM.C F0368 lines 69-71 keeps the
                         * live candidate champion panel from being redrawn
                         * as a normal champion state.  Firestaff carries
                         * the same "candidate is panel-owned" boundary into
                         * F0735 as NO_ACTION; it must not become a F0231 miss
                         * with stamina, reaction, or damage emissions. */
                        return 1;
                    }
                    orch_writeback_cmd_attack_luck_compat(
                        world, (int)input->commandArg1, &championSnapshot);
                    orch_cmd_attack_apply_f0231_side_effects_compat(
                        world, (int)input->commandArg1, actionSkillIndex,
                        &creatureSnapshot, combatResult.damageApplied);
                    if (combatResult.damageApplied > 0 &&
                        groupIndex >= 0 && groupIndex < world->things->groupCount) {
                        originalGroupCount =
                            (int)world->things->groups[groupIndex].count;
                        killedCell = orch_group_creature_cell_compat(
                            &world->things->groups[groupIndex], creatureIndex);
                        (void)F0738_COMBAT_ApplyDamageToGroup_Compat(
                            &combatResult, &world->things->groups[groupIndex],
                            creatureIndex, &applyOutcome);
                        orch_cmd_attack_apply_f0190_possession_drops_compat(
                            world, &world->things->groups[groupIndex],
                            &creatureSnapshot, killedCell, targetDirection,
                            applyOutcome);
                        orch_cmd_attack_create_f0190_death_smoke_compat(
                            world, &creatureSnapshot, killedCell,
                            targetDirection, applyOutcome);
                        fearTriggered =
                            orch_cmd_attack_apply_f0190_killed_some_state_compat(
                                world, &world->things->groups[groupIndex],
                                &creatureSnapshot, groupIndex, creatureIndex,
                                originalGroupCount, targetDirection,
                                applyOutcome);
                        orch_cmd_attack_apply_group_kill_side_effects_compat(
                            world, groupIndex, targetDirection, applyOutcome);
                        /* ReDMCSB GROUP.C:F0190 lines 892-917 compacts or
                         * unlinks the damaged group after F0231/F0738 melee
                         * damage.  Keep the raw DUNGEON.DAT group record
                         * synchronized with the decoded group state. */
                        orch_write_raw_group_compat(world->things, groupIndex);
                    }
                    if (applyOutcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES ||
                        applyOutcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
                        emit(result, EMIT_KILL_NOTIFY,
                             groupIndex, creatureIndex,
                             applyOutcome, creatureSnapshot.creatureType);
                    }
                    if (!fearTriggered) {
                        orch_cmd_attack_schedule_f0231_reaction_compat(
                            world, groupIndex, &creatureSnapshot,
                            targetDirection,
                            (applyOutcome != COMBAT_OUTCOME_INVALID)
                                ? applyOutcome
                                : combatResult.outcome);
                    }
                    emit(result, EMIT_DAMAGE_DEALT,
                         input->commandArg1, groupIndex,
                         combatResult.damageApplied,
                         (applyOutcome != COMBAT_OUTCOME_INVALID)
                             ? applyOutcome
                             : combatResult.outcome);
                    return 1;
                }
            }
        } else {
            if (input->commandArg2 == CMD_ATTACK_TARGET_AUTO_GROUP_PC34) {
                /* ReDMCSB PROJEXPL.C F0231 lines 1464-1468 rejects invalid
                 * champion ordinals and champions with no current health
                 * before damage/stamina side effects.  Live auto-target
                 * calls must not turn that rejection into legacy marker
                 * damage when no action-hand snapshot can be built. */
                return 1;
            }
            if (orch_cmd_attack_has_live_group_table_compat(world)) {
                /* With live group data present, an unresolved direct target is
                 * a source no-op, not a synthetic marker-damage request. */
                return 1;
            }
            weaponClass = (int)input->commandArg2;
        }
        if (!orch_cmd_attack_has_legacy_marker_compat(input)) {
            /* Marker damage is a synthetic M10 compatibility snapshot, not a
             * ReDMCSB F0402/F0231 live melee path.  Require an explicit marker
             * so unresolved or partially populated runtime calls cannot
             * accidentally manufacture damage. */
            return 1;
        }
        {
            uint32_t r = F0731_COMBAT_RngNextRaw_Compat(&world->masterRng);
            int skillBonus = F0888_ORCH_GetChampionF0312SkillBonus_Compat(
                world, (int)input->commandArg1, weaponClass);
            emit(result, EMIT_DAMAGE_DEALT,
                 input->commandArg1, weaponClass,
                 (int32_t)(r & 0x7FFF), skillBonus);
            return 1;
        }
    }
    case CMD_CAST_SPELL: {
        /* Full spell effect application.
         *   commandArg1 = champion index
         *   commandArg2 = spell table index (0..24)
         *   reserved    = power ordinal (1..6, 0 defaults to 1)
         */
        struct SpellDefinition_Compat spell;
        struct SpellEffect_Compat effect;
        int tableIdx = (int)input->commandArg2;
        int champIdx = (int)input->commandArg1;
        int powerOrd = (int)input->reserved;
        int emptyFlaskSlot = orch_cmd_cast_spell_empty_flask_slot_compat(input);
        int spellExperience = 0;

        (void)F0731_COMBAT_RngNextRaw_Compat(&world->masterRng);
        emit(result, EMIT_SOUND_REQUEST, tableIdx,
             world->party.mapX, world->party.mapY, 0);

        if (powerOrd < 1 || powerOrd > 6) powerOrd = 1;

        if (!F0752b_MAGIC_LookupSpellByTableIndex_Compat(tableIdx, &spell)) {
            return 1; /* unknown spell — sound already emitted */
        }

        memset(&effect, 0, sizeof(effect));

        switch (spell.kind) {
        case C2_SPELL_KIND_PROJECTILE_COMPAT: {
            int skillLvl = F0888_ORCH_GetChampionF0303SkillLevel_Compat(
                world, champIdx, spell.skillIndex);
            F0756_MAGIC_ProduceProjectileEffect_Compat(
                &spell, powerOrd, skillLvl, &world->masterRng, &effect);
            break;
        }
        case C3_SPELL_KIND_OTHER_COMPAT:
            F0757_MAGIC_ProduceOtherEffect_Compat(
                &spell, powerOrd, &world->magic, &effect);
            break;
        case C1_SPELL_KIND_POTION_COMPAT:
            F0758_MAGIC_ProducePotionEffect_Compat(
                &spell, powerOrd, emptyFlaskSlot >= 0, &world->masterRng, &effect);
            break;
        default:
            /* Unknown kind (e.g. magic map) — no effect. */
            return 1;
        }

        if (effect.castResult == SPELL_CAST_SUCCESS) {
            if (effect.spellKind == C1_SPELL_KIND_POTION_COMPAT) {
                (void)orch_cmd_cast_spell_mutate_empty_flask_f0411_compat(
                    world, champIdx, emptyFlaskSlot,
                    effect.spellType, effect.kineticEnergy);
            } else if (effect.spellKind == C3_SPELL_KIND_OTHER_COMPAT &&
                       effect.spellType == C7_SPELL_TYPE_OTHER_ZOKATHRA_COMPAT) {
                (void)orch_cmd_cast_spell_materialize_zokathra_f0412_compat(
                    world, champIdx);
            }

            /* Apply magic state deltas (light, shields, footprints, etc.) */
            F0760_MAGIC_ApplyStateDelta_Compat(&effect, &world->magic);
            spellExperience = orch_cmd_cast_spell_xp_compat(
                input, &spell, effect.powerOrdinal, &world->masterRng);

            /* Schedule follow-up timeline event if applicable */
            if (effect.followupEventKind != TIMELINE_EVENT_INVALID &&
                effect.durationTicks > 0) {
                struct TimelineEvent_Compat tlEv;
                if (F0763_MAGIC_BuildTimelineEvent_Compat(
                        &effect, world->party.mapIndex,
                        world->party.mapX, world->party.mapY,
                        0 /* partyCell */, world->gameTick, &tlEv)) {
                    F0721_TIMELINE_Schedule_Compat(&world->timeline, &tlEv);
                }
            }

            /* Emit spell effect notification:
             *   payload[0] = champIdx
             *   payload[1] = spellKind
             *   payload[2] = spellType
             *   payload[3] = packed powerOrdinal + G0487 SkillIndex + XP */
            emit(result, EMIT_SPELL_EFFECT, champIdx,
                 effect.spellKind, effect.spellType,
                 EMIT_SPELL_EFFECT_PACK_POWER_SKILL_XP(effect.powerOrdinal,
                                                       spell.skillIndex,
                                                       spellExperience));
        }

        return 1;
    }
    case CMD_USE_ITEM:
    case CMD_EAT:
    case CMD_DRINK:
    case CMD_THROW_ITEM: {
        /* Deterministic no-op emission (RNG unchanged). */
        emit(result, EMIT_SOUND_REQUEST, input->command,
             input->commandArg1, input->commandArg2, 0);
        return 1;
    }
    case CMD_REST_TOGGLE:
        world->partyIsResting = world->partyIsResting ? 0 : 1;
        return 1;
    default:
        return 0;
    }
}

int F0887_ORCH_DispatchTimelineEvents_Compat(
    struct GameWorld_Compat* world,
    struct TickResult_Compat* result)
{
    int dispatched = 0;
    struct TimelineEvent_Compat peek, ev;
    if (!world) return 0;

    while (F0722_TIMELINE_Peek_Compat(&world->timeline, &peek) == 1
           && peek.fireAtTick <= world->gameTick)
    {
        if (F0723_TIMELINE_Pop_Compat(&world->timeline, &ev) != 1) break;
        dispatched++;
        switch (ev.kind) {
        case TIMELINE_EVENT_DOOR_ANIMATE: {
            /* Pass 38 — step animating door through states 1..3.
             *
             * Mirror of F0241_TIMELINE_ProcessEvent1_DoorAnimation in
             * TIMELINE.C (step + rattle sound + reschedule).  Hazard
             * branches (champion damage / creature damage on a closing
             * door) are explicitly out of Pass 38 scope.
             *
             * Event aux encoding (see F0713_DOOR_BuildAnimationEvent_Compat):
             *   aux0 = new door state this step will drive to (set by
             *          the dispatcher after stepping; -1 on the very
             *          first step, where the dispatcher reads from the
             *          square).
             *   aux1 = effect (DOOR_EFFECT_SET=opening, DOOR_EFFECT_CLEAR=closing).
             */
            if (world->dungeon) {
                struct DoorAnimationStep_Compat step;
                int effect = ev.aux1;
                memset(&step, 0, sizeof(step));

                /* Pass 418 — source-locked F0241 closing-door hazard gate.
                 * Before the normal CLEAR (+1) step, ReDMCSB checks whether
                 * the party occupies the door square (TIMELINE.C:759-774).
                 * If so, it forces the door back open, applies damage, and
                 * reschedules the same event two ticks after the original
                 * fire time.  We keep damage as deterministic emissions here;
                 * HP mutation remains owned by the champion combat/lifecycle
                 * layers. */
                if (effect == DOOR_EFFECT_CLEAR &&
                    F0712_DOOR_StepAnimation_Compat(
                        world->dungeon, ev.mapIndex, ev.mapX, ev.mapY,
                        DOOR_EFFECT_SET, 0 /* read current only */, &step)) {
                    struct DoorClosingObstruction_Compat obstruction;
                    int groupIndex = -1;
                    int creatureHeight = 0;
                    int hasMaterialCreature = orch_find_material_group_on_square_compat(
                        world->dungeon, world->things, ev.mapIndex, ev.mapX, ev.mapY,
                        &groupIndex, &creatureHeight);
                    int partyOnDoor = (world->party.mapIndex == ev.mapIndex &&
                                       world->party.mapX == ev.mapX &&
                                       world->party.mapY == ev.mapY);
                    if (F0717_DOOR_ResolveClosingObstruction_Compat(
                            step.oldDoorState, step.doorVertical,
                            partyOnDoor, world->party.championCount,
                            hasMaterialCreature,
                            creatureHeight, &obstruction) &&
                        obstruction.kind != DOOR_OBSTRUCTION_NONE) {
                        if (obstruction.kind == DOOR_OBSTRUCTION_PARTY) {
                            int guard = 0;
                            while (guard++ < 5) {
                                struct DoorAnimationStep_Compat openStep;
                                memset(&openStep, 0, sizeof(openStep));
                                if (!F0712_DOOR_StepAnimation_Compat(
                                        world->dungeon, ev.mapIndex, ev.mapX, ev.mapY,
                                        DOOR_EFFECT_SET, 1, &openStep)) break;
                                if (openStep.newDoorState == obstruction.newDoorState ||
                                    openStep.kind == DOOR_ANIM_STEP_REACHED_TARGET) break;
                            }
                            emit(result, EMIT_DAMAGE_DEALT,
                                 obstruction.damageAmount, obstruction.woundMask,
                                 world->party.championCount, ev.mapIndex);
                        } else if (obstruction.kind == DOOR_OBSTRUCTION_CREATURE) {
                            (void)F0712_DOOR_StepAnimation_Compat(
                                world->dungeon, ev.mapIndex, ev.mapX, ev.mapY,
                                DOOR_EFFECT_SET, 1, &step);
                            if (world->things && groupIndex >= 0 &&
                                groupIndex < world->things->groupCount) {
                                (void)orch_damage_group_all_creatures_compat(
                                    &world->things->groups[groupIndex],
                                    obstruction.damageAmount);
                            }
                            emit(result, EMIT_DAMAGE_DEALT,
                                 obstruction.damageAmount, obstruction.woundMask,
                                 groupIndex, ev.mapIndex);
                        }
                        emit(result, EMIT_DOOR_STATE, ev.mapX, ev.mapY,
                             obstruction.newDoorState, ev.mapIndex);
                        emit(result, EMIT_SOUND_REQUEST,
                             obstruction.soundId, ev.mapX, ev.mapY, ev.mapIndex);
                        {
                            struct TimelineEvent_Compat next;
                            memset(&next, 0, sizeof(next));
                            next.kind       = TIMELINE_EVENT_DOOR_ANIMATE;
                            next.fireAtTick = world->gameTick +
                                (uint32_t)obstruction.rescheduleDelayTicks;
                            next.mapIndex   = ev.mapIndex;
                            next.mapX       = ev.mapX;
                            next.mapY       = ev.mapY;
                            next.cell       = ev.cell;
                            next.aux0       = obstruction.newDoorState;
                            next.aux1       = effect;
                            (void)F0721_TIMELINE_Schedule_Compat(
                                &world->timeline, &next);
                        }
                        break;
                    }
                }

                memset(&step, 0, sizeof(step));
                if (F0712_DOOR_StepAnimation_Compat(
                        world->dungeon, ev.mapIndex, ev.mapX, ev.mapY,
                        effect, 1 /* mutateSquare */, &step)) {
                    if (step.kind == DOOR_ANIM_STEP_ADVANCED ||
                        step.kind == DOOR_ANIM_STEP_REACHED_TARGET) {
                        emit(result, EMIT_DOOR_STATE, ev.mapX, ev.mapY,
                             step.newDoorState, ev.mapIndex);
                        /* Rattle sound on every non-final step, mirroring
                         * F0064_SOUND_RequestPlay(C02_SOUND_DOOR_RATTLE)
                         * in F0241. */
                        if (step.kind == DOOR_ANIM_STEP_ADVANCED) {
                            emit(result, EMIT_SOUND_REQUEST,
                                 /* rattle sound id stand-in */ 2,
                                 ev.mapX, ev.mapY, ev.mapIndex);
                            /* Reschedule the same event one tick in the
                             * future, preserving effect.  Mirror of the
                             * F0238_TIMELINE_AddEvent_GetEventIndex_CPSE
                             * tail call in F0241. */
                            {
                                struct TimelineEvent_Compat next;
                                memset(&next, 0, sizeof(next));
                                next.kind       = TIMELINE_EVENT_DOOR_ANIMATE;
                                next.fireAtTick = world->gameTick + 1;
                                next.mapIndex   = ev.mapIndex;
                                next.mapX       = ev.mapX;
                                next.mapY       = ev.mapY;
                                next.cell       = ev.cell;
                                next.aux0       = step.newDoorState;
                                next.aux1       = effect;
                                (void)F0721_TIMELINE_Schedule_Compat(
                                    &world->timeline, &next);
                            }
                        }
                    } else {
                        /* NO_CHANGE — destroyed door or invalid state.
                         * Emit nothing and do not reschedule. */
                    }
                }
            } else {
                /* Defensive fallback when the dungeon pointer is NULL
                 * (headless unit-scope orchestrator): preserve the
                 * legacy marker emission so earlier M10 probes that
                 * manually drive the queue still observe a DOOR_STATE
                 * marker for DOOR_ANIMATE events. */
                emit(result, EMIT_DOOR_STATE, ev.mapX, ev.mapY, ev.aux0, ev.mapIndex);
            }
            break;
        }
        case TIMELINE_EVENT_DOOR_DESTRUCTION:
            /* ReDMCSB PROJEXPL.C F0232 lines 1578-1589 schedules C02
             * when Ticks is non-zero; TIMELINE later applies
             * C5_DOOR_STATE_DESTROYED to the map square. */
            (void)orch_set_door_state_compat(
                world, ev.mapIndex, ev.mapX, ev.mapY, 5);
            emit(result, EMIT_DOOR_STATE, ev.mapX, ev.mapY, 5, ev.mapIndex);
            break;
        case TIMELINE_EVENT_PLAY_SOUND:
            emit(result, EMIT_SOUND_REQUEST, ev.aux0, ev.mapX, ev.mapY, ev.mapIndex);
            break;
        case TIMELINE_EVENT_WATCHDOG:
            /* NOCOPYPROTECTION: no-op. */
            break;
        case TIMELINE_EVENT_HUNGER_THIRST: {
            /* Advance RNG deterministically + schedule next. */
            struct TimelineEvent_Compat next;
            memset(&next, 0, sizeof(next));
            next.kind = TIMELINE_EVENT_HUNGER_THIRST;
            next.fireAtTick = world->gameTick + 64;
            next.mapIndex = world->partyMapIndex;
            F0721_TIMELINE_Schedule_Compat(&world->timeline, &next);
            break;
        }
        case TIMELINE_EVENT_MAGIC_LIGHT_DECAY: {
            struct LightDecayResult_Compat lr;
            memset(&lr, 0, sizeof(lr));
            if (F0864_RUNTIME_HandleLightDecay_Compat(ev.aux0, world->gameTick,
                                                      world->partyMapIndex, &lr)) {
                int newAmt;
                F0867_RUNTIME_ComputeTotalLightAmount_Compat(
                    world->magic.magicalLightAmount, lr.magicalLightAmountDelta, &newAmt);
                world->magic.magicalLightAmount = newAmt;
                if (lr.followupScheduled) {
                    F0721_TIMELINE_Schedule_Compat(&world->timeline, &lr.followupEvent);
                }
            }
            break;
        }
        case TIMELINE_EVENT_STATUS_TIMEOUT: {
            struct TimelineEvent_Compat resched;
            struct TimelineEvent_Compat statusEvent;
            int statusKind = orch_normalize_status_timeout_aux0_pc34_compat(ev.aux0);
            int statusDefense =
                orch_status_timeout_defense_pc34_compat(&ev, statusKind);
            memset(&resched, 0, sizeof(resched));
            statusEvent = ev;
            statusEvent.aux0 = statusKind;
            statusEvent.aux1 = statusDefense;
            if (statusKind == LIFECYCLE_STATUS_POISON) {
                int championIndex = statusEvent.aux4;
                if (championIndex >= 0 && championIndex < CHAMPION_MAX_PARTY) {
                    if (F0835_LIFECYCLE_HandleStatusExpiry_Compat(
                            &world->lifecycle, &statusEvent, championIndex,
                            &resched)) {
                        int damage = resched.aux3;
                        struct ChampionState_Compat* champ =
                            &world->party.champions[championIndex];
                        if (champ->present && champ->hp.current > 0) {
                            if (damage < 0) damage = 0;
                            if (damage > (int)champ->hp.current) {
                                damage = (int)champ->hp.current;
                            }
                            champ->hp.current =
                                (unsigned short)((int)champ->hp.current - damage);
                            if (resched.kind == TIMELINE_EVENT_STATUS_TIMEOUT &&
                                    resched.aux0 == LIFECYCLE_STATUS_POISON &&
                                    resched.aux1 > 0) {
                                champ->poisonDose = (unsigned short)resched.aux1;
                            } else {
                                champ->poisonDose = 0;
                            }
                        }
                        if (resched.kind != TIMELINE_EVENT_INVALID) {
                            (void)F0721_TIMELINE_Schedule_Compat(
                                &world->timeline, &resched);
                        }
                    }
                }
            } else if (statusKind == LIFECYCLE_STATUS_INVISIBILITY) {
                /* ReDMCSB TIMELINE.C C71 lines 1953-1962 decrements
                 * G0407_s_Party.Event71Count_Invisibility. */
                world->magic.event71CountInvisibility--;
                F0835_LIFECYCLE_HandleStatusExpiry_Compat(
                    &world->lifecycle, &statusEvent, statusDefense, &resched);
            } else if (statusKind == LIFECYCLE_STATUS_THIEVES_EYE) {
                /* ReDMCSB TIMELINE.C C73 lines 1973-1974. */
                world->magic.event73CountThievesEye--;
                F0835_LIFECYCLE_HandleStatusExpiry_Compat(
                    &world->lifecycle, &statusEvent, statusDefense, &resched);
            } else if (statusKind == LIFECYCLE_STATUS_PARTY_SHIELD) {
                /* ReDMCSB TIMELINE.C C74 lines 1975-1976 subtracts
                 * the event defense from G0407_s_Party.ShieldDefense. */
                world->magic.partyShieldDefense -= statusDefense;
                F0835_LIFECYCLE_HandleStatusExpiry_Compat(
                    &world->lifecycle, &statusEvent, statusDefense, &resched);
            } else if (statusKind == LIFECYCLE_STATUS_SPELL_SHIELD) {
                /* ReDMCSB TIMELINE.C C77 lines 1985-1986 subtracts the
                 * event defense from G0407_s_Party.SpellShieldDefense. */
                world->magic.spellShieldDefense -= statusDefense;
                F0835_LIFECYCLE_HandleStatusExpiry_Compat(
                    &world->lifecycle, &statusEvent, statusDefense, &resched);
            } else if (statusKind == LIFECYCLE_STATUS_FIRE_SHIELD) {
                /* ReDMCSB TIMELINE.C C78 lines 1988-1989 subtracts the
                 * event defense from G0407_s_Party.FireShieldDefense. */
                world->magic.fireShieldDefense -= statusDefense;
                F0835_LIFECYCLE_HandleStatusExpiry_Compat(
                    &world->lifecycle, &statusEvent, statusDefense, &resched);
            } else if (statusKind == LIFECYCLE_STATUS_FOOTPRINTS) {
                /* ReDMCSB TIMELINE.C C79 lines 1998-1999. */
                world->magic.event79CountFootprints--;
                if (world->magic.event79CountFootprints <= 0) {
                    world->magic.magicFootprintsActive = 0;
                }
                F0835_LIFECYCLE_HandleStatusExpiry_Compat(
                    &world->lifecycle, &statusEvent, statusDefense, &resched);
            } else {
                F0835_LIFECYCLE_HandleStatusExpiry_Compat(
                    &world->lifecycle, &statusEvent, statusDefense, &resched);
            }
            break;
        }
        case TIMELINE_EVENT_REMOVE_FLUXCAGE: {
            struct FluxcageRemoveInput_Compat in;
            struct FluxcageRemoveResult_Compat out;
            memset(&in, 0, sizeof(in));
            memset(&out, 0, sizeof(out));
            in.explosionSlotIndex = ev.aux0;
            in.mapIndex = ev.mapIndex;
            in.mapX = ev.mapX;
            in.mapY = ev.mapY;
            F0868_RUNTIME_HandleRemoveFluxcage_Compat(&in, &world->explosions, &out);
            break;
        }
        case TIMELINE_EVENT_GROUP_GENERATOR:
            if (ev.aux0 == GENERATOR_EVENT_AUX0_REENABLE) {
                (void)orch_reenable_generator_sensor_on_square_compat(
                    world->dungeon, world->things, ev.mapIndex, ev.mapX, ev.mapY);
            } else {
                /* ReDMCSB TIMELINE.C:964-977: audible C006 generator
                 * requests M560_SOUND_BUZZ after F0185 generation. */
                (void)orch_handle_group_generator_trigger_runtime_compat(world, &ev, result);
            }
            break;
        case TIMELINE_EVENT_MOVE_GROUP_SILENT:
        case TIMELINE_EVENT_MOVE_GROUP_AUDIBLE:
            (void)orch_handle_deferred_group_move_event_compat(world, &ev, result);
            break;
        case TIMELINE_EVENT_SQUARE_STATE:
        case TIMELINE_EVENT_MOVE_TIMER:
        case TIMELINE_EVENT_SPELL_TICK:
            break;
        case TIMELINE_EVENT_SENSOR_DELAYED:
            if (ev.aux0 == 10 && ev.aux1 == DOOR_EFFECT_TOGGLE) {
                int resolvedEffect = -1;
                struct TimelineEvent_Compat animEvent;
                /* ReDMCSB PROJEXPL.C:F0217 lines 485-488 schedules
                 * C10_EVENT_DOOR/C02_EFFECT_TOGGLE for Open Door projectile
                 * impacts on button doors; TIMELINE.C:F0241 then resolves
                 * that toggle into the door animation effect. */
                if (F0714_DOOR_ResolveAnimationEffect_Compat(
                        world->dungeon, ev.mapIndex, ev.mapX, ev.mapY,
                        DOOR_EFFECT_TOGGLE, &resolvedEffect, NULL) &&
                    F0713_DOOR_BuildAnimationEvent_Compat(
                        ev.mapIndex, ev.mapX, ev.mapY, resolvedEffect,
                        ev.fireAtTick, &animEvent)) {
                    (void)F0721_TIMELINE_Schedule_Compat(
                        &world->timeline, &animEvent);
                }
            }
            break;
        case TIMELINE_EVENT_CREATURE_TICK:
            (void)orch_handle_creature_tick_group_move_compat(world, &ev, result);
            break;
        case TIMELINE_EVENT_CREATURE_REACTION:
            (void)orch_handle_creature_reaction_event_compat(world, &ev, result);
            break;
        case TIMELINE_EVENT_PROJECTILE_MOVE:
            /* ReDMCSB PROJEXPL.C F0219: C48/C49 events advance one
             * projectile, update its cell/square/energy, then schedule the
             * next move if it still exists.  Full impact side effects remain
             * in the dedicated projectile/M11 paths; this dispatcher closes
             * the old no-op gap for live flight/reschedule. */
            (void)orch_handle_projectile_move_event_compat(world, &ev, result);
            break;
        case TIMELINE_EVENT_EXPLOSION_ADVANCE:
            /* ReDMCSB PROJEXPL.C F0220: event 25 advances or removes one
             * explosion and reschedules persistent clouds/smoke. */
            (void)orch_handle_explosion_advance_event_compat(world, &ev, result);
            break;
        default:
            /* v1 accepts these events as a no-op state-advance: the
             * event was popped (queue shrinks) but we do not yet run
             * the full handler. Determinism is preserved because no
             * RNG/hidden state is touched. */
            break;
        }
        if (dispatched > TIMELINE_QUEUE_CAPACITY * 2) break; /* safety */
    }
    return dispatched;
}

void F0889_ORCH_ApplyPendingDamage_Compat(
    struct GameWorld_Compat* world,
    struct TickResult_Compat* result)
{
    int i, alive = 0;
    if (!world) return;
    (void)result;
    if (world->pendingCombat.damageApplied > 0 && world->party.championCount > 0) {
        /* Apply to active champion (simplified). */
        int idx = world->party.activeChampionIndex;
        if (idx >= 0 && idx < CHAMPION_MAX_PARTY) {
            int killed = 0;
            F0737_COMBAT_ApplyDamageToChampion_Compat(
                &world->pendingCombat, &world->party.champions[idx], &killed);
            if (killed) emit(result, EMIT_CHAMPION_DOWN, idx, 0, 0, 0);
        }
        memset(&world->pendingCombat, 0, sizeof(world->pendingCombat));
    }
    for (i = 0; i < CHAMPION_MAX_PARTY; i++) {
        if (world->party.champions[i].present &&
            world->party.champions[i].hp.current > 0) alive++;
    }
    if (world->party.championCount > 0 && alive == 0) {
        world->partyDead = 1;
    }
}

void F0890_ORCH_ApplyPeriodicEffects_Compat(
    struct GameWorld_Compat* world,
    struct TickResult_Compat* result)
{
    (void)result;
    if (!world) return;
    if (world->disabledMovementTicks > 0) world->disabledMovementTicks--;
    if (world->projectileDisabledMovementTicks > 0) world->projectileDisabledMovementTicks--;
    if (world->freezeLifeTicks > 0) world->freezeLifeTicks--;
    /* ReDMCSB: GAMELOOP.C lines 124-126 increments G0313_ul_GameTime,
     * then calls PANEL.C F0338 lines 434-473 when !(GameTime & 511).
     * F0338 scans party action/ready hands for lit torch weapons and
     * decrements ChargeCount, clearing DoNotDiscard when the torch burns out.
     */
    if (((uint32_t)world->gameTick & ORCH_TORCH_DECAY_TICK_MASK_PC34) == 0u) {
        (void)orch_decrease_torches_light_power_f0338_compat(world);
    }
    world->lifecycle.gameTime = world->gameTick;
}

int F0884_ORCH_AdvanceOneTick_Compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* input,
    struct TickResult_Compat* outResult)
{
    uint32_t i, n;
    int mapTransitions;

    if (!world || !input || !outResult) return ORCH_FAIL;

    memset(outResult, 0, sizeof(*outResult));
    outResult->preTick = world->gameTick;

    /* Step 0: forced RNG advances (for fuzzing) */
    n = input->forcedRngAdvance;
    for (i = 0; i < n; i++) {
        (void)F0731_COMBAT_RngNextRaw_Compat(&world->masterRng);
    }

    /* Step 1: player input */
    if (input->command != CMD_NONE) {
        F0888_ORCH_ApplyPlayerInput_Compat(world, input, outResult);
    }

    /* Step 2/3b: map-transition loop + timeline dispatch */
    mapTransitions = 0;
    do {
        if (world->newPartyMapIndex != -1) {
            world->partyMapIndex = world->newPartyMapIndex;
            world->party.mapIndex = world->newPartyMapIndex;
            world->newPartyMapIndex = -1;
            mapTransitions++;
        }
        F0887_ORCH_DispatchTimelineEvents_Compat(world, outResult);
    } while (world->newPartyMapIndex != -1 &&
             mapTransitions < ORCH_MAX_MAP_TRANSITIONS_PER_TICK);

    /* Step 4: apply pending damage */
    F0889_ORCH_ApplyPendingDamage_Compat(world, outResult);

    /* Step 5: party dead? */
    if (world->partyDead) {
        emit(outResult, EMIT_PARTY_DEAD, 0, 0, 0, 0);
        world->gameTick++;
        outResult->postTick = world->gameTick;
        F0891_ORCH_WorldHash_Compat(world, &outResult->worldHashPost);
        return ORCH_PARTY_DEAD;
    }
    if (world->gameWon) {
        emit(outResult, EMIT_GAME_WON, 0, 0, 0, 0);
        world->gameTick++;
        outResult->postTick = world->gameTick;
        F0891_ORCH_WorldHash_Compat(world, &outResult->worldHashPost);
        return ORCH_GAME_WON;
    }

    /* Step 6: advance game time */
    world->gameTick++;
    outResult->postTick = world->gameTick;

    /* Step 7: periodic effects */
    F0890_ORCH_ApplyPeriodicEffects_Compat(world, outResult);

    /* Step 8: compute world hash */
    F0891_ORCH_WorldHash_Compat(world, &outResult->worldHashPost);

    return ORCH_OK;
}

int F0885_ORCH_RunNTicks_Compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* inputs,
    int tickCount,
    struct TickStreamRecord_Compat* outRecords,
    uint32_t* outFinalHash)
{
    int executed = 0;
    int i;
    struct TickResult_Compat tr;
    if (!world || !inputs || tickCount < 0) return 0;
    for (i = 0; i < tickCount; i++) {
        int rc = F0884_ORCH_AdvanceOneTick_Compat(world, &inputs[i], &tr);
        if (outRecords) {
            memset(&outRecords[i], 0, sizeof(outRecords[i]));
            outRecords[i].input = inputs[i];
            outRecords[i].worldHashPost = tr.worldHashPost;
            outRecords[i].emissionCount =
                (uint16_t)(tr.emissionCount < 0 ? 0 : tr.emissionCount);
        }
        executed++;
        if (rc == ORCH_PARTY_DEAD || rc == ORCH_GAME_WON) break;
        if (rc == ORCH_FAIL) break;
    }
    if (outFinalHash) {
        uint32_t h = 0;
        F0891_ORCH_WorldHash_Compat(world, &h);
        *outFinalHash = h;
    }
    return executed;
}

int F0886_ORCH_RunUntilCondition_Compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* inputs,
    int maxTicks,
    int (*condition)(const struct GameWorld_Compat*),
    struct TickStreamRecord_Compat* outRecords,
    uint32_t* outFinalHash)
{
    int executed = 0;
    int i;
    struct TickResult_Compat tr;
    if (!world || !inputs || maxTicks < 0 || !condition) return 0;
    for (i = 0; i < maxTicks; i++) {
        int rc = F0884_ORCH_AdvanceOneTick_Compat(world, &inputs[i], &tr);
        if (outRecords) {
            memset(&outRecords[i], 0, sizeof(outRecords[i]));
            outRecords[i].input = inputs[i];
            outRecords[i].worldHashPost = tr.worldHashPost;
            outRecords[i].emissionCount =
                (uint16_t)(tr.emissionCount < 0 ? 0 : tr.emissionCount);
        }
        executed++;
        if (rc == ORCH_PARTY_DEAD || rc == ORCH_GAME_WON) break;
        if (rc == ORCH_FAIL) break;
        if (condition(world)) break;
    }
    if (outFinalHash) {
        uint32_t h = 0;
        F0891_ORCH_WorldHash_Compat(world, &h);
        *outFinalHash = h;
    }
    return executed;
}

/* ================================================================
 *  Group E — Headless driver primitives
 * ================================================================ */

int F0894_DRIVER_LoadTickStream_Compat(
    const char* path,
    struct TickInput_Compat** outInputs,
    int* outCount)
{
    FILE* f;
    char line[256];
    int cap = 64, cnt = 0;
    struct TickInput_Compat* arr;

    if (!path || !outInputs || !outCount) return 0;
    f = fopen(path, "r");
    if (!f) return 0;
    arr = (struct TickInput_Compat*)calloc((size_t)cap, sizeof(*arr));
    if (!arr) { fclose(f); return 0; }

    while (fgets(line, sizeof line, f)) {
        unsigned tick, cmd, a1, a2;
        /* Skip comments / blanks */
        char* s = line;
        while (*s == ' ' || *s == '\t') s++;
        if (*s == '#' || *s == '\n' || *s == '\0') continue;
        if (sscanf(s, "%x %x %x %x", &tick, &cmd, &a1, &a2) != 4) continue;
        if (cnt == cap) {
            struct TickInput_Compat* na;
            cap *= 2;
            na = (struct TickInput_Compat*)realloc(arr, (size_t)cap * sizeof(*arr));
            if (!na) { free(arr); fclose(f); return 0; }
            arr = na;
        }
        memset(&arr[cnt], 0, sizeof(arr[cnt]));
        arr[cnt].tick = tick;
        arr[cnt].command = (uint8_t)(cmd & 0xFF);
        arr[cnt].commandArg1 = (uint8_t)(a1 & 0xFF);
        arr[cnt].commandArg2 = (uint8_t)(a2 & 0xFF);
        cnt++;
    }
    fclose(f);
    *outInputs = arr;
    *outCount = cnt;
    return 1;
}

int F0895_DRIVER_RunStream_Compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* inputs,
    int inputCount,
    struct TickStreamRecord_Compat* outRecords,
    uint32_t* outFinalHash)
{
    return F0885_ORCH_RunNTicks_Compat(world, inputs, inputCount, outRecords, outFinalHash);
}

void F0896_DRIVER_WriteSummary_Compat(
    const struct GameWorld_Compat* world,
    uint32_t finalHash,
    int ticksRun,
    FILE* outFile)
{
    int alive = 0;
    int i;
    if (!world || !outFile) return;
    for (i = 0; i < CHAMPION_MAX_PARTY; i++) {
        if (world->party.champions[i].present &&
            world->party.champions[i].hp.current > 0) alive++;
    }
    fprintf(outFile, "ReDMCSB headless summary\n");
    fprintf(outFile, "ticks_run: %d\n", ticksRun);
    fprintf(outFile, "final_hash: 0x%08X\n", finalHash);
    fprintf(outFile, "game_tick: %u\n", (unsigned)world->gameTick);
    fprintf(outFile, "champions_alive: %d\n", alive);
    fprintf(outFile, "party_map_index: %d\n", (int)world->partyMapIndex);
    fprintf(outFile, "party_x: %d\n", world->party.mapX);
    fprintf(outFile, "party_y: %d\n", world->party.mapY);
    fprintf(outFile, "party_dir: %d\n", world->party.direction);
    fprintf(outFile, "party_dead: %d\n", (int)world->partyDead);
    fprintf(outFile, "game_won: %d\n", (int)world->gameWon);
    fprintf(outFile, "rng_seed: 0x%08X\n", (unsigned)world->masterRng.seed);
    fprintf(outFile, "dungeon_fingerprint: 0x%08X\n", (unsigned)world->dungeonFingerprint);
}
