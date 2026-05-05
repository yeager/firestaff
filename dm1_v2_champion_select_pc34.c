#include "dm1_v2_champion_select_pc34.h"

#define V2_CHAMPION_SELECT_MAX 24

static struct M11_V2_ChampionEntry g_champions[V2_CHAMPION_SELECT_MAX];
static int g_current_index = 0;
static bool g_initialized = false;

void v2_champion_select_init(void) {
    memset(g_champions, 0, sizeof(g_champions));
    g_current_index = 0;
    g_initialized = true;

    const char* default_names[] = {"Warrior", "Mage", "Merlin", "Ranger", "Rogue", "Cleric"};
    for (int i = 0; i < 6; ++i) {
        g_champions[i].cls = (enum M11_V2_ChampionClass)i;
        strncpy(g_champions[i].name, default_names[i], 31);
        g_champions[i].name[31] = '\0';
        g_champions[i].selected = false;
        g_champions[i].tile_x = (i % 4) * 10;
        g_champions[i].tile_y = (i / 4) * 10;
    }
}

void v2_champion_select_render(void) {
    if (!g_initialized) return;
    /* Placeholder for rendering logic */
}

void v2_champion_select_cycle_forward(void) {
    if (!g_initialized) return;
    g_current_index = (g_current_index + 1) % V2_CHAMPION_SELECT_MAX;
}

void v2_champion_select_cycle_backward(void) {
    if (!g_initialized) return;
    g_current_index = (g_current_index - 1 + V2_CHAMPION_SELECT_MAX) % V2_CHAMPION_SELECT_MAX;
}

void v2_champion_select_toggle(void) {
    if (!g_initialized) return;
    g_champions[g_current_index].selected = !g_champions[g_current_index].selected;
}

int v2_champion_select_focus_index_pc34(unsigned int championIndex) {
    if (!g_initialized || championIndex >= 4u) return 0;
    g_current_index = (int)championIndex;
    return 1;
}

int v2_champion_select_current_index_pc34(void) {
    if (!g_initialized) return -1;
    return g_current_index;
}

const struct M11_V2_ChampionEntry* v2_champion_select_get(void) {
    if (!g_initialized) return NULL;
    return &g_champions[g_current_index];
}

int v2_champion_select_count(void) {
    if (!g_initialized) return 0;
    int count = 0;
    for (int i = 0; i < V2_CHAMPION_SELECT_MAX; ++i) {
        if (g_champions[i].selected) {
            count++;
        }
    }
    return count;
}
