
#ifndef FIRESTAFF_CSB_V1_CHARACTER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_CHARACTER_PC34_COMPAT_H

#include <stdint.h>

/* CSB V1 Character System
 *
 * CSB uses imported DM1 champions. Key differences:
 * - Champion stats are loaded from DM1 save/export
 * - No champion selection hall (champions come from DM1)
 * - Different starting equipment
 * - Skill caps may differ
 *
 * Source: CSBWin/Character.cpp (5528 lines)
 * Source: CSBWin/SaveGame.cpp (2953 lines)
 * Base: ReDMCSB CHAMPION.C (shared champion core)
 */

#define CSB_V1_MAX_CHAMPIONS 4
#define CSB_V1_SLOT_COUNT 30

typedef struct {
    char name[16];
    int health, max_health;
    int stamina, max_stamina;
    int mana, max_mana;
    int strength, dexterity, wisdom, vitality;
    int skills[16];
    int equipment[CSB_V1_SLOT_COUNT];
    int alive;
} CSB_V1_Champion;

typedef struct {
    CSB_V1_Champion champions[CSB_V1_MAX_CHAMPIONS];
    int champion_count;
    int imported_from_dm1;
} CSB_V1_PartyState;

int csb_v1_character_import_dm1(CSB_V1_PartyState *party, const uint8_t *dm1_save, int save_size);
void csb_v1_character_init_default(CSB_V1_PartyState *party);
const char *csb_v1_character_source_evidence(void);

#endif

