#ifndef FIRESTAFF_DM1_V1_CREATURE_SOUND_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CREATURE_SOUND_PC34_COMPAT_H

/*
 * DM1 V1 creature sound event selection, source-locked to ReDMCSB.
 *
 * Source anchors:
 *   GROUP.C:1806-1815   creature attack sound ordinal -> sound index -> F0064
 *   MOVESENS.C:847-854  group movement emits F0514_MOVE_GetSound result
 *   MOVESENS.C:984-995  F0514_MOVE_GetSound I34E: resting gate, ordinal table
 *   DUNGEON.C:667-733   I34E CreatureInfo AttackSoundOrdinal per creature type
 *   DUNGEON.C:735-753   G2003_aauc_CreatureSounds[18][2]
 *   DEFS.H:100-133      I34E sound indexes and attack/movement ranges
 *   SOUND.C:1475-1640   F0064 request gate/pending state
 */

#include "dm1_v1_sound_pc34_compat.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_CREATURE_ATTACK_SOUND_ORDINAL_NONE 0
#define DM1_CREATURE_ATTACK_SOUND_ORDINAL_COUNT 18

enum {
    DM1_CREATURE_SOUND_ATTACK = 0,
    DM1_CREATURE_SOUND_MOVEMENT = 1
};

int DM1_CreatureSound_AttackOrdinalForType(int creatureType);
int DM1_CreatureSound_AttackIndexForOrdinal(int attackSoundOrdinal);
int DM1_CreatureSound_MovementIndexForOrdinal(int attackSoundOrdinal);
int DM1_CreatureSound_AttackIndexForType(int creatureType, int useSpellFallback);
int DM1_CreatureSound_MovementIndexForType(int creatureType, int partyIsResting);

void DM1_CreatureSound_RequestAttack(DM1_SoundSystem* sys,
                                     int creatureType,
                                     int16_t mapX,
                                     int16_t mapY,
                                     int useSpellFallback);
void DM1_CreatureSound_RequestMovement(DM1_SoundSystem* sys,
                                       int creatureType,
                                       int16_t mapX,
                                       int16_t mapY,
                                       int partyIsResting);

const char* DM1_CreatureSound_SourceEvidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CREATURE_SOUND_PC34_COMPAT_H */
