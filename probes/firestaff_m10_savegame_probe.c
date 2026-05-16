/*
 * M10 Phase 15 probe — composite save/load verification.
 *
 * Validates the pure data-layer save-game module:
 *   - CRC32 / IEEE 802.3 (F0770) + header writer/validator (F0771/F0772)
 *   - Per-section wrappers for Phase 10–14 serialisers (F0776..F0782)
 *   - Section-table compute/validate (F0783/F0784)
 *   - Top-level save/load + round-trip (F0773/F0774/F0775)
 *   - File-IO round-trip (F0785/F0786)
 *   - Diagnostics (F0787/F0788/F0789)
 *   - Cross-phase integration: F0736 bit-identical pre-/post-load
 *
 * 40 invariants, >= 30 required for PASS.
 *
 * Authoritative plan: PHASE15_PLAN.md §5.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_savegame_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_magic_pc34_compat.h"

/* ---------- Fixture builders ------------------------------------ */

static void build_zero_party(struct PartyState_Compat* p) {
    memset(p, 0, sizeof(*p));
}

static void build_populated_party(struct PartyState_Compat* p) {
    int i;
    memset(p, 0, sizeof(*p));
    p->championCount       = 2;
    p->mapIndex            = 3;
    p->mapX                = 7;
    p->mapY                = 11;
    p->direction           = DIR_EAST;
    p->activeChampionIndex = 1;
    p->eventFlags          = 0x0123;
    p->magicShieldTime     = 42;
    p->fireShieldTime      = 17;
    for (i = 0; i < 2; i++) {
        struct ChampionState_Compat* c = &p->champions[i];
        c->portraitIndex = 10 + i;
        c->present       = 1;
        c->name[0] = (unsigned char)('A' + i);
        c->name[1] = (unsigned char)('l');
        c->name[2] = (unsigned char)('i');
        c->name[3] = (unsigned char)('c');
        c->name[4] = (unsigned char)('e');
        c->name[5] = 0; c->name[6] = 0; c->name[7] = 0;
        c->attributes[CHAMPION_ATTR_STRENGTH]  = (unsigned short)(40 + i * 5);
        c->attributes[CHAMPION_ATTR_DEXTERITY] = (unsigned short)(30 + i * 5);
        c->attributes[CHAMPION_ATTR_WISDOM]    = 50;
        c->attributes[CHAMPION_ATTR_VITALITY]  = 60;
        c->attributes[CHAMPION_ATTR_ANTIMAGIC] = 45;
        c->attributes[CHAMPION_ATTR_ANTIFIRE]  = 48;
        c->skillLevels[CHAMPION_SKILL_FIGHTER] = 3;
        c->skillLevels[CHAMPION_SKILL_WIZARD]  = 4;
        c->skillExperience[CHAMPION_SKILL_FIGHTER] = 500u;
        c->hp.current = 60; c->hp.maximum = 80; c->hp.shifted = 160;
        c->stamina.current = 70; c->stamina.maximum = 90; c->stamina.shifted = 180;
        c->mana.current = 20; c->mana.maximum = 40; c->mana.shifted = 80;
        c->direction = DIR_EAST;
        c->wounds = (unsigned short)(i ? 0x0002 : 0x0000);
        c->food = 200; c->water = 190;
    }
}

static void build_zero_movement(struct MovementResult_Compat* mv) {
    memset(mv, 0, sizeof(*mv));
}

static void build_populated_movement(struct MovementResult_Compat* mv) {
    memset(mv, 0, sizeof(*mv));
    mv->resultCode   = MOVE_OK;
    mv->newMapX      = 5;
    mv->newMapY      = 9;
    mv->newDirection = DIR_SOUTH;
    mv->newMapIndex  = 2;
}

static void build_zero_sensor(struct SensorEffectList_Compat* s) {
    memset(s, 0, sizeof(*s));
}

static void build_populated_sensor(struct SensorEffectList_Compat* s) {
    int i;
    memset(s, 0, sizeof(*s));
    s->count = 5;
    for (i = 0; i < 5; i++) {
        struct SensorEffect_Compat* e = &s->effects[i];
        e->kind         = (i & 1) ? SENSOR_EFFECT_TELEPORT : SENSOR_EFFECT_SHOW_TEXT;
        e->sensorType   = 13 + i;
        e->destMapIndex = i;
        e->destMapX     = 2 + i;
        e->destMapY     = 3 + i;
        e->destCell     = i & 3;
        e->textIndex    = 100 + i;
    }
}

static void build_zero_timeline(struct TimelineQueue_Compat* q) {
    F0720_TIMELINE_Init_Compat(q, 0u);
}

static void build_populated_timeline(struct TimelineQueue_Compat* q) {
    int i;
    F0720_TIMELINE_Init_Compat(q, 1000u);
    for (i = 0; i < 37; i++) {
        struct TimelineEvent_Compat ev;
        memset(&ev, 0, sizeof(ev));
        ev.kind       = TIMELINE_EVENT_CREATURE_TICK + (i % 8);
        ev.fireAtTick = 1100u + (uint32_t)(i * 3);
        ev.mapIndex   = i % 4;
        ev.mapX       = i;
        ev.mapY       = 31 - i;
        ev.cell       = i & 3;
        ev.aux0       = 0x1000 + i;
        ev.aux1       = 0x2000 + i;
        ev.aux2       = i;
        ev.aux3       = -i;
        ev.aux4       = 7 * i;
        F0721_TIMELINE_Schedule_Compat(q, &ev);
    }
}

static void build_full_timeline(struct TimelineQueue_Compat* q) {
    int i;
    /* Directly populate — use unique fireAtTick values 0..255 so
     * the ordered-insertion invariant of F0721 is satisfied while
     * we bypass the heap (populate in sorted order). */
    memset(q, 0, sizeof(*q));
    q->nowTick = 500u;
    q->count   = TIMELINE_QUEUE_CAPACITY;
    for (i = 0; i < TIMELINE_QUEUE_CAPACITY; i++) {
        struct TimelineEvent_Compat* ev = &q->events[i];
        ev->kind       = TIMELINE_EVENT_SPELL_TICK;
        ev->fireAtTick = (uint32_t)i;
        ev->mapIndex   = i & 7;
        ev->mapX       = i & 31;
        ev->mapY       = (i * 3) & 31;
        ev->cell       = i & 3;
        ev->aux0       = i * 17;
        ev->aux1       = i * 31;
        ev->aux2       = i * 41;
        ev->aux3       = -i;
        ev->aux4       = ~i;
    }
}

static void build_zero_combat(struct CombatScratch_Compat* c) {
    memset(c, 0, sizeof(*c));
}

static void build_populated_combat(struct CombatScratch_Compat* c) {
    memset(c, 0, sizeof(*c));
    c->lastResult.outcome             = COMBAT_OUTCOME_HIT_DAMAGE;
    c->lastResult.damageApplied       = 12;
    c->lastResult.rawAttackRoll       = 37;
    c->lastResult.defenseRoll         = 20;
    c->lastResult.hitLanded           = 1;
    c->lastResult.wasCritical         = 0;
    c->lastResult.woundMaskAdded      = COMBAT_WOUND_TORSO;
    c->lastResult.poisonAttackPending = 3;
    c->lastResult.targetKilled        = 0;
    c->lastResult.creatureSlotRemoved = -1;
    c->lastResult.followupEventKind   = TIMELINE_EVENT_CREATURE_TICK;
    c->lastResult.followupEventAux0   = 1;
    c->lastResult.rngCallCount        = 4;
    c->lastResult.wakeFromRest        = 1;

    c->lastWeapon.weaponType     = 40;
    c->lastWeapon.weaponClass    = 2;
    c->lastWeapon.weaponStrength = 30;
    c->lastWeapon.kineticEnergy  = 80;
    c->lastWeapon.hitProbability = 65;
    c->lastWeapon.damageFactor   = 25;
    c->lastWeapon.skillIndex     = CHAMPION_SKILL_FIGHTER;
    c->lastWeapon.attributes     = 0x0031;

    F0730_COMBAT_RngInit_Compat(&c->rng, 0xC0FFEE01u);
    c->combatActive = 1;
    c->reserved     = 0;
}

static void build_zero_magic(struct MagicState_Compat* m) {
    memset(m, 0, sizeof(*m));
}

static void build_populated_magic(struct MagicState_Compat* m) {
    memset(m, 0, sizeof(*m));
    m->spellShieldDefense       = 11;
    m->fireShieldDefense        = 22;
    m->partyShieldDefense       = 33;
    m->magicalLightAmount       = 5;
    m->lightDecayFireAtTick     = 0x12345678u;
    m->event70LightDirection    = 1;
    m->event71CountInvisibility = 2;
    m->event73CountThievesEye   = 3;
    m->event74CountPartyShield  = 4;
    m->event77CountSpellShield  = 5;
    m->event78CountFireShield   = 6;
    m->event79CountFootprints   = 7;
    m->freezeLifeTicks          = 8;
    m->magicFootprintsActive    = 1;
    m->luckCurrent              = 50;
    m->curseMask                = 0x0004;
}

static void build_zero_mutations(struct DungeonMutationList_Compat* lst) {
    memset(lst, 0, sizeof(*lst));
}

static void build_populated_mutations(struct DungeonMutationList_Compat* lst) {
    int i;
    memset(lst, 0, sizeof(*lst));
    lst->count = 8;
    for (i = 0; i < 8; i++) {
        struct DungeonMutation_Compat* m = &lst->entries[i];
        m->kind        = DUNGEON_MUTATION_KIND_SQUARE_BYTE + (i % 5);
        m->mapIndex    = i;
        m->x           = 2 + i;
        m->y           = 3 + i;
        m->cell        = i & 3;
        m->thingType   = i & 15;
        m->thingIndex  = 100 + i;
        m->beforeValue = 0xAABB0000u | (uint32_t)i;
        m->afterValue  = 0xCCDD0000u | (uint32_t)i;
        m->fieldMask   = 0x01 << (i & 7);
        m->reserved    = 0;
        m->reserved1   = 0;
    }
}

static void build_full_mutations(struct DungeonMutationList_Compat* lst) {
    int i;
    memset(lst, 0, sizeof(*lst));
    lst->count = DUNGEON_MUTATION_LIST_MAX_COUNT;
    for (i = 0; i < DUNGEON_MUTATION_LIST_MAX_COUNT; i++) {
        struct DungeonMutation_Compat* m = &lst->entries[i];
        m->kind        = DUNGEON_MUTATION_KIND_FIELD_GENERIC;
        m->mapIndex    = i & 15;
        m->x           = i & 31;
        m->y           = (i >> 3) & 31;
        m->cell        = i & 3;
        m->thingType   = i & 15;
        m->thingIndex  = i;
        m->beforeValue = (uint32_t)i;
        m->afterValue  = ~(uint32_t)i;
        m->fieldMask   = 1 << (i & 31);
    }
}

/* A defender snapshot used for integration invariant #45. */
static void build_defender_snapshot(struct CombatantChampionSnapshot_Compat* c) {
    memset(c, 0, sizeof(*c));
    c->championIndex      = 0;
    c->currentHealth      = 60;
    c->dexterity          = 40;
    c->strengthActionHand = 30;
    c->skillLevelParry    = 3;
    c->skillLevelAction   = 3;
    c->statisticVitality  = 50;
    c->statisticAntifire  = 60;
    c->statisticAntimagic = 60;
    c->actionHandIcon     = 0;
    c->wounds             = 0;
    c->woundDefense[0]    = 4;
    c->woundDefense[1]    = 4;
    c->woundDefense[2]    = 4;
    c->woundDefense[3]    = 4;
    c->woundDefense[4]    = 4;
    c->woundDefense[5]    = 4;
    c->isResting          = 0;
    c->partyShieldDefense = 0;
}

static void build_creature_snapshot(struct CombatantCreatureSnapshot_Compat* c) {
    memset(c, 0, sizeof(*c));
    c->creatureType        = 5;
    c->attack              = 60;
    c->defense             = 20;
    c->dexterity           = 35;
    c->baseHealth          = 40;
    c->poisonAttack        = 0;
    c->attackType          = COMBAT_ATTACK_NORMAL;
    c->attributes          = 0;
    c->woundProbabilities  = 0x2222;
    c->properties          = 0;
    c->doubledMapDifficulty = 4;
    c->creatureIndex       = 0;
    c->healthBefore        = 40;
}

/* -------- Helpers for SaveGame assembly -------------------------- */

struct SaveGameFixture {
    struct PartyState_Compat          party;
    struct MovementResult_Compat      movement;
    struct SensorEffectList_Compat    sensor;
    struct TimelineQueue_Compat       timeline;
    struct CombatScratch_Compat       combat;
    struct MagicState_Compat          magic;
    struct DungeonMutationList_Compat mutations;
    struct SaveGame_Compat            save;
};

static void bind_fixture(struct SaveGameFixture* fx) {
    F0775_SAVEGAME_InitEmpty_Compat(&fx->save);
    fx->save.party                = &fx->party;
    fx->save.lastMovement         = &fx->movement;
    fx->save.pendingSensorEffects = &fx->sensor;
    fx->save.timeline             = &fx->timeline;
    fx->save.combatScratch        = &fx->combat;
    fx->save.magic                = &fx->magic;
    fx->save.mutations            = &fx->mutations;
}

static void build_zero_fixture(struct SaveGameFixture* fx) {
    bind_fixture(fx);
    build_zero_party(&fx->party);
    build_zero_movement(&fx->movement);
    build_zero_sensor(&fx->sensor);
    build_zero_timeline(&fx->timeline);
    build_zero_combat(&fx->combat);
    build_zero_magic(&fx->magic);
    build_zero_mutations(&fx->mutations);
}

static void build_populated_fixture(struct SaveGameFixture* fx) {
    bind_fixture(fx);
    build_populated_party(&fx->party);
    build_populated_movement(&fx->movement);
    build_populated_sensor(&fx->sensor);
    build_populated_timeline(&fx->timeline);
    build_populated_combat(&fx->combat);
    build_populated_magic(&fx->magic);
    build_populated_mutations(&fx->mutations);
}

/* Compare two structs of the same type field-by-field via memcmp. */
#define BIT_EQ(a, b) (memcmp((a), (b), sizeof(*(a))) == 0)

int main(int argc, char* argv[]) {
    FILE* report;
    FILE* invariants;
    char path_buf[512];
    int failCount = 0;
    int invariantCount = 0;
    const char* dungeonPath = 0;
    const char* outputDir = 0;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
        return 1;
    }
    dungeonPath = argv[1];
    outputDir   = argv[2];
    (void)dungeonPath; /* Phase 15 doesn't need DUNGEON.DAT for any invariant. */

    snprintf(path_buf, sizeof(path_buf), "%s/savegame_probe.md", outputDir);
    report = fopen(path_buf, "w");
    if (!report) { fprintf(stderr, "FAIL: cannot write report\n"); return 1; }
    fprintf(report, "# M10 Phase 15: Save / Load Probe\n\n");
    fprintf(report, "## Scope (v1)\n\n");
    fprintf(report, "- Header + CRC32 validation (F0770-F0772)\n");
    fprintf(report, "- Per-section (de)serialisers (F0776-F0782)\n");
    fprintf(report, "- Section-table compute/validate (F0783/F0784)\n");
    fprintf(report, "- Top-level save/load + round-trip (F0773/F0774)\n");
    fprintf(report, "- File-IO round-trip (F0785/F0786)\n");
    fprintf(report, "- Cross-phase integration: Phase 13 F0736 post-load parity\n\n");
    fprintf(report, "## Known NEEDS DISASSEMBLY REVIEW (still open)\n\n");
    fprintf(report, "- Fontanel DM_SAVE_HEADER Noise[10]/Keys[16]/Checksums[16] obfuscation: v1 intentionally drops interop with DMSAVE1.DAT — our format (RDMCSB15) uses a single CRC32. Re-implementing the XOR key derivation is only required if we ever want DMSAVE1.DAT interop (plan §4.8 item 1).\n");
    fprintf(report, "- DungeonMutation_Compat.fieldMask semantics: Phase 15 carries the bytes opaquely; the replay engine in a future phase will decide which DungeonGroup_Compat.health[] / square byte / sensor bit a given mutation refers to (plan §4.8 item 1).\n");
    fprintf(report, "- GLOBAL_DATA.MusicOn / GameID fields from Fontanel's save header: not mirrored in v1; the 36-byte header.reserved block is the placeholder for a future migration (plan §4.7, §4.8 item 3).\n\n");

    snprintf(path_buf, sizeof(path_buf), "%s/savegame_invariants.md", outputDir);
    invariants = fopen(path_buf, "w");
    if (!invariants) {
        fprintf(stderr, "FAIL: cannot write invariants\n");
        fclose(report);
        return 1;
    }
    fprintf(invariants, "# Savegame Invariants\n\n");

#define CHECK(cond, msg) do { \
    invariantCount++; \
    if (cond) { \
        fprintf(invariants, "- PASS: %s\n", msg); \
    } else { \
        fprintf(invariants, "- FAIL: %s\n", msg); \
        failCount++; \
    } \
} while (0)

    /* ==============================================================
     *  Block A — sizes + CRC (invariants 1-10)
     * ============================================================== */
    CHECK(F0770_SAVEGAME_CRC32_Compat((const unsigned char*)"", 0) == 0x00000000u,
          "F0770 empty input CRC == 0x00000000");
    {
        const unsigned char tv[] = "123456789";
        CHECK(F0770_SAVEGAME_CRC32_Compat(tv, 9) == 0xCBF43926u,
              "F0770(\"123456789\") == 0xCBF43926 (IEEE 802.3 test vector)");
    }
    CHECK(SAVEGAME_HEADER_SERIALIZED_SIZE == 64,
          "SAVEGAME_HEADER_SERIALIZED_SIZE == 64");
    CHECK(SAVEGAME_SECTION_ENTRY_SERIALIZED_SIZE == 16,
          "SAVEGAME_SECTION_ENTRY_SERIALIZED_SIZE == 16");
    CHECK(SAVEGAME_SECTION_COUNT == 7,
          "SAVEGAME_SECTION_COUNT == 7");
    CHECK(DUNGEON_MUTATION_SERIALIZED_SIZE == 48,
          "DUNGEON_MUTATION_SERIALIZED_SIZE == 48");
    CHECK(DUNGEON_MUTATION_LIST_SERIALIZED_SIZE == 4 + 1024 * 48,
          "DUNGEON_MUTATION_LIST_SERIALIZED_SIZE == 49156");
    CHECK(COMBAT_SCRATCH_SERIALIZED_SIZE == 100,
          "COMBAT_SCRATCH_SERIALIZED_SIZE == 100");
    CHECK(MOVEMENT_RESULT_SERIALIZED_SIZE == 20,
          "MOVEMENT_RESULT_SERIALIZED_SIZE == 20");
    CHECK(sizeof(int) == 4 && sizeof(uint32_t) == 4,
          "sizeof(int)==4 && sizeof(uint32_t)==4");

    /* ==============================================================
     *  Block B — header writer/validator (invariants 11-18)
     * ============================================================== */
    {
        struct SaveGameHeader_Compat hdr;
        int err = F0771_SAVEGAME_WriteHeader_Compat(&hdr, 176u, 0xDEADBEEFu);
        int magic_ok = (err == SAVEGAME_OK &&
                        hdr.magic[0] == 'R' && hdr.magic[1] == 'D' &&
                        hdr.magic[2] == 'M' && hdr.magic[3] == 'C' &&
                        hdr.magic[4] == 'S' && hdr.magic[5] == 'B' &&
                        hdr.magic[6] == '1' && hdr.magic[7] == '5');
        CHECK(magic_ok,
              "F0771 writes magic \"RDMCSB15\" at bytes 0..7");
    }
    {
        struct SaveGameHeader_Compat hdr;
        int err = F0771_SAVEGAME_WriteHeader_Compat(&hdr, 176u, 0xDEADBEEFu);
        int res_all_zero = 1;
        int i;
        for (i = 0; i < 36; i++) {
            if (hdr.reserved[i] != 0) { res_all_zero = 0; break; }
        }
        CHECK(err == SAVEGAME_OK &&
              hdr.formatVersion  == SAVEGAME_FORMAT_VERSION &&
              hdr.endianSentinel == SAVEGAME_ENDIAN_SENTINEL &&
              hdr.totalFileSize  == 176u &&
              hdr.sectionCount   == SAVEGAME_SECTION_COUNT &&
              hdr.bodyCRC32      == 0xDEADBEEFu &&
              res_all_zero,
              "F0771 populates version/endian/size/count/crc + reserved zeroed");
    }
    {
        struct SaveGameHeader_Compat hdr;
        F0771_SAVEGAME_WriteHeader_Compat(&hdr, 176u, 0u);
        CHECK(F0772_SAVEGAME_ValidateHeader_Compat(&hdr, 176u) == SAVEGAME_OK,
              "F0772 accepts a valid header");
    }
    {
        struct SaveGameHeader_Compat hdr;
        F0771_SAVEGAME_WriteHeader_Compat(&hdr, 176u, 0u);
        hdr.magic[0] = 'X';
        CHECK(F0772_SAVEGAME_ValidateHeader_Compat(&hdr, 176u) ==
              SAVEGAME_ERROR_BAD_MAGIC,
              "F0772 rejects bad magic with BAD_MAGIC");
    }
    {
        struct SaveGameHeader_Compat hdr;
        F0771_SAVEGAME_WriteHeader_Compat(&hdr, 176u, 0u);
        hdr.formatVersion = 2u;
        CHECK(F0772_SAVEGAME_ValidateHeader_Compat(&hdr, 176u) ==
              SAVEGAME_ERROR_BAD_VERSION,
              "F0772 rejects wrong formatVersion with BAD_VERSION");
    }
    {
        struct SaveGameHeader_Compat hdr;
        F0771_SAVEGAME_WriteHeader_Compat(&hdr, 176u, 0u);
        hdr.endianSentinel = 0x04030201u;
        CHECK(F0772_SAVEGAME_ValidateHeader_Compat(&hdr, 176u) ==
              SAVEGAME_ERROR_BAD_ENDIAN,
              "F0772 rejects wrong endian sentinel with BAD_ENDIAN");
    }
    {
        struct SaveGameHeader_Compat hdr;
        F0771_SAVEGAME_WriteHeader_Compat(&hdr, 99u, 0u);
        CHECK(F0772_SAVEGAME_ValidateHeader_Compat(&hdr, 62080u) ==
              SAVEGAME_ERROR_BAD_SIZE,
              "F0772 rejects totalFileSize mismatch with BAD_SIZE");
    }
    {
        struct SaveGameHeader_Compat hdr;
        F0771_SAVEGAME_WriteHeader_Compat(&hdr, 176u, 0u);
        hdr.sectionCount = 6u;
        CHECK(F0772_SAVEGAME_ValidateHeader_Compat(&hdr, 176u) ==
              SAVEGAME_ERROR_BAD_SECTION_COUNT,
              "F0772 rejects sectionCount != 7 with BAD_SECTION_COUNT");
    }

    /* ==============================================================
     *  Block C — section-table compute/validate (invariants 19-22)
     * ============================================================== */
    {
        struct SaveGameFixture fx;
        struct SaveGameSectionEntry_Compat table[SAVEGAME_SECTION_COUNT];
        uint32_t total = 0;
        int err;
        build_zero_fixture(&fx);
        err = F0783_SAVEGAME_ComputeSectionTable_Compat(&fx.save, table, &total);
        CHECK(err == SAVEGAME_OK &&
              table[0].offset == 176u &&
              table[1].offset == 1232u &&
              table[2].offset == 1252u &&
              table[3].offset == 1480u &&
              table[4].offset == 12752u &&
              table[5].offset == 12852u &&
              table[6].offset == 12924u &&
              total == 62080u,
              "F0783 zero-state offsets are 176/1232/1252/1480/12752/12852/12924, total 62080");
    }
    {
        struct SaveGameSectionEntry_Compat table[SAVEGAME_SECTION_COUNT];
        uint32_t total = 0;
        F0783_SAVEGAME_ComputeSectionTable_Compat(NULL, table, &total);
        CHECK(F0784_SAVEGAME_ValidateSectionTable_Compat(table, total) ==
              SAVEGAME_OK,
              "F0784 accepts the F0783-produced table");
    }
    {
        struct SaveGameSectionEntry_Compat table[SAVEGAME_SECTION_COUNT];
        uint32_t total = 0;
        F0783_SAVEGAME_ComputeSectionTable_Compat(NULL, table, &total);
        /* Rewrite slot 2's offset backwards to overlap slot 1. */
        table[2].offset = 1240u;
        CHECK(F0784_SAVEGAME_ValidateSectionTable_Compat(table, total) ==
              SAVEGAME_ERROR_SECTION_OVERLAP,
              "F0784 rejects slot-2 offset rewritten to overlap slot 1");
    }
    {
        struct SaveGameSectionEntry_Compat table[SAVEGAME_SECTION_COUNT];
        uint32_t total = 0;
        F0783_SAVEGAME_ComputeSectionTable_Compat(NULL, table, &total);
        table[0].kind = 0xCAFEu;
        CHECK(F0784_SAVEGAME_ValidateSectionTable_Compat(table, total) ==
              SAVEGAME_ERROR_BAD_SECTION_KIND,
              "F0784 rejects slot-0 kind rewritten to 0xCAFE");
    }

    /* ==============================================================
     *  Block D — composite round-trip (invariants 23-24)
     * ============================================================== */
    {
        struct SaveGameFixture a, b;
        unsigned char bufA[62080], bufB[62080];
        int wA = 0, wB = 0;
        int errA, errB, errL;
        int firstDiff = -1;
        build_zero_fixture(&a);
        build_zero_fixture(&b);
        errA = F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, bufA, sizeof(bufA), &wA);
        errL = F0774_SAVEGAME_LoadFromBuffer_Compat(bufA, wA, &b.save);
        errB = F0773_SAVEGAME_SaveToBuffer_Compat(&b.save, bufB, sizeof(bufB), &wB);
        CHECK(errA == SAVEGAME_OK && errL == SAVEGAME_OK && errB == SAVEGAME_OK &&
              wA == 62080 && wA == wB &&
              F0787_SAVEGAME_Compare_Compat(bufA, wA, bufB, wB, &firstDiff) == 1,
              "Round-trip zero-state: F0773/F0774/F0773 produces bit-identical buffers (62080 bytes)");
    }
    {
        struct SaveGameFixture a, b;
        unsigned char* bufA = malloc(62080);
        unsigned char* bufB = malloc(62080);
        int wA = 0, wB = 0;
        int errA, errB, errL;
        int firstDiff = -1;
        int rc;
        build_populated_fixture(&a);
        build_zero_fixture(&b); /* caller-owned dests (zeroed) */
        errA = F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, bufA, 62080, &wA);
        errL = F0774_SAVEGAME_LoadFromBuffer_Compat(bufA, wA, &b.save);
        errB = F0773_SAVEGAME_SaveToBuffer_Compat(&b.save, bufB, 62080, &wB);
        rc = (errA == SAVEGAME_OK && errL == SAVEGAME_OK && errB == SAVEGAME_OK &&
              wA == 62080 && wA == wB &&
              F0787_SAVEGAME_Compare_Compat(bufA, wA, bufB, wB, &firstDiff) == 1);
        CHECK(rc,
              "Round-trip populated state: bit-identical save->load->save");
        free(bufA); free(bufB);
    }

    /* ==============================================================
     *  Block E — per-subsystem equality after load (invariants 25-30)
     *  Reuse populated round-trip setup. b.* should equal a.* by
     *  memcmp.
     * ============================================================== */
    {
        struct SaveGameFixture a, b;
        unsigned char* buf = malloc(62080);
        int w = 0;
        build_populated_fixture(&a);
        build_zero_fixture(&b);
        F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, buf, 62080, &w);
        F0774_SAVEGAME_LoadFromBuffer_Compat(buf, w, &b.save);

        CHECK(BIT_EQ(&a.party, &b.party),
              "Party state field-by-field equal after load (4 champions)");
        CHECK(BIT_EQ(&a.timeline, &b.timeline),
              "Timeline queue field-by-field equal after load");
        CHECK(BIT_EQ(&a.sensor, &b.sensor),
              "Sensor effect list field-by-field equal after load");
        CHECK(BIT_EQ(&a.combat, &b.combat),
              "Combat scratch field-by-field equal after load (result+weapon+rng)");
        CHECK(BIT_EQ(&a.magic, &b.magic),
              "Magic state field-by-field equal after load");
        {
            /* Mutations: entries[0..count-1] plus unused slots (zero). */
            int ok = BIT_EQ(&a.mutations, &b.mutations);
            CHECK(ok,
                  "Dungeon-mutation list field-by-field equal after load (count + entries + zero tail)");
        }
        free(buf);
    }

    /* ==============================================================
     *  Block F — tamper + truncation + version (invariants 31-36)
     * ============================================================== */
    {
        /* Flip a bit inside the PARTY section and expect BAD_CRC. */
        struct SaveGameFixture a, b;
        unsigned char* buf = malloc(62080);
        int w = 0;
        int errLoad;
        int tamperOffset = SAVEGAME_HEADER_SERIALIZED_SIZE +
                           SAVEGAME_SECTION_COUNT * SAVEGAME_SECTION_ENTRY_SERIALIZED_SIZE +
                           100;  /* inside slot 0 / PARTY */
        build_populated_fixture(&a);
        build_zero_fixture(&b);
        F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, buf, 62080, &w);
        buf[tamperOffset] ^= 0x01u;
        errLoad = F0774_SAVEGAME_LoadFromBuffer_Compat(buf, w, &b.save);
        CHECK(errLoad == SAVEGAME_ERROR_BAD_CRC,
              "Tampered PARTY byte -> BAD_CRC");
        free(buf);
    }
    {
        struct SaveGameFixture a, b;
        unsigned char* buf = malloc(62080);
        int w = 0;
        int errLoad;
        build_populated_fixture(&a);
        build_zero_fixture(&b);
        F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, buf, 62080, &w);
        buf[0x18] ^= 0x01u;  /* Flip bit inside CRC field itself. */
        errLoad = F0774_SAVEGAME_LoadFromBuffer_Compat(buf, w, &b.save);
        CHECK(errLoad == SAVEGAME_ERROR_BAD_CRC,
              "Tampered CRC byte -> BAD_CRC");
        free(buf);
    }
    {
        struct SaveGameFixture a, b;
        unsigned char* buf = malloc(62080);
        int w = 0;
        int errLoad;
        build_populated_fixture(&a);
        build_zero_fixture(&b);
        F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, buf, 62080, &w);
        errLoad = F0774_SAVEGAME_LoadFromBuffer_Compat(buf, 32, &b.save);
        CHECK(errLoad == SAVEGAME_ERROR_BUFFER_TOO_SMALL,
              "Truncated to 32 bytes (inside header) -> BUFFER_TOO_SMALL");
        free(buf);
    }
    {
        struct SaveGameFixture a, b;
        unsigned char* buf = malloc(62080);
        int w = 0;
        int errLoad;
        build_populated_fixture(&a);
        build_zero_fixture(&b);
        F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, buf, 62080, &w);
        errLoad = F0774_SAVEGAME_LoadFromBuffer_Compat(buf, 100, &b.save);
        CHECK(errLoad == SAVEGAME_ERROR_BAD_SIZE,
              "Truncated to 100 bytes (inside section table) -> BAD_SIZE");
        free(buf);
    }
    {
        struct SaveGameFixture a, b;
        unsigned char* buf = malloc(62080);
        int w = 0;
        int errLoad;
        build_populated_fixture(&a);
        build_zero_fixture(&b);
        F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, buf, 62080, &w);
        errLoad = F0774_SAVEGAME_LoadFromBuffer_Compat(buf, 64 + 112 + 500, &b.save);
        CHECK(errLoad == SAVEGAME_ERROR_BAD_SIZE,
              "Truncated to mid-PARTY (676 bytes) -> BAD_SIZE");
        free(buf);
    }
    {
        /* Construct a header with version 0xFFFF0001 and re-stamp CRC. */
        struct SaveGameFixture a, b;
        unsigned char* buf = malloc(62080);
        int w = 0;
        int errLoad;
        struct SaveGameHeader_Compat hdr;
        uint32_t crc;
        build_populated_fixture(&a);
        build_zero_fixture(&b);
        F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, buf, 62080, &w);
        /* Read header, mutate version, re-stamp body CRC (body is
         * unchanged so reuse the existing one) — note: version lives
         * at offset 0x08, outside the CRC range [64..end), so CRC
         * stays valid but version check fires. */
        F0788_SAVEGAME_InspectHeader_Compat(buf, w, &hdr);
        hdr.formatVersion = 0xFFFF0001u;
        crc = hdr.bodyCRC32;
        (void)crc;
        /* Rewrite header bytes in-place. */
        {
            unsigned char* p = buf;
            p[0] = hdr.magic[0]; p[1] = hdr.magic[1]; p[2] = hdr.magic[2];
            p[3] = hdr.magic[3]; p[4] = hdr.magic[4]; p[5] = hdr.magic[5];
            p[6] = hdr.magic[6]; p[7] = hdr.magic[7];
            /* Encode formatVersion LE at offset 8. */
            p[8]  = (unsigned char)(hdr.formatVersion & 0xFFu);
            p[9]  = (unsigned char)((hdr.formatVersion >> 8) & 0xFFu);
            p[10] = (unsigned char)((hdr.formatVersion >> 16) & 0xFFu);
            p[11] = (unsigned char)((hdr.formatVersion >> 24) & 0xFFu);
        }
        errLoad = F0774_SAVEGAME_LoadFromBuffer_Compat(buf, w, &b.save);
        CHECK(errLoad == SAVEGAME_ERROR_BAD_VERSION,
              "Future formatVersion (0xFFFF0001) -> BAD_VERSION");
        free(buf);
    }

    /* ==============================================================
     *  Block G — NULL safety (invariants 37-40)
     * ============================================================== */
    {
        unsigned char buf[64];
        int n = 0;
        memset(buf, 0, sizeof(buf));
        CHECK(F0773_SAVEGAME_SaveToBuffer_Compat(NULL, buf, sizeof(buf), &n) ==
              SAVEGAME_ERROR_NULL_ARG,
              "F0773(NULL state) returns NULL_ARG");
    }
    {
        struct SaveGameFixture z;
        build_zero_fixture(&z);
        CHECK(F0774_SAVEGAME_LoadFromBuffer_Compat(NULL, 100, &z.save) ==
              SAVEGAME_ERROR_NULL_ARG,
              "F0774(NULL buf) returns NULL_ARG");
    }
    CHECK(F0770_SAVEGAME_CRC32_Compat(NULL, 0) == 0u,
          "F0770(NULL, 0) returns 0 (no deref)");
    {
        const char* s = F0789_SAVEGAME_ErrorToString_Compat(-1);
        CHECK(s != NULL && s[0] != '\0',
              "F0789(-1) returns non-NULL diagnostic string");
    }

    /* ==============================================================
     *  Block H — file-IO round-trip (invariant 41)
     *  Use $OUT_DIR/phase15-test.dat per plan §5 R5.
     * ============================================================== */
    {
        struct SaveGameFixture a, b;
        unsigned char* bufA = malloc(62080);
        unsigned char* bufB = malloc(62080);
        int wA = 0, wB = 0;
        int err_save, err_load, err_re;
        int firstDiff = -1;
        int rc;
        char filePath[512];
        snprintf(filePath, sizeof(filePath), "%s/phase15-test.dat", outputDir);
        build_populated_fixture(&a);
        build_zero_fixture(&b);
        F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, bufA, 62080, &wA);
        err_save = F0785_SAVEGAME_SaveToFile_Compat(filePath, &a.save);
        err_load = F0786_SAVEGAME_LoadFromFile_Compat(filePath, &b.save);
        err_re   = F0773_SAVEGAME_SaveToBuffer_Compat(&b.save, bufB, 62080, &wB);
        rc = (err_save == SAVEGAME_OK && err_load == SAVEGAME_OK &&
              err_re == SAVEGAME_OK && wA == wB &&
              F0787_SAVEGAME_Compare_Compat(bufA, wA, bufB, wB, &firstDiff) == 1);
        CHECK(rc,
              "F0785 -> F0786 -> F0773 matches original in-memory save bit-for-bit");
        /* Do NOT remove the file — downstream verify may want to
         * inspect it; it sits in $OUT_DIR which is per-run. */
        free(bufA); free(bufB);
    }

    /* ==============================================================
     *  Block I — edge-zero + edge-max (invariants 42-44)
     * ============================================================== */
    {
        struct SaveGameFixture a, b;
        unsigned char* bufA = malloc(62080);
        unsigned char* bufB = malloc(62080);
        int wA = 0, wB = 0;
        int firstDiff = -1;
        int rc;
        build_zero_fixture(&a);
        build_zero_fixture(&b);
        F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, bufA, 62080, &wA);
        F0774_SAVEGAME_LoadFromBuffer_Compat(bufA, wA, &b.save);
        F0773_SAVEGAME_SaveToBuffer_Compat(&b.save, bufB, 62080, &wB);
        rc = (F0787_SAVEGAME_Compare_Compat(bufA, wA, bufB, wB, &firstDiff) == 1);
        CHECK(rc,
              "Edge-zero (every subsystem init-only, 0 mutations, RNG seed=0) round-trip memcmp==0");
        free(bufA); free(bufB);
    }
    {
        struct SaveGameFixture a, b;
        unsigned char* buf = malloc(62080);
        int w = 0;
        int errSave, errLoad;
        int rc;
        build_populated_fixture(&a);
        build_full_timeline(&a.timeline);  /* overwrite to 256 events */
        build_zero_fixture(&b);
        errSave = F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, buf, 62080, &w);
        errLoad = F0774_SAVEGAME_LoadFromBuffer_Compat(buf, w, &b.save);
        rc = (errSave == SAVEGAME_OK && errLoad == SAVEGAME_OK &&
              b.timeline.count == TIMELINE_QUEUE_CAPACITY &&
              b.timeline.events[255].fireAtTick == 255u &&
              b.timeline.events[0].fireAtTick == 0u);
        CHECK(rc,
              "Edge-max timeline: 256 events; after load count==256 and events[255].fireAtTick==255");
        free(buf);
    }
    {
        struct SaveGameFixture a, b;
        unsigned char* buf = malloc(62080);
        int w = 0;
        int errSave, errLoad;
        int rc;
        build_populated_fixture(&a);
        build_full_mutations(&a.mutations);  /* overwrite to 1024 */
        build_zero_fixture(&b);
        errSave = F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, buf, 62080, &w);
        errLoad = F0774_SAVEGAME_LoadFromBuffer_Compat(buf, w, &b.save);
        rc = (errSave == SAVEGAME_OK && errLoad == SAVEGAME_OK &&
              b.mutations.count == DUNGEON_MUTATION_LIST_MAX_COUNT &&
              memcmp(&a.mutations.entries[0],
                     &b.mutations.entries[0],
                     sizeof(a.mutations.entries[0])) == 0 &&
              memcmp(&a.mutations.entries[1023],
                     &b.mutations.entries[1023],
                     sizeof(a.mutations.entries[0])) == 0);
        CHECK(rc,
              "Edge-max mutations: 1024 entries; after load entries[0] and entries[1023] match original");
        free(buf);
    }

    /* ==============================================================
     *  Block J — integration / semantic parity (invariant 45)
     *  Run Phase 13 F0736 against the loaded snapshot and compare
     *  bit-for-bit against the same call on the pre-save state.
     * ============================================================== */
    {
        struct SaveGameFixture a, b;
        unsigned char* buf = malloc(62080);
        int w = 0;
        struct CombatantChampionSnapshot_Compat defender_pre, defender_post;
        struct CombatantCreatureSnapshot_Compat attacker;
        struct CombatResult_Compat result_pre, result_post;
        struct RngState_Compat rng_pre, rng_post;
        int rc;

        build_populated_fixture(&a);
        /* Seed the pre-save combat RNG deterministically. */
        F0730_COMBAT_RngInit_Compat(&a.combat.rng, 0xA11CE123u);

        /* Run F0736 against the in-memory state BEFORE saving. */
        build_defender_snapshot(&defender_pre);
        build_creature_snapshot(&attacker);
        memset(&result_pre, 0, sizeof(result_pre));
        rng_pre = a.combat.rng;  /* copy, so save observes same rng */
        F0736_COMBAT_ResolveCreatureMelee_Compat(
            &attacker, &defender_pre, &rng_pre, &result_pre);

        /* Save -> load -> F0736 on loaded state. */
        build_zero_fixture(&b);
        F0773_SAVEGAME_SaveToBuffer_Compat(&a.save, buf, 62080, &w);
        F0774_SAVEGAME_LoadFromBuffer_Compat(buf, w, &b.save);

        build_defender_snapshot(&defender_post);
        memset(&result_post, 0, sizeof(result_post));
        rng_post = b.combat.rng;
        F0736_COMBAT_ResolveCreatureMelee_Compat(
            &attacker, &defender_post, &rng_post, &result_post);

        rc = (memcmp(&result_pre, &result_post, sizeof(result_pre)) == 0 &&
              rng_pre.seed == rng_post.seed &&
              a.combat.rng.seed == b.combat.rng.seed);
        CHECK(rc,
              "Post-load F0736 bit-identical to pre-save (outcome/damage/rng-seed parity)");
        free(buf);
    }

    /* ==============================================================
     *  Trailer
     * ============================================================== */
    fprintf(invariants, "\nInvariant count: %d\n", invariantCount);
    if (failCount == 0) {
        fprintf(invariants, "Status: PASS\n");
    } else {
        fprintf(invariants, "Status: FAIL (%d failures)\n", failCount);
    }
    fclose(invariants);
    fclose(report);
    return failCount > 0 ? 1 : 0;
}
