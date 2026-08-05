#ifndef FIRESTAFF_DM2_V1_CHAMPION_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CHAMPION_LIFECYCLE_PC34_COMPAT_H

/*
 * dm2_v1_champion_lifecycle_pc34_compat.h — DM2 champion selection
 * and resurrection.
 *
 * Ports DM2_SELECT_CHAMPION and DM2_BRING_CHAMPION_TO_LIFE from
 * skproject/SKULLWIN/c_hero.cpp.
 *
 * SELECT_CHAMPION (c_hero.cpp:1052-1200):
 *   - Guard: savegamewpc.w_00 must be 0xFFFF (in-game state)
 *   - Guard: heros_in_party < 4
 *   - CHANGE_CURRENT_MAP_TO the champion's map
 *   - Walk tile record chain for DB type 3 (text record) with
 *     subtype 0x7E (champion mirror/alcove marker)
 *   - Extract hero type from bits 7+ of word at record+2
 *   - REVIVE_PLAYER: init hero struct, set type/direction/position,
 *     clear all 30 item slots to -1, find open party position
 *   - If first hero: SELECT_CHAMPION_LEADER(0)
 *   - Increment party.heros_in_party, update right panel
 *   - Move hero possessions from tile to hero inventory
 *
 * BRING_CHAMPION_TO_LIFE (c_hero.cpp:916-953):
 *   - R_36EFE: reset hero state
 *   - Clear all 30 item slots to -1
 *   - maxHP = max(25, maxHP - maxHP/64 - 1) (resurrection penalty)
 *   - curHP = maxHP / 2
 *   - Set heroflag |= 0x4000 (alive)
 *   - Clear enchantments (aura, power)
 *
 * Source: skproject/SKULLWIN/c_hero.cpp
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SELECT_CHAMPION request */
typedef struct {
    int16_t tile_x;
    int16_t tile_y;
    int16_t direction;
    int16_t map_level;
    int16_t heroes_in_party;
} DM2_V1_SelectChampionRequest;

/* SELECT_CHAMPION receipt. A valid request is not a selected champion:
 * selection remains fail-closed until the live DB3 mirror and hero records
 * are supplied by the GAME_LOAD runtime. */
typedef struct {
    int valid;
    int fail_closed;
    int champion_selected;
    int party_full;
    int16_t hero_index;
    int16_t hero_type;
    int16_t hero_position;
} DM2_V1_SelectChampionReceipt;

/* BRING_CHAMPION_TO_LIFE request */
typedef struct {
    int16_t hero_index;
    int16_t current_max_hp;
} DM2_V1_BringChampionToLifeRequest;

/* BRING_CHAMPION_TO_LIFE receipt.
 * Source: c_hero.cpp:916-954 DM2_BRING_CHAMPION_TO_LIFE */
typedef struct {
    int valid;
    int fail_closed;
    int champion_revived;
    int16_t new_max_hp;
    int16_t new_cur_hp;
    uint16_t heroflag_set_bits;
    uint8_t clear_weight;
    uint8_t clear_ench_aura;
    uint8_t clear_ench_power;
    uint8_t clear_item_slots;
    uint8_t item_slot_count;
} DM2_V1_BringChampionToLifeReceipt;

int dm2_v1_select_champion(
    const DM2_V1_SelectChampionRequest *request,
    DM2_V1_SelectChampionReceipt *receipt);

int dm2_v1_bring_champion_to_life(
    const DM2_V1_BringChampionToLifeRequest *request,
    DM2_V1_BringChampionToLifeReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CHAMPION_LIFECYCLE_PC34_COMPAT_H */
