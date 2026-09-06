#include "memory_combat_pc34_compat.h"
#undef NDEBUG
#include <assert.h>
#include <string.h>

int main(void) {
    struct CombatantChampionSnapshot_Compat s = {0}, restored = {0};
    unsigned char bytes[COMBATANT_CHAMPION_SERIALIZED_SIZE];
    int defense = -1;
    s.separateBodyShield = 1;
    s.bodyShieldDefense = 40;
    s.partyShieldDefense = 90;
    /* F0313: with zero vitality/equipment, the final half-scale is 20.
     * Elemental attack subtraction is not a body-defense contributor. */
    assert(F0733_COMBAT_GetChampionWoundDefense_Compat(&s, 0, 0, &defense));
    assert(defense == 20);
    {
        struct RngState_Compat rng = {0};
        int calls = 0;
        for (int slot = 0; slot < 6; ++slot) {
            assert(F0733b_COMBAT_GetChampionWoundDefenseRng_Compat(&s, slot, 0, &rng, &defense, &calls));
            assert(defense == 20 && calls == 1);
        }
    }
    s.partyShieldDefense = 0;
    assert(F0733_COMBAT_GetChampionWoundDefense_Compat(&s, 0, 0, &defense));
    assert(defense == 20);
    {
        struct RngState_Compat rng = {0};
        int damage, calls;
        s.statisticAntifire = s.statisticAntimagic = 42;
        s.partyShieldDefense = 10;
        /* F0307: 128*(170-42)/128 = 128. F0321 fire subtracts 10,
         * then F0313's 40/2 body defense gives 118*(130-20)/64 = 202.
         * Magic jumps past body scaling; normal skips both operations. */
        assert(F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(COMBAT_ATTACK_FIRE, 128, 63, &s, &rng, &damage, &calls));
        assert(damage == 202 && calls == 6);
        assert(F0739_COMBAT_ScaleChampionDamageF0321_Compat(COMBAT_ATTACK_FIRE, 128, 63, &s, &damage));
        assert(damage == 202);
        assert(F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(COMBAT_ATTACK_MAGIC, 128, 63, &s, &rng, &damage, &calls));
        assert(damage == 118);
        assert(F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(COMBAT_ATTACK_NORMAL, 128, 63, &s, &rng, &damage, &calls));
        assert(damage == 128);
        s.bodyShieldDefense = 0;
        assert(F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(COMBAT_ATTACK_FIRE, 128, 63, &s, &rng, &damage, &calls));
        assert(damage == 239);
        s.bodyShieldDefense = 40;
    }
    memset(bytes, 0x5a, sizeof(bytes));
    assert(!F0744_COMBAT_ChampionSnapshotSerialize_Compat(&s, bytes, sizeof(bytes)));
    for (unsigned int i = 0; i < sizeof(bytes); ++i) assert(bytes[i] == 0x5a);
    s.separateBodyShield = 0;
    s.partyShieldDefense = 40;
    assert(F0744_COMBAT_ChampionSnapshotSerialize_Compat(&s, bytes, sizeof(bytes)));
    restored.separateBodyShield = 1;
    restored.bodyShieldDefense = 99;
    assert(F0745_COMBAT_ChampionSnapshotDeserialize_Compat(&restored, bytes, sizeof(bytes)));
    assert(!restored.separateBodyShield && !restored.bodyShieldDefense);
    assert(restored.partyShieldDefense == 40);
    return 0;
}
