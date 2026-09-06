#include "memory_projectile_pc34_compat.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Bounded arithmetic fixtures, not original-media or emulator evidence.
 * PROJEXPL.C F0213:165-192 owns immediate damage; F0220:822-831
 * consumes another attack roll but damages only a door. */
#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); return 1; \
} } while (0)

static int source_random(uint32_t *seed, int modulus)
{
    *seed = *seed * UINT32_C(0xbb40e62d) + UINT32_C(11);
    return (int)(((*seed >> 8) & 65535u) % (unsigned)modulus);
}

static int check_party_and_followup(void)
{
    static const int types[2] = {C000_EXPLOSION_FIREBALL, C002_EXPLOSION_LIGHTNING_BOLT};
    static const int attacks[4] = {1, 7, 80, 255};
    static const uint32_t seeds[3] = {0u, 1u, 31459u};
    int t, a, s, door;
    for (t = 0; t < 2; ++t) for (a = 0; a < 4; ++a)
    for (s = 0; s < 3; ++s) for (door = 0; door < 2; ++door) {
        struct ExplosionInstance_Compat in = {0}, next;
        struct CellContentDigest_Compat digest = {0};
        struct ExplosionTickResult_Compat burst, advance;
        struct RngState_Compat rng = {0};
        uint32_t expectedSeed = seeds[s];
        int initial = (attacks[a] >> 1) + 1;
        int followup;
        initial += source_random(&expectedSeed, initial) + 1;
        if (t) initial >>= 1;
        rng.seed = seeds[s];
        in.explosionType = types[t]; in.attack = attacks[a];
        in.sourceC15Fingerprint = 1u; /* arithmetic fixture selects source owner */
        in.mapIndex = 1; in.mapX = 2; in.mapY = 3; in.cell = 1;
        in.slotIndex = 0; in.reserved0 = 1;
        digest.destMapIndex = 1; digest.destMapX = 2; digest.destMapY = 3;
        /* Party precedence must suppress even an overlapping group. */
        digest.destHasChampion = digest.destHasCreatureGroup = 1;
        digest.destChampionCellMask = 15;
        digest.destCreatureType = 0;
        digest.destSquareType = door ? PROJECTILE_ELEMENT_DOOR : PROJECTILE_ELEMENT_CORRIDOR;
        digest.destDoorState = door ? PROJECTILE_DOOR_STATE_CLOSED_FULL : PROJECTILE_DOOR_STATE_NONE;
        CHECK(F0213_EXPLOSION_ComputeInitialBurst_Compat(&in, &digest, -1, &rng, &burst));
        CHECK(rng.seed == expectedSeed);
        CHECK(burst.emittedCombatActionPartyCount == 1);
        CHECK(burst.emittedCombatActionGroupCount == 0);
        CHECK(burst.outActionParty.rawAttackValue == initial);
        CHECK(burst.outActionParty.attackTypeCode == COMBAT_ATTACK_FIRE);
        CHECK(burst.outActionParty.allowedWounds == 63);
        CHECK(!burst.emittedDoorDestructionEvent);
        followup = (attacks[a] >> 1) + 1;
        followup += source_random(&expectedSeed, followup) + 1;
        if (t) followup >>= 1;
        CHECK(F0220_EXPLOSION_ProcessEvent25_Compat(&in, &digest, 101, &rng, &next, &advance));
        CHECK(rng.seed == expectedSeed);
        CHECK(!advance.emittedCombatActionPartyCount);
        CHECK(!advance.emittedCombatActionGroupCount);
        CHECK(advance.despawn == 1);
        CHECK(advance.emittedDoorDestructionEvent == door);
        if (door) CHECK(advance.outNextTick.aux0 == followup);
    }
    return 0;
}

static int check_group_resistance(void)
{
    static const int resistance[3] = {0, 7, 15};
    int t, nonmaterial, r;
    for (t = 0; t < 2; ++t) for (nonmaterial = 0; nonmaterial < 2; ++nonmaterial)
    for (r = 0; r < 3; ++r) {
        struct ExplosionInstance_Compat in = {0}, next;
        struct CellContentDigest_Compat digest = {0};
        struct ExplosionTickResult_Compat burst, advance;
        struct RngState_Compat rng = {0};
        uint32_t seed = 31459u;
        int attack = 128, expected;
        attack += source_random(&seed, attack) + 1;
        if (t) attack >>= 1;
        expected = attack;
        if (resistance[r] != 15) {
            if (nonmaterial) expected >>= 2;
            /* RANDOM(1) still advances the original generator. */
            expected -= source_random(&seed, 2 * resistance[r] + 1);
        }
        in.explosionType = t ? C002_EXPLOSION_LIGHTNING_BOLT : C000_EXPLOSION_FIREBALL;
        in.sourceC15Fingerprint = 1u;
        in.attack = 255; in.cell = 2; in.slotIndex = 0; in.reserved0 = 1;
        digest.destHasCreatureGroup = 1;
        digest.destCreatureIsNonMaterial = nonmaterial;
        digest.destCreatureType = 0;
        digest.destSquareType = PROJECTILE_ELEMENT_CORRIDOR;
        digest.destDoorState = PROJECTILE_DOOR_STATE_NONE;
        rng.seed = 31459u;
        CHECK(F0213_EXPLOSION_ComputeInitialBurst_Compat(&in, &digest,
                  resistance[r], &rng, &burst));
        CHECK(rng.seed == seed);
        CHECK(!burst.emittedCombatActionPartyCount);
        CHECK(burst.emittedCombatActionGroupCount == (resistance[r] != 15 && expected > 0));
        if (burst.emittedCombatActionGroupCount) {
            CHECK(burst.outActionGroup.rawAttackValue == expected);
            CHECK(burst.outActionGroup.attackTypeCode == COMBAT_ATTACK_FIRE);
        }
        /* C25 must not repeat either group resistance or group damage. */
        (void)source_random(&seed, 128);
        CHECK(F0822_EXPLOSION_Advance_Compat(&in, &digest, 1, &rng, &next, &advance));
        CHECK(rng.seed == seed);
        CHECK(!advance.emittedCombatActionGroupCount && !advance.emittedCombatActionPartyCount);
        CHECK(advance.despawn);
    }
    return 0;
}

int main(void)
{
    if (check_party_and_followup() || check_group_resistance()) return 1;
    puts("ok: F0213 immediate burst and F0220 door-only followup");
    return 0;
}
