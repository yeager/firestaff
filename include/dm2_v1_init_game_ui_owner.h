#ifndef FIRESTAFF_DM2_V1_INIT_GAME_UI_OWNER_H
#define FIRESTAFF_DM2_V1_INIT_GAME_UI_OWNER_H

/* Private DM2__INIT_GAME UI-table owner.
 *
 * The tables are original program data transcribed in dm2_v1_hud_tables.c
 * from SKULLWIN/dm2data.cpp.  They are copied here because c_1031 mutates
 * table1d3d23 and table1d32d8.  This is deliberately not an M11 UI object.
 */

#include "dm2_v1_party.h"
#include "dm2_v1_eventqueue_pc34_compat.h"
#include "dm2_v1_skproject_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int valid;
    DM2_V1_SkprojectUiRuntimeState runtime;
    DM2_V1_SkprojectUiPredicateState predicates;
    DM2_V1_SkprojectUiNodeRef roots[10];       /* table1d3ed5 */
    DM2_V1_SkprojectUiNodeRef nodes[76];       /* table1d3ba0 */
    uint8_t child_bytes[83];                   /* table1d3cd0 */
    DM2_V1_SkprojectUiLeafMeta leaves[62];     /* table1d3d23 */
    DM2_V1_SkprojectUiClickRectNode clickrects[18]; /* table1d32d8 */
    DM2_V1_SkprojectUiSelectTreeReceipt initial_tree;
    uint32_t source_table_hash;
} DM2_V1_InitGameUiOwner;

/* startend.cpp::DM2__INIT_GAME_38c8_03ad calls DM2_1031_0541(5) before
 * LOAD_NEWMAP.  Construct that exact private table state from the selected
 * source party and c_eventqueue state.  No host input or rendering is used. */
int dm2_v1_init_game_ui_owner_init(DM2_V1_InitGameUiOwner *out,
                                   const DM2_V1_Party *party,
                                   const DM2_V1_EventQueue *event_queue);

#ifdef __cplusplus
}
#endif

#endif
