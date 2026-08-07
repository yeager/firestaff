#include "dm1_v2_inventory_sort_pc34.h"

#include <stddef.h>

/* PC34 inventory is a source-owned slot layout.  Do not duplicate item names,
 * weights, values or reorder slots through a host-only 64-entry view. */
void v2_inv_init(void) {}
bool v2_inv_add_item(const char* name, int weight, int value, M11_V2_ItemCategory cat) { (void)name; (void)weight; (void)value; (void)cat; return false; }
void v2_inv_sort(M11_V2_SortMode mode) { (void)mode; }
void v2_inv_filter(M11_V2_ItemCategory cat) { (void)cat; }
void v2_inv_clear_filter(void) {}
bool v2_inv_remove(int idx) { (void)idx; return false; }
const M11_V2_InventoryItem* v2_inv_get(int idx) { (void)idx; return NULL; }
int v2_inv_count(void) { return 0; }
