#include "dm1_v1_creature_sound_pc34_compat.h"
#include "dm1_v1_creature_render_pc34_compat.h"

/* ReDMCSB DUNGEON.C:667-733, I34E G0243 CreatureInfo AttackSoundOrdinal. */
static const int8_t kAttackOrdinalByCreatureType[DM1_CREATURE_TYPE_COUNT] = {
    4, 14, 6, 0, 18, 17, 3, 7, 2, 10, 13, 0, 11, 9,
    16, 5, 10, 15, 12, 0, 8, 3, 16, 0, 1, 0, 0
};

/* ReDMCSB DUNGEON.C:735-753, G2003_aauc_CreatureSounds[18][2]. */
static const int8_t kCreatureSounds[DM1_CREATURE_ATTACK_SOUND_ORDINAL_COUNT][2] = {
    { DM1_SND_ATTACK_PAIN_RAT,          DM1_SND_MOVE_RED_DRAGON },
    { DM1_SND_ATTACK_MUMMY_GHOST,       DM1_SND_NONE },
    { DM1_SND_ATTACK_SCREAMER_OITU,     DM1_SND_MOVE_SCREAMER_ROCK },
    { DM1_SND_ATTACK_SCORPION,          DM1_SND_MOVE_SCREAMER_ROCK },
    { DM1_SND_ATTACK_WORM,              DM1_SND_MOVE_SCREAMER_ROCK },
    { DM1_SND_ATTACK_GIGGLER,           DM1_SND_MOVE_MUMMY_TROLIN },
    { DM1_SND_ATTACK_ROCK,              DM1_SND_MOVE_SCREAMER_ROCK },
    { DM1_SND_ATTACK_WATER_ELEMENTAL,   DM1_SND_MOVE_SLIME_WATER },
    { DM1_SND_ATTACK_COUATL,            DM1_SND_MOVE_COUATL_WASP },
    { DM1_SND_WOODEN_THUD,              DM1_SND_MOVE_MUMMY_TROLIN },
    { DM1_SND_COMBAT,                   DM1_SND_MOVE_SKELETON },
    { DM1_SND_COMBAT,                   DM1_SND_MOVE_ANIMATED_ARMOUR },
    { DM1_SND_ATTACK_MUMMY_GHOST,       DM1_SND_MOVE_MUMMY_TROLIN },
    { DM1_SND_NONE,                     DM1_SND_MOVE_SLIME_WATER },
    { DM1_SND_NONE,                     DM1_SND_MOVE_COUATL_WASP },
    { DM1_SND_NONE,                     DM1_SND_MOVE_MUMMY_TROLIN },
    { DM1_SND_NONE,                     DM1_SND_MOVE_SCREAMER_ROCK },
    { DM1_SND_ATTACK_PAIN_RAT,          DM1_SND_MOVE_SCREAMER_ROCK }
};

int DM1_CreatureSound_AttackOrdinalForType(int creatureType) {
    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return DM1_CREATURE_ATTACK_SOUND_ORDINAL_NONE;
    return kAttackOrdinalByCreatureType[creatureType];
}

int DM1_CreatureSound_AttackIndexForOrdinal(int attackSoundOrdinal) {
    if (attackSoundOrdinal <= 0 || attackSoundOrdinal > DM1_CREATURE_ATTACK_SOUND_ORDINAL_COUNT) return DM1_SND_NONE;
    return kCreatureSounds[attackSoundOrdinal - 1][DM1_CREATURE_SOUND_ATTACK];
}

int DM1_CreatureSound_MovementIndexForOrdinal(int attackSoundOrdinal) {
    if (attackSoundOrdinal <= 0 || attackSoundOrdinal > DM1_CREATURE_ATTACK_SOUND_ORDINAL_COUNT) return DM1_SND_NONE;
    return kCreatureSounds[attackSoundOrdinal - 1][DM1_CREATURE_SOUND_MOVEMENT];
}

int DM1_CreatureSound_AttackIndexForType(int creatureType, int useSpellFallback) {
    int soundIndex = DM1_CreatureSound_AttackIndexForOrdinal(DM1_CreatureSound_AttackOrdinalForType(creatureType));
    if (soundIndex == DM1_SND_NONE && useSpellFallback) return DM1_SND_SPELL;
    return soundIndex;
}

int DM1_CreatureSound_MovementIndexForType(int creatureType, int partyIsResting) {
    if (partyIsResting) return DM1_SND_NONE;
    return DM1_CreatureSound_MovementIndexForOrdinal(DM1_CreatureSound_AttackOrdinalForType(creatureType));
}

int DM1_CreatureSound_AspectUpdateMovementIndexForType(int creatureType,
                                                       int attacking,
                                                       int randomBit,
                                                       int partyIsResting) {
    /* ReDMCSB GROUP.C:267-281: only Couatl idle aspect updates call
     * F0514_MOVE_GetSound, and only when M005_RANDOM(2) is true. */
    if (creatureType != DM1_CREATURE_COUATL) return DM1_SND_NONE;
    if (attacking || ((randomBit & 1) == 0)) return DM1_SND_NONE;
    return DM1_CreatureSound_MovementIndexForType(creatureType, partyIsResting);
}

void DM1_CreatureSound_RequestAttack(DM1_SoundSystem* sys,
                                     int creatureType,
                                     int16_t mapX,
                                     int16_t mapY,
                                     int useSpellFallback) {
    int soundIndex = DM1_CreatureSound_AttackIndexForType(creatureType, useSpellFallback);
    if (soundIndex != DM1_SND_NONE) {
        DM1_Sound_RequestPlay(sys, (int16_t)soundIndex, mapX, mapY, DM1_MODE_PLAY_IF_PRIORITIZED);
    }
}

void DM1_CreatureSound_RequestMovement(DM1_SoundSystem* sys,
                                       int creatureType,
                                       int16_t mapX,
                                       int16_t mapY,
                                       int partyIsResting) {
    int soundIndex = DM1_CreatureSound_MovementIndexForType(creatureType, partyIsResting);
    if (soundIndex != DM1_SND_NONE) {
        DM1_Sound_RequestPlay(sys, (int16_t)soundIndex, mapX, mapY, DM1_MODE_PLAY_IF_PRIORITIZED);
    }
}

void DM1_CreatureSound_RequestAspectUpdateMovement(DM1_SoundSystem* sys,
                                                   int creatureType,
                                                   int16_t mapX,
                                                   int16_t mapY,
                                                   int attacking,
                                                   int randomBit,
                                                   int partyIsResting) {
    int soundIndex = DM1_CreatureSound_AspectUpdateMovementIndexForType(
        creatureType, attacking, randomBit, partyIsResting);
    if (soundIndex != DM1_SND_NONE) {
        DM1_Sound_RequestPlay(sys, (int16_t)soundIndex, mapX, mapY, DM1_MODE_PLAY_IF_PRIORITIZED);
    }
}

const char* DM1_CreatureSound_SourceEvidence(void) {
    return "GROUP.C:1806-1815 attack ordinal emission; "
           "MOVESENS.C:847-854 movement emission; "
           "MOVESENS.C:984-995 F0514_MOVE_GetSound I34E resting/ordinal gate; "
           "DUNGEON.C:667-733 I34E AttackSoundOrdinal table; "
           "DUNGEON.C:735-753 G2003_aauc_CreatureSounds; "
           "GROUP.C:267-281 Couatl idle aspect update movement sound gate; "
           "DEFS.H:100-133 I34E sound constants; "
           "SOUND.C:1475-1640 F0064 request/pending gate";
}
