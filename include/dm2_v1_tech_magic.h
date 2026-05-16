
#ifndef FIRESTAFF_DM2_V1_TECH_MAGIC_H
#define FIRESTAFF_DM2_V1_TECH_MAGIC_H
#include <stdint.h>

/* DM2 Tech/Magic Hybrid System
 * DM2's unique feature: combining technology and magic.
 * Tech items: guns, bombs, mechanical devices
 * Magic items: traditional DM1 spells + new spells
 * Hybrid: some items combine both (e.g., magic-powered devices)
 * Source: SKULL.ASM tech/magic item routines */

typedef enum {
    DM2_ITEM_MAGIC = 0,
    DM2_ITEM_TECH,
    DM2_ITEM_HYBRID,
} DM2_ItemAffinity;

typedef struct {
    int item_id;
    DM2_ItemAffinity affinity;
    int tech_level;
    int magic_level;
    int power_source; /* 0=manual, 1=battery, 2=mana, 3=hybrid */
    int charges;
} DM2_V1_TechMagicItem;

int dm2_v1_item_can_use(const DM2_V1_TechMagicItem *item, int champion_tech, int champion_magic);
int dm2_v1_item_power_cost(const DM2_V1_TechMagicItem *item);
const char *dm2_v1_tech_magic_source_evidence(void);
#endif

