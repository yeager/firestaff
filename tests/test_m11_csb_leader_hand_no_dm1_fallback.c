/*
 * Data-free regression for the M11 CSB leader-hand handoff.
 *
 * CSBWin GAMEBLOCK2 and ReDMCSB CHAMPION.C F0297/F0298 preserve the
 * transient G4055 leader-hand thing separately from champion equipment.
 * Firestaff imports that raw CSB thing before full CSB object-icon binding
 * exists.  This test pins the important boundary: a missing CSB icon/name
 * binding must stay blank instead of being reinterpreted through DM1's
 * G0237/F0033 object tables.
 */

#include "m11_game_view.h"
#include "csb_v1_boot.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++g_failures;
    }
}

static void write_u16(unsigned char *p, unsigned int value)
{
    p[0] = (unsigned char)(value & 0xffu);
    p[1] = (unsigned char)((value >> 8) & 0xffu);
}

static void init_csb_dungeon(CSB_V1_DungeonData *dungeon,
                             unsigned char *raw,
                             size_t raw_size)
{
    unsigned short group = (unsigned short)((THING_TYPE_GROUP << 10) | 0);

    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, raw_size);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 1;
    dungeon->level_offsets[0] = 66;
    dungeon->square_bytes = 1;
    dungeon->raw_map_data_base = 66;
    dungeon->square_first_thing_base = 80;
    dungeon->square_first_thing_count = 1;
    dungeon->map_door_set0[0] = 1; /* Wooden door defense: 42. */
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;
    dungeon->thing_type_counts[THING_TYPE_DOOR] = 1;
    dungeon->thing_type_counts[THING_TYPE_GROUP] = 1;
    dungeon->thing_type_counts[THING_TYPE_WEAPON] = 3;
    dungeon->thing_type_counts[THING_TYPE_POTION] = 1;
    dungeon->thing_type_counts[THING_TYPE_CONTAINER] = 1;
    dungeon->thing_data_bases[THING_TYPE_DOOR] = 112;
    dungeon->thing_data_bases[THING_TYPE_GROUP] = 96;
    dungeon->thing_data_bases[THING_TYPE_WEAPON] = 0;
    dungeon->thing_data_bases[THING_TYPE_POTION] = 12;
    dungeon->thing_data_bases[THING_TYPE_CONTAINER] = 16;

    write_u16(raw + 60, 0u);
    write_u16(raw + 62, 0u);
    write_u16(raw + 64, 0u);
    raw[67] = 0x10u; /* floor with thing-list-present, one step east. */
    write_u16(raw + 80, group);
    write_u16(raw + 96, THING_ENDOFLIST);
    write_u16(raw + 98, THING_ENDOFLIST);
    raw[100] = 6u;   /* Screamer. */
    raw[101] = 0xFFu; /* centered group cell encoding. */
    write_u16(raw + 102, 80u);
    write_u16(raw + 110, 0u); /* one creature: (count - 1) << 5. */
    write_u16(raw + 112, THING_ENDOFLIST);
    write_u16(raw + 114, 0x0100u); /* Type 0, melee destructible. */
}

int main(void)
{
    M11_GameViewState state;
    char name[32];
    unsigned short thing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);

    memset(&state, 0, sizeof(state));
    memset(name, 0, sizeof(name));
    state.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
    state.leaderHandObjectPresent = 1;
    state.leaderHandThing = thing;
    state.leaderHandIconIndex = -1;

    check(M11_GameView_GetV1LeaderHandThing(&state) == thing,
          "CSB raw leader-hand thing is preserved");
    check(M11_GameView_GetV1LeaderHandObjectIconIndex(&state) == -1,
          "CSB missing icon binding does not fall back to DM1 icon tables");
    check(!M11_GameView_GetV1LeaderHandObjectName(&state, name, sizeof(name)),
          "CSB missing name binding does not fall back to DM1 item names");
    check(name[0] == '\0',
          "CSB missing name binding leaves output blank");

    state.leaderHandIconIndex = 77;
    check(M11_GameView_GetV1LeaderHandObjectIconIndex(&state) == 77,
          "CSB explicit icon binding can still surface when present");

    snprintf(state.leaderHandObjectName,
             sizeof(state.leaderHandObjectName),
             "%s",
             "DAGGER");
    memset(name, 0, sizeof(name));
    check(M11_GameView_GetV1LeaderHandObjectName(&state, name, sizeof(name)),
          "CSB explicit runtime name binding can surface when present");
    check(strcmp(name, "DAGGER") == 0,
          "CSB explicit runtime name is returned");

    memset(name, 0, sizeof(name));
    state.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state.leaderHandIconIndex = 77;
    check(M11_GameView_GetV1LeaderHandObjectIconIndex(&state) == 77,
          "DM1 cached leader-hand icon still surfaces");

    {
        CSB_V1_BootProfile profile;
        CSB_V1_DungeonData dungeon;
        unsigned char actions[3];
        unsigned char raw[128];
        unsigned short dagger =
            (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
        unsigned short bow =
            (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
        unsigned short arrow =
            (unsigned short)((THING_TYPE_WEAPON << 10) | 2);
        unsigned short ven_potion =
            (unsigned short)((THING_TYPE_POTION << 10) | 0);
        unsigned short chest =
            (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
        int sx = 0, sy = 0, sw = 0, sh = 0;

        memset(&state, 0, sizeof(state));
        memset(&profile, 0, sizeof(profile));
        csb_v1_runtime_init(&profile.runtime, NULL);
        init_csb_dungeon(&dungeon, raw, sizeof(raw));
        write_u16(raw + 0, THING_ENDOFLIST);
        write_u16(raw + 2, 8u); /* ReDMCSB OBJECT.C F0033 dagger icon C032. */
        write_u16(raw + 4, THING_ENDOFLIST);
        write_u16(raw + 6, 25u); /* BOW: class 20, action set with SHOOT. */
        write_u16(raw + 8, THING_ENDOFLIST);
        write_u16(raw + 10, 27u); /* ARROW: class 10 bow ammunition. */
        write_u16(raw + 12, THING_ENDOFLIST);
        write_u16(raw + 14, (3u << 8) | 80u); /* VEN potion, power 80. */
        write_u16(raw + 16, THING_ENDOFLIST);
        write_u16(raw + 18, THING_ENDOFLIST); /* CONTAINER.Slot. */
        write_u16(raw + 20, 0u); /* Chest subtype 0; ObjectInfo idx 1. */
        write_u16(raw + 22, 0u);

        profile.runtime.dungeon_handle = &dungeon;
        profile.runtime.dungeon_seed = 0x1234u;
        profile.runtime.party_state_valid = 1;
        profile.runtime.current_level = 0;
        profile.runtime.party_x = 0;
        profile.runtime.party_y = 0;
        profile.runtime.party_dir = 1;
        profile.runtime.party_state.ChampionCount = 1;
        profile.runtime.party_state.LeaderHandThing = THING_NONE;
        profile.runtime.party_state.Champions[0].CurrentHealth = 10;
        profile.runtime.party_state.Champions[0].CurrentStamina = 100;
        profile.runtime.party_state.Champions[0].MaximumStamina = 100;
        profile.runtime.party_state.Champions[0].CurrentMana = 10;
        profile.runtime.party_state.Champions[0].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] =
            40;
        profile.runtime.party_state.Champions[0].Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_CUR] =
            120;
        profile.runtime.party_state.Champions[0].Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_CUR] =
            80;
        profile.runtime.party_state.Champions[0].Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_MAX] =
            100;
        profile.runtime.party_state.Champions[0].Skills[10] = 3;
        profile.runtime.party_state.Champions[0].Skills[4] = 8;
        profile.runtime.party_state.Champions[0].Cell = 0;
        profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
            dagger;
        state.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
        state.csbBootProfile = &profile;
        state.active = 1;
        state.inventoryPanelActive = 1;
        state.world.party.championCount = 1;
        state.world.party.activeChampionIndex = 0;
        state.world.party.champions[0].present = 1;
        state.world.party.champions[0].hp.current = 10;
        state.world.party.champions[0].hp.maximum = 10;
        state.world.party.champions[0].stamina.current = 100;
        state.world.party.champions[0].stamina.maximum = 100;
        state.world.party.champions[0].mana.current = 10;
        state.world.party.champions[0].mana.maximum = 10;
        state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
            dagger;

        check(M11_GameView_GetObjectIconIndexForThing(&state, dagger) == 32,
              "CSB object icon accessor uses CSB runtime dungeon records without DM1 world.things");
        check(M11_GameView_GetV1InventorySlotIconIndex(
                  &state,
                  CHAMPION_SLOT_ACTION_HAND) == 32,
              "CSB inventory slot icon accessor uses CSB runtime records without DM1 world.things");
        check(M11_GameView_SetV1LeaderHandObject(&state, dagger),
              "CSB leader-hand set accepts runtime-owned object binding");
        check(M11_GameView_GetV1LeaderHandObjectIconIndex(&state) == 32,
              "CSB leader-hand icon resolves through CSB runtime object binding");
        memset(name, 0, sizeof(name));
        check(M11_GameView_GetV1LeaderHandObjectName(&state, name, sizeof(name)),
              "CSB leader-hand name resolves through CSB runtime object binding");
        check(strcmp(name, "DAGGER") == 0,
              "CSB leader-hand name comes from CSB runtime resolver");
        check(M11_GameView_HandlePointerButton(
                  &state,
                  12 + 8,
                  33 + 13 + 8,
                  M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_REDRAW,
              "CSB inventory eye click inspects leader-hand object through runtime name path");
        check(strstr(state.inspectTitle, "DAGGER") != NULL,
              "CSB eye inspect title uses runtime object name");
        check(state.v1ObjectDescriptionPanelActive == 1 &&
                  strcmp(state.v1ObjectDescriptionName, "DAGGER") == 0,
              "CSB object-description panel stores runtime object name");
        (void)M11_GameView_DismissDialogOverlay(&state);
        check(M11_GameView_SetActingChampion(&state, 0),
              "CSB action menu opens from runtime object action-set without DM1 world.things");
        check(M11_GameView_GetActingActionIndices(&state, actions),
              "CSB action rows resolve through runtime object action-set");
        check(actions[0] == 42 && actions[1] == 9 && actions[2] == 28,
              "CSB dagger exposes THROW/STAB/SLASH action rows");
        check(M11_GameView_TriggerActionRow(&state, 0) == 1,
              "CSB dagger THROW action dispatches through CSB runtime without DM1 world.things");
        check(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] ==
                  THING_NONE,
              "CSB THROW clears M11 action-hand mirror");
        check(profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] ==
                  THING_NONE,
              "CSB THROW clears runtime action-hand slot");
        check(profile.runtime.projectiles.count == 1,
              "CSB THROW allocates one runtime projectile");
        check(profile.runtime.projectiles.entries[0].reserved1 == dagger,
              "CSB THROW projectile preserves thrown thing identity");
        check(profile.runtime.projectiles.entries[0].ownerKind ==
                  PROJECTILE_OWNER_CHAMPION,
              "CSB THROW projectile is champion-owned");
        check(profile.runtime.projectiles.entries[0].kineticEnergy != 64,
              "CSB THROW uses source-style strength/weight kinetic energy instead of fixed bridge value");
        check(profile.runtime.projectiles.entries[0].attack >= 40 &&
                  profile.runtime.projectiles.entries[0].attack <= 200,
              "CSB THROW attack is source-bounded");
        check(profile.runtime.projectiles.entries[0].stepEnergy >= 5,
              "CSB THROW step energy is source-bounded");
        check(profile.runtime.timeline_queue.eventCount > 0,
              "CSB THROW schedules first projectile movement event");
        state.actionDisabledTicks[0] = 0;
        state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
            dagger;
        state.world.party.champions[0].stamina.current = 100;
        profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
            dagger;
        profile.runtime.party_state.Champions[0].CurrentStamina = 100;
        profile.runtime.party_state.Champions[0]
            .Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] = 140;
        check(M11_GameView_SetActingChampion(&state, 0),
              "CSB action menu reopens for melee action without DM1 world.things");
        check(M11_GameView_TriggerActionRow(&state, 1) == 1,
              "CSB dagger STAB action records through CSB runtime without DM1 attack tick");
        check(profile.runtime.party_state.Champions[0].ActionIndex == actions[1],
              "CSB STAB stores selected action index on runtime champion");
        check(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] ==
                  dagger,
              "CSB STAB keeps action-hand object in M11 mirror");
        check(profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] ==
                  dagger,
              "CSB STAB keeps action-hand object in runtime slot");
        check(profile.runtime.projectiles.count == 1,
              "CSB STAB does not allocate a projectile");
        check((int)(raw[102] | (raw[103] << 8)) < 80,
              "CSB STAB applies F0402/F0231 damage to runtime C04 group HP");
        check(profile.runtime.party_state.Champions[0].CurrentStamina < 100,
              "CSB STAB writes M11 stamina cost back to runtime");
        state.actionDisabledTicks[0] = 0;
        raw[67] = (unsigned char)((4u << 5) | 0x10u | 4u);
        write_u16(raw + 80, (unsigned short)((THING_TYPE_DOOR << 10) | 0));
        write_u16(raw + 2, 2u); /* Weapon type 2: action set 5 => SWING. */
        state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
            dagger;
        state.world.party.champions[0].stamina.current = 100;
        profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
            dagger;
        profile.runtime.party_state.Champions[0].CurrentStamina = 100;
        check(M11_GameView_SetActingChampion(&state, 0),
              "CSB door-hit action menu opens from runtime action set");
        check(M11_GameView_GetActingActionIndices(&state, actions),
              "CSB door-hit action rows resolve through runtime");
        check(actions[0] == DM1_ACTION_SWING,
              "CSB weapon exposes SWING door-hit action row");
        {
            int event_count_before = profile.runtime.timeline_queue.eventCount;
            check(M11_GameView_TriggerActionRow(&state, 0) == 1,
                  "CSB SWING closed-door action dispatches through CSB runtime");
            check((raw[67] & 0x07u) == 4u,
                  "CSB SWING leaves closed door unchanged before C02 event");
            check(profile.runtime.timeline_queue.eventCount ==
                      event_count_before + 1,
                  "CSB SWING schedules one C02 door destruction event");
            if (profile.runtime.timeline_queue.eventCount > event_count_before) {
                int saw_door_destruction = 0;
                int i;
                for (i = 0; i < profile.runtime.timeline_queue.eventCount; ++i) {
                    unsigned short event_index =
                        profile.runtime.timeline_queue.timeline[i];
                    if (profile.runtime.timeline_queue.events[event_index].type ==
                            DM1_EVENT_DOOR_DESTRUCTION &&
                        profile.runtime.timeline_queue.events[event_index].b_mapX == 1 &&
                        profile.runtime.timeline_queue.events[event_index].b_mapY == 0) {
                        saw_door_destruction = 1;
                    }
                }
                check(saw_door_destruction,
                      "CSB SWING schedules C02 door destruction on the closed door square");
            }
        }
        state.actionDisabledTicks[0] = 0;
        state.world.party.champions[0].mana.current = 20;
        profile.runtime.party_state.Champions[0].CurrentMana = 20;
        check(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state,
                  0,
                  DM1_ACTION_FIREBALL) == 1,
              "CSB FIREBALL action spawns through CSB runtime without DM1 projectile list");
        check(profile.runtime.projectiles.count == 2,
              "CSB FIREBALL allocates one additional runtime projectile");
        check(profile.runtime.projectiles.entries[1].projectileCategory ==
                  PROJECTILE_CATEGORY_MAGICAL,
              "CSB FIREBALL projectile is magical");
        check(profile.runtime.projectiles.entries[1].projectileSubtype ==
                  PROJECTILE_SUBTYPE_FIREBALL,
              "CSB FIREBALL projectile keeps source subtype");
        check(profile.runtime.party_state.Champions[0].ActionIndex ==
                  DM1_ACTION_FIREBALL,
              "CSB FIREBALL stores selected action index on runtime champion");
        check(profile.runtime.party_state.Champions[0].CurrentMana < 20,
              "CSB FIREBALL writes mana cost back to runtime");
        check(state.world.projectiles.count == 0,
              "CSB FIREBALL does not allocate into DM1 M11 projectile list");
        state.actionDisabledTicks[0] = 0;
        state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
            bow;
        state.world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] =
            arrow;
        profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
            bow;
        profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_READY_HAND] =
            arrow;
        profile.runtime.party_state.Champions[0].CurrentStamina = 100;
        check(M11_GameView_SetActingChampion(&state, 0),
              "CSB bow action menu opens from runtime action set");
        check(M11_GameView_GetActingActionIndices(&state, actions),
              "CSB bow action rows resolve through runtime");
        if (!(actions[0] == 32 && actions[1] == 255 && actions[2] == 255)) {
            fprintf(stderr,
                    "CSB bow actions: %u,%u,%u\n",
                    (unsigned)actions[0],
                    (unsigned)actions[1],
                    (unsigned)actions[2]);
        }
        check(actions[0] == 32 && actions[1] == 255 && actions[2] == 255,
              "CSB bow exposes source-locked SHOOT action row");
        check(M11_GameView_TriggerActionRow(&state, 0) == 1,
              "CSB SHOOT action dispatches through CSB runtime without DM1 world.things");
        check(profile.runtime.projectiles.count == 3,
              "CSB SHOOT allocates one additional runtime projectile");
        check(profile.runtime.projectiles.entries[2].projectileCategory ==
                  PROJECTILE_CATEGORY_KINETIC,
              "CSB SHOOT projectile is kinetic");
        check(profile.runtime.projectiles.entries[2].reserved1 == arrow,
              "CSB SHOOT projectile preserves ammunition thing identity");
        check(profile.runtime.party_state.Champions[0].ActionIndex ==
                  DM1_ACTION_SHOOT,
              "CSB SHOOT stores selected action index on runtime champion");
        check(state.world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] ==
                  THING_NONE,
              "CSB SHOOT clears M11 ready-hand mirror");
        check(profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_READY_HAND] ==
                  THING_NONE,
              "CSB SHOOT clears runtime ready-hand slot");
        check(state.world.projectiles.count == 0,
              "CSB SHOOT does not allocate into DM1 M11 projectile list");
        state.actionDisabledTicks[0] = 0;
        state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
            ven_potion;
        state.world.party.champions[0].stamina.current = 100;
        profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
            ven_potion;
        profile.runtime.party_state.Champions[0].CurrentStamina = 100;
        check(M11_GameView_SetActingChampion(&state, 0),
              "CSB Ven potion action menu opens from runtime action set");
        check(M11_GameView_GetActingActionIndices(&state, actions),
              "CSB Ven potion action rows resolve through runtime");
        check(actions[0] == 42 && actions[1] == 255 && actions[2] == 255,
              "CSB Ven potion exposes source-locked THROW action row");
        check(M11_GameView_TriggerActionRow(&state, 0) == 1,
              "CSB Ven potion THROW dispatches through CSB runtime");
        check(profile.runtime.projectiles.count == 4,
              "CSB Ven potion THROW allocates one additional runtime projectile");
        check(profile.runtime.projectiles.entries[3].projectileSubtype ==
                  PROJECTILE_SUBTYPE_POISON_CLOUD,
              "CSB Ven potion THROW creates poison-cloud projectile subtype");
        check(profile.runtime.projectiles.entries[3].associatedPotionPower == 80,
              "CSB Ven potion THROW preserves potion power for impact");
        check(profile.runtime.projectiles.entries[3].reserved1 == ven_potion,
              "CSB Ven potion THROW preserves potion thing identity");
        check(state.world.projectiles.count == 0,
              "CSB Ven potion THROW does not allocate into DM1 M11 projectile list");

        M11_GameView_ClearV1LeaderHandObject(&state);
        state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
            chest;
        profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
            chest;
        write_u16(raw + 12, THING_ENDOFLIST);
        write_u16(raw + 18, ven_potion);
        check(M11_GameView_GetV1InventorySlotIconIndex(
                  &state,
                  CHAMPION_SLOT_ACTION_HAND) == 144,
              "CSB non-empty chest keeps container icon from CONTAINER.Type, not Slot");
        check(M11_GameView_GetV1InventorySourceSlotBoxZone(
                  9, &sx, &sy, &sw, &sh),
              "C508 action-hand slot zone is available for CSB inventory click");
        check(M11_GameView_HandlePointerButton(
                  &state,
                  sx + sw / 2,
                  33 + sy + sh / 2,
                  M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_REDRAW,
              "CSB action-hand slot click picks container into leader hand");
        check(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] ==
                  THING_NONE,
              "CSB action-hand M11 mirror is empty after pickup");
        check(profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] ==
                  THING_NONE,
              "CSB runtime action-hand slot is empty after pickup");
        check(M11_GameView_GetV1LeaderHandThing(&state) == chest,
              "CSB leader hand holds picked-up container");
        check(profile.runtime.party_state.LeaderHandThing == chest,
              "CSB runtime leader hand holds picked-up container");
        check(M11_GameView_HandlePointerButton(
                  &state,
                  sx + sw / 2,
                  33 + sy + sh / 2,
                  M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_REDRAW,
              "CSB action-hand slot click places container from leader hand");
        check(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] ==
                  chest,
              "CSB action-hand M11 mirror stores placed container");
        check(profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] ==
                  chest,
              "CSB runtime action-hand slot stores placed container");
        check(M11_GameView_GetV1LeaderHandThing(&state) == THING_NONE,
              "CSB leader hand is empty after placing container");
        check(profile.runtime.party_state.LeaderHandThing == THING_NONE,
              "CSB runtime leader hand is empty after placing container");
        check(M11_GameView_OpenV1ActionHandChest(&state),
              "CSB action-hand chest opens from runtime container record");
        check(M11_GameView_GetV1OpenChestThing(&state) == chest,
              "CSB open chest state preserves raw container thing");
        check(M11_GameView_GetV1ChestSlotBoxZone(0, &sx, &sy, &sw, &sh),
              "C537 first chest slot zone is available");
        check(M11_GameView_HandlePointerButton(
                  &state,
                  sx + sw / 2,
                  33 + sy + sh / 2,
                  M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_REDRAW,
              "CSB chest slot click picks contained object through runtime container chain");
        check(M11_GameView_GetV1LeaderHandThing(&state) == ven_potion,
              "CSB leader hand holds object picked from chest panel");
        check((unsigned short)(raw[18] | ((unsigned short)raw[19] << 8)) ==
                  THING_ENDOFLIST,
              "CSB container Slot is empty after picking the only visible chest object");
        check((unsigned short)(raw[12] | ((unsigned short)raw[13] << 8)) ==
                  THING_ENDOFLIST,
              "CSB picked chest object is detached from the runtime Next chain");
        check(M11_GameView_HandlePointerButton(
                  &state,
                  sx + sw / 2,
                  33 + sy + sh / 2,
                  M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_REDRAW,
              "CSB chest slot click places leader-hand object back into runtime container");
        check(M11_GameView_GetV1LeaderHandThing(&state) == THING_NONE,
              "CSB leader hand is empty after placing object back in chest");
        check((unsigned short)(raw[18] | ((unsigned short)raw[19] << 8)) ==
                  ven_potion,
              "CSB container Slot points at placed object after chest writeback");
        check((unsigned short)(raw[12] | ((unsigned short)raw[13] << 8)) ==
                  THING_ENDOFLIST,
              "CSB returned chest object terminates the compact runtime chain");

        {
            int empty_projectile;
            int full_projectile;
            int empty_energy;
            int full_energy;
            int projectile_count_before_m11;
            int route_space = M11_DM1_MOUSE_SPACE_NONE;
            int route_zone = 0;
            int route_command;

            state.actionDisabledTicks[0] = 0;
            profile.runtime.game_time = 200;
            profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
                chest;
            write_u16(raw + 18, THING_ENDOFLIST);
            check(csb_v1_runtime_throw_action_hand(
                      &profile.runtime,
                      0,
                      &empty_projectile) == 1,
                  "CSB runtime can throw an empty chest from action hand");
            empty_energy =
                profile.runtime.projectiles.entries[empty_projectile].kineticEnergy;

            profile.runtime.game_time = 200;
            profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
                chest;
            write_u16(raw + 18, ven_potion);
            check(csb_v1_runtime_throw_action_hand(
                      &profile.runtime,
                      0,
                      &full_projectile) == 1,
                  "CSB runtime can throw a filled chest from action hand");
            full_energy =
                profile.runtime.projectiles.entries[full_projectile].kineticEnergy;
            check(full_energy != empty_energy,
                  "CSB filled-chest THROW energy includes CONTAINER.Slot contents");

            projectile_count_before_m11 = profile.runtime.projectiles.count;
            state.inventoryPanelActive = 0;
            state.v1ObjectDescriptionPanelActive = 0;
            state.dialogOverlayActive = 0;
            state.actingChampionOrdinal = 0;
            state.showDebugHUD = 0;
            state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
                dagger;
            profile.runtime.party_state.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] =
                dagger;
            write_u16(raw + 18, ven_potion);
            check(M11_GameView_SetV1LeaderHandObject(&state, chest),
                  "CSB leader hand can hold a filled chest before viewport throw");
            route_command = M11_GameView_GetV1MouseCommandForPoint(
                M11_DM1_MOUSE_LIST_MOVEMENT,
                100,
                33 + 20,
                M11_DM1_MOUSE_MASK_LEFT,
                &route_space,
                &route_zone);
            check(route_command == 80 &&
                      route_space == M11_DM1_MOUSE_SPACE_SCREEN &&
                      route_zone == 7,
                  "CSB viewport throw test point resolves to C080/C007");
            check(M11_GameView_HandlePointerButton(
                      &state,
                      100,
                      33 + 20,
                      M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_REDRAW,
                  "CSB viewport leader-hand throw routes through CSB runtime");
            check(M11_GameView_GetV1LeaderHandThing(&state) == THING_NONE,
                  "CSB leader hand clears after accepted viewport throw");
            check(profile.runtime.party_state.Champions[0]
                      .Slots[CSB_V1_SLOT_ACTION_HAND] == dagger,
                  "CSB leader-hand throw restores runtime action hand");
            check(state.world.party.champions[0]
                      .inventory[CHAMPION_SLOT_ACTION_HAND] == dagger,
                  "CSB leader-hand throw restores M11 action-hand mirror");
            check(profile.runtime.projectiles.count ==
                      projectile_count_before_m11 + 1,
                  "CSB leader-hand throw creates one runtime projectile");
            check(profile.runtime.projectiles
                      .entries[projectile_count_before_m11]
                      .reserved1 == chest,
                  "CSB leader-hand throw projectile preserves chest identity");
            check(state.world.projectiles.count == 0,
                  "CSB leader-hand throw does not allocate DM1 projectiles");
        }
    }

    if (g_failures != 0) {
        return 1;
    }
    printf("PASS: m11_csb_leader_hand_no_dm1_fallback\n");
    return 0;
}
