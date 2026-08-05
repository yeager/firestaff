# Pass 1144 — DM2 record_ops gaps filled

## Source

skproject/SKULLWIN/c_record.cpp

## Module

include/dm2_v1_record_ops_pc34_compat.h
src/dm2/dm2_v1_record_ops_pc34_compat.c

## Functions added

| Function | skproject source | Description |
|----------|-----------------|-------------|
| dm2_v1_get_itemdb_of_itemspec_actuator | c_record.cpp | Extract db_type from actuator itemspec |
| dm2_v1_get_itemtype_of_itemspec_actuator | c_record.cpp | Extract itemtype from actuator itemspec |
| dm2_v1_query_cls1_from_record | GET_CLS1_FROM_RECORD | Query cls1 via table1d3298 lookup |
| dm2_v1_query_cls2_from_record | GET_CLS2_FROM_RECORD | Query cls2 from record data |
| dm2_v1_query_cls2_of_text_record | GET_CLS2_OF_TEXT_RECORD | Query cls2 for text records |
| dm2_v1_get_distinctive_itemtype | GET_DISTINCTIVE_ITEMTYPE | Get distinctive type via table1d3278 |
| dm2_v1_set_itemtype | SET_ITEMTYPE | Set itemtype in record word |
| dm2_v1_set_item_importance | SET_ITEM_IMPORTANCE | Set importance bits |
| dm2_v1_query_itemdb_from_distinctive_itemtype | QUERY_ITEMDB_FROM_DISTINCTIVE_ITEMTYPE | Reverse lookup |
| dm2_v1_query_creature_ai_spec_from_record | QUERY_CREATURE_AI_SPEC_FROM_RECORD | Get AI spec |
| dm2_v1_query_creature_ai_spec_from_type | QUERY_CREATURE_AI_SPEC_FROM_TYPE | Get AI spec by type |
| dm2_v1_query_creature_ai_spec_flags | QUERY_CREATURE_AI_SPEC_FLAGS | Get AI flags |

## Architecture

All functions use callback-based architecture with receipt structs where needed.
Pure C, no global state.

## Verification

Compiles without errors via ninja -C build.
