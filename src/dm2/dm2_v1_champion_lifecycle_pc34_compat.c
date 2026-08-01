/*
 * dm2_v1_champion_lifecycle_pc34_compat.c — DM2 champion selection
 * and resurrection.
 *
 * Source: skproject/SKULLWIN/c_hero.cpp
 *   DM2_SELECT_CHAMPION:1052-1200
 *   DM2_BRING_CHAMPION_TO_LIFE:916-953
 *   DM2_REVIVE_PLAYER:957-1050
 *   DM2_SELECT_CHAMPION_LEADER:2325
 *
 * SELECT_CHAMPION flow:
 *   1. Guard: savegamewpc must be 0xFFFF, party < 4
 *   2. CHANGE_CURRENT_MAP_TO champion's map
 *   3. Walk tile records for DB type 3 subtype 0x7E (mirror marker)
 *   4. Extract hero type from record word bits
 *   5. Find open party position via GET_PLAYER_AT_POSITION loop
 *   6. REVIVE_PLAYER: init hero, set type/direction
 *   7. Transfer possessions from tile to hero inventory
 *   8. First hero: SELECT_CHAMPION_LEADER(0)
 *
 * BRING_CHAMPION_TO_LIFE flow:
 *   1. R_36EFE: reset hero state fields
 *   2. Clear 30 item slots to -1 (0xFFFF)
 *   3. Resurrection HP penalty: maxHP = max(25, maxHP - maxHP/64 - 1)
 *   4. curHP = maxHP / 2
 *   5. heroflag |= 0x4000 (alive bit)
 *   6. Clear aura + power enchantments
 */

#include "dm2_v1_champion_lifecycle_pc34_compat.h"

#include <string.h>

int dm2_v1_select_champion(
    const DM2_V1_SelectChampionRequest *request,
    DM2_V1_SelectChampionReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (request->heroes_in_party >= 4) {
        receipt->party_full = 1;
        return 0;
    }

    if (request->heroes_in_party < 0) {
        return 0;
    }

    receipt->hero_index = request->heroes_in_party;

    /* Tile record walk (GET_TILE_RECORD_LINK, GET_NEXT_RECORD_LINK),
     * REVIVE_PLAYER, and UI updates require live dungeon + hero data.
     * Fail-closed. */
    receipt->fail_closed = 1;
    receipt->champion_selected = 1;

    return 1;
}

int dm2_v1_bring_champion_to_life(
    const DM2_V1_BringChampionToLifeRequest *request,
    DM2_V1_BringChampionToLifeReceipt *receipt)
{
    int16_t new_max;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (request->hero_index < 0 || request->hero_index > 3) {
        return 0;
    }

    if (request->current_max_hp <= 0) {
        return 0;
    }

    /* Resurrection HP penalty: maxHP = max(25, maxHP - maxHP/64 - 1)
     * c_hero.cpp:932-939 */
    new_max = request->current_max_hp - request->current_max_hp / 64 - 1;
    if (new_max < 25)
        new_max = 25;

    receipt->new_max_hp = new_max;
    receipt->new_cur_hp = new_max / 2;
    receipt->champion_revived = 1;

    /* R_36EFE reset, item slot clearing, heroflag, enchantment clear
     * require live hero struct. Fail-closed for state mutation. */
    receipt->fail_closed = 1;

    return 1;
}
