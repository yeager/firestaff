#include "dm1_v2_inventory_sort_pc34.h"

static M11_V2_InventoryView g_inv;
static int s_actual_count = 0;

void v2_inv_init(void) {
    memset(&g_inv, 0, sizeof(g_inv));
    g_inv.mode = SORT_BY_NAME;
    g_inv.filter_active = false;
    s_actual_count = 0;
}

bool v2_inv_add_item(const char* name, int weight, int value, M11_V2_ItemCategory cat) {
    if (s_actual_count >= 64 || !name) return false;
    M11_V2_InventoryItem* item = &g_inv.items[s_actual_count];
    strncpy(item->name, name, 31);
    item->name[31] = '\0';
    item->weight = weight;
    item->value = value;
    item->cat = cat;
    item->slot_idx = s_actual_count;
    s_actual_count++;
    if (!g_inv.filter_active) {
        g_inv.count = s_actual_count;
    }
    return true;
}

void v2_inv_sort(M11_V2_SortMode mode) {
    g_inv.mode = mode;
    if (g_inv.count <= 1) return;

    for (int i = 1; i < g_inv.count; i++) {
        M11_V2_InventoryItem key = g_inv.items[i];
        int j = i - 1;
        while (j >= 0) {
            int cmp = 0;
            switch (g_inv.mode) {
                case SORT_BY_NAME: cmp = strcmp(g_inv.items[j].name, key.name); break;
                case SORT_BY_TYPE: cmp = (int)g_inv.items[j].cat - (int)key.cat; break;
                case SORT_BY_WEIGHT: cmp = g_inv.items[j].weight - key.weight; break;
                case SORT_BY_VALUE: cmp = g_inv.items[j].value - key.value; break;
                default: cmp = 0; break;
            }
            if (cmp <= 0) break;
            g_inv.items[j + 1] = g_inv.items[j];
            j--;
        }
        g_inv.items[j + 1] = key;
    }
}

void v2_inv_filter(M11_V2_ItemCategory cat) {
    g_inv.filter = cat;
    g_inv.filter_active = true;
    int write_idx = 0;
    for (int i = 0; i < s_actual_count; i++) {
        if (g_inv.items[i].cat == cat) {
            if (write_idx != i) {
                g_inv.items[write_idx] = g_inv.items[i];
            }
            write_idx++;
        }
    }
    g_inv.count = write_idx;
}

void v2_inv_clear_filter(void) {
    g_inv.filter_active = false;
    g_inv.count = s_actual_count;
}

bool v2_inv_remove(int idx) {
    if (idx < 0 || idx >= g_inv.count) return false;
    for (int i = idx; i < g_inv.count - 1; i++) {
        g_inv.items[i] = g_inv.items[i + 1];
    }
    g_inv.count--;
    s_actual_count--;
    for (int i = 0; i < g_inv.count; i++) {
        g_inv.items[i].slot_idx = i;
    }
    return true;
}

const M11_V2_InventoryItem* v2_inv_get(int idx) {
    if (idx < 0 || idx >= g_inv.count) return NULL;
    return &g_inv.items[idx];
}

int v2_inv_count(void) {
    return g_inv.count;
}
