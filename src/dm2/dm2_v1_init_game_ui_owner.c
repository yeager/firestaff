#include "dm2_v1_init_game_ui_owner.h"
#include "dm2_v1_hud_tables.h"

#include <string.h>

static uint32_t dm2_v1_init_game_ui_hash(const void *bytes, size_t size)
{
    const uint8_t *p = (const uint8_t *)bytes;
    uint32_t h = 2166136261u;
    size_t i;
    for (i = 0u; i < size; ++i) h = (h ^ p[i]) * 16777619u;
    return h;
}

int dm2_v1_init_game_ui_owner_init(DM2_V1_InitGameUiOwner *out,
                                   const DM2_V1_Party *party,
                                   const DM2_V1_EventQueue *event_queue)
{
    DM2_V1_InitGameUiOwner candidate;
    unsigned i;

    if (!out || !party || !event_queue || party->heros_in_party <= 0 ||
        party->heros_in_party > 4 || event_queue->event_heroidx != 0)
        return 0;
    memset(&candidate, 0, sizeof(candidate));
    dm2_v1_skproject_ui_predicate_state_init(&candidate.predicates);
    candidate.predicates.champion_inventory = (uint8_t)party->heros_in_party;
    candidate.predicates.champion_index = 0u;
    candidate.predicates.player_dir = (uint8_t)party->absdir;
    for (i = 0u; i < 4u; ++i) {
        candidate.predicates.champion_hp[i] = party->hero[i].curHP;
        candidate.predicates.player_at_position[i] = party->hero[i].partypos;
        candidate.predicates.champion_runes_count[i] = 0u;
    }
    /* SKProject dm2data.cpp:494-505, 510-581, 576-582, 585-619. */
    for (i = 0u; i < 10u; ++i) {
        candidate.roots[i].b0 = (uint8_t)dm2_v1_hud_action_icons[i].gdat_flag;
        candidate.roots[i].b1 = (uint8_t)dm2_v1_hud_action_icons[i].param;
        candidate.roots[i].w2 = (uint16_t)dm2_v1_hud_action_icons[i].icon_id;
    }
    for (i = 0u; i < 76u; ++i) {
        candidate.nodes[i].b0 = (uint8_t)dm2_v1_hud_panel_layout[i].flags;
        candidate.nodes[i].b1 = (uint8_t)dm2_v1_hud_panel_layout[i].param;
        candidate.nodes[i].w2 = (uint16_t)dm2_v1_hud_panel_layout[i].rect_id;
    }
    memcpy(candidate.child_bytes, dm2_v1_hud_clickmap,
           sizeof(candidate.child_bytes));
    for (i = 0u; i < 62u; ++i) {
        candidate.leaves[i].w0 = (uint16_t)dm2_v1_hud_button_desc[i].gdat_category;
        candidate.leaves[i].w2 = (uint16_t)dm2_v1_hud_button_desc[i].button_id;
        candidate.leaves[i].w4 = (uint16_t)dm2_v1_hud_button_desc[i].click_target;
        candidate.leaves[i].b6 = (uint8_t)dm2_v1_hud_button_desc[i].action_type;
    }
    /* c_clickrectnode table1d32d8 is BSS and starts cleared in DM2_INIT. */
    if (!dm2_v1_skproject_1031_0541_select_tree(&candidate.runtime,
            &candidate.predicates, 5u, candidate.roots, 10u,
            candidate.nodes, 76u, candidate.child_bytes,
            sizeof(candidate.child_bytes), candidate.leaves, 62u,
            candidate.clickrects, 18u, &candidate.initial_tree) ||
        !candidate.initial_tree.valid) return 0;
    candidate.source_table_hash = dm2_v1_init_game_ui_hash(
        candidate.roots, sizeof(candidate.roots));
    candidate.source_table_hash ^= dm2_v1_init_game_ui_hash(
        candidate.nodes, sizeof(candidate.nodes));
    candidate.source_table_hash ^= dm2_v1_init_game_ui_hash(
        candidate.child_bytes, sizeof(candidate.child_bytes));
    if (candidate.source_table_hash == 0u) return 0;
    candidate.valid = 1;
    *out = candidate;
    return 1;
}
