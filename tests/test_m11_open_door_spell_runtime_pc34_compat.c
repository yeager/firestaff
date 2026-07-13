/*
 * Source-lock gate for live DM1 V1 Open Door spell door-impact routing.
 *
 * ReDMCSB evidence:
 *   MENU.C lines 66 and 1867-1870: ZO / Open Door is projectile spell type 4
 *     and doubles skill before F0327 projectile creation.
 *   PROJEXPL.C lines 471-489: C0xFF84 OPEN_DOOR impacts any non-destroyed
 *     door before normal open/pass-through checks; button doors enqueue
 *     C10_EVENT_DOOR / C02_EFFECT_TOGGLE for GameTime+1.
 *   PROJEXPL.C lines 491-508 and 1554-1599: the normal door-destruction
 *     branch is separate and is not used by OPEN_DOOR.
 *   COMMAND.C lines 473-483 and 2302-2307 plus CLIKMENU.C lines 484-497:
 *     the spell-area cast zone routes through F0408 before the F0412 cast.
 *   CHAMPION.C lines 2097-2102: F0327 derives projectile step energy
 *     from MaximumMana and calls F0326/F0212 with attack 90.
 */

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_door_action_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

static const struct TimelineEvent_Compat* find_timeline_event(
    const struct TimelineQueue_Compat* timeline,
    int kind,
    int aux3) {
    int i;

    for (i = 0; i < timeline->count; ++i) {
        const struct TimelineEvent_Compat* event = &timeline->events[i];
        if (event->kind == kind && (aux3 < 0 || event->aux3 == aux3)) {
            return event;
        }
    }
    return NULL;
}

static void seed_open_door_spell_state(M11_GameViewState* state,
                                       struct DungeonDatState_Compat* dungeon,
                                       struct DungeonMapDesc_Compat maps[1],
                                       struct DungeonMapTiles_Compat tiles[1],
                                       unsigned char mapTiles[2],
                                       struct DungeonThings_Compat* things,
                                       struct DungeonDoor_Compat doors[1],
                                       unsigned short squareFirstThings[2]) {
    struct ProjectileCreateInput_Compat input;
    struct TimelineEvent_Compat firstMove;
    int slot = -1;

    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat));
    memset(tiles, 0, sizeof(struct DungeonMapTiles_Compat));
    memset(things, 0, sizeof(*things));
    memset(doors, 0, sizeof(struct DungeonDoor_Compat));
    memset(squareFirstThings, 0, sizeof(unsigned short) * 2);

    maps[0].width = 2;
    maps[0].height = 1;
    mapTiles[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    mapTiles[1] = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | 4);
    tiles[0].squareData = mapTiles;
    tiles[0].squareCount = 2;
    dungeon->header.mapCount = 1;
    dungeon->header.squareFirstThingCount = 2;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    doors[0].next = THING_ENDOFLIST;
    doors[0].button = 1;
    things->doors = doors;
    things->doorCount = 1;
    things->thingCounts[THING_TYPE_DOOR] = 1;
    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = 2;
    things->loaded = 1;
    squareFirstThings[0] = THING_ENDOFLIST;
    squareFirstThings[1] = (unsigned short)((THING_TYPE_DOOR << 10) | 0);

    M11_GameView_Init(state);
    state->active = 1;
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.gameTick = 10;
    state->world.partyMapIndex = 0;
    state->world.newPartyMapIndex = 0;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 0;
    state->world.party.mapY = 0;
    state->world.party.direction = 1;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;

    memset(&input, 0, sizeof(input));
    input.category = PROJECTILE_CATEGORY_MAGICAL;
    input.subtype = PROJECTILE_SUBTYPE_OPEN_DOOR;
    input.ownerKind = PROJECTILE_OWNER_CHAMPION;
    input.ownerIndex = 0;
    input.mapIndex = 0;
    input.mapX = 0;
    input.mapY = 0;
    input.cell = 1;
    input.direction = 1;
    input.kineticEnergy = 90;
    input.attack = 80;
    input.stepEnergy = 1;
    input.currentTick = 9;
    input.attackTypeCode = COMBAT_ATTACK_MAGIC;
    input.firstMoveGraceFlag = 0;
    ASSERT_EQ(F0810_PROJECTILE_Create_Compat(&input, &state->world.projectiles,
                                             &slot, &firstMove), 1,
              "open-door projectile seeded");
    state->world.projectiles.entries[slot].scheduledAtTick = 10;
}

static void test_open_door_projectile_schedules_delayed_toggle_and_animates(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[2];
    struct DungeonThings_Compat things;
    struct DungeonDoor_Compat doors[1];
    unsigned short squareFirstThings[2];

    seed_open_door_spell_state(&state, &dungeon, maps, tiles, mapTiles,
                               &things, doors, squareFirstThings);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1, "projectile is live before impact");
    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0, "open-door projectile is consumed on door impact");
    ASSERT_EQ(state.world.timeline.count, 1, "door toggle animation event scheduled");
    ASSERT_EQ(state.world.timeline.events[0].kind, TIMELINE_EVENT_DOOR_ANIMATE,
              "scheduled event is door animation");
    ASSERT_EQ((int)state.world.timeline.events[0].fireAtTick, 11,
              "door animation starts at GameTime+1");
    ASSERT_EQ(state.world.timeline.events[0].aux1, DOOR_EFFECT_SET,
              "closed button door resolves toggle to opening effect");
    ASSERT_EQ(mapTiles[1] & 0x07, 4, "door remains closed until delayed event fires");

    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "first idle tick reaches delayed event tick");
    ASSERT_EQ(mapTiles[1] & 0x07, 4,
              "door is still closed until the queued tick is dispatched");
    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "second idle tick dispatches delayed door animation");
    ASSERT_EQ(mapTiles[1] & 0x07, 3, "first delayed animation step opens door one state");
}

static void test_open_door_ui_cast_launches_source_projectile(void) {
    M11_GameViewState state;
    struct ProjectileInstance_Compat* projectile;
    const struct TimelineEvent_Compat* projectileMove;
    const struct TimelineEvent_Compat* enableAction;

    memset(&state, 0, sizeof(state));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.gameTick = 40;
    state.world.partyMapIndex = 0;
    state.world.party.mapIndex = 0;
    state.world.party.mapX = 3;
    state.world.party.mapY = 4;
    state.world.party.direction = 1;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].mana.current = 80;
    state.world.party.champions[0].mana.maximum = 80;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_WISDOM] = 80;
    state.world.lifecycle.champions[0].skills20[LIFECYCLE_SKILL_WIZARD].experience = 8000;
    state.world.lifecycle.champions[0].skills20[LIFECYCLE_SKILL_AIR].experience = 8000;

    ASSERT_EQ(M11_GameView_OpenSpellPanel(&state), 1, "spell panel opens");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 0), 1, "LO power rune entered");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 5), 1, "ZO element rune entered");

    ASSERT_EQ(M11_GameView_CastSpell(&state), 1, "ZO/Open Door cast consumed");
    ASSERT_EQ(state.spellPanelOpen, 0, "cast closes source spell panel");
    ASSERT_EQ(state.spellBuffer.runeCount, 0, "cast clears source rune buffer");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "Open Door UI cast launches one live projectile");
    /* MENU.C F0412 calls F0330 after a successful spell.  Its C11 receipt
     * coexists with F0327's projectile move; queue position is not ownership. */
    ASSERT_EQ(state.world.timeline.count, 2,
              "Open Door UI cast retains F0327 move and F0330 C11 receipts");
    projectileMove = find_timeline_event(&state.world.timeline,
                                         TIMELINE_EVENT_PROJECTILE_MOVE,
                                         PROJECTILE_SUBTYPE_OPEN_DOOR);
    enableAction = find_timeline_event(&state.world.timeline,
                                       TIMELINE_EVENT_ENABLE_CHAMPION_ACTION,
                                       -1);
    ASSERT_EQ(projectileMove != NULL, 1,
              "Open Door UI cast schedules its typed projectile move");
    ASSERT_EQ(enableAction != NULL, 1,
              "Open Door UI cast retains the F0330 C11 receipt");
    if (enableAction != NULL) {
        ASSERT_EQ(enableAction->aux0, DM1_EVENT_ENABLE_CHAMPION_ACTION,
                  "C11 receipt retains original event type");
        ASSERT_EQ(enableAction->aux1, 0,
                  "C11 receipt retains F0330 slot ordinal zero");
        ASSERT_EQ(enableAction->aux4, 0,
                  "C11 receipt retains the casting champion owner");
    }

    projectile = &state.world.projectiles.entries[0];
    ASSERT_EQ(projectile->projectileSubtype, PROJECTILE_SUBTYPE_OPEN_DOOR,
              "projectile subtype is Open Door");
    ASSERT_EQ(projectile->projectileCategory, PROJECTILE_CATEGORY_MAGICAL,
              "projectile category is magical");
    ASSERT_EQ(projectile->attack, 90, "F0327 launches spell projectile with attack 90");
    ASSERT_EQ(projectile->kineticEnergy, 84,
              "Open Door kinetic energy uses doubled Air skill formula");
    ASSERT_EQ(projectile->stepEnergy, 2,
              "step energy derived from champion maximum mana");
    ASSERT_EQ(projectile->direction, 1, "launch direction follows party direction");
    if (projectileMove != NULL) {
        ASSERT_EQ(projectileMove->aux3, PROJECTILE_SUBTYPE_OPEN_DOOR,
                  "projectile receipt carries Open Door subtype");
    }
}

static void test_open_door_cast_ages_existing_action_disable_before_f0412_disable(void) {
    M11_GameViewState state;
    struct TimelineEvent_Compat priorEnable;

    memset(&state, 0, sizeof(state));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.gameTick = 90;
    state.world.partyMapIndex = 0;
    state.world.party.mapIndex = 0;
    state.world.party.mapX = 3;
    state.world.party.mapY = 4;
    state.world.party.direction = 1;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].mana.current = 80;
    state.world.party.champions[0].mana.maximum = 80;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_WISDOM] = 80;
    state.world.lifecycle.champions[0].skills20[LIFECYCLE_SKILL_WIZARD].experience = 8000;
    state.world.lifecycle.champions[0].skills20[LIFECYCLE_SKILL_AIR].experience = 8000;

    /* ReDMCSB F0330 owns the prior action lock through C11.  Do not model an
     * expiry with only M11 sidecar state: F0253 must consume the actual queue
     * owner before the later F0412/F0330 spell lock is created. */
    state.actionDisabledTicks[0] = 1;
    state.actionDisabledIndex[0] = DM1_ACTION_SHOOT;
    state.actionEnableSlotOrdinal[0] = 0;
    state.world.party.champions[0].actionIndex = DM1_ACTION_SHOOT;
    memset(&priorEnable, 0, sizeof(priorEnable));
    priorEnable.kind = TIMELINE_EVENT_ENABLE_CHAMPION_ACTION;
    priorEnable.fireAtTick = state.world.gameTick - 1;
    priorEnable.aux0 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    priorEnable.aux1 = 0;
    priorEnable.aux2 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    priorEnable.aux4 = 0;
    ASSERT_EQ(F0721_TIMELINE_Schedule_Compat(&state.world.timeline, &priorEnable), 1,
              "source C11 owner is queued before the spell cast");
    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "due C11 is consumed through F0253 before the spell cast");
    ASSERT_EQ(state.actionDisabledTicks[0], 0,
              "F0253 clears the prior action-disable owner");
    ASSERT_EQ(state.actionDisabledIndex[0], 0xFF,
              "F0253 clears the prior action index");
    ASSERT_EQ(state.actionEnableSlotOrdinal[0], 0xFF,
              "consumed C11 cannot remain a live owner");

    ASSERT_EQ(M11_GameView_OpenSpellPanel(&state), 1, "cooldown cast opens spell panel");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 0), 1, "cooldown cast enters Lo");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 5), 1, "cooldown cast enters Zo");
    ASSERT_EQ(M11_GameView_CastSpell(&state), 1, "cooldown cast is committed");

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "F0412 still materializes the live Open Door projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].direction, 1,
              "materialized projectile keeps the party-facing direction");
    ASSERT_EQ(state.actionDisabledTicks[0], 15,
              "new F0412 action disable does not lose its creation tick");
    ASSERT_EQ(state.actionDisabledIndex[0], 0xFF,
              "expired action row does not leak into spell disable state");
    ASSERT_EQ(state.actionEnableSlotOrdinal[0], 0,
              "F0330 spell disable retains source slot ordinal zero");
}

static void test_open_door_ui_cast_insufficient_mana_preserves_runes_and_caster(void) {
    M11_GameViewState state;
    struct ChampionState_Compat beforeChampion;
    unsigned char beforeRunes[4];
    int beforeRuneCount;
    int beforeRuneRow;
    int beforeActiveChampion;
    uint32_t beforeGameTick;

    memset(&state, 0, sizeof(state));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.gameTick = 70;
    state.world.partyMapIndex = 0;
    state.world.party.mapIndex = 0;
    state.world.party.mapX = 3;
    state.world.party.mapY = 4;
    state.world.party.direction = 1;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].mana.current = 0;
    state.world.party.champions[0].mana.maximum = 80;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_WISDOM] = 80;
    state.world.lifecycle.champions[0].skills20[LIFECYCLE_SKILL_WIZARD].experience = 8000;
    state.world.lifecycle.champions[0].skills20[LIFECYCLE_SKILL_AIR].experience = 8000;

    ASSERT_EQ(M11_GameView_OpenSpellPanel(&state), 1, "spell panel opens for low-mana cast");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 0), 1, "LO power rune entered for low-mana cast");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 5), 1, "ZO element rune entered for low-mana cast");

    beforeChampion = state.world.party.champions[0];
    memcpy(beforeRunes, state.spellBuffer.runes, sizeof(beforeRunes));
    beforeRuneCount = state.spellBuffer.runeCount;
    beforeRuneRow = state.spellRuneRow;
    beforeActiveChampion = state.world.party.activeChampionIndex;
    beforeGameTick = state.world.gameTick;

    ASSERT_EQ(M11_GameView_CastSpell(&state), 1, "insufficient mana cast consumes input");
    ASSERT_EQ(state.spellPanelOpen, 0, "insufficient mana closes spell panel feedback");
    ASSERT_EQ(state.spellBuffer.runeCount, beforeRuneCount,
              "insufficient mana preserves selected rune count");
    ASSERT_EQ(state.spellRuneRow, beforeRuneRow,
              "insufficient mana preserves selected rune row");
    ASSERT_EQ(memcmp(state.spellBuffer.runes, beforeRunes, sizeof(beforeRunes)), 0,
              "insufficient mana preserves selected rune bytes");
    ASSERT_EQ(state.world.party.activeChampionIndex, beforeActiveChampion,
              "insufficient mana preserves active caster index");
    ASSERT_EQ(memcmp(&state.world.party.champions[0], &beforeChampion, sizeof(beforeChampion)), 0,
              "insufficient mana does not mutate caster state");
    ASSERT_EQ((int)state.world.gameTick, (int)beforeGameTick,
              "insufficient mana does not advance game tick");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "insufficient mana does not launch projectile");
    ASSERT_EQ(state.world.timeline.count, 0,
              "insufficient mana does not schedule projectile movement");
}

int main(void) {
    printf("=== M11 Open Door Spell Runtime Source-Lock Gate ===\n");
    printf("ReDMCSB: MENU.C Open Door projectile; PROJEXPL.C door impact C10/C02 toggle branch\n\n");

    test_open_door_projectile_schedules_delayed_toggle_and_animates();
    test_open_door_ui_cast_launches_source_projectile();
    test_open_door_cast_ages_existing_action_disable_before_f0412_disable();
    test_open_door_ui_cast_insufficient_mana_preserves_runes_and_caster();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
