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
#include "dm1_v1_c15_layout_pc34_compat.h"
#include "dm1_v1_c14_layout_pc34_compat.h"
#include "dm1_v1_projectile_impact_count_pc34_compat.h"
#include "dm1_v1_champion_needs_pc34_compat.h"
#include "dm1_v1_torch_drain_f0338_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_group_active_state_pc34_compat.h"
#include "dm1_v1_group_los_direction_admission_pc34_compat.h"
#include "dm1_v1_melee_target_admission_pc34_compat.h"
#include "dm1_v1_combat_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_f0259_quiver_refill_pc34_compat.h"
#include "dm1_v1_resurrection_pc34_compat.h"
#include "dm1_v1_f0249_timeline_relocation_pc34_compat.h"
#include "dm1_v1_melee_action_f0402_pc34_compat.h"
#include "dm1_v1_movement_pc34_compat.h"
#include "dm1_v1_movement_timing_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "dm1_v1_spell_casting_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "dm1_v1_teleporter_pit_pc34_compat.h"
#include "dm1_v1_throw_shoot_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_f0249_timeline_relocation_pc34_compat.h"
#include "dm1_v1_resurrection_pc34_compat.h"
#include "firestaff/dm1/v1/G0492_pc34_compat.h"
#include "firestaff/dm1/v1/G0493_pc34_compat.h"
#include "dm1_v1_sound_pc34_compat.h"        /* DM1_SND_BUZZ for C006 generator audio */
#include "memory_mov05_f0284_cell_rotation_pc34_compat.h"

enum {
    ORCH_CREATURE_BLACK_FLAME_PC34 = 11,
    ORCH_SOUND_WOODEN_THUD_PC34 = 4,
    /* ReDMCSB DATA.C G0060 I34E C04_SOUND_WOODEN_THUD priority. */
    ORCH_SOUND_WOODEN_THUD_PRIORITY_PC34 = 70,
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
static int orch_cmd_attack_find_door_on_square_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    int* outDoorIndex);
static int orch_square_first_thing_list_index_compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY);
static int orch_set_next_thing_compat(
    struct DungeonThings_Compat* things,
    unsigned short thing,
    unsigned short nextThing);
static int orch_cmd_attack_find_door_on_square_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    int* outDoorIndex);
static int orch_f0249_move_non_group_square_things_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY);
static int orch_dispatch_wall_event_f0248_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result);
static int orch_dispatch_corridor_event_f0245_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result);
static int orch_f0248_target_square_type_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY);
static int orch_handle_group_generator_trigger_runtime_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result);
static int orch_find_material_group_on_square_compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    int* outGroupIndex,
    int* outCreatureHeight);
static int orch_schedule_projectile_move_f0219_compat(
    struct GameWorld_Compat* world,
    int projectileIndex,
    const struct TimelineEvent_Compat* event);
static int orch_projectile_move_schedule_admissible_f0219_compat(
    const struct GameWorld_Compat* world,
    int projectileIndex,
    const struct TimelineEvent_Compat* event);
static int orch_cmd_attack_map_difficulty_compat(
    const struct GameWorld_Compat* world);

int DM1_V1_F0330_ScheduleEnableChampionActionPc34Compat(
    struct GameWorld_Compat* world,
    int championIndex,
    int ticks);

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
#define SEC_TAG_CHAMPION_COMBAT   0x2000001Bu

#define ORCH_SCALARS_PAYLOAD_SIZE 52  /* 13 × int32: gameTick, partyDead,
                                         gameWon, partyMapIndex,
                                         newPartyMapIndex, masterRng.seed,
                                         partyIsResting, freezeLifeTicks,
                                         disabledMovementTicks,
                                         projectileDisabledMovementTicks,
                                         lastProjectileDisabledMovementDirection,
                                         creatureAICount, pending target receipt */

#define ORCH_PENDING_DAMAGE_RECEIPT_VALID_PC34 0x100
#define ORCH_PENDING_DAMAGE_RECEIPT_CHAMPION_MASK_PC34 0x003
#define ORCH_PENDING_DAMAGE_RECEIPT_CELL_SHIFT_PC34 2
#define ORCH_PENDING_DAMAGE_RECEIPT_CELL_MASK_PC34 0x00C

static int orch_make_pending_damage_receipt_compat(int championIndex,
                                                    int targetCell)
{
    return ORCH_PENDING_DAMAGE_RECEIPT_VALID_PC34 |
           (championIndex & ORCH_PENDING_DAMAGE_RECEIPT_CHAMPION_MASK_PC34) |
           ((targetCell & 3) << ORCH_PENDING_DAMAGE_RECEIPT_CELL_SHIFT_PC34);
}

static int orch_read_pending_damage_receipt_compat(int receipt,
                                                    int* outChampionIndex,
                                                    int* outTargetCell)
{
    if (!(receipt & ORCH_PENDING_DAMAGE_RECEIPT_VALID_PC34)) return 0;
    if (outChampionIndex) {
        *outChampionIndex = receipt & ORCH_PENDING_DAMAGE_RECEIPT_CHAMPION_MASK_PC34;
    }
    if (outTargetCell) {
        *outTargetCell =
            (receipt & ORCH_PENDING_DAMAGE_RECEIPT_CELL_MASK_PC34) >>
            ORCH_PENDING_DAMAGE_RECEIPT_CELL_SHIFT_PC34;
    }
    return 1;
}

static void orch_stage_champion_combat_compat(
    struct GameWorld_Compat* world,
    int championIndex,
    int targetCell,
    const struct CombatResult_Compat* combat)
{
    struct CombatResult_Compat* pending;
    if (!world || !combat || championIndex < 0 ||
        championIndex >= CHAMPION_MAX_PARTY || combat->damageApplied <= 0) {
        return;
    }

    /* ReDMCSB CHAMPION.C F0321:1909 adds the resolved attack to
     * G0409_ai_ChampionPendingDamage[champion], while F0321:1901 ORs the
     * selected wound into G0410.  F0320 later consumes the staged totals. */
    pending = &world->pendingChampionCombat[championIndex];
    pending->damageApplied += combat->damageApplied;
    pending->woundMaskAdded |= combat->woundMaskAdded;
    pending->poisonAttackPending += combat->poisonAttackPending;
    pending->outcome = combat->outcome;
    world->pendingChampionCombatTargetReceipt[championIndex] =
        orch_make_pending_damage_receipt_compat(championIndex, targetCell);
}

/* F0230 calls F0304 for Parry before its damage roll.  Keep that award on
 * the live C38/C39 owner rather than letting a presentation/action helper
 * manufacture combat XP later.  The receipt is deliberately short-lived:
 * its raw C04 bytes, event coordinates/time and pre-hit champion state must
 * still agree at consumption, otherwise no XP is admitted. */
struct OrchF0230ParryReceipt_Compat {
    uint32_t sourceIdentity;
    int championIndex;
    int creatureIndex;
    int experience;
    int championHealth;
    unsigned short championWounds;
    int valid;
};

static uint32_t orch_f0230_parry_mix_byte_compat(uint32_t value,
                                                  unsigned char byte)
{
    return (value ^ (uint32_t)byte) * 16777619u;
}

static uint32_t orch_f0230_parry_source_identity_compat(
    const struct TimelineEvent_Compat* ev,
    const unsigned char* rawGroup,
    const struct ChampionState_Compat* champion,
    int championIndex,
    int creatureIndex,
    const struct CombatResult_Compat* combat)
{
    uint32_t hash = 2166136261u;
    int i;
    const int values[] = {
        ev ? ev->kind : -1, ev ? ev->mapIndex : -1, ev ? ev->mapX : -1,
        ev ? ev->mapY : -1, ev ? ev->aux0 : -1, ev ? ev->aux1 : -1,
        ev ? ev->aux2 : -1, ev ? ev->aux3 : -1, ev ? ev->aux4 : -1,
        ev ? (int)ev->fireAtTick : -1, championIndex, creatureIndex,
        champion ? champion->present : 0,
        champion ? champion->hp.current : -1,
        champion ? champion->hp.maximum : -1,
        champion ? champion->stamina.current : -1,
        champion ? champion->wounds : -1,
        combat ? combat->outcome : -1,
        combat ? combat->damageApplied : -1,
        combat ? combat->woundMaskAdded : -1,
        combat ? combat->poisonAttackPending : -1,
        combat ? combat->rngCallCount : -1
    };

    for (i = 0; i < 16; ++i) {
        hash = orch_f0230_parry_mix_byte_compat(hash,
                                                 rawGroup ? rawGroup[i] : 0);
    }
    for (i = 0; i < (int)(sizeof(values) / sizeof(values[0])); ++i) {
        unsigned int value = (unsigned int)values[i];
        hash = orch_f0230_parry_mix_byte_compat(hash, (unsigned char)value);
        hash = orch_f0230_parry_mix_byte_compat(hash, (unsigned char)(value >> 8));
        hash = orch_f0230_parry_mix_byte_compat(hash, (unsigned char)(value >> 16));
        hash = orch_f0230_parry_mix_byte_compat(hash, (unsigned char)(value >> 24));
    }
    return hash;
}

int F0890a_ORCH_ConsumeF0230F0304Parry_Compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    const struct DungeonGroup_Compat* group,
    const struct CombatantCreatureSnapshot_Compat* attacker,
    int championIndex,
    int creatureIndex,
    const struct CombatResult_Compat* combat,
    struct TickResult_Compat* result)
{
    struct OrchF0230ParryReceipt_Compat receipt;
    const struct ChampionState_Compat* champion;
    const unsigned char* rawGroup;
    int experience;
    int baseSkillIndex;

    if (!world || !ev || !group || !attacker || !combat || !world->things ||
        championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY ||
        creatureIndex < 0 || creatureIndex >= 4 ||
        ev->kind != TIMELINE_EVENT_CREATURE_REACTION ||
        (ev->aux2 != DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
         ev->aux2 != DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_1) ||
        ev->aux0 < 0 || ev->aux0 >= world->things->groupCount ||
        ev->aux1 != group->creatureType ||
        !world->things->rawThingData[THING_TYPE_GROUP] ||
        ev->aux0 >= world->things->thingCounts[THING_TYPE_GROUP]) {
        return 0;
    }

    champion = &world->party.champions[championIndex];
    rawGroup = world->things->rawThingData[THING_TYPE_GROUP] + ev->aux0 * 16;
    if (!champion->present || champion->hp.current <= 0 ||
        rawGroup[4] != group->creatureType) {
        return 0;
    }

    /* CREATURE_INFO.Properties bits 11:8 are M058_EXPERIENCE. */
    experience = (int)(((unsigned int)attacker->properties >> 8) & 0x0fu);
    if (experience <= 0) return 1; /* Source F0304 receives a zero no-op. */

    memset(&receipt, 0, sizeof(receipt));
    receipt.championIndex = championIndex;
    receipt.creatureIndex = creatureIndex;
    receipt.experience = experience;
    receipt.championHealth = champion->hp.current;
    receipt.championWounds = champion->wounds;
    receipt.sourceIdentity = orch_f0230_parry_source_identity_compat(
        ev, rawGroup, champion, championIndex, creatureIndex, combat);
    receipt.valid = receipt.sourceIdentity != 0u;

    /* Revalidate immediately at the F0230/F0304 boundary.  This is
     * intentionally fail-closed: no action/menu fallback can award XP when
     * C04 or the selected champion changed under a stale C38/C39 event. */
    if (!receipt.valid || champion->hp.current != receipt.championHealth ||
        champion->wounds != receipt.championWounds ||
        receipt.sourceIdentity != orch_f0230_parry_source_identity_compat(
            ev, rawGroup, champion, championIndex, creatureIndex, combat)) {
        return 0;
    }

    baseSkillIndex = dm1_skill_get_base_index(DM1_SKILL_IDX_PARRY);
    if (baseSkillIndex < 0 || baseSkillIndex >= CHAMPION_SKILL_COUNT) return 0;
    (void)F0849_LIFECYCLE_AddSkillExperience_Compat(
        &world->lifecycle.champions[championIndex], DM1_SKILL_IDX_PARRY,
        receipt.experience, orch_cmd_attack_map_difficulty_compat(world),
        world->gameTick, world->lifecycle.lastCreatureAttackTime, NULL, NULL);
    world->party.champions[championIndex].skillExperience[baseSkillIndex] =
        (unsigned long)world->lifecycle.champions[championIndex]
            .skills20[baseSkillIndex].experience;
    emit(result, EMIT_XP_AWARD, championIndex, DM1_SKILL_IDX_PARRY,
         receipt.experience, (int)(receipt.sourceIdentity & 0x7fffffffu));
    return 1;
}

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
static int champion_combat_size(void) {
    return CHAMPION_MAX_PARTY * (combat_res_size() + 4);
}

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
    total += 8 + champion_combat_size();
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
    w_i32(scalars + 48, world->pendingCombatTargetReceipt);

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
    w_u32(outBuf + off, SEC_TAG_CHAMPION_COMBAT); off += 4;
    w_u32(outBuf + off, (uint32_t)champion_combat_size()); off += 4;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        if (!F0742_COMBAT_ResultSerialize_Compat(
                &world->pendingChampionCombat[i], outBuf + off,
                combat_res_size())) return 0;
        off += combat_res_size();
        w_i32(outBuf + off, world->pendingChampionCombatTargetReceipt[i]);
        off += 4;
    }

    /* 7. Magic */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_MAGIC,
                       magic_size(), ad_magic, &world->magic)) return 0;

    /* 8. Dungeon mutations */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_DUNGEON_MUTATIONS,
                       mutations_size(), ad_mutations, &world->dungeonMutations)) return 0;

    /* 9. Creature AI list */
    w_u32(outBuf + off, SEC_TAG_CREATURE_AI); off += 4;
    w_u32(outBuf + off, (uint32_t)aiPayloadSize); off += 4;
    w_i32(outBuf + off, ai_count); off += 4;
    for (i = 0; i < ai_count; i++) {
        if (off + ai_size() > outBufSize) return 0;
        if (!F0805_CREATURE_AIStateSerialize_Compat(&world->creatureAI[i],
                                                    outBuf + off, ai_size())) return 0;
        off += ai_size();
    }

    /* 10. Projectile list */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_PROJECTILES,
                       proj_list_size(), ad_proj_list, &world->projectiles)) return 0;

    /* 11. Explosion list */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_EXPLOSIONS,
                       expl_list_size(), ad_expl_list, &world->explosions)) return 0;

    /* 12. Lifecycle */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_LIFECYCLE,
                       lifecycle_size(), ad_lifecycle, &world->lifecycle)) return 0;

    /* 13. Pending sensor effects */
    if (!write_section(outBuf, outBufSize, &off, SEC_TAG_SENSOR_PENDING,
                       sensor_size(), ad_sensor, &world->pendingSensorEffects)) return 0;

    /* 14. Save header — raw write of the 64-byte SaveGameHeader_Compat */
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
    world->pendingCombatTargetReceipt = r_i32(buf + off + 48);
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

    /* 6. F0321 per-champion pending damage/wound staging. */
    if (!read_section_hdr(buf, bufSize, &off, SEC_TAG_CHAMPION_COMBAT, &sz)) return 0;
    if ((int)sz != champion_combat_size()) return 0;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        if (!F0743_COMBAT_ResultDeserialize_Compat(
                &world->pendingChampionCombat[i], buf + off,
                combat_res_size())) return 0;
        off += combat_res_size();
        world->pendingChampionCombatTargetReceipt[i] = r_i32(buf + off);
        off += 4;
    }

    /* 7. Magic */
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
    world->pendingCombatTargetReceipt = 0;

    F0720_TIMELINE_Init_Compat(&world->timeline, 0);
    F0730_COMBAT_RngInit_Compat(&world->masterRng, seed ? seed : 1u);
    memset(&world->party, 0, sizeof(world->party));
    for (i = 0; i < CHAMPION_MAX_PARTY; i++)
        F0600_CHAMPION_InitEmpty_Compat(&world->party.champions[i]);
    world->party.activeChampionIndex = -1;

    memset(&world->pendingCombat, 0, sizeof(world->pendingCombat));
    memset(world->pendingChampionCombat, 0, sizeof(world->pendingChampionCombat));
    memset(world->pendingChampionCombatTargetReceipt, 0,
           sizeof(world->pendingChampionCombatTargetReceipt));
    memset(&world->magic, 0, sizeof(world->magic));
    memset(&world->dungeonMutations, 0, sizeof(world->dungeonMutations));
    memset(&world->pendingSensorEffects, 0, sizeof(world->pendingSensorEffects));
    memset(&world->projectiles, 0, sizeof(world->projectiles));
    memset(&world->explosions, 0, sizeof(world->explosions));
    if (!F0196_DM1_GROUP_InitializeActiveGroups_Compat(world)) return 0;
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
    (void)F0284_CHAMPION_SetPartyDirection_Compat(&outWorld->party, direction);
    outWorld->partyMapIndex = 0;

    /* ReDMCSB STARTUP2.C reaches NEWMAP.C F0003 after the initial party
     * location is decoded.  F0003 installs the current map and then invokes
     * GROUP.C F0195, so the loaded C04 chains own initial active state and
     * C37 scheduling rather than an empty host-only AI table. */
    if (F0195_DM1_GROUP_AddAllActiveGroups_Compat(outWorld) < 0) goto fail;

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

/* ReDMCSB DUNGEON.C G0243_as_Graphic559_CreatureInfo (PC 3.4 rows).
 * The Phase-16 profile owns combat values; F0209 additionally needs these
 * packed range/animation words when it schedules C29-C41 events. */
static const unsigned short s_dm1_i34_creature_ranges[27] = {
    0x1153, 0x3132, 0x1376, 0x320A, 0x1554, 0x1232, 0x1111,
    0x1463, 0x1423, 0x1023, 0x1224, 0x1312, 0x1013, 0x1343,
    0x4335, 0x1AA1, 0x1343, 0x1432, 0x1005, 0x3258, 0x1381,
    0x1592, 0x4344, 0x6369, 0x3645, 0x6369, 0x6369
};

static const unsigned short s_dm1_i34_creature_animation_ticks[27] = {
    0x0254, 0x0384, 0x0222, 0x0113, 0x0143, 0x0265, 0x02F2,
    0x01F4, 0x0116, 0x04F3, 0x0483, 0x0114, 0x0132, 0x0112,
    0x0664, 0x0253, 0x0332, 0x0112, 0x0143, 0x0117, 0x0345,
    0x0224, 0x0124, 0x0564, 0x0445, 0x0564, 0x0564
};

static int orch_get_dm1_creature_info_pc34_compat(
    int creatureType,
    struct DM1CreatureInfo_Compat* out)
{
    const struct CreatureBehaviorProfile_Compat* profile;

    if (!out || creatureType < 0 || creatureType >= CREATURE_TYPE_COUNT) return 0;
    profile = CREATURE_GetProfile_Compat(creatureType);
    if (!profile) return 0;
    memset(out, 0, sizeof(*out));
    out->attributes = s_dm1_i34_creature_attributes[creatureType];
    out->movementTicks = profile->movementTicks;
    out->attackTicks = profile->attackTicks;
    out->attack = profile->baseAttack;
    out->poisonAttack = profile->poisonAttack;
    out->dexterity = profile->dexterity;
    out->ranges = s_dm1_i34_creature_ranges[creatureType];
    out->properties = profile->properties;
    out->animationTicks = s_dm1_i34_creature_animation_ticks[creatureType];
    out->woundProbabilities = profile->woundProbabilities;
    out->attackType = profile->attackType;
    return 1;
}

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
        THING_GET_TYPE(thing) != THING_TYPE_WEAPON) return -1;
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
    unsigned short handThings[DM1_V1_F0338_HAND_COUNT_PC34];
    DM1_V1_TorchDrainReceiptF0338Pc34 receipt;
    if (!world || !world->things) return 0;
    for (championIndex = 0;
         championIndex < DM1_V1_F0338_HAND_COUNT_PC34;
         ++championIndex) handThings[championIndex] = THING_NONE;
    for (championIndex = 0;
         championIndex < world->party.championCount &&
             championIndex < CHAMPION_MAX_PARTY;
         championIndex++) {
        struct ChampionState_Compat* champion =
            &world->party.champions[championIndex];
        handThings[championIndex * 2] =
            champion->inventory[CHAMPION_SLOT_ACTION_HAND];
        handThings[championIndex * 2 + 1] =
            champion->inventory[CHAMPION_SLOT_HAND_LEFT];
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_torch_drain_f0338_pc34(
            world->things, handThings, world->party.championCount, &receipt) ||
        !receipt.valid) return 0;
    return receipt.changedCount;
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

static int orch_cmd_cast_spell_has_magic_map_compat(
    const struct TickInput_Compat* input)
{
    return input &&
        (input->reserved2 & CMD_CAST_SPELL_RESERVED2_HAS_MAGIC_MAP) != 0u;
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

static int orch_cmd_cast_spell_build_dm1_stats_f0412_compat(
    struct GameWorld_Compat* world,
    int championIndex,
    DM1_ChampionSpellStats* outStats)
{
    const struct ChampionState_Compat* champion;
    int i;
    if (!world || !outStats) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champion = &world->party.champions[championIndex];
    if (!champion->present) return 0;

    memset(outStats, 0, sizeof(*outStats));
    outStats->currentMana = (int16_t)champion->mana.current;
    outStats->maximumMana = (int16_t)champion->mana.maximum;
    outStats->currentHealth = (int16_t)champion->hp.current;
    outStats->wisdom = (uint8_t)(champion->attributes[CHAMPION_ATTR_WISDOM] > 255
                                     ? 255
                                     : champion->attributes[CHAMPION_ATTR_WISDOM]);
    for (i = 0; i < 20; ++i) {
        int level = F0888_ORCH_GetChampionF0303SkillLevel_Compat(
            world, championIndex, i);
        if (level < 0) level = 0;
        if (level > 255) level = 255;
        outStats->skillLevels[i] = (uint8_t)level;
    }
    return 1;
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

static int orch_spell_tick_has_typed_status_receipt_pc34_compat(
    const struct TimelineEvent_Compat* ev)
{
    if (!ev) return 0;

    /* ReDMCSB TIMELINE.C F0261:1953-1999 owns C71/C73/C74/C77/C78/C79
     * directly.  The richer F0412/F0763 tags are Firestaff's typed
     * compatibility receipts for exactly those source events.  Do not let
     * arbitrary legacy C05 payloads become status effects. */
    switch (ev->aux0) {
    case TIMELINE_AUX_THIEVES_EYE:
    case TIMELINE_AUX_INVISIBILITY:
    case TIMELINE_AUX_PARTY_SHIELD:
    case TIMELINE_AUX_FIRESHIELD:
    case TIMELINE_AUX_FOOTPRINTS:
    case TIMELINE_AUX_SPELL_SHIELD:
        return 1;
    case DM1_EVENT_INVISIBILITY:
    case DM1_EVENT_THIEVES_EYE:
    case DM1_EVENT_FOOTPRINTS:
        return ev->aux2 == ev->aux0 && ev->aux1 == 0 && ev->aux4 == 0;
    case DM1_EVENT_PARTY_SHIELD:
    case DM1_EVENT_SPELLSHIELD:
    case DM1_EVENT_FIRESHIELD:
        return ev->aux2 == ev->aux0 && ev->aux1 > 0 && ev->aux4 == 0;
    default:
        return 0;
    }
}

static int orch_status_timeout_defense_pc34_compat(
    const struct TimelineEvent_Compat* ev,
    int normalizedStatus)
{
    if (!ev) return 0;
    if (normalizedStatus == LIFECYCLE_STATUS_SPELL_SHIELD &&
        ev->aux0 == TIMELINE_AUX_SPELL_SHIELD) {
        return ev->aux2;
    }
    if (normalizedStatus == LIFECYCLE_STATUS_FIRE_SHIELD &&
        ev->aux0 == TIMELINE_AUX_FIRESHIELD) {
        return ev->aux3;
    }
    if (normalizedStatus == LIFECYCLE_STATUS_PARTY_SHIELD &&
        ev->aux0 == TIMELINE_AUX_PARTY_SHIELD) {
        return ev->aux4;
    }
    return ev->aux1;
}

static void orch_mirror_other_spell_lifecycle_status_pc34_compat(
    struct GameWorld_Compat* world,
    const struct SpellEffect_Compat* effect)
{
    int delta;
    if (!world || !effect) return;
    if (effect->spellKind != C3_SPELL_KIND_OTHER_COMPAT) return;

    /* ReDMCSB MENU.C F0412 updates the single G0407_s_Party status state.
     * Firestaff keeps a MagicState copy for spell/combat bookkeeping and a
     * Lifecycle copy for status rendering/expiry, so the F0412 start edge
     * must seed both mirrors before the matching C71/C73/C74/C78/C79 timeout
     * subtracts or decrements them. */
    if (effect->magicStateDelta[2] > 0) {
        world->lifecycle.status.partyShieldDefense =
            (int16_t)(world->lifecycle.status.partyShieldDefense +
                      effect->magicStateDelta[2]);
    }
    if (effect->magicStateDelta[1] > 0) {
        world->lifecycle.status.partyFireShieldDefense =
            (int16_t)(world->lifecycle.status.partyFireShieldDefense +
                      effect->magicStateDelta[1]);
    }
    if (effect->magicStateDelta[0] > 0) {
        world->lifecycle.status.partySpellShieldDefense =
            (int16_t)(world->lifecycle.status.partySpellShieldDefense +
                      effect->magicStateDelta[0]);
    }

    delta = effect->magicStateDelta[5];
    if (delta <= 0) return;
    switch (effect->spellType) {
        case C2_SPELL_TYPE_OTHER_THIEVES_EYE_COMPAT:
            world->lifecycle.status.thievesEyeCount =
                (uint16_t)(world->lifecycle.status.thievesEyeCount + delta);
            break;
        case C3_SPELL_TYPE_OTHER_INVISIBILITY_COMPAT:
            world->lifecycle.status.invisibilityCount =
                (uint16_t)(world->lifecycle.status.invisibilityCount + delta);
            break;
        case C6_SPELL_TYPE_OTHER_FOOTPRINTS_COMPAT:
            world->lifecycle.status.footprintsCount =
                (uint16_t)(world->lifecycle.status.footprintsCount + delta);
            break;
        default:
            break;
    }
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
    DM1_MeleeF0231CreatureSnapshotInputPc34 in;
    DM1_MeleeF0231CreatureSnapshotPlanPc34 plan;

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

    memset(&in, 0, sizeof(in));
    in.groupIndex = groupIndex;
    in.groupCount = group->count;
    in.groupCreatureType = group->creatureType;
    in.creatureIndex = creatureIndex;
    in.creatureHealth = group->health[creatureIndex];
    in.profileAttack = profile->baseAttack;
    in.profileDefense = profile->baseDefense;
    in.profileDexterity = profile->dexterity;
    in.profileBaseHealth = profile->baseHealth;
    in.profilePoisonAttack = profile->poisonAttack;
    in.profileAttackType = profile->attackType;
    in.profileAttributes = profile->attributes;
    in.profileWoundProbabilities = profile->woundProbabilities;
    in.profileProperties = profile->properties;
    in.doubledMapDifficulty = doubledMapDifficulty;
    if (world->magic.event71CountInvisibility > 0 &&
        !(profile->attributes & DM1_ATTR_SEE_INVISIBLE)) {
        in.creatureDifficulty = 16;
    } else if (profile->attributes & DM1_ATTR_NIGHT_VISION) {
        in.creatureDifficulty = 0;
    } else {
        struct DungeonViewLight_Compat light;
        if (!F0890b_ORCH_ComputeDungeonViewLight_Compat(world, &light)) {
            return 0;
        }
        in.creatureDifficulty = light.paletteIndex << 1;
    }
    in.candidateInvulnerableEnabled =
        world->candidateAttackInvulnerableEnabled;
    in.candidateInvulnerableGroupIndex =
        world->candidateAttackInvulnerableGroupIndex;
    in.candidateInvulnerableCreatureIndex =
        world->candidateAttackInvulnerableCreatureIndex;
    if (!dm1_v1_melee_creature_snapshot_plan_f0231_pc34(&in, &plan) ||
        !plan.valid) {
        return 0;
    }
    *outSnapshot = plan.snapshot;
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
    DM1_MeleeF0312StrengthInputPc34 in;
    DM1_MeleeF0312StrengthPlanPc34 plan;

    if (!world || !champion || !weaponInfo) return 0;

    memset(&in, 0, sizeof(in));
    memset(&plan, 0, sizeof(plan));
    in.championStrength = (int)champion->attributes[CHAMPION_ATTR_STRENGTH];
    in.currentStamina = (int)champion->stamina.current;
    in.maximumStamina = (int)champion->stamina.maximum;
    in.maximumLoad = (int)champion->maxLoad;
    in.random16 = F0732_COMBAT_RngRandom_Compat(&world->masterRng, 16);
    in.objectWeight = weaponInfo->weight;
    in.hasActionHandWeapon = hasActionHandWeapon;
    in.weaponStrength = weaponInfo->strength;
    in.weaponSkillBonus = F0888_ORCH_GetChampionF0312SkillBonus_Compat(
        world, championIndex, weaponInfo->weaponClass);
    in.actionHandWounded =
        (champion->wounds & COMBAT_WOUND_ACTION_HAND) != 0;
    if (!dm1_v1_melee_strength_plan_f0312_pc34(&in, &plan) ||
        !plan.valid) {
        return 0;
    }
    return plan.strengthActionHand;
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
    DM1_MeleeF0231ChampionSnapshotInputPc34 in;
    DM1_MeleeF0231ChampionSnapshotPlanPc34 plan;
    int normalizedActionSkillIndex;

    if (!outChampion || !world || !weaponInfo) return 0;
    memset(outChampion, 0, sizeof(*outChampion));
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champion = &world->party.champions[championIndex];

    memset(&in, 0, sizeof(in));
    memset(&plan, 0, sizeof(plan));
    in.championIndex = championIndex;
    in.championPresent = champion->present;
    in.currentHealth = champion->hp.current;
    in.actionSkillIndex = actionSkillIndex;
    in.weaponClass = weaponInfo->weaponClass;
    if (!dm1_v1_melee_champion_snapshot_plan_f0231_pc34(&in, &plan)) {
        return 0;
    }
    normalizedActionSkillIndex = plan.normalizedActionSkillIndex;

    in.dexterity = champion->attributes[CHAMPION_ATTR_DEXTERITY];
    in.strengthActionHand =
        orch_cmd_attack_f0312_strength_action_hand_compat(
            world, champion, championIndex, weaponInfo, hasActionHandWeapon);
    in.skillLevelParry = F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        world, championIndex, DM1_SKILL_IDX_PARRY);
    in.skillLevelAction = F0888_ORCH_GetChampionF0303SkillLevel_Compat(
        world, championIndex, normalizedActionSkillIndex);
    in.statisticVitality = champion->attributes[CHAMPION_ATTR_VITALITY];
    in.statisticAntifire = champion->attributes[CHAMPION_ATTR_ANTIFIRE];
    in.statisticAntimagic = champion->attributes[CHAMPION_ATTR_ANTIMAGIC];
    in.statisticWisdom = champion->attributes[CHAMPION_ATTR_WISDOM];
    in.statisticLuck = (int)world->lifecycle.champions[championIndex]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT];
    in.statisticLuckMax = (int)world->lifecycle.champions[championIndex]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_MAXIMUM];
    in.statisticLuckMin = (int)world->lifecycle.champions[championIndex]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_MINIMUM];
    in.actionHandIcon = weaponType;
    in.wounds = champion->wounds;
    in.isResting = world->partyIsResting || world->lifecycle.rest.isResting;
    in.partyShieldDefense = champion->actionDefense;
    if (!dm1_v1_melee_champion_snapshot_plan_f0231_pc34(&in, &plan) ||
        !plan.valid) {
        return 0;
    }
    *outChampion = plan.snapshot;
    return 1;
}

static void orch_writeback_cmd_attack_luck_compat(
    struct GameWorld_Compat* world,
    int championIndex,
    const struct CombatantChampionSnapshot_Compat* championSnapshot)
{
    DM1_MeleeF0231LuckWritebackInputPc34 in;
    DM1_MeleeF0231LuckWritebackPlanPc34 plan;
    if (!world || !championSnapshot) return;
    memset(&in, 0, sizeof(in));
    memset(&plan, 0, sizeof(plan));
    in.championIndex = championIndex;
    in.snapshotLuck = championSnapshot->statisticLuck;
    in.championCount = CHAMPION_MAX_PARTY;
    if (!dm1_v1_melee_luck_writeback_plan_f0231_pc34(&in, &plan) ||
        !plan.valid || !plan.shouldWriteBack) {
        return;
    }
    world->lifecycle.champions[plan.championIndex]
        .statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT] =
            (uint8_t)plan.clampedLuck;
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

static int orch_build_cmd_attack_weapon_profile_compat(
    const DM1_WeaponInfo* weaponInfo,
    int weaponType,
    int actionIndex,
    int actionSkillIndex,
    struct WeaponProfile_Compat* outWeapon)
{
    DM1_MeleeWeaponProfileInputPc34 in;
    DM1_MeleeWeaponProfilePlanPc34 plan;
    if (!weaponInfo || !outWeapon) return 0;
    memset(&in, 0, sizeof(in));
    memset(&plan, 0, sizeof(plan));
    in.weaponType = weaponType;
    in.weaponClass = weaponInfo->weaponClass;
    in.weaponStrength = weaponInfo->strength;
    in.kineticEnergy = weaponInfo->kineticEnergy;
    in.weaponAttributes = weaponInfo->attributes;
    in.actionIndex = actionIndex;
    in.actionSkillIndex = actionSkillIndex;
    if (!dm1_v1_melee_weapon_profile_plan_f0402_f0231_pc34(&in, &plan) ||
        !plan.valid) {
        memset(outWeapon, 0, sizeof(*outWeapon));
        return 0;
    }
    *outWeapon = plan.weaponProfile;
    return 1;
}

static int orch_cmd_attack_has_live_group_table_compat(
    const struct GameWorld_Compat* world)
{
    return world && world->things && world->things->groups &&
        world->things->groupCount > 0;
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

static void orch_cmd_cast_spell_award_f0412_experience_compat(
    struct GameWorld_Compat* world,
    int championIndex,
    int skillIndex,
    int experience,
    int successfulCast,
    struct TickResult_Compat* result)
{
    int baseSkillIndex;

    if (!world || championIndex < 0 ||
        championIndex >= CHAMPION_MAX_PARTY || experience <= 0 ||
        skillIndex < 0 || skillIndex >= DM1_TOTAL_SKILL_COUNT) {
        return;
    }

    baseSkillIndex = dm1_skill_get_base_index(skillIndex);
    if (baseSkillIndex < 0 || baseSkillIndex >= CHAMPION_SKILL_COUNT) {
        return;
    }

    /* ReDMCSB MENU.C F0412 lines 1835-1841 calls F0304 with the
     * shifted experience before returning NEEDS_MORE_PRACTICE.  Use the
     * same live lifecycle/F0304 bridge as command-side action XP; a failed
     * spell must not silently discard the receipt's source-owned partial XP. */
    (void)F0849_LIFECYCLE_AddSkillExperience_Compat(
        &world->lifecycle.champions[championIndex], skillIndex, experience,
        orch_cmd_attack_map_difficulty_compat(world), world->gameTick,
        world->lifecycle.lastCreatureAttackTime, NULL, NULL);
    world->party.champions[championIndex].skillExperience[baseSkillIndex] =
        (unsigned long)world->lifecycle.champions[championIndex]
            .skills20[baseSkillIndex].experience;
    emit(result, EMIT_XP_AWARD, championIndex, skillIndex, experience,
         successfulCast ? 1 : 0);
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

static int orch_unlink_thing_from_square_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned short thingToUnlink);

static int orch_c24_find_fluxcage_thing_compat(
    const struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    unsigned short* outThing);

static void orch_remove_active_group_state_compat(
    struct GameWorld_Compat* world,
    int groupIndex);
static void orch_cmd_attack_cleanup_f0190_killed_all_events_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY);

static unsigned short orch_make_thing_ref_compat(int type, int index);

static int orch_group_creature_cell_compat(
    const struct DungeonGroup_Compat* group,
    int creatureIndex);
static int orch_find_active_group_state_index_compat(
    const struct GameWorld_Compat* world,
    int groupIndex);
static int orch_pack_group_directions_compat(int direction, int creatureCount);
static int orch_active_group_directions_compat(
    const struct CreatureAIState_Compat* ai,
    const struct DungeonGroup_Compat* group);
static int orch_apply_f0205_active_creature_direction_compat(
    struct GameWorld_Compat* world,
    struct CreatureAIState_Compat* ai,
    struct DungeonGroup_Compat* group,
    struct DM1ActiveGroup_Compat* activeGroup,
    int direction,
    int creatureIndex,
    int creatureSize);
static int orch_ai_state_to_dm1_behavior_compat(int stateKind);

static void orch_cmd_attack_apply_f0231_side_effects_compat(
    struct GameWorld_Compat* world,
    int championIndex,
    int actionSkillIndex,
    const struct CombatantCreatureSnapshot_Compat* creature,
    int damageApplied,
    struct TickResult_Compat* result)
{
    struct ChampionState_Compat* champion;
    DM1_MeleeF0231SideEffectInputPc34 in;
    DM1_MeleeF0231SideEffectPlanPc34 plan;

    if (!world) return;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return;
    champion = &world->party.champions[championIndex];
    if (!champion->present || champion->hp.current <= 0) return;

    memset(&in, 0, sizeof(in));
    memset(&plan, 0, sizeof(plan));
    in.championIndex = championIndex;
    in.actionSkillIndex = actionSkillIndex;
    in.damageApplied = damageApplied;
    in.creatureProperties = creature ? creature->properties : 0;
    in.mapDifficulty = orch_cmd_attack_map_difficulty_compat(world);
    in.currentTick = world->gameTick;
    in.lastCreatureAttackTime = world->lifecycle.lastCreatureAttackTime;
    in.currentStamina = champion->stamina.current;
    in.maximumStamina = champion->stamina.maximum;
    in.currentHealth = champion->hp.current;
    if (!dm1_v1_melee_side_effect_plan_f0231_pc34(&in, &plan) ||
        !plan.valid) {
        return;
    }
    in.staminaRandomValue = (int)F0732_COMBAT_RngRandom_Compat(
        &world->masterRng, plan.staminaRandomModulus);
    if (!dm1_v1_melee_side_effect_plan_f0231_pc34(&in, &plan) ||
        !plan.valid) {
        return;
    }
    if (plan.shouldAwardXp) {
        (void)F0849_LIFECYCLE_AddSkillExperience_Compat(
            &world->lifecycle.champions[plan.xpChampionIndex],
            plan.skillIndex,
            plan.experienceGain,
            plan.xpMapDifficulty,
            plan.xpCurrentTick,
            plan.xpLastCreatureAttackTime,
            0,
            0);
    }
    if (plan.shouldWriteChampionState) {
        world->party.champions[plan.championIndex].stamina.current =
            (unsigned short)plan.currentStaminaAfter;
        world->party.champions[plan.championIndex].hp.current =
            (int16_t)plan.currentHealthAfter;
        if (plan.currentHealthAfter <= 0) {
            /* ReDMCSB CHAMPION.C F0319 is reached when F0325/F0231 stamina
             * underflow drains the last HP; keep the M10 event stream aligned. */
            emit(result, EMIT_CHAMPION_DOWN, plan.championIndex, 0, 0, 0);
        }
    }
}

/* ReDMCSB GROUP.C F0181: only C29..C41 reactions on the exact current-map
 * square are deleted.  F0182 and F0195 share this primitive. */
static int orch_delete_group_reaction_events_f0181_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY)
{
    int oldEventCount;
    int readIndex;
    int writeIndex = 0;

    if (!world || world->timeline.count < 0 ||
        world->timeline.count > TIMELINE_QUEUE_CAPACITY) {
        return 0;
    }
    oldEventCount = world->timeline.count;
    for (readIndex = 0; readIndex < oldEventCount; ++readIndex) {
        const struct TimelineEvent_Compat* event =
            &world->timeline.events[readIndex];

        if (event->kind == TIMELINE_EVENT_CREATURE_REACTION &&
            event->mapIndex == mapIndex && event->mapX == mapX &&
            event->mapY == mapY &&
            event->aux2 >= DM1_EVENT_REACTION_DANGER_ON_SQUARE &&
            event->aux2 <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
            continue;
        }
        world->timeline.events[writeIndex++] = *event;
    }
    world->timeline.count = writeIndex;
    while (writeIndex < oldEventCount) {
        memset(&world->timeline.events[writeIndex], 0,
               sizeof(world->timeline.events[writeIndex]));
        ++writeIndex;
    }
    return 1;
}

int F0182_DM1_GROUP_StopAttacking_Compat(
    struct GameWorld_Compat* world,
    struct DM1ActiveGroup_Compat* activeGroup,
    int mapIndex,
    int mapX,
    int mapY)
{
    int aspectIndex;
    if (!world || !activeGroup) return 0;

    /* GROUP.C F0182:374-381 clears only MASK0x0080 in all four Aspect
     * bytes, then delegates the square-local C29..C41 removal to F0181. */
    for (aspectIndex = 0; aspectIndex < 4; ++aspectIndex) {
        activeGroup->aspect[aspectIndex] &= ~0x80;
    }
    return orch_delete_group_reaction_events_f0181_compat(
        world, mapIndex, mapX, mapY);
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
        /* ReDMCSB MENU.C F0407:1266-1272 derives L1251/L1252 from the
         * champion-facing direction before it invokes F0402.  This helper
         * needs that coordinate only; rebuilding a F0402 receipt here lost
         * its action-owner fields and fail-closed to (0,0). */
        mapIndex = world->party.mapIndex;
        mapX = world->party.mapX;
        mapY = world->party.mapY;
        switch (direction & 3) {
        case DIR_NORTH: --mapY; break;
        case DIR_EAST:  ++mapX; break;
        case DIR_SOUTH: ++mapY; break;
        case DIR_WEST:  --mapX; break;
        }
    }
    if (outMapIndex) *outMapIndex = mapIndex;
    if (outMapX) *outMapX = mapX;
    if (outMapY) *outMapY = mapY;
}

/* F0231 ends by asking F0209 to create C31.  F0209 then owns the live
 * C38/C39 recoil/attack progression.  Do not schedule that bridge from a
 * decoded-only group: PC34 C04 and the physical SFT owner must still agree. */
int F0890b_ORCH_AdmitF0231ReactionSource_Compat(
    const struct GameWorld_Compat* world,
    int groupIndex,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonGroup_Compat* group;
    const unsigned char* raw;
    unsigned short thing;
    int matches = 0;
    int guard = 0;

    if (!world || !world->dungeon || !world->things ||
        !world->dungeon->loaded || !world->dungeon->tilesLoaded ||
        !world->things->loaded || !world->things->groups ||
        !world->things->rawThingData[THING_TYPE_GROUP] ||
        groupIndex < 0 || groupIndex >= world->things->groupCount ||
        groupIndex >= world->things->thingCounts[THING_TYPE_GROUP] ||
        mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount ||
        orch_find_active_group_state_index_compat(world, groupIndex) < 0) {
        return 0;
    }
    group = &world->things->groups[groupIndex];
    raw = world->things->rawThingData[THING_TYPE_GROUP] + groupIndex * 16;
    if (group->next == THING_NONE || r_u16(raw) != group->next ||
        r_u16(raw + 2) != group->slot || raw[4] != group->creatureType) {
        return 0;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        world->dungeon, world->things, mapIndex, mapX, mapY);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && guard++ < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_GROUP &&
            THING_GET_INDEX(thing) == groupIndex) {
            ++matches;
        }
        thing = orch_next_thing_compat(world->things, thing);
    }
    return matches == 1 && guard < 64;
}

static int orch_cmd_attack_schedule_f0231_reaction_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    const struct CombatantCreatureSnapshot_Compat* creature,
    int targetDirection,
    int outcome)
{
    struct TimelineEvent_Compat reaction;
    DM1_MeleeF0231ReactionInputPc34 in;
    DM1_MeleeF0231ReactionPlanPc34 plan;
    int mapIndex;
    int mapX;
    int mapY;

    if (!world || groupIndex < 0) return 0;

    orch_cmd_attack_target_square_compat(
        world, targetDirection, &mapIndex, &mapX, &mapY);
    memset(&in, 0, sizeof(in));
    memset(&plan, 0, sizeof(plan));
    in.groupIndex = groupIndex;
    in.creatureType = creature ? creature->creatureType : -1;
    in.mapIndex = mapIndex;
    in.mapX = mapX;
    in.mapY = mapY;
    in.currentTick = world->gameTick;
    in.outcome = outcome;
    if (!dm1_v1_melee_reaction_plan_f0231_pc34(&in, &plan) ||
        !plan.valid || !plan.shouldSchedule ||
        !F0890b_ORCH_AdmitF0231ReactionSource_Compat(
            world, plan.groupIndex, plan.mapIndex, plan.mapX, plan.mapY)) {
        return 0;
    }
    memset(&reaction, 0, sizeof(reaction));
    reaction.kind = TIMELINE_EVENT_CREATURE_REACTION;
    reaction.fireAtTick = plan.fireAtTick;
    reaction.mapIndex = plan.mapIndex;
    reaction.mapX = plan.mapX;
    reaction.mapY = plan.mapY;
    reaction.aux0 = plan.groupIndex;
    reaction.aux1 = plan.creatureType;
    reaction.aux2 = plan.eventKind;
    return F0721_TIMELINE_Schedule_Compat(&world->timeline, &reaction);
}

static void orch_cmd_attack_apply_group_kill_side_effects_plan_f0190_compat(
    struct GameWorld_Compat* world,
    const DM1_MeleeF0190KilledAllStatePlanPc34* plan);

static void orch_cmd_attack_apply_group_kill_side_effects_plan_f0190_compat(
    struct GameWorld_Compat* world,
    const DM1_MeleeF0190KilledAllStatePlanPc34* plan)
{
    DM1_MeleeF0190KilledAllStateApplyPlanPc34 applyPlan;

    if (!world || !world->things || !plan || !plan->valid) return;
    memset(&applyPlan, 0, sizeof(applyPlan));
    if (!dm1_v1_melee_killed_all_state_apply_plan_f0190_pc34(
            plan, &applyPlan) ||
        !applyPlan.valid) {
        return;
    }
    if (applyPlan.shouldClearGroupNext &&
        applyPlan.groupIndex >= 0 &&
        applyPlan.groupIndex < world->things->groupCount &&
        world->things->groups) {
        /* ReDMCSB GROUP.C F0189 clears all C04 health slots before F0267
         * unlinks the group. F0190 only guarantees the killed slot itself,
         * so stale unused HP values must not reach native save/export. */
        memset(world->things->groups[applyPlan.groupIndex].health, 0,
               sizeof(world->things->groups[applyPlan.groupIndex].health));
    }
    if (applyPlan.shouldUnlinkGroupFromSquare) {
        (void)orch_unlink_thing_from_square_compat(
            world, applyPlan.mapIndex, applyPlan.mapX, applyPlan.mapY,
            applyPlan.groupThing);
    }
    if (applyPlan.shouldClearGroupNext &&
        applyPlan.groupIndex >= 0 &&
        applyPlan.groupIndex < world->things->groupCount &&
        world->things->groups) {
        world->things->groups[applyPlan.groupIndex].next =
            applyPlan.clearedNextThing;
        /* ReDMCSB GROUP.C F0188 drops GROUP.Slot before F0189 clears
         * GROUP.Next. Persist both writes together: native save/export
         * reads the raw C04 record, not this decoded-only mutation. */
        orch_write_raw_group_compat(world->things, applyPlan.groupIndex);
    }
    /* ReDMCSB GROUP.C F0189 lines 759-766 retires ACTIVE_GROUP only while
     * the killed group is on the party's current map. The unlink, Next clear,
     * and F0181 event cleanup remain source-square operations even off-map. */
    if (applyPlan.shouldRemoveActiveGroupState &&
        applyPlan.mapIndex == world->partyMapIndex) {
        orch_remove_active_group_state_compat(world, applyPlan.groupIndex);
    }
    if (applyPlan.shouldDeleteGroupEvents) {
        orch_cmd_attack_cleanup_f0190_killed_all_events_compat(
            world, applyPlan.mapIndex, applyPlan.mapX, applyPlan.mapY);
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

/* ReDMCSB GROUP.C F0209:2173-2185 reaches MOVESENS.C F0267 only after
 * F0202 has selected a legal LoS/target step.  A Firestaff-side record
 * capacity failure must therefore leave that source C04 chain untouched:
 * F0267 did not complete, so F0209 does not commit its active-group move.
 * Keep the exact predecessor/successor pair so rollback preserves record
 * order instead of reinserting the group at the square head. */
struct OrchThingUnlinkReceipt_Compat {
    int valid;
    int sourceFirstThingIndex;
    unsigned short thing;
    unsigned short previousThing;
    unsigned short nextThing;
};

static int orch_unlink_thing_from_square_with_receipt_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned short thingToUnlink,
    struct OrchThingUnlinkReceipt_Compat* outReceipt)
{
    int sftIndex;
    unsigned short thing;
    unsigned short previous = THING_NONE;
    unsigned short target = (unsigned short)(thingToUnlink & 0x3FFFu);
    int safety = 0;

    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!world || !world->dungeon || !world->things || !outReceipt ||
        thingToUnlink == THING_NONE || thingToUnlink == THING_ENDOFLIST) {
        return 0;
    }
    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
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
            if (!orch_set_next_thing_compat(world->things, thingToUnlink,
                                            THING_ENDOFLIST)) {
                return 0;
            }
            outReceipt->valid = 1;
            outReceipt->sourceFirstThingIndex = sftIndex;
            outReceipt->thing = thingToUnlink;
            outReceipt->previousThing = previous;
            outReceipt->nextThing = nextThing;
            return 1;
        }
        previous = thing;
        thing = nextThing;
    }
    return 0;
}

static int orch_restore_unlinked_thing_receipt_compat(
    struct GameWorld_Compat* world,
    const struct OrchThingUnlinkReceipt_Compat* receipt)
{
    if (!world || !world->things || !receipt || !receipt->valid ||
        receipt->sourceFirstThingIndex < 0 ||
        receipt->sourceFirstThingIndex >= world->things->squareFirstThingCount) {
        return 0;
    }
    if (!orch_set_next_thing_compat(
            world->things, receipt->thing, receipt->nextThing)) {
        return 0;
    }
    if (receipt->previousThing == THING_NONE) {
        world->things->squareFirstThings[receipt->sourceFirstThingIndex] =
            receipt->thing;
        return 1;
    }
    return orch_set_next_thing_compat(
        world->things, receipt->previousThing, receipt->thing);
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
    struct GameWorld_Compat* world,
    int championIndex,
    int groupIndex,
    int groupMapX,
    int groupMapY)
{
    const struct DungeonGroup_Compat* group;
    DM1_MeleeF0177TargetCreatureInputPc34 in;
    DM1_MeleeF0177TargetCreaturePlanPc34 plan;
    int i;

    if (!world || !world->things || !world->things->groups) return -1;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return -1;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return -1;

    group = &world->things->groups[groupIndex];
    memset(&in, 0, sizeof(in));
    memset(&plan, 0, sizeof(plan));
    in.groupCount = (int)group->count;
    in.groupCells = (int)group->cells;
    in.groupDirection = (int)group->direction;
    in.creatureSize = 0;
    {
        const struct CreatureBehaviorProfile_Compat* profile =
            CREATURE_GetProfile_Compat((int)group->creatureType);
        if (profile) {
            in.creatureSize = (int)(profile->attributes & DM1_ATTR_SIZE_MASK);
        }
    }
    for (i = 0; i <= (int)group->count && i < 4; ++i) {
        in.creatureHealth[i] = (int)group->health[i];
    }
    in.championCell = (int)(world->party.champions[championIndex].cell & 3);
    if (!F0229_DM1_GROUP_SetOrderedCellsToAttack_Compat(
            in.orderedCells, groupMapX, groupMapY,
            world->party.mapX, world->party.mapY,
            (unsigned int)in.championCell, &world->masterRng)) {
        return -1;
    }
    in.hasOrderedCells = 1;

    if (!dm1_v1_melee_target_creature_plan_f0177_pc34(&in, &plan) ||
        !plan.valid) {
        return -1;
    }
    return plan.selectedCreatureIndex;
}

static int orch_cmd_attack_champion_reach_blocked_f0407_compat(
    const struct GameWorld_Compat* world,
    int championIndex,
    int targetDirection)
{
    DM1_MeleeReachGateInputPc34 in;
    DM1_MeleeReachGatePlanPc34 plan;
    const struct ChampionState_Compat* champion;
    int i;

    if (!world) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champion = &world->party.champions[championIndex];
    memset(&in, 0, sizeof(in));
    memset(&plan, 0, sizeof(plan));
    in.championIndex = championIndex;
    in.championPresent = champion->present;
    in.championCurrentHealth = champion->hp.current;
    in.championCell = (int)champion->cell;
    in.targetDirection = targetDirection;
    in.partyChampionCount = CHAMPION_MAX_PARTY;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        const struct ChampionState_Compat* other = &world->party.champions[i];
        in.otherChampionPresent[i] = other->present;
        in.otherChampionCurrentHealth[i] = other->hp.current;
        in.otherChampionCell[i] = (int)other->cell;
    }
    return dm1_v1_melee_reach_gate_plan_f0402_pc34(&in, &plan) &&
           plan.valid && plan.blocked;
}

static int orch_cmd_attack_disrupt_material_blocked_f0407_compat(
    const struct GameWorld_Compat* world,
    int actionIndex,
    int groupIndex)
{
    DM1_MeleeDisruptMaterialGateInputPc34 in;
    DM1_MeleeDisruptMaterialGatePlanPc34 plan;
    const struct CreatureBehaviorProfile_Compat* profile;
    int creatureType;

    if (!world || !world->things) return 0;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;

    creatureType = world->things->groups[groupIndex].creatureType;
    profile = CREATURE_GetProfile_Compat(creatureType);
    if (!profile) return 0;

    memset(&in, 0, sizeof(in));
    memset(&plan, 0, sizeof(plan));
    in.actionIndex = actionIndex;
    in.targetCreatureAttributes = profile->attributes;
    return dm1_v1_melee_disrupt_material_gate_plan_f0402_pc34(&in, &plan) &&
           plan.valid && plan.blocked;
}

static int orch_cmd_attack_resolve_target_compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* input,
    const DM1_MeleeF0402CommandDecodePlanPc34* decodePlan,
    int* outGroupIndex,
    int* outCreatureIndex)
{
    int groupIndex;
    int creatureIndex;

    if (outGroupIndex) *outGroupIndex = -1;
    if (outCreatureIndex) *outCreatureIndex = -1;
    if (!world || !input || !decodePlan || !world->things) return 0;

    groupIndex = decodePlan->directGroupIndex;
    creatureIndex = decodePlan->directCreatureIndex;
    if (decodePlan->requestedAutoTarget) {
        if (!orch_cmd_attack_find_group_on_square_compat(
                world, decodePlan->targetMapIndex, decodePlan->targetMapX,
                decodePlan->targetMapY, &groupIndex)) {
            return 0;
        }
    }

    if (groupIndex < 0 || groupIndex >= world->things->groupCount ||
        !world->things->groups) {
        return 0;
    }
    if (decodePlan->requestedAutoCreature) {
        creatureIndex = orch_cmd_attack_f0177_creature_slot_compat(
            world, (int)input->commandArg1, groupIndex,
            decodePlan->targetMapX, decodePlan->targetMapY);
        if (creatureIndex < 0 ||
            creatureIndex > (int)world->things->groups[groupIndex].count) {
            return 0;
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

static int orch_f0200_closed_door_blocks_view_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned char squareByte)
{
    const struct DungeonMapDesc_Compat* map;
    int doorIndex = -1;
    int doorSet;

    if ((squareByte & DUNGEON_SQUARE_MASK_TYPE) !=
        (DUNGEON_ELEMENT_DOOR << 5) ||
        ((squareByte & 0x07u) != 3u && (squareByte & 0x07u) != 4u)) {
        return 0;
    }
    if (!world || !world->dungeon || !world->dungeon->maps ||
        mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 1;
    }

    /* ReDMCSB GROUP.C F0197 lines 1200-1207 asks the active map's
     * DoorInfo whether a three-quarter or closed door is see-through.
     * DUNGEON.C lines 560-565 defines Portcullis (0) and Ra (3) as the
     * two see-through DM1 door types. */
    map = &world->dungeon->maps[mapIndex];
    if (orch_cmd_attack_find_door_on_square_compat(
            world, mapIndex, mapX, mapY, &doorIndex) &&
        doorIndex >= 0 && world->things && world->things->doors &&
        doorIndex < world->things->doorCount) {
        doorSet = world->things->doors[doorIndex].type ? map->doorSet1
                                                        : map->doorSet0;
        return (doorSet & 3) != 0 && (doorSet & 3) != 3;
    }
    return 1;
}

static int orch_f0199_square_blocks_view_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY)
{
    unsigned char squareByte;
    int squareType;

    if (!orch_read_square_byte_compat(
            world ? world->dungeon : NULL, mapIndex, mapX, mapY,
            &squareByte)) {
        return 1;
    }
    squareType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    /* ReDMCSB GROUP.C F0197 lines 1191-1207: walls, closed fakewalls,
     * and opaque three-quarter/closed doors stop creature sight. */
    return squareType == DUNGEON_ELEMENT_WALL ||
           (squareType == DUNGEON_ELEMENT_FAKEWALL &&
            !(squareByte & 0x04u)) ||
           orch_f0200_closed_door_blocks_view_compat(
               world, mapIndex, mapX, mapY, squareByte);
}

/* GROUP.C F0198 differs from F0197 only at its blockers: doors and imaginary
 * fake walls do not stop smell.  This reads the loaded raw square on every
 * F0199 step; absent/unsupported data remains blocking. */
static int orch_f0198_square_blocks_smell_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY)
{
    unsigned char squareByte;
    int squareType;

    if (!orch_read_square_byte_compat(
            world ? world->dungeon : NULL, mapIndex, mapX, mapY,
            &squareByte)) {
        return 1;
    }
    squareType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    return squareType == DUNGEON_ELEMENT_WALL ||
           (squareType == DUNGEON_ELEMENT_FAKEWALL &&
            !(squareByte & 0x04u) && !(squareByte & 0x01u));
}

static int orch_f0198_square_blocks_smell_callback_compat(
    int mapX, int mapY, void* context)
{
    const struct GameWorld_Compat* world =
        (const struct GameWorld_Compat*)context;
    if (!world) return 1;
    return orch_f0198_square_blocks_smell_compat(
        world, world->party.mapIndex, mapX, mapY);
}

static uint32_t orch_party_scent_fingerprint_compat(
    const DM1_V1_NeedsScentListPc34Compat* scents)
{
    uint32_t hash = 2166136261u;
    uint16_t index;
    if (!scents) return 0;
#define ORCH_SCENT_FNV_BYTE(value) \
    do { hash ^= (uint8_t)(value); hash *= 16777619u; } while (0)
    ORCH_SCENT_FNV_BYTE(scents->count);
    ORCH_SCENT_FNV_BYTE(scents->count >> 8);
    ORCH_SCENT_FNV_BYTE(scents->firstScentIndex);
    ORCH_SCENT_FNV_BYTE(scents->firstScentIndex >> 8);
    ORCH_SCENT_FNV_BYTE(scents->lastScentIndex);
    ORCH_SCENT_FNV_BYTE(scents->lastScentIndex >> 8);
    for (index = 0; index < scents->count; ++index) {
        const DM1_V1_NeedsScentPc34Compat* scent = &scents->entries[index];
        ORCH_SCENT_FNV_BYTE(scent->mapX);
        ORCH_SCENT_FNV_BYTE(scent->mapX >> 8);
        ORCH_SCENT_FNV_BYTE(scent->mapY);
        ORCH_SCENT_FNV_BYTE(scent->mapY >> 8);
        ORCH_SCENT_FNV_BYTE(scent->mapIndex);
        ORCH_SCENT_FNV_BYTE(scent->mapIndex >> 8);
        ORCH_SCENT_FNV_BYTE(scent->strength);
    }
#undef ORCH_SCENT_FNV_BYTE
    return hash;
}

int F0890d_ORCH_PublishPartyScentReceipt_Compat(
    struct GameWorld_Compat* world,
    const DM1_V1_NeedsScentListPc34Compat* sourceScents,
    uint32_t sourceFingerprint)
{
    uint16_t index;

    if (!world || !sourceScents || !world->dungeon ||
        sourceScents->count == 0 ||
        sourceScents->count > DM1_V1_NEEDS_SCENT_CAPACITY ||
        sourceScents->firstScentIndex > sourceScents->count ||
        sourceScents->lastScentIndex > sourceScents->count ||
        sourceFingerprint == 0 ||
        orch_party_scent_fingerprint_compat(sourceScents) != sourceFingerprint) {
        return 0;
    }
    for (index = 0; index < sourceScents->count; ++index) {
        const DM1_V1_NeedsScentPc34Compat* scent = &sourceScents->entries[index];
        const struct DungeonMapDesc_Compat* map;
        if (scent->strength == 0 ||
            scent->mapIndex >= world->dungeon->header.mapCount) {
            return 0;
        }
        map = &world->dungeon->maps[scent->mapIndex];
        if (scent->mapX >= map->width || scent->mapY >= map->height) return 0;
    }
    world->pc34PartyScentReceipt = *sourceScents;
    world->pc34PartyScentReceiptFingerprint = sourceFingerprint;
    world->pc34PartyScentReceiptValid = 1;
    return 1;
}

int F0890e_ORCH_BuildGroupSmellDirectionPlan_Compat(
    struct GameWorld_Compat* world,
    const struct DM1GroupBehaviorContext_Compat* context,
    struct DM1GroupSmellDirectionPlan_Compat* outPlan)
{
    struct DM1GroupBehaviorContext_Compat sourceContext;
    struct DM1GroupScent_Compat scent;
    const struct DM1GroupScent_Compat* scentPtr = NULL;
    int scentOrdinal;

    if (!world || !context || !outPlan || !world->dungeon ||
        context->currentMapIndex != world->party.mapIndex ||
        context->partyMapIndex != world->party.mapIndex) {
        return 0;
    }
    memset(&sourceContext, 0, sizeof(sourceContext));
    memset(&scent, 0, sizeof(scent));
    sourceContext = *context;
    sourceContext.isSmellSquareBlocked =
        orch_f0198_square_blocks_smell_callback_compat;
    sourceContext.smellBlockerContext = world;

    if (world->pc34PartyScentReceiptValid &&
        orch_party_scent_fingerprint_compat(&world->pc34PartyScentReceipt) ==
            world->pc34PartyScentReceiptFingerprint) {
        scentOrdinal = DM1_V1_Needs_GetScentOrdinalPc34Compat(
            &world->pc34PartyScentReceipt,
            (uint16_t)context->currentMapIndex,
            (uint16_t)context->partyMapX,
            (uint16_t)context->partyMapY);
        if (scentOrdinal > 0 &&
            scentOrdinal <= (int)world->pc34PartyScentReceipt.count) {
            const DM1_V1_NeedsScentPc34Compat* entry =
                &world->pc34PartyScentReceipt.entries[scentOrdinal - 1];
            scent.present = 1;
            scent.strength = entry->strength;
            scent.mapX = entry->mapX;
            scent.mapY = entry->mapY;
            scentPtr = &scent;
        }
    }
    return F0819c_DM1_GROUP_BuildSmelledPartyDirectionPlanWithRoute_Compat(
        &sourceContext, scentPtr, &world->masterRng, outPlan);
}

/* ReDMCSB GROUP.C F0199 lines 1239-1320.  This is deliberately not a
 * generic Bresenham walk: equal-axis paths test both orthogonal corners,
 * while non-equal paths retain F0199's fixed-point tie behavior. */
static int orch_f0199_distance_between_unblocked_squares_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY)
{
    int distanceX;
    int distanceY;
    int pathMapX;
    int pathMapY;
    int xAxisStep;
    int yAxisStep;
    int largestAxisDistance;
    int valueA;
    int valueB;
    int valueC;
    int distanceXSmallerThanDistanceY;
    int distanceXEqualsDistanceY;
    int distance;

    distance = abs(sourceMapX - destinationMapX) +
               abs(sourceMapY - destinationMapY);
    if (distance <= 1) return 1;

    distanceX = abs(destinationMapX - sourceMapX);
    distanceY = abs(destinationMapY - sourceMapY);
    distanceXSmallerThanDistanceY = distanceX < distanceY;
    distanceXEqualsDistanceY = distanceX == distanceY;
    pathMapX = destinationMapX;
    pathMapY = destinationMapY;
    xAxisStep = (pathMapX - sourceMapX) > 0 ? -1 : 1;
    yAxisStep = (pathMapY - sourceMapY) > 0 ? -1 : 1;

    largestAxisDistance = distanceXSmallerThanDistanceY
        ? pathMapY - sourceMapY : pathMapX - sourceMapX;
    valueC = largestAxisDistance != 0
        ? ((distanceXSmallerThanDistanceY
                ? pathMapX - sourceMapX : pathMapY - sourceMapY) * 64) /
              largestAxisDistance
        : 0x80;

    do {
        if (distanceXEqualsDistanceY) {
            if ((orch_f0199_square_blocks_view_compat(
                     world, mapIndex, pathMapX + xAxisStep, pathMapY) &&
                 orch_f0199_square_blocks_view_compat(
                     world, mapIndex, pathMapX, pathMapY + yAxisStep)) ||
                orch_f0199_square_blocks_view_compat(
                    world, mapIndex, pathMapX += xAxisStep,
                    pathMapY += yAxisStep)) {
                return 0;
            }
        } else {
            int candidateXAxis;
            int candidateYAxis;

            largestAxisDistance = distanceXSmallerThanDistanceY
                ? pathMapY - sourceMapY : pathMapX + xAxisStep - sourceMapX;
            candidateXAxis = largestAxisDistance != 0
                ? ((distanceXSmallerThanDistanceY
                        ? pathMapX + xAxisStep - sourceMapX
                        : pathMapY - sourceMapY) * 64) / largestAxisDistance
                : 0x80;
            valueA = abs(candidateXAxis - valueC);

            largestAxisDistance = distanceXSmallerThanDistanceY
                ? pathMapY + yAxisStep - sourceMapY : pathMapX - sourceMapX;
            candidateYAxis = largestAxisDistance != 0
                ? ((distanceXSmallerThanDistanceY
                        ? pathMapX - sourceMapX
                        : pathMapY + yAxisStep - sourceMapY) * 64) /
                      largestAxisDistance
                : 0x80;
            valueB = abs(candidateYAxis - valueC);

            if (valueA < valueB) pathMapX += xAxisStep;
            else pathMapY += yAxisStep;

            if (orch_f0199_square_blocks_view_compat(
                    world, mapIndex, pathMapX, pathMapY) &&
                (valueA != valueB ||
                 orch_f0199_square_blocks_view_compat(
                     world, mapIndex, pathMapX += xAxisStep,
                     pathMapY -= yAxisStep))) {
                return 0;
            }
        }
    } while (abs(pathMapX - sourceMapX) + abs(pathMapY - sourceMapY) > 1);

    return distance;
}

static int orch_f0200_get_distance_to_visible_party_compat(
    struct GameWorld_Compat* world,
    const struct DM1GroupBehaviorContext_Compat* ctx,
    const struct DungeonGroup_Compat* group)
{
    struct DungeonViewLight_Compat dungeonLight;
    int dx;
    int dy;
    int distance;
    int direction;
    int sightRange;
    if (!world || !ctx || !group || !world->dungeon ||
        ctx->currentMapIndex != ctx->partyMapIndex) {
        return 0;
    }
    dx = ctx->partyMapX - ctx->currentGroupMapX;
    dy = ctx->partyMapY - ctx->currentGroupMapY;
    distance = abs(dx) + abs(dy);

    direction = (int)group->direction & 3;
    if (!(ctx->creatureInfo.attributes & DM1_ATTR_SIDE_ATTACK) &&
        !F0227_DM1_GROUP_IsDestinationVisibleFromSource_Compat(
            direction, ctx->currentGroupMapX, ctx->currentGroupMapY,
            ctx->partyMapX, ctx->partyMapY)) {
        return 0;
    }

    /* ReDMCSB GROUP.C F0200 lines 1367-1405: PC34 first applies the
     * invisibility hard gate, then dims ordinary sight by the current
     * dungeon-view palette. Adjacent detection retains the source's bounded
     * random awareness exception, so this path advances only masterRng. */
    sightRange = DM1_SIGHT_RANGE(ctx->creatureInfo.ranges);
    if (world->magic.event71CountInvisibility > 0 &&
        !(ctx->creatureInfo.attributes & DM1_ATTR_SEE_INVISIBLE)) {
        sightRange = -10;
    } else if (!(ctx->creatureInfo.attributes & DM1_ATTR_NIGHT_VISION)) {
        memset(&dungeonLight, 0, sizeof(dungeonLight));
        if (F0890b_ORCH_ComputeDungeonViewLight_Compat(world, &dungeonLight)) {
            sightRange -= dungeonLight.paletteIndex >> 1;
        }
    }
    if (distance > sightRange) {
        if (distance == 1) {
            sightRange += F0732_COMBAT_RngRandom_Compat(
                              &world->masterRng,
                              DM1_XXX_RANGE(ctx->creatureInfo.ranges) + 1) +
                          F0732_COMBAT_RngRandom_Compat(
                              &world->masterRng,
                              DM1_SMELL_RANGE(ctx->creatureInfo.ranges) + 1);
            if (F0732_COMBAT_RngRandom_Compat(&world->masterRng, 8) == 0) {
                sightRange += F0732_COMBAT_RngRandom_Compat(
                    &world->masterRng, 1) + 5;
            }
        }
        if (distance > sightRange +
                           F0732_COMBAT_RngRandom_Compat(
                               &world->masterRng, 8) - 3) {
            return 0;
        }
    }

    return orch_f0199_distance_between_unblocked_squares_compat(
        world, ctx->currentMapIndex, ctx->currentGroupMapX,
        ctx->currentGroupMapY, ctx->partyMapX, ctx->partyMapY);
}

int F0890c_ORCH_GetGroupVisibleDistance_Compat(
    struct GameWorld_Compat* world,
    const struct DM1GroupBehaviorContext_Compat* context,
    const struct DungeonGroup_Compat* group)
{
    return orch_f0200_get_distance_to_visible_party_compat(
        world, context, group);
}

struct OrchF0200ViewRouteContext_Compat {
    const struct GameWorld_Compat* world;
    int mapIndex;
};

static int orch_f0200_view_square_blocked_callback_compat(
    int mapX, int mapY, void* opaque)
{
    const struct OrchF0200ViewRouteContext_Compat* context =
        (const struct OrchF0200ViewRouteContext_Compat*)opaque;
    if (!context || !context->world) return 1;
    return orch_f0199_square_blocks_view_compat(
        context->world, context->mapIndex, mapX, mapY);
}

int F0890f_ORCH_GetActiveGroupVisibleDistance_Compat(
    struct GameWorld_Compat* world,
    const struct DM1GroupBehaviorContext_Compat* context,
    const struct DM1ActiveGroup_Compat* activeGroup)
{
    struct DM1GroupBehaviorContext_Compat sourceContext;
    struct OrchF0200ViewRouteContext_Compat routeContext;
    struct DungeonViewLight_Compat dungeonLight;
    int distance = 0;

    if (!world || !context || !activeGroup || !world->dungeon ||
        context->currentMapIndex != context->partyMapIndex) {
        return 0;
    }
    sourceContext = *context;
    routeContext.world = world;
    routeContext.mapIndex = context->currentMapIndex;
    sourceContext.isViewSquareBlocked =
        orch_f0200_view_square_blocked_callback_compat;
    sourceContext.viewBlockerContext = &routeContext;
    sourceContext.partyInvisibilityEventCount =
        world->magic.event71CountInvisibility;
    if (!(sourceContext.creatureInfo.attributes & DM1_ATTR_NIGHT_VISION)) {
        memset(&dungeonLight, 0, sizeof(dungeonLight));
        if (F0890b_ORCH_ComputeDungeonViewLight_Compat(world, &dungeonLight)) {
            sourceContext.dungeonViewPaletteIndex = dungeonLight.paletteIndex;
        }
    }
    if (!F0818a_DM1_GROUP_GetDistanceToVisiblePartyWithRoute_Compat(
            &sourceContext, activeGroup, -1, &world->masterRng, &distance)) {
        return 0;
    }
    return distance;
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

static int orch_f0245_f0248_door_source_bound_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonDoor_Compat* door;
    const unsigned char* raw;
    unsigned short bits;
    int doorIndex = -1;

    if (!world || !world->things || !world->things->loaded ||
        !orch_cmd_attack_find_door_on_square_compat(
            world, mapIndex, mapX, mapY, &doorIndex) ||
        doorIndex < 0 || doorIndex >= world->things->doorCount ||
        doorIndex >= world->things->thingCounts[THING_TYPE_DOOR]) {
        return 0;
    }
    raw = dm1_v1_dungeon_get_thing_data_pc34(
        world->things, orch_make_thing_ref_compat(THING_TYPE_DOOR, doorIndex));
    if (!raw) return 0;
    door = &world->things->doors[doorIndex];
    bits = (unsigned short)((unsigned short)raw[2] |
                            ((unsigned short)raw[3] << 8));
    return (unsigned short)((unsigned short)raw[0] |
                            ((unsigned short)raw[1] << 8)) == door->next &&
           (bits & 1u) == door->type &&
           ((bits >> 1) & 0x0fu) == door->ornamentOrdinal &&
           ((bits >> 5) & 1u) == door->vertical &&
           ((bits >> 6) & 1u) == door->button &&
           ((bits >> 7) & 1u) == door->magicDestructible &&
           ((bits >> 8) & 1u) == door->meleeDestructible;
}

static int orch_f0249_move_group_first_square_thing_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    struct TickResult_Compat* result);

/* ReDMCSB TIMELINE.C F0242/F0244/F0245/F0248/F0250/F0251 dispatches C05..C10
 * square effects after F0261 extracts the event.  M10 represents that
 * original event family as TIMELINE_EVENT_SQUARE_STATE: aux0 is the
 * original C05..C10 type and aux1 is C00_SET/C01_CLEAR/C02_TOGGLE. */
static int orch_dispatch_square_state_event_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result)
{
    const struct DungeonMapDesc_Compat* map;
    struct DungeonMapTiles_Compat* tiles;
    unsigned char* square;
    int index;
    int effect;

    if (!world || !world->dungeon || !ev || !world->dungeon->maps ||
        !world->dungeon->tiles || ev->mapIndex < 0 ||
        ev->mapIndex >= (int)world->dungeon->header.mapCount) return 0;
    map = &world->dungeon->maps[ev->mapIndex];
    if (ev->mapX < 0 || ev->mapX >= (int)map->width ||
        ev->mapY < 0 || ev->mapY >= (int)map->height) return 0;
    tiles = &world->dungeon->tiles[ev->mapIndex];
    if (!tiles->squareData) return 0;
    index = ev->mapX * (int)map->height + ev->mapY;
    square = &tiles->squareData[index];
    effect = ev->aux1;
    if (effect < DOOR_EFFECT_SET || effect > DOOR_EFFECT_TOGGLE) return 0;

    switch (ev->aux0) {
    case DM1_EVENT_CORRIDOR:
        return orch_dispatch_corridor_event_f0245_compat(world, ev, result);
    case DM1_EVENT_WALL:
        return orch_dispatch_wall_event_f0248_compat(world, ev, result);
    case DM1_EVENT_DOOR: {
        int resolvedEffect = -1;
        struct TimelineEvent_Compat animation;
        /* F0244 only resolves the requested effect and requeues the same
         * record as C01; F0241 owns the actual state transition. */
        if (!orch_f0245_f0248_door_source_bound_compat(
                world, ev->mapIndex, ev->mapX, ev->mapY) ||
            !F0714_DOOR_ResolveAnimationEffect_Compat(
                world->dungeon, ev->mapIndex, ev->mapX, ev->mapY,
                effect, &resolvedEffect, NULL) ||
            !F0713_DOOR_BuildAnimationEvent_Compat(
                ev->mapIndex, ev->mapX, ev->mapY, resolvedEffect,
                ev->fireAtTick, &animation)) return 0;
        return F0721_TIMELINE_Schedule_Compat(&world->timeline, &animation);
    }
    case DM1_EVENT_FAKEWALL:
        /* ReDMCSB TIMELINE.C F0242:820-870 defers a CLEAR while either the
         * party or a material group occupies the fakewall square. It does
         * not relocate or mutate that group; the same C07 retries next tick. */
        if (((*square & DUNGEON_SQUARE_MASK_TYPE) >> 5) !=
            DUNGEON_ELEMENT_FAKEWALL) {
            return 0;
        }
        if (effect == DOOR_EFFECT_TOGGLE) effect = (*square & 0x04) ?
            DOOR_EFFECT_CLEAR : DOOR_EFFECT_SET;
        if (effect == DOOR_EFFECT_CLEAR && world->party.mapIndex == ev->mapIndex &&
            world->party.mapX == ev->mapX && world->party.mapY == ev->mapY) {
            struct TimelineEvent_Compat retry = *ev;
            retry.fireAtTick = world->gameTick + 1u;
            return F0721_TIMELINE_Schedule_Compat(&world->timeline, &retry);
        }
        if (effect == DOOR_EFFECT_CLEAR &&
            orch_find_material_group_on_square_compat(
                world->dungeon, world->things, ev->mapIndex, ev->mapX,
                ev->mapY, NULL, NULL)) {
            struct TimelineEvent_Compat retry = *ev;
            retry.fireAtTick = world->gameTick + 1u;
            return F0721_TIMELINE_Schedule_Compat(&world->timeline, &retry);
        }
        if (effect == DOOR_EFFECT_SET) *square |= 0x04u;
        else *square &= (unsigned char)~0x04u;
        return 1;
    case DM1_EVENT_TELEPORTER:
    case DM1_EVENT_PIT:
        /* ReDMCSB TIMELINE.C F0250/F0251 opens the square before F0249
         * re-submits party and its resident Things to F0267 at the same
         * coordinates. The group branch retains its dedicated active-group
         * F0267 owner. */
        if (effect == DOOR_EFFECT_TOGGLE) effect = (*square & 0x08) ?
            DOOR_EFFECT_CLEAR : DOOR_EFFECT_SET;
        if (effect == DOOR_EFFECT_SET) {
            int championIndex;

            *square |= 0x08u;
            /* ReDMCSB TIMELINE.C F0249:1382-1385 re-enters F0267 with
             * THING_PARTY before it walks the group/object chain. Reuse the
             * existing party F0267 environment resolver so opening a square
             * below the party immediately applies its original destination,
             * rotation, and pit damage. */
            if (world->party.mapIndex == ev->mapIndex &&
                world->party.mapX == ev->mapX &&
                world->party.mapY == ev->mapY) {
                struct PostMoveResolution_Compat postMove;

                memset(&postMove, 0, sizeof(postMove));
                if (F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat(
                        world->dungeon, world->things, &world->party,
                        world->gameTick, &postMove)) {
                    world->party.mapIndex = postMove.finalMapIndex;
                    world->party.mapX = postMove.finalMapX;
                    world->party.mapY = postMove.finalMapY;
                    (void)F0284_CHAMPION_SetPartyDirection_Compat(
                        &world->party, postMove.finalDirection);
                    world->partyMapIndex = postMove.finalMapIndex;
                    for (championIndex = 0;
                         championIndex < CHAMPION_MAX_PARTY;
                         ++championIndex) {
                        int damage = postMove.championFallDamage[championIndex];
                        if (damage > 0 &&
                            world->party.champions[championIndex].present &&
                            world->party.champions[championIndex].hp.current > 0) {
                            int health = world->party.champions[championIndex].hp.current -
                                         damage;
                            world->party.champions[championIndex].hp.current =
                                (int16_t)(health > 0 ? health : 0);
                        }
                    }
                }
            }
            /* ReDMCSB TIMELINE.C F0249 moves C04 before it snapshots the
             * ordinary C05..C15 list.  A source-square group move must keep
             * F0266's projectile-impact preflight; the C60/C61 insertion
             * owner handles only the later blocked-destination retry. */
            if (!orch_f0249_move_group_first_square_thing_compat(
                    world, ev->mapIndex, ev->mapX, ev->mapY, result)) {
                return 0;
            }
            return orch_f0249_move_non_group_square_things_compat(
                world, ev->mapIndex, ev->mapX, ev->mapY);
        }
        *square &= (unsigned char)~0x08u;
        return 1;
    default:
        return 0;
    }
}

/* ReDMCSB TIMELINE.C F0245:920-1006 walks every corridor Thing in source
 * list order. TextStrings are not cell-filtered here, unlike F0248 wall
 * text. Each C006 is consumed immediately through the established F0185
 * runtime materializer, with aux4 carrying an explicit 1-based sensor index
 * so a later C006 cannot accidentally reuse the first one on the square. */
static int orch_dispatch_corridor_event_f0245_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result)
{
    int squareIndex;
    unsigned short thing;
    int safety = 0;
    int applied = 0;

    if (!world || !world->dungeon || !world->things || !ev || !result ||
        !world->things->loaded || !world->things->squareFirstThings ||
        ev->aux1 < DM1_EFFECT_SET || ev->aux1 > DM1_EFFECT_TOGGLE ||
        orch_f0248_target_square_type_compat(
            world, ev->mapIndex, ev->mapX, ev->mapY) != DM1_SQUARE_CORRIDOR) {
        return 0;
    }
    squareIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, ev->mapIndex, ev->mapX, ev->mapY);
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) {
        return 0;
    }

    thing = world->things->squareFirstThings[squareIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        int thingIndex = THING_GET_INDEX(thing);
        unsigned short next = orch_next_thing_compat(world->things, thing);

        if (type == THING_TYPE_TEXTSTRING &&
            thingIndex >= 0 && thingIndex < world->things->textStringCount) {
            struct DungeonTextString_Compat* text =
                &world->things->textStrings[thingIndex];
            int wasVisible = text->visible != 0;
            text->visible = (unsigned char)(ev->aux1 == DM1_EFFECT_TOGGLE ?
                !text->visible : ev->aux1 == DM1_EFFECT_SET);
            /* ReDMCSB TIMELINE.C F0245:949-954 prints exactly when a
             * corridor TextString changes from hidden to visible on the
             * current party square.  M10 carries the original Thing index;
             * M11 decodes it through F0168's MESSAGE path. */
            if (!wasVisible && text->visible &&
                ev->mapIndex == world->party.mapIndex &&
                ev->mapX == world->party.mapX &&
                ev->mapY == world->party.mapY) {
                emit(result, EMIT_TEXT_MESSAGE, thingIndex, ev->mapIndex,
                     ev->mapX, ev->mapY);
            }
            applied = 1;
        } else if (type == THING_TYPE_SENSOR &&
                   thingIndex >= 0 && thingIndex < world->things->sensorCount &&
                   world->things->sensors[thingIndex].sensorType ==
                       DM1_SENSOR_FLOOR_GROUP_GENERATOR) {
            struct TimelineEvent_Compat generatorEvent = *ev;

            generatorEvent.kind = TIMELINE_EVENT_GROUP_GENERATOR;
            generatorEvent.aux0 = GENERATOR_EVENT_AUX0_TRIGGER;
            generatorEvent.aux4 = thingIndex + 1;
            applied |= orch_handle_group_generator_trigger_runtime_compat(
                world, &generatorEvent, result);
        }
        thing = next;
    }
    return applied;
}

static int orch_f0248_target_square_type_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat* map;
    const struct DungeonMapTiles_Compat* tiles;
    int squareIndex;

    if (!world || !world->dungeon || !world->dungeon->maps ||
        !world->dungeon->tiles || mapIndex < 0 ||
        mapIndex >= (int)world->dungeon->header.mapCount) {
        return -1;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= (int)map->width ||
        mapY < 0 || mapY >= (int)map->height) {
        return -1;
    }
    tiles = &world->dungeon->tiles[mapIndex];
    if (!tiles->squareData) return -1;
    squareIndex = mapX * (int)map->height + mapY;
    return (tiles->squareData[squareIndex] & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

/* ReDMCSB MOVESENS.C F0271: the last local rotation effect from a
 * processed sensor batch moves the first matching sensor behind the last
 * matching sensor in the contiguous sensor run. */
static int orch_f0271_rotate_sensor_chain_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    int cell,
    int effect)
{
    int squareIndex;
    unsigned short thing;
    unsigned short previous = THING_NONE;
    unsigned short first = THING_NONE;
    unsigned short firstPrevious = THING_NONE;
    unsigned short last = THING_NONE;
    unsigned short nextFirst;
    unsigned short nextLast;
    int safety = 0;

    if (!world || !world->dungeon || !world->things ||
        !world->things->squareFirstThings ||
        (effect != DM1_EFFECT_CLEAR && effect != DM1_EFFECT_TOGGLE)) {
        return 0;
    }
    squareIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) {
        return 0;
    }

    thing = world->things->squareFirstThings[squareIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        unsigned short next = orch_next_thing_compat(world->things, thing);

        if (first == THING_NONE) {
            if (type == THING_TYPE_SENSOR &&
                (cell < 0 || THING_GET_CELL(thing) == (unsigned int)(cell & 3))) {
                first = thing;
                firstPrevious = previous;
                last = thing;
            }
        } else {
            /* F0271 only extends the candidate run while Things remain
             * sensors; a later group/object is not part of the rotation. */
            if (type != THING_TYPE_SENSOR) break;
            if (cell < 0 || THING_GET_CELL(thing) == (unsigned int)(cell & 3)) {
                last = thing;
            }
        }
        previous = thing;
        thing = next;
    }
    if (first == THING_NONE || first == last) return 0;

    nextFirst = orch_next_thing_compat(world->things, first);
    nextLast = orch_next_thing_compat(world->things, last);
    if (firstPrevious == THING_NONE) {
        world->things->squareFirstThings[squareIndex] = nextFirst;
    } else {
        orch_set_thing_next_compat(world->things, firstPrevious, nextFirst);
        orch_write_raw_next_compat(world->things, firstPrevious);
    }
    orch_set_thing_next_compat(world->things, first, nextLast);
    orch_set_thing_next_compat(world->things, last, first);
    orch_write_raw_next_compat(world->things, first);
    orch_write_raw_next_compat(world->things, last);
    return 1;
}

static int orch_f0248_schedule_remote_effect_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* source,
    const struct SensorTriggerResult_Compat* trigger)
{
    struct TimelineEvent_Compat event;

    if (!world || !source || !trigger || !trigger->triggered ||
        trigger->isLocal ||
        (trigger->targetEventType != DM1_EVENT_WALL &&
         (trigger->targetEventType < DM1_EVENT_FAKEWALL ||
          trigger->targetEventType > DM1_EVENT_DOOR))) {
        return 0;
    }
    /* F0268 queues the resolved square event.  Keep M10's existing
     * zero-delay convention: it becomes observable on the next tick. */
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_SQUARE_STATE;
    event.fireAtTick = world->gameTick +
        (uint32_t)(trigger->delayTicks > 0 ? trigger->delayTicks : 1);
    event.mapIndex = source->mapIndex;
    event.mapX = trigger->targetMapX;
    event.mapY = trigger->targetMapY;
    event.cell = trigger->targetCell;
    event.aux0 = trigger->targetEventType;
    event.aux1 = trigger->resolvedEffect;
    return F0721_TIMELINE_Schedule_Compat(&world->timeline, &event);
}

/* ReDMCSB TIMELINE.C F0247:1033-1133 creates C008/C010 launcher
 * projectiles from a Fontanel explosion Thing.  These two launcher forms
 * need no object allocation or linked-list transfer, so M10 can consume
 * them directly without guessing at C007/C009/C014/C015 ownership. */
static int orch_f0248_explosion_launcher_subtype_compat(
    unsigned short associatedThing)
{
    unsigned int explosionType;

    if (associatedThing < DM1_THING_FIRST_EXPLOSION) {
        return PROJECTILE_SUBTYPE_FIREBALL;
    }
    explosionType = (unsigned int)(associatedThing - DM1_THING_FIRST_EXPLOSION);
    switch (explosionType) {
    case C000_EXPLOSION_FIREBALL:          return PROJECTILE_SUBTYPE_FIREBALL;
    case C001_EXPLOSION_SLIME:             return PROJECTILE_SUBTYPE_SLIME;
    case C002_EXPLOSION_LIGHTNING_BOLT:    return PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
    case C003_EXPLOSION_HARM_NON_MATERIAL: return PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
    case C004_EXPLOSION_OPEN_DOOR:         return PROJECTILE_SUBTYPE_OPEN_DOOR;
    case C007_EXPLOSION_POISON_CLOUD:      return PROJECTILE_SUBTYPE_POISON_CLOUD;
    default:                               return PROJECTILE_SUBTYPE_FIREBALL;
    }
}

static int orch_f0248_explosion_launcher_attack_type_compat(int subtype)
{
    switch (subtype) {
    case PROJECTILE_SUBTYPE_FIREBALL:
        return COMBAT_ATTACK_FIRE;
    case PROJECTILE_SUBTYPE_LIGHTNING_BOLT:
        return COMBAT_ATTACK_LIGHTNING;
    case PROJECTILE_SUBTYPE_HARM_NON_MATERIAL:
    case PROJECTILE_SUBTYPE_OPEN_DOOR:
        return COMBAT_ATTACK_MAGIC;
    default:
        return COMBAT_ATTACK_NORMAL;
    }
}

/* F0247 reaches F0212 from the wall-launcher families.  In a loaded PC34
 * world the C14 Thing and its C48/C49 owner are one transaction: publishing
 * a host projectile first would leave a synthetic, ownerless runtime entry
 * when the original pool or timeline cannot accept it. */
static int orch_f0248_publish_launcher_projectile_f0212_compat(
    struct GameWorld_Compat* world,
    const struct ProjectileCreateInput_Compat* input,
    int* outSlot)
{
    DM1_C14PoolReservationPc34 c14;
    struct ProjectileList_Compat projectilesBefore;
    struct TimelineQueue_Compat timelineBefore;
    struct TimelineEvent_Compat firstMove;
    int slot = -1;

    if (outSlot) *outSlot = -1;
    if (!world || !input || !world->dungeon || !world->things ||
        !world->things->loaded ||
        !dm1_v1_c14_pool_reserve_pc34(world->things, &c14)) {
        return 0;
    }
    projectilesBefore = world->projectiles;
    timelineBefore = world->timeline;
    memset(&firstMove, 0, sizeof(firstMove));
    if (!F0810_PROJECTILE_Create_Compat(
            input, &world->projectiles, &slot, &firstMove) ||
        THING_GET_INDEX(c14.thing) != slot ||
        !dm1_v1_c14_pool_initialize_and_link_pc34(
            &c14, world->dungeon, (unsigned short)input->associatedThing,
            input->kineticEnergy, input->attack, 0xffffu, input->cell,
            input->mapIndex, input->mapX, input->mapY) ||
        !orch_schedule_projectile_move_f0219_compat(
            world, slot, &firstMove)) {
        world->projectiles = projectilesBefore;
        world->timeline = timelineBefore;
        if (c14.active) (void)dm1_v1_c14_pool_rollback_pc34(&c14);
        return 0;
    }
    if (outSlot) *outSlot = slot;
    return 1;
}

static int orch_f0248_consume_explosion_launcher_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct DungeonSensor_Compat* sensor,
    int sensorCell)
{
    struct ProjectileLauncherContext_Compat context;
    struct ProjectileLauncherResult_Compat launcher;
    int launchIndex;
    int applied = 0;

    if (!world || !ev || !sensor ||
        (sensor->sensorType != DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION &&
         sensor->sensorType != DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION)) {
        return 0;
    }
    /* F0248 reaches F0247 only after the wall-event cell selects this
     * sensor.  In particular, a C008 on another cell must not consume
     * M005_RANDOM(2). */
    if ((sensorCell & 3) != (ev->cell & 3)) return 0;
    memset(&context, 0, sizeof(context));
    context.newObjectThings[0] = THING_NONE;
    context.newObjectThings[1] = THING_NONE;
    /* F0247 calls M005_RANDOM(2) only for C008 after it has selected the
     * explosion Thing; C010 must not advance the source RNG. */
    if (sensor->sensorType == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION) {
        context.randomBit = F0732_COMBAT_RngRandom_Compat(&world->masterRng, 2);
    }
    memset(&launcher, 0, sizeof(launcher));
    if (!F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
            sensor, sensorCell, ev->mapX, ev->mapY, ev->cell,
            &context, &launcher) || !launcher.triggered) {
        return 0;
    }
    if (launcher.sensorDisabled) {
        sensor->sensorType = DM1_SENSOR_DISABLED;
        applied = 1;
    }
    for (launchIndex = 0; launchIndex < launcher.launchCount; ++launchIndex) {
        const struct ProjectileLauncherLaunch_Compat* launch =
            &launcher.launches[launchIndex];
        struct ProjectileCreateInput_Compat input;
        int slot = -1;
        int subtype;

        if (!launch->valid) continue;
        subtype = orch_f0248_explosion_launcher_subtype_compat(
            launch->associatedThing);
        memset(&input, 0, sizeof(input));
        input.category = PROJECTILE_CATEGORY_MAGICAL;
        input.subtype = subtype;
        input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
        input.ownerIndex = -1;
        input.mapIndex = ev->mapIndex;
        input.mapX = launch->mapX;
        input.mapY = launch->mapY;
        input.cell = launch->cell;
        input.direction = launch->direction;
        input.kineticEnergy = launch->kineticEnergy;
        input.attack = launch->attack;
        input.launcherStrength = launch->attack;
        input.stepEnergy = launch->stepEnergy;
        input.currentTick = (int)world->gameTick;
        input.poisonAttack = subtype == PROJECTILE_SUBTYPE_POISON_CLOUD
            ? launch->attack : 0;
        input.attackTypeCode =
            orch_f0248_explosion_launcher_attack_type_compat(subtype);
        input.associatedThing = (int)launch->associatedThing;
        input.firstMoveGraceFlag = 0;
        if (orch_f0248_publish_launcher_projectile_f0212_compat(
                world, &input, &slot)) {
            applied = 1;
        }
    }
    return applied;
}

/* ReDMCSB TIMELINE.C F0247:1066-1100 walks the live square chain for
 * C014/C015, selects ordinary Things on the event/next cell, and unlinks
 * each selected Thing through F0164 before F0212 turns it into a kinetic
 * projectile.  Keep that ownership transfer inside M10. */
static int orch_f0248_collect_square_launcher_things_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    struct ProjectileLauncherSquareThing_Compat* outThings,
    int capacity)
{
    int squareIndex;
    unsigned short thing;
    int count = 0;
    int safety = 0;

    if (!world || !world->dungeon || !world->things || !outThings ||
        capacity <= 0 || !world->things->squareFirstThings) {
        return 0;
    }
    squareIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) {
        return 0;
    }
    thing = world->things->squareFirstThings[squareIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST &&
           count < capacity && safety++ < 64) {
        outThings[count].thing = thing;
        outThings[count].cell = (int)THING_GET_CELL(thing);
        outThings[count].thingType = (int)THING_GET_TYPE(thing);
        ++count;
        thing = orch_next_thing_compat(world->things, thing);
    }
    return count;
}

static int orch_f0248_consume_square_object_launcher_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct DungeonSensor_Compat* sensor,
    int sensorCell)
{
    struct ProjectileLauncherSquareThing_Compat squareThings[64];
    struct ProjectileLauncherContext_Compat context;
    struct ProjectileLauncherResult_Compat launcher;
    int squareThingCount;
    int unlinkIndex;
    int launchIndex;
    int applied = 0;

    if (!world || !ev || !sensor ||
        (sensor->sensorType != DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ &&
         sensor->sensorType != DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_SQUARE_OBJ) ||
        (sensorCell & 3) != (ev->cell & 3)) {
        return 0;
    }
    squareThingCount = orch_f0248_collect_square_launcher_things_compat(
        world, ev->mapIndex, ev->mapX, ev->mapY, squareThings,
        (int)(sizeof(squareThings) / sizeof(squareThings[0])));
    if (squareThingCount <= 0) return 0;
    memset(&context, 0, sizeof(context));
    context.newObjectThings[0] = THING_NONE;
    context.newObjectThings[1] = THING_NONE;
    context.squareThings = squareThings;
    context.squareThingCount = squareThingCount;
    memset(&launcher, 0, sizeof(launcher));
    /* First evaluate without RNG so an empty C014/C015 selection does not
     * advance M005_RANDOM(2). A one-object C015 collapses to the source's
     * single-launch path and therefore consumes exactly one random bit. */
    if (!F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
            sensor, sensorCell, ev->mapX, ev->mapY, ev->cell,
            &context, &launcher) || !launcher.triggered ||
        launcher.launchCount <= 0) {
        return 0;
    }
    if (launcher.launchSingleProjectile) {
        context.randomBit = F0732_COMBAT_RngRandom_Compat(&world->masterRng, 2);
        if (!F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
                sensor, sensorCell, ev->mapX, ev->mapY, ev->cell,
                &context, &launcher) || !launcher.triggered) {
            return 0;
        }
    }
    if (launcher.sensorDisabled) {
        sensor->sensorType = DM1_SENSOR_DISABLED;
        applied = 1;
    }
    for (unlinkIndex = 0; unlinkIndex < launcher.unlinkCount; ++unlinkIndex) {
        applied |= orch_unlink_thing_from_square_compat(
            world, ev->mapIndex, ev->mapX, ev->mapY,
            launcher.unlinkThings[unlinkIndex]);
    }
    for (launchIndex = 0; launchIndex < launcher.launchCount; ++launchIndex) {
        const struct ProjectileLauncherLaunch_Compat* launch =
            &launcher.launches[launchIndex];
        struct ProjectileCreateInput_Compat input;
        int slot = -1;

        if (!launch->valid) continue;
        memset(&input, 0, sizeof(input));
        input.category = PROJECTILE_CATEGORY_KINETIC;
        input.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
        input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
        input.ownerIndex = -1;
        input.mapIndex = ev->mapIndex;
        input.mapX = launch->mapX;
        input.mapY = launch->mapY;
        input.cell = launch->cell;
        input.direction = launch->direction;
        input.kineticEnergy = launch->kineticEnergy;
        input.attack = launch->attack;
        input.launcherStrength = launch->attack;
        input.stepEnergy = launch->stepEnergy;
        input.currentTick = (int)world->gameTick;
        input.attackTypeCode = COMBAT_ATTACK_BLUNT;
        input.associatedThing = (int)launch->associatedThing;
        if (orch_f0248_publish_launcher_projectile_f0212_compat(
                world, &input, &slot)) {
            applied = 1;
        }
    }
    return applied;
}

/* ReDMCSB DUNGEON.C F0167:2140-2200 is the only object factory used by
 * C007/C009.  Keep its exact launcher-icon subset here rather than inventing
 * an item from a generic icon or allocating a new host-side record. */
static int orch_f0248_launcher_icon_to_object_compat(
    int iconIndex,
    int* outThingType,
    int* outObjectType)
{
    int thingType = THING_TYPE_WEAPON;
    int objectType;

    if (iconIndex >= 4 && iconIndex <= 7) iconIndex = 4;
    switch (iconIndex) {
    case 4:   objectType = 2; break;   /* C004 torch -> C02 weapon torch */
    case 32:  objectType = 8; break;   /* C032 dagger */
    case 51:  objectType = 27; break;  /* C051 arrow */
    case 52:  objectType = 28; break;  /* C052 slayer */
    case 54:  objectType = 30; break;  /* C054 rock */
    case 55:  objectType = 31; break;  /* C055 poison dart */
    case 56:  objectType = 32; break;  /* C056 throwing star */
    case 128:
        objectType = 25;               /* C128 boulder */
        thingType = THING_TYPE_JUNK;
        break;
    default:
        return 0;
    }
    if (outThingType) *outThingType = thingType;
    if (outObjectType) *outObjectType = objectType;
    return 1;
}

static int orch_f0248_allocate_new_launcher_object_compat(
    struct GameWorld_Compat* world,
    int iconIndex,
    unsigned short* outThing)
{
    int thingType;
    int objectType;
    int i;

    if (outThing) *outThing = THING_NONE;
    if (!world || !world->things || !outThing ||
        !orch_f0248_launcher_icon_to_object_compat(
            iconIndex, &thingType, &objectType)) {
        return 0;
    }
    if (thingType == THING_TYPE_WEAPON) {
        if (!world->things->weapons) return 0;
        for (i = 0; i < world->things->weaponCount; ++i) {
            struct DungeonWeapon_Compat* weapon = &world->things->weapons[i];
            if (weapon->next != THING_NONE) continue;
            /* F0166 clears the complete source record before F0167 assigns
             * Type; an unlit generator torch consequently has no charges. */
            memset(weapon, 0, sizeof(*weapon));
            weapon->next = THING_ENDOFLIST;
            weapon->type = (unsigned char)objectType;
            orch_write_raw_weapon_compat(world->things, i);
            *outThing = orch_make_thing_ref_compat(THING_TYPE_WEAPON, i);
            return 1;
        }
    } else if (thingType == THING_TYPE_JUNK) {
        if (!world->things->junks) return 0;
        for (i = 0; i < world->things->junkCount; ++i) {
            struct DungeonJunk_Compat* junk = &world->things->junks[i];
            if (junk->next != THING_NONE) continue;
            memset(junk, 0, sizeof(*junk));
            junk->next = THING_ENDOFLIST;
            junk->type = (unsigned char)objectType;
            orch_write_raw_junk_compat(world->things, i);
            *outThing = orch_make_thing_ref_compat(THING_TYPE_JUNK, i);
            return 1;
        }
    }
    return 0;
}

static int orch_f0248_consume_new_object_launcher_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct DungeonSensor_Compat* sensor,
    int sensorCell)
{
    struct ProjectileLauncherContext_Compat context;
    struct ProjectileLauncherResult_Compat launcher;
    int launchIndex;
    int applied = 0;

    if (!world || !ev || !sensor ||
        (sensor->sensorType != DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ &&
         sensor->sensorType != DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ) ||
        (sensorCell & 3) != (ev->cell & 3)) {
        return 0;
    }
    memset(&context, 0, sizeof(context));
    context.newObjectThings[0] = THING_NONE;
    context.newObjectThings[1] = THING_NONE;
    if (!orch_f0248_allocate_new_launcher_object_compat(
            world, sensor->sensorData, &context.newObjectThings[0])) {
        /* F0248 disables an once-only source sensor after F0247 returns,
         * including F0167 allocation failure. */
        if (sensor->onceOnly) {
            sensor->sensorType = DM1_SENSOR_DISABLED;
            return 1;
        }
        return 0;
    }
    if (sensor->sensorType == DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ) {
        (void)orch_f0248_allocate_new_launcher_object_compat(
            world, sensor->sensorData, &context.newObjectThings[1]);
    }
    memset(&launcher, 0, sizeof(launcher));
    if (!F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
            sensor, sensorCell, ev->mapX, ev->mapY, ev->cell,
            &context, &launcher) || !launcher.triggered) {
        return 0;
    }
    if (launcher.launchSingleProjectile) {
        context.randomBit = F0732_COMBAT_RngRandom_Compat(&world->masterRng, 2);
        if (!F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
                sensor, sensorCell, ev->mapX, ev->mapY, ev->cell,
                &context, &launcher) || !launcher.triggered) {
            return 0;
        }
    }
    if (launcher.sensorDisabled) {
        sensor->sensorType = DM1_SENSOR_DISABLED;
        applied = 1;
    }
    for (launchIndex = 0; launchIndex < launcher.launchCount; ++launchIndex) {
        const struct ProjectileLauncherLaunch_Compat* launch =
            &launcher.launches[launchIndex];
        struct ProjectileCreateInput_Compat input;
        int slot = -1;

        if (!launch->valid) continue;
        memset(&input, 0, sizeof(input));
        input.category = PROJECTILE_CATEGORY_KINETIC;
        input.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
        input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
        input.ownerIndex = -1;
        input.mapIndex = ev->mapIndex;
        input.mapX = launch->mapX;
        input.mapY = launch->mapY;
        input.cell = launch->cell;
        input.direction = launch->direction;
        input.kineticEnergy = launch->kineticEnergy;
        input.attack = launch->attack;
        input.launcherStrength = launch->attack;
        input.stepEnergy = launch->stepEnergy;
        input.currentTick = (int)world->gameTick;
        input.attackTypeCode = COMBAT_ATTACK_BLUNT;
        input.associatedThing = (int)launch->associatedThing;
        if (orch_f0248_publish_launcher_projectile_f0212_compat(
                world, &input, &slot)) {
            applied = 1;
        }
    }
    return applied;
}

/* ReDMCSB MOVESENS.C F0269/F0270:1043-1097 awards C10 local-effect
 * Steal XP through F0304. Wall events always carry a real cell, so F0270
 * selects G0411_i_LeaderIndex rather than splitting the award across party
 * members. Firestaff's lifecycle state is the F0304 owner; ChampionState
 * persists only the four base skills, so mirror Ninja while keeping hidden
 * Steal experience in the lifecycle's 20-skill source state. */
static int orch_f0248_award_steal_skill_xp_compat(
    struct GameWorld_Compat* world,
    int eventCell,
    struct TickResult_Compat* result)
{
    int championIndex;
    struct ChampionState_Compat* champion;
    struct ChampionLifecycleState_Compat* lifecycleChampion;

    if (!world || (eventCell & 3) != eventCell) return 0;
    championIndex = world->party.activeChampionIndex;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champion = &world->party.champions[championIndex];
    lifecycleChampion = &world->lifecycle.champions[championIndex];
    if (!champion->present) return 0;
    (void)F0849_LIFECYCLE_AddSkillExperience_Compat(
        lifecycleChampion, DM1_SKILL_IDX_STEAL, 300,
        orch_cmd_attack_map_difficulty_compat(world), world->gameTick,
        world->lifecycle.lastCreatureAttackTime, NULL, NULL);
    champion->skillExperience[DM1_SKILL_IDX_NINJA] =
        (unsigned long)lifecycleChampion->skills20[DM1_SKILL_IDX_NINJA].experience;
    emit(result, EMIT_XP_AWARD, championIndex, DM1_SKILL_IDX_STEAL, 300, 1);
    return 1;
}

/* ReDMCSB COMMAND.C F0380:2308-2312 forwards an action-area click only when
 * the command layer has a valid party-panel owner; MENU.C F0407 then reaches
 * F0330 with that selected Party.Champion. Imported/corrupt state can retain
 * a typed C11 for a slot no longer in Party.ChampionCount; compact only that
 * stale owner before a later invalid input leaves it pending until its due
 * tick. F0407 can schedule C04 and C11 from the same action, but C04's
 * SOUND.C C20 layout has no champion owner and is deliberately outside this
 * cleanup. */
static void orch_cleanup_invalid_action_enable_receipts_compat(
    struct GameWorld_Compat* world,
    int championIndex)
{
    int partyChampionCount;
    int newEventCount;
    int readIndex;
    int writeIndex = 0;

    if (!world || championIndex < 0) return;
    partyChampionCount = world->party.championCount;
    if (partyChampionCount < 0) partyChampionCount = 0;
    if (partyChampionCount > CHAMPION_MAX_PARTY) {
        partyChampionCount = CHAMPION_MAX_PARTY;
    }
    for (readIndex = 0; readIndex < world->timeline.count; ++readIndex) {
        const struct TimelineEvent_Compat* event =
            &world->timeline.events[readIndex];
        if (event->kind == TIMELINE_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux0 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux2 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux4 == championIndex &&
            (championIndex >= CHAMPION_MAX_PARTY ||
             championIndex >= partyChampionCount)) {
            continue;
        }
        world->timeline.events[writeIndex++] = *event;
    }
    newEventCount = writeIndex;
    while (writeIndex < world->timeline.count) {
        memset(&world->timeline.events[writeIndex++], 0,
               sizeof(world->timeline.events[0]));
    }
    world->timeline.count = newEventCount;
}

void DM1_V1_F0407_ClearRemovedChampionActionReceiptsPc34Compat(
    struct GameWorld_Compat* world,
    int championIndex)
{
    /* ReDMCSB REVIVE.C F0282 C162 removes the candidate from the party;
     * TIMELINE.C C11 later dispatches solely by its stored Priority.  The
     * original event has no generation field, so the host must discard it
     * while the removed slot is absent, before a different mirror can take
     * the same party ordinal. */
    orch_cleanup_invalid_action_enable_receipts_compat(world, championIndex);
}

/* ReDMCSB CHAMPION.C F0330:2233-2255 owns the single pending C11 event
 * per champion. A later disable replaces its prior event using the source
 * half-distance timing rule. MENU.C F0407 alone changes SlotOrdinal to
 * C01's ordinal after a successful throw; this producer must not invent a
 * refill target for every disabled action. */
int DM1_V1_F0330_ScheduleEnableChampionActionPc34Compat(
    struct GameWorld_Compat* world,
    int championIndex,
    int ticks)
{
    struct TimelineEvent_Compat event;
    uint32_t targetTick;
    int i;

    if (!world || championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY ||
        championIndex >= world->party.championCount || ticks <= 0) {
        orch_cleanup_invalid_action_enable_receipts_compat(world, championIndex);
        return 0;
    }
    targetTick = world->gameTick + (uint32_t)ticks;
    for (i = 0; i < world->timeline.count; ++i) {
        struct TimelineEvent_Compat* prior = &world->timeline.events[i];
        uint32_t currentTick;

        if (prior->kind != TIMELINE_EVENT_ENABLE_CHAMPION_ACTION ||
            prior->aux0 != DM1_EVENT_ENABLE_CHAMPION_ACTION ||
            prior->aux2 != DM1_EVENT_ENABLE_CHAMPION_ACTION ||
            prior->aux4 != championIndex) {
            continue;
        }
        currentTick = prior->fireAtTick;
        if (targetTick >= currentTick) {
            targetTick += (currentTick - world->gameTick) >> 1;
        } else {
            targetTick = currentTick + ((uint32_t)ticks >> 1);
        }
        memmove(prior, prior + 1,
                (size_t)(world->timeline.count - i - 1) * sizeof(*prior));
        --world->timeline.count;
        memset(&world->timeline.events[world->timeline.count], 0,
               sizeof(world->timeline.events[world->timeline.count]));
        break;
    }
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_ENABLE_CHAMPION_ACTION;
    event.fireAtTick = targetTick;
    event.mapIndex = world->party.mapIndex;
    event.mapX = world->party.mapX;
    event.mapY = world->party.mapY;
    /* F0330 writes EVENT.Priority plus B.SlotOrdinal == 0. A proven
     * MENU.C F0407 throw route may later set ordinal two on this C11. */
    event.aux0 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    event.aux1 = 0;
    event.aux2 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    event.aux4 = championIndex;
    return F0721_TIMELINE_Schedule_Compat(&world->timeline, &event);
}

int DM1_V1_F0407_MarkPendingThrowActionHandPc34Compat(
    struct GameWorld_Compat* world,
    int championIndex)
{
    int found = -1;
    int i;

    if (!world || championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY ||
        championIndex >= world->party.championCount) {
        orch_cleanup_invalid_action_enable_receipts_compat(world, championIndex);
        return 0;
    }
    /* ReDMCSB MENU.C F0407:1613-1617 reaches this only after F0328 has
     * accepted the action-hand throw. It writes the indexed F0330 event's
     * B.SlotOrdinal to M000_INDEX_TO_ORDINAL(C01_SLOT_ACTION_HAND) == 2.
     * Firestaff has no Champion.EnableActionEventIndex mirror, so accept
     * exactly one still-pending typed C11 owner and fail closed otherwise. */
    for (i = 0; i < world->timeline.count; ++i) {
        const struct TimelineEvent_Compat* event = &world->timeline.events[i];

        if (event->kind != TIMELINE_EVENT_ENABLE_CHAMPION_ACTION ||
            event->aux0 != DM1_EVENT_ENABLE_CHAMPION_ACTION ||
            event->aux2 != DM1_EVENT_ENABLE_CHAMPION_ACTION ||
            event->aux4 != championIndex || event->aux1 != 0 ||
            event->fireAtTick < world->gameTick) {
            continue;
        }
        if (found >= 0) return 0;
        found = i;
    }
    if (found < 0) return 0;
    world->timeline.events[found].aux1 = 2;
    return 1;
}

static void orch_f0330_schedule_action_disabled_emissions_compat(
    struct GameWorld_Compat* world,
    const struct TickResult_Compat* result)
{
    int i;

    if (!world || !result) return;
    for (i = 0; i < result->emissionCount; ++i) {
        const struct TickEmission_Compat* emission = &result->emissions[i];
        if (emission->kind != EMIT_ACTION_DISABLED) continue;
        (void)DM1_V1_F0330_ScheduleEnableChampionActionPc34Compat(
            world, emission->payload[0], emission->payload[1]);
    }
}

/* ReDMCSB TIMELINE.C F0248:1136-1350 walks the complete wall list in
 * order, changes only TextStrings on the event cell, evaluates C005/C006,
 * consumes C007/C009 F0167 materialized, C008/C010 explosion, and C014/C015
 * live-object launchers, then calls F0271 once after the batch. */
static int orch_dispatch_wall_event_f0248_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result)
{
    int squareIndex;
    unsigned short thing;
    int rotationEffect = DM1_EFFECT_NONE;
    int safety = 0;
    int applied = 0;

    if (!world || !world->dungeon || !world->things || !ev ||
        !world->things->loaded || !world->things->squareFirstThings ||
        ev->aux1 < DM1_EFFECT_SET || ev->aux1 > DM1_EFFECT_TOGGLE ||
        orch_f0248_target_square_type_compat(
            world, ev->mapIndex, ev->mapX, ev->mapY) != DM1_SQUARE_WALL) {
        return 0;
    }
    squareIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, ev->mapIndex, ev->mapX, ev->mapY);
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) {
        return 0;
    }

    thing = world->things->squareFirstThings[squareIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        int thingIndex = THING_GET_INDEX(thing);
        unsigned short next = orch_next_thing_compat(world->things, thing);

        if (type == THING_TYPE_TEXTSTRING &&
            thingIndex >= 0 && thingIndex < world->things->textStringCount &&
            THING_GET_CELL(thing) == (unsigned int)(ev->cell & 3)) {
            struct DungeonTextString_Compat* text =
                &world->things->textStrings[thingIndex];
            text->visible = (unsigned char)(ev->aux1 == DM1_EFFECT_TOGGLE ?
                !text->visible : ev->aux1 == DM1_EFFECT_SET);
            applied = 1;
        } else if (type == THING_TYPE_SENSOR &&
                   thingIndex >= 0 && thingIndex < world->things->sensorCount) {
            struct DungeonSensor_Compat* sensor = &world->things->sensors[thingIndex];
            struct SensorTriggerResult_Compat trigger;
            int targetSquareType = -1;
            int evaluated = 0;

            memset(&trigger, 0, sizeof(trigger));
            if (sensor->sensorType == DM1_SENSOR_WALL_COUNTDOWN) {
                /* F0248 passes the event cell, not the sensor Thing cell,
                 * through F0272 to F0270/F0271. */
                evaluated = F0729_SENSOR_EvaluateWallCountdownEvent_Compat(
                    sensor, ev->aux1, ev->mapX, ev->mapY, ev->cell, &trigger);
            } else if (sensor->sensorType == DM1_SENSOR_WALL_AND_OR_GATE) {
                targetSquareType = orch_f0248_target_square_type_compat(
                    world, ev->mapIndex, sensor->targetMapX, sensor->targetMapY);
                if (targetSquareType >= 0) {
                    evaluated = F0730_SENSOR_EvaluateWallAndOrGateEvent_Compat(
                        sensor, ev->cell, ev->aux1, targetSquareType,
                        ev->mapX, ev->mapY, &trigger);
                }
            } else if (sensor->sensorType ==
                           DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION ||
                       sensor->sensorType ==
                           DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION) {
                applied |= orch_f0248_consume_explosion_launcher_compat(
                    world, ev, sensor, THING_GET_CELL(thing));
            } else if (sensor->sensorType ==
                           DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ ||
                       sensor->sensorType ==
                           DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_SQUARE_OBJ) {
                applied |= orch_f0248_consume_square_object_launcher_compat(
                    world, ev, sensor, THING_GET_CELL(thing));
            } else if (sensor->sensorType ==
                           DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ ||
                       sensor->sensorType ==
                           DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ) {
                applied |= orch_f0248_consume_new_object_launcher_compat(
                    world, ev, sensor, THING_GET_CELL(thing));
            } else if (sensor->sensorType == DM1_SENSOR_WALL_END_GAME) {
                /* ReDMCSB TIMELINE.C F0248:1317-1339 does not cell-filter
                 * C018.  Its M10-visible state transition is immediate;
                 * the F0444/F0446 presentation sequence remains the M11
                 * consumer of world->gameWon. */
                if (F0731_SENSOR_EvaluateWallEndGameEvent_Compat(
                        sensor, THING_GET_CELL(thing), ev->aux1, ev->cell,
                        &trigger) && trigger.triggered &&
                    trigger.endGameGameWon) {
                    world->gameWon = 1;
                    applied = 1;
                }
            }
            if (evaluated) {
                if (trigger.sensorDataChanged) {
                    sensor->sensorData = (unsigned short)trigger.sensorDataAfter;
                    applied = 1;
                }
                if (trigger.triggered && trigger.sensorDisabled) {
                    sensor->sensorType = DM1_SENSOR_DISABLED;
                    applied = 1;
                }
                if (trigger.triggered && trigger.isLocal &&
                    trigger.localEffectValue == DM1_EFFECT_ADD_300XP_STEAL_SKILL) {
                    applied |= orch_f0248_award_steal_skill_xp_compat(
                        world, ev->cell, result);
                } else if (trigger.triggered && trigger.isLocal) {
                    /* F0270 stores the last non-XP local effect. */
                    rotationEffect = trigger.localEffectValue;
                }
                if (trigger.triggered && !trigger.isLocal) {
                    applied |= orch_f0248_schedule_remote_effect_compat(
                        world, ev, &trigger);
                }
            }
        }
        thing = next;
    }

    if (rotationEffect != DM1_EFFECT_NONE) {
        applied |= orch_f0271_rotate_sensor_chain_compat(
            world, ev->mapIndex, ev->mapX, ev->mapY, ev->cell,
            rotationEffect);
    }
    return applied;
}

static int orch_cmd_attack_f0407_closed_door_compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* input,
    const DM1_WeaponInfo* weaponInfo,
    int hasActionHandWeapon,
    int actionIndex,
    int targetDirection,
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
    DM1_ActionClosedDoorMeleeInputPc34 doorPlanIn;
    DM1_ActionClosedDoorMeleePlanPc34 doorPlan;
    DM1_ActionClosedDoorDestructionInputPc34 destructionIn;
    DM1_ActionClosedDoorDestructionPlanPc34 destructionPlan;

    (void)result;
    if (!world || !input || !weaponInfo || !world->dungeon ||
        !world->dungeon->tiles || !world->dungeon->maps) {
        return 0;
    }
    memset(&doorPlanIn, 0, sizeof(doorPlanIn));
    memset(&doorPlan, 0, sizeof(doorPlan));
    doorPlanIn.actionIndex = actionIndex;
    if (!dm1_v1_action_closed_door_melee_plan_f0407_pc34(
            &doorPlanIn, &doorPlan) ||
        !doorPlan.valid ||
        !doorPlan.isClosedDoorMeleeAction) {
        return 0;
    }

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
        (int)input->commandArg1 >= CHAMPION_MAX_PARTY ||
        (int)input->commandArg1 >= world->party.championCount) {
        return 1;
    }
    champion = &world->party.champions[(int)input->commandArg1];
    if (!champion->present || champion->hp.current == 0) {
        return 1;
    }

    doorPlanIn.observedWoodenThudSound = 1;
    (void)dm1_v1_action_closed_door_melee_plan_f0407_pc34(
        &doorPlanIn, &doorPlan);

    /* ReDMCSB MENU.C F0407 lines 1268-1275 handles closed-door melee
     * before F0402/F0231 creature melee.  DM1 owns the branch action/delay
     * plan; M10 keeps live door lookup, F0312 strength, and event scheduling. */
    {
        struct TimelineEvent_Compat thud;
        memset(&thud, 0, sizeof(thud));
        thud.kind = TIMELINE_EVENT_PLAY_SOUND;
        thud.fireAtTick = world->gameTick + 1u;
        thud.mapIndex = mapIndex;
        thud.mapX = mapX;
        thud.mapY = mapY;
        thud.aux0 = ORCH_SOUND_WOODEN_THUD_PC34;
        /* ReDMCSB SOUND.C F0064:1536-1543 schedules the delayed thud as
         * native C20 with B.Location, C.SoundIndex and Sound->Priority.
         * Keep the full receipt so M11 cannot confuse another sound-4
         * producer with F0407's closed-door branch. */
        thud.aux2 = DM1_EVENT_PLAY_SOUND;
        thud.aux4 = ORCH_SOUND_WOODEN_THUD_PRIORITY_PC34;
        (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &thud);
    }

    attack = orch_cmd_attack_f0312_strength_action_hand_compat(
        world, champion, (int)input->commandArg1, weaponInfo,
        hasActionHandWeapon);
    memset(&destructionIn, 0, sizeof(destructionIn));
    memset(&destructionPlan, 0, sizeof(destructionPlan));
    destructionIn.closedDoorState = 1;
    destructionIn.meleeDestructible = door->meleeDestructible;
    destructionIn.attack = attack;
    destructionIn.defense = orch_cmd_attack_door_defense_pc34_compat(world, door);
    destructionIn.destructionDelayTicks = doorPlan.destructionDelayTicks;
    destructionIn.currentTick = world->gameTick;
    destructionIn.mapIndex = mapIndex;
    destructionIn.mapX = mapX;
    destructionIn.mapY = mapY;
    if (dm1_v1_action_closed_door_destruction_plan_f0232_pc34(
            &destructionIn, &destructionPlan) &&
        destructionPlan.valid &&
        destructionPlan.shouldScheduleDestruction) {
        struct TimelineEvent_Compat destruction;
        memset(&destruction, 0, sizeof(destruction));
        destruction.kind = TIMELINE_EVENT_DOOR_DESTRUCTION;
        destruction.fireAtTick = destructionPlan.fireAtTick;
        destruction.mapIndex = destructionPlan.mapIndex;
        destruction.mapX = destructionPlan.mapX;
        destruction.mapY = destructionPlan.mapY;
        destruction.aux0 = destructionPlan.destroyedDoorState;
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

/* Materialize exactly the F0202 input record from the loaded DUNGEON square
 * and its C00/C04/C15 chain.  A missing list owner, malformed chain, or an
 * unsupported raw square leaves `available` clear; GROUP.C must not treat it
 * as a walkable corridor. */
static int orch_build_group_movement_facts_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    struct DM1GroupMovementFacts_Compat* outFacts)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char square;
    unsigned short thing;
    int squareType;
    int sftIndex;
    int safety = 0;

    if (!outFacts) return 0;
    memset(outFacts, 0, sizeof(*outFacts));
    if (!world || !world->dungeon || !world->things ||
        mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= (int)map->width ||
        mapY < 0 || mapY >= (int)map->height ||
        !orch_read_square_byte_compat(
            world->dungeon, mapIndex, mapX, mapY, &square)) {
        return 0;
    }
    squareType = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    if (squareType < DUNGEON_ELEMENT_WALL ||
        squareType >= DUNGEON_ELEMENT_COUNT) {
        return 0;
    }

    outFacts->available = 1;
    outFacts->inBounds = 1;
    outFacts->isWall = squareType == DUNGEON_ELEMENT_WALL;
    outFacts->isStairs = squareType == DUNGEON_ELEMENT_STAIRS;
    outFacts->isOpenPit = squareType == DUNGEON_ELEMENT_PIT &&
        (square & 0x08u) && !(square & 0x01u);
    outFacts->isImaginaryPit = squareType == DUNGEON_ELEMENT_PIT &&
        (square & 0x01u);
    outFacts->isFakeWall = squareType == DUNGEON_ELEMENT_FAKEWALL;
    outFacts->isOpenFakeWall = outFacts->isFakeWall && (square & 0x04u);
    outFacts->isImaginaryFakeWall = outFacts->isFakeWall &&
        (square & 0x01u);
    outFacts->doorBlocksCreature = squareType == DUNGEON_ELEMENT_DOOR &&
        !F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            world->dungeon, mapIndex, mapX, mapY,
            MOVEMENT_PASS_CTX_CREATURE);
    outFacts->occupiedByParty = world->party.mapIndex == mapIndex &&
        world->party.mapX == mapX && world->party.mapY == mapY;

    if (!(square & DUNGEON_SQUARE_MASK_THING_LIST)) return 1;
    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount ||
        !world->things->squareFirstThings) {
        memset(outFacts, 0, sizeof(*outFacts));
        return 0;
    }
    thing = world->things->squareFirstThings[sftIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        unsigned short next = orch_next_thing_compat(world->things, thing);

        if (type == THING_TYPE_GROUP) {
            if (index < 0 || index >= world->things->groupCount ||
                !world->things->groups) goto malformed;
            outFacts->occupiedByGroup = 1;
        } else if (type == THING_TYPE_EXPLOSION) {
            if (index < 0 || index >= world->things->explosionCount ||
                !world->things->explosions) goto malformed;
            if (world->things->explosions[index].type == C050_EXPLOSION_FLUXCAGE) {
                outFacts->hasFluxcage = 1;
            }
        } else if (type < THING_TYPE_DOOR || type > THING_TYPE_EXPLOSION ||
                   index < 0 || index >= world->things->thingCounts[type] ||
                   !world->things->rawThingData[type]) {
            goto malformed;
        }
        if (next == THING_NONE) goto malformed;
        thing = next;
    }
    if (safety >= 64) goto malformed;
    return 1;

malformed:
    memset(outFacts, 0, sizeof(*outFacts));
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
    struct GameWorld_Compat* world,
    int groupIndex,
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
     * for generated/deferred insertion. F0262 still rotates every C04
     * creature as each admitted teleporter hop is consumed. */
    for (remaining = 100; remaining > 0; --remaining) {
        const struct DungeonMapDesc_Compat* map;
        unsigned char squareByte;
        int squareIndex;
        int squareType;
        struct DungeonTeleporter_Compat tp;
        int teleporterFound;
        DM1_V1_GroupTeleporterDestinationPlanPc34 plan;

        if (*inOutMapIndex < 0 || *inOutMapIndex >= (int)world->dungeon->header.mapCount) break;
        map = &world->dungeon->maps[*inOutMapIndex];
        if (*inOutMapX < 0 || *inOutMapX >= map->width ||
            *inOutMapY < 0 || *inOutMapY >= map->height) break;
        if (!world->dungeon->tiles || !world->dungeon->tiles[*inOutMapIndex].squareData) break;

        squareIndex = (*inOutMapX * map->height) + *inOutMapY;
        squareByte = world->dungeon->tiles[*inOutMapIndex].squareData[squareIndex];
        squareType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
        memset(&tp, 0, sizeof(tp));
        teleporterFound = orch_find_teleporter_on_square_compat(
            world, *inOutMapIndex, *inOutMapX, *inOutMapY, &tp);
        if (!DM1_V1_PlanGroupTeleporterDestinationF0267Pc34Compat(
                squareType, DUNGEON_ELEMENT_TELEPORTER,
                (squareByte & 0x08) ? 1 : 0, teleporterFound,
                (int)tp.scope, (int)tp.audible, (int)tp.targetMapIndex,
                (int)tp.targetMapX, (int)tp.targetMapY, *inOutMapIndex,
                *inOutMapX, *inOutMapY,
                (int)world->dungeon->header.mapCount, &plan) ||
            !plan.valid || !plan.shouldTeleport) {
            break;
        }

        if (groupIndex >= 0 && world->things && world->things->groups &&
            groupIndex < world->things->groupCount) {
            struct DungeonGroup_Compat* group = &world->things->groups[groupIndex];
            const struct CreatureBehaviorProfile_Compat* profile =
                CREATURE_GetProfile_Compat((int)group->creatureType);
            DM1_V1_TeleporterDefPc34 rotationTeleporter;
            unsigned int rotatedDirections;
            unsigned int rotatedCells;
            unsigned int directions;
            unsigned int cells;
            int creatureSize;

            if (!profile) return 0;
            creatureSize = profile->attributes & DM1_ATTR_SIZE_MASK;

            memset(&rotationTeleporter, 0, sizeof(rotationTeleporter));
            rotationTeleporter.destFacing = (int)tp.rotation;
            rotationTeleporter.absoluteRotation = tp.absoluteRotation ? 1 : 0;
            if (!dm1_v1_dungeon_get_group_directions_f0147_pc34(
                    world->things, world->creatureAI, world->creatureAICount,
                    world->partyMapIndex, *inOutMapIndex, groupIndex,
                    &directions) ||
                !dm1_v1_dungeon_get_group_cells_f0145_pc34(
                    world->things, world->creatureAI, world->creatureAICount,
                    world->partyMapIndex, *inOutMapIndex, groupIndex,
                    &cells)) {
                return 0;
            }
            if (!DM1_V1_ApplyGroupTeleporterRotationF0262Pc34Compat(
                    &rotationTeleporter, (int)group->count, creatureSize,
                    directions, cells,
                    &rotatedDirections, &rotatedCells)) {
                return 0;
            }
            if (!dm1_v1_dungeon_set_group_directions_f0148_pc34(
                    world->things, world->creatureAI, world->creatureAICount,
                    world->partyMapIndex, *inOutMapIndex, groupIndex,
                    rotatedDirections) ||
                !dm1_v1_dungeon_set_group_cells_f0146_pc34(
                    world->things, world->creatureAI, world->creatureAICount,
                    world->partyMapIndex, *inOutMapIndex, groupIndex,
                    rotatedCells)) {
                return 0;
            }
        }

        *inOutMapIndex = plan.targetMapIndex;
        *inOutMapX = plan.targetMapX;
        *inOutMapY = plan.targetMapY;
        if (plan.shouldEmitAudibleBuzz) {
            orch_record_teleporter_buzz_compat(
                outTeleporterBuzzes, *inOutMapIndex, *inOutMapX, *inOutMapY);
        }
        if (plan.shouldStopChain) break;
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

static int orch_c24_find_fluxcage_thing_compat(
    const struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    unsigned short* outThing)
{
    unsigned short thing;
    unsigned short expected;
    int safety = 0;

    if (outThing) *outThing = THING_NONE;
    if (!world || !world->dungeon || !world->things || !ev ||
        ev->kind != TIMELINE_EVENT_REMOVE_FLUXCAGE ||
        ev->aux0 < 0 || ev->aux0 >= EXPLOSION_LIST_CAPACITY ||
        ev->aux1 != C050_EXPLOSION_FLUXCAGE || ev->aux2 < 0 ||
        ev->aux2 > 0xffff || ev->aux4 != 0 || ev->cell != 0 ||
        ev->mapIndex < 0 ||
        ev->mapIndex >= (int)world->dungeon->header.mapCount ||
        ev->mapX < 0 || ev->mapY < 0 ||
        ev->mapX >= (int)world->dungeon->maps[ev->mapIndex].width ||
        ev->mapY >= (int)world->dungeon->maps[ev->mapIndex].height ||
        world->explosions.entries[ev->aux0].reserved0 == 0 ||
        world->explosions.entries[ev->aux0].explosionType !=
            C050_EXPLOSION_FLUXCAGE ||
        world->explosions.entries[ev->aux0].mapIndex != ev->mapIndex ||
        world->explosions.entries[ev->aux0].mapX != ev->mapX ||
        world->explosions.entries[ev->aux0].mapY != ev->mapY ||
        !world->things->loaded ||
        !world->things->rawThingData[THING_TYPE_EXPLOSION]) {
        return 0;
    }
    expected = (unsigned short)ev->aux2;
    if (THING_GET_TYPE(expected) != THING_TYPE_EXPLOSION ||
        THING_GET_CELL(expected) != 0) {
        return 0;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        world->dungeon, world->things, ev->mapIndex, ev->mapX, ev->mapY);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int index = (int)THING_GET_INDEX(thing);
        if (thing == expected && THING_GET_TYPE(thing) == THING_TYPE_EXPLOSION &&
            index >= 0 && index < world->things->explosionCount &&
            world->things->explosions &&
            world->things->explosions[index].type ==
                C050_EXPLOSION_FLUXCAGE) {
            const unsigned char* raw = dm1_v1_dungeon_get_thing_data_pc34(
                world->things, thing);
            const struct DungeonExplosion_Compat* source =
                &world->things->explosions[index];
            if (!raw || raw[0] != (unsigned char)(source->next & 0xffu) ||
                raw[1] != (unsigned char)(source->next >> 8) ||
                (raw[2] & 0x7fu) != source->type ||
                ((raw[2] >> 7) & 1u) != source->centered ||
                raw[3] != source->attack || ev->aux3 <= 0 ||
                (uint32_t)ev->aux3 != dm1_v1_c15_layout_fingerprint_pc34(raw, 4u)) {
                return 0;
            }
            if (outThing) *outThing = thing;
            return 1;
        }
        thing = orch_next_thing_compat(world->things, thing);
    }
    return 0;
}

static int orch_c13_find_vi_altar_bones_compat(
    const struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    unsigned short* out_thing)
{
    unsigned short thing;
    int safety = 0;

    if (out_thing) *out_thing = THING_NONE;
    if (!world || !world->dungeon || !world->things || !ev ||
        ev->aux4 < 0 || ev->aux4 >= CHAMPION_MAX_PARTY || ev->cell < 0 ||
        ev->cell > 3 || ev->mapIndex < 0 ||
        ev->mapIndex >= (int)world->dungeon->header.mapCount ||
        ev->mapX < 0 || ev->mapY < 0 ||
        ev->mapX >= (int)world->dungeon->maps[ev->mapIndex].width ||
        ev->mapY >= (int)world->dungeon->maps[ev->mapIndex].height) {
        return 0;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        world->dungeon, world->things, ev->mapIndex, ev->mapX, ev->mapY);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int index = (int)THING_GET_INDEX(thing);
        if (THING_GET_TYPE(thing) == THING_TYPE_JUNK &&
            (int)THING_GET_CELL(thing) == ev->cell &&
            index >= 0 && index < world->things->junkCount &&
            world->things->junks &&
            world->things->junks[index].type == DM1_JUNK_TYPE_BONES &&
            world->things->junks[index].chargeCount == ev->aux4) {
            const unsigned char* raw = dm1_v1_dungeon_get_thing_data_pc34(
                world->things, thing);
            const struct DungeonJunk_Compat* junk = &world->things->junks[index];
            unsigned short bits;
            if (!raw) return 0;
            bits = (unsigned short)((unsigned short)raw[2] |
                                    ((unsigned short)raw[3] << 8));
            if ((unsigned short)((unsigned short)raw[0] |
                                 ((unsigned short)raw[1] << 8)) != junk->next ||
                (bits & 0x7fu) != junk->type ||
                ((bits >> 7) & 1u) != junk->doNotDiscard ||
                ((bits >> 8) & 1u) != junk->cursed ||
                ((bits >> 9) & 0x03u) != junk->chargeCount) {
                return 0;
            }
            if (out_thing) *out_thing = thing;
            return 1;
        }
        thing = orch_next_thing_compat(world->things, thing);
    }
    return 0;
}

static int orch_c25_event_source_bound_compat(
    const struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev)
{
    unsigned short thing;
    int matches = 0;
    int safety = 0;

    if (!world || !world->dungeon || !world->things || !ev ||
        !world->things->loaded || !world->things->explosions ||
        !world->things->rawThingData[THING_TYPE_EXPLOSION] ||
        ev->aux0 < 0 || ev->aux0 >= EXPLOSION_LIST_CAPACITY ||
        ev->aux1 < 0 || ev->aux2 < 0 || ev->aux3 <= 0 ||
        world->explosions.entries[ev->aux0].reserved0 == 0) {
        return 0;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        world->dungeon, world->things, ev->mapIndex, ev->mapX, ev->mapY);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int index = THING_GET_INDEX(thing);
        if (THING_GET_TYPE(thing) == THING_TYPE_EXPLOSION &&
            THING_GET_CELL(thing) == (unsigned int)(ev->cell & 3) &&
            index >= 0 && index < world->things->explosionCount &&
            index < world->things->thingCounts[THING_TYPE_EXPLOSION]) {
            const unsigned char* raw = dm1_v1_dungeon_get_thing_data_pc34(
                world->things, thing);
            const struct DungeonExplosion_Compat* source =
                &world->things->explosions[index];
            if (raw && (raw[2] & 0x7fu) == source->type &&
                raw[3] == source->attack && source->type == ev->aux1 &&
                source->attack == ev->aux2 &&
                (uint32_t)ev->aux3 ==
                    dm1_v1_c15_layout_fingerprint_pc34(raw, 4u)) {
                ++matches;
            }
        }
        thing = orch_next_thing_compat(world->things, thing);
    }
    return matches == 1;
}

static int orch_c13_apply_vi_altar_rebirth_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev)
{
    struct TimelineEvent_Compat next;
    struct ChampionState_Compat* champion;
    unsigned short bones;
    int i;

    if (!world || !ev || ev->aux0 != DM1_EVENT_VI_ALTAR_REBIRTH ||
        ev->aux4 < 0 || ev->aux4 >= CHAMPION_MAX_PARTY || ev->aux1 < 0 ||
        ev->aux1 > 2) {
        return 0;
    }
    champion = &world->party.champions[ev->aux4];
    if (!champion->present) return 0;

    /* ReDMCSB TIMELINE.C F0255:1665-1699 has three separate transitions.
     * Each successor is staged before this handler reports success, so a
     * full queue cannot publish a partial C13 sequence. */
    if (ev->aux1 == 2) {
        struct ExplosionCreateInput_Compat input;
        struct TimelineEvent_Compat explosion_advance;
        int explosion_slot = -1;

        if (world->timeline.count > TIMELINE_QUEUE_CAPACITY - 2) return 0;
        memset(&input, 0, sizeof(input));
        input.explosionType = C100_EXPLOSION_REBIRTH_STEP1;
        input.mapIndex = ev->mapIndex;
        input.mapX = ev->mapX;
        input.mapY = ev->mapY;
        input.cell = ev->cell;
        input.currentTick = (int)world->gameTick;
        input.ownerKind = -1;
        input.ownerIndex = ev->aux4;
        input.creatorProjectileSlot = -1;
        if (!F0213_EXPLOSION_Create_Compat(&input, &world->explosions,
                                            &explosion_slot,
                                            &explosion_advance)) {
            return 0;
        }
        next = *ev;
        next.fireAtTick = world->gameTick + 5u;
        next.aux1 = 1;
        if (!F0721_TIMELINE_Schedule_Compat(&world->timeline,
                                             &explosion_advance) ||
            !F0721_TIMELINE_Schedule_Compat(&world->timeline, &next)) {
            return 0;
        }
        return 1;
    }

    if (ev->aux1 == 1) {
        /* The original ends this branch if the matching bones have gone;
         * it does not revive a champion without the exact square-chain hit. */
        if (!orch_c13_find_vi_altar_bones_compat(world, ev, &bones)) return 1;
        if (world->timeline.count >= TIMELINE_QUEUE_CAPACITY ||
            !orch_unlink_thing_from_square_compat(world, ev->mapIndex,
                                                   ev->mapX, ev->mapY, bones)) {
            return 0;
        }
        next = *ev;
        next.fireAtTick = world->gameTick + 1u;
        next.aux1 = 0;
        return F0721_TIMELINE_Schedule_Compat(&world->timeline, &next);
    }

    /* ReDMCSB REVIVE.C F0283:915-937: relocate only if the old cell is
     * occupied by a living champion, clear slots/load, reduce health, and
     * align direction with the party. The M10 state has no UI attribute mask;
     * M11 redraw remains the presentation owner. */
    if (champion->cell <= 3) {
        int occupied = 0;
        for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
            if (i != ev->aux4 && world->party.champions[i].present &&
                world->party.champions[i].hp.current > 0 &&
                world->party.champions[i].cell == champion->cell) {
                occupied = 1;
                break;
            }
        }
        if (occupied) {
            for (i = 0; i < 4; ++i) {
                int used = 0;
                int j;
                for (j = 0; j < CHAMPION_MAX_PARTY; ++j) {
                    if (j != ev->aux4 && world->party.champions[j].present &&
                        world->party.champions[j].hp.current > 0 &&
                        world->party.champions[j].cell == i) {
                        used = 1;
                        break;
                    }
                }
                if (!used) {
                    champion->cell = (unsigned char)i;
                    break;
                }
            }
            if (i == 4) return 0;
        }
    }
    champion->load = 0;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) champion->inventory[i] = THING_NONE;
    {
        RebirthHealthResult_Compat health =
            F0863_RESURRECTION_ComputeRebirthHealth_Compat(
                (int16_t)champion->hp.maximum);
        champion->hp.maximum = (unsigned short)health.newMaxHealth;
        champion->hp.current = (unsigned short)health.newCurrentHealth;
        champion->hp.shifted = (unsigned short)(champion->hp.maximum << 1);
    }
    champion->direction = (unsigned char)world->party.direction;
    return 1;
}

static int orch_write_raw_projectile_f0219_compat(
    struct DungeonThings_Compat* things,
    int projectileIndex);

/* ReDMCSB TIMELINE.C F0255:1665-1699 owns the live C13 rebirth state
 * machine: step 2 stages the rebirth explosion, step 1 consumes the
 * authenticated bones and step 0 applies REVIVE.C F0283.  F0887 is the
 * F0435 save/HOC boundary and deliberately consumes external C13 receipts
 * without chaining, so live hosts (M11) drive the source sequence
 * through this boundary before their general queue dispatch. */
int DM1_V1_F0255_DispatchDueViAltarRebirthPc34Compat(
    struct GameWorld_Compat* world)
{
    int dispatched = 0;
    int i;

    if (!world) return 0;
    for (i = world->timeline.count - 1; i >= 0; --i) {
        struct TimelineEvent_Compat ev;

        if (world->timeline.events[i].kind !=
                TIMELINE_EVENT_VI_ALTAR_REBIRTH ||
            world->timeline.events[i].fireAtTick > world->gameTick) {
            continue;
        }
        ev = world->timeline.events[i];
        memmove(&world->timeline.events[i],
                &world->timeline.events[i + 1],
                (size_t)(world->timeline.count - i - 1) *
                    sizeof(world->timeline.events[0]));
        --world->timeline.count;
        memset(&world->timeline.events[world->timeline.count], 0,
               sizeof(world->timeline.events[0]));
        /* ReDMCSB PROJEXPL.C F0214 C14 records index the queue
         * physically: a mid-queue removal shifts only the receipts
         * that were queued after it. */
        if (world->things && world->things->projectiles) {
            int p;
            for (p = 0; p < world->things->projectileCount; ++p) {
                struct DungeonProjectile_Compat* projectile =
                    &world->things->projectiles[p];
                if (projectile->eventIndex != 0xFFFFu &&
                    (int)projectile->eventIndex > i) {
                    --projectile->eventIndex;
                    (void)orch_write_raw_projectile_f0219_compat(
                        world->things, p);
                }
            }
        }
        if (orch_c13_apply_vi_altar_rebirth_compat(world, &ev)) {
            ++dispatched;
        }
    }
    return dispatched;
}

/* ReDMCSB PROJEXPL.C F0214:207-223 deletes exactly the Timeline record
 * named by the raw C14 PROJECTILE.EventIndex.  The old compat route swept
 * every C49 with the same host slot, which can discard a distinct queued
 * movement receipt.  The compact host queue has no native event allocator,
 * so accept its physical index only when it still names the exact C49 owner;
 * malformed/imported indexes fail closed. */
static int orch_delete_projectile_event_f0214_compat(
    struct GameWorld_Compat* world,
    int projectileIndex)
{
    const struct TimelineEvent_Compat* event;
    const struct DungeonProjectile_Compat* projectile;
    int i;
    int eventIndex;

    if (!world || !world->things || !world->things->projectiles ||
        projectileIndex < 0 ||
        projectileIndex >= world->things->projectileCount ||
        world->timeline.count < 0 ||
        world->timeline.count > TIMELINE_QUEUE_CAPACITY) {
        return 0;
    }
    projectile = &world->things->projectiles[projectileIndex];
    eventIndex = (int)projectile->eventIndex;
    if (eventIndex < 0 || eventIndex >= world->timeline.count) return 0;
    event = &world->timeline.events[eventIndex];
    if (event->kind != TIMELINE_EVENT_PROJECTILE_MOVE ||
        event->aux0 != projectileIndex) {
        return 0;
    }

    for (i = eventIndex + 1; i < world->timeline.count; ++i) {
        world->timeline.events[i - 1] = world->timeline.events[i];
    }
    --world->timeline.count;
    memset(&world->timeline.events[world->timeline.count], 0,
           sizeof(world->timeline.events[world->timeline.count]));

    /* Queue compaction changes only later host positions.  Preserve the
     * native C14 owner relation for every live record that named one. */
    for (i = 0; i < world->things->projectileCount; ++i) {
        struct DungeonProjectile_Compat* candidate =
            &world->things->projectiles[i];
        if (candidate->eventIndex != 0xFFFFu &&
            candidate->eventIndex > (unsigned short)eventIndex) {
            --candidate->eventIndex;
            if (!orch_write_raw_projectile_f0219_compat(world->things, i)) {
                return 0;
            }
        }
    }
    return 1;
}

/* The host queue is ordered rather than the original fixed EVENT pool.  Keep
 * a decoded C14 EventIndex coherent whenever its physical owner moves left
 * after dispatch.  An index of zero may name the event being dispatched, so
 * it is retained until F0214/F0215 or F0219 resolves that owner. */
static void orch_shift_projectile_event_indexes_after_pop_compat(
    struct GameWorld_Compat* world)
{
    int i;

    if (!world || !world->things || !world->things->projectiles) return;
    for (i = 0; i < world->things->projectileCount; ++i) {
        struct DungeonProjectile_Compat* projectile =
            &world->things->projectiles[i];
        if (projectile->eventIndex != 0xFFFFu && projectile->eventIndex > 0) {
            --projectile->eventIndex;
            (void)orch_write_raw_projectile_f0219_compat(world->things, i);
        }
    }
}

/* ReDMCSB PROJEXPL.C F0219 owns one C14 record and one live projectile.
 * A loaded raw record is authoritative: do not advance a diverged decoded
 * mirror and then overwrite the original bytes with a synthetic state. */
static int orch_validate_raw_projectile_f0219_compat(
    const struct DungeonThings_Compat* things,
    int projectileIndex)
{
    const struct DungeonProjectile_Compat* projectile;
    const unsigned char* raw;
    unsigned short value;

    if (!things || !things->projectiles || projectileIndex < 0 ||
        projectileIndex >= things->projectileCount) {
        return 0;
    }
    raw = things->rawThingData[THING_TYPE_PROJECTILE];
    if (!raw) return 1;
    if (things->thingCounts[THING_TYPE_PROJECTILE] < things->projectileCount) {
        return 0;
    }
    projectile = &things->projectiles[projectileIndex];
    raw += (size_t)projectileIndex * 8u;
    value = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
    if (value != projectile->next) return 0;
    value = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
    if (value != projectile->slot || raw[4] != projectile->kineticEnergy ||
        raw[5] != projectile->attack) {
        return 0;
    }
    value = (unsigned short)(raw[6] | ((unsigned short)raw[7] << 8));
    return value == projectile->eventIndex;
}

/* GROUP.C F0209 reaches PROJEXPL.C F0218 while the C14 remains linked to
 * the struck square.  The physical host queue is not an original EVENT
 * pool, so its index is admissible only when it still names the exact C48/C49
 * receipt for this raw C14 owner. */
int F0890g_ORCH_ValidateF0218ImpactOwner_Compat(
    const struct GameWorld_Compat* world,
    int projectileIndex)
{
    const struct DungeonProjectile_Compat* projectile;
    const struct TimelineEvent_Compat* event;
    int eventIndex;

    if (!world || !world->things || !world->things->projectiles ||
        projectileIndex < 0 ||
        projectileIndex >= world->things->projectileCount ||
        !orch_validate_raw_projectile_f0219_compat(
            world->things, projectileIndex) ||
        world->timeline.count < 0 ||
        world->timeline.count > TIMELINE_QUEUE_CAPACITY) {
        return 0;
    }
    projectile = &world->things->projectiles[projectileIndex];
    eventIndex = (int)projectile->eventIndex;
    if (eventIndex < 0 || eventIndex >= world->timeline.count) return 0;
    event = &world->timeline.events[eventIndex];
    return event->kind == TIMELINE_EVENT_PROJECTILE_MOVE &&
           event->aux0 == projectileIndex;
}

static int orch_write_raw_projectile_f0219_compat(
    struct DungeonThings_Compat* things,
    int projectileIndex)
{
    struct DungeonProjectile_Compat* projectile;
    unsigned char* raw;

    if (!things || !things->projectiles || projectileIndex < 0 ||
        projectileIndex >= things->projectileCount) {
        return 0;
    }
    projectile = &things->projectiles[projectileIndex];
    raw = things->rawThingData[THING_TYPE_PROJECTILE];
    if (!raw) return 1;
    if (things->thingCounts[THING_TYPE_PROJECTILE] < things->projectileCount) {
        return 0;
    }
    raw += (size_t)projectileIndex * 8u;
    raw[0] = (unsigned char)(projectile->next & 0xffu);
    raw[1] = (unsigned char)(projectile->next >> 8);
    raw[2] = (unsigned char)(projectile->slot & 0xffu);
    raw[3] = (unsigned char)(projectile->slot >> 8);
    raw[4] = projectile->kineticEnergy;
    raw[5] = projectile->attack;
    raw[6] = (unsigned char)(projectile->eventIndex & 0xffu);
    raw[7] = (unsigned char)(projectile->eventIndex >> 8);
    return 1;
}

/* F0219 inserts a C48/C49 into an ordered host queue. Every later C14
 * physical index shifts with that insertion, so validate all affected raw
 * owners before the flight relink commits any C14, SFT or timeline state. */
static int orch_projectile_move_schedule_admissible_f0219_compat(
    const struct GameWorld_Compat* world,
    int projectileIndex,
    const struct TimelineEvent_Compat* event)
{
    int insertIndex;
    int i;

    if (!world || !event || event->kind != TIMELINE_EVENT_PROJECTILE_MOVE ||
        event->aux0 != projectileIndex ||
        world->timeline.count < 0 ||
        world->timeline.count >= TIMELINE_QUEUE_CAPACITY) {
        return 0;
    }
    insertIndex = world->timeline.count;
    while (insertIndex > 0 &&
           world->timeline.events[insertIndex - 1].fireAtTick > event->fireAtTick) {
        --insertIndex;
    }
    if (!world->things || !world->things->rawThingData[THING_TYPE_PROJECTILE]) {
        return 1;
    }
    if (!world->things->projectiles || projectileIndex < 0 ||
        projectileIndex >= world->things->projectileCount ||
        world->things->thingCounts[THING_TYPE_PROJECTILE] <
            world->things->projectileCount ||
        !orch_validate_raw_projectile_f0219_compat(
            world->things, projectileIndex)) {
        return 0;
    }
    for (i = 0; i < world->things->projectileCount; ++i) {
        const struct DungeonProjectile_Compat* candidate =
            &world->things->projectiles[i];
        if (i != projectileIndex && candidate->eventIndex != 0xFFFFu &&
            candidate->eventIndex >= (unsigned short)insertIndex &&
            !orch_validate_raw_projectile_f0219_compat(world->things, i)) {
            return 0;
        }
    }
    return 1;
}

int F0890h_ORCH_AdmitF0219Reschedule_Compat(
    const struct GameWorld_Compat* world,
    int projectileIndex,
    const struct TimelineEvent_Compat* event)
{
    return orch_projectile_move_schedule_admissible_f0219_compat(
        world, projectileIndex, event);
}

/* F0219 creates the next C48/C49, then stores its EventIndex in the same
 * C14 owner that F0214 later consumes. C49 does not participate in the
 * host's door-only merge path, so the admitted insertion position is stable. */
static int orch_schedule_projectile_move_f0219_compat(
    struct GameWorld_Compat* world,
    int projectileIndex,
    const struct TimelineEvent_Compat* event)
{
    int insertIndex;
    int i;

    if (!orch_projectile_move_schedule_admissible_f0219_compat(
            world, projectileIndex, event)) {
        return 0;
    }
    insertIndex = world->timeline.count;
    while (insertIndex > 0 &&
           world->timeline.events[insertIndex - 1].fireAtTick > event->fireAtTick) {
        --insertIndex;
    }
    if (!F0721_TIMELINE_Schedule_Compat(&world->timeline, event)) return 0;
    if (!world->things || !world->things->projectiles ||
        projectileIndex < 0 || projectileIndex >= world->things->projectileCount) {
        return 1;
    }
    for (i = 0; i < world->things->projectileCount; ++i) {
        struct DungeonProjectile_Compat* candidate =
            &world->things->projectiles[i];
        if (i != projectileIndex && candidate->eventIndex != 0xFFFFu &&
            candidate->eventIndex >= (unsigned short)insertIndex) {
            ++candidate->eventIndex;
        }
    }
    world->things->projectiles[projectileIndex].eventIndex =
        (unsigned short)insertIndex;
    return orch_write_raw_projectile_f0219_compat(world->things, projectileIndex);
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

static int orch_projectile_instance_active_compat(
    const struct ProjectileInstance_Compat* projectile)
{
    return projectile && projectile->slotIndex >= 0 && projectile->reserved3 != 0;
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
        if (i == projectileIndex || !orch_projectile_instance_active_compat(other)) continue;
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
        if (i == projectileIndex || !orch_projectile_instance_active_compat(other)) continue;
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

static int orch_pack_group_directions_compat(int direction, int creatureCount)
{
    int packed = 0;
    int creatureIndex;
    if (creatureCount < 0) creatureCount = 0;
    if (creatureCount > 3) creatureCount = 3;
    for (creatureIndex = 0; creatureIndex <= creatureCount; ++creatureIndex) {
        packed |= (direction & 0x03) << (creatureIndex << 1);
    }
    return packed;
}

static int orch_active_group_directions_compat(
    const struct CreatureAIState_Compat* ai,
    const struct DungeonGroup_Compat* group)
{
    int packed;
    if (!group) return 0;
    packed = ai ? ai->groupDirection : 0;
    /* Older in-memory/save records held only GROUP.Direction.  Promote that
     * scalar at the M10 boundary instead of treating missing high slots as
     * North-facing creatures. */
    if ((packed & ~0x03) == 0) {
        packed = orch_pack_group_directions_compat(packed, (int)group->count);
    }
    return packed & 0xff;
}


static int orch_apply_f0205_active_creature_direction_compat(
    struct GameWorld_Compat* world,
    struct CreatureAIState_Compat* ai,
    struct DungeonGroup_Compat* group,
    struct DM1ActiveGroup_Compat* activeGroup,
    int direction,
    int creatureIndex,
    int creatureSize)
{
    if (!world || !ai || !group || !activeGroup || direction < 0 || direction > 3 ||
        !F0817b_DM1_GROUP_SetCreatureDirectionWithRng_Compat(
            activeGroup, direction, creatureIndex, creatureSize,
            (int)group->count, &world->masterRng)) {
        return 0;
    }
    ai->groupDirection = activeGroup->directions & 0xff;
    group->direction = (unsigned char)(activeGroup->directions & 0x03);
    return 1;
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

/* ReDMCSB GROUP.C:F0190 lines 892-917 compacts the packed direction and
 * ACTIVE_GROUP aspect entries alongside Health/Cells after a single creature
 * dies.  The projectile precheck owns the raw Health/Count/Cells mutation;
 * M10 owns this live-party ACTIVE_GROUP counterpart. */
static void orch_compact_active_group_after_f0190_killed_some_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    int mapIndex,
    int killedCreatureIndex,
    int originalGroupCount)
{
    struct CreatureAIState_Compat* ai;
    int activeIndex;
    int i;

    if (!world || !group || killedCreatureIndex < 0 ||
        killedCreatureIndex >= originalGroupCount ||
        mapIndex != world->partyMapIndex) {
        return;
    }
    activeIndex = orch_find_active_group_state_index_compat(
        world, (int)(group - world->things->groups));
    if (activeIndex < 0) return;

    ai = &world->creatureAI[activeIndex];
    for (i = killedCreatureIndex; i < originalGroupCount; ++i) {
        int nextDirection = (ai->groupDirection >> ((i + 1) << 1)) & 0x03;
        ai->groupDirection =
            (ai->groupDirection & ~(0x03 << (i << 1))) |
            (nextDirection << (i << 1));
        ai->aspect[i] = ai->aspect[i + 1];
    }
    /* F0184/F0148 expose only creature zero in raw GROUP.Direction. */
    group->direction = (unsigned char)(ai->groupDirection & 0x03);
}

static int orch_maybe_attach_projectile_weapon_to_group_slot_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const struct ProjectileInstance_Compat* projectile,
    int damageOutcome);

static int orch_cmd_attack_apply_f0190_mutation_dispatch_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const DM1_MeleeF0190MutationDispatchPlanPc34* plan);

static void orch_schedule_group_reaction_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    const struct DungeonGroup_Compat* group,
    const struct CombatAction_Compat* action,
    int reactionKind);

static int orch_apply_projectile_creature_precheck_with_plan_compat(
    struct DungeonGroup_Compat* group,
    int creatureIndex,
    const struct ProjectileInstance_Compat* projectile,
    DM1_ProjectileCreaturePrecheckDamagePlanPc34* outPlan)
{
    const struct CreatureBehaviorProfile_Compat* profile;
    int defense;
    int i;
    DM1_ProjectileCreaturePrecheckDamagePlanPc34 plan;

    if (!group || !projectile || creatureIndex < 0 || creatureIndex > 3) return 0;
    profile = CREATURE_GetProfile_Compat(group->creatureType);
    if (profile && (profile->attributes & CREATURE_ATTR_MASK_ARCHENEMY)) return 0;

    defense = (profile && profile->baseDefense > 0) ? profile->baseDefense : 64;
    memset(&plan, 0, sizeof(plan));
    if (!dm1_v1_projectile_creature_precheck_damage_plan_pc34(
            projectile, group, creatureIndex, defense,
            profile ? profile->attributes : 0, &plan) ||
        !plan.valid || !plan.handled) {
        return 0;
    }
    if (!plan.shouldWriteGroup) return 0;
    for (i = 0; i < 4; ++i) group->health[i] = plan.newHealth[i];
    group->count = plan.newCount;
    group->cells = plan.newCells;
    if (outPlan) *outPlan = plan;
    return plan.outcomeCode;
}

int F0890a_ORCH_ApplyProjectileCreatureImpact_Compat(
    struct DungeonGroup_Compat* group,
    int creatureIndex,
    const struct ProjectileInstance_Compat* projectile)
{
    return orch_apply_projectile_creature_precheck_with_plan_compat(
        group, creatureIndex, projectile, NULL);
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

static int orch_projectile_associated_weapon_type_compat(
    const struct GameWorld_Compat* world,
    const struct ProjectileInstance_Compat* projectile)
{
    unsigned short associatedThing;
    int weaponIndex;

    if (!world || !world->things || !projectile || !world->things->weapons) {
        return -1;
    }
    associatedThing = (unsigned short)projectile->reserved1;
    if (associatedThing == THING_NONE ||
        associatedThing == THING_ENDOFLIST ||
        THING_GET_TYPE(associatedThing) != THING_TYPE_WEAPON) {
        return -1;
    }
    weaponIndex = (int)THING_GET_INDEX(associatedThing);
    if (weaponIndex < 0 || weaponIndex >= world->things->weaponCount) {
        return -1;
    }
    return (int)world->things->weapons[weaponIndex].type;
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
    int hasProjectileOnSquare = 0;

    if (outKilledGroup) *outKilledGroup = 0;
    if (!world || !group || !ordinalInCell || !world->dungeon || !world->things) return 0;
    sftIndex = orch_square_first_thing_list_index_compat(world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 1;

    /* F0209 calls F0218 before the deferred C38 cell write. A loaded C14
     * chain is authoritative when the square actually contains a C14.
     * Empty projectile chains are not malformed PC34 state and must not
     * block an otherwise authenticated C04/SFT group transition. */
    {
        int cell;
        unsigned short thing = world->things->squareFirstThings[sftIndex];
        int safety = 0;

        while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
            if (THING_GET_TYPE(thing) == THING_TYPE_PROJECTILE) {
                hasProjectileOnSquare = 1;
                break;
            }
            thing = orch_next_thing_compat(world->things, thing);
        }
        if (safety >= 64) return 0;
        if (!hasProjectileOnSquare) goto authenticated_projectile_scan_complete;
        for (cell = 0; cell < 4; ++cell) {
            int count;
            if (ordinalInCell[cell] &&
                !dm1_v1_f0218_count_authenticated_c14_at_cell_pc34(
                    world->dungeon, world->things, &world->projectiles,
                    mapIndex, mapX, mapY, cell, &count)) return 0;
        }
    }

authenticated_projectile_scan_complete:

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
                const struct CreatureBehaviorProfile_Compat* profile;
                DM1_ProjectileCreaturePrecheckDamagePlanPc34 precheckPlan;
                DM1_ProjectileCreatureImpactAftermathPc34 aftermath;
                DM1_MeleeF0190MutationDispatchInputPc34 dispatchIn;
                DM1_MeleeF0190MutationDispatchPlanPc34 dispatchPlan;
                struct CombatAction_Compat reactionAction;
                int creatureIndex = (int)ordinalInCell[cell] - 1;
                int outcome;
                int combatOutcome;
                int creatureAttributes = 0;
                int creatureProperties = 0;
                int originalGroupCount = (int)group->count;
                int associatedWeaponType = -1;

                orch_build_precheck_projectile_instance_compat(
                    projectile, index, &compatProjectile);
                profile = CREATURE_GetProfile_Compat((int)group->creatureType);
                if (profile) {
                    creatureAttributes = profile->attributes;
                    creatureProperties = profile->properties;
                }
                associatedWeaponType =
                    orch_projectile_associated_weapon_type_compat(
                        world, &compatProjectile);
                /* MOVESENS.C:F0266:292-301 calls F0217 on matching projectile
                 * cells, then F0214 deletes exactly C14.EventIndex before
                 * F0217/PROJEXPL.C:607-608 unlinks/deletes the Thing. */
                /* F0218 reaches F0217 only through the exact C14-owned
                 * C48/C49 event that F0214 can delete. A raw C14 whose
                 * physical queue owner drifted is not an impact receipt. */
                if (!F0890g_ORCH_ValidateF0218ImpactOwner_Compat(
                        world, index) ||
                    !orch_delete_projectile_event_f0214_compat(world, index)) {
                    return 0;
                }
                (void)orch_unlink_thing_from_square_compat(world, mapIndex, mapX, mapY, thing);
                projectile->next = THING_NONE;
                projectile->eventIndex = 0xFFFFu;
                /* F0217/F0214 have consumed the authenticated C14/C49 pair
                 * before C38 commits its deferred C04 cell write. Retire the
                 * matching M10 projection here as well: leaving it live lets
                 * a later dispatcher re-run an already-resolved impact. */
                F0813_PROJECTILE_Despawn_Compat(&world->projectiles, index);
                memset(&precheckPlan, 0, sizeof(precheckPlan));
                outcome = orch_apply_projectile_creature_precheck_with_plan_compat(
                    group, creatureIndex, &compatProjectile, &precheckPlan);
                if (outcome == 1) { /* F0190 C1 killed some */
                    orch_compact_active_group_after_f0190_killed_some_compat(
                        world, group, mapIndex, creatureIndex,
                        originalGroupCount);
                }
                /* The F0217 precheck keeps the original C0/C1/C2 outcome
                 * ordinal.  Convert once for M10 F0190 dispatch consumers. */
                combatOutcome = (outcome == 2) ? COMBAT_OUTCOME_KILLED_ALL_CREATURES :
                    ((outcome == 1) ? COMBAT_OUTCOME_KILLED_SOME_CREATURES :
                                      COMBAT_OUTCOME_KILLED_NO_CREATURES);
                memset(&aftermath, 0, sizeof(aftermath));
                if (dm1_v1_projectile_creature_precheck_aftermath_pc34(
                        &precheckPlan, &compatProjectile, creatureAttributes,
                        associatedWeaponType, &aftermath) &&
                    aftermath.keepSharpWeaponInGroup) {
                    /* ReDMCSB PROJEXPL.C:F0217 lines 540-553 can pass
                     * GROUP.Slot to F0215 so F0266 source/destination
                     * projectile prechecks preserve thrown sharp weapons as
                     * group possessions when no creature is killed. */
                    (void)orch_maybe_attach_projectile_weapon_to_group_slot_compat(
                        world, group, &compatProjectile, combatOutcome);
                }
                if (outcome == 1 || outcome == 2) {
                    /* ReDMCSB PROJEXPL.C F0217 calls GROUP.C F0190 after a
                     * projectile kill.  F0266 and C38 both use this helper,
                     * so one source-owned dispatch must provide possession
                     * drops, C29-C41 cleanup, fear, unlink and raw writeback. */
                    memset(&dispatchIn, 0, sizeof(dispatchIn));
                    memset(&dispatchPlan, 0, sizeof(dispatchPlan));
                    dispatchIn.outcome = combatOutcome;
                    dispatchIn.groupIndex = (int)(group - world->things->groups);
                    dispatchIn.groupBehavior = (int)group->behavior;
                    dispatchIn.killedCreatureIndex = creatureIndex;
                    dispatchIn.originalGroupCount = originalGroupCount;
                    dispatchIn.creatureType = (int)group->creatureType;
                    dispatchIn.creatureAttributes = creatureAttributes;
                    dispatchIn.creatureProperties = creatureProperties;
                    dispatchIn.killedCell = precheckPlan.killedCell;
                    dispatchIn.mapIndex = mapIndex;
                    dispatchIn.mapX = mapX;
                    dispatchIn.mapY = mapY;
                    dispatchIn.partyMapIndex = world->partyMapIndex;
                    dispatchIn.partyMapX = world->party.mapX;
                    dispatchIn.partyMapY = world->party.mapY;
                    if (!dm1_v1_melee_mutation_dispatch_plan_f0190_pc34(
                            &dispatchIn, &dispatchPlan) || !dispatchPlan.valid) {
                        return 0;
                    }
                    /* This helper returns whether F0190 fear triggered, not
                     * a success code; a calm survivor is still a valid hit. */
                    (void)orch_cmd_attack_apply_f0190_mutation_dispatch_compat(
                        world, group, &dispatchPlan);
                }
                /* ReDMCSB PROJEXPL.C:F0217 calls F0209 C30 after every
                 * creature impact except a whole-group kill.  This includes
                 * F0190 C1 killed-some: compaction/fear happens first, then
                 * the surviving group receives its projectile reaction. */
                if (outcome != 2 && aftermath.scheduleReaction) {
                    memset(&reactionAction, 0, sizeof(reactionAction));
                    reactionAction.targetMapIndex = mapIndex;
                    reactionAction.targetMapX = mapX;
                    reactionAction.targetMapY = mapY;
                    orch_schedule_group_reaction_compat(
                        world, (int)(group - world->things->groups), group,
                        &reactionAction, DM1_CM2_REACTION_HIT_BY_PROJECTILE);
                }
                orch_write_raw_group_compat(
                    world->things, (int)(group - world->things->groups));
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
    out->destTeleporterNewDirection = PROJECTILE_TELEPORTER_ROTATION_NONE;

    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat* other =
            &world->projectiles.entries[i];
        if (i == projectileIndex || !orch_projectile_instance_active_compat(other)) continue;
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
    if (world->things && world->things->loaded &&
        (destSquare & DUNGEON_SQUARE_MASK_THING_LIST) &&
        !dm1_v1_f0221_fluxcage_on_square_pc34(
            world->dungeon, world->things, projectile->mapIndex, destX, destY,
            &out->destHasFluxcage)) {
        /* F0221 owns the C15 test before F0219 may classify the blocker.
         * A malformed raw/decoded C15 chain cannot become a host fluxcage. */
        return 0;
    }
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

    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat* other =
            &world->projectiles.entries[i];
        if (i == projectileIndex || !orch_projectile_instance_active_compat(other)) continue;
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

    /* ReDMCSB MOVESENS.C F0267:450-482 admits a C14 through an open
     * object-scope C04 teleporter after F0219 has crossed the source edge;
     * lines 526-531 apply F0263 to that landing cell.  Keep the landing
     * square's blockers for this C48 pass, then publish only the real target
     * and packed rotation for the next continuation. */
    if (out->destSquareType == DUNGEON_ELEMENT_TELEPORTER &&
        (destSquare & 0x08) && world->things && world->things->loaded) {
        struct DungeonTeleporter_Compat teleporter;
        int landingCell;
        int rotatedDirection;
        int rotatedCell;
        DM1_V1_TeleporterDefPc34 rotationTeleporter;
        const struct DungeonMapDesc_Compat* targetMap;

        if ((projectile->direction == projectile->cell ||
             ((projectile->direction + 1) & 3) == projectile->cell) &&
            orch_find_teleporter_on_square_compat(
                world, projectile->mapIndex, destX, destY, &teleporter) &&
            (teleporter.scope & 0x02) &&
            teleporter.targetMapIndex < world->dungeon->header.mapCount) {
            targetMap = &world->dungeon->maps[teleporter.targetMapIndex];
            if (teleporter.targetMapX < targetMap->width &&
                teleporter.targetMapY < targetMap->height) {
                landingCell = ((projectile->direction & 1) ==
                               (projectile->cell & 1))
                    ? ((projectile->cell - 1) & 3)
                    : ((projectile->cell + 1) & 3);
                memset(&rotationTeleporter, 0, sizeof(rotationTeleporter));
                rotationTeleporter.destFacing = (int)teleporter.rotation;
                rotationTeleporter.absoluteRotation =
                    teleporter.absoluteRotation ? 1 : 0;
                if (!DM1_V1_ApplyTeleporterRotationF0267Pc34Compat(
                        DM1_V1_TELEPORTER_ROTATE_THING_PROJECTILE_PC34,
                        projectile->mapX, &rotationTeleporter,
                        projectile->direction, landingCell,
                        &rotatedDirection, &rotatedCell)) {
                    return 0;
                }
                out->destMapIndex = teleporter.targetMapIndex;
                out->destMapX = teleporter.targetMapX;
                out->destMapY = teleporter.targetMapY;
                out->destTeleporterNewDirection =
                    PROJECTILE_TELEPORTER_DIRECTION_CELL(
                        rotatedDirection, rotatedCell);
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
static void orch_schedule_group_reaction_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    const struct DungeonGroup_Compat* group,
    const struct CombatAction_Compat* action,
    int reactionKind);
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
    int mapY,
    int* outSoundId);
static int orch_drop_group_slot_possessions_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    int mapIndex,
    int mapX,
    int mapY,
    int* outSoundId);
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
    const DM1_MeleeF0190MutationDispatchPlanPc34* dispatchPlan);
static int orch_cmd_attack_apply_f0190_mutation_dispatch_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const DM1_MeleeF0190MutationDispatchPlanPc34* plan);

static int orch_materialize_projectile_associated_thing_compat(
    struct GameWorld_Compat* world,
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* tickResult,
    int associatedThingMovedToGroup)
{
    int sftIndex;
    DM1_ProjectileMaterializationReceiptPc34 receipt;
    unsigned short chainThings[64];
    unsigned short current;
    int chainCount = 0;
    int mapIndex;
    int mapX;
    int mapY;
    int associatedIndex;

    if (!world || !projectile || associatedThingMovedToGroup) return 1;
    if (!world->things || !world->dungeon) return 1;

    mapIndex = projectile->mapIndex;
    mapX = projectile->mapX;
    mapY = projectile->mapY;
    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) {
        return 1;
    }

    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_projectile_materialization_receipt_f0215_pc34(
            projectile, tickResult, associatedThingMovedToGroup,
            world->things->potionCount,
            world->things->squareFirstThings[sftIndex],
            NULL, 0, &receipt) ||
        !receipt.valid || !receipt.handled) {
        return 1;
    }

    /* PROJEXPL.C F0217 consumes a thrown potion before its F0215 tail.  The
     * receipt carries that earlier ownership decision so the M10 consumer
     * can update the same authenticated C05 record rather than merely hide
     * it in M11.  Preflight before C14/chain mutation. */
    if (receipt.shouldConsumePotion) {
        associatedIndex = (int)THING_GET_INDEX(
            receipt.materialization.associatedThing);
        if (THING_GET_TYPE(receipt.materialization.associatedThing) !=
                THING_TYPE_POTION ||
            !world->things->potions || associatedIndex < 0 ||
            associatedIndex >= world->things->potionCount) {
            return 0;
        }
        if (world->things->projectiles &&
            projectile->slotIndex >= 0 &&
            projectile->slotIndex < world->things->projectileCount &&
            (world->things->projectiles[projectile->slotIndex].slot & 0x3fffu) !=
                (receipt.materialization.associatedThing & 0x3fffu)) {
            return 0;
        }
    }

    if (receipt.shouldMaterialize) {
        if (receipt.mapIndex != mapIndex ||
            receipt.mapX != mapX ||
            receipt.mapY != mapY) {
            sftIndex = orch_square_first_thing_list_index_compat(
                world->dungeon, receipt.mapIndex, receipt.mapX, receipt.mapY);
            if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) {
                return 1;
            }
        }
        current = world->things->squareFirstThings[sftIndex];
        while (current != THING_NONE &&
               current != THING_ENDOFLIST &&
               chainCount < (int)(sizeof(chainThings) / sizeof(chainThings[0]))) {
            chainThings[chainCount++] = current;
            current = orch_next_thing_compat(world->things, current);
        }
        if (chainCount < (int)(sizeof(chainThings) / sizeof(chainThings[0]))) {
            chainThings[chainCount++] = current;
        }

        if (!dm1_v1_projectile_materialization_receipt_f0215_pc34(
                projectile, tickResult, associatedThingMovedToGroup,
                world->things->potionCount,
                world->things->squareFirstThings[sftIndex],
                chainThings, chainCount, &receipt) ||
            !receipt.valid || !receipt.handled || !receipt.shouldMaterialize) {
            return 1;
        }
    }

    /* ReDMCSB PROJEXPL.C:F0215 lines 248-260 moves Projectile.Slot to
     * the projectile map square when F0217 does not pass a GROUP.Slot
     * pointer.  F0219 lines 687-743 can retarget champion impacts before
     * delete/materialization reaches F0215.  DUNGEON.C:F0163 lines
     * 1798-1837 owns the empty-square vs append-after-tail writeback; M10
     * now applies DM1's combined delete/materialize receipt instead of
     * rebuilding link or projectile-delete decisions locally. */
    if (!receipt.shouldDeleteProjectile || !receipt.shouldClearProjectileNext) {
        return 0;
    }
    if (world->things->projectiles &&
        world->things->projectileCount >
            (int)THING_GET_INDEX(receipt.projectileThing) &&
        !orch_set_next_thing_compat(
            world->things, receipt.projectileThing,
            receipt.projectileNextAfterDelete)) {
        return 0;
    }
    if (receipt.shouldConsumePotion && !orch_set_next_thing_compat(
            world->things, receipt.materialization.associatedThing,
            THING_NONE)) {
        return 0;
    }
    if (!receipt.shouldMaterialize) {
        return 1;
    }
    if (receipt.squareAttach.chainOverflow ||
        !receipt.squareAttach.shouldSetDroppedNextEnd ||
        !orch_set_next_thing_compat(
            world->things, receipt.squareAttach.baseThing, THING_ENDOFLIST)) {
        return 0;
    }
    if (receipt.squareAttach.shouldSetSquareFirstThing) {
        world->things->squareFirstThings[sftIndex] =
            receipt.squareAttach.droppedThing;
        return 1;
    }
    if (receipt.squareAttach.shouldAppendAfterTail &&
        receipt.squareAttach.foundTail) {
        return orch_set_next_thing_compat(
            world->things, receipt.squareAttach.tailThing,
            receipt.squareAttach.droppedThing);
    }
    return 0;
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

static int orch_validate_f0217_thrown_potion_receipt_compat(
    const struct GameWorld_Compat* world,
    const struct ProjectileInstance_Compat* projectile,
    int projectileIndex,
    const struct ProjectileTickResult_Compat* tickResult,
    const struct ExplosionCreateInput_Compat* explosionIn,
    int* outC15Cell)
{
    const struct DungeonProjectile_Compat* sourceProjectile;
    const struct DungeonPotion_Compat* potion;
    const unsigned char* rawPotion;
    unsigned short potionThing;
    unsigned short rawNext;
    int potionIndex;

    if (!world || !world->things || !projectile || !tickResult ||
        !explosionIn || !outC15Cell ||
        !(projectile->flags & PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT) ||
        projectileIndex < 0 || projectileIndex >= world->things->projectileCount ||
        !world->things->projectiles ||
        tickResult->outExplosion.creatorProjectileSlot != projectile->slotIndex ||
        projectile->slotIndex != projectileIndex ||
        !orch_validate_raw_projectile_f0219_compat(world->things, projectileIndex)) {
        return 0;
    }

    sourceProjectile = &world->things->projectiles[projectileIndex];
    potionThing = sourceProjectile->slot;
    if (potionThing != (unsigned short)projectile->reserved1 ||
        potionThing == THING_NONE || THING_GET_TYPE(potionThing) != THING_TYPE_POTION) {
        return 0;
    }
    potionIndex = THING_GET_INDEX(potionThing);
    if (!world->things->potions ||
        !world->things->rawThingData[THING_TYPE_POTION] ||
        potionIndex < 0 || potionIndex >= world->things->potionCount ||
        potionIndex >= world->things->thingCounts[THING_TYPE_POTION]) {
        return 0;
    }
    potion = &world->things->potions[potionIndex];
    rawPotion = world->things->rawThingData[THING_TYPE_POTION] +
        (size_t)potionIndex * 4u;
    rawNext = (unsigned short)(rawPotion[0] | ((unsigned short)rawPotion[1] << 8));
    if (rawNext != potion->next || rawPotion[2] != potion->power ||
        (rawPotion[3] & 0x7fu) != potion->type ||
        ((rawPotion[3] >> 7) & 0x01u) != potion->doNotDiscard ||
        projectile->associatedPotionPower != potion->power ||
        explosionIn->attack != potion->power) {
        return 0;
    }

    if (potion->type == DM1_POTION_VEN_PC34) {
        if (projectile->projectileSubtype != PROJECTILE_SUBTYPE_POISON_CLOUD ||
            explosionIn->explosionType != C007_EXPLOSION_POISON_CLOUD ||
            explosionIn->centered != 1 ||
            explosionIn->cell != EXPLOSION_CELL_CENTERED) {
            return 0;
        }
        /* F0213 stores centered state in the C15 high bit; C.Slot remains
         * a legal square cell rather than the runtime-only 0xFF sentinel. */
        *outC15Cell = 0;
        return 1;
    }
    if (potion->type == DM1_POTION_FUL_BOMB_PC34) {
        if (projectile->projectileSubtype != PROJECTILE_SUBTYPE_FIREBALL ||
            explosionIn->explosionType != C000_EXPLOSION_FIREBALL ||
            explosionIn->centered != 0 || explosionIn->cell < 0 ||
            explosionIn->cell > 3) {
            return 0;
        }
        *outC15Cell = explosionIn->cell;
        return 1;
    }
    return 0;
}

static int orch_materialize_projectile_tick_explosion_compat(
    struct GameWorld_Compat* world,
    const struct ProjectileInstance_Compat* projectile,
    int projectileIndex,
    const struct ProjectileTickResult_Compat* tickResult)
{
    struct ExplosionCreateInput_Compat explosionIn;
    struct TimelineEvent_Compat firstExplosionAdvance;
    struct ExplosionList_Compat explosionsBefore;
    struct TimelineQueue_Compat timelineBefore;
    DM1_C15PoolReservationPc34 reservation;
    DM1_C15C25PublicationReceiptPc34 publication;
    int explosionSlot = -1;
    int c15Cell;
    int sourceOwnedPotion;

    if (!world || !tickResult || !tickResult->emittedExplosion) return 0;

    if (!dm1_v1_projectile_explosion_create_input_pc34(
            tickResult, (int)world->gameTick, &explosionIn)) {
        return 0;
    }

    sourceOwnedPotion = projectile &&
        (projectile->flags & PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT);
    if (sourceOwnedPotion) {
        if (!orch_validate_f0217_thrown_potion_receipt_compat(
                world, projectile, projectileIndex, tickResult, &explosionIn,
                &c15Cell)) {
            return 0;
        }
    } else {
        /* A centered C15 still has a real square-cell owner in PC34's
         * C25 union.  The runtime-only 0xff sentinel is never exported. */
        c15Cell = explosionIn.centered ? 0 : explosionIn.cell;
    }

    /* F0213/F0220 must not synthesize a C25 from an in-memory explosion when
     * a loaded PC34 world is available.  Every impact family instead reserves
     * a genuine C15 row, binds its C25 location/slot receipt, then publishes
     * the runtime record and the first advance event atomically. */
    if (world->things && world->things->loaded) {
        if (!world->dungeon || world->gameTick >= 0x00ffffffu ||
            !dm1_v1_c15_pool_reserve_pc34(world->things, &reservation) ||
            !dm1_v1_c15_c25_publish_pc34(
                &reservation, world->dungeon, explosionIn.explosionType,
                explosionIn.attack, explosionIn.centered, c15Cell,
                explosionIn.mapIndex, explosionIn.mapX, explosionIn.mapY,
                world->gameTick + 1u, 0, &publication)) {
            return 0;
        }
        explosionsBefore = world->explosions;
        timelineBefore = world->timeline;
        explosionIn.cell = c15Cell;
        if (!F0213_EXPLOSION_Create_Compat(
                &explosionIn, &world->explosions, &explosionSlot,
                &firstExplosionAdvance) ||
            firstExplosionAdvance.fireAtTick != (publication.mapTime & 0x00ffffffu) ||
            firstExplosionAdvance.mapIndex != (int)(publication.mapTime >> 24) ||
            firstExplosionAdvance.mapX != publication.mapX ||
            firstExplosionAdvance.mapY != publication.mapY ||
            firstExplosionAdvance.cell != c15Cell) {
            world->explosions = explosionsBefore;
            world->timeline = timelineBefore;
            (void)dm1_v1_c15_pool_rollback_pc34(&reservation);
            return 0;
        }
        firstExplosionAdvance.aux3 = (int)publication.c15Fingerprint;
        firstExplosionAdvance.aux4 = publication.priority;
        if (!F0721_TIMELINE_Schedule_Compat(
                &world->timeline, &firstExplosionAdvance)) {
            world->explosions = explosionsBefore;
            world->timeline = timelineBefore;
            (void)dm1_v1_c15_pool_rollback_pc34(&reservation);
            return 0;
        }
        return 1;
    }

    /* Headless memory-only tests have no PC34 Thing pool to authenticate.
     * They retain the isolated F0213 primitive; a loaded game never reaches
     * this fallback. */
    if (F0213_EXPLOSION_Create_Compat(
            &explosionIn, &world->explosions, &explosionSlot,
            &firstExplosionAdvance)) {
        (void)F0721_TIMELINE_Schedule_Compat(
            &world->timeline, &firstExplosionAdvance);
        return 1;
    }
    return 0;
}

static void orch_sync_live_projectile_c14_f0219_compat(
    struct GameWorld_Compat* world,
    const struct ProjectileInstance_Compat* projectile)
{
    struct DungeonProjectile_Compat* source;
    int projectileIndex;

    if (!world || !projectile || !world->things ||
        !world->things->projectiles) {
        return;
    }
    projectileIndex = projectile->slotIndex;
    if (projectileIndex < 0 ||
        projectileIndex >= world->things->projectileCount) {
        return;
    }

    /* ReDMCSB PROJEXPL.C F0219:687-714 mutates PROJECTILE.KineticEnergy
     * and PROJECTILE.Attack before it relinks and requeues the C49 event.
     * The M10 runtime projection owns those values while the original C14
     * array remains the F0802 save source; keep the decoded source record
     * coherent after a successful flight step.  EVENT index ownership stays
     * with F0802's reconstructed EVENTS/TIMELINE heap because TimelineQueue
     * deliberately has no PC34 slot allocator. */
    source = &world->things->projectiles[projectileIndex];
    source->kineticEnergy = (unsigned char)projectile->kineticEnergy;
    source->attack = (unsigned char)projectile->attack;
}

/* F0214 normally removes a queued C48/C49 by its C14 EventIndex. When the
 * dispatcher has already popped that exact event, F0217 still retires the
 * same C14 owner before F0215 deletes or materializes its associated Thing. */
static int orch_retire_dispatched_projectile_event_owner_compat(
    struct GameWorld_Compat* world,
    int projectileIndex)
{
    struct DungeonProjectile_Compat* source;

    if (!world || !world->things || !world->things->projectiles) return 1;
    if (projectileIndex < 0 || projectileIndex >= world->things->projectileCount) {
        return 0;
    }
    source = &world->things->projectiles[projectileIndex];
    source->eventIndex = 0xFFFFu;
    return orch_write_raw_projectile_f0219_compat(world->things, projectileIndex);
}

/* ReDMCSB F0217 publishes the C02 door follow-up before its F0215 C14
 * cleanup. Keep that door route explicit so a rejected C02 receipt cannot
 * consume the source projectile owner. */
static int orch_schedule_projectile_door_impact_followup_compat(
    struct GameWorld_Compat* world,
    const struct ProjectileTickResult_Compat* tickResult)
{
    if (!world || !tickResult) return 0;
    if (!tickResult->emittedDoorDestructionEvent &&
        !tickResult->emittedDoorToggleEvent) {
        return 1;
    }
    if (tickResult->outNextTick.kind == TIMELINE_EVENT_INVALID ||
        tickResult->outNextTick.kind == 0) {
        return 0;
    }
    return F0721_TIMELINE_Schedule_Compat(
        &world->timeline, &tickResult->outNextTick);
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
    DM1_ProjectileFlightRelinkReceiptPc34 relinkReceipt;
    int projectileIndex;
    int associatedThingMovedToGroup = 0;

    if (!world || !event) return 0;
    projectileIndex = event->aux0;
    if (projectileIndex < 0 || projectileIndex >= PROJECTILE_LIST_CAPACITY) return 0;
    projectile = &world->projectiles.entries[projectileIndex];
    if (projectile->slotIndex < 0 || projectile->reserved3 == 0) return 1;
    if (world->things && !orch_validate_raw_projectile_f0219_compat(
            world->things, projectileIndex)) {
        return 0;
    }
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

    if (!F0219_PROJECTILE_ProcessEvents48To49_Compat(
            &projectileForAdvance, &digest, world->gameTick, &world->masterRng,
            &newState, &tickResult)) {
        F0813_PROJECTILE_Despawn_Compat(&world->projectiles, projectileIndex);
        return 1;
    }
    if (!tickResult.despawn &&
        !orch_projectile_move_schedule_admissible_f0219_compat(
            world, projectile->slotIndex, &tickResult.outNextTick)) {
        return 0;
    }

    /* F0219 has consumed this C48/C49 slot for the current dispatch pass.
     * Publish the one-frame ownership receipt before either the impact or
     * flight branch mutates the live C14 projection. */
    world->pc34M10ProjectileDispatchMask |= UINT64_C(1) << projectileIndex;
    world->pc34M10ProjectileDispatchTick = world->gameTick;

    if (tickResult.despawn) {
        if (!orch_schedule_projectile_door_impact_followup_compat(
                world, &tickResult)) {
            return 0;
        }
        if (!orch_retire_dispatched_projectile_event_owner_compat(
                world, projectileIndex)) {
            return 0;
        }
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
                world, projectile, projectileIndex, &tickResult);
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
                        world, other, otherIndex, &peerImpact);
                    if (peerImpact.emittedSoundRequest) {
                        emit(result, EMIT_SOUND_REQUEST,
                             peerImpact.emittedSoundCode,
                             other->mapX, other->mapY, other->mapIndex);
                    }
                }
                (void)orch_materialize_projectile_associated_thing_compat(
                    world, other, &peerImpact, 0);
                /* The runtime projection's slotIndex is the raw C14 owner;
                 * F0214 must consume that owner's EventIndex, never every
                 * C49 that happens to carry otherIndex in aux0. */
                (void)orch_delete_projectile_event_f0214_compat(
                    world, other->slotIndex);
                (void)F0813_PROJECTILE_Despawn_Compat(
                    &world->projectiles, otherIndex);
            }
        }
        (void)orch_materialize_projectile_associated_thing_compat(
            world, projectile, &tickResult, associatedThingMovedToGroup);
        F0813_PROJECTILE_Despawn_Compat(&world->projectiles, projectileIndex);
        return 1;
    }

    memset(&relinkReceipt, 0, sizeof(relinkReceipt));
    if (!dm1_v1_projectile_flight_relink_receipt_f0219_pc34(
            projectile, &newState, &tickResult, &relinkReceipt) ||
        !relinkReceipt.valid || !relinkReceipt.shouldApply) {
        return 0;
    }
    if (relinkReceipt.shouldUnlinkSourceSquare) {
        (void)orch_unlink_thing_from_square_compat(
            world,
            relinkReceipt.sourceMapIndex,
            relinkReceipt.sourceMapX,
            relinkReceipt.sourceMapY,
            relinkReceipt.sourceProjectileThing);
    }
    if (relinkReceipt.shouldLinkDestinationSquare) {
        (void)orch_link_thing_to_square_tail_compat(
            world,
            relinkReceipt.destinationMapIndex,
            relinkReceipt.destinationMapX,
            relinkReceipt.destinationMapY,
            relinkReceipt.destinationProjectileThing);
    }
    if (relinkReceipt.shouldWriteProjectileState) {
        /* ReDMCSB PROJEXPL.C:F0219 lines 735-762 commits the live
         * projectile's moved cell/square/direction/energy fields before
         * requeueing its C48/C49 move event.  The field set is now selected
         * by the DM1 flight relink receipt rather than by this M10 adapter. */
        *projectile = newState;
        projectile->scheduledAtTick = (int)tickResult.outNextTick.fireAtTick;
        if (!world->things || !world->things->projectiles ||
            projectileIndex < 0 || projectileIndex >= world->things->projectileCount) {
            return 0;
        }
        world->things->projectiles[projectileIndex].kineticEnergy =
            (unsigned char)newState.kineticEnergy;
        world->things->projectiles[projectileIndex].attack =
            (unsigned char)newState.attack;
        if (!orch_write_raw_projectile_f0219_compat(
                world->things, projectileIndex)) {
            return 0;
        }
    }
    if (relinkReceipt.shouldScheduleNextMove) {
        if (!orch_schedule_projectile_move_f0219_compat(
                world, projectile->slotIndex, &tickResult.outNextTick)) {
            return 0;
        }
    }
    return 1;
}

static int orch_apply_projectile_champion_action_compat(
    struct GameWorld_Compat* world,
    const struct CombatAction_Compat* action,
    const struct ProjectileInstance_Compat* projectile,
    struct TickResult_Compat* result)
{
    struct CombatantChampionSnapshot_Compat defender;
    struct ChampionState_Compat* champion;
    int championIndex;
    DM1_ProjectileChampionImpactPlanPc34 impactPlan;
    DM1_ProjectileChampionDamageApplyPlanPc34 damagePlan;

    if (!world || !action) return 0;
    if (!dm1_v1_projectile_champion_action_plan_pc34(
            projectile, action, 1, &impactPlan) ||
        !impactPlan.handled) {
        return 0;
    }
    if (impactPlan.rawAttackValue <= 0) return 0;
    championIndex = impactPlan.championIndex;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champion = &world->party.champions[championIndex];
    if (!champion->present || champion->hp.current == 0) return 0;

    if (!orch_build_defender_champion_snapshot_compat(
            world, championIndex, impactPlan.attackTypeCode, &defender) ||
        !dm1_v1_projectile_champion_damage_apply_pc34(
            &impactPlan, &defender, &world->masterRng, champion,
            &damagePlan)) {
        return 0;
    }
    if (damagePlan.scaledAttack <= 0) {
        return 1;
    }
    if (damagePlan.killed) {
        emit(result, EMIT_CHAMPION_DOWN, championIndex, 0, 0, 0);
    }

    if (!damagePlan.killed && projectile && projectile->poisonAttack > 0) {
        DM1_ProjectileChampionPoisonApplyPlanPc34 poisonApply;

        if (!dm1_v1_projectile_champion_poison_apply_pc34(
                &impactPlan, projectile, damagePlan.damage.damageApplied,
                F0732_COMBAT_RngRandom_Compat(&world->masterRng, 2),
                world->gameTick, impactPlan.impactMapIndex,
                impactPlan.impactMapX, impactPlan.impactMapY,
                champion, &poisonApply)) {
            return 0;
        }
        if (!poisonApply.shouldApply) {
            return 1;
        }
        if (poisonApply.schedulePoisonEvent) {
            int nextPoisonEventCount = 0;
            (void)F0721_TIMELINE_Schedule_Compat(
                &world->timeline, &poisonApply.poisonEvent);
            if (!dm1_v1_projectile_champion_poison_event_count_after_pc34(
                    &poisonApply,
                    world->lifecycle.champions[championIndex].poisonEventCount,
                    &nextPoisonEventCount)) {
                return 0;
            }
            world->lifecycle.champions[championIndex].poisonEventCount =
                (uint8_t)nextPoisonEventCount;
        }
        if (poisonApply.championDown) {
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

static int orch_maybe_attach_projectile_weapon_to_group_slot_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const struct ProjectileInstance_Compat* projectile,
    int damageOutcome)
{
    const struct CreatureBehaviorProfile_Compat* profile;
    DM1_ProjectileGroupSlotMaterializationPlanPc34 plan;
    DM1_ProjectileGroupSlotAttachReceiptPc34 attachReceipt;
    unsigned short chainThings[64];
    unsigned short associatedThing;
    unsigned short current;
    int chainCount = 0;
    int weaponIndex;
    int weaponType;

    if (!world || !group || !projectile || !world->things) return 0;

    associatedThing = (unsigned short)projectile->reserved1;
    if (associatedThing == THING_NONE || associatedThing == THING_ENDOFLIST) return 0;
    if (THING_GET_TYPE(associatedThing) != THING_TYPE_WEAPON) return 0;

    profile = CREATURE_GetProfile_Compat((int)group->creatureType);

    weaponIndex = THING_GET_INDEX(associatedThing);
    if (!world->things->weapons ||
        weaponIndex < 0 ||
        weaponIndex >= world->things->weaponCount) {
        return 0;
    }
    weaponType = (int)world->things->weapons[weaponIndex].type;
    memset(&plan, 0, sizeof(plan));
    if (!dm1_v1_projectile_group_slot_materialization_plan_pc34(
            projectile, damageOutcome,
            profile ? profile->attributes : 0,
            weaponType, &plan) ||
        !plan.valid || !plan.shouldAttachToGroupSlot) {
        return 0;
    }

    /* ReDMCSB PROJEXPL.C:F0217 lines 540-553 selects GROUP.Slot as the
     * projectile-delete target for non-exploding sharp weapon projectiles
     * that survive impact against KEEP_THROWN_SHARP_WEAPONS creatures.
     * F0215 lines 248-256 then uses DUNGEON.C:F0163 lines 1798-1837:
     * empty possession lists get the thrown weapon as head; existing lists
     * keep their head and append the thrown weapon at the tail. */
    current = group->slot;
    while (current != THING_NONE &&
           current != THING_ENDOFLIST &&
           chainCount < (int)(sizeof(chainThings) / sizeof(chainThings[0]))) {
        chainThings[chainCount++] = current;
        current = orch_next_thing_compat(world->things, current);
    }
    if (chainCount < (int)(sizeof(chainThings) / sizeof(chainThings[0]))) {
        chainThings[chainCount++] = current;
    }

    memset(&attachReceipt, 0, sizeof(attachReceipt));
    if (!dm1_v1_projectile_group_slot_attach_receipt_f0215_pc34(
            associatedThing, group->slot, chainThings, chainCount,
            &attachReceipt) ||
        !attachReceipt.valid ||
        !attachReceipt.shouldSetAssociatedNextEnd ||
        attachReceipt.chainOverflow ||
        !orch_set_next_thing_compat(
            world->things, attachReceipt.associatedThing,
            THING_ENDOFLIST)) {
        return 0;
    }
    if (attachReceipt.shouldSetGroupSlotHead) {
        group->slot = attachReceipt.associatedThing;
        return 1;
    }
    if (attachReceipt.shouldAppendAfterTail && attachReceipt.foundTail) {
        return orch_set_next_thing_compat(
            world->things, attachReceipt.tailThing,
            attachReceipt.associatedThing);
    }
    return 0;
}

static int orch_apply_projectile_group_action_compat(
    struct GameWorld_Compat* world,
    const struct CombatAction_Compat* action,
    const struct ProjectileInstance_Compat* projectile,
    struct TickResult_Compat* result)
{
    struct DungeonGroup_Compat* group;
    struct CombatantCreatureSnapshot_Compat creatureSnapshot;
    int groupIndex = -1;
    int creatureIndex;
    int killedCell;
    int originalCreatureType;
    int originalGroupCount;
    int associatedThingMovedToGroup = 0;
    int creatureAttributes = 0;
    int associatedWeaponType = -1;
    DM1_ProjectileCreatureActionPlanPc34 actionPlan;
    DM1_ProjectileCreatureActionApplyPlanPc34 applyPlan;
    DM1_ProjectileCreatureImpactAftermathPc34 aftermath;
    DM1_MeleeF0231AftermathInputPc34 f0231AftermathIn;
    DM1_MeleeF0231AftermathPlanPc34 f0231AftermathPlan;
    DM1_MeleeF0231AftermathApplyPlanPc34 f0231ApplyPlan;
    DM1_MeleeF0190KilledAllAfterplayReceiptPc34 killedAllAfterplay;

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
    originalCreatureType = (int)group->creatureType;
    associatedWeaponType =
        orch_projectile_associated_weapon_type_compat(world, projectile);
    {
        const struct CreatureBehaviorProfile_Compat* profile =
            CREATURE_GetProfile_Compat(originalCreatureType);
        if (profile) creatureAttributes = profile->attributes;
    }
    memset(&actionPlan, 0, sizeof(actionPlan));
    if (!dm1_v1_projectile_creature_action_plan_pc34(
            projectile, action, group, creatureAttributes, &actionPlan) ||
        !actionPlan.handled) {
        return 0;
    }
    if (actionPlan.blockedByNonMaterial) {
        return 1;
    }
    if (actionPlan.healsBlackFlame) {
        group->health[actionPlan.slotIndex] =
            (unsigned short)actionPlan.newHealth;
        orch_write_raw_group_compat(world->things, groupIndex);
        return 1;
    }
    if (!actionPlan.shouldApplyDamage || actionPlan.slotIndex < 0) {
        return 0;
    }
    creatureIndex = actionPlan.slotIndex;
    killedCell = actionPlan.killedCell;
    originalGroupCount = actionPlan.originalGroupCount;
    memset(&creatureSnapshot, 0, sizeof(creatureSnapshot));
    if (!F0888_ORCH_GetCreatureSnapshot_Compat(
            world, groupIndex, creatureIndex, 0, &creatureSnapshot)) {
        const struct CreatureBehaviorProfile_Compat* profile =
            CREATURE_GetProfile_Compat(originalCreatureType);
        creatureSnapshot.creatureType = originalCreatureType;
        if (profile) {
            creatureSnapshot.attributes = profile->attributes;
            creatureSnapshot.properties = profile->properties;
        }
    }

    if (!dm1_v1_projectile_creature_action_apply_pc34(
            &actionPlan, group, &applyPlan) ||
        !applyPlan.handled) {
        return 0;
    }
    memset(&aftermath, 0, sizeof(aftermath));
    if (!dm1_v1_projectile_creature_action_aftermath_pc34(
            &actionPlan, projectile, creatureAttributes, (int)group->behavior,
            applyPlan.outcomeCode, associatedWeaponType, &aftermath)) {
        return 0;
    }
    if (applyPlan.outcomeCode == COMBAT_OUTCOME_KILLED_SOME_CREATURES ||
        applyPlan.outcomeCode == COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
        memset(&f0231AftermathIn, 0, sizeof(f0231AftermathIn));
        memset(&f0231AftermathPlan, 0, sizeof(f0231AftermathPlan));
        memset(&f0231ApplyPlan, 0, sizeof(f0231ApplyPlan));
        f0231AftermathIn.groupIndex = groupIndex;
        f0231AftermathIn.creatureIndex = creatureIndex;
        f0231AftermathIn.creatureType = originalCreatureType;
        f0231AftermathIn.creatureAttributes = creatureSnapshot.attributes;
        f0231AftermathIn.creatureProperties = creatureSnapshot.properties;
        f0231AftermathIn.groupBehavior = (int)group->behavior;
        f0231AftermathIn.originalGroupCount = originalGroupCount;
        f0231AftermathIn.partyMapIndex = world->partyMapIndex;
        f0231AftermathIn.partyMapX = world->party.mapX;
        f0231AftermathIn.partyMapY = world->party.mapY;
        f0231AftermathIn.targetMapIndex = action->targetMapIndex;
        f0231AftermathIn.targetMapX = action->targetMapX;
        f0231AftermathIn.targetMapY = action->targetMapY;
        f0231AftermathIn.currentTick = world->gameTick;
        f0231AftermathIn.killedCell = killedCell;
        f0231AftermathIn.damageOutcome = applyPlan.outcomeCode;
        f0231AftermathIn.fallbackCombatOutcome = applyPlan.outcomeCode;
        (void)dm1_v1_melee_aftermath_plan_f0231_pc34(
            &f0231AftermathIn, &f0231AftermathPlan);
        (void)dm1_v1_melee_aftermath_apply_plan_f0231_pc34(
            &f0231AftermathPlan, &f0231ApplyPlan);
        memset(&killedAllAfterplay, 0, sizeof(killedAllAfterplay));
        (void)dm1_v1_melee_killed_all_afterplay_receipt_f0190_pc34(
            &f0231ApplyPlan, &killedAllAfterplay);
        if (f0231ApplyPlan.shouldApplyMutationDispatch) {
            (void)orch_cmd_attack_apply_f0190_mutation_dispatch_compat(
                world, group, &f0231ApplyPlan.mutationDispatchPlan);
        }
        if (killedAllAfterplay.shouldPresentSourceSmoke) {
            struct TimelineEvent_Compat advance;
            int slotIndex = -1;
            memset(&advance, 0, sizeof(advance));
            if (F0821_EXPLOSION_Create_Compat(
                    &killedAllAfterplay.sourceSmokeCreateInput,
                    &world->explosions,
                    &slotIndex, &advance)) {
                (void)F0721_TIMELINE_Schedule_Compat(
                    &world->timeline, &advance);
            }
        } else if (f0231ApplyPlan.shouldCreateDeathSmoke) {
            struct TimelineEvent_Compat advance;
            int slotIndex = -1;
            memset(&advance, 0, sizeof(advance));
            if (F0213_EXPLOSION_Create_Compat(
                    &f0231ApplyPlan.smokeCreateInput, &world->explosions,
                    &slotIndex, &advance)) {
                (void)F0721_TIMELINE_Schedule_Compat(
                    &world->timeline, &advance);
            }
        }
        if (f0231ApplyPlan.shouldEmitKillNotify) {
            emit(result, EMIT_KILL_NOTIFY,
                 f0231ApplyPlan.killNotifyGroupIndex,
                 f0231ApplyPlan.killNotifyCreatureIndex,
                 f0231ApplyPlan.killNotifyOutcome,
                 f0231ApplyPlan.killNotifyCreatureType);
        }
        if (applyPlan.outcomeCode != COMBAT_OUTCOME_KILLED_ALL_CREATURES &&
            aftermath.scheduleReaction) {
            orch_schedule_group_reaction_compat(
                world, groupIndex, group, action,
                DM1_CM2_REACTION_HIT_BY_PROJECTILE);
        }
    } else {
        if (aftermath.keepSharpWeaponInGroup) {
            if (orch_maybe_attach_projectile_weapon_to_group_slot_compat(
                    world, group, projectile, applyPlan.outcomeCode)) {
                associatedThingMovedToGroup = 1;
            }
        }
        if (aftermath.scheduleReaction) {
            orch_schedule_group_reaction_compat(
                world, groupIndex, group, action,
                DM1_CM2_REACTION_HIT_BY_PROJECTILE);
        }
    }
    /* ReDMCSB GROUP.C:F0190 lines 892-917 mutates the live group record
     * after projectile damage: surviving groups carry compacted HP/cells and
     * all-kill groups are unlinked.  Mirror those decoded changes into the
     * raw DUNGEON.DAT record used by save/export and later raw inspections. */
    orch_write_raw_group_compat(world->things, groupIndex);
    return associatedThingMovedToGroup ? 2 : 1;
}

static void orch_schedule_group_reaction_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    const struct DungeonGroup_Compat* group,
    const struct CombatAction_Compat* action,
    int reactionKind)
{
    int activeIndex;
    int ticksSinceLastMove = 0;
    const struct CreatureBehaviorProfile_Compat* profile;
    struct DM1GroupBehaviorContext_Compat ctx;
    struct DM1ActiveGroup_Compat activeGroup;
    struct DM1BehaviorResult_Compat behavior;
    struct DM1BehaviorReactionSchedulePlan_Compat schedulePlan;
    struct TimelineEvent_Compat reaction;

    if (!world || !group || !action || groupIndex < 0) return;

    activeIndex = orch_find_active_group_state_index_compat(world, groupIndex);
    profile = CREATURE_GetProfile_Compat((int)group->creatureType);

    memset(&ctx, 0, sizeof(ctx));
    memset(&activeGroup, 0, sizeof(activeGroup));
    memset(&behavior, 0, sizeof(behavior));
    memset(&schedulePlan, 0, sizeof(schedulePlan));

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
    ctx.eventType = reactionKind;
    ctx.eventTicks = (int)world->gameTick;

    activeGroup.groupThingIndex = groupIndex;
    activeGroup.cells = group->cells;
    activeGroup.directions = (activeIndex >= 0)
        ? orch_active_group_directions_compat(&world->creatureAI[activeIndex], group)
        : orch_pack_group_directions_compat(group->direction, (int)group->count);
    activeGroup.lastMoveTime = (activeIndex >= 0)
        ? world->creatureAI[activeIndex].lastSeenPartyTick : 0;
    activeGroup.priorMapX = action->targetMapX;
    activeGroup.priorMapY = action->targetMapY;

    /* ReDMCSB GROUP.C:F0209 creates a concrete C29-C31 reaction from the
     * source event kind.  Projectile impacts pass CM2; F0241 door hazards
     * pass CM3 after F0191 leaves a survivor. */
    if (!F0810_DM1_GROUP_DispatchBehavior_Compat(
            &ctx, &activeGroup, &world->masterRng, &behavior)) {
        return;
    }
    if (!F0810c_DM1_GROUP_PlanReactionSchedule_Compat(
            &behavior, groupIndex, (int)group->creatureType,
            action->targetMapIndex, action->targetMapX, action->targetMapY,
            world->gameTick, &schedulePlan) ||
        !schedulePlan.shouldSchedule) {
        return;
    }

    memset(&reaction, 0, sizeof(reaction));
    reaction.kind = TIMELINE_EVENT_CREATURE_REACTION;
    reaction.fireAtTick = schedulePlan.fireAtTick;
    reaction.mapIndex = schedulePlan.mapIndex;
    reaction.mapX = schedulePlan.mapX;
    reaction.mapY = schedulePlan.mapY;
    reaction.aux0 = schedulePlan.groupIndex;
    reaction.aux1 = schedulePlan.creatureType;
    reaction.aux2 = schedulePlan.eventType;
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
    if (world->things && world->things->loaded &&
        !orch_c25_event_source_bound_compat(world, event)) {
        return 0;
    }
    explosionSlot = event->aux0;
    if (explosionSlot < 0 || explosionSlot >= EXPLOSION_LIST_CAPACITY) return 0;
    explosion = &world->explosions.entries[explosionSlot];
    if (explosion->slotIndex < 0 || explosion->reserved0 == 0) return 1;
    if (!orch_build_explosion_digest_compat(world, explosion, &digest)) {
        return 0;
    }
    if (!F0220_EXPLOSION_ProcessEvent25_Compat(
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
    DM1_ExplosionPartyDamageFanoutPlanPc34 fanoutPlan;
    int i;
    int applied = 0;

    if (!world || !action) return 0;
    if (!dm1_v1_explosion_party_damage_fanout_plan_pc34(
            action->rawAttackValue,
            action->attackTypeCode,
            action->allowedWounds,
            &fanoutPlan)) {
        return 0;
    }
    if (!fanoutPlan.handled) return 0;

    /* ReDMCSB PROJEXPL.C:F0213 line 173 and F0220 line 861 route
     * party-square fireball/lightning and poison-cloud explosions through
     * CHAMPION.C:F0324.  F0324 randomizes attack per champion by +/- 1/8
     * and then calls F0321.  DM1 owns the F0324 fanout plan; M10 applies
     * the live F0321 shield/defense/wound mutation. */
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        DM1_ExplosionPartyChampionDamagePlanPc34 championPlan;
        DM1_ExplosionPartyChampionApplyPlanPc34 applyPlan;
        struct CombatantChampionSnapshot_Compat defender;
        struct ChampionState_Compat* champion = &world->party.champions[i];

        if (!champion->present || champion->hp.current == 0) {
            continue;
        }
        if (!dm1_v1_explosion_party_champion_damage_plan_pc34(
                &fanoutPlan, i, champion->present, champion->hp.current,
                F0732_COMBAT_RngRandom_Compat(&world->masterRng,
                                              fanoutPlan.rngModulus),
                &championPlan)) {
            return 0;
        }
        if (!championPlan.shouldAttemptDamage) continue;

        if (!orch_build_defender_champion_snapshot_compat(
                world, i, championPlan.attackTypeCode, &defender) ||
            !dm1_v1_explosion_party_champion_apply_pc34(
                &championPlan, &defender, &world->masterRng, champion,
                &applyPlan) ||
            applyPlan.scaledAttack <= 0) {
            continue;
        }
        if (applyPlan.valid) {
            applied++;
            if (applyPlan.killed) emit(result, EMIT_CHAMPION_DOWN, i, 0, 0, 0);
        }
    }
    return applied > 0;
}

static int orch_apply_explosion_group_action_compat(
    struct GameWorld_Compat* world,
    const struct CombatAction_Compat* action)
{
    struct DungeonGroup_Compat* group;
    DM1_ExplosionGroupApplyPlanPc34 applyPlan;
    int groupIndex = -1;

    if (!world || !action || !world->things || !world->things->groups) return 0;
    if (!orch_cmd_attack_find_group_on_square_compat(
            world, action->targetMapIndex, action->targetMapX,
            action->targetMapY, &groupIndex)) {
        return 0;
    }
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;
    group = &world->things->groups[groupIndex];

    if (!dm1_v1_explosion_group_apply_pc34(action, group, &applyPlan)) {
        return 0;
    }
    return applyPlan.handled && applyPlan.appliedCount > 0;
}

/* ReDMCSB TIMELINE.C F0241:761-770 calls CHAMPION.C F0324 directly when
 * a closing door catches the party.  The F0324/F0321 owner is shared with
 * party-square explosions because its fanout and mutation contract is the
 * same; this wrapper keeps the door's C2_ATTACK_SELF source identity local. */
static int orch_apply_door_party_damage_f0324_compat(
    struct GameWorld_Compat* world,
    int attack,
    int wounds,
    struct TickResult_Compat* result)
{
    struct CombatAction_Compat action;
    if (!world) return 0;
    memset(&action, 0, sizeof(action));
    action.kind = COMBAT_ACTION_APPLY_DAMAGE_CHAMPION;
    action.rawAttackValue = attack;
    action.allowedWounds = wounds;
    action.attackTypeCode = COMBAT_ATTACK_SELF;
    return orch_apply_explosion_party_action_compat(world, &action, result);
}

/* ReDMCSB TIMELINE.C F0241:783 calls GROUP.C F0191.  Its attack is
 * randomized independently for each creature before the F0190/F0738 slot
 * mutation; descending traversal is required because a killed slot compacts
 * the remaining group entries. */
static int orch_apply_door_group_damage_f0191_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    int mapIndex,
    int mapX,
    int mapY,
    int attack,
    int* outKilledAll)
{
    struct CombatResult_Compat damage;
    struct DungeonGroup_Compat* group;
    struct DM1CreatureInfo_Compat creatureInfo;
    int baseAttack;
    int rngModulus;
    int creatureIndex;
    int outcome = COMBAT_OUTCOME_KILLED_NO_CREATURES;
    int killedAll = 0;

    if (outKilledAll) *outKilledAll = 0;
    if (!world || !world->things || !world->things->groups ||
        groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;
    group = &world->things->groups[groupIndex];
    if (attack <= 0) return 1;
    memset(&creatureInfo, 0, sizeof(creatureInfo));
    if (!orch_get_dm1_creature_info_pc34_compat(
            (int)group->creatureType, &creatureInfo)) {
        return 0;
    }
    baseAttack = attack - ((attack >> 3) + 1);
    rngModulus = ((attack >> 3) + 1) << 1;
    for (creatureIndex = (int)group->count;
         creatureIndex >= 0 && creatureIndex < 4;
         --creatureIndex) {
        DM1_MeleeF0190GroupDamageApplyPlanPc34 damageApplyPlan;
        memset(&damage, 0, sizeof(damage));
        damage.damageApplied = baseAttack +
            F0732_COMBAT_RngRandom_Compat(&world->masterRng, rngModulus);
        if (damage.damageApplied < 1) damage.damageApplied = 1;
        memset(&damageApplyPlan, 0, sizeof(damageApplyPlan));
        if (!dm1_v1_melee_apply_group_damage_plan_f0190_pc34(
                &damage, group, creatureIndex, &damageApplyPlan) ||
            !damageApplyPlan.valid || !damageApplyPlan.shouldApplyDamage) {
            return 0;
        }
        outcome = damageApplyPlan.outcome;
        if (outcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES ||
            outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
            DM1_MeleeF0190DeathSmokeInputPc34 smokeIn;
            DM1_MeleeF0190DeathSmokePlanPc34 smokePlan;
            DM1_MeleeF0190MutationDispatchInputPc34 dispatchIn;
            DM1_MeleeF0190MutationDispatchPlanPc34 dispatchPlan;
            struct TimelineEvent_Compat advance;
            int slotIndex = -1;

            /* ReDMCSB: GROUP.C F0191 lines 956-980 calls F0190 for each
             * descending slot.  F0190 lines 824-917 owns the per-kill
             * possessions, smoke, unlink, and active-group aftermath. */
            memset(&dispatchIn, 0, sizeof(dispatchIn));
            memset(&dispatchPlan, 0, sizeof(dispatchPlan));
            dispatchIn.outcome = outcome;
            dispatchIn.groupIndex = groupIndex;
            dispatchIn.groupBehavior = (int)group->behavior;
            dispatchIn.killedCreatureIndex = creatureIndex;
            dispatchIn.originalGroupCount = damageApplyPlan.originalGroupCount;
            dispatchIn.creatureType = (int)group->creatureType;
            dispatchIn.creatureAttributes = creatureInfo.attributes;
            dispatchIn.creatureProperties = creatureInfo.properties;
            dispatchIn.killedCell = damageApplyPlan.killedCell;
            dispatchIn.mapIndex = mapIndex;
            dispatchIn.mapX = mapX;
            dispatchIn.mapY = mapY;
            dispatchIn.partyMapIndex = world->partyMapIndex;
            dispatchIn.partyMapX = world->party.mapX;
            dispatchIn.partyMapY = world->party.mapY;
            if (!dm1_v1_melee_mutation_dispatch_plan_f0190_pc34(
                    &dispatchIn, &dispatchPlan) || !dispatchPlan.valid) {
                return 0;
            }
            (void)orch_cmd_attack_apply_f0190_mutation_dispatch_compat(
                world, group, &dispatchPlan);

            memset(&smokeIn, 0, sizeof(smokeIn));
            memset(&smokePlan, 0, sizeof(smokePlan));
            smokeIn.shouldCreate = 1;
            smokeIn.smokeAttack = dm1_v1_melee_death_smoke_attack_f0190_pc34(
                creatureInfo.attributes);
            smokeIn.smokeCell = damageApplyPlan.killedCell;
            smokeIn.mapIndex = mapIndex;
            smokeIn.mapX = mapX;
            smokeIn.mapY = mapY;
            smokeIn.currentTick = (int)world->gameTick;
            if (dm1_v1_melee_death_smoke_plan_f0190_pc34(
                    &smokeIn, &smokePlan) &&
                smokePlan.valid && smokePlan.shouldCreate) {
                memset(&advance, 0, sizeof(advance));
                if (F0213_EXPLOSION_Create_Compat(
                        &smokePlan.createInput, &world->explosions,
                        &slotIndex, &advance)) {
                    (void)F0721_TIMELINE_Schedule_Compat(
                        &world->timeline, &advance);
                }
            }
        }
        if (outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
            killedAll = 1;
            break;
        }
    }
    orch_write_raw_group_compat(world->things, groupIndex);
    if (outKilledAll) *outKilledAll = killedAll;
    (void)mapIndex;
    (void)mapX;
    (void)mapY;
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
    DM1_ProjectileSquareAttachReceiptPc34 attachReceipt;
    unsigned short chainThings[64];
    unsigned short current;
    int safety = 0;
    int chainCount = 0;

    if (!world || !world->dungeon || !world->things) return 0;
    if (thing == THING_NONE || thing == THING_ENDOFLIST) return 0;
    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;

    current = world->things->squareFirstThings[sftIndex];
    while (current != THING_NONE &&
           current != THING_ENDOFLIST &&
           chainCount < (int)(sizeof(chainThings) / sizeof(chainThings[0]))) {
        chainThings[chainCount++] = current;
        current = orch_next_thing_compat(world->things, current);
        ++safety;
        if (safety >= 64) break;
    }
    if (chainCount < (int)(sizeof(chainThings) / sizeof(chainThings[0]))) {
        chainThings[chainCount++] = current;
    }

    memset(&attachReceipt, 0, sizeof(attachReceipt));
    /* ReDMCSB MOVESENS.C:F0267 keeps C15 in the ordinary square chain;
     * TIMELINE.C:F0249 then updates only its C25 coordinate. C15 owns Next
     * directly, so F0215's C14/C05 projectile-drop receipt must not admit it. */
    if (THING_GET_TYPE(thing) == THING_TYPE_EXPLOSION) {
        if (!orch_set_next_thing_compat(
                world->things, thing, THING_ENDOFLIST)) {
            return 0;
        }
        orch_write_raw_next_compat(world->things, thing);
        if (world->things->squareFirstThings[sftIndex] == THING_NONE ||
            world->things->squareFirstThings[sftIndex] == THING_ENDOFLIST) {
            world->things->squareFirstThings[sftIndex] = thing;
            return 1;
        }
        if (current != THING_ENDOFLIST || chainCount < 2 ||
            !orch_set_next_thing_compat(
                world->things, chainThings[chainCount - 2], thing)) {
            return 0;
        }
        orch_write_raw_next_compat(world->things, chainThings[chainCount - 2]);
        return 1;
    }

    if (!dm1_v1_projectile_square_attach_receipt_f0215_pc34(
            thing, world->things->squareFirstThings[sftIndex],
            chainThings, chainCount, &attachReceipt) ||
        !attachReceipt.valid ||
        !attachReceipt.shouldSetDroppedNextEnd ||
        attachReceipt.chainOverflow) {
        return 0;
    }
    if (!orch_set_next_thing_compat(
            world->things, attachReceipt.baseThing, THING_ENDOFLIST)) {
        return 0;
    }
    orch_write_raw_next_compat(world->things, attachReceipt.baseThing);
    if (attachReceipt.shouldSetSquareFirstThing) {
        world->things->squareFirstThings[sftIndex] =
            attachReceipt.droppedThing;
        return 1;
    }
    if (attachReceipt.shouldAppendAfterTail && attachReceipt.foundTail) {
        if (!orch_set_next_thing_compat(
                world->things, attachReceipt.tailThing,
                attachReceipt.droppedThing)) {
            return 0;
        }
        orch_write_raw_next_compat(world->things, attachReceipt.tailThing);
        return 1;
    }
    return 0;
}

static int orch_f0267_thing_is_present_on_square_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned short needle)
{
    int sftIndex;
    unsigned short current;
    int safety = 0;

    if (!world || !world->dungeon || !world->things) return 0;
    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;
    current = world->things->squareFirstThings[sftIndex];
    while (current != THING_NONE && current != THING_ENDOFLIST && safety++ < 64) {
        if ((current & 0x3fffu) == (needle & 0x3fffu)) return 1;
        current = orch_next_thing_compat(world->things, current);
    }
    return 0;
}

static int orch_f0267_sensor_pass_count_compat(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned short movingThing,
    int isAddition)
{
    struct SensorOnSquare_Compat sensors[SENSOR_ENUM_CAPACITY];
    struct FloorSensorContext_Compat context;
    struct SensorTriggerResultList_Compat results;
    int type;
    int index;
    int objectType = -1;
    int sensorCount;

    if (!world || !world->dungeon || !world->things) return 0;
    type = THING_GET_TYPE(movingThing);
    index = THING_GET_INDEX(movingThing);
    if (type > THING_TYPE_GROUP && type < THING_TYPE_PROJECTILE &&
        index >= 0 && index < world->things->thingCounts[type] &&
        world->things->rawThingData[type] &&
        s_orch_thing_data_byte_count[type] >= 4) {
        objectType = (int)(r_u16(world->things->rawThingData[type] +
            (index * s_orch_thing_data_byte_count[type]) + 2) & 0x007fu);
    }

    memset(&context, 0, sizeof(context));
    context.mapX = mapX;
    context.mapY = mapY;
    context.thingType = type;
    context.objectType = objectType;
    context.partyOnSquare = (world->partyMapIndex == mapIndex &&
        world->party.mapX == mapX && world->party.mapY == mapY);
    context.partyChampionCount = world->party.championCount;
    context.isAddition = isAddition ? 1 : 0;
    sensorCount = F0717_SENSOR_EnumerateOnSquare_Compat(
        world->dungeon, world->things, mapIndex, mapX, mapY, sensors);
    if (sensorCount <= 0) return 0;
    memset(&results, 0, sizeof(results));
    if (!F0725_SENSOR_ProcessFloorSquare_Compat(
            sensors, sensorCount, world->things->sensors,
            world->things->sensorCount, &context, &results)) {
        return 0;
    }
    return results.count;
}

static void orch_f0267_dispatch_sensor_results_compat(
    struct GameWorld_Compat* world,
    const struct SensorTriggerResultList_Compat* results,
    int sourceMapIndex,
    int sourceMapX,
    int sourceMapY,
    struct F0267ThingMoveResultPc34Compat* moveResult)
{
    int i;

    if (!world || !results || !moveResult) return;
    /* ReDMCSB MOVESENS.C F0276 lines 1771-1785 sends each evaluated
     * remote result to F0268 in list order.  The common M10 route owns
     * the supported remote-event receipt; local rotation remains owned by
     * the corresponding party/group movement paths. */
    for (i = 0; i < results->count; ++i) {
        const struct SensorTriggerResult_Compat* trigger = &results->results[i];
        struct SensorTriggerResult_Compat resolved;
        struct SensorEffect_Compat* effect;
        const struct DungeonSensor_Compat* sensor;
        const struct DungeonMapDesc_Compat* map;
        int targetSquareType;
        int targetIndex;
        if (!trigger->triggered || !world->dungeon || !world->things ||
            !world->dungeon->maps ||
            !world->dungeon->tiles || sourceMapIndex < 0 ||
            sourceMapIndex >= (int)world->dungeon->header.mapCount ||
            trigger->sensorIndex < 0 ||
            trigger->sensorIndex >= world->things->sensorCount ||
            !world->things->sensors) {
            continue;
        }
        if (trigger->isLocal) {
            if (trigger->sensorDisabled) {
                world->things->sensors[trigger->sensorIndex].sensorType =
                    DM1_SENSOR_DISABLED;
            }
            continue;
        }
        map = &world->dungeon->maps[sourceMapIndex];
        if (!world->dungeon->tiles[sourceMapIndex].squareData ||
            trigger->targetMapX < 0 || trigger->targetMapX >= map->width ||
            trigger->targetMapY < 0 || trigger->targetMapY >= map->height) {
            continue;
        }
        targetIndex = trigger->targetMapX * map->height + trigger->targetMapY;
        targetSquareType = (world->dungeon->tiles[sourceMapIndex].squareData[
            targetIndex] & DUNGEON_SQUARE_MASK_TYPE) >> 5;
        sensor = &world->things->sensors[trigger->sensorIndex];
        memset(&resolved, 0, sizeof(resolved));
        if (!F0724_SENSOR_ResolveEffectDispatch_Compat(sensor,
                trigger->resolvedEffect, targetSquareType, sourceMapX,
                sourceMapY, &resolved) ||
            resolved.targetEventType == DM1_EVENT_NONE) {
            continue;
        }
        if (world->pendingSensorEffects.count >= SENSOR_EFFECT_LIST_MAX_COUNT) {
            moveResult->sensorDispatchOverflow = 1;
            continue;
        }
        effect = &world->pendingSensorEffects.effects[
            world->pendingSensorEffects.count++];
        memset(effect, 0, sizeof(*effect));
        effect->kind = SENSOR_EFFECT_TOGGLE_REMOTE;
        effect->sensorType = resolved.effectKind;
        effect->destMapIndex = -1;
        effect->destMapX = resolved.targetMapX;
        effect->destMapY = resolved.targetMapY;
        effect->destCell = resolved.targetCell;
        effect->textIndex = resolved.resolvedEffect;
        moveResult->sensorDispatches++;

        /* ReDMCSB MOVESENS.C F0268 queues the remote square event after
         * F0276 has evaluated the sensor. Remote floor sensors do not carry
         * a target map, so F0272 keeps the triggering map context. The
         * source's zero-delay form still waits one timeline tick. */
        if (resolved.targetEventType == DM1_EVENT_FAKEWALL ||
            resolved.targetEventType == DM1_EVENT_TELEPORTER ||
            resolved.targetEventType == DM1_EVENT_PIT ||
            resolved.targetEventType == DM1_EVENT_DOOR) {
            struct TimelineEvent_Compat event;
            memset(&event, 0, sizeof(event));
            event.kind = TIMELINE_EVENT_SQUARE_STATE;
            event.fireAtTick = world->gameTick +
                (uint32_t)(resolved.delayTicks > 0 ? resolved.delayTicks : 1);
            event.mapIndex = sourceMapIndex;
            event.mapX = resolved.targetMapX;
            event.mapY = resolved.targetMapY;
            event.cell = resolved.targetCell;
            event.aux0 = resolved.targetEventType;
            event.aux1 = resolved.resolvedEffect;
            if (!F0721_TIMELINE_Schedule_Compat(&world->timeline, &event)) {
                moveResult->sensorDispatchOverflow = 1;
            }
        }
    }
}

static int orch_f0267_sensor_pass_dispatch_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned short movingThing,
    int isAddition,
    struct F0267ThingMoveResultPc34Compat* moveResult)
{
    struct SensorOnSquare_Compat sensors[SENSOR_ENUM_CAPACITY];
    struct FloorSensorContext_Compat context;
    struct SensorTriggerResultList_Compat results;
    int type;
    int index;
    int objectType = -1;
    int sensorCount;

    if (!world || !world->dungeon || !world->things || !moveResult) return 0;
    type = THING_GET_TYPE(movingThing);
    index = THING_GET_INDEX(movingThing);
    if (type > THING_TYPE_GROUP && type < THING_TYPE_PROJECTILE &&
        index >= 0 && index < world->things->thingCounts[type] &&
        world->things->rawThingData[type] &&
        s_orch_thing_data_byte_count[type] >= 4) {
        objectType = (int)(r_u16(world->things->rawThingData[type] +
            (index * s_orch_thing_data_byte_count[type]) + 2) & 0x007fu);
    }
    memset(&context, 0, sizeof(context));
    context.mapX = mapX;
    context.mapY = mapY;
    context.thingType = type;
    context.objectType = objectType;
    context.partyOnSquare = (world->partyMapIndex == mapIndex &&
        world->party.mapX == mapX && world->party.mapY == mapY);
    context.partyChampionCount = world->party.championCount;
    context.isAddition = isAddition ? 1 : 0;
    sensorCount = F0717_SENSOR_EnumerateOnSquare_Compat(
        world->dungeon, world->things, mapIndex, mapX, mapY, sensors);
    if (sensorCount <= 0) return 0;
    memset(&results, 0, sizeof(results));
    if (!F0725_SENSOR_ProcessFloorSquare_Compat(
            sensors, sensorCount, world->things->sensors,
            world->things->sensorCount, &context, &results)) {
        return 0;
    }
    orch_f0267_dispatch_sensor_results_compat(
        world, &results, mapIndex, mapX, mapY, moveResult);
    if (results.rotationPending &&
        (results.rotationEffect == DM1_EFFECT_CLEAR ||
         results.rotationEffect == DM1_EFFECT_TOGGLE) &&
        orch_f0271_rotate_sensor_chain_compat(
            world, mapIndex, results.rotationMapX, results.rotationMapY,
            results.rotationCell, results.rotationEffect)) {
        moveResult->localSensorRotations++;
        moveResult->lastLocalSensorRotation.effect = results.rotationEffect;
        moveResult->lastLocalSensorRotation.mapX = results.rotationMapX;
        moveResult->lastLocalSensorRotation.mapY = results.rotationMapY;
        moveResult->lastLocalSensorRotation.cell = results.rotationCell;
    }
    return results.count;
}

static int orch_f0267_stairs_exit_direction_compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char square;
    int northSouth;
    int checkX;
    int checkY;
    int blocked = 1;

    if (!dungeon || !dungeon->maps || !dungeon->tiles ||
        mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return 0;
    map = &dungeon->maps[mapIndex];
    if (!dungeon->tiles[mapIndex].squareData || mapX < 0 || mapX >= map->width ||
        mapY < 0 || mapY >= map->height) return 0;
    square = dungeon->tiles[mapIndex].squareData[mapX * map->height + mapY];
    northSouth = (square & 0x08) ? 0 : 1;
    checkX = mapX + (northSouth ? 1 : 0);
    checkY = mapY + (northSouth ? 0 : -1);
    if (checkX >= 0 && checkX < map->width && checkY >= 0 && checkY < map->height) {
        int type = (dungeon->tiles[mapIndex].squareData[
            checkX * map->height + checkY] & DUNGEON_SQUARE_MASK_TYPE) >> 5;
        blocked = (type == DUNGEON_ELEMENT_WALL || type == DUNGEON_ELEMENT_STAIRS);
    }
    return (blocked << 1) + northSouth;
}

static int orch_f0267_resolve_non_group_chain_compat(
    const struct GameWorld_Compat* world,
    unsigned short* inOutThing,
    int* inOutMapIndex,
    int* inOutMapX,
    int* inOutMapY,
    struct F0267ThingMoveResultPc34Compat* result)
{
    int remaining;
    int type;

    if (!world || !world->dungeon || !inOutThing || !inOutMapIndex ||
        !inOutMapX || !inOutMapY || !result) return 0;
    type = THING_GET_TYPE(*inOutThing);
    for (remaining = 100; remaining > 0; --remaining) {
        const struct DungeonMapDesc_Compat* map;
        unsigned char square;
        int squareType;
        if (*inOutMapIndex < 0 ||
            *inOutMapIndex >= (int)world->dungeon->header.mapCount) return 0;
        map = &world->dungeon->maps[*inOutMapIndex];
        if (!world->dungeon->tiles || !world->dungeon->tiles[*inOutMapIndex].squareData ||
            *inOutMapX < 0 || *inOutMapX >= map->width ||
            *inOutMapY < 0 || *inOutMapY >= map->height) return 0;
        square = world->dungeon->tiles[*inOutMapIndex].squareData[
            *inOutMapX * map->height + *inOutMapY];
        squareType = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
        if (squareType == DUNGEON_ELEMENT_TELEPORTER && (square & 0x08)) {
            struct DungeonTeleporter_Compat teleporter;
            int selfTarget;
            if (!orch_find_teleporter_on_square_compat(
                    world, *inOutMapIndex, *inOutMapX, *inOutMapY,
                    &teleporter) || !(teleporter.scope & 0x02) ||
                teleporter.targetMapIndex >= world->dungeon->header.mapCount) break;
            selfTarget = teleporter.targetMapIndex == *inOutMapIndex &&
                teleporter.targetMapX == *inOutMapX &&
                teleporter.targetMapY == *inOutMapY;
            *inOutMapIndex = teleporter.targetMapIndex;
            *inOutMapX = teleporter.targetMapX;
            *inOutMapY = teleporter.targetMapY;
            /* ReDMCSB MOVESENS.C F0267 lines 450-482 admits ordinary
             * Things and C14 projectiles only through object/party-scope
             * teleporters. Lines 526-531 then apply F0263: a relative
             * rotation changes the placed C14 cell and direction; absolute
             * rotation changes only the direction.  The final F0249 handoff
             * publishes that direction to the authenticated M10 projection
             * while its C48/C49 receipt keeps the packed cell. */
            if (type == THING_TYPE_PROJECTILE) {
                DM1_V1_TeleporterDefPc34 rotationTeleporter;
                int rotatedDirection;
                int rotatedCell;

                memset(&rotationTeleporter, 0, sizeof(rotationTeleporter));
                rotationTeleporter.destFacing = (int)teleporter.rotation;
                rotationTeleporter.absoluteRotation = teleporter.absoluteRotation ? 1 : 0;
                if (!DM1_V1_ApplyTeleporterRotationF0267Pc34Compat(
                        DM1_V1_TELEPORTER_ROTATE_THING_PROJECTILE_PC34,
                        *inOutMapX, &rotationTeleporter,
                        result->projectileTeleporterDirectionValid
                            ? result->projectileTeleporterDirection : 0,
                        THING_GET_CELL(*inOutThing),
                        &rotatedDirection, &rotatedCell)) {
                    return 0;
                }
                *inOutThing = orch_thing_with_cell_compat(*inOutThing, rotatedCell);
                if (result->projectileTeleporterDirectionValid) {
                    result->projectileTeleporterDirection = rotatedDirection;
                }
            } else if (!teleporter.absoluteRotation) {
                *inOutThing = orch_thing_with_cell_compat(
                    *inOutThing, THING_GET_CELL(*inOutThing) + teleporter.rotation);
            }
            result->teleporterChainCount++;
            if (selfTarget) break;
            continue;
        }
        if (squareType == DUNGEON_ELEMENT_PIT && type != THING_TYPE_PROJECTILE &&
            (square & 0x08) && !(square & 0x01)) {
            int target = orch_group_level_change_location_compat(
                world->dungeon, *inOutMapIndex, 1, inOutMapX, inOutMapY);
            if (target < 0) break;
            *inOutMapIndex = target;
            result->pitChainCount++;
            continue;
        }
        if (squareType == DUNGEON_ELEMENT_STAIRS && type != THING_TYPE_PROJECTILE &&
            !(square & 0x04)) {
            int exitDirection;
            int target = orch_group_level_change_location_compat(
                world->dungeon, *inOutMapIndex, 1, inOutMapX, inOutMapY);
            if (target < 0) break;
            *inOutMapIndex = target;
            exitDirection = orch_f0267_stairs_exit_direction_compat(
                world->dungeon, *inOutMapIndex, *inOutMapX, *inOutMapY);
            if (exitDirection == 0) --*inOutMapY;
            else if (exitDirection == 1) ++*inOutMapX;
            else if (exitDirection == 2) ++*inOutMapY;
            else --*inOutMapX;
            exitDirection = (exitDirection + 2) & 3;
            *inOutThing = orch_thing_with_cell_compat(*inOutThing,
                ((((THING_GET_CELL(*inOutThing) - exitDirection + 1) & 2) >> 1) +
                 exitDirection));
            result->stairsChainCount++;
            continue;
        }
        break;
    }
    if (remaining == 0) result->chainedMoveLimitHit = 1;
    return 1;
}

int F0267_MOVE_MoveThingOnLoadedChain_Compat(
    struct GameWorld_Compat* world,
    const struct F0267ThingMoveRequestPc34Compat* request,
    struct F0267ThingMoveResultPc34Compat* outResult)
{
    struct F0267ThingMoveResultPc34Compat result;
    struct F0267ThingMoveRequestPc34Compat resolvedRequest;
    int type;

    memset(&result, 0, sizeof(result));
    if (!world || !world->dungeon || !world->things || !request ||
        !world->things->loaded || request->thing == THING_NONE ||
        request->thing == THING_ENDOFLIST) {
        if (outResult) *outResult = result;
        return 0;
    }
    resolvedRequest = *request;
    type = THING_GET_TYPE(request->thing);
    result.thingType = type;
    if (type == THING_TYPE_PROJECTILE) {
        int projectileIndex = THING_GET_INDEX(request->thing);
        if (projectileIndex >= 0 && projectileIndex < PROJECTILE_LIST_CAPACITY) {
            const struct ProjectileInstance_Compat* live =
                &world->projectiles.entries[projectileIndex];
            if (live->reserved3 != 0 && live->slotIndex == projectileIndex) {
                result.projectileTeleporterDirectionValid = 1;
                result.projectileTeleporterDirection = live->direction & 3;
            }
        }
    }
    /* C04 owns active-group state, collision deferral and C04 rotation in
     * the existing F0267 branch.  Keeping it there prevents this common
     * object route from changing source C04 move ordering. */
    if (type == THING_TYPE_GROUP || type < THING_TYPE_WEAPON ||
        type > THING_TYPE_EXPLOSION ||
        !orch_f0267_thing_is_present_on_square_compat(
            world, request->sourceMapIndex, request->sourceMapX,
            request->sourceMapY, request->thing)) {
        if (outResult) *outResult = result;
        return 0;
    }

    result.valid = 1;
    result.levitates = F0264_DM1_MOVE_IsLevitating_Compat(type, 0);
    if (!orch_f0267_resolve_non_group_chain_compat(
            world, &resolvedRequest.thing, &resolvedRequest.destinationMapIndex,
            &resolvedRequest.destinationMapX, &resolvedRequest.destinationMapY, &result)) {
        if (outResult) *outResult = result;
        return 0;
    }
    result.finalMapIndex = resolvedRequest.destinationMapIndex;
    result.finalMapX = resolvedRequest.destinationMapX;
    result.finalMapY = resolvedRequest.destinationMapY;
    result.finalThing = resolvedRequest.thing;
    /* ReDMCSB MOVESENS.C:F0267 lines 799-807 runs F0276 before the
     * source unlink for ordinary things. C14 projectiles and C15 explosions
     * levitate under F0264 and therefore use F0164 directly. */
    if (!result.levitates) {
        result.sourceSensorPasses = orch_f0267_sensor_pass_dispatch_compat(
            world, resolvedRequest.sourceMapIndex, resolvedRequest.sourceMapX,
            resolvedRequest.sourceMapY, resolvedRequest.thing, 0, &result);
    }
    if (!orch_unlink_thing_from_square_compat(
            world, resolvedRequest.sourceMapIndex, resolvedRequest.sourceMapX,
            resolvedRequest.sourceMapY, resolvedRequest.thing)) {
        if (outResult) *outResult = result;
        return 0;
    }
    result.sourceUnlinked = 1;

    /* MOVESENS.C:F0267 lines 892-897 runs the destination F0276 pass
     * before linking ordinary things; F0264 keeps C14/C15 levitating. */
    if (!result.levitates) {
        result.destinationSensorPasses = orch_f0267_sensor_pass_dispatch_compat(
            world, resolvedRequest.destinationMapIndex, resolvedRequest.destinationMapX,
            resolvedRequest.destinationMapY, resolvedRequest.thing, 1, &result);
    }
    if (!orch_link_thing_to_square_tail_compat(
            world, resolvedRequest.destinationMapIndex, resolvedRequest.destinationMapX,
            resolvedRequest.destinationMapY, resolvedRequest.thing)) {
        /* A failed destination link is not allowed to discard an original
         * Thing. Restore its source position before reporting failure. */
        (void)orch_link_thing_to_square_tail_compat(
            world, resolvedRequest.sourceMapIndex, resolvedRequest.sourceMapX,
            resolvedRequest.sourceMapY, resolvedRequest.thing);
        result.sourceUnlinked = 0;
        if (outResult) *outResult = result;
        return 0;
    }
    result.destinationLinked = 1;
    result.moved = 1;
    if (type == THING_TYPE_PROJECTILE || type == THING_TYPE_EXPLOSION) {
        result.timelineRelocationCount =
            DM1_V1_F0249_RelocateTimelineForMovedThingPc34Compat(
                world->timeline.events, world->timeline.count, type,
                THING_GET_INDEX(resolvedRequest.thing),
                result.finalMapIndex, result.finalMapX, result.finalMapY,
                THING_GET_CELL(result.finalThing));
        if (result.timelineRelocationCount < 0) {
            /* F0249 is a post-move event-coordinate repair only. A malformed
             * timeline must not turn a successful source-list move into an
             * invented event or mutate an unrelated queue entry. */
            result.timelineRelocationCount = 0;
        }
    }
    if (type == THING_TYPE_PROJECTILE) {
        int projectileIndex = THING_GET_INDEX(result.finalThing);
        if (projectileIndex >= 0 && projectileIndex < PROJECTILE_LIST_CAPACITY) {
            struct ProjectileInstance_Compat* live =
                &world->projectiles.entries[projectileIndex];
            /* F0249 has already moved the authenticated C14 and its C48/C49
             * receipt. Keep the matching live M10 projection on that same
             * teleporter/materialized square so the next F0217 damage pass
             * cannot resolve at the old location. */
            if (live->reserved3 != 0 && live->slotIndex == projectileIndex) {
                live->mapIndex = result.finalMapIndex;
                live->mapX = result.finalMapX;
                live->mapY = result.finalMapY;
                live->cell = THING_GET_CELL(result.finalThing);
            }
        }
    }
    if (type == THING_TYPE_PROJECTILE &&
        result.projectileTeleporterDirectionValid) {
        int projectileIndex = THING_GET_INDEX(result.finalThing);
        if (projectileIndex >= 0 && projectileIndex < PROJECTILE_LIST_CAPACITY) {
            struct ProjectileInstance_Compat* live =
                &world->projectiles.entries[projectileIndex];
            if (live->reserved3 != 0 && live->slotIndex == projectileIndex) {
                live->direction = result.projectileTeleporterDirection;
            }
        }
    }
    if (outResult) *outResult = result;
    return 1;
}

/* ReDMCSB TIMELINE.C F0249:1352-1465 snapshots the source square before
 * moving its Things through F0267, because a move can unlink a later entry
 * or loop back onto the same square. Party and GROUP use their own F0267
 * branches; this helper owns only the ordinary type C05..C15 chain that the
 * public F0267 object route can faithfully materialize today. */
static int orch_f0249_move_non_group_square_things_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY)
{
    unsigned short snapshot[64];
    unsigned short thing;
    int squareFirstThingIndex;
    int snapshotCount = 0;
    int safety = 0;
    int i;

    if (!world || !world->dungeon || !world->things ||
        !world->things->loaded || !world->things->squareFirstThings) {
        return 0;
    }
    squareFirstThingIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (squareFirstThingIndex < 0 ||
        squareFirstThingIndex >= world->things->squareFirstThingCount) {
        return 0;
    }

    thing = world->things->squareFirstThings[squareFirstThingIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST &&
           safety++ < (int)(sizeof(snapshot) / sizeof(snapshot[0]))) {
        int type = THING_GET_TYPE(thing);
        unsigned short nextThing = orch_next_thing_compat(world->things, thing);

        /* F0249 moves the GROUP before later Things. Its active-group,
         * fall-damage and timeline work belongs to the existing group F0267
         * route, so do not misroute it through the ordinary object path. */
        if (type > THING_TYPE_GROUP && type <= THING_TYPE_EXPLOSION) {
            snapshot[snapshotCount++] = thing;
        }
        thing = nextThing;
    }

    for (i = 0; i < snapshotCount; ++i) {
        struct F0267ThingMoveRequestPc34Compat request;
        struct F0267ThingMoveResultPc34Compat moveResult;
        int thingType = THING_GET_TYPE(snapshot[i]);

        memset(&request, 0, sizeof(request));
        memset(&moveResult, 0, sizeof(moveResult));
        request.thing = snapshot[i];
        request.sourceMapIndex = mapIndex;
        request.sourceMapX = mapX;
        request.sourceMapY = mapY;
        request.destinationMapIndex = mapIndex;
        request.destinationMapX = mapX;
        request.destinationMapY = mapY;
        /* A prior F0249 move can have consumed this snapshot entry. F0267
         * validates source membership and makes that case a no-op. */
        if (F0267_MOVE_MoveThingOnLoadedChain_Compat(
                world, &request, &moveResult) && moveResult.moved &&
            (thingType == DM1_F0249_THING_PROJECTILE_PC34 ||
             thingType == DM1_F0249_THING_EXPLOSION_PC34)) {
            /* ReDMCSB TIMELINE.C F0249:1420-1452 repairs runtime event
             * coordinates only after F0267 has published the final chain
             * location and cell. Fluxcage removal remains source-faithfully
             * excluded inside the DM1-owned relocation helper. */
            (void)DM1_V1_F0249_RelocateTimelineForMovedThingPc34Compat(
                world->timeline.events, world->timeline.count, thingType,
                THING_GET_INDEX(snapshot[i]), moveResult.finalMapIndex,
                moveResult.finalMapX, moveResult.finalMapY,
                THING_GET_CELL(moveResult.finalThing));
        }
    }
    return 1;
}

static int orch_drop_creature_fixed_possessions_compat(
    struct GameWorld_Compat* world,
    int creatureType,
    int sourceCell,
    int mapIndex,
    int mapX,
    int mapY,
    int* outSoundId)
{
    struct DM1FixedPossessionDrop_Compat drops[DM1_MAX_FIXED_POSSESSION_DROPS];
    int dropCount = 0;
    int weaponDropped = 0;
    int droppedAny = 0;
    int i;

    if (outSoundId) *outSoundId = -1;
    if (!world || !world->things) return 0;
    if (!F0824_DM1_GROUP_ResolveFixedPossessionDrops_Compat(
            creatureType, sourceCell, &world->masterRng, drops,
            DM1_MAX_FIXED_POSSESSION_DROPS, &dropCount, &weaponDropped)) {
        return 0;
    }
    if (outSoundId && dropCount > 0) {
        *outSoundId = weaponDropped ? 0 : ORCH_SOUND_WOODEN_THUD_PC34;
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
    DM1_MeleeF0190FixedDropCellsPlanPc34 plan;

    memset(&plan, 0, sizeof(plan));
    if (!dm1_v1_melee_moving_fixed_drop_cells_plan_f0187_pc34(
            cells, cellCount, &plan) ||
        !plan.valid) {
        return 0;
    }
    for (i = 0; i < plan.dropCellCount; ++i) {
        droppedAny |= orch_drop_creature_fixed_possessions_compat(
            world, creatureType, plan.dropCells[i], mapIndex, mapX, mapY,
            NULL);
    }
    return droppedAny || plan.dropCellCount == 0;
}

static int orch_drop_group_fixed_possessions_compat(
    struct GameWorld_Compat* world,
    const struct DungeonGroup_Compat* group,
    int mapIndex,
    int mapX,
    int mapY)
{
    int i;
    DM1_MeleeF0190FixedDropCellsPlanPc34 plan;

    if (!world || !group) return 0;
    memset(&plan, 0, sizeof(plan));
    if (!dm1_v1_melee_group_fixed_drop_cells_plan_f0190_pc34(
            group, &plan) ||
        !plan.valid) {
        return 0;
    }
    for (i = 0; i < plan.dropCellCount; ++i) {
        (void)orch_drop_creature_fixed_possessions_compat(
            world, group->creatureType, plan.dropCells[i],
            mapIndex, mapX, mapY, NULL);
    }
    return 1;
}

static int orch_drop_group_slot_possessions_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    int mapIndex,
    int mapX,
    int mapY,
    int* outSoundId);

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
    return orch_drop_group_slot_possessions_compat(
        world, group, mapIndex, mapX, mapY, NULL);
}

static int orch_apply_group_move_removal_plan_f0267_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const unsigned char* movingFixedDropCells,
    int movingFixedDropCellCount,
    int fallKilledGroup,
    int creatureAllowedOnDestinationMap,
    int sourceMapX,
    int sourceMapY,
    int destinationMapIndex,
    int destinationMapX,
    int destinationMapY)
{
    DM1_V1_GroupMoveRemovalPlanPc34 plan;

    if (!world || !group) return 0;
    memset(&plan, 0, sizeof(plan));
    if (!DM1_V1_PlanGroupMoveRemovalAfterPitTeleporterF0267Pc34Compat(
            fallKilledGroup, creatureAllowedOnDestinationMap,
            sourceMapX, sourceMapY, destinationMapX, destinationMapY,
            &plan)) {
        return 0;
    }
    if (!plan.movePrevented) return 1;
    if (plan.dropMovingCreatureFixedPossessions) {
        (void)orch_drop_moving_fixed_possessions_compat(
            world, group->creatureType, movingFixedDropCells,
            movingFixedDropCellCount, destinationMapIndex,
            plan.dropMapX, plan.dropMapY);
    }
    if (plan.dropGroupPossessions) {
        (void)orch_drop_group_fixed_possessions_compat(
            world, group, destinationMapIndex, plan.dropMapX, plan.dropMapY);
        return orch_drop_group_slot_possessions_compat(
            world, group, destinationMapIndex, plan.dropMapX, plan.dropMapY,
            NULL);
    }
    return 1;
}

static void orch_apply_moving_killed_all_afterplay_f0190_compat(
    struct GameWorld_Compat* world,
    int groupIndex,
    const struct DungeonGroup_Compat* group,
    int sourceSquareKnown,
    int sourceMapIndex,
    int sourceMapX,
    int sourceMapY,
    int sourceCell,
    int destinationMapIndex,
    int destinationMapX,
    int destinationMapY)
{
    struct DM1CreatureInfo_Compat creatureInfo;
    DM1_MeleeF0190MovingKilledAllAfterplayInputPc34 input;
    DM1_MeleeF0190MovingKilledAllAfterplayPlanPc34 plan;
    struct TimelineEvent_Compat advance;
    int slotIndex = -1;

    if (!world || !group || !sourceSquareKnown) return;
    if (!orch_get_dm1_creature_info_pc34_compat(
            (int)group->creatureType, &creatureInfo)) {
        return;
    }
    memset(&input, 0, sizeof(input));
    memset(&plan, 0, sizeof(plan));
    input.outcome = COMBAT_OUTCOME_KILLED_ALL_CREATURES;
    input.groupIndex = groupIndex;
    input.creatureAttributes = creatureInfo.attributes;
    input.sourceMapIndex = sourceMapIndex;
    input.sourceMapX = sourceMapX;
    input.sourceMapY = sourceMapY;
    input.sourceCell = sourceCell;
    input.destinationMapIndex = destinationMapIndex;
    input.destinationMapX = destinationMapX;
    input.destinationMapY = destinationMapY;
    input.currentTick = world->gameTick;

    /* ReDMCSB GROUP.C F0190:834-839 defers F0188/F0189 for a moving
     * final creature, while lines 907-917 still call F0213 at P0371/P0372.
     * MOVESENS.C F0267:656-663 owns the later destination drops/delete.
     * Do not invent a source coordinate for C60/C61 or C006 routes. */
    if (!dm1_v1_melee_moving_killed_all_afterplay_plan_f0190_pc34(
            &input, &plan) || !plan.valid ||
        !plan.shouldPresentSourceSmoke ||
        !plan.requiresDeferredDestinationCleanup) {
        return;
    }
    memset(&advance, 0, sizeof(advance));
    if (F0821_EXPLOSION_Create_Compat(&plan.sourceSmokeCreateInput,
                                      &world->explosions, &slotIndex,
                                      &advance)) {
        (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &advance);
    }
}

static int orch_damage_group_by_pit_fall_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    unsigned char movingFixedDropCells[4],
    int* movingFixedDropCellCount,
    int* outFinalKillCell)
{
    int originalCount;
    int creatureIndex;
    int killedAny = 0;

    if (outFinalKillCell) *outFinalKillCell = EXPLOSION_CELL_CENTERED;
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
                if (outFinalKillCell) *outFinalKillCell = killedCell;
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
    int mapY,
    int* outSoundId)
{
    int sftIndex;
    unsigned short thing;
    int i;
    DM1_MeleeF0188GroupSlotDropInputPc34 in;
    DM1_MeleeF0188GroupSlotDropPlanPc34 plan;

    if (outSoundId) *outSoundId = -1;
    if (!world || !group || !world->dungeon || !world->things) return 0;
    thing = group->slot;
    if (thing == THING_NONE || thing == THING_ENDOFLIST) return 1;

    sftIndex = orch_square_first_thing_list_index_compat(world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) return 0;

    memset(&in, 0, sizeof(in));
    memset(&plan, 0, sizeof(plan));
    in.slotHead = thing;
    while (thing != THING_NONE && thing != THING_ENDOFLIST &&
           in.chainEntryCount < DM1_MELEE_F0188_GROUP_SLOT_DROP_MAX_PC34) {
        unsigned short nextThing;
        nextThing = orch_next_thing_compat(world->things, thing);
        in.chain[in.chainEntryCount].thing = thing;
        in.chain[in.chainEntryCount].nextThing = nextThing;
        in.randomCells[in.randomCellCount++] =
            (unsigned char)F0732_COMBAT_RngRandom_Compat(&world->masterRng, 4);
        ++in.chainEntryCount;
        thing = nextThing;
    }
    if (!dm1_v1_melee_group_slot_drop_plan_f0188_pc34(&in, &plan) ||
        !plan.valid || !plan.shouldDrop || plan.truncated) {
        return 0;
    }
    for (i = 0; i < plan.stepCount; ++i) {
        if (!orch_link_thing_to_square_tail_compat(
                world, mapIndex, mapX, mapY, plan.steps[i].droppedThing)) {
            return 0;
        }
    }
    if (plan.shouldClearGroupSlot) {
        group->slot = THING_ENDOFLIST;
    }
    if (outSoundId) *outSoundId = plan.soundId;
    return 1;
}

static void orch_cmd_attack_apply_f0190_possession_drop_plan_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const DM1_MeleeF0190PossessionDropPlanPc34* dropPlan)
{
    DM1_MeleeF0190PossessionDropApplyPlanPc34 applyPlan;
    int slotDropSoundId = -1;
    int i;

    if (!world || !group || !dropPlan || !dropPlan->valid) return;
    memset(&applyPlan, 0, sizeof(applyPlan));
    if (!dm1_v1_melee_possession_drop_apply_plan_f0190_pc34(
            dropPlan, group, &applyPlan) ||
        !applyPlan.valid) {
        return;
    }
    if (applyPlan.shouldDropGroupFixedPossessions) {
        for (i = 0; i < applyPlan.groupFixedCellCount; ++i) {
            int fixedDropSoundId = -1;

            (void)orch_drop_creature_fixed_possessions_compat(
                world, group->creatureType, applyPlan.groupFixedCells[i],
                applyPlan.mapIndex, applyPlan.mapX, applyPlan.mapY,
                &fixedDropSoundId);
            if (fixedDropSoundId >= 0) {
                struct TimelineEvent_Compat sound;
                memset(&sound, 0, sizeof(sound));
                sound.kind = TIMELINE_EVENT_PLAY_SOUND;
                sound.fireAtTick = world->gameTick + 1u;
                sound.mapIndex = applyPlan.mapIndex;
                sound.mapX = applyPlan.mapX;
                sound.mapY = applyPlan.mapY;
                sound.aux0 = fixedDropSoundId;
                (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &sound);
            }
        }
    }
    if (applyPlan.shouldDropGroupSlotPossessions) {
        if (orch_drop_group_slot_possessions_compat(
                world, group, applyPlan.mapIndex, applyPlan.mapX,
                applyPlan.mapY, &slotDropSoundId) &&
            slotDropSoundId >= 0) {
            struct TimelineEvent_Compat sound;

            /* ReDMCSB GROUP.C F0188 lines 724-736 requests one thud using
             * C02_MODE_PLAY_ONE_TICK_LATER after the whole Slot chain has
             * entered the square list. Keep it as a timeline event so M10's
             * sound dispatcher owns the delayed request. */
            memset(&sound, 0, sizeof(sound));
            sound.kind = TIMELINE_EVENT_PLAY_SOUND;
            sound.fireAtTick = world->gameTick + 1u;
            sound.mapIndex = applyPlan.mapIndex;
            sound.mapX = applyPlan.mapX;
            sound.mapY = applyPlan.mapY;
            sound.aux0 = slotDropSoundId;
            (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &sound);
        }
    }
    if (applyPlan.shouldDropCreatureFixedPossessions) {
        int fixedDropSoundId = -1;

        (void)orch_drop_creature_fixed_possessions_compat(
            world, applyPlan.creatureType, applyPlan.creatureCell,
            applyPlan.mapIndex, applyPlan.mapX, applyPlan.mapY,
            &fixedDropSoundId);
        if (fixedDropSoundId >= 0) {
            struct TimelineEvent_Compat sound;
            memset(&sound, 0, sizeof(sound));
            sound.kind = TIMELINE_EVENT_PLAY_SOUND;
            sound.fireAtTick = world->gameTick + 1u;
            sound.mapIndex = applyPlan.mapIndex;
            sound.mapX = applyPlan.mapX;
            sound.mapY = applyPlan.mapY;
            sound.aux0 = fixedDropSoundId;
            (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &sound);
        }
    }
}

static void orch_cmd_attack_cleanup_f0190_creature_events_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    int killedCreatureIndex)
{
    DM1_MeleeF0190TimelineCleanupBatchInputPc34 cleanupIn;
    DM1_MeleeF0190TimelineCleanupBatchPlanPc34 cleanupPlan;
    int i;

    if (!world) return;
    if (killedCreatureIndex < 0 || killedCreatureIndex > 3) return;

    memset(&cleanupIn, 0, sizeof(cleanupIn));
    memset(&cleanupPlan, 0, sizeof(cleanupPlan));
    cleanupIn.eventCount = world->timeline.count;
    if (cleanupIn.eventCount < 0 ||
        cleanupIn.eventCount > TIMELINE_QUEUE_CAPACITY) {
        return;
    }
    for (i = 0; i < cleanupIn.eventCount; ++i) {
        cleanupIn.events[i] = world->timeline.events[i];
    }
    cleanupIn.targetMapIndex = mapIndex;
    cleanupIn.targetMapX = mapX;
    cleanupIn.targetMapY = mapY;
    cleanupIn.killedCreatureIndex = killedCreatureIndex;

    if (!dm1_v1_melee_timeline_cleanup_batch_plan_f0190_pc34(
            &cleanupIn, &cleanupPlan) ||
        !cleanupPlan.valid) {
        return;
    }
    for (i = 0; i < cleanupPlan.newEventCount; ++i) {
        world->timeline.events[i] = cleanupPlan.events[i];
    }
    while (i < cleanupPlan.oldEventCount) {
        memset(&world->timeline.events[i], 0,
               sizeof(world->timeline.events[i]));
        ++i;
    }
    world->timeline.count = cleanupPlan.newEventCount;
}

static void orch_cmd_attack_cleanup_f0190_killed_all_events_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY)
{
    DM1_MeleeF0190TimelineCleanupBatchInputPc34 cleanupIn;
    DM1_MeleeF0190TimelineCleanupBatchPlanPc34 cleanupPlan;
    int readIndex;
    int writeIndex = 0;

    if (!world) return;
    memset(&cleanupIn, 0, sizeof(cleanupIn));
    memset(&cleanupPlan, 0, sizeof(cleanupPlan));
    cleanupIn.eventCount = world->timeline.count;
    if (cleanupIn.eventCount < 0 ||
        cleanupIn.eventCount > TIMELINE_QUEUE_CAPACITY) {
        return;
    }
    for (readIndex = 0; readIndex < cleanupIn.eventCount; ++readIndex) {
        cleanupIn.events[readIndex] = world->timeline.events[readIndex];
    }
    cleanupIn.targetMapIndex = mapIndex;
    cleanupIn.targetMapX = mapX;
    cleanupIn.targetMapY = mapY;
    /* Use the current typed F0190 timeline receipt for its bounded queue
     * validation and compaction contract. F0181 then removes the whole
     * group range, not merely one creature's aspect/behavior entries. */
    cleanupIn.killedCreatureIndex = 0;
    if (!dm1_v1_melee_timeline_cleanup_batch_plan_f0190_pc34(
            &cleanupIn, &cleanupPlan) || !cleanupPlan.valid) {
        return;
    }
    for (readIndex = 0; readIndex < cleanupPlan.newEventCount; ++readIndex) {
        const struct TimelineEvent_Compat* event =
            &cleanupPlan.events[readIndex];

        /* ReDMCSB GROUP.C F0181 lines 340-371 deletes every C29..C41
         * entry on the dead group's square. This is intentionally broader
         * than F0190 killed-some's per-creature timeline compaction. */
        if (event->kind == TIMELINE_EVENT_CREATURE_REACTION &&
            event->mapIndex == mapIndex && event->mapX == mapX &&
            event->mapY == mapY &&
            event->aux2 >= DM1_EVENT_REACTION_DANGER_ON_SQUARE &&
            event->aux2 <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
            continue;
        }
        world->timeline.events[writeIndex++] = *event;
    }
    world->timeline.count = writeIndex;
    while (writeIndex < cleanupPlan.oldEventCount) {
        memset(&world->timeline.events[writeIndex], 0,
               sizeof(world->timeline.events[writeIndex]));
        ++writeIndex;
    }
}

static int orch_cmd_attack_apply_f0190_fear_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const DM1_MeleeF0190MutationDispatchPlanPc34* dispatchPlan)
{
    DM1_MeleeF0190FearRollPlanPc34 rollPlan;
    DM1_MeleeF0190KilledSomeStatePlanPc34 applyPlan;
    int activeIndex;
    int shouldFlee = 0;
    int fleeDelay = 0;

    if (!world || !group || !dispatchPlan) {
        return 0;
    }
    memset(&rollPlan, 0, sizeof(rollPlan));
    if (!dm1_v1_melee_mutation_dispatch_fear_roll_plan_f0190_pc34(
            dispatchPlan, &rollPlan) ||
        !rollPlan.valid || !rollPlan.shouldEvaluateFear) {
        return 0;
    }

    if (!F0821_DM1_GROUP_ShouldFrighten_Compat(
            &rollPlan.fearContext, rollPlan.originalGroupCount,
            &world->masterRng,
            &shouldFlee, &fleeDelay)) {
        return 0;
    }
    memset(&applyPlan, 0, sizeof(applyPlan));
    if (!dm1_v1_melee_mutation_dispatch_fear_apply_plan_f0190_pc34(
            dispatchPlan, shouldFlee, fleeDelay, &applyPlan) ||
        !applyPlan.valid) {
        return 0;
    }
    {
        DM1_MeleeF0190KilledSomeStateApplyPlanPc34 receipt;
        memset(&receipt, 0, sizeof(receipt));
        if (!dm1_v1_melee_killed_some_state_apply_plan_f0190_pc34(
                &applyPlan, &receipt) ||
            !receipt.valid || !receipt.shouldApplyFear) {
            return 0;
        }
        group->behavior = (unsigned char)receipt.newGroupBehavior;
        activeIndex =
            orch_find_active_group_state_index_compat(
                world, receipt.groupIndex);
        if (activeIndex >= 0) {
            world->creatureAI[activeIndex].stateKind =
                receipt.newAiStateKind;
            world->creatureAI[activeIndex].fearCounter =
                receipt.fearCounter;
        }
    }
    return 1;
}

static int orch_cmd_attack_apply_killed_some_receipt_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const DM1_MeleeF0190KilledSomeStatePlanPc34* statePlan)
{
    DM1_MeleeF0190KilledSomeStateApplyPlanPc34 receipt;
    int activeIndex;

    if (!world || !group || !statePlan) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_melee_killed_some_state_apply_plan_f0190_pc34(
            statePlan, &receipt) ||
        !receipt.valid) {
        return 0;
    }
    if (receipt.shouldCleanupCreatureEvents) {
        orch_cmd_attack_cleanup_f0190_creature_events_compat(
            world, receipt.mapIndex, receipt.mapX, receipt.mapY,
            receipt.killedCreatureIndex);
    }
    if (receipt.shouldApplyFear) {
        group->behavior = (unsigned char)receipt.newGroupBehavior;
        activeIndex =
            orch_find_active_group_state_index_compat(
                world, receipt.groupIndex);
        if (activeIndex >= 0) {
            world->creatureAI[activeIndex].stateKind =
                receipt.newAiStateKind;
            world->creatureAI[activeIndex].fearCounter =
                receipt.fearCounter;
        }
    }
    return receipt.shouldApplyFear;
}

static int orch_cmd_attack_apply_f0190_mutation_dispatch_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    const DM1_MeleeF0190MutationDispatchPlanPc34* plan)
{
    DM1_MeleeF0190MutationDispatchApplyPlanPc34 applyPlan;
    int fearTriggered = 0;

    if (!world || !group || !plan) return 0;

    memset(&applyPlan, 0, sizeof(applyPlan));
    if (!dm1_v1_melee_mutation_dispatch_apply_plan_f0190_pc34(
            plan, &applyPlan) ||
        !applyPlan.valid) {
        return 0;
    }
    if (applyPlan.shouldDropPossessions) {
        orch_cmd_attack_apply_f0190_possession_drop_plan_compat(
            world, group, &applyPlan.possessionDropPlan);
    }
    if (applyPlan.shouldApplyKilledSomeState) {
        (void)orch_cmd_attack_apply_killed_some_receipt_compat(
            world, group, &applyPlan.killedSomeStatePlan);
        if (applyPlan.shouldEvaluateFear) {
            fearTriggered =
                orch_cmd_attack_apply_f0190_fear_compat(
                    world, group, plan);
        }
    }
    if (applyPlan.shouldApplyKilledAllSideEffects) {
        orch_cmd_attack_apply_group_kill_side_effects_plan_f0190_compat(
            world, &applyPlan.killedAllStatePlan);
    }
    return fearTriggered;
}

static int orch_resolve_group_f0267_pit_destination_compat(
    struct GameWorld_Compat* world,
    struct DungeonGroup_Compat* group,
    int* inOutMapIndex,
    int* inOutMapX,
    int* inOutMapY,
    int* outFallKilledGroup,
    unsigned char movingFixedDropCells[4],
    int* movingFixedDropCellCount,
    int* outFinalKillCell)
{
    int remaining;

    if (outFallKilledGroup) *outFallKilledGroup = 0;
    if (movingFixedDropCellCount) *movingFixedDropCellCount = 0;
    if (outFinalKillCell) *outFinalKillCell = EXPLOSION_CELL_CENTERED;
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
        DM1_V1_GroupPitFallSquarePlanPc34 pitPlan;

        if (*inOutMapIndex < 0 || *inOutMapIndex >= (int)world->dungeon->header.mapCount) break;
        map = &world->dungeon->maps[*inOutMapIndex];
        if (*inOutMapX < 0 || *inOutMapX >= map->width ||
            *inOutMapY < 0 || *inOutMapY >= map->height) break;
        if (!world->dungeon->tiles || !world->dungeon->tiles[*inOutMapIndex].squareData) break;

        squareByte = world->dungeon->tiles[*inOutMapIndex].squareData[*inOutMapX * map->height + *inOutMapY];
        squareType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
        memset(&pitPlan, 0, sizeof(pitPlan));
        if (!DM1_V1_PlanGroupPitFallSquareF0267Pc34Compat(
                squareType, DUNGEON_ELEMENT_PIT,
                (squareByte & 0x08) != 0, (squareByte & 0x01) != 0,
                &pitPlan) ||
            !pitPlan.valid) {
            return 0;
        }
        if (!pitPlan.shouldFall) break;

        targetMapIndex = orch_group_level_change_location_compat(
            world->dungeon, *inOutMapIndex, 1, inOutMapX, inOutMapY);
        if (targetMapIndex < 0 || targetMapIndex >= (int)world->dungeon->header.mapCount) break;
        *inOutMapIndex = targetMapIndex;

        {
            unsigned char fallDropCells[4];
            int fallDropCellCount = 0;
            int finalKillCell = EXPLOSION_CELL_CENTERED;
            int fallOutcome = orch_damage_group_by_pit_fall_compat(
                world, group, fallDropCells, &fallDropCellCount,
                &finalKillCell);
            int i;
            for (i = 0; movingFixedDropCells && movingFixedDropCellCount &&
                        i < fallDropCellCount && *movingFixedDropCellCount < 4; ++i) {
                movingFixedDropCells[(*movingFixedDropCellCount)++] = fallDropCells[i];
            }
            if (fallOutcome == 2) {
                if (outFallKilledGroup) *outFallKilledGroup = 1;
                if (outFinalKillCell) *outFinalKillCell = finalKillCell;
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
    unsigned short groupThing;
    DM1_V1_F0139_CreatureAllowedOnMapReceipt_PC34 receipt;
    int groupIndex;

    if (!world || !world->things || !world->dungeon || !group ||
        !world->things->groups) {
        return 0;
    }
    for (groupIndex = 0; groupIndex < world->things->groupCount; ++groupIndex) {
        if (&world->things->groups[groupIndex] == group) break;
    }
    if (groupIndex >= world->things->groupCount) return 0;
    groupThing = orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex);

    /* ReDMCSB DUNGEON.C:F0139:1050-1079 reads the group creature type and
     * scans the target map metadata creature list. MOVESENS.C:F0267:656-663
     * performs this check after teleporter/pit resolution even when the source
     * map X sentinel is CM1_MAPX_NOT_ON_A_SQUARE. */
    return dm1_v1_dungeon_is_creature_allowed_on_map_f0139_pc34(
        world->things, world->dungeon, groupThing, mapIndex, &receipt);
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
    int randomGate;
    int randomDirection;
    DM1_V1_LordChaosAdjacentRetryPlanPc34 plan;

    if (!world || !group || !ev || !outMapX || !outMapY) return 0;
    randomGate = F0732_COMBAT_RngRandom_Compat(&world->masterRng, 4);
    memset(&plan, 0, sizeof(plan));
    if (!DM1_V1_PlanLordChaosAdjacentRetryF0252Pc34Compat(
            group->creatureType, randomGate, 0,
            ev->mapX, ev->mapY, -1, -1, &plan) ||
        !plan.valid || !plan.shouldAttempt) {
        return 0;
    }
    randomDirection = F0732_COMBAT_RngRandom_Compat(&world->masterRng, 4);
    memset(&plan, 0, sizeof(plan));
    if (!DM1_V1_PlanLordChaosAdjacentRetryF0252Pc34Compat(
            group->creatureType, randomGate, randomDirection,
            ev->mapX, ev->mapY, -1, -1, &plan) ||
        !plan.valid || !plan.shouldAttempt) {
        return 0;
    }
    candidateX = plan.candidateMapX;
    candidateY = plan.candidateMapY;
    memset(&plan, 0, sizeof(plan));
    if (!DM1_V1_PlanLordChaosAdjacentRetryF0252Pc34Compat(
            group->creatureType, randomGate, randomDirection,
            ev->mapX, ev->mapY,
            orch_is_lord_chaos_allowed_square_compat(
                world->dungeon, ev->mapIndex, candidateX, candidateY),
            orch_square_has_group_or_party_compat(
                world, ev->mapIndex, candidateX, candidateY),
            &plan) ||
        !plan.valid || !plan.shouldInsertAdjacent) {
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
    int mapY,
    int activeGroupCapacity)
{
    struct CreatureAIState_Compat* ai;
    DM1_V1_GeneratedGroupPlacementPlanPc34 plan;
    if (!world || !group) return 0;
    memset(&plan, 0, sizeof(plan));
    if (!DM1_V1_PlanGeneratedGroupPlacementF0183F0180Pc34Compat(
            world->partyMapIndex, mapIndex, mapX, mapY, groupIndex,
            group->creatureType, group->cells, group->direction,
            world->creatureAICount, activeGroupCapacity,
            world->gameTick, &plan) ||
        !plan.valid) {
        return 0;
    }
    if (!plan.shouldCreateActiveState) return 1;

    /* ReDMCSB GROUP.C:414-447/F0183 creates ACTIVE_GROUP state for a
     * group that arrives on the party map.  Phase 20 stores the closest
     * persistent active-group analogue in creatureAI[]. */
    ai = &world->creatureAI[world->creatureAICount++];
    memset(ai, 0, sizeof(*ai));
    ai->stateKind = plan.activeStateKind;
    ai->creatureType = plan.activeCreatureType;
    ai->groupMapIndex = plan.activeMapIndex;
    ai->groupMapX = plan.activeMapX;
    ai->groupMapY = plan.activeMapY;
    ai->groupCells = plan.activeCells;
    ai->groupDirection = orch_pack_group_directions_compat(
        plan.activeDirection, (int)group->count);
    ai->targetChampionIndex = plan.activeTargetChampionIndex;
    ai->lastSeenPartyMapX = plan.activeLastSeenPartyMapX;
    ai->lastSeenPartyMapY = plan.activeLastSeenPartyMapY;
    ai->lastSeenPartyTick = plan.activeLastSeenPartyTick;
    ai->reserved0 = plan.activeReservedGroupIndex;
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
    DM1_V1_GeneratedGroupPlacementPlanPc34 plan;
    if (!world || !group) return;

    /* ReDMCSB GROUP.C:311-338/F0180: newly placed groups start
     * wandering by scheduling C37 for game time +1 on their square. */
    memset(&plan, 0, sizeof(plan));
    if (!DM1_V1_PlanGeneratedGroupPlacementF0183F0180Pc34Compat(
            world->partyMapIndex, mapIndex, mapX, mapY, groupIndex,
            group->creatureType, group->cells, group->direction,
            -1, 0,
            world->gameTick, &plan) ||
        !plan.valid || !plan.shouldScheduleWanderEvent) {
        return;
    }
    memset(&wander, 0, sizeof(wander));
    wander.kind = TIMELINE_EVENT_CREATURE_TICK;
    wander.fireAtTick = plan.wanderFireAtTick;
    wander.mapIndex = plan.wanderMapIndex;
    wander.mapX = plan.wanderMapX;
    wander.mapY = plan.wanderMapY;
    wander.aux0 = plan.wanderGroupIndex;
    wander.aux1 = plan.wanderCreatureType;
    wander.aux2 = plan.wanderEventType;
    (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &wander);
}

int F0196_DM1_GROUP_InitializeActiveGroups_Compat(
    struct GameWorld_Compat* world)
{
    int activeIndex;

    if (!world || GAMEWORLD_CREATURE_AI_CAPACITY <
                      DM1_PC34_ACTIVE_GROUP_CAPACITY) {
        return 0;
    }

    /* GROUP.C:F0196 PC 3.4 allocates sixty ACTIVE_GROUP records for a new
     * game and sets every GroupThingIndex to -1. M10 uses fixed caller-owned
     * storage; initialize exactly that source range and preserve spare slots. */
    for (activeIndex = 0;
         activeIndex < DM1_PC34_ACTIVE_GROUP_CAPACITY;
         ++activeIndex) {
        memset(&world->creatureAI[activeIndex], 0,
               sizeof(world->creatureAI[activeIndex]));
        world->creatureAI[activeIndex].reserved0 = -1;
    }
    world->creatureAICount = 0;
    return 1;
}

int F0195_DM1_GROUP_AddAllActiveGroups_Compat(
    struct GameWorld_Compat* world)
{
    const struct DungeonMapDesc_Compat* map;
    const struct DungeonMapTiles_Compat* tiles;
    int mapX;
    int mapY;
    int added = 0;

    if (!world || !world->dungeon || !world->things ||
        !world->dungeon->tilesLoaded || !world->dungeon->maps ||
        !world->dungeon->tiles || !world->things->loaded ||
        world->things->groupCount < 0 ||
        world->creatureAICount < 0 ||
        world->creatureAICount > DM1_PC34_ACTIVE_GROUP_CAPACITY) {
        return -1;
    }
    if (world->partyMapIndex < 0 ||
        world->partyMapIndex >= (int)world->dungeon->header.mapCount) {
        return -1;
    }
    map = &world->dungeon->maps[world->partyMapIndex];
    tiles = &world->dungeon->tiles[world->partyMapIndex];
    if (!tiles->squareData || map->width == 0 || map->height == 0 ||
        tiles->squareCount < (int)map->width * (int)map->height ||
        world->things->squareFirstThingCount < 0) {
        return -1;
    }

    /* GROUP.C F0195: G0271 and G0283 are walked in X-major/Y-minor order.
     * For a square with a Thing list, only the first C04 reached in that
     * native chain receives F0181/F0183/F0180. */
    for (mapX = 0; mapX < (int)map->width; ++mapX) {
        for (mapY = 0; mapY < (int)map->height; ++mapY) {
            unsigned short thing;
            int sftIndex;
            int guard;

            if (!(tiles->squareData[mapX * (int)map->height + mapY] &
                  DUNGEON_SQUARE_MASK_THING_LIST)) {
                continue;
            }
            if (!world->things->squareFirstThings) return -1;
            sftIndex = orch_square_first_thing_list_index_compat(
                world->dungeon, world->partyMapIndex, mapX, mapY);
            if (sftIndex < 0 ||
                sftIndex >= world->things->squareFirstThingCount) {
                return -1;
            }
            thing = world->things->squareFirstThings[sftIndex];
            for (guard = 0; guard < 1024 && thing != THING_NONE &&
                 thing != THING_ENDOFLIST; ++guard) {
                int groupIndex;
                const struct DungeonGroup_Compat* group;
                int wasActive;
                int eventCount;

                if (THING_GET_TYPE(thing) != THING_TYPE_GROUP) {
                    unsigned short next = orch_next_thing_compat(
                        world->things, thing);
                    if (next == THING_NONE) return -1;
                    thing = next;
                    continue;
                }
                groupIndex = (int)THING_GET_INDEX(thing);
                if (!world->things->groups || groupIndex < 0 ||
                    groupIndex >= world->things->groupCount) {
                    return -1;
                }
                group = &world->things->groups[groupIndex];
                wasActive = orch_find_active_group_state_index_compat(
                    world, groupIndex) >= 0;
                if (!orch_delete_group_reaction_events_f0181_compat(
                        world, world->partyMapIndex, mapX, mapY)) {
                    return -1;
                }
                if (world->creatureAICount < DM1_PC34_ACTIVE_GROUP_CAPACITY) {
                    if (!orch_add_generated_group_active_state_compat(
                            world, groupIndex, group, world->partyMapIndex,
                            mapX, mapY, DM1_PC34_ACTIVE_GROUP_CAPACITY)) {
                        return -1;
                    }
                }
                if (!wasActive && orch_find_active_group_state_index_compat(
                        world, groupIndex) >= 0) {
                    ++added;
                }
                eventCount = world->timeline.count;
                orch_schedule_generated_group_wandering_event_compat(
                    world, groupIndex, group, world->partyMapIndex,
                    mapX, mapY);
                if (world->timeline.count != eventCount + 1) return -1;
                break;
            }
            if (guard == 1024 && thing != THING_NONE &&
                thing != THING_ENDOFLIST) {
                return -1;
            }
        }
    }
    return added;
}

/* ReDMCSB NEWMAP.C F0003 changes the party map between GROUP.C F0194 and
 * F0195.  Both sides write source-owned state (C04 and the event heap), so
 * stage the complete ownership boundary before publishing it to M10. */
static int orch_transition_party_map_f0194_f0195_compat(
    struct GameWorld_Compat* world,
    int destinationMapIndex)
{
    struct GameWorld_Compat staged;
    struct DungeonThings_Compat stagedThings;
    struct DungeonGroup_Compat* stagedGroups = NULL;
    struct DM1ActiveGroup_Compat activeGroups[DM1_PC34_ACTIVE_GROUP_CAPACITY];
    int activeCount;
    int i;

    if (!world || !world->dungeon || !world->things ||
        !world->dungeon->maps || !world->dungeon->tiles ||
        !world->dungeon->tilesLoaded || !world->things->loaded ||
        destinationMapIndex < 0 ||
        destinationMapIndex >= (int)world->dungeon->header.mapCount ||
        world->things->groupCount < 0 ||
        world->creatureAICount < 0 ||
        world->creatureAICount > DM1_PC34_ACTIVE_GROUP_CAPACITY) {
        return 0;
    }

    activeCount = world->creatureAICount;
    if (activeCount > 0 && (!world->things->groups ||
                            world->things->groupCount == 0)) {
        return 0;
    }
    if (world->things->groupCount > 0) {
        stagedGroups = (struct DungeonGroup_Compat*)malloc(
            (size_t)world->things->groupCount * sizeof(*stagedGroups));
        if (!stagedGroups) return 0;
        memcpy(stagedGroups, world->things->groups,
               (size_t)world->things->groupCount * sizeof(*stagedGroups));
    }

    staged = *world;
    stagedThings = *world->things;
    stagedThings.groups = stagedGroups;
    staged.things = &stagedThings;
    memset(activeGroups, 0, sizeof(activeGroups));
    for (i = 0; i < DM1_PC34_ACTIVE_GROUP_CAPACITY; ++i) {
        activeGroups[i].groupThingIndex = -1;
    }

    /* F0194's raw C04 index must be a resolved original group table index.
     * Validate every live owner before F0184 gets an opportunity to write. */
    for (i = 0; i < activeCount; ++i) {
        const struct CreatureAIState_Compat* ai = &world->creatureAI[i];
        if (ai->reserved0 < 0 || ai->reserved0 >= world->things->groupCount) {
            free(stagedGroups);
            return 0;
        }
        activeGroups[i].groupThingIndex = ai->reserved0;
        activeGroups[i].directions =
            i < world->pc34ActiveGroupSourceCount
                ? world->pc34ActiveGroupDirections[i]
                : (ai->groupDirection & 0x03);
        activeGroups[i].cells = ai->groupCells;
        activeGroups[i].lastMoveTime = ai->lastSeenPartyTick;
        activeGroups[i].delayFleeingFromTarget = ai->fearCounter;
        activeGroups[i].targetMapX = ai->lastSeenPartyMapX;
        activeGroups[i].targetMapY = ai->lastSeenPartyMapY;
        activeGroups[i].priorMapX = ai->groupMapX;
        activeGroups[i].priorMapY = ai->groupMapY;
        activeGroups[i].homeMapX = world->pc34ActiveGroupHomeMapX[i];
        activeGroups[i].homeMapY = world->pc34ActiveGroupHomeMapY[i];
        memcpy(activeGroups[i].aspect, ai->aspect, sizeof(ai->aspect));
    }
    if (activeCount > 0 &&
        !F0817c_DM1_GROUP_RemoveAllActiveGroups_Compat(
            activeGroups, DM1_PC34_ACTIVE_GROUP_CAPACITY, &activeCount,
            stagedThings.groups, stagedThings.groupCount)) {
        free(stagedGroups);
        return 0;
    }
    if (activeCount != 0) {
        free(stagedGroups);
        return 0;
    }

    for (i = 0; i < DM1_PC34_ACTIVE_GROUP_CAPACITY; ++i) {
        memset(&staged.creatureAI[i], 0, sizeof(staged.creatureAI[i]));
        staged.creatureAI[i].reserved0 = -1;
    }
    staged.creatureAICount = 0;
    staged.pc34ActiveGroupSourceCount = 0;
    memset(staged.pc34ActiveGroupDirections, 0,
           sizeof(staged.pc34ActiveGroupDirections));
    memset(staged.pc34ActiveGroupHomeMapX, 0,
           sizeof(staged.pc34ActiveGroupHomeMapX));
    memset(staged.pc34ActiveGroupHomeMapY, 0,
           sizeof(staged.pc34ActiveGroupHomeMapY));
    staged.partyMapIndex = destinationMapIndex;
    staged.party.mapIndex = destinationMapIndex;
    staged.newPartyMapIndex = -1;
    if (F0195_DM1_GROUP_AddAllActiveGroups_Compat(&staged) < 0) {
        free(stagedGroups);
        return 0;
    }

    if (world->things->groupCount > 0) {
        memcpy(world->things->groups, stagedGroups,
               (size_t)world->things->groupCount * sizeof(*stagedGroups));
    }
    free(stagedGroups);
    world->partyMapIndex = staged.partyMapIndex;
    world->party.mapIndex = staged.party.mapIndex;
    world->newPartyMapIndex = staged.newPartyMapIndex;
    memcpy(world->creatureAI, staged.creatureAI, sizeof(world->creatureAI));
    world->creatureAICount = staged.creatureAICount;
    memcpy(world->pc34ActiveGroupDirections, staged.pc34ActiveGroupDirections,
           sizeof(world->pc34ActiveGroupDirections));
    memcpy(world->pc34ActiveGroupHomeMapX, staged.pc34ActiveGroupHomeMapX,
           sizeof(world->pc34ActiveGroupHomeMapX));
    memcpy(world->pc34ActiveGroupHomeMapY, staged.pc34ActiveGroupHomeMapY,
           sizeof(world->pc34ActiveGroupHomeMapY));
    world->pc34ActiveGroupSourceCount = staged.pc34ActiveGroupSourceCount;
    world->timeline = staged.timeline;
    return 1;
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
        world, groupIndex, group, mapIndex, mapX, mapY,
        GAMEWORLD_CREATURE_AI_CAPACITY);
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
    struct DM1_Event_V1 sourceEvent;
    unsigned short groupThing;

    if (!world || !ev || groupIndex < 0) return 0;

    /* ReDMCSB MOVESENS.C:F0265:169-192 creates C60/C61 at
     * G0313_ul_GameTime + 5 with destination map/x/y and the group thing
     * slot; MOVESENS.C:F0267:830-844 calls it when insertion is blocked
     * by the party or another group. */
    groupThing = orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex);
    if (!dm1v1_f0265_build_move_group_event(
            world->gameTick, (uint8_t)ev->mapIndex, (uint8_t)ev->mapX,
            (uint8_t)ev->mapY, groupThing, audible, &sourceEvent)) {
        return 0;
    }
    memset(&deferred, 0, sizeof(deferred));
    deferred.kind = sourceEvent.type == DM1_EVENT_MOVE_GROUP_AUDIBLE
        ? TIMELINE_EVENT_MOVE_GROUP_AUDIBLE
        : TIMELINE_EVENT_MOVE_GROUP_SILENT;
    deferred.fireAtTick = DM1_MAP_TIME_TIME(sourceEvent.map_time);
    deferred.mapIndex = DM1_MAP_TIME_MAP(sourceEvent.map_time);
    deferred.mapX = sourceEvent.b_mapX;
    deferred.mapY = sourceEvent.b_mapY;
    deferred.aux0 = groupIndex;
    deferred.aux1 = (int)(uint16_t)(sourceEvent.c_cell |
                                    ((uint16_t)sourceEvent.c_effect << 8));
    deferred.aux2 = sourceEvent.type;
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
    orch_write_raw_group_compat(world->things, groupIndex);

    /* ReDMCSB GROUP.C:F0185:543-545 keeps the initialized group slot
     * referenced by the deferred move event instead of linking it when
     * F0267 reports a blocked destination. */
    {
        int destMapIndex = ev->mapIndex;
        int destMapX = ev->mapX;
        int destMapY = ev->mapY;
        int fallKilledGroup = 0;
        int creatureAllowed = 0;
        int destinationBlocked = 0;
        unsigned char movingFixedDropCells[4];
        int movingFixedDropCellCount = 0;
        struct TimelineEvent_Compat resolvedEvent = *ev;
        DM1_V1_GroupMoveRoutePlanPc34 routePlan;

        (void)orch_resolve_group_f0267_teleporter_destination_compat(
            world, groupIndex, &destMapIndex, &destMapX, &destMapY,
            outTeleporterBuzzes);
        if (!orch_resolve_group_f0267_pit_destination_compat(
                world, group, &destMapIndex, &destMapX, &destMapY,
                &fallKilledGroup, movingFixedDropCells,
                &movingFixedDropCellCount, NULL)) {
            return 0;
        }
        resolvedEvent.mapIndex = destMapIndex;
        resolvedEvent.mapX = destMapX;
        resolvedEvent.mapY = destMapY;
        creatureAllowed =
            orch_is_group_creature_allowed_on_map_compat(
                world, group, destMapIndex);
        destinationBlocked =
            !fallKilledGroup && creatureAllowed &&
            orch_square_has_group_or_party_compat(
                world, destMapIndex, destMapX, destMapY);
        memset(&routePlan, 0, sizeof(routePlan));
        if (!DM1_V1_PlanDeferredGroupMoveRouteF0267Pc34Compat(
                fallKilledGroup, creatureAllowed, destinationBlocked,
                0, 0, world->gameTick, destMapX, destMapY, 0, 0,
                &routePlan) ||
            !routePlan.valid) {
            return 0;
        }

        if (routePlan.route == DM1_V1_GROUP_MOVE_ROUTE_REMOVE_PC34) {
            if (!fallKilledGroup) {
                (void)orch_drop_moving_fixed_possessions_compat(
                    world, group->creatureType, movingFixedDropCells,
                    movingFixedDropCellCount, destMapIndex,
                    routePlan.mapX, routePlan.mapY);
            }
            if (!orch_apply_group_move_removal_plan_f0267_compat(
                    world, group,
                    fallKilledGroup ? movingFixedDropCells : NULL,
                    fallKilledGroup ? movingFixedDropCellCount : 0,
                    fallKilledGroup, creatureAllowed, -1, 0,
                    destMapIndex, routePlan.mapX, routePlan.mapY)) {
                return 0;
            }
            if (outGroupIndex) *outGroupIndex = groupIndex;
            return 0;
        }
        (void)orch_drop_moving_fixed_possessions_compat(
            world, group->creatureType, movingFixedDropCells,
            movingFixedDropCellCount, destMapIndex, destMapX, destMapY);

        if (routePlan.route == DM1_V1_GROUP_MOVE_ROUTE_RETRY_PC34) {
            if (!orch_schedule_deferred_group_move_compat(world, &resolvedEvent, groupIndex, 0)) {
                return 0;
            }
            if (outGroupIndex) *outGroupIndex = groupIndex;
            return 0;
        }

        if (!orch_link_existing_group_to_square_compat(
                world, groupIndex, destMapIndex,
                routePlan.mapX, routePlan.mapY)) {
            return 0;
        }
        if (outGroupIndex) *outGroupIndex = groupIndex;
        return 1;
    }
}

static int orch_handle_deferred_group_move_event_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result,
    int sourceSquareKnown)
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
        world, groupIndex, &retry.mapIndex, &targetMapX, &targetMapY,
        &teleporterBuzzes);
    {
        int fallKilledGroup = 0;
        int creatureAllowed = 0;
        int destinationBlocked = 0;
        int finalKillCell = EXPLOSION_CELL_CENTERED;
        int chaosAdjacentAvailable = 0;
        int chaosAdjacentMapX = targetMapX;
        int chaosAdjacentMapY = targetMapY;
        unsigned char movingFixedDropCells[4];
        int movingFixedDropCellCount = 0;
        DM1_V1_GroupMoveRoutePlanPc34 routePlan;
        if (!orch_resolve_group_f0267_pit_destination_compat(
                world, group, &retry.mapIndex, &targetMapX, &targetMapY,
                &fallKilledGroup, movingFixedDropCells,
                &movingFixedDropCellCount, &finalKillCell)) {
            return 0;
        }
        creatureAllowed =
            orch_is_group_creature_allowed_on_map_compat(
                world, group, retry.mapIndex);
        if (!fallKilledGroup && creatureAllowed) {
            (void)orch_drop_moving_fixed_possessions_compat(
                world, group->creatureType, movingFixedDropCells,
                movingFixedDropCellCount, retry.mapIndex,
                targetMapX, targetMapY);
        }
        destinationBlocked =
            !fallKilledGroup && creatureAllowed &&
            orch_square_has_group_or_party_compat(
                world, retry.mapIndex, targetMapX, targetMapY);
        if (destinationBlocked &&
            orch_try_lord_chaos_random_adjacent_retry_compat(
                world, group, ev, &chaosAdjacentMapX, &chaosAdjacentMapY) &&
            !orch_square_has_group_or_party_compat(
                world, ev->mapIndex, chaosAdjacentMapX, chaosAdjacentMapY)) {
            chaosAdjacentAvailable = 1;
        }
        memset(&routePlan, 0, sizeof(routePlan));
        if (!DM1_V1_PlanDeferredGroupMoveRouteF0267Pc34Compat(
                fallKilledGroup, creatureAllowed, destinationBlocked,
                ev->kind == TIMELINE_EVENT_MOVE_GROUP_AUDIBLE,
                chaosAdjacentAvailable, ev->fireAtTick, targetMapX,
                targetMapY, chaosAdjacentMapX, chaosAdjacentMapY,
                &routePlan) ||
            !routePlan.valid) {
            return 0;
        }
        if (routePlan.route == DM1_V1_GROUP_MOVE_ROUTE_REMOVE_PC34) {
            if (routePlan.shouldEmitAudibleBuzz) {
                emit(result, EMIT_SOUND_REQUEST, DM1_SND_BUZZ,
                     routePlan.mapX, routePlan.mapY, retry.mapIndex);
            }
            orch_emit_teleporter_buzzes_compat(result, &teleporterBuzzes);
            if (fallKilledGroup) {
                orch_apply_moving_killed_all_afterplay_f0190_compat(
                    world, groupIndex, group, sourceSquareKnown,
                    ev->mapIndex, ev->mapX, ev->mapY, finalKillCell,
                    retry.mapIndex, routePlan.mapX, routePlan.mapY);
            }
            return orch_apply_group_move_removal_plan_f0267_compat(
                world, group,
                fallKilledGroup ? movingFixedDropCells : NULL,
                fallKilledGroup ? movingFixedDropCellCount : 0,
                fallKilledGroup, creatureAllowed, -1, 0,
                retry.mapIndex, routePlan.mapX, routePlan.mapY);
        }
        if (routePlan.route == DM1_V1_GROUP_MOVE_ROUTE_RETRY_PC34) {
            retry.fireAtTick = routePlan.retryFireAtTick;
            retry.mapX = routePlan.mapX;
            retry.mapY = routePlan.mapY;
            return F0721_TIMELINE_Schedule_Compat(&world->timeline, &retry);
        }
        if (routePlan.shouldEmitAudibleBuzz) {
            int buzzMapIndex =
                routePlan.route == DM1_V1_GROUP_MOVE_ROUTE_CHAOS_ADJACENT_INSERT_PC34
                    ? ev->mapIndex
                    : retry.mapIndex;
            emit(result, EMIT_SOUND_REQUEST, DM1_SND_BUZZ,
                 routePlan.mapX, routePlan.mapY, buzzMapIndex);
        }
        if (routePlan.route != DM1_V1_GROUP_MOVE_ROUTE_CHAOS_ADJACENT_INSERT_PC34) {
            orch_emit_teleporter_buzzes_compat(result, &teleporterBuzzes);
        }
        return orch_link_existing_group_to_square_compat(
            world, groupIndex,
            routePlan.route == DM1_V1_GROUP_MOVE_ROUTE_CHAOS_ADJACENT_INSERT_PC34
                ? ev->mapIndex
                : retry.mapIndex,
            routePlan.mapX, routePlan.mapY);
    }
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

/* ReDMCSB TIMELINE.C F0249:1387-1401 locates the C04 group before every
 * ordinary Thing, then re-enters MOVESENS.C F0267 from its real source
 * square.  That distinction matters: F0267 invokes F0266 on a group moving
 * on the party map, whereas C006/C60/C61 use MAPX_NOT_ON_A_SQUARE and skip
 * that projectile preflight.  After the source group is unlinked, the
 * existing C60/C61 owner performs the same teleporter/pit, destination and
 * retry handoff as the remainder of F0267. */
static int orch_f0249_move_group_first_square_thing_compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    struct TickResult_Compat* result)
{
    int sftIndex;
    unsigned short thing;
    int safety = 0;
    int groupIndex = -1;
    int killedByProjectile = 0;
    struct TimelineEvent_Compat groupMove;

    if (!world || !world->dungeon || !world->things ||
        !world->things->groups || !result) {
        return 0;
    }
    sftIndex = orch_square_first_thing_list_index_compat(
        world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= world->things->squareFirstThingCount) {
        return 0;
    }
    thing = world->things->squareFirstThings[sftIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_GROUP) {
            int index = THING_GET_INDEX(thing);
            if (index >= 0 && index < world->things->groupCount) {
                groupIndex = index;
                break;
            }
        }
        thing = orch_next_thing_compat(world->things, thing);
    }
    if (groupIndex < 0) return 1;

    if (!orch_apply_f0266_group_projectile_precheck_compat(
            world, groupIndex, mapIndex, mapX, mapY, mapX, mapY,
            &killedByProjectile)) {
        return 0;
    }
    if (killedByProjectile) {
        /* F0217's shared F0190 aftermath has already unlinked the group,
         * cleaned C29-C41, and retired active state.  F0249 must not try to
         * relink or delete the now-dead source record a second time. */
        return 1;
    }

    if (!orch_unlink_thing_from_square_compat(
            world, mapIndex, mapX, mapY,
            orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex))) {
        return 0;
    }
    memset(&groupMove, 0, sizeof(groupMove));
    groupMove.kind = TIMELINE_EVENT_MOVE_GROUP_SILENT;
    groupMove.fireAtTick = world->gameTick;
    groupMove.mapIndex = mapIndex;
    groupMove.mapX = mapX;
    groupMove.mapY = mapY;
    groupMove.aux0 = groupIndex;
    return orch_handle_deferred_group_move_event_compat(
        world, &groupMove, result, 1);
}

/* ReDMCSB TIMELINE.C F0249 moves the resident C04 before C05..C15 when a
 * C08/C09 square opens. This narrow runtime owner consumes only an admitted
 * loaded C04 and the existing F0267 teleporter resolver; blocked or
 * disallowed destinations remain untouched for their C60/C61 owner. */
static int orch_handle_creature_reaction_event_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result);

/* ReDMCSB GROUP.C F0209 rejects off-party-map C29-C41 entries before it
 * reads C04. Only C32, C33, C37 and C38 reach the native off-map path. */
static int orch_f0209_off_party_map_event_is_admitted_compat(int eventType)
{
    return eventType == DM1_EVENT_UPDATE_ASPECT_GROUP ||
           eventType == DM1_EVENT_UPDATE_ASPECT_CREATURE_0 ||
           eventType == DM1_EVENT_UPDATE_BEHAVIOR_GROUP ||
           eventType == DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
}

/* ReDMCSB GROUP.C F0209 reaches MOVESENS.C F0267 after C29-C36 and
 * C38-C41 select an ordinary one-square move.  C37 already has its own
 * tick route below; keep these reaction branches in M10 so their source
 * event plan, rather than the C37 retry owner, controls the next event. */

/* ReDMCSB GROUP.C F0209 lines 2173-2185 selects a direction through the
 * visibility/behavior chain, then enters MOVESENS.C F0267 from the source
 * square.  Keep the physical half separate from timeline routing so C37 can
 * consume that decision without re-dispatching F0209. */
static int orch_apply_creature_tick_group_move_f0267_compat(
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
    int destinationPassable;
    int destinationBlocked;
    struct DungeonGroup_Compat* group;
    struct TimelineEvent_Compat nextEvent;
    DM1_V1_OrdinaryGroupMovePlanPc34 movePlan;
    DM1_V1_OrdinaryGroupMoveApplyPlanPc34 applyPlan;
    struct OrchThingUnlinkReceipt_Compat unlinkReceipt;

    (void)result;
    if (!world || !ev || !world->things || !world->dungeon) return 0;

    groupIndex = ev->aux0;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount || !world->things->groups) return 0;
    activeIndex = orch_find_active_group_state_index_compat(world, groupIndex);
    if (activeIndex < 0) return 1;
    group = &world->things->groups[groupIndex];
    direction = world->creatureAI[activeIndex].groupDirection & 3;
    memset(&movePlan, 0, sizeof(movePlan));
    if (!DM1_V1_PlanOrdinaryGroupMoveF0267Pc34Compat(
            ev->mapX, ev->mapY, direction, 1, 0, 0,
            world->gameTick, &movePlan) ||
        !movePlan.valid) {
        return 0;
    }
    destMapX = movePlan.destinationMapX;
    destMapY = movePlan.destinationMapY;

    destinationPassable = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
        world->dungeon, ev->mapIndex, destMapX, destMapY,
        MOVEMENT_PASS_CTX_CREATURE);
    destinationBlocked = orch_square_has_group_or_party_compat(
        world, ev->mapIndex, destMapX, destMapY);
    if (!DM1_V1_PlanOrdinaryGroupMoveF0267Pc34Compat(
            ev->mapX, ev->mapY, direction, destinationPassable,
            destinationBlocked, 0, world->gameTick, &movePlan) ||
        !movePlan.valid) {
        return 0;
    }
    if (movePlan.route != DM1_V1_GROUP_MOVE_ROUTE_RETRY_PC34) {
        if (!orch_apply_f0266_group_projectile_precheck_compat(
                world, groupIndex, ev->mapIndex, ev->mapX, ev->mapY,
                destMapX, destMapY, &killedByProjectile)) {
            return 0;
        }
        if (!DM1_V1_PlanOrdinaryGroupMoveF0267Pc34Compat(
                ev->mapX, ev->mapY, direction, destinationPassable,
                destinationBlocked, killedByProjectile, world->gameTick,
                &movePlan) ||
            !movePlan.valid) {
            return 0;
        }
        if (killedByProjectile) {
            return 1;
        }
    }
    memset(&applyPlan, 0, sizeof(applyPlan));
    if (!DM1_V1_PlanOrdinaryGroupMoveApplyF0267Pc34Compat(
            &movePlan, ev->mapIndex, ev->mapX, ev->mapY, direction, group->cells,
            world->gameTick, &applyPlan) ||
        !applyPlan.valid) {
        return 0;
    }
    if (applyPlan.shouldRemoveActiveGroup) {
        (void)orch_unlink_thing_from_square_compat(
            world, applyPlan.sourceMapIndex, applyPlan.sourceMapX,
            applyPlan.sourceMapY,
            orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex));
        group->next = THING_NONE;
        orch_remove_active_group_state_compat(world, groupIndex);
        return 1;
    }

    memset(&unlinkReceipt, 0, sizeof(unlinkReceipt));
    if (applyPlan.shouldUnlinkSource &&
        !orch_unlink_thing_from_square_with_receipt_compat(
                world, applyPlan.sourceMapIndex, applyPlan.sourceMapX,
                applyPlan.sourceMapY,
                orch_make_thing_ref_compat(THING_TYPE_GROUP, groupIndex),
                &unlinkReceipt)) {
        return 0;
    }
    if (applyPlan.shouldLinkDestination &&
        !orch_link_existing_group_to_square_head_only_compat(
            world, groupIndex, applyPlan.activeMapIndex,
            applyPlan.activeMapX, applyPlan.activeMapY)) {
        if (applyPlan.shouldUnlinkSource &&
            !orch_restore_unlinked_thing_receipt_compat(world, &unlinkReceipt)) {
            return 0;
        }
        return 1;
    }
    group->direction = (unsigned char)applyPlan.groupDirection;

    world->creatureAI[activeIndex].groupMapIndex = applyPlan.activeMapIndex;
    world->creatureAI[activeIndex].groupMapX = applyPlan.activeMapX;
    world->creatureAI[activeIndex].groupMapY = applyPlan.activeMapY;
    world->creatureAI[activeIndex].groupCells = applyPlan.activeCells;

    if (!applyPlan.shouldRequeue) return 1;
    nextEvent = *ev;
    nextEvent.fireAtTick = applyPlan.nextFireAtTick;
    nextEvent.mapIndex = applyPlan.nextEventMapIndex;
    nextEvent.mapX = applyPlan.nextEventMapX;
    nextEvent.mapY = applyPlan.nextEventMapY;
    return F0721_TIMELINE_Schedule_Compat(&world->timeline, &nextEvent);
}

static int orch_handle_creature_tick_group_move_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct TickResult_Compat* result)
{
    /* C37 is the F0180 wander event as well as a F0209 behavior update.
     * ReDMCSB GROUP.C F0209 owns the decision first; only its selected move
     * enters MOVESENS.C F0267. */
    if (ev && ev->aux2 >= DM1_EVENT_REACTION_DANGER_ON_SQUARE &&
        ev->aux2 <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
        return orch_handle_creature_reaction_event_compat(world, ev, result);
    }
    return orch_apply_creature_tick_group_move_f0267_compat(world, ev, result);
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

static int orch_apply_f0206_active_group_directions_compat(
    struct GameWorld_Compat* world,
    struct CreatureAIState_Compat* ai,
    struct DungeonGroup_Compat* group,
    struct DM1ActiveGroup_Compat* activeGroup,
    int activeIndex,
    int direction,
    int creatureSize)
{
    if (!world || !ai || !group || !activeGroup || activeIndex < 0 ||
        direction < 0 || direction > 3) {
        return 0;
    }
    if (!F0817a_DM1_GROUP_SetGroupDirectionsWithRng_Compat(
            activeGroup, direction, creatureSize, (int)group->count,
            &world->masterRng)) {
        return 0;
    }
    /* C04 holds its primary direction, while ACTIVE_GROUP retains all four
     * F0205-packed creature directions between C29-C41 events. */
    group->direction = (unsigned char)(activeGroup->directions & 0x03);
    ai->groupDirection = group->direction;
    if (activeIndex < world->pc34ActiveGroupSourceCount) {
        world->pc34ActiveGroupDirections[activeIndex] =
            (unsigned char)activeGroup->directions;
    }
    /* F0206 updates C04's primary direction together with ACTIVE_GROUP's
     * packed F0205 directions. C30 can take this route after a projectile
     * impact, before the next C38 reads the active-group receipt. */
    orch_write_raw_group_compat(world->things, activeGroup->groupThingIndex);
    return 1;
}

static int orch_apply_f0205_creature_turn_retry_compat(
    struct GameWorld_Compat* world, const struct TimelineEvent_Compat* ev,
    struct CreatureAIState_Compat* ai, struct DungeonGroup_Compat* group,
    struct DM1ActiveGroup_Compat* activeGroup, int activeIndex,
    int creatureIndex, int direction, int creatureSize)
{
    struct DM1ActiveGroup_Compat staged;
    struct RngState_Compat stagedRng;
    struct TimelineEvent_Compat retry;

    if (!world || !ev || !ai || !group || !activeGroup || activeIndex < 0 ||
        creatureIndex < 0 || creatureIndex > (int)group->count ||
        direction < 0 || direction > 3) return 0;
    if (((activeGroup->directions >> (creatureIndex * 2)) & 3) == direction) return 0;
    staged = *activeGroup;
    stagedRng = world->masterRng;
    if (!F0817b_DM1_GROUP_SetCreatureDirectionWithRng_Compat(
            &staged, direction, creatureIndex, creatureSize, (int)group->count,
            &stagedRng)) return 0;
    retry = *ev;
    retry.fireAtTick = world->gameTick + 2u;
    if (!F0721_TIMELINE_Schedule_Compat(&world->timeline, &retry)) return 0;
    *activeGroup = staged;
    world->masterRng = stagedRng;
    group->direction = (unsigned char)(staged.directions & 3);
    ai->groupDirection = group->direction;
    if (activeIndex < world->pc34ActiveGroupSourceCount)
        world->pc34ActiveGroupDirections[activeIndex] = (unsigned char)staged.directions;
    orch_write_raw_group_compat(world->things, activeGroup->groupThingIndex);
    return 1;
}

static int orch_apply_f0209_reaction_move_f0267_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    int groupIndex,
    const struct DM1BehaviorResult_Compat* behavior,
    int* outReactionMoveHandled,
    int* outGroupRemoved)
{
    if (outReactionMoveHandled) *outReactionMoveHandled = 0;
    if (outGroupRemoved) *outGroupRemoved = 0;
    if (!world || !ev || !behavior || groupIndex != ev->aux0) return 0;
    if (behavior->actionKind != DM1_ACTION_MOVE &&
        behavior->actionKind != DM1_ACTION_FLEE_MOVE) {
        return 1;
    }

    /* GROUP.C F0209 reaches MOVESENS.C F0267 only after its authenticated
     * C29-C41 direction decision.  Do not manufacture a group move when
     * the physical source owner refuses the loaded C04/SFT state. */
    if (!orch_apply_creature_tick_group_move_f0267_compat(world, ev, NULL)) {
        return 0;
    }
    if (outReactionMoveHandled) *outReactionMoveHandled = 1;
    return 1;
}

/* GROUP.C F0209 consumes an event only when its C04 owner is still the
 * group on the event square.  The runtime mirror is not sufficient: a
 * stale timeline entry must not create ACTIVE_GROUP state or reanimate a
 * raw group that no longer owns that SFT chain. */
static int orch_f0209_authenticate_reaction_source_compat(
    const struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    int groupIndex,
    int activeIndex)
{
    const struct DungeonGroup_Compat* group;
    const unsigned char* raw;
    unsigned short thing;
    int groupMatches = 0;
    int safety = 0;

    if (!world || !ev || !world->dungeon || !world->things ||
        !world->dungeon->loaded || !world->things->loaded ||
        !world->things->groups || activeIndex < 0 ||
        ev->kind != TIMELINE_EVENT_CREATURE_REACTION ||
        ev->aux2 < DM1_EVENT_REACTION_DANGER_ON_SQUARE ||
        ev->aux2 > DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3 ||
        ev->aux0 != groupIndex || groupIndex < 0 ||
        groupIndex >= world->things->groupCount ||
        !world->things->rawThingData[THING_TYPE_GROUP] ||
        groupIndex >= world->things->thingCounts[THING_TYPE_GROUP] ||
        world->creatureAI[activeIndex].reserved0 != groupIndex ||
        world->creatureAI[activeIndex].groupMapIndex != ev->mapIndex ||
        world->creatureAI[activeIndex].groupMapX != ev->mapX ||
        world->creatureAI[activeIndex].groupMapY != ev->mapY ||
        activeIndex >= world->pc34ActiveGroupSourceCount) {
        return 0;
    }

    group = &world->things->groups[groupIndex];
    raw = world->things->rawThingData[THING_TYPE_GROUP] + groupIndex * 16;
    /* The C04 Thing identity is immutable for the lifetime of this event.
     * Its mutable cells/health/behavior bytes may have been changed by an
     * earlier authenticated event in the same dispatch pass. */
    if (r_u16(raw) != group->next || r_u16(raw + 2) != group->slot ||
        raw[4] != group->creatureType || ev->aux1 != group->creatureType) {
        return 0;
    }

    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        world->dungeon, world->things, ev->mapIndex, ev->mapX, ev->mapY);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_GROUP &&
            THING_GET_INDEX(thing) == groupIndex) {
            ++groupMatches;
        }
        thing = orch_next_thing_compat(world->things, thing);
    }
    return groupMatches == 1 && safety < 64;
}

/* ReDMCSB GROUP.C F0208 prepares EVENT.Map_Time/C.Ticks and TIMELINE.C
 * F0238 inserts that exact event.  Keep C33-C36's delayed C38-C41 handoff
 * atomic with F0179's source timestamp: queue failure restores the aspect
 * and RNG so no unauthenticated future event becomes observable. */
static int orch_f0209_insert_next_event_f0238_compat(
    struct GameWorld_Compat* world,
    const struct DM1BehaviorReactionApplyPlan_Compat* applyPlan,
    struct DM1ActiveGroup_Compat* activeGroup,
    struct CreatureAIState_Compat* ai,
    const struct DungeonGroup_Compat* group,
    const struct DM1CreatureInfo_Compat* creatureInfo)
{
    struct TimelineEvent_Compat next;
    struct TimelineQueue_Compat timelineBefore;
    struct RngState_Compat rngBefore;
    struct DM1GroupAddEventPlan_Compat addPlan;
    DM1_V1_F0179_CreatureAspectUpdateReceipt_PC34 aspectReceipt;
    int aspectBefore[4];
    uint32_t requestedTime;
    int creatureIndex;

    if (!world || !applyPlan || !activeGroup || !ai || !group ||
        !creatureInfo || !applyPlan->shouldScheduleNextEvent ||
        applyPlan->nextEventType < DM1_EVENT_REACTION_DANGER_ON_SQUARE ||
        applyPlan->nextEventType > DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
        return 0;
    }

    requestedTime = applyPlan->nextEventFireAtTick;
    timelineBefore = world->timeline;
    rngBefore = world->masterRng;
    memcpy(aspectBefore, activeGroup->aspect, sizeof(aspectBefore));
    memset(&aspectReceipt, 0, sizeof(aspectReceipt));

    if (applyPlan->nextEventType >= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
        applyPlan->nextEventType <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
        creatureIndex = applyPlan->nextEventType -
            DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
        if (!F0179_DM1_GROUP_GetCreatureAspectUpdateTime_Compat(
                activeGroup, group, creatureInfo, creatureIndex, 1,
                world->gameTick, &world->masterRng, &aspectReceipt) ||
            !aspectReceipt.valid) {
            world->masterRng = rngBefore;
            memcpy(activeGroup->aspect, aspectBefore, sizeof(aspectBefore));
            return 0;
        }
        requestedTime = aspectReceipt.next_update_time;
    }

    if (!F0208_DM1_GROUP_BuildAddEventPlan_Compat(
            applyPlan->nextEventType, applyPlan->nextEventFireAtTick,
            requestedTime, &addPlan) || !addPlan.valid ||
        addPlan.eventType < DM1_EVENT_REACTION_DANGER_ON_SQUARE ||
        addPlan.eventType > DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
        world->masterRng = rngBefore;
        memcpy(activeGroup->aspect, aspectBefore, sizeof(aspectBefore));
        return 0;
    }

    memset(&next, 0, sizeof(next));
    next.kind = TIMELINE_EVENT_CREATURE_REACTION;
    next.fireAtTick = addPlan.mapTime;
    next.mapIndex = applyPlan->nextEventMapIndex;
    next.mapX = applyPlan->nextEventMapX;
    next.mapY = applyPlan->nextEventMapY;
    next.aux0 = applyPlan->nextEventGroupIndex;
    next.aux1 = applyPlan->nextEventCreatureType;
    next.aux2 = addPlan.eventType;
    next.aux3 = (int)addPlan.ticks;
    next.aux4 = 0x100; /* EVENT.C.Ticks is source-owned, never inferred. */
    if (!F0721_TIMELINE_Schedule_Compat(&world->timeline, &next)) {
        world->timeline = timelineBefore;
        world->masterRng = rngBefore;
        memcpy(activeGroup->aspect, aspectBefore, sizeof(aspectBefore));
        return 0;
    }

    memcpy(ai->aspect, activeGroup->aspect, sizeof(ai->aspect));
    return 1;
}

/* ReDMCSB GROUP.C F0207 lines 1695-1815 performs the action selected by
 * F0209's C38-C41 branch.  F0810 already owns the behavior decision; this
 * M10 bridge owns only the live projectile/champion mutation and receipt. */
static int orch_apply_f0207_creature_attack_compat(
    struct GameWorld_Compat* world,
    const struct TimelineEvent_Compat* ev,
    struct DungeonGroup_Compat* group,
    const struct DM1GroupBehaviorContext_Compat* ctx,
    const struct DM1ActiveGroup_Compat* activeGroup,
    const struct DM1BehaviorResult_Compat* behavior,
    struct TickResult_Compat* result)
{
    int creatureIndex;

    if (!world || !ev || !group || !ctx || !activeGroup || !behavior) return 0;
    if (behavior->actionKind != DM1_ACTION_ATTACK) return 1;
    creatureIndex = ev->aux2 - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    if (creatureIndex < 0 || creatureIndex > (int)group->count) return 1;

    if (behavior->attackIsProjectile) {
        DM1_CreatureProjectileCreateRequestPc34 request;
        struct ProjectileCreateInput_Compat input;
        struct TimelineEvent_Compat firstMove;
        struct ProjectileList_Compat projectilesBefore;
        struct TimelineQueue_Compat timelineBefore;
        DM1_C14PoolReservationPc34 c14;
        int slot = -1;
        int eventIndex = -1;
        int i;

        memset(&request, 0, sizeof(request));
        request.creatureGroupIndex = ev->aux0;
        request.partyMapIndex = ev->mapIndex;
        request.groupMapX = ev->mapX;
        request.groupMapY = ev->mapY;
        request.projectileThing = behavior->projectileThing;
        request.targetCell = behavior->attackTargetCell;
        request.direction = behavior->projectileDirection;
        request.kineticEnergy = behavior->projectileKineticEnergy;
        request.attack = behavior->projectileAttack;
        request.stepEnergy = behavior->projectileStepEnergy;
        request.gameTick = (int)world->gameTick;
        memset(&input, 0, sizeof(input));
        memset(&firstMove, 0, sizeof(firstMove));
        if (!world->things || !world->dungeon ||
            !dm1_v1_c14_pool_reserve_pc34(world->things, &c14)) return 0;
        projectilesBefore = world->projectiles;
        timelineBefore = world->timeline;
        if (!dm1_v1_build_creature_projectile_create_input_pc34(&request, &input) ||
            !F0810_PROJECTILE_Create_Compat(
                &input, &world->projectiles, &slot, &firstMove)) {
            (void)dm1_v1_c14_pool_rollback_pc34(&c14);
            return 0;
        }
        if (THING_GET_INDEX(c14.thing) != slot) {
            world->projectiles = projectilesBefore;
            (void)dm1_v1_c14_pool_rollback_pc34(&c14);
            return 0;
        }
        /* ReDMCSB PROJEXPL.C:F0212 links the projectile and its first C48/C49
         * movement event as one live handoff.  A queue rejection must not
         * leave a renderable C14-equivalent slot with no owner event. */
        if (!F0721_TIMELINE_Schedule_Compat(&world->timeline, &firstMove)) {
            world->projectiles = projectilesBefore;
            world->timeline = timelineBefore;
            (void)dm1_v1_c14_pool_rollback_pc34(&c14);
            return 0;
        }
        for (i = 0; i < world->timeline.count; ++i) {
            if (world->timeline.events[i].kind == TIMELINE_EVENT_PROJECTILE_MOVE &&
                world->timeline.events[i].aux0 == slot &&
                world->timeline.events[i].fireAtTick == firstMove.fireAtTick) {
                if (eventIndex >= 0) { eventIndex = -2; break; }
                eventIndex = i;
            }
        }
        if (eventIndex < 0 || !dm1_v1_c14_pool_initialize_and_link_pc34(
                &c14, world->dungeon, (unsigned short)behavior->projectileThing,
                input.kineticEnergy, input.attack, (unsigned short)eventIndex,
                input.cell, input.mapIndex, input.mapX, input.mapY)) {
            world->projectiles = projectilesBefore;
            world->timeline = timelineBefore;
            (void)dm1_v1_c14_pool_rollback_pc34(&c14);
            return 0;
        }
        emit(result, EMIT_CREATURE_ATTACK, ev->aux0, creatureIndex, slot, 1);
        return 1;
    }

    {
        int targetCell;
        int targetChampion = -1;
        int i;
        DM1_MeleeTargetAdmissionInputPc34 targetAdmission;
        DM1_MeleeTargetAdmissionReceiptPc34 targetReceipt;
        struct CombatantCreatureSnapshot_Compat attacker;
        struct CombatantChampionSnapshot_Compat defender;
        struct CombatResult_Compat combat;

        /* F0229 orders party cells before F0230 begins its damage work.
         * The admission previews that source selection, but deliberately
         * leaves the F0230/F0304 consumer and its live RNG ownership intact. */
        memset(&targetAdmission, 0, sizeof(targetAdmission));
        memset(&targetReceipt, 0, sizeof(targetReceipt));
        targetAdmission.things = world->things;
        targetAdmission.groupIndex = ev->aux0;
        targetAdmission.group = group;
        targetAdmission.activeGroup = activeGroup;
        targetAdmission.event = ev;
        targetAdmission.partyMapIndex = world->partyMapIndex;
        targetAdmission.partyMapX = world->party.mapX;
        targetAdmission.partyMapY = world->party.mapY;
        targetAdmission.creatureIndex = creatureIndex;
        targetAdmission.rng = &world->masterRng;
        if (!dm1_v1_melee_target_admit_f0229_f0230_pc34(
                &targetAdmission, &targetReceipt) || !targetReceipt.valid) {
            return 1;
        }

        /* GROUP.C F0207 derives the party-facing target cell before it
         * branches between ranged and melee. */
        if (activeGroup->cells == DM1_SINGLE_CENTERED_CREATURE_CELL) {
            targetCell = F0732_COMBAT_RngRandom_Compat(&world->masterRng, 2);
        } else {
            targetCell = (((activeGroup->cells >> (creatureIndex * 2)) +
                           5 - ctx->currentGroupPrimaryDirToParty) & 2) >> 1;
        }
        targetCell = (targetCell + ctx->currentGroupPrimaryDirToParty) & 3;

        if (ctx->creatureInfo.attributes & DM1_ATTR_ATTACK_ANY_CHAMPION) {
            targetChampion = F0732_COMBAT_RngRandom_Compat(&world->masterRng, 4);
            for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
                int candidate = (targetChampion + i) & 3;
                if (world->party.champions[candidate].present &&
                    world->party.champions[candidate].hp.current > 0) {
                    targetChampion = candidate;
                    break;
                }
            }
            if (i == CHAMPION_MAX_PARTY) return 1;
        } else {
            targetChampion = F0286_CHAMPION_GetTargetChampionIndex_Compat(
                &world->party, ev->mapX, ev->mapY,
                (unsigned int)targetCell, &world->masterRng);
            if (targetChampion < 0) return 1;
        }

        memset(&attacker, 0, sizeof(attacker));
        memset(&defender, 0, sizeof(defender));
        memset(&combat, 0, sizeof(combat));
        if (!F0888_ORCH_GetCreatureSnapshot_Compat(
                world, ev->aux0, creatureIndex,
                orch_cmd_attack_doubled_map_difficulty_compat(world), &attacker) ||
            !orch_build_defender_champion_snapshot_compat(
                world, targetChampion, attacker.attackType, &defender) ||
            !F0736_COMBAT_ResolveCreatureMelee_Compat(
                &attacker, &defender, &world->masterRng, &combat)) {
            return 0;
        }
        orch_writeback_cmd_attack_luck_compat(world, targetChampion, &defender);
        if (combat.wakeFromRest) {
            world->partyIsResting = 0;
            world->lifecycle.rest.isResting = 0;
        }
        /* ReDMCSB PROJEXPL.C F0230 calls F0304(PARRY,
         * CREATURE_INFO.Properties experience) for every authenticated
         * living champion target, before the damage branch.  We consume the
         * bound receipt after the resolver has produced its real action
         * result so the event/champion/result identity is observable, while
         * retaining the source's hit-or-miss XP semantics. */
        (void)F0890a_ORCH_ConsumeF0230F0304Parry_Compat(
            world, ev, group, &attacker, targetChampion, creatureIndex,
            &combat, result);
        if (combat.damageApplied > 0) {
            /* ReDMCSB GROUP.C F0207 lines 1788-1797 chooses this champion
             * and cell before F0230 calls F0321.  Keep the receipt until
             * F0889's same-tick damage boundary rather than collapsing it
             * onto PartyState.activeChampionIndex. */
            orch_stage_champion_combat_compat(world, targetChampion,
                                              targetCell, &combat);
        }
        emit(result, EMIT_CREATURE_ATTACK, ev->aux0, creatureIndex,
             combat.damageApplied, 0);
    }
    return 1;
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
    struct DM1BehaviorReactionApplyPlan_Compat applyPlan;
    int reactionMoveHandled = 0;
    int cellsBeforeBehavior;
    int creatureCountBeforeBehavior;

    (void)result;
    if (!world || !ev || !world->things || !world->things->groups) return 0;
    if (ev->mapIndex != world->partyMapIndex &&
        !orch_f0209_off_party_map_event_is_admitted_compat(ev->aux2)) {
        /* Source-order fence: do not consume RNG or inspect a C04 receipt
         * for an event ReDMCSB ignores before group lookup. */
        return 1;
    }
    groupIndex = ev->aux0;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount) return 0;
    group = &world->things->groups[groupIndex];
    if (group->next == THING_NONE) return 1;
    cellsBeforeBehavior = group->cells;
    creatureCountBeforeBehavior = group->count;

    activeIndex = orch_find_active_group_state_index_compat(world, groupIndex);
    if (activeIndex < 0 ||
        !orch_f0209_authenticate_reaction_source_compat(
            world, ev, groupIndex, activeIndex)) {
        /* A stale C29-C41 record is consumed without side effects.  F0209
         * may not synthesize ACTIVE_GROUP from a decoded C04 mirror. */
        return 1;
    }

    ai = &world->creatureAI[activeIndex];
    memset(&ctx, 0, sizeof(ctx));
    memset(&activeGroup, 0, sizeof(activeGroup));
    memset(&behavior, 0, sizeof(behavior));
    memset(&applyPlan, 0, sizeof(applyPlan));

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
    if (!orch_get_dm1_creature_info_pc34_compat(group->creatureType,
                                                 &ctx.creatureInfo)) {
        return 0;
    }
    ctx.creatureSize = ctx.creatureInfo.attributes & DM1_ATTR_SIZE_MASK;
    ctx.isArchenemy = (ctx.creatureInfo.attributes & DM1_ATTR_ARCHENEMY) != 0;
    ctx.freezeLifeTicks = world->freezeLifeTicks;
    ctx.movementTicks = ctx.creatureInfo.movementTicks;
    ctx.currentGroupDistanceToParty =
        F0226_DM1_GROUP_GetDistanceBetweenSquares_Compat(
            ctx.currentGroupMapX, ctx.currentGroupMapY,
            ctx.partyMapX, ctx.partyMapY);
    if (!F0228_DM1_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource_Compat(
            ctx.currentGroupMapX, ctx.currentGroupMapY,
            ctx.partyMapX, ctx.partyMapY, &world->masterRng,
            &ctx.currentGroupPrimaryDirToParty,
            &ctx.currentGroupSecondaryDirToParty)) {
        return 0;
    }
    activeGroup.groupThingIndex = groupIndex;
    activeGroup.cells = group->cells;
    activeGroup.directions = world->pc34ActiveGroupDirections[activeIndex];
    activeGroup.lastMoveTime = ai->lastSeenPartyTick;
    activeGroup.targetMapX = ai->lastSeenPartyMapX;
    activeGroup.targetMapY = ai->lastSeenPartyMapY;
    activeGroup.priorMapX = ai->groupMapX;
    activeGroup.priorMapY = ai->groupMapY;
    memcpy(activeGroup.aspect, ai->aspect, sizeof(activeGroup.aspect));

    ctx.distanceToVisibleParty =
        F0890f_ORCH_GetActiveGroupVisibleDistance_Compat(
            world, &ctx, &activeGroup);
    if (ctx.distanceToVisibleParty == 0) {
        struct DM1GroupSmellDirectionPlan_Compat smellPlan;

        memset(&smellPlan, 0, sizeof(smellPlan));
        if (F0890e_ORCH_BuildGroupSmellDirectionPlan_Compat(
                world, &ctx, &smellPlan) && smellPlan.valid &&
            (smellPlan.usedDirectPartyRoute || smellPlan.usedStoredScent)) {
            ctx.smelledPartyDirectionOrdinal = smellPlan.directionOrdinal;
            ctx.smelledPartySecondaryDirection = smellPlan.secondaryDirection;
        }
    }
    {
        static const int directionDeltaX[4] = { 0, 1, 0, -1 };
        static const int directionDeltaY[4] = { -1, 0, 1, 0 };
        int direction;

        /* GROUP.C F0209 calls F0202 before choosing a reaction move.  The
         * pure decision receives the same loaded-tile and occupancy facts
         * which its later F0267 application will consume. */
        for (direction = 0; direction < 4; ++direction) {
            int destX = ev->mapX + directionDeltaX[direction];
            int destY = ev->mapY + directionDeltaY[direction];

            if (!orch_build_group_movement_facts_compat(
                    world, ev->mapIndex, destX, destY,
                    &ctx.groupMovementFacts[direction])) {
                /* The pure F0202 owner retains `available == 0` and blocks
                 * this direction.  Do not pre-mark it: F0203 owns G0384's
                 * source-order tested-direction write. */
                continue;
            }
            (void)orch_build_group_movement_facts_compat(
                world, ev->mapIndex,
                destX + directionDeltaX[direction],
                destY + directionDeltaY[direction],
                &ctx.archenemySecondStepMovementFacts[direction]);
        }
    }
    ctx.ticksSinceLastMove = (int)world->gameTick - ai->lastSeenPartyTick;
    if (ctx.ticksSinceLastMove < 0) ctx.ticksSinceLastMove = 0;
    ctx.currentTickLow = (int)world->gameTick;
    ctx.eventType = ev->aux2;
    ctx.eventTicks = (ev->aux4 & 0x100) ? ev->aux3 : (int)ev->fireAtTick;

    cellsBeforeBehavior = activeGroup.cells;
    creatureCountBeforeBehavior = (int)group->count;

    if (ev->aux2 >= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
        ev->aux2 <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
        int creatureIndex = ev->aux2 - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
        int currentDirection;

        /* ReDMCSB GROUP.C F0209 lines 2414-2442: a non-side-attacking C38
         * creature that can see the party but is not facing it turns through
         * F0205 and retries C38 two ticks later.  It must not fall through
         * Firestaff's broader attack resolver before that source turn. */
        currentDirection =
            (activeGroup.directions >> (creatureIndex << 1)) & 0x03;
        if (creatureIndex <= (int)group->count &&
            !(activeGroup.aspect[creatureIndex] & 0x80) &&
            ctx.distanceToVisibleParty > 0 &&
            !(ctx.creatureInfo.attributes & DM1_ATTR_SIDE_ATTACK) &&
            currentDirection != ctx.currentGroupPrimaryDirToParty) {
            struct TimelineEvent_Compat retry = *ev;
            if (!orch_apply_f0205_active_creature_direction_compat(
                    world, ai, group, &activeGroup,
                    ctx.currentGroupPrimaryDirToParty, creatureIndex,
                    ctx.creatureSize)) {
                return 0;
            }
            /* Keep the loaded ACTIVE_GROUP receipt authoritative across the
             * immediate C38-C41 retry boundary. */
            if (activeIndex < world->pc34ActiveGroupSourceCount) {
                world->pc34ActiveGroupDirections[activeIndex] =
                    (unsigned char)activeGroup.directions;
            }
            ai->lastSeenPartyMapX = world->party.mapX;
            ai->lastSeenPartyMapY = world->party.mapY;
            retry.fireAtTick = world->gameTick + 2u;
            (void)F0721_TIMELINE_Schedule_Compat(&world->timeline, &retry);
            return 1;
        }
    }

    if (ev->aux2 >= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
        ev->aux2 <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
        int creatureIndex = ev->aux2 - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
        int inAttackLine = ctx.currentGroupDistanceToParty > 0 &&
            ctx.currentGroupDistanceToParty <= DM1_ATTACK_RANGE(ctx.creatureInfo.ranges) &&
            (ctx.currentGroupMapX == ctx.partyMapX || ctx.currentGroupMapY == ctx.partyMapY);
        if (inAttackLine && !(activeGroup.aspect[creatureIndex] & 0x80) &&
            orch_apply_f0205_creature_turn_retry_compat(
                world, ev, ai, group, &activeGroup, activeIndex, creatureIndex,
                ctx.currentGroupPrimaryDirToParty, ctx.creatureSize)) {
            return 1;
        }
    }

    /* ReDMCSB: PROJEXPL.C F0231 calls GROUP.C F0209 with
     * CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT unless the
     * whole group died.  The scheduler has already converted CM1 into the
     * concrete C31 reaction event; this dispatch applies F0209's C31 branch
     * to the active-group analogue. */
    if (!F0810_DM1_GROUP_DispatchBehavior_Compat(
            &ctx, &activeGroup, &world->masterRng, &behavior)) {
        return 0;
    }

    /* GROUP.C F0209 calls F0182 whenever this source decision abandons an
     * attack.  The decision already carries that source fact; consume it
     * before any replacement C37/C38 event is scheduled. */
    if (behavior.stopAttacking &&
        !F0182_DM1_GROUP_StopAttacking_Compat(
            world, &activeGroup, ev->mapIndex, ev->mapX, ev->mapY)) {
        return 0;
    }

    /* ReDMCSB GROUP.C F0209 enters F0206 at T0209073 for a movement or
     * reaction turn, and calls F0205 once per creature at T0209044 before
     * scheduling C38-C41 attacks.  Keep that packed ACTIVE_GROUP state in
     * M10 rather than collapsing it to GROUP.Direction after each event. */
    if (behavior.actionKind == DM1_ACTION_MOVE ||
        behavior.actionKind == DM1_ACTION_FLEE_MOVE ||
        ev->aux2 == DM1_EVENT_UPDATE_BEHAVIOR_GROUP ||
        (ev->aux2 == DM1_EVENT_REACTION_HIT_BY_PROJECTILE &&
         behavior.actionKind == DM1_ACTION_SET_DIRECTION)) {
        int direction = -1;
        if (behavior.actionKind == DM1_ACTION_MOVE ||
            behavior.actionKind == DM1_ACTION_FLEE_MOVE) {
            direction = behavior.moveDirection;
        } else if (behavior.actionKind == DM1_ACTION_ATTACK ||
                   behavior.actionKind == DM1_ACTION_SET_DIRECTION ||
                   behavior.setDirectionOnly) {
            direction = behavior.newDirectionForGroup;
        }
        if (direction >= 0 &&
            !orch_apply_f0206_active_group_directions_compat(
                world, ai, group, &activeGroup, activeIndex, direction,
                ctx.creatureSize)) {
            return 0;
        }
    }

    {
        int groupRemoved = 0;

        if (!orch_apply_f0209_reaction_move_f0267_compat(
                world, ev, groupIndex, &behavior, &reactionMoveHandled,
                &groupRemoved)) {
            return 0;
        }
        if (groupRemoved) return 1;
        if (reactionMoveHandled) {
            activeIndex = orch_find_active_group_state_index_compat(
                world, groupIndex);
            if (activeIndex < 0) return 1;
            group = &world->things->groups[groupIndex];
        }
    }

    /* ReDMCSB GROUP.C F0209 lines 2402-2408 calls F0218 against the old
     * packed cells before it writes a quarter-square creature's new cell.
     * A projectile can kill or compact the group in that interval.  Do not
     * let the later cell write resurrect the old slot layout. */
    if (behavior.actionKind == DM1_ACTION_ADJUST_CELL &&
        behavior.meleeCellAdjustment) {
        int killedAllByPendingProjectile = 0;
        int cellsChangedByPendingProjectile = 0;
        unsigned char ordinalInCell[4];

        if (behavior.adjustedCreatureCell >= 0) {
            group->cells = (unsigned char)(cellsBeforeBehavior & 0xff);
            orch_build_group_projectile_impact_cells_compat(
                group, ordinalInCell);
            if (!orch_process_group_projectile_impacts_on_square_compat(
                    world, group, ev->mapIndex, ev->mapX, ev->mapY,
                    ordinalInCell, &killedAllByPendingProjectile)) {
                return 0;
            }
            cellsChangedByPendingProjectile =
                ((int)group->count != creatureCountBeforeBehavior);
            if (killedAllByPendingProjectile) {
                /* F0218 has already completed F0190's whole aftermath;
                 * never restore the deferred C38 cell state after a kill. */
                return 1;
            }
        }

        if (!cellsChangedByPendingProjectile) {
            group->cells = (unsigned char)(behavior.updatedGroupCells & 0xff);
        } else {
            /* F0190 shifted the surviving slot before this C38 dispatch
             * reaches F0208.  Do not copy the pre-impact ACTIVE_GROUP back
             * over that source mutation at the common apply-plan tail. */
            activeGroup.directions = ai->groupDirection;
            memcpy(activeGroup.aspect, ai->aspect, sizeof(activeGroup.aspect));
        }
        activeGroup.cells = group->cells;
        ai->groupCells = group->cells;
        orch_write_raw_group_compat(world->things, groupIndex);
    }

    if (!orch_apply_f0207_creature_attack_compat(
            world, ev, group, &ctx, &activeGroup, &behavior, result)) {
        return 0;
    }

    /* ReDMCSB GROUP.C F0179 lines 224-305 writes ACTIVE_GROUP::Aspect
     * around each C38 attack and C33 aspect handoff.  Keep the persistent
     * latch in M10 so a later C38 observes the prior attack state instead
     * of receiving a freshly zeroed ACTIVE_GROUP every event.  The bitmap
     * offset/flip fields remain owned by the renderer's F0179 frame route. */
    if (ev->aux2 >= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
        ev->aux2 <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
        int creatureIndex = ev->aux2 - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
        if (behavior.actionKind == DM1_ACTION_ATTACK ||
            behavior.actionKind == DM1_ACTION_STEAL) {
            activeGroup.aspect[creatureIndex] |= 0x80;
        }
    } else if (ev->aux2 >= DM1_EVENT_UPDATE_ASPECT_CREATURE_0 &&
               ev->aux2 <= DM1_EVENT_UPDATE_ASPECT_CREATURE_3) {
        int creatureIndex = ev->aux2 - DM1_EVENT_UPDATE_ASPECT_CREATURE_0;
        activeGroup.aspect[creatureIndex] &= (int)~0x80;
    }

    if (!F0810b_DM1_GROUP_PlanReactionApply_Compat(
            &behavior, &activeGroup, groupIndex, group->creatureType,
            world->creatureAI[activeIndex].groupMapIndex,
            world->creatureAI[activeIndex].groupMapX,
            world->creatureAI[activeIndex].groupMapY, group->cells,
            AI_STATE_WANDER, AI_STATE_ATTACK, AI_STATE_APPROACH,
            AI_STATE_FLEE, world->gameTick, &applyPlan) ||
        !applyPlan.valid) {
        return 0;
    }

    ai->stateKind = applyPlan.newAiStateKind;
    ai->groupMapIndex = applyPlan.groupMapIndex;
    ai->groupMapX = applyPlan.groupMapX;
    ai->groupMapY = applyPlan.groupMapY;
    ai->groupCells = applyPlan.groupCells;
    ai->lastSeenPartyMapX = applyPlan.lastSeenPartyMapX;
    ai->lastSeenPartyMapY = applyPlan.lastSeenPartyMapY;
    ai->lastSeenPartyTick = applyPlan.lastSeenPartyTick;
    memcpy(ai->aspect, activeGroup.aspect, sizeof(ai->aspect));
    group->behavior = (unsigned char)applyPlan.groupBehavior;

    if (behavior.actionKind == DM1_ACTION_MOVE ||
        behavior.actionKind == DM1_ACTION_FLEE_MOVE) {
        if (reactionMoveHandled) {
            /* C29-C36/C38-C41 already used F0267 above; their source-shaped
             * reaction plan remains responsible for the next event. */
            goto schedule_next;
        }
        /* ReDMCSB GROUP.C F0209 lines 2173-2185: the F0200/F0199 visibility
         * decision supplies this direction, then F0267 commits the move or
         * retains the source square for the next C37 retry. */
        if (behavior.moveDirection < 0 || behavior.moveDirection > 3) {
            return 0;
        }
        /* The F0206 write above owns the persistent packed directions; raw
         * GROUP.Direction is only its low two-bit F0184-compatible view. */
        group->direction = (unsigned char)(ai->groupDirection & 0x03);
        return orch_apply_creature_tick_group_move_f0267_compat(
            world, ev, result);
    }

schedule_next:
    if (applyPlan.shouldScheduleNextEvent) {
        if (!orch_f0209_insert_next_event_f0238_compat(
                world, &applyPlan, &activeGroup, ai, group,
                &ctx.creatureInfo)) {
            return 0;
        }
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
    if (ev->aux4 > 0) {
        sensorIndex = ev->aux4 - 1;
        if (sensorIndex < 0 || sensorIndex >= world->things->sensorCount ||
            !world->things->sensors ||
            world->things->sensors[sensorIndex].sensorType !=
                RUNTIME_SENSOR_TYPE_FLOOR_GROUP_GENERATOR) {
            return 0;
        }
    } else if (!orch_find_generator_sensor_on_square_compat(
                   world->dungeon, world->things, ev->mapIndex, ev->mapX,
                   ev->mapY, &sensorIndex)) {
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
        DM1_V1_MovementOrchestratorRoutePlanPc34Compat routePlan;
        int mv;
        memset(&routePlan, 0, sizeof(routePlan));
        if (!DM1_V1_Movement_OrchestratorRoutePlanPc34Compat(
                input->command, world->party.direction,
                world->disabledMovementTicks,
                world->projectileDisabledMovementTicks,
                world->lastProjectileDisabledMovementDirection,
                &routePlan) ||
            !routePlan.valid) {
            return 0;
        }
        if (routePlan.movementDisabledGate) return 0;
        mv = routePlan.moveAction;
        if (!world->dungeon) {
            /* no dungeon: succeed deterministically (unit-test path) */
            if (routePlan.dispatchedTurn) {
                (void)F0284_CHAMPION_SetPartyDirection_Compat(&world->party,
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
                (void)F0284_CHAMPION_SetPartyDirection_Compat(&world->party, mr.newDirection);
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
                (void)F0284_CHAMPION_SetPartyDirection_Compat(&world->party, postMove.finalDirection);
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
                world->disabledMovementTicks =
                    DM1_V1_MovementTiming_ComputePartyStepTicksPc34Compat(
                        &world->party, NULL);
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
        DM1_MeleeF0402CommandDecodeInputPc34 decodeIn;
        DM1_MeleeF0402CommandDecodePlanPc34 decodePlan;
        int weaponClass;
        DM1_MeleeF0402WeaponAvailabilityInputPc34 availabilityIn;
        DM1_MeleeF0402WeaponAvailabilityPlanPc34 availabilityPlan;
        /* ReDMCSB COMMAND.C F0380:2308-2312 and MENU.C F0407 receive a
         * selected Party.Champion ordinal, not an arbitrary Champion[] slot.
         * Reject a populated out-of-party slot before it can create F0407
         * C04/C11 receipts; cleanup preserves historic valid C11 and C04. */
        if ((int)input->commandArg1 >= CHAMPION_MAX_PARTY ||
            (int)input->commandArg1 >= world->party.championCount) {
            orch_cleanup_invalid_action_enable_receipts_compat(
                world, (int)input->commandArg1);
            return 1;
        }
        int hasWeaponInfo = F0888_ORCH_GetChampionActionHandWeaponInfo_Compat(
            world, (int)input->commandArg1, &weaponInfo) > 0;
        memset(&decodeIn, 0, sizeof(decodeIn));
        memset(&decodePlan, 0, sizeof(decodePlan));
        decodeIn.commandArg2 = input->commandArg2;
        decodeIn.reserved = input->reserved;
        decodeIn.reserved2 = input->reserved2;
        decodeIn.partyMapIndex = world ? world->party.mapIndex : 0;
        decodeIn.partyMapX = world ? world->party.mapX : 0;
        decodeIn.partyMapY = world ? world->party.mapY : 0;
        decodeIn.partyDirection = world ? world->party.direction : 0;
        (void)dm1_v1_melee_command_decode_plan_f0402_pc34(
            &decodeIn, &decodePlan);
        memset(&availabilityIn, 0, sizeof(availabilityIn));
        memset(&availabilityPlan, 0, sizeof(availabilityPlan));
        availabilityIn.hasWeaponInfo = hasWeaponInfo;
        availabilityIn.hasLiveActionIndex = decodePlan.hasLiveActionIndex;
        availabilityIn.actionHandEmpty =
            orch_cmd_attack_action_hand_is_empty_compat(
                world, (int)input->commandArg1);
        (void)dm1_v1_melee_weapon_availability_plan_f0402_pc34(
            &availabilityIn, &availabilityPlan);
        if (availabilityPlan.useEmptyHandWeaponInfo) {
            orch_cmd_attack_empty_hand_weapon_info_compat(&weaponInfo);
        }
        if (availabilityPlan.hasUsableF0231WeaponInfo) {
            struct CombatantChampionSnapshot_Compat championSnapshot;
            struct CombatantCreatureSnapshot_Compat creatureSnapshot;
            struct WeaponProfile_Compat weaponProfile;
            struct CombatResult_Compat combatResult;
            int groupIndex = -1;
            int creatureIndex = -1;
            int applyOutcome = COMBAT_OUTCOME_INVALID;
            int weaponType = -1;
            int actionIndex = decodePlan.actionIndex;
            int actionSkillIndex = decodePlan.actionSkillIndex;
            int killedCell = EXPLOSION_CELL_CENTERED;
            int originalGroupCount = -1;
            int fearTriggered = 0;
            DM1_MeleeF0190GroupDamageApplyPlanPc34 damageApplyPlan;
            DM1_MeleeF0231AftermathInputPc34 aftermathIn;
            DM1_MeleeF0231AftermathPlanPc34 aftermathPlan;
            DM1_MeleeF0231AftermathApplyPlanPc34 aftermathApplyPlan;
            DM1_MeleeF0190KilledAllAfterplayReceiptPc34 killedAllAfterplay;
            DM1_MeleeF0231ResolveRuntimeInputPc34 resolveRuntimeIn;
            DM1_MeleeF0231ResolveRuntimePlanPc34 resolveRuntimePlan;
            DM1_MeleeF0231RuntimeApplyPlanPc34 runtimeApplyPlan;
            struct CombatResult_Compat groupDamageResult;
            int targetDirection = decodePlan.targetDirection;
            int targetResolved;
            int reachBlocked = 0;
            int disruptBlocked = 0;
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
                    targetDirection, result)) {
                return 1;
            }
            targetResolved = orch_cmd_attack_resolve_target_compat(
                world, input, &decodePlan, &groupIndex, &creatureIndex);
            if (targetResolved) {
                reachBlocked =
                    orch_cmd_attack_champion_reach_blocked_f0407_compat(
                        world, (int)input->commandArg1, targetDirection);
                disruptBlocked =
                    orch_cmd_attack_disrupt_material_blocked_f0407_compat(
                        world, actionIndex, groupIndex);
            }
            {
                DM1_MeleeF0402PreflightInputPc34 preflightIn;
                DM1_MeleeF0402PreflightPlanPc34 preflightPlan;
                memset(&preflightIn, 0, sizeof(preflightIn));
                memset(&preflightPlan, 0, sizeof(preflightPlan));
                preflightIn.requestedAutoTarget = decodePlan.requestedAutoTarget;
                preflightIn.hasLiveActionIndex = decodePlan.hasLiveActionIndex;
                preflightIn.hasLiveGroupTable =
                    orch_cmd_attack_has_live_group_table_compat(world);
                preflightIn.targetResolved = targetResolved;
                preflightIn.reachBlocked = reachBlocked;
                preflightIn.disruptBlocked = disruptBlocked;
                preflightIn.candidateInvulnerable =
                    targetResolved &&
                    world->candidateAttackInvulnerableEnabled &&
                    world->candidateAttackInvulnerableGroupIndex == groupIndex &&
                    world->candidateAttackInvulnerableCreatureIndex ==
                        creatureIndex;
                preflightIn.championSnapshotReady = targetResolved;
                preflightIn.creatureSnapshotReady = targetResolved;
                (void)dm1_v1_melee_preflight_plan_f0402_pc34(
                    &preflightIn, &preflightPlan);
                if (preflightPlan.shouldEmitDamageDealt) {
                    emit(result, EMIT_DAMAGE_DEALT,
                         input->commandArg1, groupIndex, 0,
                         preflightPlan.emitOutcome);
                }
                if (preflightPlan.shouldReturnHandled) {
                    return 1;
                }
                if (!targetResolved && preflightPlan.canUseLegacyMarker) {
                    goto cmd_attack_legacy_marker;
                }
                if (!targetResolved) {
                    return 1;
                }
            }
            if (targetResolved) {
                creatureSnapshotReady =
                    F0888_ORCH_GetCreatureSnapshot_Compat(
                        world, groupIndex, creatureIndex,
                        orch_cmd_attack_doubled_map_difficulty_compat(world),
                        &creatureSnapshot);
                championSnapshotReady =
                    orch_build_cmd_attack_champion_snapshot_compat(
                        world, (int)input->commandArg1, &weaponInfo, weaponType,
                        hasWeaponInfo, actionSkillIndex, &championSnapshot);
            }
            {
                DM1_MeleeF0402PreflightInputPc34 preflightIn;
                DM1_MeleeF0402PreflightPlanPc34 preflightPlan;
                memset(&preflightIn, 0, sizeof(preflightIn));
                memset(&preflightPlan, 0, sizeof(preflightPlan));
                preflightIn.requestedAutoTarget = decodePlan.requestedAutoTarget;
                preflightIn.hasLiveActionIndex = decodePlan.hasLiveActionIndex;
                preflightIn.hasLiveGroupTable =
                    orch_cmd_attack_has_live_group_table_compat(world);
                preflightIn.targetResolved = targetResolved;
                preflightIn.reachBlocked = reachBlocked;
                preflightIn.disruptBlocked = disruptBlocked;
                preflightIn.candidateInvulnerable =
                    creatureSnapshotReady &&
                    creatureSnapshot.isCandidateInvulnerable;
                preflightIn.championSnapshotReady = championSnapshotReady;
                preflightIn.creatureSnapshotReady = creatureSnapshotReady;
                (void)dm1_v1_melee_preflight_plan_f0402_pc34(
                    &preflightIn, &preflightPlan);
                if (preflightPlan.shouldReturnHandled) {
                    return 1;
                }
                if (!preflightPlan.canResolveDamage) {
                    return 1;
                }
            }
            {
                if (orch_build_cmd_attack_weapon_profile_compat(
                        &weaponInfo, weaponType, actionIndex, actionSkillIndex,
                        &weaponProfile) &&
                    (memset(&resolveRuntimeIn, 0, sizeof(resolveRuntimeIn)),
                     memset(&resolveRuntimePlan, 0, sizeof(resolveRuntimePlan)),
                     resolveRuntimeIn.championIndex = (int)input->commandArg1,
                     resolveRuntimeIn.groupIndex = groupIndex,
                     resolveRuntimeIn.creatureIndex = creatureIndex,
                     resolveRuntimeIn.groupCount =
                         world && world->things ? world->things->groupCount : 0,
                     dm1_v1_melee_resolve_runtime_f0231_pc34(
                        &championSnapshot, &weaponProfile, &creatureSnapshot,
                        &world->masterRng, &resolveRuntimeIn,
                        &resolveRuntimePlan))) {
                    combatResult = resolveRuntimePlan.combatResult;
                    if (!resolveRuntimePlan.valid ||
                        !resolveRuntimePlan.runtimeResultPlan.valid) {
                        return 1;
                    }
                    memset(&runtimeApplyPlan, 0, sizeof(runtimeApplyPlan));
                    if (!dm1_v1_melee_runtime_apply_plan_f0231_pc34(
                            &resolveRuntimePlan.runtimeResultPlan,
                            &runtimeApplyPlan) ||
                        !runtimeApplyPlan.valid) {
                        return 1;
                    }
                    if (runtimeApplyPlan.shouldReturnHandledNoAction) {
                        /* ReDMCSB CLIKCHAM.C F0368 lines 69-71 keeps the
                         * live candidate champion panel from being redrawn
                         * as a normal champion state.  Firestaff carries
                         * the same "candidate is panel-owned" boundary into
                         * F0735 as NO_ACTION; it must not become a F0231 miss
                         * with stamina, reaction, or damage emissions. */
                        return 1;
                    }
                    if (runtimeApplyPlan.shouldWriteBackLuck) {
                        orch_writeback_cmd_attack_luck_compat(
                            world, (int)input->commandArg1, &championSnapshot);
                    }
                    if (runtimeApplyPlan.shouldApplySideEffects) {
                        orch_cmd_attack_apply_f0231_side_effects_compat(
                            world, (int)input->commandArg1, actionSkillIndex,
                            &creatureSnapshot, combatResult.damageApplied,
                            result);
                    }
                    /* ReDMCSB: PROJEXPL.C F0231 line 1534 obtains F0190's
                     * outcome before any F0190/F0231 aftermath. The source
                     * GROUP.Count is an input to the F0190 fear roll, so do
                     * not construct a provisional receipt with an unset count. */
                    memset(&aftermathIn, 0, sizeof(aftermathIn));
                    aftermathIn.groupIndex = groupIndex;
                    aftermathIn.creatureIndex = creatureIndex;
                    aftermathIn.creatureType = creatureSnapshot.creatureType;
                    aftermathIn.creatureAttributes = creatureSnapshot.attributes;
                    aftermathIn.creatureProperties = creatureSnapshot.properties;
                    aftermathIn.groupBehavior =
                        world->things->groups[groupIndex].behavior;
                    aftermathIn.partyMapIndex = world->partyMapIndex;
                    aftermathIn.partyMapX = world->party.mapX;
                    aftermathIn.partyMapY = world->party.mapY;
                    orch_cmd_attack_target_square_compat(
                        world, targetDirection,
                        &aftermathIn.targetMapIndex,
                        &aftermathIn.targetMapX,
                        &aftermathIn.targetMapY);
                    aftermathIn.currentTick = world->gameTick;
                    aftermathIn.fallbackCombatOutcome = combatResult.outcome;
                    if (runtimeApplyPlan.shouldApplyGroupDamage) {
                        memset(&groupDamageResult, 0, sizeof(groupDamageResult));
                        groupDamageResult.outcome =
                            runtimeApplyPlan.groupDamageFallbackOutcome;
                        groupDamageResult.damageApplied =
                            runtimeApplyPlan.groupDamageApplied;
                        memset(&damageApplyPlan, 0, sizeof(damageApplyPlan));
                        (void)dm1_v1_melee_apply_group_damage_plan_f0190_pc34(
                            &groupDamageResult,
                            &world->things->groups[
                                runtimeApplyPlan.groupDamageGroupIndex],
                            runtimeApplyPlan.groupDamageCreatureIndex,
                            &damageApplyPlan);
                        originalGroupCount = damageApplyPlan.originalGroupCount;
                        killedCell = damageApplyPlan.killedCell;
                        applyOutcome = damageApplyPlan.outcome;
                        aftermathIn.originalGroupCount = originalGroupCount;
                        aftermathIn.killedCell = killedCell;
                        aftermathIn.damageOutcome = applyOutcome;
                        (void)dm1_v1_melee_aftermath_plan_f0231_pc34(
                            &aftermathIn, &aftermathPlan);
                        memset(&aftermathApplyPlan, 0,
                               sizeof(aftermathApplyPlan));
                        (void)dm1_v1_melee_aftermath_apply_plan_f0231_pc34(
                            &aftermathPlan, &aftermathApplyPlan);
                        memset(&killedAllAfterplay, 0,
                               sizeof(killedAllAfterplay));
                        (void)dm1_v1_melee_killed_all_afterplay_receipt_f0190_pc34(
                            &aftermathApplyPlan, &killedAllAfterplay);
                        if (aftermathApplyPlan.shouldApplyMutationDispatch) {
                            fearTriggered =
                                orch_cmd_attack_apply_f0190_mutation_dispatch_compat(
                                    world, &world->things->groups[groupIndex],
                                    &aftermathApplyPlan
                                        .mutationDispatchPlan);
                        }
                        if (killedAllAfterplay.shouldPresentSourceSmoke) {
                            struct TimelineEvent_Compat advance;
                            int slotIndex = -1;
                            memset(&advance, 0, sizeof(advance));
                            if (F0821_EXPLOSION_Create_Compat(
                                    &killedAllAfterplay.sourceSmokeCreateInput,
                                    &world->explosions,
                                    &slotIndex,
                                    &advance)) {
                                (void)F0721_TIMELINE_Schedule_Compat(
                                    &world->timeline, &advance);
                            }
                        } else if (aftermathApplyPlan.shouldCreateDeathSmoke) {
                            struct TimelineEvent_Compat advance;
                            int slotIndex = -1;
                            memset(&advance, 0, sizeof(advance));
                            if (F0213_EXPLOSION_Create_Compat(
                                    &aftermathApplyPlan.smokeCreateInput,
                                    &world->explosions,
                                    &slotIndex,
                                    &advance)) {
                                (void)F0721_TIMELINE_Schedule_Compat(
                                    &world->timeline, &advance);
                            }
                        }
                        if (aftermathApplyPlan.shouldWriteRawGroup) {
                            orch_write_raw_group_compat(
                                world->things,
                                aftermathApplyPlan.rawGroupWritebackPlan
                                    .groupIndex);
                        }
                    }
                    aftermathIn.killedCell = killedCell;
                    aftermathIn.damageOutcome = applyOutcome;
                    aftermathIn.fearTriggered = fearTriggered;
                    (void)dm1_v1_melee_aftermath_plan_f0231_pc34(
                        &aftermathIn, &aftermathPlan);
                    memset(&aftermathApplyPlan, 0,
                           sizeof(aftermathApplyPlan));
                    (void)dm1_v1_melee_aftermath_apply_plan_f0231_pc34(
                        &aftermathPlan, &aftermathApplyPlan);
                    if (aftermathApplyPlan.shouldEmitKillNotify) {
                        emit(result, EMIT_KILL_NOTIFY,
                             aftermathApplyPlan.killNotifyGroupIndex,
                             aftermathApplyPlan.killNotifyCreatureIndex,
                             aftermathApplyPlan.killNotifyOutcome,
                             aftermathApplyPlan.killNotifyCreatureType);
                    }
                    if (aftermathApplyPlan.shouldScheduleReaction) {
                        /* F0231 -> F0209 C31 is the source recoil bridge.
                         * Its later C38/C39 events must originate from an
                         * authenticated PC34 C04/SFT active-group owner. */
                        (void)orch_cmd_attack_schedule_f0231_reaction_compat(
                            world, aftermathApplyPlan.reactionGroupIndex,
                            &creatureSnapshot, targetDirection,
                            aftermathPlan.outcome);
                    }
                    if (runtimeApplyPlan.shouldEmitDamageDealt) {
                        emit(result, EMIT_DAMAGE_DEALT,
                             runtimeApplyPlan.emitChampionIndex,
                             runtimeApplyPlan.emitGroupIndex,
                             runtimeApplyPlan.emitDamageApplied,
                             aftermathPlan.outcome);
                    }
                    return 1;
                }
            }
        } else {
            if (decodePlan.requestedAutoTarget) {
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
cmd_attack_legacy_marker:
        if (!decodePlan.hasLegacyMarker) {
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
        int receiptExperience = -1;
        int actionDisabledTicks = 0;
        int failedCastExperience = 0;
        int failedCastSkillIndex = -1;
        uint32_t spellRngRaw;

        spellRngRaw = F0731_COMBAT_RngNextRaw_Compat(&world->masterRng);
        emit(result, EMIT_SOUND_REQUEST, tableIdx,
             world->party.mapX, world->party.mapY, 0);

        if (powerOrd < 1 || powerOrd > 6) powerOrd = 1;

        if (!F0752b_MAGIC_LookupSpellByTableIndex_Compat(tableIdx, &spell)) {
            return 1; /* unknown spell — sound already emitted */
        }

        memset(&effect, 0, sizeof(effect));

        switch (spell.kind) {
        case C2_SPELL_KIND_PROJECTILE_COMPAT: {
            DM1_ChampionSpellStats dm1Stats;
            DM1_SpellF0412RuntimeReceipt receipt;
            DM1_SpellF0327ProjectileContextPc34 projectileContext;
            struct ProjectileCreateInput_Compat projectileInput;
            struct TimelineEvent_Compat firstMove;
            struct ProjectileList_Compat projectilesBefore;
            struct TimelineQueue_Compat timelineBefore;
            DM1_C14PoolReservationPc34 c14;
            int projectileSlot = -1;
            int eventIndex = -1;
            int i;

            if (!orch_cmd_cast_spell_build_dm1_stats_f0412_compat(
                    world, champIdx, &dm1Stats) ||
                !dm1_spell_f0412RuntimeReceiptForTableIndex(
                    tableIdx, powerOrd, champIdx, &dm1Stats,
                    (uint16_t)(spellRngRaw & 0xFFFFu),
                    world->party.champions[champIdx].direction,
                    world->party.direction,
                    world->magic.partyShieldDefense,
                    &receipt)) {
                return 0;
            }

            if (!dm1_spell_f0412ReceiptToSpellEffectPc34(
                    &receipt, world->magic.fireShieldDefense, &effect)) {
                return 0;
            }
            receiptExperience = receipt.experience;
            actionDisabledTicks = receipt.disabledTicks;

            if (receipt.castResult != DM1_SPELL_CAST_SUCCESS &&
                receipt.failureType == DM1_FAILURE_NEEDS_MORE_PRACTICE &&
                receipt.partialExperience > 0) {
                failedCastExperience = receipt.partialExperience;
                failedCastSkillIndex = receipt.skillIndex;
            }

            if (receipt.castResult != DM1_SPELL_CAST_SUCCESS ||
                !receipt.createsProjectile) {
                break;
            }

            if (receipt.rotatesChampion) {
                world->party.champions[champIdx].direction =
                    (unsigned char)(receipt.championDirectionAfter & 3);
            }

            memset(&projectileContext, 0, sizeof(projectileContext));
            projectileContext.championIndex = champIdx;
            projectileContext.championCell =
                world->party.champions[champIdx].cell;
            projectileContext.partyMapIndex = world->party.mapIndex;
            projectileContext.partyMapX = world->party.mapX;
            projectileContext.partyMapY = world->party.mapY;
            projectileContext.gameTick = (int)world->gameTick;

            if (!dm1_v1_build_spell_projectile_create_input_f0327_pc34(
                    &receipt, &projectileContext, &projectileInput)) {
                return 0;
            }
            projectilesBefore = world->projectiles;
            timelineBefore = world->timeline;
            memset(&c14, 0, sizeof(c14));
            /* A loaded DM1 source world must publish F0212's C14/C49 pair
             * atomically.  The memory-only harness has no original C14 pool
             * to authenticate, so it retains the existing isolated route. */
            if (world->things && world->things->loaded &&
                (!world->dungeon ||
                 !dm1_v1_c14_pool_reserve_pc34(world->things, &c14))) {
                return 0;
            }
            if (!F0810_PROJECTILE_Create_Compat(
                    &projectileInput, &world->projectiles,
                    &projectileSlot, &firstMove) ||
                (c14.active && THING_GET_INDEX(c14.thing) != projectileSlot) ||
                !F0721_TIMELINE_Schedule_Compat(&world->timeline, &firstMove)) {
                world->projectiles = projectilesBefore;
                world->timeline = timelineBefore;
                if (c14.active) (void)dm1_v1_c14_pool_rollback_pc34(&c14);
                return 0;
            }
            if (c14.active) {
                for (i = 0; i < world->timeline.count; ++i) {
                    if (world->timeline.events[i].kind == TIMELINE_EVENT_PROJECTILE_MOVE &&
                        world->timeline.events[i].aux0 == projectileSlot &&
                        world->timeline.events[i].fireAtTick == firstMove.fireAtTick) {
                        if (eventIndex >= 0) { eventIndex = -2; break; }
                        eventIndex = i;
                    }
                }
                if (eventIndex < 0 || !dm1_v1_c14_pool_initialize_and_link_pc34(
                        &c14, world->dungeon,
                        (unsigned short)projectileInput.associatedThing,
                        projectileInput.kineticEnergy, projectileInput.attack,
                        (unsigned short)eventIndex, projectileInput.cell,
                        projectileInput.mapIndex, projectileInput.mapX,
                        projectileInput.mapY)) {
                    world->projectiles = projectilesBefore;
                    world->timeline = timelineBefore;
                    (void)dm1_v1_c14_pool_rollback_pc34(&c14);
                    return 0;
                }
            }
            break;
        }
        case C3_SPELL_KIND_OTHER_COMPAT: {
            DM1_ChampionSpellStats dm1Stats;
            DM1_SpellF0412RuntimeReceipt receipt;

            if (!orch_cmd_cast_spell_build_dm1_stats_f0412_compat(
                    world, champIdx, &dm1Stats) ||
                !dm1_spell_f0412RuntimeReceiptForTableIndex(
                    tableIdx, powerOrd, champIdx, &dm1Stats,
                    (uint16_t)(spellRngRaw & 0xFFFFu),
                    world->party.champions[champIdx].direction,
                    world->party.direction,
                    world->magic.partyShieldDefense,
                    &receipt) ||
                !dm1_spell_f0412ReceiptToSpellEffectPc34(
                    &receipt, world->magic.fireShieldDefense, &effect)) {
                return 0;
            }
            receiptExperience = receipt.experience;
            actionDisabledTicks = receipt.disabledTicks;
            if (receipt.castResult != DM1_SPELL_CAST_SUCCESS &&
                receipt.failureType == DM1_FAILURE_NEEDS_MORE_PRACTICE &&
                receipt.partialExperience > 0) {
                failedCastExperience = receipt.partialExperience;
                failedCastSkillIndex = receipt.skillIndex;
            }
            break;
        }
        case C1_SPELL_KIND_POTION_COMPAT: {
            DM1_ChampionSpellStats dm1Stats;
            DM1_SpellF0412RuntimeReceipt receipt;
            uint16_t potionPowerRng16 = 0;

            if (emptyFlaskSlot >= 0) {
                potionPowerRng16 =
                    (uint16_t)F0732_COMBAT_RngRandom_Compat(
                        &world->masterRng, 16);
            }
            if (!orch_cmd_cast_spell_build_dm1_stats_f0412_compat(
                    world, champIdx, &dm1Stats) ||
                !dm1_spell_f0412PotionReceiptForTableIndex(
                    tableIdx, powerOrd, champIdx, &dm1Stats,
                    (uint16_t)(spellRngRaw & 0xFFFFu),
                    potionPowerRng16, emptyFlaskSlot >= 0, &receipt) ||
                !dm1_spell_f0412ReceiptToSpellEffectPc34(
                    &receipt, world->magic.fireShieldDefense, &effect)) {
                return 0;
            }
            receiptExperience = receipt.experience;
            actionDisabledTicks = receipt.disabledTicks;
            if (receipt.castResult != DM1_SPELL_CAST_SUCCESS &&
                receipt.failureType == DM1_FAILURE_NEEDS_MORE_PRACTICE &&
                receipt.partialExperience > 0) {
                failedCastExperience = receipt.partialExperience;
                failedCastSkillIndex = receipt.skillIndex;
            }
            break;
        }
        case C4_SPELL_KIND_MAGIC_MAP_COMPAT:
            F0759_MAGIC_ProduceMagicMapEffect_Compat(
                &spell, powerOrd,
                orch_cmd_cast_spell_has_magic_map_compat(input),
                champIdx, world->party.direction, &effect);
            actionDisabledTicks = spell.disabledTicks;
            break;
        default:
            /* Unknown kind — no effect. */
            return 1;
        }

        if (effect.castResult != SPELL_CAST_SUCCESS) {
            orch_cmd_cast_spell_award_f0412_experience_compat(
                world, champIdx, failedCastSkillIndex, failedCastExperience,
                0, result);
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
            orch_mirror_other_spell_lifecycle_status_pc34_compat(
                world, &effect);
            spellExperience =
                (receiptExperience >= 0 &&
                 (!input ||
                  (input->reserved2 &
                   CMD_CAST_SPELL_RESERVED2_HAS_SPELL_XP) == 0u))
                    ? receiptExperience
                    : orch_cmd_cast_spell_xp_compat(
                          input, &spell, effect.powerOrdinal,
                          &world->masterRng);

            /* Schedule follow-up timeline event if applicable.
             * ReDMCSB MENU.C F0412 T0412033 always adds the status
             * EVENT after T0412032. Status spells use per-family aux tags
             * so the timeout path can undo the matching runtime state. */
            if (effect.followupEventKind != TIMELINE_EVENT_INVALID &&
                (effect.durationTicks > 0 ||
                 effect.followupEventKind == TIMELINE_EVENT_STATUS_TIMEOUT)) {
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
            if (actionDisabledTicks > 0) {
                /* ReDMCSB: MENU.C F0412 lines 2034-2039 awards spell XP,
                 * then calls F0330_CHAMPION_DisableAction.  CHAMPION.C
                 * F0330 lines 2233-2255 stores slot ordinal 0 for spell
                 * cooldowns; spells do not bind a Graphic560 action index.
                 * F0884 schedules the canonical C11 receipt from this
                 * emission after player input returns. */
                emit(result, EMIT_ACTION_DISABLED, champIdx,
                     actionDisabledTicks, 0xFF, 0);
            }
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
    world->pc34M10ProjectileDispatchMask = 0;
    world->pc34M10ProjectileDispatchTick = world->gameTick;

    while (F0722_TIMELINE_Peek_Compat(&world->timeline, &peek) == 1
           && peek.fireAtTick <= world->gameTick)
    {
        if (F0723_TIMELINE_Pop_Compat(&world->timeline, &ev) != 1) break;
        orch_shift_projectile_event_indexes_after_pop_compat(world);
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
                 * If so, it forces the door back open, applies F0324 damage,
                 * and reschedules the same event two ticks after the original
                 * fire time. */
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
                            (void)orch_apply_door_party_damage_f0324_compat(
                                world, obstruction.damageAmount,
                                obstruction.woundMask, result);
                            emit(result, EMIT_DAMAGE_DEALT,
                                 obstruction.damageAmount, obstruction.woundMask,
                                 world->party.championCount, ev.mapIndex);
                        } else if (obstruction.kind == DOOR_OBSTRUCTION_CREATURE) {
                            int killedAll = 0;
                            struct CombatAction_Compat dangerAction;
                            (void)F0712_DOOR_StepAnimation_Compat(
                                world->dungeon, ev.mapIndex, ev.mapX, ev.mapY,
                                DOOR_EFFECT_SET, 1, &step);
                            if (world->things && groupIndex >= 0 &&
                                groupIndex < world->things->groupCount) {
                                (void)orch_apply_door_group_damage_f0191_compat(
                                    world, groupIndex, ev.mapIndex, ev.mapX, ev.mapY,
                                    obstruction.damageAmount, &killedAll);
                                if (!killedAll) {
                                    memset(&dangerAction, 0, sizeof(dangerAction));
                                    dangerAction.targetMapIndex = ev.mapIndex;
                                    dangerAction.targetMapX = ev.mapX;
                                    dangerAction.targetMapY = ev.mapY;
                                    /* ReDMCSB TIMELINE.C F0241:789 invokes
                                     * GROUP.C F0209 CM3 only for survivors. */
                                    orch_schedule_group_reaction_compat(
                                        world, groupIndex,
                                        &world->things->groups[groupIndex],
                                        &dangerAction,
                                        DM1_CM3_REACTION_DANGER_ON_SQUARE);
                                }
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
             * C5_DOOR_STATE_DESTROYED to that door square.  Do not emit a
             * host transition when a malformed caller names another tile. */
            if (orch_set_door_state_compat(
                    world, ev.mapIndex, ev.mapX, ev.mapY, 5)) {
                emit(result, EMIT_DOOR_STATE, ev.mapX, ev.mapY, 5,
                     ev.mapIndex);
            }
            break;
        case TIMELINE_EVENT_ENABLE_CHAMPION_ACTION:
            /* ReDMCSB TIMELINE.C C11:1927-1932 first invokes F0253 for
             * Priority, then F0259 only for MENU.C's C01 ordinal two.
             * COMMAND.C F0380 only gates the action-area route; if imported
             * state loses the C11 owner after it becomes due, consume the
             * receipt without either action. C20 is independently dispatched
             * from its location/sound receipt. C11 Priority is never a C49
             * projectile slot or owner and has no projectile generation or
             * impact-tick field, so pool reuse cannot revive this receipt.
             * MENU.C F0407/F0330 retain this champion index while party cells
             * move, so a party-position swap cannot remap it. A cleared
             * Party slot fails the live owner gate rather than rolling C11
             * onto later slot contents. Imported B/C padding is deliberately
             * not interpreted. */
            if (ev.aux0 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
                ev.aux2 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
                ev.aux4 >= 0 && ev.aux4 < CHAMPION_MAX_PARTY &&
                ev.aux4 < world->party.championCount &&
                world->party.champions[ev.aux4].present &&
                (ev.aux1 == 0 || ev.aux1 == 2)) {
                /* C11's data mutation lives in M10.  F0253's visual lock
                 * completion remains consumed by the host action receipt,
                 * while F0259 may cross inventory slots only from a loaded,
                 * byte-matching PC34 C05 weapon record.  A bad/missing raw
                 * record therefore leaves the quiver untouched. */
                if (ev.aux1 == 2) {
                    struct DM1F0259QuiverRefillPlanPc34 refill;
                    (void)DM1_V1_F0259_ApplyQuiverRefillFromDungeonPc34Compat(
                        &world->party.champions[ev.aux4], ev.aux4,
                        CHAMPION_SLOT_ACTION_HAND, world->things, &refill);
                }
                emit(result, EMIT_ACTION_ENABLED, ev.aux4, ev.aux1, 0, 0);
            }
            break;
        case TIMELINE_EVENT_VI_ALTAR_REBIRTH:
            /* ReDMCSB TIMELINE.C F0255 owns the C13 rebirth state machine
             * for live play.  An F0435-admitted C13 record is an external
             * receipt: the handoff boundary must not synthesize a rebirth
             * follow-up for it, so the dispatcher consumes it here. */
            break;
        case TIMELINE_EVENT_PLAY_SOUND:
            /* ReDMCSB SOUND.C F0064:1536-1543 produces delayed C20 only
             * with a non-negative SoundIndex and its native receipt.  Keep
             * the older generic sound path for non-save callers, but do not
             * let a malformed claimed C20 reach the emission surface.
             * In particular, F0407's C04 uses SOUND_DATA.Priority (70),
             * never a champion owner: a C11 owner encoded there is stale
             * imported data, not a second source C04. */
            if (ev.aux2 != DM1_EVENT_PLAY_SOUND ||
                (ev.aux0 >= 0 && ev.aux1 == 0 && ev.aux3 == 0 &&
                 ev.cell == 0 && ev.aux4 >= 0 && ev.aux4 <= 0xff &&
                 (ev.aux0 != ORCH_SOUND_WOODEN_THUD_PC34 ||
                  ev.aux4 == ORCH_SOUND_WOODEN_THUD_PRIORITY_PC34))) {
                emit(result, EMIT_SOUND_REQUEST, ev.aux0, ev.mapX, ev.mapY,
                     ev.mapIndex);
            }
            break;
        case TIMELINE_EVENT_CPSE_CHECK:
            /* ReDMCSB TIMELINE.C:1920-1925 dispatches C22 only inside the
             * optional copy-protection path.  Firestaff deliberately has
             * no synthetic fuzzy-sector result: a typed original C22 is
             * consumed as the NOCOPYPROTECTION build does. */
            break;
        case TIMELINE_EVENT_WATCHDOG:
            /* ReDMCSB TIMELINE.C F0256:1710-1715 re-arms C53 only as part
             * of the copy-protection watchdog.  As with C22, Firestaff
             * ships the NOCOPYPROTECTION behavior: an imported C53 receipt
             * is consumed without a host re-arm. */
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
                    /* ReDMCSB TIMELINE.C F0257:1761-1765 makes the next
                     * event another native C70. Preserve the original-save
                     * receipt so F0802 can export this source-owned chain. */
                    if (ev.aux1 == DM1_EVENT_LIGHT && ev.aux4 == 0) {
                        lr.followupEvent.aux1 = DM1_EVENT_LIGHT;
                        lr.followupEvent.aux4 = 0;
                    }
                    F0721_TIMELINE_Schedule_Compat(&world->timeline, &lr.followupEvent);
                }
            }
            break;
        }
        case TIMELINE_EVENT_SPELL_TICK:
            /* Firestaff's legacy spell-tick queue kind carries the same
             * typed F0412/F0763 status receipt as STATUS_TIMEOUT.  ReDMCSB
             * has no generic C05 spell event here: F0261:1953-1999 dispatches
             * the original C71/C73/C74/C77/C78/C79 bytes directly.  Consume
             * only those already-typed payloads through that source route;
             * unknown payloads remain a no-op instead of becoming a host
             * spell effect. */
            if (!orch_spell_tick_has_typed_status_receipt_pc34_compat(&ev)) {
                break;
            }
            /* fall through */
        case TIMELINE_EVENT_STATUS_TIMEOUT: {
            struct TimelineEvent_Compat resched;
            struct TimelineEvent_Compat statusEvent;
            /* ReDMCSB TIMELINE.C F0254 consumes C12 through Priority only:
             * clear the source damage indicator in M11 and do not route its
             * undefined B/C union bytes through spell-status lifecycle. */
            if (ev.aux0 == DM1_EVENT_HIDE_DAMAGE_RECEIVED) {
                if (ev.aux4 >= 0 && ev.aux4 < CHAMPION_MAX_PARTY &&
                    world->party.champions[ev.aux4].present) {
                    emit(result, EMIT_CHAMPION_DAMAGE_HIDDEN,
                         ev.aux4, 0, 0, 0);
                }
                break;
            }
            if (ev.aux0 == DM1_EVENT_CHAMPION_SHIELD &&
                ev.aux2 == DM1_EVENT_CHAMPION_SHIELD &&
                ev.aux4 >= 0 && ev.aux4 < CHAMPION_MAX_PARTY &&
                world->party.champions[ev.aux4].present) {
                /* ReDMCSB TIMELINE.C C72:1964-1967 subtracts B.Defense
                 * from the selected champion only; do not route C72 through
                 * the host status-lifecycle aliases. */
                world->lifecycle.champions[ev.aux4].shieldDefense -= (int16_t)ev.aux1;
                break;
            }
            if (ev.aux0 == DM1_EVENT_INVISIBILITY &&
                ev.aux2 == DM1_EVENT_INVISIBILITY && ev.aux1 == 0 &&
                ev.aux4 == 0) {
                /* ReDMCSB TIMELINE.C C71:1953-1964 decrements only the
                 * party invisibility count; B/C and Priority are not a
                 * runtime input beyond F0412's required zero Priority. */
                world->magic.event71CountInvisibility--;
                (void)F0839_LIFECYCLE_HandleCounterExpiry_Compat(
                    &world->lifecycle, LIFECYCLE_STATUS_INVISIBILITY);
                break;
            }
            if (ev.aux0 == DM1_EVENT_THIEVES_EYE &&
                ev.aux2 == DM1_EVENT_THIEVES_EYE && ev.aux1 == 0 &&
                ev.aux4 == 0) {
                /* ReDMCSB TIMELINE.C C73:1972-1974 decrements only
                 * Event73Count_ThievesEye; C73 owns no B/C union arm. */
                world->magic.event73CountThievesEye--;
                (void)F0839_LIFECYCLE_HandleCounterExpiry_Compat(
                    &world->lifecycle, LIFECYCLE_STATUS_THIEVES_EYE);
                break;
            }
            if (ev.aux0 == DM1_EVENT_PARTY_SHIELD &&
                ev.aux2 == DM1_EVENT_PARTY_SHIELD && ev.aux1 > 0 &&
                ev.aux4 == 0) {
                /* ReDMCSB TIMELINE.C C74:1975-1976 subtracts the signed
                 * B.Defense from party shield. Keep both M10 mirrors in
                 * lockstep without interpreting C as another payload. */
                world->magic.partyShieldDefense -= (int16_t)ev.aux1;
                world->lifecycle.status.partyShieldDefense -= (int16_t)ev.aux1;
                break;
            }
            if (ev.aux0 == DM1_EVENT_SPELLSHIELD &&
                ev.aux2 == DM1_EVENT_SPELLSHIELD && ev.aux1 > 0 &&
                ev.aux4 == 0) {
                /* ReDMCSB TIMELINE.C C77:1985-1986 subtracts signed
                 * B.Defense from spell shield with no C union payload. */
                world->magic.spellShieldDefense -= (int16_t)ev.aux1;
                world->lifecycle.status.partySpellShieldDefense -= (int16_t)ev.aux1;
                break;
            }
            if (ev.aux0 == DM1_EVENT_FIRESHIELD &&
                ev.aux2 == DM1_EVENT_FIRESHIELD && ev.aux1 > 0 &&
                ev.aux4 == 0) {
                /* ReDMCSB TIMELINE.C C78:1988-1989 subtracts signed
                 * B.Defense from fire shield with no C union payload. */
                world->magic.fireShieldDefense -= (int16_t)ev.aux1;
                world->lifecycle.status.partyFireShieldDefense -= (int16_t)ev.aux1;
                break;
            }
            if (ev.aux0 == DM1_EVENT_FOOTPRINTS &&
                ev.aux2 == DM1_EVENT_FOOTPRINTS && ev.aux1 == 0 && ev.aux4 == 0) {
                /* ReDMCSB TIMELINE.C C79:1998-2000 decrements only count. */
                world->magic.event79CountFootprints--;
                if (world->magic.event79CountFootprints <= 0) world->magic.magicFootprintsActive = 0;
                (void)F0839_LIFECYCLE_HandleCounterExpiry_Compat(&world->lifecycle, LIFECYCLE_STATUS_FOOTPRINTS);
                break;
            }
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
                            if (damage > 0) {
                                /* ReDMCSB TIMELINE.C C75 calls CHAMPION.C
                                 * F0322 for this poison hit. M10 owns the
                                 * HP write above; M11 consumes this existing
                                 * live-damage signal for its panel overlay.
                                 * C75 owns neither a wound mask nor a display
                                 * cell, so those optional payloads are zero. */
                                emit(result,
                                     EMIT_CHAMPION_DAMAGED,
                                     championIndex,
                                     0,
                                     damage,
                                     0);
                            }
                            if (resched.kind == TIMELINE_EVENT_STATUS_TIMEOUT &&
                                    resched.aux0 == LIFECYCLE_STATUS_POISON &&
                                    resched.aux1 > 0) {
                                champ->poisonDose = (unsigned short)resched.aux1;
                            } else {
                                champ->poisonDose = 0;
                            }
                        }
                        if (resched.kind != TIMELINE_EVENT_INVALID) {
                            if (ev.aux0 == DM1_EVENT_POISON_CHAMPION &&
                                ev.aux2 == DM1_EVENT_POISON_CHAMPION) {
                                /* ReDMCSB CHAMPION.C F0322:1954-1960
                                 * requeues native C75 on the party's
                                 * current map with Attack-1. Preserve the
                                 * original-save receipt for native export. */
                                resched.mapIndex = world->partyMapIndex;
                                resched.aux0 = DM1_EVENT_POISON_CHAMPION;
                                resched.aux2 = DM1_EVENT_POISON_CHAMPION;
                                resched.aux4 = championIndex;
                            }
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
            unsigned short sourceThing;

            /* ReDMCSB TIMELINE.C F0261:1906-1916 removes C24's exact
             * C15 slot only while G0302_B_GameWon is false, then marks
             * the source record unused. An unbound C24 is not allowed to
             * consume a host-only explosion. */
            if (world->gameWon ||
                !orch_c24_find_fluxcage_thing_compat(world, &ev,
                                                      &sourceThing)) {
                break;
            }
            memset(&in, 0, sizeof(in));
            memset(&out, 0, sizeof(out));
            in.explosionSlotIndex = ev.aux0;
            in.mapIndex = ev.mapIndex;
            in.mapX = ev.mapX;
            in.mapY = ev.mapY;
            if (F0868_RUNTIME_HandleRemoveFluxcage_Compat(
                    &in, &world->explosions, &out) && out.removed) {
                if (!orch_unlink_thing_from_square_compat(
                        world, ev.mapIndex, ev.mapX, ev.mapY, sourceThing)) {
                    break;
                }
                (void)orch_set_next_thing_compat(world->things, sourceThing,
                                                  THING_NONE);
            }
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
            (void)orch_handle_deferred_group_move_event_compat(
                world, &ev, result, 0);
            break;
        case TIMELINE_EVENT_SQUARE_STATE:
            (void)orch_dispatch_square_state_event_compat(world, &ev, result);
            break;
        case TIMELINE_EVENT_MOVE_TIMER:
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
            {
                DM1_GroupLosDirectionAdmissionInputPc34 losInput;
                DM1_GroupLosDirectionAdmissionReceiptPc34 losReceipt;
                int activeIndex = orch_find_active_group_state_index_compat(
                    world, ev.aux0);

                /* F0227/F0228 selects a group route for C29-C37. C38-C41
                 * consume the already-published packed ACTIVE_GROUP result
                 * in F0209 and must not re-admit a second LoS decision. */
                if (ev.aux2 < DM1_EVENT_REACTION_DANGER_ON_SQUARE ||
                    ev.aux2 > DM1_EVENT_UPDATE_BEHAVIOR_GROUP) {
                    (void)orch_handle_creature_reaction_event_compat(
                        world, &ev, result);
                    break;
                }
                memset(&losInput, 0, sizeof(losInput));
                memset(&losReceipt, 0, sizeof(losReceipt));
                if (activeIndex < 0 ||
                    activeIndex >= world->pc34ActiveGroupSourceCount) {
                    break;
                }
                losInput.things = world->things;
                losInput.groupIndex = ev.aux0;
                losInput.activeGroup = &world->creatureAI[activeIndex];
                losInput.activeDirections =
                    world->pc34ActiveGroupDirections[activeIndex];
                losInput.event = &ev;
                losInput.partyMapIndex = world->partyMapIndex;
                losInput.partyMapX = world->party.mapX;
                losInput.partyMapY = world->party.mapY;
                losInput.rng = &world->masterRng;
                if (dm1_v1_group_los_direction_admit_f0227_f0228_pc34(
                        &losInput, &losReceipt) && losReceipt.valid) {
                    (void)orch_handle_creature_reaction_event_compat(
                        world, &ev, result);
                }
            }
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
    /* ReDMCSB CHAMPION.C F0320:1720-1727 loops over all champions after
     * the timeline pass. Each entry is a total of every F0321 call made
     * during this tick, with wounds ORed before HP is reduced. */
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        struct CombatResult_Compat* pending = &world->pendingChampionCombat[i];
        int targetCell = -1;
        int killed = 0;
        if (pending->damageApplied <= 0) continue;
        (void)orch_read_pending_damage_receipt_compat(
            world->pendingChampionCombatTargetReceipt[i], NULL, &targetCell);
        if (world->party.champions[i].present) {
            F0737_COMBAT_ApplyDamageToChampion_Compat(
                pending, &world->party.champions[i], &killed);
            if (pending->poisonAttackPending > 0) {
                world->party.champions[i].poisonDose +=
                    (unsigned short)pending->poisonAttackPending;
            }
            emit(result, EMIT_CHAMPION_DAMAGED, i, targetCell,
                 pending->damageApplied, pending->woundMaskAdded);
            if (killed) emit(result, EMIT_CHAMPION_DOWN, i, 0, 0, 0);
        }
        memset(pending, 0, sizeof(*pending));
        world->pendingChampionCombatTargetReceipt[i] = 0;
    }

    /* Compatibility for pre-DM1-008 producers that still publish one
     * active-champion result instead of the F0321-shaped staging buffer. */
    if (world->pendingCombat.damageApplied > 0 && world->party.championCount > 0) {
        int idx = world->party.activeChampionIndex;
        int targetCell = -1;
        int hasTargetReceipt = orch_read_pending_damage_receipt_compat(
            world->pendingCombatTargetReceipt, &idx, &targetCell);
        /* ReDMCSB GROUP.C F0207 selects the defender before F0230/F0321.
         * Preserve that concrete receipt.  Old producers without a receipt
         * retain the active-champion behavior until they migrate. */
        if (hasTargetReceipt) {
            (void)targetCell; /* retained in state/serialization for M11 receipt use */
        }
        if (idx >= 0 && idx < CHAMPION_MAX_PARTY) {
            int killed = 0;
            F0737_COMBAT_ApplyDamageToChampion_Compat(
                &world->pendingCombat, &world->party.champions[idx], &killed);
            if (world->pendingCombat.poisonAttackPending > 0) {
                world->party.champions[idx].poisonDose +=
                    (unsigned short)world->pendingCombat.poisonAttackPending;
            }
            emit(result, EMIT_CHAMPION_DAMAGED, idx, targetCell,
                 world->pendingCombat.damageApplied,
                 world->pendingCombat.woundMaskAdded);
            if (killed) emit(result, EMIT_CHAMPION_DOWN, idx, 0, 0, 0);
        }
        memset(&world->pendingCombat, 0, sizeof(world->pendingCombat));
        world->pendingCombatTargetReceipt = 0;
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
    DM1_V1_MovementTiming_DecrementCooldownsPc34Compat(
        &world->disabledMovementTicks,
        &world->projectileDisabledMovementTicks);
    if (world->freezeLifeTicks > 0) world->freezeLifeTicks--;
    /* ReDMCSB MAIN.C decrements PARTY.FreezeLifeTicks. Keep the M10 magic
     * mirror on that saved-runtime owner rather than letting a resumed save
     * retain a stale value after its first periodic tick. */
    world->magic.freezeLifeTicks = world->freezeLifeTicks;
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
        /* ReDMCSB CHAMPION.C F0330 schedules C11 immediately after an
         * action emits its disabled duration, before the next timeline
         * extraction can refill the ready hand. */
        orch_f0330_schedule_action_disabled_emissions_compat(world, outResult);
    }

    /* Step 2/3b: map-transition loop + timeline dispatch */
    mapTransitions = 0;
    do {
        if (world->newPartyMapIndex != -1) {
            if (world->newPartyMapIndex == world->partyMapIndex) {
                /* F0435 restores a current-map anchor, not a newly entered
                 * map.  Its authenticated ACTIVE_GROUP/timeline state stays
                 * intact until an actual F0003 handoff occurs. */
                world->newPartyMapIndex = -1;
            } else if (!orch_transition_party_map_f0194_f0195_compat(
                           world, world->newPartyMapIndex)) {
                return ORCH_FAIL;
            }
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
