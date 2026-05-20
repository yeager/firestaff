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
#include "dm1_v1_sound_pc34_compat.h"        /* DM1_SND_BUZZ for C006 generator audio */

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
    if ((int)sz != party_size()) return 0;
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

static int orch_materialize_generated_group_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    const struct GeneratorResult_Compat* generator,
    int* outGroupIndex)
{
    struct DungeonGroup_Compat* resized;
    struct DungeonGroup_Compat* group;
    int sftIndex;
    int groupIndex;
    int i;

    if (outGroupIndex) *outGroupIndex = -1;
    if (!world || !ev || !generator || !world->dungeon || !world->things) return 0;
    if (!generator->spawned) return 0;

    /* ReDMCSB MOVESENS.C:830-844 defers group insertion if the
     * destination already holds the party or another group.  Event60/61
     * follow-up is not represented in Phase 20 yet, so this narrow slice
     * only materializes the successful empty-square path. */
    if (orch_square_has_group_or_party_compat(world, ev->mapIndex, ev->mapX, ev->mapY)) {
        return 0;
    }

    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, ev->mapIndex, ev->mapX, ev->mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;
    if (world->things->groupCount < 0 || world->things->groupCount >= 1024) return 0;

    groupIndex = world->things->groupCount;
    resized = (struct DungeonGroup_Compat*)realloc(
        world->things->groups,
        (size_t)(groupIndex + 1) * sizeof(*world->things->groups));
    if (!resized) return 0;
    world->things->groups = resized;
    group = &world->things->groups[groupIndex];
    memset(group, 0, sizeof(*group));

    group->next = world->things->squareFirstThings[sftIndex];
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

    world->things->groupCount = groupIndex + 1;
    world->things->thingCounts[THING_TYPE_GROUP] = world->things->groupCount;
    world->things->squareFirstThings[sftIndex] =
        orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex);

    (void)orch_add_generated_group_active_state_compat(
        world, groupIndex, group, ev->mapIndex, ev->mapX, ev->mapY);
    orch_schedule_generated_group_wandering_event_compat(
        world, groupIndex, group, ev->mapIndex, ev->mapX, ev->mapY);
    if (outGroupIndex) *outGroupIndex = groupIndex;
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

    if (orch_materialize_generated_group_compat(world, ev, &generator, 0)) {
        /* ReDMCSB GROUP.C:543-547/F0185: successful placement requests
         * M560_SOUND_BUZZ independently of the sensor's Audible flag. */
        emit(result, EMIT_SOUND_REQUEST, DM1_SND_BUZZ,
             ev->mapX, ev->mapY, ev->mapIndex);
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
        /* Minimal v1: emit EMIT_DAMAGE_DEALT marker + advance RNG once so
         * the tick is not a no-op. Fontanel's full combat pipeline is
         * reachable via F0735 given proper combatant snapshots — wired in
         * Phase 20 v2 when the party builds snapshots from live state. */
        uint32_t r = F0731_COMBAT_RngNextRaw_Compat(&world->masterRng);
        emit(result, EMIT_DAMAGE_DEALT,
             input->commandArg1, input->commandArg2, (int32_t)(r & 0x7FFF), 0);
        return 1;
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
            int skillLvl = 0;
            if (champIdx >= 0 && champIdx < CHAMPION_MAX_PARTY &&
                world->party.champions[champIdx].present &&
                spell.skillIndex >= 0 && spell.skillIndex < CHAMPION_SKILL_COUNT) {
                skillLvl = world->party.champions[champIdx].skillLevels[spell.skillIndex];
            }
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
                &spell, powerOrd, 0 /* hasEmptyFlaskInHand: UI checked */, &world->masterRng, &effect);
            break;
        default:
            /* Unknown kind (e.g. magic map) — no effect. */
            return 1;
        }

        if (effect.castResult == SPELL_CAST_SUCCESS) {
            /* Apply magic state deltas (light, shields, footprints, etc.) */
            F0760_MAGIC_ApplyStateDelta_Compat(&effect, &world->magic);

            /* Schedule follow-up timeline event if applicable */
            if (effect.followupEventKind != TIMELINE_EVENT_INVALID &&
                effect.durationTicks > 0) {
                struct TimelineEvent_Compat tlEv;
                if (F0763_MAGIC_BuildTimelineEvent_Compat(
                        &effect, world->partyMapIndex,
                        world->party.mapX, world->party.mapY,
                        0 /* partyCell */, world->gameTick, &tlEv)) {
                    F0721_TIMELINE_Schedule_Compat(&world->timeline, &tlEv);
                }
            }

            /* Emit spell effect notification:
             *   payload[0] = champIdx
             *   payload[1] = spellKind
             *   payload[2] = spellType
             *   payload[3] = powerOrdinal */
            emit(result, EMIT_SPELL_EFFECT, champIdx,
                 effect.spellKind, effect.spellType, effect.powerOrdinal);
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
            emit(result, EMIT_DOOR_STATE, ev.mapX, ev.mapY, ev.aux0, ev.mapIndex);
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
            memset(&resched, 0, sizeof(resched));
            F0835_LIFECYCLE_HandleStatusExpiry_Compat(
                &world->lifecycle, &ev, ev.aux1, &resched);
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
        case TIMELINE_EVENT_SQUARE_STATE:
        case TIMELINE_EVENT_SENSOR_DELAYED:
        case TIMELINE_EVENT_MOVE_TIMER:
        case TIMELINE_EVENT_SPELL_TICK:
        case TIMELINE_EVENT_CREATURE_TICK:
        case TIMELINE_EVENT_PROJECTILE_MOVE:
        case TIMELINE_EVENT_EXPLOSION_ADVANCE:
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
    /* Torch decay every 512 ticks: stub — real torch decrement lives in
     * Phase 18's inventory ticker. */
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
