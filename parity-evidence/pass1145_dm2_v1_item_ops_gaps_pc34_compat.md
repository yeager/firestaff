# Pass 1145 — DM2 item_ops gaps filled

## Source

skproject/SKULLWIN/c_item.cpp

## Module

include/dm2_v1_item_ops_pc34_compat.h
src/dm2/dm2_v1_item_ops_pc34_compat.c

## Functions added

| Function | skproject source | Description |
|----------|-----------------|-------------|
| dm2_v1_retrieve_item_bonus | DM2_RETRIEVE_ITEM_BONUS (line 22) | Query item bonus with conditional/equipped logic |
| dm2_v1_get_max_charge | DM2_GET_MAX_CHARGE (line 344) | Max charge by db_type (pure function) |
| dm2_v1_add_item_charge | DM2_ADD_ITEM_CHARGE (line 251) | Read/modify charge count in record |
| dm2_v1_query_item_weight | DM2_QUERY_ITEM_WEIGHT (line 496) | Query weight via item_value category 1 |
| dm2_v1_get_item_name | DM2_GET_ITEM_NAME (line 502) | Get item name, handles hero bones |

## Architecture

Callback-based with DM2_V1_ItemBonusCallbacks, DM2_V1_ChargeCallbacks,
DM2_V1_ItemNameCallbacks. DM2_V1_ItemNameReceipt returns name and hero_index.

## Verification

Compiles without errors via ninja -C build.
