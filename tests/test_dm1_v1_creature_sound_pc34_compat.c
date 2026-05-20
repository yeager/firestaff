#include "dm1_v1_creature_sound_pc34_compat.h"
#include "dm1_v1_creature_render_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

static void test_ordinals_from_i34e_creature_info(void) {
    CHECK(DM1_CreatureSound_AttackOrdinalForType(DM1_CREATURE_GIANT_SCORPION) == 4,
          "scorpion attack ordinal = 4");
    CHECK(DM1_CreatureSound_AttackOrdinalForType(DM1_CREATURE_SWAMP_SLIME) == 14,
          "slime attack ordinal = 14");
    CHECK(DM1_CreatureSound_AttackOrdinalForType(DM1_CREATURE_SKELETON) == 11,
          "skeleton attack ordinal = 11");
    CHECK(DM1_CreatureSound_AttackOrdinalForType(DM1_CREATURE_RED_DRAGON) == 1,
          "red dragon attack ordinal = 1");
    CHECK(DM1_CreatureSound_AttackOrdinalForType(DM1_CREATURE_LORD_CHAOS) == 0,
          "lord chaos has no direct attack ordinal");
    CHECK(DM1_CreatureSound_AttackOrdinalForType(-1) == 0,
          "invalid low creature type has no ordinal");
    CHECK(DM1_CreatureSound_AttackOrdinalForType(DM1_CREATURE_TYPE_COUNT) == 0,
          "invalid high creature type has no ordinal");
}

static void test_creature_sound_table(void) {
    CHECK(DM1_CreatureSound_AttackIndexForOrdinal(1) == DM1_SND_ATTACK_PAIN_RAT,
          "ordinal 1 attack sound = pain rat / red dragon");
    CHECK(DM1_CreatureSound_MovementIndexForOrdinal(1) == DM1_SND_MOVE_RED_DRAGON,
          "ordinal 1 movement sound = red dragon");
    CHECK(DM1_CreatureSound_AttackIndexForOrdinal(11) == DM1_SND_COMBAT,
          "ordinal 11 attack sound = combat");
    CHECK(DM1_CreatureSound_MovementIndexForOrdinal(11) == DM1_SND_MOVE_SKELETON,
          "ordinal 11 movement sound = skeleton");
    CHECK(DM1_CreatureSound_AttackIndexForOrdinal(14) == DM1_SND_NONE,
          "ordinal 14 has no attack sound");
    CHECK(DM1_CreatureSound_MovementIndexForOrdinal(14) == DM1_SND_MOVE_SLIME_WATER,
          "ordinal 14 movement sound = slime/water");
    CHECK(DM1_CreatureSound_AttackIndexForOrdinal(19) == DM1_SND_NONE,
          "invalid ordinal has no attack sound");
}

static void test_type_sound_selection(void) {
    CHECK(DM1_CreatureSound_AttackIndexForType(DM1_CREATURE_GIANT_SCORPION, 0) == DM1_SND_ATTACK_SCORPION,
          "scorpion attack sound");
    CHECK(DM1_CreatureSound_MovementIndexForType(DM1_CREATURE_GIANT_SCORPION, 0) == DM1_SND_MOVE_SCREAMER_ROCK,
          "scorpion movement sound");
    CHECK(DM1_CreatureSound_AttackIndexForType(DM1_CREATURE_SWAMP_SLIME, 0) == DM1_SND_NONE,
          "slime has no direct attack sound");
    CHECK(DM1_CreatureSound_AttackIndexForType(DM1_CREATURE_SWAMP_SLIME, 1) == DM1_SND_SPELL,
          "spell fallback may be requested by spell attack path");
    CHECK(DM1_CreatureSound_MovementIndexForType(DM1_CREATURE_SWAMP_SLIME, 0) == DM1_SND_MOVE_SLIME_WATER,
          "slime movement sound");
    CHECK(DM1_CreatureSound_MovementIndexForType(DM1_CREATURE_SWAMP_SLIME, 1) == DM1_SND_NONE,
          "party resting gates movement sound");
}

static void test_request_attack_seam(void) {
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    DM1_Sound_SetPartyPosition(&sys, 10, 10, 0, 0);

    DM1_CreatureSound_RequestAttack(&sys, DM1_CREATURE_GIANT_SCORPION, 10, 10, 0);
    CHECK(sys.pending.pendingSoundIndex == DM1_SND_ATTACK_SCORPION,
          "scorpion attack queues attack sound");
    CHECK(sys.totalSoundRequests == 1, "attack request increments sound request count");

    DM1_Sound_PlayPending(&sys);
    CHECK(sys.lastPlayedSoundIndex == DM1_SND_ATTACK_SCORPION,
          "attack sound flushes through existing sound system");

    DM1_CreatureSound_RequestAttack(&sys, DM1_CREATURE_SWAMP_SLIME, 10, 10, 0);
    CHECK(sys.totalSoundRequests == 1, "none attack sound does not request playback");

    DM1_CreatureSound_RequestAttack(&sys, DM1_CREATURE_SWAMP_SLIME, 10, 10, 1);
    CHECK(sys.pending.pendingSoundIndex == DM1_SND_SPELL,
          "spell fallback queues spell sound");
}

static void test_request_movement_seam(void) {
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    DM1_Sound_SetPartyPosition(&sys, 10, 10, 0, 0);

    DM1_CreatureSound_RequestMovement(&sys, DM1_CREATURE_RED_DRAGON, 10, 10, 0);
    CHECK(sys.pending.pendingSoundIndex == DM1_SND_MOVE_RED_DRAGON,
          "red dragon movement queues movement sound");

    DM1_Sound_PlayPending(&sys);
    CHECK(sys.lastPlayedSoundIndex == DM1_SND_MOVE_RED_DRAGON,
          "red dragon movement flushes through existing sound system");

    DM1_CreatureSound_RequestMovement(&sys, DM1_CREATURE_RED_DRAGON, 10, 10, 1);
    CHECK(sys.pending.pendingSoundIndex == DM1_PENDING_NONE,
          "resting movement gate queues nothing");
}

static void test_source_evidence(void) {
    const char* evidence = DM1_CreatureSound_SourceEvidence();
    CHECK(evidence != NULL, "source evidence exists");
    CHECK(strstr(evidence, "GROUP.C:1806-1815") != NULL, "attack trigger evidence cited");
    CHECK(strstr(evidence, "MOVESENS.C:847-854") != NULL, "movement trigger evidence cited");
    CHECK(strstr(evidence, "DUNGEON.C:735-753") != NULL, "sound table evidence cited");
    CHECK(strstr(evidence, "SOUND.C:1475-1640") != NULL, "audio gate evidence cited");
}

int main(void) {
    test_ordinals_from_i34e_creature_info();
    test_creature_sound_table();
    test_type_sound_selection();
    test_request_attack_seam();
    test_request_movement_seam();
    test_source_evidence();

    printf("dm1_v1_creature_sound_pc34_compat: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
