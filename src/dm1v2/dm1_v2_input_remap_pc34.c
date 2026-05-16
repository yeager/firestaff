#include "dm1_v2_input_remap_pc34.h"
#include <string.h>
#include <stdio.h>

#define M11_V2_MAX_BINDINGS 32

static M11_V2_KeyBinding g_v2_bindings[M11_V2_MAX_BINDINGS];
static int g_v2_binding_count = 0;

static void v2_input_add_default(M11_V2_GameAction action, int primary, int alt, int gp) {
    if (g_v2_binding_count >= M11_V2_MAX_BINDINGS) return;
    g_v2_bindings[g_v2_binding_count].action = action;
    g_v2_bindings[g_v2_binding_count].primary_key = primary;
    g_v2_bindings[g_v2_binding_count].alt_key = alt;
    g_v2_bindings[g_v2_binding_count].gamepad_button = gp;
    g_v2_binding_count++;
}

void v2_input_init_defaults(void) {
    memset(g_v2_bindings, 0, sizeof(g_v2_bindings));
    g_v2_binding_count = 0;
    
    v2_input_add_default(M11_V2_GA_MOVE_FORWARD, 87, 0, 0);
    v2_input_add_default(M11_V2_GA_MOVE_BACK, 83, 0, 0);
    v2_input_add_default(M11_V2_GA_STRAFE_LEFT, 65, 0, 0);
    v2_input_add_default(M11_V2_GA_STRAFE_RIGHT, 68, 0, 0);
    v2_input_add_default(M11_V2_GA_TURN_LEFT, 65, 0, 0);
    v2_input_add_default(M11_V2_GA_TURN_RIGHT, 68, 0, 0);
    v2_input_add_default(M11_V2_GA_ATTACK, 32, 0, 0);
    v2_input_add_default(M11_V2_GA_CAST, 69, 0, 0);
    v2_input_add_default(M11_V2_GA_USE, 81, 0, 0);
    v2_input_add_default(M11_V2_GA_PICKUP, 70, 0, 0);
    v2_input_add_default(M11_V2_GA_DROP, 82, 0, 0);
    v2_input_add_default(M11_V2_GA_MAP_TOGGLE, 77, 0, 0);
    v2_input_add_default(M11_V2_GA_INVENTORY, 73, 0, 0);
    v2_input_add_default(M11_V2_GA_JOURNAL, 74, 0, 0);
    v2_input_add_default(M11_V2_GA_QUICK_SAVE, 18, 0, 0);
    v2_input_add_default(M11_V2_GA_QUICK_LOAD, 19, 0, 0);
    v2_input_add_default(M11_V2_GA_REST, 84, 0, 0);
}

void v2_input_remap(M11_V2_GameAction action, int key) {
    for (int i = 0; i < g_v2_binding_count; i++) {
        if (g_v2_bindings[i].action == action) {
            g_v2_bindings[i].primary_key = key;
            return;
        }
    }
}

int v2_input_get_action(int scancode) {
    for (int i = 0; i < g_v2_binding_count; i++) {
        if (g_v2_bindings[i].primary_key == scancode || g_v2_bindings[i].alt_key == scancode) {
            return (int)g_v2_bindings[i].action;
        }
    }
    return -1;
}

bool v2_input_save(const char* path) {
    if (!path) return false;
    FILE* f = fopen(path, "wb");
    if (!f) return false;
    
    fwrite(&g_v2_binding_count, sizeof(int), 1, f);
    fwrite(g_v2_bindings, sizeof(M11_V2_KeyBinding), g_v2_binding_count, f);
    
    fclose(f);
    return true;
}

bool v2_input_load(const char* path) {
    if (!path) return false;
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    
    memset(g_v2_bindings, 0, sizeof(g_v2_bindings));
    fread(&g_v2_binding_count, sizeof(int), 1, f);
    
    if (g_v2_binding_count > M11_V2_MAX_BINDINGS) g_v2_binding_count = M11_V2_MAX_BINDINGS;
    if (g_v2_binding_count > 0) {
        fread(g_v2_bindings, sizeof(M11_V2_KeyBinding), g_v2_binding_count, f);
    }
    
    fclose(f);
    return true;
}

void v2_input_reset_defaults(void) {
    v2_input_init_defaults();
}
