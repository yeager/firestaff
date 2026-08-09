/*
 * Round-trip gate for the F0776-F0781 savegame section (de)serialisers.
 *
 * The 2026-08-09 symbol reachability survey
 * (docs/parity/DM1_SYMBOL_REACHABILITY_SURVEY.md) found fourteen SAVEGAME
 * serialise/deserialise functions with neither a production caller nor any
 * test exercising them -- the largest single untested block in the DM1
 * source-locked surface, and the place where a silent format divergence
 * would be most expensive, because a save written by one build and refused
 * (or misread) by another is unrecoverable for the player.
 *
 * This gate closes the coverage hole. For every pair it asserts the three
 * properties that matter for a save format:
 *
 *   1. serialise returns SAVEGAME_OK (which is 0, not 1 -- these use a
 *      status enum rather than a boolean) and reports a byte count within
 *      the caller's buffer;
 *   2. deserialise into a *differently* pre-poisoned struct reproduces the
 *      original field-for-field (poisoning first so a no-op deserialise
 *      cannot masquerade as success);
 *   3. re-serialising the round-tripped value yields byte-identical output,
 *      which catches fields that survive one hop but are dropped on the way
 *      back out.
 *
 * It also pins the fail-closed behaviour on short buffers: a serialiser that
 * silently truncates would produce a save that deserialises into a plausible
 * but wrong world.
 *
 * Non-claims: this proves internal round-trip consistency only. It does NOT
 * claim byte compatibility with an original PC 3.4 save file; no original
 * save corpus is consulted here.
 */

#include "memory_savegame_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define CHECK_EQ(actual, expected, msg) do { \
    long a_ = (long)(actual); \
    long e_ = (long)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; \
        fprintf(stderr, "FAIL: %s: got %ld expected %ld\n", (msg), a_, e_); } \
} while (0)

#define BUF_CAP 65536
static unsigned char g_bufA[BUF_CAP];
static unsigned char g_bufB[BUF_CAP];

/* Fill a struct with a non-zero, non-uniform pattern so that a deserialiser
 * which forgets a field cannot accidentally match. */
static void poison(void* p, size_t n, unsigned char seed) {
    unsigned char* b = (unsigned char*)p;
    size_t i;
    for (i = 0; i < n; ++i) {
        b[i] = (unsigned char)(seed + (unsigned char)(i * 31u));
    }
}

/*
 * Shared body for one serialise/deserialise pair.
 *
 * `src` holds the populated value, `dst` is a scratch struct of the same
 * type, and `size` is sizeof that type. The callbacks adapt each pair's
 * signature.
 */
typedef int (*SerFn)(const void* obj, unsigned char* buf, int bufSize,
                     int* outBytes);
typedef int (*DeserFn)(void* obj, const unsigned char* buf, int bufSize);

static void roundtrip_case(const char* label,
                           SerFn ser,
                           DeserFn deser,
                           const void* src,
                           void* dst,
                           size_t size) {
    char msg[160];
    int bytesA = -1;
    int bytesB = -1;

    memset(g_bufA, 0xAA, sizeof(g_bufA));
    memset(g_bufB, 0x55, sizeof(g_bufB));

    snprintf(msg, sizeof(msg), "%s: serialise accepts a sufficient buffer", label);
    CHECK(ser(src, g_bufA, BUF_CAP, &bytesA) == SAVEGAME_OK, msg);

    snprintf(msg, sizeof(msg), "%s: serialise reports a positive byte count", label);
    CHECK(bytesA > 0 && bytesA <= BUF_CAP, msg);
    if (bytesA <= 0 || bytesA > BUF_CAP) {
        return;
    }

    /* Poison the destination so a deserialise that writes nothing fails. */
    poison(dst, size, 0xC3);

    snprintf(msg, sizeof(msg), "%s: deserialise accepts its own output", label);
    CHECK(deser(dst, g_bufA, bytesA) == SAVEGAME_OK, msg);

    snprintf(msg, sizeof(msg), "%s: re-serialise accepts the round-tripped value", label);
    CHECK(ser(dst, g_bufB, BUF_CAP, &bytesB) == SAVEGAME_OK, msg);

    snprintf(msg, sizeof(msg), "%s: re-serialise yields the same byte count", label);
    CHECK_EQ(bytesB, bytesA, msg);

    snprintf(msg, sizeof(msg), "%s: re-serialise is byte-identical", label);
    CHECK(bytesA == bytesB && memcmp(g_bufA, g_bufB, (size_t)bytesA) == 0, msg);

    /* Fail-closed: a buffer one byte short must be refused, not truncated. */
    if (bytesA > 1) {
        int shortBytes = -1;
        snprintf(msg, sizeof(msg),
                 "%s: serialise refuses a one-byte-short buffer", label);
        CHECK(ser(src, g_bufB, bytesA - 1, &shortBytes) != SAVEGAME_OK, msg);
    }

    /* Fail-closed: a truncated payload must be refused by the reader. */
    if (bytesA > 1) {
        snprintf(msg, sizeof(msg),
                 "%s: deserialise refuses a truncated payload", label);
        CHECK(deser(dst, g_bufA, bytesA - 1) != SAVEGAME_OK, msg);
    }
}

/* ---- per-pair adapters ------------------------------------------------ */

static int ser_party(const void* o, unsigned char* b, int n, int* out) {
    return F0776_SAVEGAME_SerializeParty_Compat(
        (const struct PartyState_Compat*)o, b, n, out);
}
static int deser_party(void* o, const unsigned char* b, int n) {
    return F0776_SAVEGAME_DeserializeParty_Compat(
        (struct PartyState_Compat*)o, b, n);
}

static int ser_movement(const void* o, unsigned char* b, int n, int* out) {
    return F0777_SAVEGAME_SerializeMovement_Compat(
        (const struct MovementResult_Compat*)o, b, n, out);
}
static int deser_movement(void* o, const unsigned char* b, int n) {
    return F0777_SAVEGAME_DeserializeMovement_Compat(
        (struct MovementResult_Compat*)o, b, n);
}

static int ser_sensor(const void* o, unsigned char* b, int n, int* out) {
    return F0778_SAVEGAME_SerializeSensor_Compat(
        (const struct SensorEffectList_Compat*)o, b, n, out);
}
static int deser_sensor(void* o, const unsigned char* b, int n) {
    return F0778_SAVEGAME_DeserializeSensor_Compat(
        (struct SensorEffectList_Compat*)o, b, n);
}

static int ser_timeline(const void* o, unsigned char* b, int n, int* out) {
    return F0779_SAVEGAME_SerializeTimeline_Compat(
        (const struct TimelineQueue_Compat*)o, b, n, out);
}
static int deser_timeline(void* o, const unsigned char* b, int n) {
    return F0779_SAVEGAME_DeserializeTimeline_Compat(
        (struct TimelineQueue_Compat*)o, b, n);
}

static int ser_combat(const void* o, unsigned char* b, int n, int* out) {
    return F0780_SAVEGAME_SerializeCombat_Compat(
        (const struct CombatScratch_Compat*)o, b, n, out);
}
static int deser_combat(void* o, const unsigned char* b, int n) {
    return F0780b_SAVEGAME_DeserializeCombat_Compat(
        (struct CombatScratch_Compat*)o, b, n);
}

static int ser_magic(const void* o, unsigned char* b, int n, int* out) {
    return F0781_SAVEGAME_SerializeMagic_Compat(
        (const struct MagicState_Compat*)o, b, n, out);
}
static int deser_magic(void* o, const unsigned char* b, int n) {
    return F0781_SAVEGAME_DeserializeMagic_Compat(
        (struct MagicState_Compat*)o, b, n);
}

/* ---- populated fixtures ----------------------------------------------- */

static void build_party(struct PartyState_Compat* p) {
    int i;
    memset(p, 0, sizeof(*p));
    /* The party section serialises ALL CHAMPION_MAX_PARTY slots, not just the
     * populated ones, and the deserialiser canonicalises actionIndex 0 to
     * 0xFF (ACTION_NONE) in every slot. An empty slot left at memset-0 would
     * therefore fail byte identity on the unpopulated tail. Canonicalise every
     * slot up front, exactly as champion init does. */
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        p->champions[i].actionIndex = 0xFFu;
    }
    p->championCount = 3;
    p->activeChampionIndex = 1;
    p->mapIndex = 4;
    p->mapX = 11;
    p->mapY = 7;
    p->direction = 2;
    for (i = 0; i < 3; ++i) {
        struct ChampionState_Compat* c = &p->champions[i];
        int s;
        c->present = 1;
        c->portraitIndex = i + 2;
        memset(c->name, ' ', sizeof(c->name));
        memcpy(c->name, "SAVE", 4);
        c->hp.current = (unsigned short)(50 + i * 7);
        c->hp.maximum = 120;
        c->stamina.current = (unsigned short)(40 + i);
        c->stamina.maximum = 90;
        c->mana.current = (unsigned short)(10 + i * 3);
        c->mana.maximum = 60;
        c->wounds = (unsigned short)(1u << i);
        c->poisonDose = (unsigned short)(i * 5);
        c->food = (int16_t)(500 - i * 40);
        c->water = (int16_t)(400 - i * 30);
        c->cell = (unsigned char)i;
        c->direction = (unsigned char)(i & 3);
        /* 0xFF is ACTION_NONE and is the value champion init uses
         * (memory_champion_state_pc34_compat.c:39). A memset-0 fixture would
         * instead set actionIndex 0, which the deserialiser deliberately
         * canonicalises back to 0xFF when actionDefense is also 0 -- see
         * test_action_index_normalisation below, which pins that rule. Using
         * the canonical value here keeps this case a pure round-trip. */
        c->actionIndex = 0xFFu;
        for (s = 0; s < CHAMPION_SLOT_COUNT; ++s) {
            c->inventory[s] = THING_NONE;
        }
    }
}

/*
 * Pin the one intentional non-identity in the party format.
 *
 * memory_champion_state_pc34_compat.c canonicalises a deserialised champion
 * whose actionIndex is 0 AND whose actionDefense is 0 back to 0xFF
 * (ACTION_NONE). A save carrying the 0/0 pair therefore does NOT re-serialise
 * byte-identically, by design. This asserts the rule directly so the round-trip
 * gate above can legitimately demand byte identity for canonical input.
 */
static void test_action_index_normalisation(void) {
    struct PartyState_Compat src, dst;
    int bytes = -1;

    build_party(&src);
    src.champions[0].actionIndex = 0;
    src.champions[0].actionDefense = 0;

    memset(g_bufA, 0, sizeof(g_bufA));
    CHECK(F0776_SAVEGAME_SerializeParty_Compat(&src, g_bufA, BUF_CAP, &bytes)
              == SAVEGAME_OK,
          "normalisation: party with actionIndex 0 serialises");
    memset(&dst, 0, sizeof(dst));
    CHECK(F0776_SAVEGAME_DeserializeParty_Compat(&dst, g_bufA, bytes)
              == SAVEGAME_OK,
          "normalisation: party deserialises");
    CHECK_EQ(dst.champions[0].actionIndex, 0xFF,
             "normalisation: actionIndex 0 with actionDefense 0 becomes ACTION_NONE");

    /* A non-zero actionDefense must suppress the canonicalisation. */
    build_party(&src);
    src.champions[0].actionIndex = 0;
    src.champions[0].actionDefense = 7;
    CHECK(F0776_SAVEGAME_SerializeParty_Compat(&src, g_bufA, BUF_CAP, &bytes)
              == SAVEGAME_OK,
          "normalisation: party with live actionDefense serialises");
    memset(&dst, 0, sizeof(dst));
    CHECK(F0776_SAVEGAME_DeserializeParty_Compat(&dst, g_bufA, bytes)
              == SAVEGAME_OK,
          "normalisation: party with live actionDefense deserialises");
    CHECK_EQ(dst.champions[0].actionIndex, 0,
             "normalisation: actionIndex 0 survives when actionDefense is set");
}

int main(void) {
    test_action_index_normalisation();
    {
        struct PartyState_Compat src, dst;
        build_party(&src);
        roundtrip_case("F0776 party", ser_party, deser_party,
                       &src, &dst, sizeof(src));
    }
    {
        struct MovementResult_Compat src, dst;
        memset(&src, 0, sizeof(src));
        src.resultCode = MOVE_OK;
        src.newMapX = 9;
        src.newMapY = 13;
        src.newDirection = 3;
        src.newMapIndex = 2;
        roundtrip_case("F0777 movement", ser_movement, deser_movement,
                       &src, &dst, sizeof(src));
    }
    {
        struct SensorEffectList_Compat src, dst;
        memset(&src, 0, sizeof(src));
        roundtrip_case("F0778 sensor", ser_sensor, deser_sensor,
                       &src, &dst, sizeof(src));
    }
    {
        struct TimelineQueue_Compat src, dst;
        memset(&src, 0, sizeof(src));
        F0720_TIMELINE_Init_Compat(&src, 100u);
        roundtrip_case("F0779 timeline", ser_timeline, deser_timeline,
                       &src, &dst, sizeof(src));
    }
    {
        struct CombatScratch_Compat src, dst;
        memset(&src, 0, sizeof(src));
        roundtrip_case("F0780 combat", ser_combat, deser_combat,
                       &src, &dst, sizeof(src));
    }
    {
        struct MagicState_Compat src, dst;
        memset(&src, 0, sizeof(src));
        src.fireShieldDefense = 12;
        src.spellShieldDefense = 8;
        src.partyShieldDefense = 5;
        roundtrip_case("F0781 magic", ser_magic, deser_magic,
                       &src, &dst, sizeof(src));
    }

    printf("dm1 savegame section round-trip: %d passed, %d failed\n",
           g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
