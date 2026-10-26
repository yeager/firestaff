#include "dm2_v1_runtime.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include <string.h>

int dm2_v1_runtime_live_missile_selector_receipt(
    int missile_record_handle,
    DM2_V1_G1DirectMissileReceipt *out_missile,
    DM2_V1_G1FlyingItemSelectorReceipt *out_selector)
{
    const DM2_V1_DungeonData *dungeon = NULL;
    const DM2_V1_RecordPoolSet *pools = NULL;
    const DM2_V1_AssetLoader *loader = NULL;

    if (out_missile) memset(out_missile, 0, sizeof(*out_missile));
    if (out_selector) memset(out_selector, 0, sizeof(*out_selector));
    if (!out_missile || !out_selector ||
        !dm2_v1_runtime_live_missile_selector_context(
            &dungeon, &pools, &loader)) {
        return 0;
    }
    return dm2_v1_g1_live_flying_item_selector_receipt(
        dungeon, pools, loader, (uint16_t)missile_record_handle,
        out_missile, out_selector);
}
