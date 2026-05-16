#ifndef FIRESTAFF_DM1_V2_CHAMPION_SELECT_PC34_H
#define FIRESTAFF_DM1_V2_CHAMPION_SELECT_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum M11_V2_ChampionClass {
    CLASS_WARRIOR,
    CLASS_MAGE,
    CLASS_MERLIN,
    CLASS_RANGER,
    CLASS_ROGUE,
    CLASS_CLERIC
};

struct M11_V2_ChampionEntry {
    enum M11_V2_ChampionClass cls;
    char name[32];
    bool selected;
    int tile_x;
    int tile_y;
};

void v2_champion_select_init(void);
void v2_champion_select_render(void);
void v2_champion_select_cycle_forward(void);
void v2_champion_select_cycle_backward(void);
void v2_champion_select_toggle(void);
int v2_champion_select_focus_index_pc34(unsigned int championIndex);
int v2_champion_select_current_index_pc34(void);
const struct M11_V2_ChampionEntry* v2_champion_select_get(void);
int v2_champion_select_count(void);

#endif
